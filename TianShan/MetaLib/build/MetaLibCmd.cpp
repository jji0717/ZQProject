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
// Ident : $Id: MetaLibCmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MetaLib/build/MetaLibCmd.cpp $
// 
// 2     1/02/14 9:58a Hui.shao
// pglog
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 4     09-06-29 16:46 Li.huang
// 
// 3     09-05-13 14:00 Haoyuan.lu
// 
// 2     09-04-02 11:15 Hui.shao
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================

#include "MetaLibImpl.h"

#include "Guid.h"
#include "Log.h"
#include "ZQResource.h"
#include "FileLog.h"

extern "C"
{
#include <time.h>
#include <stdio.h>
#include <direct.h>
}

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

int main(int argc, char* argv[])
{
	int ch;
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

	strcpy(p, "logs\\");
	mkdir(path);

	strcpy(p, "logs\\MetaLib.log");
	ZQ::common::FileLog MetaLibLogger(path, ZQ::common::Log::L_DEBUG, 1024*1024*10);

	MetaLibLogger.setVerbosity(ZQ::common::Log::L_DEBUG);
	ZQ::common::setGlogger(&MetaLibLogger);

	MetaLibLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "==== %s; Version %d.%d.%d.%d"), ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	MetaLibLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "====================================================="));

	::ZQ::common::NativeThreadPool threadpool;

	{
		int i =0;
		Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);

		strcpy(p, "data\\MetaLib");
		::ZQTianShan::MetaLib::MetaLibImpl::Ptr lib = new ::ZQTianShan::MetaLib::MetaLibImpl(MetaLibLogger, threadpool, ic, path, false);
		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE)==FALSE)
		{
			printf("Unable to install handler!                      \n");
			return -1;
		}

		::TianShanIce::Repository::MetaDataMap schema;
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.hintedType = ::TianShanIce::vtStrings;

		char buf[80];
		TianShanIce::StrValues expectedMetaDataNames;

		for (i=0; i <3; i++)
		{
			sprintf(buf, "MetaData#%02d", i);
			schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(buf, metaData));
			expectedMetaDataNames.push_back(buf);
		}

		lib->proxy()->registerMetaClass("MyClass", schema);

		ZQ::common::Guid guid;


		for (i=0; i <10000; i++)
		{
			guid.create();
			guid.toCompactIdstr(buf, sizeof(buf)-2);
			std::string objId = buf;
			metaData.value="1";

			lib->proxy()->setMetaData(objId, "MetaData#00", metaData);

			sprintf(buf, "val#%d", 2);
			metaData.value = buf;
			lib->proxy()->setMetaData(objId, META_DATA "#02", metaData);
		}
// 

		TianShanIce::Properties searchForMetaData;
		searchForMetaData.insert(TianShanIce::Properties::value_type("MetaData#00", "1"));
		searchForMetaData.insert(TianShanIce::Properties::value_type("MetaData#02", "val#2"));
//		searchForMetaData.insert(TianShanIce::Properties::value_type("MetaData#17", "val#17"));
//		searchForMetaData.insert(TianShanIce::Properties::value_type("MetaData#02", "val#1234"));
	    int count = 0;		
		TianShanIce::Repository::MetaObjectInfos ret = lib->proxy()->lookup("", searchForMetaData, expectedMetaDataNames);
	
		for (TianShanIce::Repository::MetaObjectInfos::iterator itObj = ret.begin(); itObj < ret.end(); itObj++)
		{
			printf("found obj[%s:%s]\n", itObj->id.c_str(), itObj->type.c_str());
			for (TianShanIce::Repository::MetaDataMap::iterator itMd = itObj->metaDatas.begin(); itMd != itObj->metaDatas.end(); itMd++)
				printf("\t%s = %s\n", itMd->first.c_str(), itMd->second.value.c_str());
			count++;
		}
        
		printf("count %d\n", count);
		printf("\"Ctrl-C\" at any time to exit the program.              \n");
		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rMetaLib is now listening %c", chs[chi]);
			Sleep(200);
		}

		lib = NULL;
		printf("\rMetaLib is quiting...                         ");
		ic->destroy();
	}

	Sleep(100);
	ZQ::common::setGlogger(NULL);
	Sleep(100);

	printf("\rMetaLib stopped                    \n");
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
