#ifndef FirebirdBLOBH
#define FirebirdBLOBH
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

//------------------------------------------------------------------------------

class CFirebirdTransaction;
class CXMLNode;

//------------------------------------------------------------------------------

//! firebird BLOB support

class FIREBIRD_PACKAGE CFirebirdBlob{
    public:
        CFirebirdBlob(void);

    //! assign BLOB object to transaction
    bool AssignToTransaction(CFirebirdTransaction* p_transaction);

    //! create new BLOB object
    bool CreateBlob(ISC_QUAD& blobid);

    //! open existing BLOB object
    bool OpenBlob(ISC_QUAD& blobid);

    //! close BLOB object
    bool CloseBlob(void);

    //! read XML data - automatically detect TXML/BXML
    bool ReadXMLData(CXMLNode* p_ele);

    //! write XML data
    bool WriteXMLData(CXMLNode* p_ele,bool binary);

    //! write data to BLOB object
    bool          WriteData(void* buffer,unsigned int len);

    //! read data from BLOB object
    /*! return number of read data
    */
    unsigned int  ReadData(void* buffer,unsigned int len);

    //! get data length of BLOB object
    int GetBlobLength(void);

// section of private data ----------------------------------------------------
    private:
    CFirebirdTransaction*   Transaction;
    isc_blob_handle         BlobHandle;
    ISC_QUAD                BlobID;
    ISC_STATUS              StatusVector[20];
    };

//------------------------------------------------------------------------------

#endif
