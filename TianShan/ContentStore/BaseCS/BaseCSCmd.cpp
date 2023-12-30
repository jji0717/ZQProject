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
// Ident : $Id: BaseCSCmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/BaseCS/BaseCSCmd.cpp $
// 
// 17    1/02/14 6:05p Zonghuan.xiao
// 
// 15    7/11/13 11:31a Li.huang
// 
// 14    6/28/12 9:46a Li.huang
// 
// 13    6/20/12 2:45p Li.huang
// 
// 12    4/23/12 3:19p Hui.shao
// 
// 11    4/23/12 2:55p Hui.shao
// configuration of up/down stream nic
// 
// 10    4/16/12 4:12p Hui.shao
// 
// 9     4/13/12 1:40p Hui.shao
// 
// 8     4/13/12 1:37p Hui.shao
// test with external CDNSS at exportCotentAsStream
// 
// 7     4/11/12 11:35a Li.huang
// add linux build
// 
// 6     4/09/12 2:34p Hui.shao
// 
// 5     1/19/12 12:08p Hui.shao
// 
// 4     1/18/12 6:24p Li.huang
// 
// 3     1/18/12 1:51p Hui.shao
// linkable for CacheStore
// 
// 2     2/15/11 4:15p Fei.huang
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 7     10-04-21 11:37 Haoyuan.lu
// 
// 6     09-02-25 16:47 Yixin.tian
// 
// 5     08-12-25 18:15 Fei.huang
// + linux support
// 
// 4     08-11-03 11:37 Hui.shao
// fixed the deconstruct seq
// 
// 3     08-10-23 18:38 Hui.shao
// 
// 2     08-10-07 19:57 Hui.shao
// added volume layer
// 
// 1     08-08-14 15:14 Hui.shao
// merged from 1.7.10
// 
// 6     08-07-29 13:39 Hui.shao
// fixed the quitting bug
// 
// 5     08-07-29 12:16 Hui.shao
// 
// 4     08-07-21 10:36 Hui.shao
// check in the works of the weekend
// 
// 3     08-07-18 15:12 Hui.shao
// 
// 2     08-07-15 14:18 Hui.shao
// ===========================================================================

#include "ContentImpl.h"
#include "FileSystemOp.h"
#include "CacheStoreImpl.h"

#include "Guid.h"
#include "Log.h"
#include "ZQResource.h"
#include "FileLog.h"

#include <ctime>
#include <cstdio>

#ifdef ZQ_OS_MSWIN
extern BOOL WINAPI ConsoleHandler(DWORD event);
#else
#include <signal.h>
extern "C" void handler(int);
#endif

#define EMBEDDED_STREAMSVC_EP       "CDNSS: tcp -h 10.15.10.50 -p 10700"

extern void SleepMilli(int milliseconds);

bool bQuit = false;

#define CACHESTORE_TEST

#ifdef CACHESTORE_TEST
void CacheTests(::TianShanIce::Storage::CacheStorePrx store);
#endif // CACHESTORE_TEST


int main(int argc, char* argv[])
{
    std::string path = FS::getImagePath();
	int i=0x3300, j; 

	swab((char*) &i, (char*) &j, sizeof(j));

    size_t pos = path.find_last_of(FNSEPC);
    if (pos != std::string::npos)
	{
        path = path.substr(0, pos);

        size_t pos2 = path.find_last_of(FNSEPC);
        if(pos2 != std::string::npos && !path.compare(pos2, 4, FNSEPS"bin")) {
            path = path.substr(0, pos2); 
        }
    }

    std::ostringstream oss;
    oss << path << FNSEPS << "logs" << FNSEPS;

    FS::createDirectory(oss.str());
    std::string file = oss.str() + "BaseCS.log";

	ZQ::common::FileLog BaseCSLogger(file.c_str(), ZQ::common::Log::L_DEBUG, 1024*1024*50);

	file = oss.str() + "BaseCS_events.log";
	ZQ::common::FileLog BaseCSEventLogger(file.c_str(), ZQ::common::Log::L_INFO, 1024*1024*20, 1);

	BaseCSLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "==== %s; Version %d.%d.%d.%d"), 
                    ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	BaseCSLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "====================================================="));

	ZQ::common::setGlogger(&BaseCSLogger);

	ZQ::common::NativeThreadPool threadpool(100);
	{
		int i =0;
		Ice::InitializationData initData;
		initData.properties = Ice::createProperties(i, NULL);
		initData.properties->setProperty("Ice.ThreadPool.Server.Size","20");
		initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax","50");
		initData.properties->setProperty("Ice.ThreadPool.Client.Size","20");
		initData.properties->setProperty("Ice.ThreadPool.Client.SizeMax","50");
		Ice::CommunicatorPtr ic = Ice::initialize(initData);
		ZQADAPTER_DECLTYPE adapter;

		char * endpoint;
		try {
			if(argc > 1)
				endpoint = argv[1];
			else
				endpoint = DEFAULT_ENDPOINT_ContentStore;//default -p 10400
			adapter = ZQADAPTER_CREATE(ic, "BaseCS", endpoint, BaseCSLogger);
		}
		catch(Ice::Exception& ex) {
			BaseCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(BaseCS, 
					"Create adapter failed with endpoint=%s and exception is %s"),
                    endpoint, ex.ice_name().c_str());
			return -2;
		}

        oss.str("");
        oss << path << FNSEPS << "data" << FNSEPS << "BaseCS";

		ZQTianShan::ContentStore::ContentStoreImpl::Ptr store = 
                    new ZQTianShan::ContentStore::ContentStoreImpl(BaseCSLogger, BaseCSEventLogger, threadpool, adapter, oss.str().c_str());


