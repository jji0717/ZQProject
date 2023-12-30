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
// Ident : $Id: PathSvc.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/service/PathSvc.cpp $
// 
// 2     1/02/14 3:20p Zonghuan.xiao
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 16    07-10-25 13:30 Yixin.tian
// 
// 15    06-12-25 16:58 Hui.shao
// fixed glog to envlog; _throw with envlog
// 
// 14    06-12-19 10:06 Yonghua.deng
// 
// 13    06-12-15 10:54 Yonghua.deng
// 
// 12    9/21/06 4:34p Hui.shao
// batch checkin on 20060921
// 
// 11    06-09-19 11:47 Hui.shao
// ===========================================================================

#include "PathManagerImpl.h"
#include "getopt.h"
#include "Log.h"
#include "ZQResource.h"
#include <WeiwooConfig.h>
// #include "../pho/IpEdgePHO.h"
// #include "../pho/Raid5sqrPHO.h"

#include <sclog.h>

extern "C"
{
#include <time.h>
#include <stdio.h>
}

WeiwooConfig					gWeiwooServiceConfig;

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

void usage()
{
	printf("Usage: PathSvc [-e \"<endpoint>\"]\n");
	printf("       PathSvc -h\n");
	printf("PathSvc console mode server demo.\n");
	printf("options:\n");
	printf("\t-e       the local endpoint to bind, default %s\n", DEFAULT_ENDPOINT_PathManager);

#ifdef WITH_ICESTORM
	printf("\t-t   the IceStorm endpoint to publish events\n");
#endif // WITH_ICESTORM
	printf("\t-h   display this help\n");
}

#include "DynSharedObj.h"

int main(int argc, char* argv[])
{
	int ch;
	std::string endpoint = DEFAULT_ENDPOINT_PathManager, epIceStorm = "default -h 192.168.80.49 -p 10000";

#ifdef _DEBUG
	int trace = ZQ::common::Log::L_DEBUG;
#else 
	int trace = ZQ::common::Log::L_ERROR;
#endif // _DEBUG

	int round = 1000;

	char*	pLogPath=NULL;
	//Set logPath to current path/////////////////////////////////////////////
	char szLocalPath[1024];
	GetModuleFileNameA(NULL,szLocalPath,sizeof(szLocalPath));
	
	szLocalPath[strlen(szLocalPath)-4]='\0';
	strcat(szLocalPath,".log");
	pLogPath=szLocalPath;

	while((ch = getopt(argc, argv, "hd:e:t:")) != EOF)
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

		case 'd':
			trace = atoi(optarg);
			break;

		case 't':
			epIceStorm = optarg;
			break;

		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	ZQ::common::Log* pathSvcLog = new ZQ::common::ScLog(pLogPath,trace,1024*1024*10);
	trace = (trace>7) ? 7 : ((trace<0) ? 0 : trace);
	pathSvcLog->setVerbosity(trace);
	(*pathSvcLog)(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "==== %s; Version %d.%d.%d.%d"), ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	ZQ::common::setGlogger(pathSvcLog);

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);

    printf("Start " ADAPTER_NAME_PathManager " at \"%s\"\n", endpoint.c_str());
	
	ZQ::common::NativeThreadPool gThreadPool;

	::ZQTianShan::AccreditedPath::PathSvcEnv env(glog, gThreadPool, ic);

	TianShanIce::Transport::PathManagerPtr ADPaths = new ZQTianShan::AccreditedPath::AccreditedPathsImpl(env);

    env._adapter->activate();

	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}

	printf("\"Ctrl-C\" at any time to exit the program.              \n");
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		printf("\r" ADAPTER_NAME_PathManager " is now listening %c", chs[chi]);
		Sleep(200);
	}

	printf("\r" ADAPTER_NAME_PathManager " is quiting                   ");
	ic->destroy();
	Sleep(100);

	ZQ::common::setGlogger(NULL);
	(*pathSvcLog)(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceShutDown, "================"));
	delete pathSvcLog;
	pathSvcLog = NULL;


	printf("\r" ADAPTER_NAME_PathManager " stopped                    \n");
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

