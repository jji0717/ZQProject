// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Name  : nsBuilder.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : impl for class NavigationService
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/NavigationService.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 15    08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 14    08-04-17 13:44 Ken.qian
// make MaxWQOptRebuild configurable
// 
// 13    07-03-16 15:36 Ken.qian
// 
// 12    07-03-16 15:25 Ken.qian
// 
// 11    12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 10    06-10-19 13:56 Ken.qian
// 
// 9     06-08-04 14:12 Ken.qian
// move from comextra::Thread to common::NativeThread
// 
// 8     5/16/06 8:30a Bernie.zhao
// fixed entrycount calc bug, added auto-detect of 'ns_folderupdate'
// parameter count
// 
// 7     06-01-12 21:11 Bernie.zhao
// added logic to support PM
// 
// 6     05-12-20 11:42 Bernie.zhao
// 
// 5     05-10-13 6:26 Bernie.zhao
// quick sp to telewest, with same version number as 1.1.2
// 
// 3     05-07-22 11:57 Bernie.zhao
// ver 1.1.1,
// 1. modified rebuild strategy
// 2. update view/viewfolders when update space
// 
// 2     05-04-19 10:30 Bernie.zhao
// autobuild modification
// 
// 1     05-03-25 12:47 Bernie.zhao
// ===========================================================================
#include ".\navigationservice.h"
#include "zqresource.h"
#include "MiniDump.h"

NavigationService gNavigator;
BaseSchangeServiceApplication *Application = &gNavigator;
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

// static members initialization
DWORD NavigationService::m_dwTimerInterval;
DWORD NavigationService::m_dwDBTimeout;
DWORD NavigationService::m_dwTeaTime;
wchar_t NavigationService::m_wszRegionID[16];
DWORD NavigationService::m_maxWQOptRebuild;
DWORD NavigationService::m_dwSQLTraceEnabled;

DWORD NavigationService::m_dwDBConnRetryInterval;

int NavigationService::m_folderUpdateParamCount;
bool NavigationService::m_bQANavigationEnabled;


wchar_t NavigationService::m_wszDSN[MAXLEN_DSN];
wchar_t NavigationService::m_wszUID[MAXLEN_UID];
wchar_t NavigationService::m_wszPWD[MAXLEN_PWD];
wchar_t NavigationService::m_wszDATABASE[MAXLEN_DB];
wchar_t NavigationService::m_wszDBTYPE[MAXLEN_DBTYPE];

wchar_t NavigationService::m_wszVerInfo[64];

DWORD NavigationService::m_folderUpdateSPType = 0;
DWORD NavigationService::m_dwUpdateWaitTime = 10;
//////////////////////////////////////////////////////////////////////////
// Here we added the top level exception handler
// So each time a crash happens, we can generate minidump
// The setting of the dump class is OnInit()
// Don't forget to add Debug info into release version
// And deliver the .pdb file with the release
//							Bernie Zhao, 2006.11.30
//////////////////////////////////////////////////////////////////////////
ZQ::common::MiniDump gCrashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


NavigationService::NavigationService(void)
{
	m_dwTimerInterval = DEFAULT_INTERVAL;
	m_dwDBTimeout = DEFAULT_TIMEOUT;
	m_dwTeaTime = DEFAULT_TEATIME;
	m_dwDBConnRetryInterval = DEFAULT_DBCONN_INTERVAL;

	wcscpy(m_wszRegionID, _T("TWT"));
	
	wcscpy(m_wszDSN, _T("SyncDSN"));
	wcscpy(m_wszUID, _T("multiverse"));
	wcscpy(m_wszPWD, _T("multiverse"));
	wcscpy(m_wszDATABASE, _T("multiverse"));
	wcscpy(m_wszDBTYPE, _T("SQL Server"));

	wcscpy(m_wszVerInfo, _T(ZQ_PRODUCT_VER_STR3));

	m_maxWQOptRebuild = MAX_WQ_OPT_REBUILD;
	m_dwSQLTraceEnabled = 1;

	m_folderUpdateParamCount = 0;
	m_bQANavigationEnabled = true;
}

NavigationService::~NavigationService(void)
{
}

