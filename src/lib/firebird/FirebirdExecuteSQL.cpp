//==============================================================================
//    Copyright 2003,2004,2005,2008 Petr Kulhanek
//
//    This file is part of Firebird library.
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA  02110-1301  USA
//==============================================================================

#include <FirebirdExecuteSQL.hpp>
#include <string.h>
#include <stdlib.h>
#include <ErrorSystem.hpp>
#include <FirebirdTransaction.hpp>
#include <FirebirdDatabase.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CFirebirdExecuteSQL::CFirebirdExecuteSQL(void)
{
    Transaction = NULL;
    sqlda = NULL;
}

//------------------------------------------------------------------------------

CFirebirdExecuteSQL::~CFirebirdExecuteSQL(void)
{
    FreeInputItems();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdExecuteSQL::AssignToTransaction(CFirebirdTransaction* p_transaction)
{
    Transaction = p_transaction;
    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdExecuteSQL::AllocateInputItems(int numofitems)
{
    FreeInputItems();

    if(numofitems <= 0) {
        sqlda = NULL;
        return(true);
    }

    sqlda = (XSQLDA ISC_FAR*) malloc(XSQLDA_LENGTH(numofitems));
    if(sqlda == NULL) {
        ES_ERROR("unable to allocate memory for items");
        return(false);
    }

    sqlda->sqld = numofitems;
    sqlda->sqln = numofitems;
    sqlda->version = 1;

// clear all fields
    for(int i = 0; i < sqlda->sqln; i++) {
        sqlda->sqlvar[i].sqldata = NULL;
        sqlda->sqlvar[i].sqltype = SQL_TEXT;
        sqlda->sqlvar[i].sqllen  = 0;
        sqlda->sqlvar[i].sqlind = NULL;
    }

// allocate necessary items
    for(int i = 0; i < sqlda->sqln; i++) {
        sqlda->sqlvar[i].sqlind = (short*)malloc(sizeof(short));
        if(sqlda->sqlvar[i].sqlind == NULL) {
            for(int j = 0; j < i; j++) {
                free(sqlda->sqlvar[i].sqlind);
            }
            FreeInputItems();
            ES_ERROR("unable to allocate memory for item");
            return(false);
        }
        *(sqlda->sqlvar[i].sqlind) = -1;
    }

    return(true);
}

//------------------------------------------------------------------------------

void CFirebirdExecuteSQL::FreeInputItems(void)
{
    if(sqlda == NULL) {
        return;
    }

    for(int i = 0; i < sqlda->sqln; i++) {
        if(sqlda->sqlvar[i].sqldata != NULL) {
            free(sqlda->sqlvar[i].sqldata);
        }
        if(sqlda->sqlvar[i].sqlind != NULL) {
            free(sqlda->sqlvar[i].sqlind);
        }
    }

    free(sqlda);
    sqlda = NULL;

    return;
}

//------------------------------------------------------------------------------

CFirebirdItem* CFirebirdExecuteSQL::GetInputItem(int index)
{
    if(sqlda == NULL) {
        ES_ERROR("Input items are not allocated !");
        return(NULL);
    }

    XSQLVAR ISC_FAR* var = sqlda->sqlvar;

    if((index < 0) || (index >= sqlda->sqln)) {
        ES_ERROR("Index of input item is out of legal range !");
        return(NULL);
    }

    for(int i = 0; i < index; i++) {
        var++;
    }

    return((CFirebirdItem*)var);
}

//------------------------------------------------------------------------------

bool CFirebirdExecuteSQL::ExecuteSQL(const CSmallString& command)
{
    if(Transaction == NULL) {
        ES_ERROR("Transaction is not assigned !");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("Transaction is not active !");
        return(false);
    }

    if(isc_dsql_execute_immediate(StatusVector, &Transaction->Database->DatabaseHandle, &Transaction->TransactionHandle, 0, command, 3, sqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

