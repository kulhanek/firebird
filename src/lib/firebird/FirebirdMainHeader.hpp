#ifndef FirebirdMainHeaderH
#define FirebirdMainHeaderH
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

/* currently only UNIX and WIN32 architectures are supported */

#ifdef UNIX // setup for WIN32 architecture
    #define FIREBIRD_PACKAGE
    #define __fbspec
    #define FIREBIRD_UNIX
#else // setup for WIN32 architecture
    #ifdef FIREBIRD_COMPILE_DLL
        #define FIREBIRD_PACKAGE _export
        #define EXPORT
    #else
        #define FIREBIRD_PACKAGE _import
        #define IMPORT
    #endif
    #define FIREBIRD_WIN32
    #define __fastcall
#endif

//------------------------------------------------------------------------------

#if ! (defined WIN32 ||  defined UNIX)
    #error "Firebird: Unsupported system!"
#endif

//------------------------------------------------------------------------------

#define FB_ERROR_BUFFER_LEN 512

//------------------------------------------------------------------------------

#endif