HRESULT NavigationService::OnInit()
{
	//////////////////////////////////////////////////////////////////////////
	// log init
	BaseSchangeServiceApplication::OnInit();
	pGlog = m_pReporter;
	//m_pReporter->setReportLevel(ALL_LOGS, Log::L_NOTICE);

	glog(Log::L_DEBUG,L"NSSync %s", m_wszVerInfo);
	//////////////////////////////////////////////////////////////////////////
	// get registry config
	DWORD dwNum=0;
	DWORD dwError = 0;
	manageVar(L"Version", MAN_STR, (DWORD)&m_wszVerInfo, TRUE, &dwError);

	// ------------ nsTimer
	getConfigValue(L"SuperviseInterval",&m_dwTimerInterval,m_dwTimerInterval,true,true);
	// ------------ nsBuild

	getConfigValue(L"IZQDBTimeout",&m_dwDBTimeout,m_dwDBTimeout,true,true);

	getConfigValue(L"IZQDBConnRetryInterval",&m_dwDBConnRetryInterval,m_dwDBConnRetryInterval,true,true);

	getConfigValue(L"TeaTime", &m_dwTeaTime, m_dwTeaTime, true, true);

	dwNum = 16*2;
	getConfigValue(L"RegionID", m_wszRegionID, m_wszRegionID, &dwNum, true, true);
	
	dwNum = MAXLEN_DSN*2;
	getConfigValue(L"IZQDBDSN",m_wszDSN,m_wszDSN,&dwNum,true,true);
	dwNum = MAXLEN_UID*2;
	getConfigValue(L"IZQDBUserName",m_wszUID,m_wszUID,&dwNum,true,true);
	dwNum = MAXLEN_PWD*2;
	getConfigValue(L"IZQDBPassword",m_wszPWD,m_wszPWD,&dwNum,true,true);
	dwNum = MAXLEN_DB*2;
	getConfigValue(L"IZQDBName",m_wszDATABASE,m_wszDATABASE,&dwNum,true,true);
	dwNum = MAXLEN_DBTYPE*2;
	getConfigValue(L"IZQDBType",m_wszDBTYPE, m_wszDBTYPE, &dwNum, true,true);

	getConfigValue(L"MaxWQOptRebuild", &m_maxWQOptRebuild, m_maxWQOptRebuild, true, true);
//	if(m_maxWQOptRebuild>=MAX_WQ_HISTORY)
//		m_maxWQOptRebuild = MAX_WQ_HISTORY-1;

	getConfigValue(L"SQLTraceEnabled", &m_dwSQLTraceEnabled, m_dwSQLTraceEnabled, true, true);

	getConfigValue(L"WQHandleDelaySeconds", &m_dwUpdateWaitTime, m_dwUpdateWaitTime, true, true);
	//////////////////////////////////////////////////////////////////////////
	// set minidump attributes
	WCHAR dumpPath[MaxPath];
	_tcscpy(dumpPath, m_wsLogFileName);
	TCHAR* pos = _tcsrchr(dumpPath, _T('\\'));
	if(NULL != pos)
	{
		// put the mini dump to the same folder of the log file
		*pos = _T('\0');
	}
	else
	{
		// put into temp folder, otherwise "c:\temp\"
		if (GetTempPath( _MAX_PATH, dumpPath ) == 0)
			_tcscpy( dumpPath, _T("c:\\temp\\") );
	}
	
	gCrashDump.setDumpPath(dumpPath);
	gCrashDump.enableFullMemoryDump(TRUE);
	gCrashDump.setExceptionCB(CrashExceptionCallBack);	

	//////////////////////////////////////////////////////////////////////////
	// component init and config
	_theBuilder			= new nsBuilder();
	_theTimerChecker	= new nsTimerChecker();
	_theTimer			= new nsTimer(_theBuilder);

	_theBuilder->setDSN(m_wszDSN);
	_theBuilder->setUID(m_wszUID);
	_theBuilder->setPWD(m_wszPWD);
	_theBuilder->setDATABASE(m_wszDATABASE);
	_theBuilder->setTimeout(m_dwDBTimeout);
	_theBuilder->setMaxWQOptRebuild(m_maxWQOptRebuild);
	
	int count = 0;
	while(!m_bShutdownFlag && !_theBuilder->init())
	{
		count++;
		glog(Log::L_WARNING, L"nsBuild init() failed with %d times, will retry %d seconds later", count, m_dwDBConnRetryInterval);

		Sleep(m_dwDBConnRetryInterval*1000);
	}

	if(m_bShutdownFlag)
	{
		glog(Log::L_NOTICE, L"NSSync Service was manually stopped");

		return NS_ERROR;
	}

	_theTimer->setInterval(m_dwTimerInterval);

	//////////////////////////////////////////////////////////////////////////
	// register ManUtil complex lists
	manageVar(L"Navigation Work Queue", MAN_COMPLEX, (DWORD)(UINT *)nsBuilder::wqInfoCallBack, TRUE, &dwError);
	manageVar(L"QA Navigation Work Queue", MAN_COMPLEX, (DWORD)(UINT *)nsBuilder::qaWqInfoCallBack, TRUE, &dwError);

	glog(Log::L_NOTICE, L"*************** NSSync service kernel starting. Version: %s ***************", m_wszVerInfo);
	return NS_SUCCESS;
}

HRESULT NavigationService::OnUnInit()
{
	HRESULT hr =BaseSchangeServiceApplication::OnUnInit();
	_theBuilder->uninit();

	if(_theTimerChecker) {
		if(_theTimerChecker->isRunning())
		{
			_theTimerChecker->signalTerm();
			_theTimerChecker->waitHandle(5000);
		}
		delete _theTimerChecker;
	}
	if(_theTimer){
		if(_theTimer->isRunning())
		{
			_theTimer->signalTerm();
			_theTimer->waitHandle(5000);
		}
		delete _theTimer;
	}
	if(_theBuilder)
		delete _theBuilder;

	return hr;
}

HRESULT NavigationService::OnStart()
{
	HRESULT hr = BaseSchangeServiceApplication::OnStart();
	_theTimer->start();
	_theTimerChecker->start();
	return hr;
}

HRESULT NavigationService::OnStop()
{
	HRESULT hr = BaseSchangeServiceApplication::OnStop() ;
	_theBuilder->signalTerm();
	_theTimerChecker->signalTerm();
	_theTimer->signalTerm();
	return hr;
}

void NavigationService::exceptionCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog(Log::L_EMERG, L"NSSync service crashed. Crash exception callback called, ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x", ExceptionCode, ExceptionAddress, dwThreadID);

	m_pReporter->flush();
}

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	gNavigator.exceptionCallback(ExceptionCode,ExceptionAddress);
}

