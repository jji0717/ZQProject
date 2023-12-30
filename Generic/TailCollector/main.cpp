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
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : collect the expected clog messages from multiple log files
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/TailCollector/main.cpp 1     10-11-12 15:59 Admin $
// $Log: /ZQProjs/Generic/TailCollector/main.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 3     6/24/05 3:58p Hui.shao
// 
// 2     6/24/05 2:49p Hui.shao
// 
// 1     6/24/05 2:34p Hui.shao
// ============================================================================================

#include "TailCollector.h"

#include "getopt.h"
BOOL WINAPI ConsoleHandler(DWORD CEvent);
void usage();
static bool bQuit = false;

#define MAX_SRC_OUT  20

void usage()
{
	printf("Usage: TailCollector -c <configuration file>\n");
	printf("       TailCollector -h\n");
	printf("Collects specific log messages from clog files.\n");
	printf("options:\n");
	printf("\t-c   specify the log file name\n");
	printf("\t-h   display this help\n");
	printf("\nINI config file:\n");
	printf("section [Global]:\n");
	printf("\tSourceCount      total number of the log file to monitor\n");
	printf("\tOutputFileCount  total number of output files\n");
	printf("\tOutputFile<nn>   the filename for the output, start from 01\n");
	printf("section [Source<nn>]:\n");
	printf("\tfilename         pathname of the log file to monitor\n");
	printf("\tHandlerCount     total number of handler attached to this source\n");
	printf("section [S<nn>H<mmmmm>]:\n");
	printf("\tOutputFileId     id of the output defined in [Global]\n");
	printf("\tSyntax           regular expression for matching the log messages\n");
	printf("\tOutputFormat     format of output\n");
}

int main(int argc, char* argv[])
{
	std::string configfile;

	if (argc <2)
	{
		usage();
		exit(0);
	}

	int ch;
	while((ch = getopt(argc, argv, "hc:")) != EOF)
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

	IniFile ini(configfile.c_str());

	std::string value = ini.ReadKey("global", "SourceCount");
	int sourceCount = atoi(value.c_str());

	CTailCollector tails[MAX_SRC_OUT];

	value = ini.ReadKey("global", "OutputFileCount");
	int OfileCount = atoi(value.c_str());
	if (OfileCount>MAX_SRC_OUT)
		OfileCount = MAX_SRC_OUT;
	
	FILE* Ofiles[MAX_SRC_OUT];
	memset(Ofiles, 0, sizeof(Ofiles));

	int i=0;
	for (i=1; i <= OfileCount; i++)
	{
		char buf[32];
		sprintf(buf, "OutputFile%02d", i);
		value = ini.ReadKey("global", buf);
		if( value.empty() || (Ofiles[i]  = fopen( value.c_str(), "a" )) == NULL )
		{
			printf( "could not open the output file %d: %s\n", i, value.c_str());
			Ofiles[i] = stdout;
		}
		else printf("%s: %s opened\n", buf, value.c_str());
	}
	Ofiles[0] = stdout;

	for (i=0; i<sourceCount; i++)
	{
		char sectname[16];
		sprintf(sectname, "Source%02d", i+1);
		std::string filename = ini.ReadKey(sectname, "filename");
		
		printf(">> Start monitoring %s: %s opened\n", sectname, filename.c_str());
		tails[i].setFilename(filename.c_str());

		int ec = atoi(ini.ReadKey(sectname, "handlercount").c_str());

		for (int j =1; j <= ec; j++)
		{
			sprintf(sectname, "S%02dH%04d", i+1, j);
			int outputId = atoi(ini.ReadKey(sectname, "OutputFileId").c_str());
			std::string syntax = ini.ReadKey(sectname, "Syntax");
			std::string outputfmt = ini.ReadKey(sectname, "OutputFormat");

			if (syntax.empty() || outputfmt.empty())
				continue;

			FILE* pf = (outputId<MAX_SRC_OUT && outputId>0) ? Ofiles[outputId] : Ofiles[0];

			printf("apply handler %s on to Source%02d\n", sectname, i+1);
			tails[i].addHandler(MessageHandler(pf, syntax.c_str(), outputfmt.c_str()));
		}
	}

	for (i=0; i<sourceCount; i++)
		tails[i].start();

	while(!bQuit)
		::Sleep(500);

	for (i=0; i<sourceCount; i++)
		tails[i].stop();

	::Sleep(2000);

	for (i=1; i <= OfileCount; i++)
	{
		if (Ofiles[i])
			fclose(Ofiles[i]);
		Ofiles[i] = NULL;
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
