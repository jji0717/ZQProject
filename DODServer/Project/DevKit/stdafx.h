// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
/*
Revision History:

Rev			   Date        Who			Description
----		---------    ------		----------------------------------
V1,0,0,3	2006.07.26	zhenan_ji 	change port number limited about portinfo struct to list; 
V1,0,0,4	2006.08.23	zhenan_ji 	1 :Add parameter(IssettingIP) to setSrmAddress; 
									Fixed bug: if srm is not run,DCA will repeat setting SRM's IP ,this is error mechanism.
									2: remove some invalidate processing : E.g (getLocalIP ,getHostname,getpeername )	
V1,0,0,5	2006.09.07	zhenan_ji 	1 :Add CheckRead and CheckWrite in CSCSocket;
									   CheckRead:: if no data will be received, please don't call socket->recv ...
									   CheckWrite:: if current socket is not accept message ,don't call socket->Accept()...
									2: Replace all TerminateThread() by WaitForSingleObject() and CloseHandle();



*/
#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400	// Change this to the appropriate value to target Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit



#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>		// MFC socket extensions


#include <iostream>
#include<Winsock2.h> 
#include<list>
#include"scsocket.h"
#include "clog.h"
#import "msxml3.dll"
#include <atlbase.h>

#define LOG_DEBUG   0

#define LOGFILENAME "C:\\DCAKit.log"
#define LOGMAXSIZE  (64*1024*1024)     //64M
#define LOGMAXLEVEL 5




