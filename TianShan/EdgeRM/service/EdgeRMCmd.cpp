// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: EdgeRMCmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/service/EdgeRMCmd.cpp $
// 
// 2     1/02/14 3:18p Zonghuan.xiao
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 6     09-08-07 10:40 Xiaoming.li
// 
// 4     09-07-27 17:08 Xiaoming.li
// 
// 3     09-04-14 9:02 Xiaoming.li
// initialize in ENV
// 
// 2     09-03-05 19:46 Hui.shao
// 
// 1     09-02-26 17:54 Hui.shao
// initial created
// ===========================================================================

#include "TianShanDefines.h"
#include "../EdgeRMImpl.h"
#include "Guid.h"
#include "Log.h"
#include "getopt.h"
#include "ZQResource.h"
#include "FileLog.h"
#include "RtspEngine.h"
#include "S6Handler.h"

extern "C"
{
#include <time.h>
#include <stdio.h>
#include <direct.h>
}

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

void usage()
{
/*
	printf("Usage: EdgeRMCmd [-e \"<endpoint>\"]\n");
	printf("       EdgeRMCmd -h\n");
	printf("EdgeRMCmd console mode server demo.\n");
	printf("options:\n");
	printf("\t-e   the local endpoint to bind, default %d\n", DEFAULT_ENDPOINT_Weiwoo);
#ifdef WITH_ICESTORM
	printf("\t-t   the IceStorm endpoint to publish events\n");
#endif // WITH_ICESTORM
	printf("\t-h   display this help\n");
*/
}

