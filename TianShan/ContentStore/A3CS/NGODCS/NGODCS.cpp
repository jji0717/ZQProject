// NGODCS.cpp : Defines the entry point for the console application.
//

#include "NGODCSEnv.h"
#include <stdio.h>
#include <direct.h>


BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

const char* DefaultConfigPath = "NSS.xml";

ZQ::common::Config::Loader< ::ZQTianShan::NSS::NSSCfg > pConfig(DefaultConfigPath);

ZQTianShan::NSS::NSSBaseConfig::NSSHolder *pNSSBaseConfig = NULL; ;



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

	ZQTianShan::ContentStore::NGODCSEnv	ngodCSEnv;
	ngodCSEnv.setServiceName("NSS");
	ngodCSEnv.setLogPath(path);
	
	strConfig += "etc\\";

	bool bR = pConfig.loadInFolder(strConfig.c_str());
	if(!bR)
	{
		printf("load config (%s) failed\n",strConfig.c_str());
		return -1;
	}

	std::string strNetId = "ZQ_OSTR";
	for (::ZQTianShan::NSS::NSSBaseConfig::NSSHolderVec::iterator iter = pConfig._nssBaseConfig.NSSVec.begin(); iter != pConfig._nssBaseConfig.NSSVec.end(); iter++)
	{
		if (strNetId == (*iter).netId)
		{
			printf("find NetID=%s in NSS.xml\n", strNetId.c_str());
			pNSSBaseConfig = &(*iter);
		}
	}
	if (NULL == pNSSBaseConfig)
	{
		printf("couldn't find specified netID(%s) in NSS.xml\n", strNetId.c_str());
		return -1;
	}
	ngodCSEnv.setConfig(pNSSBaseConfig);

	int i =0;
	Ice::InitializationData iceInitData;
	iceInitData.properties =Ice::createProperties(i,NULL);
	iceInitData.properties->setProperty("Ice.ThreadPool.Server.Size","200");

	
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL,iceInitData);
	
	::ZQ::common::NativeThreadPool threadpool;
	ZQADAPTER_DECLTYPE adapter;

	std::string endpoint;
	endpoint = (!pNSSBaseConfig->_bind.endPoint.empty()) ? pNSSBaseConfig->_bind.endPoint : DEFAULT_ENDPOINT_NSS;

	try
	{
		//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_Weiwoo, endpoint);		
		adapter = ZQADAPTER_CREATE(ic, "NGODCS", endpoint.c_str(), glog);
	}
	catch(Ice::Exception& ex)
	{
		printf("Create adapter failed with endpoint=%s and exception is %s", DEFAULT_ENDPOINT_ContentStore, ex.ice_name().c_str());
		return -2;
	}

	ngodCSEnv.setIceAdapter(adapter);
	ngodCSEnv.setDataPath(pConfig._dataBase.path.c_str());
	ngodCSEnv.setThreadPool(&threadpool);
	if (!ngodCSEnv.initEnv())
	{
		printf("Failed to init NGODCSEnv\n");
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
		printf("\rNGODCS is now listening %c", chs[chi]);
		Sleep(200);
	}

	ZQ::common::setGlogger(NULL);
	printf("\rNGODCS is quiting                   \n");
		
	adapter->deactivate();

	ngodCSEnv.uninitEnv();

	printf("store uninitialized\n");


	Sleep(1000);
	try{
		ic->destroy();
	}
	catch(...)
	{
		printf("Ice destroy catch exception\n");
	}

	printf("\rNGODCS stopped                    \n");
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

