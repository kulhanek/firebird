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

#include <FirebirdTransaction.hpp>
#include <ErrorSystem.hpp>
#include <FirebirdDatabase.hpp>
#include <string.h>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CFirebirdTransaction::CFirebirdTransaction(void)
{
    Database = NULL;
    TransactionHandle = 0;
    Active = false;
}

//------------------------------------------------------------------------------

CFirebirdTransaction::~CFirebirdTransaction(void)
{
    if(IsActive() == true) {
        RollbackTransaction();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdTransaction::AssignToDatabase(CFirebirdDatabase* p_database)
{
    if(Active == true) {
        ES_ERROR("transaction is active");
        return(false);
    }

    Database = p_database;
    TransactionHandle = 0;
    Active = false;
    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdTransaction::StartTransaction(void)
{
    if(Database == NULL) {
        ES_ERROR("database is not assigned");
        return(false);
    }
    if(Database->IsLogged() == false) {
        ES_ERROR("database is not connected");
        return(false);
    }
    if(Active == true) {
        ES_ERROR("transaction is active");
        return(false);
    }

    static char isc_tpb[] = {isc_tpb_version3, isc_tpb_write};
    isc_start_transaction(StatusVector, &TransactionHandle, 1, &Database->DatabaseHandle, sizeof(isc_tpb), isc_tpb);

    if((StatusVector[0] == 1) && (StatusVector[1] > 0)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        return(false);
    }
    Active = true;
    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdTransaction::CommitTransaction(bool retaining)
{
    if(retaining == false) {
        isc_commit_transaction(StatusVector, &TransactionHandle);
    } else {
        isc_commit_retaining(StatusVector, &TransactionHandle);
    }

    if((StatusVector[0] == 1) && (StatusVector[1] > 0)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        Active = false;
        return(false);
    }
    Active = retaining;
    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdTransaction::RollbackTransaction(void)
{
    isc_rollback_transaction(StatusVector, &TransactionHandle);

    if((StatusVector[0] == 1) && (StatusVector[1] > 0)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        Active = false;
        return(false);
    }

    Active = false;
    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdTransaction::IsActive(void)
{
    return(Active);
}

//------------------------------------------------------------------------------

