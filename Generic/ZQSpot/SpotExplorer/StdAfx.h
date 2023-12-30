// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__D07F4D07_F672_4672_8AA0_234192F15316__INCLUDED_)
#define AFX_STDAFX_H__D07F4D07_F672_4672_8AA0_234192F15316__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED


//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module

#include "InetAddr.h"

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>

#include "../SpotData.h"
#include "../ZQSpot.h"
#include "Test.h"
#include "TestI.h"
#include "Guid.h"

#include "log.h"
#include <iostream>
#include "variant.h"
#include <string>
using namespace std;
extern "C" int Init(int argc, char* argv[]);
extern "C" DWORD WINAPI DataChange(LPVOID pParam);
extern
ZQ::Spot::SpotEnv *g_pspotEnv;
extern	Ice::CommunicatorPtr g_ic;
extern char* g_Ip;
extern char* g_Port;
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D07F4D07_F672_4672_8AA0_234192F15316__INCLUDED)
