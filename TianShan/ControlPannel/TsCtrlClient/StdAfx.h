// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__7936DDFE_BD63_447A_BE5D_6E1C8F1F4DA2__INCLUDED_)
#define AFX_STDAFX_H__7936DDFE_BD63_447A_BE5D_6E1C8F1F4DA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZQ_common_conf.h"
#include "XMLPreference.h"
#include <windows.h>
#include <string>
#include <Locks.h>

using std::string;
#pragma warning(disable:4518  4786 4502)


#include <Ice/Ice.h>
#include <Ice/Exception.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <Freeze/Freeze.h>
#include "TianShanDefines.h"
#include "TianShanIce.h"
#include "TsStorage.h"
#include "TsStreamer.h"
#include "EventChannel.h"
#include "StreamSmithAdmin.h"
#include "ChannelOnDemand.h"
#include "ChannelOnDemandEx.h"
#include "TsTransport.h"
#include "TsSRM.h"
#include "TsPathAdmin.h"
#include "WeiwooAdmin.h"
#include "TsSite.h"
#include "SiteAdminSvc.h"


#include "Exception.h"

typedef struct _tagAllItemsData
{
	int iIndex;
	string strDataTime;
	string strCateGory;
	string strEventData;
}ALLITEMSDATA,*PALLITEMSDATA;
//extern ALLITEMSDATA  m_EventData;
extern  ZQ::common::Mutex		m_Mutex;

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define EVENTSOURCE "EventSource"
#define ICESTORM    "IceStorm"
#define SERVICE     "Service"
#define ENDPOINT    "EndPoint"
#define SOURCENAME  "Name"
#define XMLNAME_LEN  64
#define XMLDATA_LEN  200

extern "C"  BOOL GetPrivateDataStr(char *dest,::TianShanIce::Variant *pvar);
extern "C"  BOOL CheckPrivateData(const std::string strKey,INT type, char *psrc,::TianShanIce::Variant *pvar);
extern "C"  char * Delblank(char *psrc);
extern "C"  BOOL DoubleChar(char *psrc);
extern "C"  BOOL IsInt(char *psrc);
extern "C"  BOOL IsString(char ch, char *psrc);



// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7936DDFE_BD63_447A_BE5D_6E1C8F1F4DA2__INCLUDED_)
