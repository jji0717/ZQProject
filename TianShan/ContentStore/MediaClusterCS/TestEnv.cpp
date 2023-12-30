

#include "TestEnv.h"
#include "IceLog.h"
#include "MCCSCfg.h"
#include "FileLog.h"

#define TESTDIR		"C:\\TEST\\MCCS\\" 

bool TestEnv::bQuit = false;
std::auto_ptr<TestEnv> TestEnv::env;

BOOL WINAPI TestEnv::ConsoleHandler(DWORD CEvent)
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

//only one instance
TestEnv* TestEnv::getInstance()
{
	if (!env.get())
	{
		env.reset(new TestEnv());			
	}
	return env.get();
}

void TestEnv::deleteInstance()
{
	if (env.get())
	{
		env.reset();
	}
}

TestEnv::TestEnv()
{
	//
	// log
	//
	{
		log.reset(new FileLog( TESTDIR "MCCS.log", Log::L_DEBUG, 1024*1024*10));
		eventlog.reset(new FileLog(TESTDIR "MCCS_events.log", Log::L_INFO, 1024*1024*5));
		setGlogger(log.get());
	}

	//
	// load configuration
	//
	{
		configGroup.setLogger(log.get());		
		std::string filename = "c:\\Tianshan\\etc\\MediaClusterCS.xml";
		if(!configGroup.load(filename.c_str())) 
		{
			MOLOG(Log::L_ERROR, CLOGFMT(MCCS, "failed to load configuration from (%s)"), filename.c_str());
			return;
		}
		configGroup.snmpRegister("");
		std::string strNetId = "";
		configGroup.setContentStoreNetId(strNetId);
	}

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
			icelog.reset(new FileLog(TESTDIR "MCCS.IceTrace.log",
				configGroup.iceTraceLevel,
				ZQLOG_DEFAULT_FILENUM,
				configGroup.iceTraceSize));

			iceInitData.logger = new TianShanIce::common::IceLogI(icelog.get());
		}

		ic=Ice::initialize(i,NULL,iceInitData);

		try
		{
			//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_Weiwoo, endpoint);		
			adapter = ZQADAPTER_CREATE(ic, "MediaClusterCS", DEFAULT_ENDPOINT_ContentStore, *log.get());
		}
		catch(Ice::Exception& ex)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(MCCS, "Create adapter failed with endpoint=%s and exception is %s"),
				DEFAULT_ENDPOINT_ContentStore, ex.ice_name().c_str());
			return;
		}
	}


	threadpool.resize(configGroup.mccsConfig.workerThreadSize);

	store = new ::ZQTianShan::ContentStore::ContentStoreImpl(*log.get(), *eventlog.get(), threadpool, adapter, configGroup.dbPath.c_str());
	store->_replicaGroupId = configGroup.mccsConfig.csStrReplicaGroupId;
	store->_replicaId = configGroup.mccsConfig.csStrReplicaId;
	store->_replicaPriority = configGroup.mccsConfig.csIReplicaPriority;

	std::string volName = "MediaClusterCS";
	std::string volPath =  volName + "/";
	try
	{
		store->mountStoreVolume(volName.c_str(),volPath.c_str(), true);
	}
	catch(...)
	{
		printf("mount store volume name (%s) path (%s) catch an exception\n",volName.c_str(),volPath.c_str());
		return;
	}

	adapter->activate();
}

TestEnv::~TestEnv()
{
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
	{
		printf("Unable to install Ctrl+C handler!                      \n");
	}
	else
	{
		printf("\"Ctrl-C\" at any time to exit the program.              \n");

		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rMCCS is now listening %c", chs[chi]);
			Sleep(200);
		}
	}

	ZQ::common::setGlogger(NULL);
	adapter->destroy();
	ic->destroy();
	store = NULL;		
}