#ifdef CACHESTORE_TEST

		store->_storeFlags |= STOREFLAG_checkFSonOpenContentByFullName;
		ZQTianShan::ContentStore::CacheStoreImpl::Ptr cachestore = 
			new ZQTianShan::ContentStore::CacheStoreImpl(BaseCSLogger, BaseCSEventLogger, "CacheStore", *store, NULL, threadpool);

		cachestore->_prxStreamService = ::TianShanIce::Streamer::StreamServicePrx::checkedCast(ic->stringToProxy(EMBEDDED_STREAMSVC_EP));
		cachestore->_downStreamBindIP = "192.168.80.224"; // the downstream NIC to the StreamServer
		cachestore->_upStreamBindIP   = "192.168.80.224"; // the upstream NIC to the Library
		cachestore->_totalProvisionKbps = 20000000;
		cachestore->_minBitratePercent = 20;
		ZQTianShan::ContentStore::SourceStores::StoreInfo si;

//		si.sessionInterface = "c2http://172.16.20.40:10080/";
	    si.sessionInterface = "c2http://172.16.20.40:10080/";
		cachestore->_extSourceStores.set("zq.com", si);

		si.sessionInterface = "c2http://172.16.20.42:10080/";
		cachestore->_extSourceStores.set("xor.com", si);

		si.sessionInterface = "c2http://172.12.23.54:8080/";
		cachestore->_extSourceStores.set("news[1-3].cnn.com", si);

		si.sessionInterface = "c2http://172.12.23.55:8080/";
		cachestore->_extSourceStores.set("news[4-9].cnn.com", si);

		cachestore->setAccessThreshold(300000, 4 , 30000, Ice::Current());

 //       cachestore->_flags = 1;

#endif // CACHESTORE_TEST

		if(!store->initializeContentStore())
		{
			BaseCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(BaseCS, "failed to initialize ContentStore"));
			store = NULL;
			return (-2);
		}

		std::string netId = (argc > 2) ? argv[2] : "AAA";
		store->_netId = netId;
		store->_replicaGroupId = "SEAC00000";
		BaseCSLogger(ZQ::common::Log::L_DEBUG, CLOGFMT(BaseCS, 
			"NetId = %s"), store->_netId.c_str());

		Ice::Current c;
		store->enableAutoFileSystemSync(false, c);
#ifdef ZQ_OS_MSWIN
		store->mountStoreVolume("$", "d:\\temp\\aaa\\", true);
		store->mountStoreVolume("V2", "d:\\temp\\bbb\\");

#ifdef CACHESTORE_TEST
		{
			const char* testfolders[] = {
				"/V2/abc1/cde/efg/aaa/bbb", "abc", "/abc1", "abc/cde", "/abc/cde/efg/aaa/bbb", NULL
			};

			//TianShanIce::Storage::ContentStoreExPrx cs = ::TianShanIce::Storage::ContentStoreExPrx::checkedCast(store->proxy());

			for (int f=0; testfolders[f]; f++)
				store->openFolderEx(testfolders[f], true, 0, Ice::Current());
		}


		if(!cachestore->doInit())
		{
			BaseCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(BaseCS, "failed to initialize ContentStore"));
			cachestore = NULL;
			return (-2);
		}
