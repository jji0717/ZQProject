// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__28FEC5B1_001A_4D86_B2A7_6A7D0BE8D730__INCLUDED_)
#define AFX_STDAFX_H__28FEC5B1_001A_4D86_B2A7_6A7D0BE8D730__INCLUDED_

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

#import "C:\Program Files\Common Files\MSSoap\Binaries\MSSOAP30.dll" raw_interfaces_only, raw_native_types, named_guids , exclude("IStream", "ISequentialStream", "_LARGE_INTEGER", "_ULARGE_INTEGER", "tagSTATSTG", "_FILETIME", "IErrorInfo") 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__28FEC5B1_001A_4D86_B2A7_6A7D0BE8D730__INCLUDED)
