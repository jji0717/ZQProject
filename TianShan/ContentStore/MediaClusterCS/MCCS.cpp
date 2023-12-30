// MCCS.cpp : Defines the entry point for the console application.
//

#include "FileLog.h"
#include "contentImpl.h"
#include <stdio.h>
#include <direct.h>
#include "MCCSCfg.h"
#include "IceLog.h"

#define MOLOG	(glog)


BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

int main(int argc, char* argv[])
{
	//	int ch;
	if (argc <2)
	{
		printf("MCCS <configuration xml> \n");
		return 0;
	}

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

	strcpy(p, "logs\\MCCS.log");
	ZQ::common::FileLog MCCSLogger(path, ZQ::common::Log::L_DEBUG, 1024*1024*20);

	strcpy(p, "logs\\MCCS_events.log");
	ZQ::common::FileLog MCCSEventLogger(path, ZQ::common::Log::L_INFO, 1024*1024*20);

	MCCSLogger.setVerbosity(ZQ::common::Log::L_DEBUG);
	//	MCCSLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "==== %s; Version %d.%d.%d.%d"), ZQ_FILE_DESCRIPTION, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH,ZQ_PRODUCT_VER_BUILD);
	//	MCCSLogger(ZQ::common::Log::L_CRIT, CLOGFMT(ServiceStart, "====================================================="));
	ZQ::common::setGlogger(&MCCSLogger);

	/*
	*	initialize config loader.
	*/

	configGroup.setLogger(&MCCSEventLogger);		
	std::string filename = argv[1];
	if(!configGroup.load(filename.c_str())) {
//		MOLOG(Log::L_ERROR, CLOGFMT(CPC, "failed to load configuration from (%s)"), filename.c_str());
//		return false;
		MCCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(MCCS, "failed to load configuration from (%s)"), filename.c_str());
	
		return 0;
	}
	configGroup.snmpRegister("");

	std::string strNetId = "";
	if (argc>=3)
		strNetId = argv[2];
	configGroup.setContentStoreNetId(strNetId);


	Ice::CommunicatorPtr ic;
	std::auto_ptr<Log> icelog;

	::ZQ::common::NativeThreadPool threadpool;
	ZQADAPTER_DECLTYPE adapter;

	//
	// initialize ice environment 
	//
	{
		int i=0;
		Ice::InitializationData iceInitData;
		iceInitData.properties =Ice::createProperties(i,NULL);
		iceInitData.properties->setProperty("Ice.ThreadPool.Client.Size","5");
		iceInitData.properties->setProperty("Ice.ThreadPool.Client.SizeMax","10");
		iceInitData.properties->setProperty("Ice.ThreadPool.Server.Size","5");
		iceInitData.properties->setProperty("Ice.ThreadPool.Server.SizeMax","10");
		//			iceInitData.properties->setProperty("PLInfoStore.ThreadPool.Size",gStreamSmithConfig.szAdapterThreadpoolSize);
		//			iceInitData.properties->setProperty("PLInfoStore.ThreadPool.SizeMax",gStreamSmithConfig.szAdapterThreadpoolSizeMax);

		std :: map<std::string, std::string>::const_iterator it = configGroup.mccsConfig.iceProp.begin();
		for( ; it != configGroup.mccsConfig.iceProp.end(); it++ )
		{		
			iceInitData.properties->setProperty( it->first , it->second );
		}

		///initialize ice communicator

		if( configGroup.iceTraceEnabled)
		{	
			strcpy(p, "logs\\MCCS.IceTrace.log");
			icelog.reset(new FileLog(path,
				configGroup.iceTraceLevel,
				ZQLOG_DEFAULT_FILENUM,
				configGroup.iceTraceSize));

			iceInitData.logger = new TianShanIce::common::IceLogI(icelog.get());
		}

		ic=Ice::initialize(i,NULL,iceInitData);
	}

	threadpool.resize(configGroup.mccsConfig.workerThreadSize);

	try
	{
		//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_Weiwoo, endpoint);		
		adapter = ZQADAPTER_CREATE(ic, ADAPTER_NAME_ContentStore, configGroup.mccsConfig.clusterEndpoint.c_str(), MCCSLogger);
	}
	catch(Ice::Exception& ex)
	{
		MCCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(MCCS, "Create adapter failed with endpoint=%s and exception is %s"),
			configGroup.mccsConfig.clusterEndpoint.c_str(), ex.ice_name().c_str());
		return -2;
	}

	{
		strcpy(p, "data\\MCCS");
		::ZQTianShan::ContentStore::ContentStoreImpl::Ptr store = new ::ZQTianShan::ContentStore::ContentStoreImpl(MCCSLogger,MCCSEventLogger, threadpool, adapter, path);
		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
		{
			printf("Unable to install handler!                      \n");
			return -1;
		}

		store->_replicaGroupId = configGroup.mccsConfig.csStrReplicaGroupId;
		store->_replicaId = configGroup.mccsConfig.csStrReplicaId;
		store->_replicaPriority = configGroup.mccsConfig.csIReplicaPriority;
		store->_netId = configGroup.mccsConfig.netId;
		store->_contentEvictorSize = configGroup.mccsConfig.contentEvictorSize;
		store->_volumeEvictorSize = configGroup.mccsConfig.volumeEvictorSize;

/*		std::string volName = "MediaClusterCS";
		std::string volPath = "";
		try
		{
			store->mountStoreVolume(volName.c_str(),volPath.c_str(), true);
		}
		catch(...)
		{
			printf("mount store volume name (%s) path (%s) catch an exception\n",volName.c_str(),volPath.c_str());
			return 0;
		}
*/
		printf("\"Ctrl-C\" at any time to exit the program.              \n");

		adapter->activate();

		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rMCCS is now listening %c", chs[chi]);
			Sleep(200);
		}

		printf("\rMCCS is quiting                   ");
		ic->shutdown();
		ic->destroy();
		Sleep(1000);
	}

	ZQ::common::setGlogger(NULL);

	printf("\rMCCS stopped                    \n");
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




