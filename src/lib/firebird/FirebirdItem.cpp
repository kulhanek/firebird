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

#include <FirebirdItem.hpp>
#include <stdlib.h>
#include <string.h>
#include <ErrorSystem.hpp>
#include <FirebirdBLOB.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdItem::IsNULL(void)
{
    if(sqltype & 1) {
        if(sqlind == NULL) {
            return(false);
        }
        short data = *((short*)sqlind);
        if(data == -1) {
            return(true);
        }
    }
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CFirebirdItem::GetInt(void)
{
    if((sqltype&~1) == SQL_LONG) {
        int data = *((long*)sqldata);
        return(data);
    }
    return(0);
}

//------------------------------------------------------------------------------

bool CFirebirdItem::SetInt(int num, bool null)
{
    CSmallString sindex;

    sindex.IntToStr(num);
    return(SetString(sindex, null));

// // free previous data
// if( sqldata != NULL ) free(sqldata);
//
// // allocate new data
// sqllen = sizeof(long);
//
// if( null == true ){
//    sqltype = SQL_LONG+1;
//    if(sqlind == NULL) sqlind = (short*)malloc(sizeof(short));
//    if(sqlind == NULL) return(false);
//    *sqlind = -1;
//    }
//  else{
//    sqltype = SQL_LONG;
//    if( sqlind != NULL ){
//        free(sqlind);
//        sqlind = NULL;
//        }
//    }
// long* data = (long*)malloc(sqllen);
// data[0] = num;
// sqldata = (char ISC_FAR *)data;
// return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdItem::SetDouble(double num, bool null)
{
// free previous data
    if(sqldata != NULL) {
        free(sqldata);
    }

// allocate new data
    sqllen = sizeof(double);

    if(null == true) {
        sqltype = SQL_DOUBLE + 1;
        if(sqlind == NULL) {
            sqlind = (short*)malloc(sizeof(short));
        }
        if(sqlind == NULL) {
            return(false);
        }
        *sqlind = -1;
    } else {
        sqltype = SQL_DOUBLE;
        if(sqlind != NULL) {
            free(sqlind);
            sqlind = NULL;
        }
    }
    double* data = (double*)malloc(sqllen);
    data[0] = num;
    sqldata = (char ISC_FAR*)data;
    return(true);
}

//------------------------------------------------------------------------------

double  CFirebirdItem::GetDouble(void)
{
// get double precission
    if((sqltype&~1) == SQL_DOUBLE) {
        double data = *((double*)sqldata);
        return(data);
    }

// convert from integer
    if((sqltype&~1) == SQL_LONG) {
        return(GetInt());
    }

    return(0.0);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CFirebirdItem::GetString(void)
{
    CSmallString text;
    if((sqltype&~1) == SQL_TEXT) {
        text.SetLength(sqllen);
        memcpy(text.GetBuffer(), sqldata, sqllen);
        text.GetBuffer()[sqllen] = '\0';
    }
    if((sqltype&~1) == SQL_VARYING) {
        short len = *((short*)sqldata);
        if(len > 0) {
            text.SetLength(len);
            memcpy(text.GetBuffer(), &sqldata[2], len);
        }
    }

    return(text);
}

//------------------------------------------------------------------------------

const CSmallString CFirebirdItem::GetTruncatedString(void)
{
    CSmallString text;
    if((sqltype&~1) == SQL_TEXT) {
        int len = sqllen;
        while(len > 0) {
            if(sqldata[len - 1] != ' ') {
                break;
            }
            len--;
        }
        text.SetLength(len);
        if(len != 0) {
            memcpy(text.GetBuffer(), sqldata, len);
        }
    }
    if((sqltype&~1) == SQL_VARYING) {
        short len = *((short*)sqldata);
        if(len > 0) {
            while(len > 0) {
                if(sqldata[len + 1] != ' ') {
                    break;
                }
                len--;
            }
            text.SetLength(len);
            if(len != 0) {
                memcpy(text.GetBuffer(), &sqldata[2], len);
            }
        }
    }

    return(text);
}

//------------------------------------------------------------------------------

bool CFirebirdItem::SetString(const CSmallString& text, bool null)
{
// free previous data
    if(sqldata != NULL) {
        free(sqldata);
    }

// allocate new data
    sqllen = text.GetLength();
    if(null == true) {
        sqltype = SQL_TEXT + 1;
        if(sqlind == NULL) {
            sqlind = (short*)malloc(sizeof(short));
        }
        if(sqlind == NULL) {
            return(false);
        }
        *sqlind = -1;
    } else {
        sqltype = SQL_TEXT;
        if(sqlind != NULL) {
            free(sqlind);
            sqlind = NULL;
        }
    }
    if(sqllen != 0) {
        sqldata = (char*)malloc(sqllen);
        if(sqldata == NULL) {
            return(false);
        }
        strncpy(sqldata, text, sqllen);
    } else {
        sqllen = 1;
        sqldata = (char*)malloc(sizeof(char));
        if(sqldata == NULL) {
            return(false);
        }
        *((char*)sqldata) = ' ';
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdItem::GetBlobID(ISC_QUAD& blobid)
{
    memset(&blobid, 0, sizeof(ISC_QUAD));

    if((sqltype&~1) != SQL_BLOB) {
        return(false);
    }
    if(sqlind != NULL) {
        if(*sqlind == -1) {
            return(false);
        }
    }

    if(sqllen != sizeof(ISC_QUAD)) {
        return(false);
    }

    blobid = *((ISC_QUAD*)sqldata);

    return(true);
}

//------------------------------------------------------------------------------

bool CFirebirdItem::SetBlobID(ISC_QUAD blobid, bool null)
{
// free previous data
    if(sqldata != NULL) {
        free(sqldata);
    }

// allocate new data
    sqllen = sizeof(ISC_QUAD);
    if(null == true) {
        sqltype = SQL_BLOB + 1;
        if(sqlind == NULL) {
            sqlind = (short*)malloc(sizeof(short));
        }
        if(sqlind == NULL) {
            return(false);
        }
        *sqlind = -1;
    } else {
        sqltype = SQL_BLOB;
        if(sqlind != NULL) {
            free(sqlind);
            sqlind = NULL;
        }
    }
    sqldata = (char*)malloc(sizeof(ISC_QUAD));
    if(sqldata == NULL) {
        return(false);
    }
    memcpy(sqldata, &blobid, sizeof(ISC_QUAD));
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallTimeAndDate CFirebirdItem::GetTimeAndDate(void)
{
    CSmallTimeAndDate zero_dt;

    if((sqltype&~1) != SQL_TIMESTAMP) {
        ES_ERROR("item is not TIMESTAMP type");
        return(zero_dt);
    }

    if(sqldata == NULL) {
        return(zero_dt);
    }
    if(sqlind != NULL) {
        if(*sqlind == -1) {
            return(zero_dt);
        }
    }

    struct tm  tm_time;

    isc_decode_timestamp((ISC_TIMESTAMP ISC_FAR*)sqldata, &tm_time);

    CSmallTimeAndDate time(tm_time);

    return(time);
}

//------------------------------------------------------------------------------

bool CFirebirdItem::SetTimeAndDate(const CSmallTimeAndDate& timeanddate, bool null)
{
    return(SetString(timeanddate.GetSDateAndTime(), null));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallTime CFirebirdItem::GetTime(void)
{
    CSmallTime zero_time;

    if((sqltype&~1) != SQL_TYPE_TIME) {
        ES_ERROR("item is not TYPE_TIME type");
        return(zero_time);
    }

    if(sqldata == NULL) {
        return(zero_time);
    }
    if(sqlind != NULL) {
        if(*sqlind == -1) {
            return(zero_time);
        }
    }

    struct tm  tm_time;

    isc_decode_sql_time((ISC_TIME ISC_FAR*)sqldata, &tm_time);

    time_t time_i = tm_time.tm_sec + tm_time.tm_min * 60 + tm_time.tm_hour * 3600;

    CSmallTime time(time_i);

    return(time);
}

//------------------------------------------------------------------------------

bool CFirebirdItem::SetTime(const CSmallTime& time, bool null)
{
    return(SetString(time.GetSTime(), null));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallDate CFirebirdItem::GetDate(void)
{
    CSmallDate zero_date;

    if((sqltype&~1) != SQL_TYPE_DATE) {
        ES_ERROR("item is not TYPE_DATE type");
        return(zero_date);
    }

    if(sqldata == NULL) {
        return(zero_date);
    }
    if(sqlind != NULL) {
        if(*sqlind == -1) {
            return(zero_date);
        }
    }

    struct tm  tm_time;

    isc_decode_sql_date((ISC_DATE ISC_FAR*)sqldata, &tm_time);

    CSmallTimeAndDate time(tm_time);

    return(time); // only date will be extracted
}

//------------------------------------------------------------------------------

bool CFirebirdItem::SetDate(const CSmallDate& date, bool null)
{
    return(SetString(date.GetSDate(), null));
}

//------------------------------------------------------------------------------

bool CFirebirdItem::AllocateItemData(void)
{
    short            dtype;

    dtype = (sqltype&~1);

    if(sqldata != NULL) {
        free(sqldata);
        sqldata = NULL;
    }

    if(sqlind != NULL) {
        free(sqlind);
        sqlind = NULL;
    }

    switch(dtype) {
        case SQL_VARYING:
            sqldata = (char*)malloc(sizeof(char) * sqllen + 2);
            break;
        case SQL_TEXT:
            sqldata = (char*)malloc(sizeof(char) * sqllen);
            break;
        case SQL_LONG:
            sqldata = (char*)malloc(sizeof(long));
            break;
        case SQL_DOUBLE:
            sqldata = (char*)malloc(sizeof(double));
            break;
        case SQL_TIMESTAMP:
            sqldata = (char*)malloc(sizeof(char) * sqllen);
            break;
        case SQL_TYPE_TIME:
            sqldata = (char*)malloc(sizeof(char) * sqllen);
            break;
        case SQL_BLOB:
            sqldata = (char*)malloc(sizeof(char) * sqllen);
            break;
        default:
            sqldata = NULL;
            break;
    }

    if(sqltype & 1) {
        sqlind = (short*)malloc(sizeof(short));
    } else {
        sqlind = NULL;
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdItem::SetXMLRecord(CFirebirdTransaction* p_tran,
                                 CXMLNode* p_ele,
                                 bool binary)
{
    ISC_QUAD blob_id;

    if(p_ele == NULL) {
        return(SetBlobID(blob_id, true));
    }

    if(p_tran == NULL) {
        ES_ERROR("Transaction is not specified!");
        return(false);
    }

    CFirebirdBlob blob;

    blob.AssignToTransaction(p_tran);

    if(blob.CreateBlob(blob_id) == false) {
        ES_ERROR("unable to create BLOB!");
        return(false);
    }

    if(blob.WriteXMLData(p_ele, binary) == false) {
        ES_ERROR("unable to WriteXMLData to BLOB!");
        blob.CloseBlob();
        return(false);
    }

    blob.CloseBlob();

    return(SetBlobID(blob_id));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CFirebirdItem::GetXMLRecord(CFirebirdTransaction* p_tran, CXMLNode* p_ele)
{
    if(IsNULL() == true) {
        return(true);
    }

    ISC_QUAD blob_id;

    if(GetBlobID(blob_id) == false) {
        ES_ERROR("Unable to obtain BLOB id!");
        return(false);
    }

    if(IsNULL() == true) {
        return(true); // no data
    }

    if(p_tran == NULL) {
        ES_ERROR("Transaction is not specified!");
        return(false);
    }

    if(p_ele == NULL) {
        ES_ERROR("Element is not specified!");
        return(false);
    }

    CFirebirdBlob blob;

    blob.AssignToTransaction(p_tran);

    if(blob.OpenBlob(blob_id) == false) {
        ES_ERROR("Unable to open BLOB!");
        return(false);
    }

    if(blob.ReadXMLData(p_ele) == false) {
        ES_ERROR("Unable to ReadXMLData to BLOB!");
        blob.CloseBlob();
        return(false);
    }

    blob.CloseBlob();

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



