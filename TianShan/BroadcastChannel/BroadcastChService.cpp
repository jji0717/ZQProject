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

// Branch: $Name:BroadcastChService.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BroadcastChService.cpp $
// 
// 11    2/26/16 5:00p Li.huang
// fix bug 22376
// 
// 10    7/08/15 3:21p Li.huang
// 
// 9     7/07/15 4:37p Li.huang
// fix bug 21518
// 
// 8     10/21/14 3:32p Li.huang
// 
// 7     10/17/14 3:37p Li.huang
// 
// 6     6/03/14 2:31p Li.huang
// 
// 5     5/23/14 2:35p Li.huang
// 
// 4     1/02/14 6:07p Zonghuan.xiao
// 
// 3     12/31/13 5:21p Hui.shao
// 
// 2     12/31/13 4:23p Hui.shao
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 7     10-06-01 16:00 Li.huang
// add ICE Trace file log count
// 
// 6     09-12-15 16:10 Li.huang
// 
// 6     09-12-09 18:22 Li.huang
// fix some bugs
// 
// 5     09-09-11 16:02 Li.huang
// 
// 4     09-07-06 9:48 Li.huang
// 
// 3     09-06-30 11:31 Li.huang
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "Log.h"
#include "BroadcastChService.h"
#include "BroadCastChannelEnv.h"
#include "IceLog.h"
#include "PlaylistEventSinkImpl.h"


#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif

using ZQ::common::BaseZQServiceApplication;
using ZQ::common::Log;

BroadcastChService g_server;

BaseZQServiceApplication* Application = &g_server;

ZQ::common::Config::Loader<BroadcastChCfg> gBroadcastChCfg("BroadcastChannel.xml");
ZQ::common::Config::ILoader *configLoader = &gBroadcastChCfg;

int pauseMax;
int pauseMin;

#ifdef ZQ_OS_MSWIN

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

ZQ::common::MiniDump g_minidump;


void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog(ZQ::common::Log::L_ERROR,
		"Crash exception callback called,ExceptionCode 0x%08x, "
		"ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);
}
#else
extern const char* DUMP_PATH;
#endif

BroadcastChService::BroadcastChService(void)
{
	_pBcastCHSvcEnv = NULL;
	icelog = NULL;
}

BroadcastChService::~BroadcastChService(void)
{
}

HRESULT 
BroadcastChService::OnInit()
{
	BaseZQServiceApplication::OnInit();

	// ZQ::common::setGlogger(m_pReporter);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "OnInit() enter"));

	pauseMax=gBroadcastChCfg.mrtStreamServiceCfg.pauseMaxCfg;
	pauseMin=gBroadcastChCfg.mrtStreamServiceCfg.pauseMinCfg;

	if(gBroadcastChCfg.broadcastPPendpoint.size() < 1)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadcastChService, "key [BroadcastPublisherEndPoint] must be configured"));
		return S_FALSE;
	}

	// get the endpoint of topic manager
	if(gBroadcastChCfg.TopicMgrEndPoint.size() < 1)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadcastChService, "key [EventChannelEndPoint] must be configured"));
		return S_FALSE;
	}	

	// get the endpoint of Weiwoo
	if(gBroadcastChCfg.weiwooendpoint.size() < 1)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadcastChService, "key [WeiooEndPoint] must be configured"));
		return S_FALSE;
	}	

	if(gBroadcastChCfg.portIncreaseBase <= 0 )
		gBroadcastChCfg.portIncreaseBase = 1;

	if(gBroadcastChCfg.miniPLcount< MINIMUMPLITEMCOUNT)
		gBroadcastChCfg.miniPLcount = MINIMUMPLITEMCOUNT;
	//
	// Get Configuration from Application's sub Registry Level - ICE
	//	
	_properties = Ice::createProperties();

	std::map<std::string, std::string>::iterator iter = gBroadcastChCfg.icePropMap.begin();
	for (; iter != gBroadcastChCfg.icePropMap.end(); ++iter) 
	{
		_properties->setProperty(iter->first, iter->second);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "OnInit() set ICE properties <%s>--<%s>."), (iter->first).c_str(), (iter->second).c_str());
	}
