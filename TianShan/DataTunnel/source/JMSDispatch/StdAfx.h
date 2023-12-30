// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E096AA12_D93B_46A4_8A6C_74EB784AFD65__INCLUDED_)
#define AFX_STDAFX_H__E096AA12_D93B_46A4_8A6C_74EB784AFD65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x400
#endif


#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#import "msxml3.dll"

#include <string>
#include<list>
#include<vector>
#include <map>
using namespace std;
#include <atlbase.h>
#include <tchar.h>
#include <stdio.h>
#include "Dispatch.h"
#include "log.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#define ZQDataApp TianShanIce::Application::DataOnDemand 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#define DCADERRORCODE	0X30010
#define DODJMSINITIATIONERROR			DCADERRORCODE+1	// JMS 's initiation value is negative
#define DODRECEIVERDATAFOLDERERROR		DCADERRORCODE+2	// receiveddatafold, xml message error!
#define DODRECEIVERDATATCPERROR			DCADERRORCODE+3	// receiveddataTCP, xml message error!
#define DODRECEIVERPORTCONFIGMSGERROR	DCADERRORCODE+4	// receiveddataconfig,xml message error!
#define DODReceiverUpDataTCPRROR		DCADERRORCODE+5	// receiveupdateTCP, xml message error!
#define DODReceiverUpDataTCPERROR		DCADERRORCODE+6	// receiveupdatefold, xml message error! 
#define DODSendConfigRequestERROR		DCADERRORCODE+7	// sendconfigurationrequest, the operation is not respondence! 
#define DODsendgetfulldataERROR			DCADERRORCODE+8	// sendgetfull, the operation is not respondence! 
#define DODcreatetempdirERROR			DCADERRORCODE+9	// when receive configuration message, create tempdirectory error
#define DODcreatetempfileERROR			DCADERRORCODE+10// when receive configuration message, create tempfile error! 


// DOD send message_code

#define PORTCONFIGURATIONREQUEST	3001	//Port Configuration request
#define GETFULLDATA					3002	//GetFullData ,request send data

// DOD received message_code

#define PORTCONFIGURATION			3003	//Port Configuration
#define FULLDATAWITHSHAREDFOLDERS	3004	//FullData with shared Folders
#define FULLDATAWITHTCP				3005	//FullData with TCP
#define UPDATEINSHAREFOLDERMODE		3006	//Channel Data Update in Share-Folder mode
#define UPDATEINTCPMODE				3007	//Channel Data Update in TCP mode
#define FIRSTDATATYPEMESSAGE		3008	//FirstDataTypeMsg with DCA service start 
#define RETUENMESSAGEABOUT3008		3009	//Return message about MSG_3008 


// DOD send message_type
#define PORTCONFIGURATIONREQUEST   3001

#define REALPORTCONFIGURATIONREQUEST   1515

//void writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...);
void  GetErrorDescription(int nErrorcode, char *StrError);
#endif // !defined(AFX_STDAFX_H__00ADBB74_94A9_4C7A_B98B_8AFA0FE6AA1B__INCLUDED_)


