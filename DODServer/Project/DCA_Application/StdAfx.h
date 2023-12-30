
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 项目特定的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 标头中排除不常使用的资料
#endif

// 如果您必须使用下列所指定的平台之前的平台，则修改下面的定义。
// 有关不同平台的相应值的最新信息，请参考 MSDN。
#ifndef WINVER				// 允许使用 Windows 95 和 Windows NT 4 或更高版本的特定功能。
#define WINVER 0x0400		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif

#ifndef _WIN32_WINNT		// 允许使用 Windows NT 4 或更高版本的特定功能。
#define _WIN32_WINNT 0x0400		//为 Windows98 和 Windows 2000 及更新版本改变为适当的值。
#endif						

#ifndef _WIN32_WINDOWS		// 允许使用 Windows 98 或更高版本的特定功能。
#define _WIN32_WINDOWS 0x0410 //为 Windows Me 及更新版本改变为适当的值。
#endif

#ifndef _WIN32_IE			// 允许使用 IE 4.0 或更高版本的特定功能。
#define _WIN32_IE 0x0400	//为 IE 5.0 及更新版本改变为适当的值。
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常被安全忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心和标准组件
#include <afxext.h>         // MFC 扩展
#include <afxdisp.h>        // MFC 自动化类

//#include <afxwin.h>

#include <afxdtctl.h>		// Internet Explorer 4 公共控件的 MFC 支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// Windows 公共控件的 MFC 支持

//import activeJMS component
#import "C:\Program Files\Active JMS\bin\active-jms.tlb"

#define LOG_NORECORD 0
#define LOG_ERROR	 1
#define LOG_RELEASE	 2
#define LOG_DEBUG	 3

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


// DOD send message_type
#define PORTCONFIGURATIONREQUEST   3001

#define REALPORTCONFIGURATIONREQUEST   1515

#endif // _AFX_NO_AFXCMN_SUPPORT


/*
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__E2458693_8662_4DDB_82D3_173E56A0F256__INCLUDED_)
#define AFX_STDAFX_H__E2458693_8662_4DDB_82D3_173E56A0F256__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


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


// DOD send message_type
#define PORTCONFIGURATIONREQUEST   3001

#define REALPORTCONFIGURATIONREQUEST   1515





// DOD received message_type




#import "C:\Program Files\Active JMS\bin\active-jms.tlb"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E2458693_8662_4DDB_82D3_173E56A0F256__INCLUDED_)
*/