int main(int argc, char* argv[])
{
//	int ch;
	std::string programRoot = ZQTianShan::getProgramRoot();
	programRoot += FNSEPS;

	//printf("port=%d; endpoint=%s; adapter=%s; interface=%s\n", DEFAULT_BINDPORT_Weiwoo, DEFAULT_ENDPOINT_Weiwoo, ADAPTER_NAME_Weiwoo, SERVICE_NAME_SessionManager);
	//return  0;

	::mkdir((programRoot + "logs" FNSEPS).c_str());

	ZQ::common::FileLog EdgeRMLogger((programRoot + "logs" FNSEPS "EdgeRMCmd.log").c_str(), ZQ::common::Log::L_DEBUG, 1024*1024*50);
	EdgeRMLogger.setFileSize(1024*1024*50);

	ZQ::common::FileLog EdgeRMEventLogger((programRoot + "logs" FNSEPS "EdgeRMCmd_events.log").c_str(), ZQ::common::Log::L_INFO, 1024*1024*20, 1);
	EdgeRMEventLogger.setFileSize(1024*1024*20);

	EdgeRMLogger.setVerbosity(ZQ::common::Log::L_DEBUG);
//	EdgeRMLogger.enableAsciiCopyInHexMsg(true);
//	EdgeRMLogger.hexDump(ZQ::common::Log::L_CRIT, "==== %s; \r\nVersion\r sfds\t  safsa \rsdffs \r\n", 60, "hahaha");
	EdgeRMLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "==== %s; Version %d.%d.%d.%d"), ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	EdgeRMLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "====================================================="));
	ZQ::common::setGlogger(&EdgeRMLogger);

	EdgeRMEventLogger.setVerbosity(ZQ::common::Log::L_INFO);
	::ZQ::common::NativeThreadPool threadpool;
	threadpool.resize(50);

	{
		int i =0;
		::Ice::InitializationData initData;
		initData.properties = Ice::createProperties(i, NULL);
		initData.properties->setProperty("Freeze.Trace.DbEnv", "2");
		initData.properties->setProperty("Ice.ThreadPool.Server.Size","30");
		initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax","60");
		initData.properties->setProperty("Ice.ThreadPool.Client.Size","30");
		initData.properties->setProperty("Ice.ThreadPool.Client.SizeMax","60");
		initData.properties->setProperty("Ice.Override.ConnectTimeout", "0");
		initData.properties->setProperty("Ice.Override.Timeout", "2000");
		Ice::CommunicatorPtr ic = Ice::initialize(initData);
		ZQADAPTER_DECLTYPE adapter;

		try
		{
			adapter = ZQADAPTER_CREATE(ic, "EdgeRM", DEFAULT_ENDPOINT(EdgeRM), EdgeRMLogger);
		}
		catch(Ice::Exception& ex)
		{
			EdgeRMLogger(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "Create adapter failed with endpoint=%s and exception is %s"), DEFAULT_ENDPOINT(EdgeRM), ex.ice_name().c_str());
			return -2;
		}

		::ZQTianShan::EdgeRM::EdgeRMEnv env(EdgeRMLogger, EdgeRMEventLogger, threadpool, adapter, (programRoot + "data" FNSEPS "EdgeRM").c_str());
		//env.initialize();

		::ZQTianShan::EdgeRM::EdgeRMImpl::Ptr erm = new ::ZQTianShan::EdgeRM::EdgeRMImpl(env);
		env._netId="Foo";

		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
		{
			printf("Unable to install the console handler!                      \n");
			return -1;
		}

		::TianShanIce::EdgeResource::EdgeRMPrx _edgeRMPrx = NULL;
		try
		{
			::std::string strERMPrxEndpoint = ::std::string("EdgeRM") + DEFAULT_ENDPOINT(EdgeRM);
			_edgeRMPrx = ::TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(env._adapter->getCommunicator()->stringToProxy(strERMPrxEndpoint.c_str()));
			EdgeRMLogger(::ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "Edge Resouce Management Proxy create success"));
		}
		catch(...)
		{
			EdgeRMLogger(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "Edge Resouce Management Proxy create failed"));
			return S_FALSE;
		}

		ZQRtspEngine::RtspEngine *_rtspEngine = new (std::nothrow) ZQRtspEngine::RtspEngine(EdgeRMLogger, 10);
		//ZQRtspEngine::IHandler * rtspHandler = NULL;
		::ZQTianShan::EdgeRM::S6Handler *rtspHandler = NULL;
		::ZQTianShan::EdgeRM::S6HandlerList _s6HandlerList;
		for (int32 i = 0; i < 10; i++)
		{
			rtspHandler = new (std::nothrow) ::ZQTianShan::EdgeRM::S6Handler(env, _edgeRMPrx);
			_s6HandlerList.push_back(rtspHandler);
			_rtspEngine->registerHandler(rtspHandler);
		}

		if (!_rtspEngine->startTCPRtsp("0.0.0.0", "00:00", "554"))
		{
			_rtspEngine->stop();
			return 1;
		}

		printf("\"Ctrl-C\" at any time to exit the program.              \n");

		adapter->activate();
		EdgeRMEventLogger(EventFMT(env._netId.c_str(), Service, Activated, 0, "service[%s] Version[%d.%d.%d.%d]"), ZQ_INTERNAL_FILE_NAME, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);

		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rEdgeRM is now serving at [%s] %c", DEFAULT_ENDPOINT_ContentStore, chs[chi]);
			Sleep(200);
		}

		printf("\rEdgeRM is quiting . . .                                                      \r");
		env.uninitialize();
		erm = NULL;
		Sleep(6000);
		adapter->deactivate();
		ic->destroy();

		_rtspEngine->stopAllCommunicators();
		_rtspEngine->stop();
		delete _rtspEngine;
		_rtspEngine = NULL;

		for (::ZQTianShan::EdgeRM::S6HandlerList::iterator iter = _s6HandlerList.begin(); iter != _s6HandlerList.end(); iter++)
			delete (*iter);
		_s6HandlerList.clear();

		EdgeRMEventLogger(EventFMT(env._netId.c_str(), Service, Deactivated, 1, "service[%s] Version[%d.%d.%d.%d]"), ZQ_INTERNAL_FILE_NAME, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	}

	Sleep(100);

	ZQ::common::setGlogger(NULL);
	printf("\rEdgeRM stopped             \n");
	Sleep(100);

	return 0;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
        break;
    }
    return TRUE;
}
