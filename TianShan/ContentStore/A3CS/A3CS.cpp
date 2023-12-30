// A3CS.cpp : Defines the entry point for the console application.
//

#include "FileLog.h"
#include "ConfigHelper.h"
#include "NSSConfig.h"
#include "NSSCfgLoader.h"
#include "contentImpl.h"
#include <stdio.h>
#include <direct.h>

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

const char* DefaultConfigPath = "NSS.xml";
ZQ::common::Config::Loader<::ZQTianShan::NSS::NSSCfg> pConfig(DefaultConfigPath);

int main(int argc, char* argv[])
{

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

	std::string strConfig = path;

	strcpy(p, "logs\\");
	mkdir(path);

	strcpy(p, "logs\\A3CS.log");
	ZQ::common::FileLog A3CSLogger(path, ZQ::common::Log::L_DEBUG, 1024*1024*20);
	A3CSLogger(ZQ::common::Log::L_DEBUG,"==========================================A3ContentStore starting=======================================");

	
	strConfig += "etc\\";
	pConfig.setLogger(&A3CSLogger);
	bool bR = pConfig.loadInFolder(strConfig.c_str());
	if(!bR)
	{
		printf("load config (%s) failed\n",strConfig.c_str());
		return -1;
	}
/*
	strcpy(p, "logs\\");
	mkdir(path);

	strcpy(p, "logs\\A3CS.log");
	ZQ::common::FileLog A3CSLogger(path, ZQ::common::Log::L_DEBUG, 1024*1024*20);
*/
	strcpy(p, "logs\\A3CS_events.log");
	ZQ::common::FileLog A3CSEventLogger(path, ZQ::common::Log::L_INFO, 1024*1024*20);
	

	A3CSLogger.setVerbosity(ZQ::common::Log::L_DEBUG);
	ZQ::common::setGlogger(&A3CSLogger);

	int i =0;
//	Ice::InitializationData iceInitData;
//	iceInitData.properties =Ice::createProperties(i,NULL);
//	iceInitData.properties->setProperty("Ice.ThreadPool.Server.Size","5");

	
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL/*,iceInitData*/);
	
	::ZQ::common::NativeThreadPool threadpool;
	ZQADAPTER_DECLTYPE adapter;

	try
	{
		//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_Weiwoo, endpoint);		
		 adapter = ZQADAPTER_CREATE(ic, "A3CS", DEFAULT_ENDPOINT_ContentStore, A3CSLogger);
	}
	catch(Ice::Exception& ex)
	{
		A3CSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(A3CS, "Create adapter failed with endpoint=%s and exception is %s"), DEFAULT_ENDPOINT_ContentStore, ex.ice_name().c_str());
		return -2;
	}

	{
		strcpy(p, "data\\A3CS");
		::ZQTianShan::ContentStore::ContentStoreImpl::Ptr store = new ::ZQTianShan::ContentStore::ContentStoreImpl(A3CSLogger,A3CSEventLogger, threadpool, adapter, path);
		
		std::string volName = pConfig._a3VolumeInfo.name;
		std::string volPath =  pConfig._a3VolumeInfo.path + FNSEPS;
		
		if(! store->initializeContentStore())
		{
			A3CSLogger(ZQ::common::Log::L_ERROR,CLOGFMT(A3CS, "Initialize ContentStore failed"));
			return -1;
		}

		try{
			store->mountStoreVolume(volName.c_str(),volPath.c_str(),true);
			
		}
		catch(...)
		{
			printf("mount store volume name (%s) path (%s) catch an exception\n",volName.c_str(),volPath.c_str());
			return -1;
		}

		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
		{
			printf("Unable to install handler!                      \n");
			return -1;
		}

		printf("\"Ctrl-C\" at any time to exit the program.              \n");

		adapter->activate();

		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rA3CS is now listening %c", chs[chi]);
			Sleep(200);
		}

		ZQ::common::setGlogger(NULL);
		printf("\rA3CS is quiting                   ");
	
		ic->destroy();
	
		if(store != NULL)
			store->unInitializeContentStore();
	}

	ZQ::common::setGlogger(NULL);
	Sleep(1000);

	printf("\rA3CS stopped                    \n");
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
