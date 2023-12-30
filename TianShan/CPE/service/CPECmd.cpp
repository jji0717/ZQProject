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
// Ident : $Id: CPECmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/service/CPECmd.cpp $
// 
// 3     12/31/13 7:18p Hui.shao
// pGlog
// 
// 2     12/12/13 1:48p Hui.shao
// %lld
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 9     08-03-27 16:50 Jie.zhang
// 
// 10    08-03-17 19:36 Jie.zhang
// 
// 9     08-03-17 18:41 Jie.zhang
// 
// 8     08-03-04 18:12 Jie.zhang
// 
// 8     08-03-04 17:23 Jie.zhang
// 
// 7     08-02-28 16:27 Jie.zhang
// 
// 6     08-02-20 16:36 Jie.zhang
// 
// 5     08-02-18 18:42 Jie.zhang
// changes check in
// 
// 4     08-02-15 12:35 Jie.zhang
// changes check in 
// 
// 3     08-02-14 18:49 Hui.shao
// 
// 2     08-02-14 12:16 Hui.shao
// 
// 1     08-02-13 17:49 Hui.shao
// initial check in
// ===========================================================================

#include "CPEEnv.h"
#include "CPEImpl.h"
#include "ProvisionState.h"

#include "Guid.h"
#include "Log.h"
#include "getopt.h"
#include "ZQResource.h"
#include "FileLog.h"
#include "CPECfg.h"
#include "CPHInc.h"

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
	printf("Usage: CPE [-e \"<endpoint>\"]\n");
	printf("       CPE -h\n");
	printf("CPE console mode server demo.\n");
	printf("options:\n");
	printf("\t-e   the local endpoint to bind, default %d\n", DEFAULT_ENDPOINT_CPE);
	printf("\t-h   display this help\n");
}

// -----------------------------
// class ProvisionSessImpl
// -----------------------------
class ProvisionSessionBindImpl : public TianShanIce::ContentProvision::ProvisionSessionBind
{
public:
	typedef ::IceInternal::Handle< ProvisionSessionBindImpl> Ptr;

