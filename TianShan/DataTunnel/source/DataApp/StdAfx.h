// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2E0E59A9_A750_4C42_A35D_89B9D2A5D9F3__INCLUDED_)
#define AFX_STDAFX_H__2E0E59A9_A750_4C42_A35D_89B9D2A5D9F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <afx.h>
#include <stdio.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>
//#include <application.h>
#include <log.h>

#include <io.h>
#include <assert.h>

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include "DataAppImpl.h"
#pragma warning(disable : 4244)
#pragma warning(disable : 4786)

// TODO: reference additional headers your program requires here
//////////////////////////////////////////////////////////////////////////
#define ADAPTER_NAME_DODAPP "DataTunnelAppService"
#define DATA_ONDEMAND_APPNAME "DataOnDemandApp"
#define DATA_ONDEMAND_DODAPPNAME "DataPointPublisher"
#define Servant_DataTunnel  "DataTunnelEvictor"

#define QUEUECF           _T("queue/queue_cf")
#define CONFIGMSGTIMEOUT  7000
#define UPDATEINTERVAL    5
//////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2E0E59A9_A750_4C42_A35D_89B9D2A5D9F3__INCLUDED_)
