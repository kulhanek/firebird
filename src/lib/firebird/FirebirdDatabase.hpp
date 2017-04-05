#ifndef FirebirdDatabase_H
#define FirebirdDatabase_H
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

#include <ibase.h>
#include <FirebirdMainHeader.hpp>
#include <SmallString.hpp>

//------------------------------------------------------------------------------

//! firebird database access class

class FIREBIRD_PACKAGE CFirebirdDatabase{
    public:
        CFirebirdDatabase(void);
        ~CFirebirdDatabase(void);

    //! set name and location of database
    void SetDatabaseName(const CSmallString& name);

    //! get name and location of database
    const CSmallString& GetDatabaseName(void);

    //! log user to database
    bool Login(const CSmallString& login,const CSmallString& password);

    //! logout user from database
    bool Logout(void);

    //! check if user is logged
    bool IsLogged(void);

 // section of private data ----------------------------------------------------
    private:
    CSmallString    DatabaseName;
    CSmallString    LoginName;
    CSmallString    Password;
    bool            Logged;
    isc_db_handle   DatabaseHandle;
    ISC_STATUS      StatusVector[20];

    friend class CFirebirdTransaction;
    friend class CFirebirdQuerySQL;
    friend class CFirebirdExecuteSQL;
    friend class CFirebirdBlob;
    };

//------------------------------------------------------------------------------

#endif
