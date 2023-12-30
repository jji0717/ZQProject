// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__83153DA8_DF89_4ED7_B8EF_C2B64B9EF00B__INCLUDED_)
#define AFX_STDAFX_H__83153DA8_DF89_4ED7_B8EF_C2B64B9EF00B__INCLUDED_

#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include  <io.h>

#endif

#include  <stdio.h>
#include  <stdlib.h>

// TODO: reference additional headers your program requires here#include "ZQ_common_Conf.h"


#include <Exception.h>
#include "Locks.h"
//#include <string>
//#include <vector>

#include "IMODHelperObj.h"
#include "../MOD2/MODDefines.h"

#define ZQAPPMOD ZQTianShan::Application::MOD
#define		_TRACE		logTrace
inline void logTrace(const char* lpszFormat, ...)  PRINTFLIKE(1, 2);

#define AuthorFmt(_C, _X) CLOGFMT(_C, "[%s][%s] " _X), authorInfo.serverSessionId.c_str(), authorInfo.ident.name.c_str()

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__83153DA8_DF89_4ED7_B8EF_C2B64B9EF00B__INCLUDED_)
