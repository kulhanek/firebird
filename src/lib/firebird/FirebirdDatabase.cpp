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

#include <FirebirdDatabase.hpp>
#include <ErrorSystem.hpp>
#include <string.h>
#include <limits.h>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CFirebirdDatabase::CFirebirdDatabase(void)
{
    DatabaseHandle = 0;
    Logged = false;
}

//------------------------------------------------------------------------------

CFirebirdDatabase::~CFirebirdDatabase(void)
{
    if(Logged) {
        Logout();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CFirebirdDatabase::SetDatabaseName(const CSmallString& name)
{
    DatabaseName = name;
}

//------------------------------------------------------------------------------

const CSmallString& CFirebirdDatabase::GetDatabaseName(void)
{
    return(DatabaseName);
}

//------------------------------------------------------------------------------

bool CFirebirdDatabase::Login(const CSmallString& login, const CSmallString& password)
{
    if(Logged == true) {
        if(LoginName == login) {
            return(true);
        }
        // logout old user
        Logout();
    }

// try login new user
    LoginName = login;
    Password = password;
    DatabaseHandle = 0;

// prepare dpb structure
    ISC_SCHAR      dpb_buffer[255];
    ISC_SCHAR*     dpb;
    short          dpb_length, len;

    dpb = dpb_buffer;

    *dpb  = isc_dpb_version1;
    dpb++;

    *dpb  = isc_dpb_num_buffers;
    dpb++;
    *dpb  = 1;
    dpb++;
    *dpb  = 90;
    dpb++;

    len = LoginName.GetLength();
    if(len >= 50) {
        len = 50;
    }
    *dpb = isc_dpb_user_name;
    dpb++;
    *dpb = len;
    dpb++;
    for(int i = 0; i < len; i++) {
        *dpb = LoginName[i];
        dpb++;
    }

    len = Password.GetLength();
    if(len >= 50) {
        len = 50;
    }
    *dpb = isc_dpb_password;
    dpb++;
    *dpb = len;
    dpb++;
    for(int i = 0; i < len; i++) {
        *dpb = Password[i];
        dpb++;
    }

    dpb_length = dpb - dpb_buffer;

// connect to database
    isc_attach_database(StatusVector, DatabaseName.GetLength(), DatabaseName, &DatabaseHandle, dpb_length, dpb_buffer);

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

    Logged = true;
    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdDatabase::Logout(void)
{
    if(Logged == false) {
        ES_ERROR("Cannot logout user who is not logged");
        return(false);
    }

    isc_detach_database(StatusVector, &DatabaseHandle);

    Logged = false;
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

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdDatabase::IsLogged(void)
{
    return(Logged);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

