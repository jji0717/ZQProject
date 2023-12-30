// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.


#include "StdAfx.h"
#include "InfoCollector.h"


#include "getopt.h"
BOOL WINAPI ConsoleHandler(DWORD CEvent);
void usage();
static bool bQuit = false;

#define MAX_SRC_OUT  20

void usage()
{
	printf("Usage: InfoCol -c <configuration file>\n");
	printf("       InfoCol -l [log file name, not specified will log on screen]\n");
	printf("       InfoCol -h\n");
	printf("Collects specific log messages from clog files.\n");
	printf("options:\n");
	printf("\t-c   specify the log file name\n");
	printf("\t-h   display this help\n");
	printf("\nFor INI config file information, look at sample.ini, keys defined in keydefine.h\n");
}

#include "Log.h"
#include "ScLog.h"
using namespace ZQ::common;

int main(int argc, char* argv[])
{
	std::string configfile;
	std::string logfile;	

	if (argc <2)
	{
		usage();
		exit(0);
	}

	int ch;
	while((ch = getopt(argc, argv, "hl:c:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);

		case 'c':
			configfile = optarg;
			break;

		case 'l':
			logfile = optarg;
			break;
		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
		return -1;
	}

	printf("config file: %s\nlog file: %s\n", configfile.c_str(), logfile.c_str());

	ScLog*  sclog = NULL;
	if (logfile.empty())
	{

	}
	else
	{
		sclog = new ScLog(logfile.c_str(), Log::L_DEBUG, 8*1024*1024);
		pGlog = sclog; 
	}

	InfoCollector  collecter;
	if (!collecter.init(configfile.c_str()))
	{
		printf("init fail with configfile %s\n", configfile.c_str());
		return 0;
	}	

	collecter.start();

	while(!bQuit)
		::Sleep(1000);

	collecter.close();

	if (sclog)
	{
		pGlog = &NullLogger;
		sclog->flush();
		delete sclog;
		sclog = NULL;
	}

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
