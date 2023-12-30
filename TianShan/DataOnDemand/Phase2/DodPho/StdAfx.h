// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__56CDDDBF_BFC1_4BBE_8A8B_74DF973AE61C__INCLUDED_)
#define AFX_STDAFX_H__56CDDDBF_BFC1_4BBE_8A8B_74DF973AE61C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

/*
#ifdef _DEBUG
#include <atlbase.h>
#define		_TRACE		AtlTrace
#else
#include <stdio.h>
inline void _cdecl _Trace(LPCSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	char szBuffer[512];

	nBuf = vsprintf(szBuffer, lpszFormat, args);
	OutputDebugStringA(szBuffer);
	va_end(args);
}

#define		_TRACE		_Trace
#endif
*/

#define		_TRACE		logTrace
inline void _cdecl logTrace(LPCSTR lpszFormat, ...);

#include "ZQ_common_Conf.h"
#include <Exception.h>
#include "Locks.h"
#include "IPathHelperObj.h"

#define ZQTsAcPath		::ZQTianShan::AccreditedPath
#define TsTrans			::TianShanIce::Transport
#define TsSrm			::TianShanIce::SRM
#define ZQLIB			::ZQ::common

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__56CDDDBF_BFC1_4BBE_8A8B_74DF973AE61C__INCLUDED_)
