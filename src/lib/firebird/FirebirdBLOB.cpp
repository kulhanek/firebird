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

#include <FirebirdBLOB.hpp>
#include <stdio.h>
#include <string.h>
#include <FirebirdTransaction.hpp>
#include <ErrorSystem.hpp>
#include <FirebirdDatabase.hpp>
#include <XMLNode.hpp>
#include <XMLPrinter.hpp>
#include <XMLParser.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CFirebirdBlob::CFirebirdBlob(void)
{
    Transaction = NULL;
    BlobHandle = 0;
    memset(&BlobID, 0, sizeof(BlobID));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdBlob::AssignToTransaction(CFirebirdTransaction* p_transaction)
{
    if(BlobHandle != 0) {
        ES_ERROR("Cannot change transaction if BLOB object is opened !");
        return(false);
    }
    Transaction = p_transaction;
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdBlob::CreateBlob(ISC_QUAD& blobid)
{
    if(BlobHandle != 0) {
        ES_ERROR("BLOB object is already created/opened");
        return(false);
    }

    if(Transaction == NULL) {
        ES_ERROR("transaction is not assigned");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("transaction is not active");
        return(false);
    }

    if(isc_create_blob(StatusVector, &Transaction->Database->DatabaseHandle, &Transaction->TransactionHandle, &BlobHandle, &blobid) == 0) {
        BlobID = blobid;
        return(true);
    }

    char msg[FB_ERROR_BUFFER_LEN];
    memset(msg, 0, FB_ERROR_BUFFER_LEN);
    const ISC_STATUS* p_status = &StatusVector[0];
    while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
        ES_ERROR(msg);
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CFirebirdBlob::OpenBlob(ISC_QUAD& blobid)
{
    if(BlobHandle != 0) {
        ES_ERROR("BLOB object is already created/opened");
        return(false);
    }

    if(Transaction == NULL) {
        ES_ERROR("ransaction is not assigned");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("ransaction is not active");
        return(false);
    }

    if(isc_open_blob(StatusVector, &Transaction->Database->DatabaseHandle, &Transaction->TransactionHandle, &BlobHandle, &blobid) == 0) {
        BlobID = blobid;
        return(true);
    }

    char msg[FB_ERROR_BUFFER_LEN];
    memset(msg, 0, FB_ERROR_BUFFER_LEN);
    const ISC_STATUS* p_status = &StatusVector[0];
    while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
        ES_ERROR(msg);
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CFirebirdBlob::CloseBlob(void)
{
    if(BlobHandle == 0) {
        ES_ERROR("BLOB object is not created/opened");
        return(false);
    }

    if(Transaction == NULL) {
        ES_ERROR("transaction is not assigned");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("transaction is not active");
        return(false);
    }

    if(isc_close_blob(StatusVector, &BlobHandle) == 0) {
        BlobHandle = 0;
        memset(&BlobID, 0, sizeof(BlobID));
        return(true);
    }

    char msg[FB_ERROR_BUFFER_LEN];
    memset(msg, 0, FB_ERROR_BUFFER_LEN);
    const ISC_STATUS* p_status = &StatusVector[0];
    while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
        ES_ERROR(msg);
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdBlob::WriteData(void* buffer, unsigned int len)
{
    if(BlobHandle == 0) {
        ES_ERROR("BLOB object is not created/opened");
        return(false);
    }

    if(Transaction == NULL) {
        ES_ERROR("ransaction is not assigned");
        return(false);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("ransaction is not active");
        return(false);
    }

    unsigned int remaininglen = len;
    unsigned short segment_len;
    char*          data = (char*)buffer;

    while(remaininglen > 0) {
        if(remaininglen > 65535) {
            segment_len = 65535;
        } else {
            segment_len = remaininglen;
        }
        if(isc_put_segment(StatusVector, &BlobHandle, segment_len, data)) {
            char msg[FB_ERROR_BUFFER_LEN];
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
            const ISC_STATUS* p_status = &StatusVector[0];
            while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
                ES_ERROR(msg);
                memset(msg, 0, FB_ERROR_BUFFER_LEN);
            }
            return(false);
        }
        data += segment_len;
        remaininglen -= segment_len;
    }

    return(true);
}

//------------------------------------------------------------------------------

unsigned int CFirebirdBlob::ReadData(void* buffer, unsigned int len)
{
    if(BlobHandle == 0) {
        ES_ERROR("BLOB object is not created/opened");
        return(0);
    }

    if(Transaction == NULL) {
        ES_ERROR("transaction is not assigned");
        return(0);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("transaction is not active");
        return(0);
    }

    unsigned int remaininglen = len;
    unsigned int read_data_len = 0;
    unsigned short segment_len;
    unsigned short segment_buffer_len;
    char*          data = (char*)buffer;

    while(remaininglen > 0) {
        if(remaininglen > 65535) {
            segment_buffer_len = 65535;
        } else {
            segment_buffer_len = remaininglen;
        }
        if(isc_get_segment(StatusVector, &BlobHandle, &segment_len, segment_buffer_len, data) || StatusVector[1] == isc_segment) {
            char msg[FB_ERROR_BUFFER_LEN];
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
            const ISC_STATUS* p_status = &StatusVector[0];
            while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
                ES_ERROR(msg);
                memset(msg, 0, FB_ERROR_BUFFER_LEN);
            }

            return(read_data_len);
        }
        data += segment_len;
        remaininglen -= segment_len;
        read_data_len += segment_len;
    }

    return(read_data_len);
}

//------------------------------------------------------------------------------

int CFirebirdBlob::GetBlobLength(void)
{
    if(BlobHandle == 0) {
        ES_ERROR("BLOB object is not created/opened");
        return(-1);
    }

    if(Transaction == NULL) {
        ES_ERROR("transaction is not assigned");
        return(-1);
    }

    if(Transaction->IsActive() == false) {
        ES_ERROR("transaction is not active");
        return(-1);
    }

    char blob_items[] = {isc_info_blob_total_length};
    char res_buffer[20];

    if(isc_blob_info(StatusVector, &BlobHandle, sizeof(blob_items), blob_items, sizeof(res_buffer), res_buffer)) {
        char msg[FB_ERROR_BUFFER_LEN];
        memset(msg, 0, FB_ERROR_BUFFER_LEN);
        const ISC_STATUS* p_status = &StatusVector[0];
        while(fb_interpret(msg, FB_ERROR_BUFFER_LEN - 1, &p_status)) {
            ES_ERROR(msg);
            memset(msg, 0, FB_ERROR_BUFFER_LEN);
        }
        return(-1);
    }

    char*  p;
    short  length;
    char   item;
    int    total_len = 0;

    for(p = res_buffer; *p != isc_info_end;) {
        item = *p++;
        length = (short)isc_vax_integer(p, 2);
        p += 2;
        switch(item) {
            case isc_info_blob_total_length:
                total_len = isc_vax_integer(p, length);
                break;
        }
        p += length;
    }

    return(total_len);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdBlob::ReadXMLData(CXMLNode* p_ele)
{
    if(p_ele == NULL) {
        ES_ERROR("XML element is NULL");
        return(false);
    }

    char*   p_data = NULL;
    unsigned int     length = 0;

    length = GetBlobLength();
    if(length == 0) {
        ES_ERROR("BLOB is empty");
        return(false);
    }

    p_data = new char[length];
    if(ReadData(p_data, length) != length) {
        delete[] p_data;
        ES_ERROR("unable to read BLOB data");
        return(false);
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(p_ele);

    if(xml_parser.Parse(p_data, length) == false) {
        delete[] p_data;
        ES_ERROR("unable to decode XML data");
        return(false);
    }

    delete[] p_data;

    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdBlob::WriteXMLData(CXMLNode* p_ele, bool binary)
{
    if(p_ele == NULL) {
        ES_ERROR("XML element is NULL");
        return(false);
    }

    CXMLPrinter xml_printer;

    xml_printer.SetPrintedXMLNode(p_ele);
    if(binary == true) {
        xml_printer.SetOutputFormat(EXF_BXML);
    } else {
        xml_printer.SetOutputFormat(EXF_TEXT);
    }

    unsigned char* p_data;
    unsigned int length;

    if((p_data = xml_printer.Print(length)) == NULL) {
        ES_ERROR("no data in XML node?");
        return(false);
    }

    if(WriteData(p_data, length) == false) {
        delete[] p_data;
        ES_ERROR("unable to save data block");
        return(false);
    }

    delete[] p_data;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