#endif // CACHESTORE_TEST

		if (SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
		{
			std::cerr << "Unable to install handler!" << std::endl;
			return -1;
		}
#else
		store->mountStoreVolume("V1", "/mnt/hyperfs/c2files/");
		store->mountStoreVolume("V2", "/tmp/bbb/");

        struct sigaction act;
        memset(&act, '\0', sizeof(act));
        sigemptyset(&act.sa_mask);
        sigaddset(&act.sa_mask, SIGINT);
        act.sa_handler = handler;

        if(sigaction(SIGINT, &act, 0)<0) {
			std::cerr << "Unable to install handler!" << std::endl;
			return -1;
        }
#endif
	
		std::cout << "\"Ctrl-C\" at any time to exit the program." << std::endl;

		adapter->activate();
//		store->dumpDB("V2", false, "/V2");

		BaseCSEventLogger(EventFMT(netId.c_str(), Service, Activated, 0, "service[%s] Version[%d.%d.%d.%d]"), 
                ZQ_INTERNAL_FILE_NAME, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, 
				ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);

#ifdef CACHESTORE_TEST
		CacheTests(cachestore->proxy());
#endif // CACHESTORE_TEST

        std::cout << "BaseCS is now listening ";
		while (!bQuit) {
			static const char* chs="-\\|/";
            static const char* p = chs;
            if(*p == '\0') p = chs;
            std::cout << *p++ << '\b';
            std::cout.flush();
			SleepMilli(100);
		}

		std::cout << "\rBaseCS is quiting                   " << std::endl;

#ifdef CACHESTORE_TEST
		try {
			if (cachestore)
				cachestore->doUninit();
			cachestore = NULL;
		}
		catch(Ice::Exception&) { }
		catch(...) { }
#else 
		{
			try {
				if(store)
					store->unInitializeContentStore();
				store = 0;
				ic->destroy();
			}
			catch(Ice::Exception&) { }
			catch(...) { }
		}
#endif // CACHESTORE_TEST

		BaseCSEventLogger(EventFMT(netId.c_str(), Service, Deactivated, 1, "service[%s] Version[%d.%d.%d.%d]"), 
                ZQ_INTERNAL_FILE_NAME, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, 
				ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	}

	SleepMilli(100);

	ZQ::common::setGlogger(NULL);
	std::cout << "\rBaseCS stopped                    " << std::endl;

	SleepMilli(100);

	return 0;
}

#ifdef ZQ_OS_MSWIN
BOOL WINAPI ConsoleHandler(DWORD CEvent) {
    switch(CEvent) {
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
#else
extern "C" void handler(int signal) {
    if(signal == SIGINT) {
        bQuit = true;
    }
}
#endif

void SleepMilli(int milliseconds) {
#ifdef ZQ_OS_MSWIN
    ::Sleep(milliseconds);
#else
    struct timespec spec;
    spec.tv_sec = milliseconds/1000;
    spec.tv_nsec = (milliseconds%1000)*1000*1000;
    nanosleep(&spec, 0);
#endif
}


#ifdef CACHESTORE_TEST
void CacheTests(::TianShanIce::Storage::CacheStorePrx store)
{
	if (!store)
		return;

	::TianShanIce::Storage::CacheCandidates cacheCandidates = store->getCandidatesOfContent("aadddd0001", true);
	std::string since;
	//int c = store->getAccessCount("aadddd0001", since);

	::TianShanIce::SRM::ResourceMap resources; TianShanIce::Properties params;

	// for the resource of rtTsUpstreamBandwidth
	do {
		::TianShanIce::SRM::Resource res;
		res.status   = TianShanIce::SRM::rsRequested;
		res.attr   = TianShanIce::SRM::raNonMandatoryNonNegotiable;

		{
			::TianShanIce::Variant v;
			v.type = ::TianShanIce::vtLongs;
			v.bRange = true;
			v.lints.push_back(4000000);
			v.lints.push_back(10000000);
			MAPSET(::TianShanIce::ValueMap, res.resourceData, "bandwidth", v);
		}

		{
			::TianShanIce::Variant v;
			v.type = ::TianShanIce::vtStrings;
			v.bRange = false;
			v.strs.push_back("c2http://10.11.123.12:1234/");
			MAPSET(::TianShanIce::ValueMap, res.resourceData, "sessionInterface", v);
		}

		MAPSET(::TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtTsUpstreamBandwidth, res);

	} while(0); // end of rtTsUpstreamBandwidth

	// for the resource of rtEthernetInterface
	do {
		::TianShanIce::SRM::Resource res;
		res.status   = TianShanIce::SRM::rsRequested;
		res.attr   = TianShanIce::SRM::raNonMandatoryNonNegotiable;

		{
			::TianShanIce::Variant v;
			v.type = ::TianShanIce::vtLongs;
			v.bRange = true;
			v.lints.push_back(4000000);
			v.lints.push_back(10000000);
			MAPSET(::TianShanIce::ValueMap, res.resourceData, "bandwidth", v);
		}
		MAPSET(::TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtTsDownstreamBandwidth, res);

		res.resourceData.clear();
		{
			::TianShanIce::Variant v;
			v.type = ::TianShanIce::vtStrings;
			v.bRange = false;
			v.strs.push_back("192.168.80.224");
			MAPSET(::TianShanIce::ValueMap, res.resourceData, "destIP", v);
		}

		MAPSET(::TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtEthernetInterface, res);

	} while(0); // end of rtEthernetInterface

	MAPSET(TianShanIce::Properties, params, "Range",   "0.0-");
	MAPSET(TianShanIce::Properties, params, "CDNType", "");
	MAPSET(TianShanIce::Properties, params, "TansferDeplay", "0");

	while(0)
	{
		::TianShanIce::Streamer::StreamPrx stream = store->exportContentAsStream("cdntest1234567890127zq.com", "*", 60*1000, 0, resources, params);
	}

	params.clear();
	while(0)
	{
		try {
		store->cacheContent("cdntest1234567890XXXzq.com", NULL, params);
		}
		catch(...) {}
	}

}
#endif // CACHESTORE_TEST
