// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Name  : StdAfx.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-12
// Desc  : precompiled header
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTSOAPUDTMAPPER/StdAfx.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 5     05-04-20 19:03 Bernie.zhao
// 
// 4     05-04-20 14:45 Bernie.zhao
// 
// 3     05-03-31 15:53 Kaliven.lee
// 
// 2     05-03-29 15:31 Kaliven.lee
// 
// 1     05-02-16 11:56 Bernie.zhao
// new mapper
// 
// 1     05-01-14 11:05 Bernie.zhao
// ===========================================================================
// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__976F0F8F_01A3_43F4_ADF3_2C455CC11C07__INCLUDED_)
#define AFX_STDAFX_H__976F0F8F_01A3_43F4_ADF3_2C455CC11C07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

#import "msxml4.dll" raw_interfaces_only, raw_native_types
#import "MSSOAP30.dll" raw_interfaces_only, raw_native_types, named_guids , exclude("IStream", "ISequentialStream", "_LARGE_INTEGER", "_ULARGE_INTEGER", "tagSTATSTG", "_FILETIME", "IErrorInfo") 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_STDAFX_H__976F0F8F_01A3_43F4_ADF3_2C455CC11C07__INCLUDED)
