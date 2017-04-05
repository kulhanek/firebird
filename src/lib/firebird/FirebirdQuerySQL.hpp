#ifndef FirebirdQuerySQL_H
#define FirebirdQuerySQL_H
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

#include <FirebirdMainHeader.hpp>
#include <ibase.h>
#include <SmallString.hpp>

//------------------------------------------------------------------------------

class CFirebirdTransaction;
class CFirebirdItem;

//------------------------------------------------------------------------------

//! firebird SQL query statement
/*!
How to use CFirebirdQuerySQL
  1. allocate input items if necessary
  2. fill input items if necessary
  3. open query
  4. while ( query record ){
  5.    get item data
  6     }
  5. close query

  output items are allocated automatically
*/

class FIREBIRD_PACKAGE CFirebirdQuerySQL{
    public:
         CFirebirdQuerySQL(void);
         ~CFirebirdQuerySQL(void);

    //! assign Query object to transaction
    bool AssignToTransaction(CFirebirdTransaction* p_transaction);

    //! prepare query
    bool PrepareQuery(const CSmallString& command);

    //! execute query
    bool ExecuteQuery(void);

    //! execute query  - execute + query
    bool ExecuteQueryOnce(void);

    //! start query = prepare + execute
    bool OpenQuery(const CSmallString& command);

    //! get record
    bool QueryRecord(void);

    //! close qury
    bool CloseQuery(void);

    //! get input item
    CFirebirdItem* GetInputItem(int index);

    //! get output item
    CFirebirdItem* GetOutputItem(int index);

// section of private data ----------------------------------------------------
    private:
    CFirebirdTransaction*   Transaction;
    isc_stmt_handle         stmt;
    ISC_STATUS              StatusVector[20];

    XSQLDA    ISC_FAR*      insqlda;
    XSQLDA    ISC_FAR*      outsqlda;

    //! prealocate output items
    bool AllocateOutputItems(int numofitems);
    //! allocate data for items
    bool AllocateOutputItemData(void);
    //! free aoutput items
    void FreeOutputItems(void);

    //! prealocate input items
    bool AllocateInputItems(int numofitems);
    //! allocate data for items
    bool AllocateInputItemData(void);
    //! free input items
    void FreeInputItems(void);
};

//------------------------------------------------------------------------------

#endif

