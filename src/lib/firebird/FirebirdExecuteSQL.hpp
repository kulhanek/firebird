#ifndef FirebirdExecuteSQL_H
#define FirebirdExecuteSQL_H
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
#include <SmallString.hpp>
#include <FirebirdMainHeader.hpp>
#include <SmallTime.hpp>

//------------------------------------------------------------------------------

class CFirebirdTransaction;
class CFirebirdItem;

//------------------------------------------------------------------------------

//! firebird execute SQL statement
/*!
How to use CFirebirdExecuteSQL:
 1. allocate items
 2. fill items with data
 3. execute SQL stattement
*/

class FIREBIRD_PACKAGE CFirebirdExecuteSQL{
    public:
         CFirebirdExecuteSQL(void);
         ~CFirebirdExecuteSQL(void);

    //! assign statement object to transaction
    bool AssignToTransaction(CFirebirdTransaction* p_transaction);

    //! prealocate items
    /*! these items are not changed with AssignToTransaction or ExecuteSQL
    */
    bool AllocateInputItems(int numofitems);

    //! get acces to item
    CFirebirdItem* GetInputItem(int index);

    //! free allocated items
    void FreeInputItems(void);

    //! execute SQL statement
    bool ExecuteSQL(const CSmallString& command);

// section of private data ----------------------------------------------------
    private:
    CFirebirdTransaction*   Transaction;
    XSQLDA    ISC_FAR*      sqlda;
    ISC_STATUS              StatusVector[20];
    };

//------------------------------------------------------------------------------

#endif
