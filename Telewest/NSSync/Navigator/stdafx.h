// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxstr.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//////////////////////////////////////////////////////////////////////////
// if local AM database uses multi-language definition
// and define string as NVARCHAR instead of VARCHAR
// please enable MULTI_LANG_DB macro
//////////////////////////////////////////////////////////////////////////

//#define MULTI_LANG_DB

#ifdef MULTI_LANG_DB

#	define CStringX CStringW
#	define CX2T		CW2T

#else // MULTI_LANG_DB

#	define CStringX CStringA
#	define CX2T		CA2T

#endif // MULTI_LANG_DB

// TODO: reference additional headers your program requires here
