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
// Ident : $Id: SentryCmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/service/SentryCmd.cpp $
// 
// 2     1/02/14 10:23a Hui.shao
// pglog
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 3     07-08-01 17:12 Hongquan.zhang
// 
// 2     07-05-29 18:47 Hui.shao
// rewrote the ZQAdapter
// ===========================================================================

#include "SentryImpl.h"
#include "getopt.h"
#include "FileLog.h"
#include "ZQResource.h"
#include "TsLayout.h"

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
	printf("Usage: Sentry [-e \"<endpoint>\"]\n");
	printf("       Sentry -h\n");
	printf("Sentry console mode server demo.\n");
	printf("options:\n");
	printf("\t-e   the local endpoint to bind, default %d\n", DEFAULT_ENDPOINT_Sentry);
#ifdef WITH_ICESTORM
	printf("\t-t   the IceStorm endpoint to publish events\n");
#endif // WITH_ICESTORM
	printf("\t-h   display this help\n");
}
extern bool initHttpd();

int main(int argc, char* argv[])
{
	initHttpd();
	int ch;
	std::string endpoint = DEFAULT_ENDPOINT_Sentry, epIceStorm = "default -h 192.168.80.49 -p 10000";
	int traceLevel = ZQ::common::Log::L_DEBUG;	
	char*	pLogPath=NULL;
	
	//Set logPath to current path/////////////////////////////////////////////
	char path[MAX_PATH] = ".", *p=path;
	if (::GetModuleFileName(NULL, path, MAX_PATH-1)>0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
				*p='\0';
		}
	}
	strcat(path, FNSEPS);
	p = path+strlen(path);

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

	strcpy(p, "logs\\");
	mkdir(path);

	strcpy(p, "logs\\Sentry.log");
	ZQ::common::FileLog SentryLogger(path, traceLevel, 1024*1024*10);

	SentryLogger.setVerbosity(traceLevel);
	SentryLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "==== %s; Version %d.%d.%d.%d"), ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	ZQ::common::setGlogger(&SentryLogger);


	
	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
    printf("Start Sentry at \"%s\"\n", endpoint.c_str());

	::ZQ::common::NativeThreadPool threadpool;

	::ZQTianShan::Sentry::SentryEnv nenv(SentryLogger, threadpool, ic);

	::ZqSentryIce::AdapterCollectorPtr adapterCollector = new ZQTianShan::Sentry::AdapterCollectorImpl(nenv);
    nenv._loopbackAdapter->activate();
	
	::ZqSentryIce::SentryServicePtr nodeService = new ZQTianShan::Sentry::SentryServiceImpl(nenv);
    nenv._adapter->activate();

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
		printf("\rSentry is now listening %c", chs[chi]);
		Sleep(200);
	}

	printf("\rSentry is quiting                   ");
	ic->destroy();
	Sleep(100);
	
	SentryLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceShutDown, "================"));

	ZQ::common::setGlogger(NULL);

	printf("\rSentry stopped                    \n");
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

