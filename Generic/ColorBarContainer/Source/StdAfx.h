// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__65CFD857_A9E6_4631_990D_C679C9CCC101__INCLUDED_)
#define AFX_STDAFX_H__65CFD857_A9E6_4631_990D_C679C9CCC101__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include "ZQ_common_conf.h"
#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;

#include <atlbase.h>
#include <atlapp.h>

#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlmisc.h>



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__65CFD857_A9E6_4631_990D_C679C9CCC101__INCLUDED)
