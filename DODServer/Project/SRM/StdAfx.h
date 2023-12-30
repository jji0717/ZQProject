// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__FA3C028E_1619_49EA_8DB2_F06E705E9245__INCLUDED_)
#define AFX_STDAFX_H__FA3C028E_1619_49EA_8DB2_F06E705E9245__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
#include<Winsock2.h> 
#include<list>
#include"scsocket.h"
#include"clog.h"
#import "msxml3.dll"
#include <atlbase.h>


#define LOGFILENAME "C:\\LOG\\SRM.log"
#define LOGMAXSIZE  (64*1024*1024)     //64M
#define LOGMAXLEVEL 5

#define LOG_DEBUG   1

//////////////////////////////////////////////////////////

BOOL ParseFtpServerLocation(char* pServer,
					CString& strAddress, 
					CString& strUser, 
					CString& strPassword, 
					long* lPort, 
					CString& strPath);
//BMS, BMC, BML
//Format is Address:<address>;Port:<port>;Path:[path];
BOOL ParseBMSServerLocation(char* pServer,
					CString& strAddress, 
					long* lPort, 
					CString& strPath);

//FS Format is Address:<address>;
BOOL ParseFSLocation(char* pServer,
					 CString& strAddress);

// get SRM path.
BOOL GetControllerPath(CString& strController);

BOOL CheckUser();
// do with MOG license.
BOOL MOGRegistry();

BOOL IsAdministrator(char* username);

BOOL CheckSystem();


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__FA3C028E_1619_49EA_8DB2_F06E705E9245__INCLUDED_)