#ifdef ZQ_OS_MSWIN	
	// init Minidump	
	if (!initMiniDump())
	{
		return S_FALSE;
	}
#else
	DUMP_PATH = gBroadcastChCfg.crushDumpPath.c_str();
#endif
		

	gBroadcastChCfg.snmpRegister("");
	glog(ZQ::common::Log::L_INFO, "OnInit() leave");

	return S_OK;
}

HRESULT 
BroadcastChService::OnStart()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "OnStart() enter"));

	BaseZQServiceApplication::OnStart();

	//
	// Initialize Ice communicator properties
	//
	int argc = 0;
	try
	{

		std::string strLogFolder;

#ifdef ZQ_OS_MSWIN
		strLogFolder = m_wsLogFolder;
#else
		strLogFolder = _logDir;
#endif
		int size = strLogFolder.size();
		if(size > 0 && strLogFolder[size -1] != '\\' && strLogFolder[size -1] != '/')
			strLogFolder += "\\";

		icelog = new ZQ::common::FileLog(gBroadcastChCfg.iceLogPath.c_str(),
			gBroadcastChCfg.iceLogLevel, gBroadcastChCfg.iceLogCount, gBroadcastChCfg.iceLogSize);
		_icelog = new TianShanIce::common::IceLogI(icelog);
		Ice::InitializationData initData;
		initData.logger = _icelog;
		initData.properties = _properties;
		_communicator = Ice::initialize(argc, 0, initData);
		glog(ZQ::common::Log::L_INFO, "Ice communicator created");

		_pBcastCHSvcEnv = new ZQBroadCastChannel::BroadCastChannelEnv(_communicator);	
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "BroadCastChannelEnv object created"));		
		if (!_pBcastCHSvcEnv->init())
		{
			return S_FALSE;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "BroadCastChannelEnv object initialized"));

	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadcastChService, "Ice initialize properties met exception with error: %s, %s @ line %d"), 
			ex.ice_name().c_str(), ex.ice_file(), ex.ice_line());
		return S_FALSE;					
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "OnStart() leave"));

	return S_OK;
}

HRESULT 
BroadcastChService::OnStop()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "OnStop() enter"));

	try
	{
		if (_pBcastCHSvcEnv)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "Start to delete BroadCastChannelEnv object"));
			_pBcastCHSvcEnv->unInit();
			delete _pBcastCHSvcEnv;
			_pBcastCHSvcEnv = NULL;
			glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "BroadCastChannelEnv object deleted"));
		}

		if(_communicator != NULL)
		{
			_communicator->destroy();
		}		
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadcastChService, "Ice destroy met exception with error: %s @ line %d"), 
			ex.ice_file(), ex.ice_line());
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadcastChService, "Ice destroy met unknown exception"));
	}

	BaseZQServiceApplication::OnStop();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadcastChService, "OnStop() leave"));

	return S_OK;
}

HRESULT 
BroadcastChService::OnUnInit()
{
	try
	{
		if(icelog)
			delete icelog;
		icelog = NULL;
	}
	catch (...)
	{

	}
    BaseZQServiceApplication::OnUnInit();

	return S_OK;	
}
#ifdef ZQ_OS_MSWIN
bool BroadcastChService::initMiniDump()
{
	std::string dumpPath = gBroadcastChCfg.crushDumpPath;
	if (dumpPath[dumpPath.size() - 1] != '\\' && dumpPath[dumpPath.size() - 1] != '/')
		dumpPath += '\\';

	if(!g_minidump.setDumpPath((char*)dumpPath.c_str()))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadcastChService, 
			"key [CrashDumpPath] is not correct directory"));
		return false;
	}
	g_minidump.enableFullMemoryDump(true);
	g_minidump.setExceptionCB(MiniDumpCallback);
	return true;
}
#endif