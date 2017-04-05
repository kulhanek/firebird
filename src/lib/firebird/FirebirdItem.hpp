#ifndef FirebirdItem_H
#define FirebirdItem_H
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
#include <SmallTime.hpp>
#include <SmallDate.hpp>
#include <SmallTimeAndDate.hpp>

//------------------------------------------------------------------------------

class CFirebirdTransaction;
class CXMLNode;

//------------------------------------------------------------------------------

//! firebird data item

class FIREBIRD_PACKAGE CFirebirdItem : public XSQLVAR{
    public:

    //! check if item contain valid data
    bool IsNULL(void);

    //! set string
    bool SetString(const CSmallString& text,bool null=false);
    //! get string
    const CSmallString GetString(void);
    //! get truncated string, e.g. without end spaces
    const CSmallString GetTruncatedString(void);

    //! set integer
    bool SetInt(int num,bool null=false);
    //! get integer
    int  GetInt(void);

    //! set double
    bool SetDouble(double num,bool null=false);
    //! get double
    double  GetDouble(void);

    //! assign BLOB id
    bool SetBlobID(ISC_QUAD blobid,bool null=false);
    //! get BLOB id
    bool GetBlobID(ISC_QUAD& blobid);

    //! set time and date
    bool SetTimeAndDate(const CSmallTimeAndDate& timeanddate,bool null=false);
    //! get time and date
    const CSmallTimeAndDate GetTimeAndDate(void);

    //! set time
    bool SetTime(const CSmallTime& time,bool null=false);
    //! get time
    const CSmallTime GetTime(void);

    //! set date
    bool SetDate(const CSmallDate& date,bool null=false);
    //! get date
    const CSmallDate GetDate(void);

    // special operation for XML file
    bool SetXMLRecord(CFirebirdTransaction* p_tran,
                               CXMLNode* p_ele,
                               bool binary=false);
    bool GetXMLRecord(CFirebirdTransaction* p_tran,
                               CXMLNode* p_ele);

// section of private data ----------------------------------------------------
    private:
    bool AllocateItemData(void);

    friend class CFirebirdQuerySQL;
    };

//------------------------------------------------------------------------------

#endif
