#include "FileLog.h"
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#include <direct.h>
#endif
#include "IceLog.h"

#include "CDNCSSvc.h"
#include "MCCSCfg.h"
#include "ContentImpl.h"
#include "FileSystemOp.h"
#include "NativeCS.h"


#define CONFIGURATION_XML	"CDNCS.xml"

using namespace ZQ::common;


#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
ZQ::common::MiniDump			g_crashDump;
#endif

ZQTianShan::ContentStore::CDNCSEngineSvc g_server;
::ZQ::common::NativeThreadPool*	g_pMCCSThreadPool=NULL;
::ZQTianShan::ContentStore::ContentStoreImpl::Ptr g_pStore;

ZQADAPTER_DECLTYPE g_adapter;

Ice::CommunicatorPtr	g_ic;					/*<!-ice communicator*/	

ZQ::common::BaseZQServiceApplication *Application = &g_server;

Config::Loader<ConfigGroup> configGroup(CONFIGURATION_XML);
ZQ::common::Config::ILoader		*configLoader=&configGroup;

#ifdef ZQ_OS_MSWIN
extern const char* DUMP_PATH = NULL;
#else
extern const char* DUMP_PATH;
#endif//ZQ_OS_MSWIN

#define MOLOG		glog
#define CDNCS		"CDNCS"	

#ifdef ZQ_OS_MSWIN
bool validatePath  ( const char * wszPath )
{
	if (-1 != ::GetFileAttributes(wszPath))
		return true;

	DWORD dwErr = ::GetLastError();
	if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
	{
		if (!::CreateDirectory(wszPath, NULL))
		{
			dwErr = ::GetLastError();
			if ( dwErr != ERROR_ALREADY_EXISTS)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();	
	glog( ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
	glog.flush();
}
#endif


/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


namespace ZQTianShan{
	namespace ContentStore{

CDNCSEngineSvc::CDNCSEngineSvc()			
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "CDNCS");
	strcpy(prodname, "TianShan");
#endif
}

CDNCSEngineSvc::~CDNCSEngineSvc()
{

}

HRESULT CDNCSEngineSvc::OnInit()
{
	DUMP_PATH = configGroup.dumpPath.c_str();
	std::string			_strConfigFile;
	std::string			_strLogFolder;
	_strConfigFile=configGroup.getConfigFilePath();

#ifdef ZQ_OS_MSWIN
	//get the install root
	char	szProgramRootPath[2048];
	ZeroMemory(szProgramRootPath,sizeof(szProgramRootPath));
	GetModuleFileNameA(NULL,szProgramRootPath,sizeof(szProgramRootPath)-1);
	int iLens = strlen(szProgramRootPath)-1;
	bool bSecondToken = false;
	while ( iLens > 0 )
	{
		if (szProgramRootPath[iLens]=='\\') 
		{
			if (bSecondToken) 
			{
				szProgramRootPath[iLens+1]='\0';
				break;
			}
			else
			{
				bSecondToken =true;
			}			
		}
		iLens--;
	}

	if (iLens <= 0) 
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "Can't get install root path"));
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get install root path"));
		return S_FALSE;
	}
	m_strProgramRootPath = szProgramRootPath;

	std::string	strCrashDumpPath ;
	if (strstr(configGroup.dumpPath.c_str(),":")!=NULL) 
	{
		strCrashDumpPath = configGroup.dumpPath;
	}
	else
	{
		strCrashDumpPath=szProgramRootPath;
		strCrashDumpPath+=configGroup.dumpPath;		
	}

	if (!validatePath(strCrashDumpPath.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "CrashDumpPath %s error"), strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR, "invalid minidump path %s",strCrashDumpPath.c_str());
		return S_FALSE;
	}	
	g_crashDump.setDumpPath((char*)strCrashDumpPath.c_str());
	g_crashDump.enableFullMemoryDump(configGroup.dumpEnabled);
	g_crashDump.setExceptionCB(CrashExceptionCallBack);
	g_crashDump.setServiceName(m_sServiceName);

	std::string strLogFolder= m_wsLogFolder;
#else
	std::string strLogFolder= _logDir;
