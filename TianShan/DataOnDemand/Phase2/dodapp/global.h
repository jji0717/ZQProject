// global.h: interface for the global class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBAL_H__C0FEA66D_5710_40F5_A2E2_B0D8703D74B9__INCLUDED_)
#define AFX_GLOBAL_H__C0FEA66D_5710_40F5_A2E2_B0D8703D74B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MiniDump.h"
#include "ActiveChannelMgr.h"
#include "dodappsvc.h"
#include "DODAppMain.h"
#include "DODAppCfg.h"
extern std::string			sourceCachePath;
extern bool					g_bServiceStarted;
extern std::string			configSpaceName;
extern ActiveChannelMgr		activeChannelManager;
extern ZQ::common::MiniDump	g_minidump;
extern ZQ::common::Config::Loader<DODAppCfg> gDODAppServiceConfig;

void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress);
bool initGlobal();

#endif // !defined(AFX_GLOBAL_H__C0FEA66D_5710_40F5_A2E2_B0D8703D74B9__INCLUDED_)
