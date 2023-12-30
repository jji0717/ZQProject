// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

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
#define _WIN32_WINNT 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define LOG_NORECORD 0
#define LOG_ERROR	 1
#define LOG_RELEASE	 2
#define LOG_DEBUG	 3


#define LOGMAXSIZE  (64*1024*1024)     //64M
#define LOGMAXLEVEL 5


#define DODMESSAGEFLAG			_T("DODMessage")
#define DODMESSAGEHEADER		_T("DODMessageHeader")
#define DODMESSAGECODE			_T("MessageCode")
#define DODMESSAGETIME			_T("Time")
#define DODMESSAGERETURN		_T("BeReturn")
#define DODMESSAGEBODY			_T("DODMessageBody")

// For message content
#define DODMESSAGECONTENT		_T("Message")
#define MESSAGEDATATYPE			_T("DataType")
#define MESSAGEDODSTARTUP		_T("DODStartup")
#define MESSAGEGROUPID			_T("GroupID")
#define MESSAGEMESSAGEID		_T("MessageID")