#endif
	if (strLogFolder.at(strLogFolder.length()-1)!=FNSEPC) 
	{
		strLogFolder+=FNSEPS;
	}

#ifdef ZQ_OS_MSWIN
	std::string logName = std::string(strLogFolder)+ servname + "_events.log";
#else
	std::string logName = std::string(strLogFolder)+ getServiceName() + "_events.log";
#endif
	_EventLogFile=new ZQ::common::FileLog( logName.c_str(),
		configGroup.iceTraceLevel ,
		ZQLOG_DEFAULT_FILENUM,
		configGroup.iceTraceSize);

	g_pMCCSThreadPool=new ZQ::common::NativeThreadPool();
	if ( !g_pMCCSThreadPool) 
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "Can't create threadpool instance"));
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_CRIT,"Can't create threadpool instance ,service down");
#endif
		return S_FALSE;
	}

	std::string strNetId="";
	if(m_argc == 1 && m_argv[0]) {
		strNetId = m_argv[0];
	}

	MOLOG(Log::L_INFO, CLOGFMT(CDNCS, "Content Store NetId %s"),strNetId.c_str());

	if (!configGroup.setContentStoreNetId(strNetId))
	{
		if (!strNetId.empty())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "NetId %s isn't exist"),strNetId.c_str());
#ifdef ZQ_OS_MSWIN
			logEvent(ZQ::common::Log::L_CRIT,"NetId %s isn't exist",strNetId.c_str());
#endif
		}
		return S_FALSE;
	}

	{
		int i=0;
		Ice::InitializationData iceInitData;
		iceInitData.properties =Ice::createProperties(i,NULL);
		iceInitData.properties->setProperty("Ice.ThreadPool.Client.Size","5");
		iceInitData.properties->setProperty("Ice.ThreadPool.Client.SizeMax","10");
		iceInitData.properties->setProperty("Ice.ThreadPool.Server.Size","5");
		iceInitData.properties->setProperty("Ice.ThreadPool.Server.SizeMax","10");

		std :: map<std::string, std::string>::const_iterator it = configGroup.mccsConfig.iceProp.begin();
		for( ; it != configGroup.mccsConfig.iceProp.end(); it++ )
		{		
			iceInitData.properties->setProperty( it->first , it->second );
		}

		///initialize ice communicator
		if( configGroup.iceTraceEnabled)
		{

			if (_iceLogFile != NULL)
			{
				delete _iceLogFile;
				_iceLogFile= NULL;
			}
#ifdef ZQ_OS_MSWIN
			std::string iceLogName = std::string(strLogFolder)+ servname + ".IceTrace.log";
#else
			std::string iceLogName = std::string(strLogFolder)+ getServiceName() + ".IceTrace.log";
#endif//ZQ_OS_MSWIN
			_iceLogFile = new ZQ::common::FileLog(iceLogName.c_str(), configGroup.iceTraceLevel, ZQLOG_DEFAULT_FILENUM, configGroup.iceTraceSize);

			iceInitData.logger = new TianShanIce::common::IceLogI(_iceLogFile);
			//iceInitData.logger = new TianShanIce::common::IceLogI(m_pReporter);
		}

		g_ic=Ice::initialize(i,NULL,iceInitData);
	}

	g_pMCCSThreadPool->resize(configGroup.mccsConfig.workerThreadSize);

	try
	{	
		g_adapter = ZQADAPTER_CREATE(g_ic, ADAPTER_NAME_ContentStore, configGroup.mccsConfig.clusterEndpoint.c_str(), *m_pReporter);
	}
	catch(Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CDNCS, "Create g_adapter failed with endpoint=%s and exception is %s"),
			configGroup.mccsConfig.clusterEndpoint.c_str(), ex.ice_name().c_str());
		return S_FALSE;
	}


	//
	// publish log to sentry
	//
	std::vector<MonitoredLog>::iterator iter;
	for (iter = configGroup.mccsConfig.monitoredLogs.begin(); iter != configGroup.mccsConfig.monitoredLogs.end(); ++iter) 
	{			
		if (!g_adapter->publishLogger(iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str()))				
		{				
			MOLOG(ZQ::common::Log::L_ERROR, "Failed to publish logger name[%s] synax[%s] key[%s]", 					
				iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str());				
		}			
		else				
		{				
			MOLOG(ZQ::common::Log::L_INFO, "Publish logger name[%s] synax[%s] key[%s] successful", 					
				iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str());				
		}			
	}	

	try
	{
		std::string	strDbPath ;
		strDbPath = configGroup.dbPath;
		if (!FS::createDirectory(strDbPath))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "Data db path %s error"), strDbPath.c_str());
