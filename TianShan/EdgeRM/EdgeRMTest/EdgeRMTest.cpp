// EdgeRMTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TianShanDefines.h"
#include "EdgeRM.h"
#include "AllocationOwnerImpl.h"
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "AllocThrd.h"
#include "FileLog.h"
#include "Log.h"
#include <direct.h>
#include "AllocationRequest.h"
#include "Locks.h"

using namespace TianShanIce::EdgeResource;

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

extern std::vector<TianShanIce::EdgeResource::AllocationPrx> AllAllocation;
extern ZQ::common::Mutex GlobleMutex;

int main(int argc, char* argv[])
{
	int ThreadCount = 0;
	int AllocationCount = 0;
	ThreadCount = atoi(argv[2]);
	AllocationCount = atoi(argv[3]);
	printf("ThreadCount=%d\n", ThreadCount);
	printf("AllocationCount=%d\n", AllocationCount);
	if(ThreadCount < 1 || AllocationCount < 1)
	{
		printf("Argv uncorrect!");
		return 0;
	}

	std::string programRoot = ZQTianShan::getProgramRoot();
	programRoot += FNSEPS;

	::mkdir((programRoot + "logs" FNSEPS).c_str());

	ZQ::common::FileLog EdgeRMTestLogger((programRoot + "logs" FNSEPS "EdgeRMTest.log").c_str(), ZQ::common::Log::L_DEBUG, 1024*1024*50);
	EdgeRMTestLogger.setFileSize(1024*1024*50);

	EdgeRMTestLogger.setVerbosity(ZQ::common::Log::L_DEBUG);

	int i =0;
	::Ice::InitializationData initData;
	initData.properties = Ice::createProperties(i, NULL);
	initData.properties->setProperty("Ice.Override.Timeout", "10000");

	Ice::CommunicatorPtr ic = Ice::initialize(initData);
	::Ice::ObjectAdapterPtr adapter;
	::TianShanIce::EdgeResource::AllocationOwnerPrx allocOwnerPrx;
	std::vector<AllocThrd*> allocThreads;
	DWORD sleepTime = 1000 * 60 * 2;
	char localEndPoint[256];
	snprintf(localEndPoint, sizeof(localEndPoint), "tcp -h %s -p %s", argv[4], argv[5]);
	try
	{
		adapter = ic->createObjectAdapterWithEndpoints("EdgeRMTest", (std::string)localEndPoint);
		AllocationOwnerImpl* owner =  new AllocationOwnerImpl();
	
		Ice::Identity ident;
		ident.category = "owner";
		ident.name = "AllocationOwner";

		adapter->add(owner, ident);

		allocOwnerPrx = ::TianShanIce::EdgeResource::AllocationOwnerPrx::checkedCast(adapter->createProxy(ident));

		adapter->activate();
	}
	catch(const Ice::Exception &ex)
	{
		return -1;
	}
	catch (...) {
		return -1;
	}

	try {
		EdgeResouceManagerPrx erm = EdgeResouceManagerPrx::checkedCast(ic->stringToProxy((std::string)argv[1]));
		if (!erm) {
			std::cerr << "failed connecting with (" << argv[0] << ")" << std::endl;
			return 0;
		}

		EdgeRMTestLogger(ZQ::common::Log::L_INFO, CLOGFMT(main, "connect EdgeEM [%s]"), argv[1]);

// 		for(int i = 0; i < ThreadCount; i++)
// 		{
// 			AllocThrd* pThrd = new AllocThrd(EdgeRMTestLogger, erm, allocOwnerPrx, AllocationCount, sleepTime);
// 			if(pThrd)
// 			{
// 				allocThreads.push_back(pThrd);
// 				pThrd->start();
// 			}
// 		}

		ZQ::common::NativeThreadPool* _pThreadPool = new ZQ::common::NativeThreadPool(ThreadCount);
		DWORD begintime = GetTickCount();

		Ice::Long starttimestamp = ZQTianShan::now();;

		printf("******************Start to create allocation *******************\n");
		for (unsigned int i = 0; i < AllocationCount; i ++)
		{
// 			if((i != 0) && ((i%200) == 0))
// 			{
// 				DWORD past = GetTickCount() - begintime;
// 				begintime = GetTickCount();
// 				EdgeRMTestLogger(ZQ::common::Log::L_DEBUG, CLOGFMT(main, "Sleep %d"), 1000 - past);
// 				Sleep( 1000 - past );
// 			}
			AllocationRequest* pRequest = new AllocationRequest(*_pThreadPool, EdgeRMTestLogger, erm, allocOwnerPrx);
			if (pRequest)
			{
				pRequest->start();
			}	
			else 
			{
				EdgeRMTestLogger(ZQ::common::Log::L_ERROR, CLOGFMT(main, "create purchase request failed"));
			}
		}
		printf("******************End to create allocation, took %dms *******************\n",  ZQTianShan::now() - starttimestamp);

		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
		{
			printf("Unable to install the console handler!                      \n");
			return -1;
		}

		printf("\"Ctrl-C\" at any time to exit the program.              \n");

		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rEdgeRMTest is now serving %c", chs[chi]);
			Sleep(200);
		}

// 		for(std::vector<AllocThrd*>::iterator it = allocThreads.begin(); it != allocThreads.end(); it++)
// 		{
// 			delete (AllocThrd*)(*it);
// 		}

		for(std::vector<TianShanIce::EdgeResource::AllocationPrx>::iterator it = AllAllocation.begin(); it != AllAllocation.end(); it++)
		{
			try
			{
				(*it)->destroy();
			}
			catch(const ::Ice::ObjectNotExistException&)
			{
				continue;
			}
			catch(const Ice::Exception& e) 
			{
				return 0;
			}
			catch(...)
			{
				return 0;
			}
		}

		if(_pThreadPool)
		{
			delete _pThreadPool;
			_pThreadPool = NULL;
		}
	}
	catch(const Ice::Exception& e) {
		return 0;
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