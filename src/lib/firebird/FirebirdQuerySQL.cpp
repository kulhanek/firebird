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

#include <FirebirdQuerySQL.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FirebirdTransaction.hpp>
#include <FirebirdDatabase.hpp>
#include <ErrorSystem.hpp>
#include <FirebirdItem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CFirebirdQuerySQL::CFirebirdQuerySQL(void)
{
    Transaction = NULL;
    stmt = 0;
    insqlda = NULL;
    outsqlda = NULL;
}

//------------------------------------------------------------------------------

CFirebirdQuerySQL::~CFirebirdQuerySQL(void)
{
    FreeInputItems();
    FreeOutputItems();
    if( stmt != 0 ) {
        if( CloseQuery() == false ){
            ES_ERROR("unable to close query handle");
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdQuerySQL::AssignToTransaction(CFirebirdTransaction* p_transaction)
{
    if(stmt != 0) {
        ES_ERROR("Query statement is open !");
        return(false);
    }

    Transaction = p_transaction;
    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdQuerySQL::PrepareQuery(const CSmallString& command)
{
    if(Transaction == NULL) {
        ES_ERROR("Transaction is not assigned !");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("Transaction is not active !");
        return(false);
    }

    if(stmt != 0) {
        ES_ERROR("Query is already opened !");
        return(false);
    }

    FreeOutputItems();
    if(AllocateOutputItems(1) == false) {
        ES_ERROR("Unable allocate initial output items !");
        return(false);
    }

    if(isc_dsql_allocate_statement(StatusVector, &Transaction->Database->DatabaseHandle, &stmt)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        return(false);
    }

    if(isc_dsql_prepare(StatusVector, &Transaction->TransactionHandle, &stmt, 0, command, 3, outsqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

    if(AllocateInputItems(1) == false) {
        ES_ERROR("Unable allocate initial input items !");
        return(false);
    }

    if(isc_dsql_describe_bind(StatusVector, &stmt, 1, insqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

// reallocate input items
    if(insqlda->sqln < insqlda->sqld) {
        if(AllocateInputItems(insqlda->sqld) == false) {
            ES_ERROR("Unable reallocate input items");
            CloseQuery();
            return(false);
        }
        if(isc_dsql_describe_bind(StatusVector, &stmt, 1, insqlda)) {
            char msg[FB_ERROR_BUFFER_LEN];
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
            const ISC_STATUS* p_status = &StatusVector[0];
            while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
                ES_ERROR(msg);
                memset(msg, 0, FB_ERROR_BUFFER_LEN);
            }
            CloseQuery();
            return(false);
        }
    }

// realocate input item fields
    if(AllocateInputItemData() == false) {
        ES_ERROR("Unable allocate data for input items");
        CloseQuery();
        return(false);
    }

    if(isc_dsql_describe(StatusVector, &stmt, 1, outsqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

// reallocate output items
    if(outsqlda->sqln < outsqlda->sqld) {
        if(AllocateOutputItems(outsqlda->sqld) == false) {
            ES_ERROR("Unable reallocate output items");
            CloseQuery();
            return(false);
        }
        if(isc_dsql_describe(StatusVector, &stmt, 1, outsqlda)) {
            char msg[FB_ERROR_BUFFER_LEN];
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
            const ISC_STATUS* p_status = &StatusVector[0];
            while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
                ES_ERROR(msg);
                memset(msg, 0, FB_ERROR_BUFFER_LEN);
            }
            CloseQuery();
            return(false);
        }
    }

// realocate output item fields
    if(AllocateOutputItemData() == false) {
        ES_ERROR("Unable allocate data for output items");
        CloseQuery();
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdQuerySQL::ExecuteQuery(void)
{
    if(Transaction == NULL) {
        ES_ERROR("Transaction is not assigned!");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("Transaction is not active!");
        return(false);
    }

    if(stmt == 0) {
        ES_ERROR("Query is not opened!");
        return(false);
    }

// ----------------------------------------------

    if(isc_dsql_execute(StatusVector, &Transaction->TransactionHandle, &stmt, 1, insqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdQuerySQL::ExecuteQueryOnce(void)
{
    if(Transaction == NULL) {
        ES_ERROR("Transaction is not assigned!");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("Transaction is not active!");
        return(false);
    }

    if(stmt == 0) {
        ES_ERROR("Query is not opened!");
        return(false);
    }

// ----------------------------------------------

    if(isc_dsql_execute2(StatusVector, &Transaction->TransactionHandle, &stmt, 1, insqlda, outsqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_WARNING(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdQuerySQL::OpenQuery(const CSmallString& command)
{
    FreeInputItems();

    if(Transaction == NULL) {
        ES_ERROR("Transaction is not assigned !");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("Transaction is not active !");
        return(false);
    }

    if(stmt != 0) {
        ES_ERROR("Query is already opened !");
        return(false);
    }

    FreeOutputItems();
    if(AllocateOutputItems(1) == false) {
        ES_ERROR("Unable allocate initial output items !");
        return(false);
    }

    if(isc_dsql_allocate_statement(StatusVector, &Transaction->Database->DatabaseHandle, &stmt)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        return(false);
    }

    if(isc_dsql_prepare(StatusVector, &Transaction->TransactionHandle, &stmt, 0, command, 3, outsqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

    if(isc_dsql_describe(StatusVector, &stmt, 1, outsqlda)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

// reallocate output items
    if(outsqlda->sqln < outsqlda->sqld) {
        if(AllocateOutputItems(outsqlda->sqld) == false) {
            ES_ERROR("Unable reallocate output items");
            CloseQuery();
            return(false);
        }
        if(isc_dsql_describe(StatusVector, &stmt, 1, outsqlda)) {
            char msg[FB_ERROR_BUFFER_LEN];
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
            const ISC_STATUS* p_status = &StatusVector[0];
            while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
                ES_ERROR(msg);
                memset(msg, 0, FB_ERROR_BUFFER_LEN);
            }
            CloseQuery();
            return(false);
        }
    }

// realocate output item fields
    if(AllocateOutputItemData() == false) {
        ES_ERROR("Unable allocate data for output items");
        CloseQuery();
        return(false);
    }

// ----------------------------------------------

    if(isc_dsql_execute(StatusVector, &Transaction->TransactionHandle, &stmt, 1, NULL)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        CloseQuery();
        return(false);
    }

    return(true);
}

//---------------------------------------------------------------------------

bool CFirebirdQuerySQL::AllocateInputItemData(void)
{
    CFirebirdItem* p_item;

    for(int i = 0; i < insqlda->sqln; i++) {
        p_item = GetInputItem(i);
        if(p_item == NULL) {
            return(false);
        }
        if(p_item->AllocateItemData() == false) {
            return(false);
        }
    }

    return(true);
}

//---------------------------------------------------------------------------

bool CFirebirdQuerySQL::AllocateOutputItemData(void)
{
    CFirebirdItem* p_item;

    for(int i = 0; i < outsqlda->sqln; i++) {
        p_item = GetOutputItem(i);
        if(p_item == NULL) {
            return(false);
        }
        if(p_item->AllocateItemData() == false) {
            return(false);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdQuerySQL::QueryRecord(void)
{
    if(Transaction == NULL) {
        ES_ERROR("Transaction is not assigned !");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("Transaction is not active !");
        return(false);
    }

    if(stmt == 0) {
        ES_ERROR("Query is not opened !");
        return(false);
    }

    if(isc_dsql_fetch(StatusVector, &stmt, 3, outsqlda) != 0) {
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

bool CFirebirdQuerySQL::CloseQuery(void)
{
    if(Transaction == NULL) {
        ES_ERROR("Transaction is not assigned !");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("Transaction is not active !");
        return(false);
    }

    if(stmt == 0) {
        ES_ERROR("Query is not opened !");
        return(false);
    }

    if(isc_dsql_free_statement(StatusVector, &stmt, DSQL_drop)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        return(false);
    }

    FreeOutputItems();

    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdQuerySQL::AllocateInputItems(int numofitems)
{
    FreeInputItems();

    if(numofitems <= 0) {
        insqlda = NULL;
        return(true);
    }

    insqlda = (XSQLDA ISC_FAR*) malloc(XSQLDA_LENGTH(numofitems));
    if(insqlda == NULL) {
        ES_ERROR("unable allocate memory for items");
        return(false);
    }

    insqlda->sqld = numofitems;
    insqlda->sqln = numofitems;
    insqlda->version = 1;

// clear all fields
    for(int i = 0; i < insqlda->sqln; i++) {
        insqlda->sqlvar[i].sqldata = NULL;
        insqlda->sqlvar[i].sqltype = SQL_TEXT;
        insqlda->sqlvar[i].sqllen  = 0;
        insqlda->sqlvar[i].sqlind = NULL;
    }

// allocate necessary items
    for(int i = 0; i < insqlda->sqln; i++) {
        insqlda->sqlvar[i].sqlind = (short*)malloc(sizeof(short));
        if(insqlda->sqlvar[i].sqlind == NULL) {
            for(int j = 0; j < i; j++) {
                free(insqlda->sqlvar[i].sqlind);
            }
            FreeInputItems();
            ES_ERROR("unable allocate memory for item");
            return(false);
        }
        *(insqlda->sqlvar[i].sqlind) = -1;
    }

    return(true);
}

//------------------------------------------------------------------------------

void CFirebirdQuerySQL::FreeInputItems(void)
{
    if(insqlda == NULL) {
        return;
    }

    for(int i = 0; i < insqlda->sqln; i++) {
        if(insqlda->sqlvar[i].sqldata != NULL) {
            free(insqlda->sqlvar[i].sqldata);
        }
        if(insqlda->sqlvar[i].sqlind != NULL) {
            free(insqlda->sqlvar[i].sqlind);
        }
    }

    free(insqlda);
    insqlda = NULL;

    return;
}

//------------------------------------------------------------------------------

CFirebirdItem* CFirebirdQuerySQL::GetInputItem(int index)
{
    if(insqlda == NULL) {
        ES_ERROR("Input items are not allocated !");
        return(NULL);
    }

    XSQLVAR ISC_FAR* var = insqlda->sqlvar;

    if((index < 0) || (index >= insqlda->sqln)) {
        ES_ERROR("Index of input item is out of legal range !");
        return(NULL);
    }

    for(int i = 0; i < index; i++) {
        var++;
    }

    return((CFirebirdItem*)var);
}

//------------------------------------------------------------------------------

bool CFirebirdQuerySQL::AllocateOutputItems(int numofitems)
{
    FreeOutputItems();

    if(numofitems <= 0) {
        outsqlda = NULL;
        return(true);
    }

    outsqlda = (XSQLDA ISC_FAR*) malloc(XSQLDA_LENGTH(numofitems));
    if(outsqlda == NULL) {
        ES_ERROR("unable allocate memory for items");
        return(false);
    }

    outsqlda->sqld = numofitems;
    outsqlda->sqln = numofitems;
    outsqlda->version = 1;

// clear all fields
    for(int i = 0; i < outsqlda->sqln; i++) {
        outsqlda->sqlvar[i].sqldata = NULL;
        outsqlda->sqlvar[i].sqltype = SQL_TEXT;
        outsqlda->sqlvar[i].sqllen  = 0;
        outsqlda->sqlvar[i].sqlind = NULL;
    }

// allocate necessary items
    for(int i = 0; i < outsqlda->sqln; i++) {
        outsqlda->sqlvar[i].sqlind = (short*)malloc(sizeof(short));
        if(outsqlda->sqlvar[i].sqlind == NULL) {
            for(int j = 0; j < i; j++) {
                free(outsqlda->sqlvar[i].sqlind);
            }
            FreeOutputItems();
            ES_ERROR("unable allocate memory for item");
            return(false);
        }
        *(outsqlda->sqlvar[i].sqlind) = -1;
    }

    return(true);
}

//------------------------------------------------------------------------------

CFirebirdItem* CFirebirdQuerySQL::GetOutputItem(int index)
{
    if(outsqlda == NULL) {
        ES_ERROR("Output items are not allocated !");
        return(NULL);
    }

    XSQLVAR ISC_FAR* var = outsqlda->sqlvar;

    if((index < 0) || (index >= outsqlda->sqln)) {
        ES_ERROR("Index of output item is out of legal range !");
        return(NULL);
    }

    for(int i = 0; i < index; i++) {
        var++;
    }

    return((CFirebirdItem*)var);
}

//------------------------------------------------------------------------------

void CFirebirdQuerySQL::FreeOutputItems(void)
{
    if(outsqlda == NULL) {
        return;
    }

    for(int i = 0; i < outsqlda->sqln; i++) {
        if(outsqlda->sqlvar[i].sqldata != NULL) {
            free(outsqlda->sqlvar[i].sqldata);
        }
        if(outsqlda->sqlvar[i].sqlind != NULL) {
            free(outsqlda->sqlvar[i].sqlind);
        }
    }

    free(outsqlda);
    outsqlda = NULL;

    return;
}

//------------------------------------------------------------------------------

