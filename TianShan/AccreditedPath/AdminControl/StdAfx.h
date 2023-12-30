// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__01E22716_8B43_4B12_A757_35A04ACAD102__INCLUDED_)
#define AFX_STDAFX_H__01E22716_8B43_4B12_A757_35A04ACAD102__INCLUDED_

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
#include <atlcom.h>
#include <atlctl.h>
#include <atlapp.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlmisc.h>
#include <atlsplit.h>
#include <atlctrlx.h>
#include <atlwin.h>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <Freeze/Freeze.h>
#include "TianShanDefines.h"
#include "TianShanIce.h"
#include "TsStorage.h"
#include "TsStreamer.h"
#include "EventChannel.h"
#include "TsApplication.h"
#include "TsTransport.h"
#include "TsSRM.h"
#include "TsPathAdmin.h"
#include "WeiwooAdmin.h"
#include "TsSite.h"
#include "SiteAdminSvc.h"

extern "C"  BOOL GetPrivateDataStr(char *dest,::TianShanIce::Variant *pvar);
extern "C"  BOOL CheckPrivateData(const std::string strKey,INT type, char *psrc,::TianShanIce::Variant *pvar);
extern "C"  char * Delblank(char *psrc);
extern "C"  BOOL DoubleChar(char *psrc);
extern "C"  BOOL IsInt(char *psrc);
extern "C"  BOOL IsString(char ch, char *psrc);



#pragma warning(disable:4518)
#pragma warning(disable:4502)



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__01E22716_8B43_4B12_A757_35A04ACAD102__INCLUDED)