#ifdef ZQ_OS_MSWIN
			logEvent(ZQ::common::Log::L_ERROR, "Data db path %s",strDbPath.c_str());
#endif
			return S_FALSE;
		}

		if (FNSEPC != strDbPath[strDbPath.length()-1])
			strDbPath += FNSEPS;

#ifdef ZQ_OS_MSWIN
		strDbPath = strDbPath + servname;
#else
		strDbPath = strDbPath + getServiceName();
#endif
		if (!FS::createDirectory(strDbPath))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "Data db path %s error"), strDbPath.c_str());
#ifdef ZQ_OS_MSWIN
			logEvent(ZQ::common::Log::L_ERROR, "Data db path %s",strDbPath.c_str());
#endif
			return S_FALSE;
		}

		g_pStore = new ZQTianShan::ContentStore::ContentStoreImpl(*m_pReporter,*_EventLogFile, *g_pMCCSThreadPool, g_adapter, strDbPath.c_str());
		if (!g_pStore)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "create content store object failed."));
			return S_FALSE;
		}

		g_pStore->_replicaGroupId = configGroup.mccsConfig.csStrReplicaGroupId;
		g_pStore->_replicaId = configGroup.mccsConfig.csStrReplicaId;
		g_pStore->_replicaPriority = configGroup.mccsConfig.csIReplicaPriority;
		g_pStore->_netId = configGroup.mccsConfig.netId;
		g_pStore->_contentEvictorSize = configGroup.mccsConfig.contentEvictorSize;
		g_pStore->_volumeEvictorSize = configGroup.mccsConfig.volumeEvictorSize;
		g_pStore->_replicaTimeout = configGroup.mccsConfig.csReplicaTimeout;
		g_pStore->_cacheable = configGroup.mccsConfig.isCacheMode;
		g_pStore->_cacheLevel = configGroup.mccsConfig.cacheLevel & 0xff;
		g_pStore->_exposeStreamService = 0;
		g_pStore->_contentSavePeriod = configGroup.mccsConfig.contentSavePeriod;
		g_pStore->_contentSaveSizeTrigger = configGroup.mccsConfig.contentSaveSizeTrigger;
		g_pStore->_storeAggregate = 1;
		g_pStore->_enableInServiceCheck = configGroup.mccsConfig.enalbeInServiceCheck;
		g_pStore->_timeoutNotProvisioned = configGroup.mccsConfig.timeoutNotProvisioned;
		g_pStore->_timeoutIdleProvisioning = configGroup.mccsConfig.timeoutIdleProvisioning;

		g_pStore->_hashFolderExpsn = configGroup.mccsConfig.hashFolder.folderName;
		if (!configGroup.mccsConfig.rootPath.empty())
			g_pStore->_storeAggregate = 0;
		else
			g_pStore->_storeAggregate = 1;

		g_pStore->_storeType = "SeaChange.MediaCluster";
		if (g_pStore->_cacheable)
			g_pStore->_storeType += ".cache";

		g_pStore->_autoFileSystemSync = false;
		if (!g_pStore->initializeContentStore())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "initializeContentStore() failed"));
			return S_FALSE;
		}

		if(!configGroup.mccsConfig.cpcEndPoint.empty() && !configGroup.mccsConfig.rootPath.empty()) 
		{
			std::string rootPath;
			size_t slashPos     = configGroup.mccsConfig.rootPath.rfind("/");
			size_t altSlashPos  = configGroup.mccsConfig.rootPath.rfind("\\");
			size_t lastNotSpace = configGroup.mccsConfig.rootPath.find_last_not_of("  ");
			if (std::string::npos != slashPos && slashPos == lastNotSpace)
				rootPath = configGroup.mccsConfig.rootPath;
			else if (std::string::npos != altSlashPos && altSlashPos == lastNotSpace)
				rootPath = configGroup.mccsConfig.rootPath;
			else 
				rootPath = configGroup.mccsConfig.rootPath + "/";

			std::ostringstream oss;
			oss << SERVICE_NAME_ContentStore << ":" << configGroup.mccsConfig.cpcEndPoint;
			g_pStore->subscribeStoreReplica(oss.str(), true); 
			TianShanIce::Replicas exported = g_pStore->exportStoreReplicas();
			g_pStore->updateStoreReplicas(exported);
			g_pStore->mountStoreVolume("$", rootPath, true);//as native default cs
			MOLOG(Log::L_INFO, CLOGFMT(CDNCS, "init() fresh native contentstore succeed"));
		}
	}
	catch(Ice::Exception& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "create content store object failed and exception is %s"), ex.ice_name().c_str());
		return S_FALSE;
	}

	return BaseZQServiceApplication::OnInit();
}