	ProvisionSessionBindImpl() {}

protected:
    virtual void OnProvisionStateChanged(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, ::TianShanIce::ContentProvision::ProvisionState prevState, ::TianShanIce::ContentProvision::ProvisionState currentState, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
	{
		char buf[64];
		printf("\n%s {content:%s; storage:%s; volume:%s} prevState: %s(%d); currentState:%s(%d)", ZQTianShan::TimeToUTC(timeStamp, buf, sizeof(buf)-2),
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(),
			ZQTianShan::CPE::ProvisionStateBase::stateStr(prevState), prevState, ZQTianShan::CPE::ProvisionStateBase::stateStr(currentState), currentState);
	}

    virtual void OnProvisionProgress(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, ::Ice::Long processed, ::Ice::Long total, const ::TianShanIce::Properties&params, const ::Ice::Current& c)
	{
		char buf[64];
		printf("\n%s {content:%s; storage:%s; volume:%s} progress: %lld of %lld", ZQTianShan::TimeToUTC(timeStamp, buf, sizeof(buf)-2),
			contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(),
			processed, total);
	}

    virtual void OnProvisionStreamable(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, bool streamable, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
	{
	}

    virtual void OnProvisionDestroyed(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, ::Ice::Long timeStamp, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
	{
	}
    virtual void OnProvisionStarted(const ::TianShanIce::ContentProvision::ProvisionContentKey&, ::Ice::Long, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current())
	{
	}
	
    virtual void OnProvisionStopped(const ::TianShanIce::ContentProvision::ProvisionContentKey&, ::Ice::Long, bool, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current())
	{
	}	
};

int main(int argc, char* argv[])
{
	int ch;
	std::string endpoint = DEFAULT_ENDPOINT_CPE, epIceStorm = "default -h 192.168.80.49 -p 10000";
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

	_gCPECfg.setConfigFileName("CPE.xml");
	_gCPECfg.loadWithConfigFolder("c:\\cfg");

	strcpy(p, "logs\\");
	mkdir(path);

	strcpy(p, "logs\\CPE.log");
	ZQ::common::FileLog CPELogger(path, traceLevel, 1024*1024*10);

	CPELogger.setVerbosity(traceLevel);
	CPELogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "==== %s; Version %d.%d.%d.%d"), ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	ZQ::common::setGlogger(&CPELogger);

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
	endpoint=_gCPECfg._cpeEndPoint;
    printf("Start CPE at \"%s\"\n", endpoint.c_str());

	::ZQ::common::NativeThreadPool threadpool;

	::ZQTianShan::CPE::CPEEnv env(CPELogger, threadpool, ic);

	TianShanIce::ContentProvision::ContentProvisionServicePtr cps = new ZQTianShan::CPE::CPEImpl(env);
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}

    env._adapter->activate();

/// test

	try {
		Ice::CommunicatorPtr ic2 = Ice::initialize(i, NULL);
		ProvisionSessionBindImpl::Ptr bind = new ProvisionSessionBindImpl();
		::Ice::ObjectAdapterPtr testAdapter = ic2->createObjectAdapterWithEndpoints("testAdapter", "default -p 11111");
		Ice::Identity identBind;
		identBind.name = identBind.category = "testBind";
		testAdapter->add(bind, identBind);
		testAdapter->activate();
		TianShanIce::ContentProvision::ProvisionSessionBindPrx bindPrx
			= ::TianShanIce::ContentProvision::ProvisionSessionBindPrx::checkedCast(testAdapter->createProxy(identBind));

		std::string strContentProvisionSvc = std::string(SERVICE_NAME_ContentProvisionService ":") + _gCPECfg._cpeEndPoint;
		TianShanIce::ContentProvision::ContentProvisionServicePrx cpePrx = TianShanIce::ContentProvision::ContentProvisionServicePrx::checkedCast(ic2->stringToProxy(strContentProvisionSvc));
		TianShanIce::ContentProvision::ProvisionSessionPrx sess;
		if (!cpePrx)
			return 1000;
		
		::Ice::Long c = ZQTianShan::now();
		char buf[32];

//#define TEST_RDS

#ifdef TEST_RDS
		{
			TianShanIce::ContentProvision::ProvisionSessionPrx sess;
			sprintf(buf, "%lld", c++);
			TianShanIce::ContentProvision::ProvisionContentKey contentKey;
			contentKey.content = buf;
			contentKey.contentStoreNetId = "rds";
			sess = cpePrx->createSession(contentKey, METHODTYPE_RDSVSVSTRM, TianShanIce::ContentProvision::potDirect, NULL, bindPrx);
			char ttt[256];
			sprintf(ttt, "ftp://192.168.81.100:21/rds/%s", buf);
			printf("push url: [%s]\n", ttt);

			TianShanIce::ValueMap resVars;
			TianShanIce::Variant	var;
			var.type = TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.clear();
			var.strs.push_back(contentKey.content);
			resVars[CPHPM_FILENAME] = var;
			var.strs.clear();
			sess->addResource(::TianShanIce::SRM::rtURI, resVars);
			
			resVars.clear();		
			var.type = TianShanIce::vtLongs;
			var.lints.push_back(2000000); // 2Mbps
			resVars[RDSPARAM_BANDWIDTH] = var;
			sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
			
			::Ice::Long n = ZQTianShan::now() + 10*1000;	//delay 10 seconds
			std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
			::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
			
			std::string endTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now()+1200000, buf, sizeof(buf) -2);
			
			sess->setup(startTimeUTC, endTimeUTC);
			sess->commit();

		}
#endif

		
#if 0
		sprintf(buf, "%lld", c++);
		TianShanIce::ContentProvision::ProvisionContentKey contentKey;
		contentKey.content = buf;
		contentKey.contentStoreNetId = "dummy_storage";
		sess = cpePrx->createSession(contentKey, "CopyDemo", TianShanIce::ContentProvision::potDirect, NULL, bindPrx);

		//#define FEED
		
#ifdef FEED
		for(int j=0;j < 10; j++)
		{
			sprintf(buf, "content_%08d", c++);
			contentKey.content = buf;
			sess = cpePrx->createSession(contentKey, "CopyDemo", TianShanIce::ContentProvision::potDirect, NULL, bindPrx);
		}
#endif

		TianShanIce::ValueMap resVars;
		TianShanIce::Variant	var;
		var.type = TianShanIce::vtStrings;
		var.bRange = false;
		var.strs.clear();
		var.strs.push_back("file://./D:/temp/AutoCAD2000.rar");
		resVars["sourceUri"] = var;
		var.strs.clear();
		var.strs.push_back("file://./D:/temp/AutoCAD2002.rar");
		resVars["destUri"] = var;
		var.strs.clear();
		sess->addResource(::TianShanIce::SRM::rtURI, resVars);
		
		resVars.clear();
		var.lints.push_back(2000000); // 2Mbps
		resVars["bandwidth"] = var;
		sess->addResource(::TianShanIce::SRM::rtProvisionBandwidth, resVars);
		
		::Ice::Long n = ZQTianShan::now();
		std::string startTimeUTC = ZQTianShan::TimeToUTC(n, buf, sizeof(buf) -2);
		::Ice::Long n2 = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());
		
		std::string endTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now()+120000, buf, sizeof(buf) -2);

		::TianShanIce::ContentProvision::ProvisionSubscribeMask mask = {1, true, true, true, true, true};

		sess->setSubscribeMask(mask);
		
		sess->setup(startTimeUTC, endTimeUTC);
		sess->commit();
#endif

	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		printf("caught exception[%s]: %s\n", ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		printf("caught exception[%s]\n", ex.ice_name().c_str());
	}


/// end of test

	printf("\"Ctrl-C\" at any time to exit the program.              \n");
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		printf("\rCPE is now listening %c", chs[chi]);
		Sleep(200);
	}

	printf("\rCPE is quiting                   ");
	ic->destroy();
	Sleep(1000);
	
	CPELogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceShutDown, "================"));

	ZQ::common::setGlogger(NULL);

	printf("\rCPE stopped                    \n");
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
