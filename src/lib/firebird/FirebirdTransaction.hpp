#ifndef FirebirdTransaction_H
#define FirebirdTransaction_H
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

class CFirebirdDatabase;

//------------------------------------------------------------------------------

//! firebird transaction class

class FIREBIRD_PACKAGE CFirebirdTransaction{
    public:
         CFirebirdTransaction(void);
         ~CFirebirdTransaction(void);

    //! assign transaction to database
    bool AssignToDatabase(CFirebirdDatabase* p_database);

    //! start transaction
    bool StartTransaction(void);

    //! commit all operation
    bool CommitTransaction(bool retaining=false);

    //! discard all operation
    bool RollbackTransaction(void);

    //! check if transaction is active
    bool IsActive(void);

// section of private data ----------------------------------------------------
    private:
    CFirebirdDatabase*  Database;
    isc_tr_handle       TransactionHandle;
    ISC_STATUS          StatusVector[20];
    bool                Active;

    friend class CFirebirdQuerySQL;
    friend class CFirebirdExecuteSQL;
    friend class CFirebirdBlob;
};

//------------------------------------------------------------------------------

#endif