HRESULT CDNCSEngineSvc::OnStart()
{
	try
	{
		g_adapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CDNCS, "activate g_adapter caught exception: %s"), ex.ice_name().c_str());
	}

	return BaseZQServiceApplication ::OnStart();
}

HRESULT CDNCSEngineSvc::OnUnInit()
{

	try
	{				
		g_adapter->destroy();
		g_ic->shutdown();
		g_ic->destroy();
	}
	catch (...)
	{
	}	

	
	try
	{
		if (_iceLogFile)
		{
			delete _iceLogFile;
			_iceLogFile = NULL;
		}
	}
	catch (...)
	{
		_iceLogFile = NULL;
	}
	
	try
	{
		if (_EventLogFile)
		{
			delete _EventLogFile;
			_EventLogFile= NULL;
		}
	}
	catch (...)
	{
		_EventLogFile= NULL;
	}

	if(g_pMCCSThreadPool)
	{
		try
		{
			delete g_pMCCSThreadPool;
			g_pMCCSThreadPool=NULL;
		}
		catch(...){ }
	}

	return BaseZQServiceApplication ::OnUnInit();	
}

HRESULT CDNCSEngineSvc::OnStop()
{
	try
	{		
		g_adapter->deactivate();
	}
	catch( const Ice::Exception& ex)
	{
		MOLOG( ZQ::common::Log::L_ERROR,CLOGFMT(CDNCS,"caught execption during deactivating adapter: %s"),
			ex.ice_name().c_str());
	}
	try
	{
		if( g_pStore )
		{
			g_pStore->unInitializeContentStore();
		}
	}
	catch( const TianShanIce::BaseException& ex)
	{
		MOLOG( ZQ::common::Log::L_ERROR,CLOGFMT(CDNCS,"caught execption during unInitializeContentStore: %s"),
			ex.message.c_str());
	}
	catch( const Ice::Exception& ex)
	{
		MOLOG( ZQ::common::Log::L_ERROR,CLOGFMT(CDNCS,"caught execption during unInitializeContentStore: %s"),
			ex.ice_name().c_str());
	}
	try
	{
		if (g_pStore)
		{
			g_pStore= NULL;
		}
	}
	catch (const TianShanIce::BaseException& ex)
	{
		MOLOG( ZQ::common::Log::L_ERROR,CLOGFMT(CDNCS,"caught execption during deleting svc instance: %s"),
			ex.message.c_str());
	}
	catch( const Ice::Exception& ex)
	{
		MOLOG( ZQ::common::Log::L_ERROR,CLOGFMT(CDNCS,"caught execption during deleting svc instance: %s"),
			ex.ice_name().c_str());
	}

	return BaseZQServiceApplication::OnStop();	
}

void CDNCSEngineSvc::OnSnmpSet( const char* varName)
{
	if(0 == strcmp(varName, "default/IceTrace/level"))
	{
		if(_iceLogFile)
		{
			_iceLogFile->setVerbosity(configGroup.iceTraceLevel);
		}
	}
}

}}
