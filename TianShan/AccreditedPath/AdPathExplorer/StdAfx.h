// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__FDC70246_FE53_4E1A_A57D_B50E82EFA3EA__INCLUDED_)
#define AFX_STDAFX_H__FDC70246_FE53_4E1A_A57D_B50E82EFA3EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include "InetAddr.h"
#include <atlbase.h>
#include "ADPIceImpl.h"
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
#include "commctrl.h"
#include "defition.h"

extern "C" 
BOOL GetPrivateDataStr(char *dest,::TianShanIce::Variant *pvar);
extern "C"
BOOL CheckPrivateData(const std::string strKey,INT type, char *psrc,::TianShanIce::Variant *pvar);
extern "C"
char * Delblank(char *psrc);
extern "C"
BOOL DoubleChar(char *psrc);
extern "C"
BOOL IsInt(char *psrc);
extern "C"
BOOL IsString(char ch, char *psrc);
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__FDC70246_FE53_4E1A_A57D_B50E82EFA3EA__INCLUDED)
