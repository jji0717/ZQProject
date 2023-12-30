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
// Ident : $Id: WeiwooCmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/service/WeiwooCmd2.cpp $
// 
// 2     1/02/14 10:19a Hui.shao
// pglog
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 13    06-12-25 16:05 Hui.shao
// stamp service version and start
// 
// 12    06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 11    06-12-19 10:11 Yonghua.deng
// 
// 9     06-12-13 18:48 Hongquan.zhang
// 
// 8     9/21/06 5:11p Hui.shao
// 
// 7     06-07-17 14:47 Hui.shao
// initial impl of session manager
// 
// 6     06-07-13 13:48 Hui.shao
// 
// 5     06-07-10 11:33 Hui.shao
// 
// 4     06-07-07 14:52 Hui.shao
// 
// 3     06-07-05 19:53 Hui.shao
// 
// 2     06-07-05 15:46 Hui.shao
// console demo ready
// 
// 1     06-07-05 15:25 Hui.shao
// ===========================================================================

#include "BusinessRouterImpl.h"
#include "SessionImpl.h"
#include "Guid.h"
#include "Log.h"
#include "getopt.h"
#include "ZQResource.h"

#include <sclog.h>

extern "C"
{
#include <time.h>
#include <stdio.h>
}

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

void usage()
{
	printf("Usage: Weiwoo [-e \"<endpoint>\"]\n");
	printf("       Weiwoo -h\n");
	printf("Weiwoo console mode server demo.\n");
	printf("options:\n");
	printf("\t-e   the local endpoint to bind, default %d\n", DEFAULT_ENDPOINT_Weiwoo);
#ifdef WITH_ICESTORM
	printf("\t-t   the IceStorm endpoint to publish events\n");
#endif // WITH_ICESTORM
	printf("\t-h   display this help\n");
}

#include "DynSharedObj.h"

int main(int argc, char* argv[])
{
	int ch;
	std::string endpoint = DEFAULT_ENDPOINT_Weiwoo, epIceStorm = "default -h 192.168.80.49 -p 10000";
	int traceLevel = ZQ::common::Log::L_DEBUG;	
	char*	pLogPath=NULL;
	
	//Set logPath to current path/////////////////////////////////////////////
	char szLocalPath[1024];
	GetModuleFileNameA(NULL,szLocalPath,sizeof(szLocalPath));

	szLocalPath[strlen(szLocalPath)-4]='\0';
	strcat(szLocalPath,".log");


	pLogPath=szLocalPath;
	//////////////////////////////////////////////////////////////////////////
	
	while((ch = getopt(argc, argv, "he:t:d:l:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);

		case 'e':
			endpoint = optarg;
			break;

		case 't':
			epIceStorm = optarg;
			break;

		case 'd':
			traceLevel = atoi(optarg);
			break;
		case 'l':
			pLogPath=optarg;
			break;
		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	ZQ::common::Log plog=new ZQ::common::ScLog(pLogPath,traceLevel,1024*1024*10);
	ZQ::common::setGlogger(plog);

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
	plog->setVerbosity(traceLevel);

	glog(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "%s; Version %d.%d.%d.%d"), ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
    printf("Start Weiwoo at \"%s\"\n", endpoint.c_str());

	::ZQ::common::NativeThreadPool threadpool;

	::ZQTianShan::Weiwoo::WeiwooSvcEnv env(glog, threadpool, ic);

#ifdef WITH_ICESTORM
	if (!epIceStorm.empty())
	{
		env._log(ZQ::common::Log::L_DEBUG, CLOGFMT(Weiwoo, "opening TianShan topic manager at \"%s\""), epIceStorm.c_str());
		try {
			IceStorm::TopicManagerPrx topicManager = IceStorm::TopicManagerPrx::checkedCast(ic->stringToProxy(std::string(SERVICE_NAME_TopicManager ":") + epIceStorm));
			env.openPublisher(topicManager);
		}
		catch(Ice::Exception& e)
		{
			env._log(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to init event publisher at \"%s\": %s"), epIceStorm.c_str(), e.ice_name().c_str());
		}
		catch(...)
		{
			env._log(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to init event publisher at \"%s\": unknown exception"), epIceStorm.c_str());
		}
	}
#endif // WITH_ICESTORM

	TianShanIce::Weiwoo::BusinessRouterPtr brouter = new ZQTianShan::Weiwoo::BusinessRouterImpl(env);
	TianShanIce::Weiwoo::SessionManagerPtr sessmgr = new ZQTianShan::Weiwoo::SessionManagerImpl(env);
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}

    env._adapter->activate();

	printf("\"Ctrl-C\" at any time to exit the program.              \n");
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		printf("\rWeiwoo is now listening %c", chs[chi]);
		Sleep(200);
	}

	printf("\rWeiwoo is quiting                   ");
	ic->destroy();
	Sleep(100);
	
	glog(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceShutDown, "================"));
	ZQ::common::setGlogger();

	delete plog;

	printf("\rWeiwoo stopped                    \n");
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
