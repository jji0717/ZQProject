#include "FileLog.h"
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#include <direct.h>
#endif
#include "IceLog.h"
#include "MCCSSvc.h"
#include "MCCSCfg.h"
#include "ContentImpl.h"
#include "FileSystemOp.h"
#include "NativeCS.h"

using namespace ZQ::SNMP;
#define CONFIGURATION_XML	"MediaClusterCS.xml"

using namespace ZQ::common;


#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
ZQ::common::MiniDump			g_crashDump;
#endif

ZQTianShan::ContentStore::MCCSEngineSvc g_server;
::ZQ::common::NativeThreadPool*	g_pMCCSThreadPool=NULL;
::ZQTianShan::ContentStore::ContentStoreImpl::Ptr g_pStore;

ZQADAPTER_DECLTYPE g_adapter;

Ice::CommunicatorPtr	g_ic;					/*<!-ice communicator*/	
ZQ::common::BaseZQServiceApplication *Application = &g_server;

ZQ::common::Config::Loader<ConfigGroup> configGroup(CONFIGURATION_XML);
ZQ::common::Config::ILoader		*configLoader=&configGroup;

#ifdef ZQ_OS_MSWIN
extern const char* DUMP_PATH = NULL;
#else
extern const char* DUMP_PATH;
#endif//ZQ_OS_MSWIN

#define MOLOG		glog
#define MediaClusterCS		"MediaClusterCS"	

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

MCCSEngineSvc::MCCSEngineSvc()			
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "MediaClusterCS");
	strcpy(prodname, "TianShan");
#endif
}

MCCSEngineSvc::~MCCSEngineSvc()
{

}

HRESULT MCCSEngineSvc::OnInit()
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
		MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "Can't get install root path"));
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
		MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "CrashDumpPath %s error"), strCrashDumpPath.c_str());
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
		MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "Can't create threadpool instance"));
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_CRIT,"Can't create threadpool instance ,service down");
#endif
		return S_FALSE;
	}

	std::string strNetId="";
	if(m_argc == 1 && m_argv[0]) {
		strNetId = m_argv[0];
	}

	MOLOG(Log::L_INFO, CLOGFMT(MediaClusterCS, "Content Store NetId %s"),strNetId.c_str());

	if (!configGroup.setContentStoreNetId(strNetId))
	{
		if (!strNetId.empty())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "NetId %s isn't exist"),strNetId.c_str());
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
#ifdef ZQ_OS_MSWIN
			if (_iceLogFile != NULL)
			{
				delete _iceLogFile;
				_iceLogFile= NULL;
			}

			_iceLogFile=new ZQ::common::FileLog((std::string(strLogFolder)+ servname + ".IceTrace.log").c_str(),
				configGroup.iceTraceLevel,
				ZQLOG_DEFAULT_FILENUM,
				configGroup.iceTraceSize);

			iceInitData.logger = new TianShanIce::common::IceLogI(_iceLogFile);
#else
			iceInitData.logger = new TianShanIce::common::IceLogI(m_pReporter);
#endif
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
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(MediaClusterCS, "Create g_adapter failed with endpoint=%s and exception is %s"),
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
			MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "Data db path %s error"), strDbPath.c_str());
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
			MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "Data db path %s error"), strDbPath.c_str());
#ifdef ZQ_OS_MSWIN
			logEvent(ZQ::common::Log::L_ERROR, "Data db path %s",strDbPath.c_str());
#endif
			return S_FALSE;
		}

		g_pStore = new ZQTianShan::ContentStore::ContentStoreImpl(*m_pReporter,*_EventLogFile, *g_pMCCSThreadPool, g_adapter, strDbPath.c_str());
		if (!g_pStore)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "create content store object failed."));
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
		g_pStore->_storeAggregate = 1;
		g_pStore->_contentSavePeriod = configGroup.mccsConfig.contentSavePeriod;
		g_pStore->_contentSaveSizeTrigger = configGroup.mccsConfig.contentSaveSizeTrigger;
		g_pStore->_enableInServiceCheck = configGroup.mccsConfig.enalbeInServiceCheck;
		g_pStore->_warningFreeSpacePercent = configGroup.mccsConfig.warningFreeSpacePercent;
		g_pStore->_stepFreeSpacePercent = configGroup.mccsConfig.stepFreeSpacePercent;

		int supportNativeCS = 0;
		if (supportNativeCS)
		{
			g_pStore->_hashFolderExpsn = configGroup.mccsConfig.hashFolder.folderName;
			if (!configGroup.mccsConfig.rootPath.empty())
				g_pStore->_storeAggregate = 0;
			else
				g_pStore->_storeAggregate = 1;
		}
		
		g_pStore->_storeType = "SeaChange.MediaCluster";
		if (g_pStore->_cacheable)
			g_pStore->_storeType += ".cache";

		g_pStore->_autoFileSystemSync = false;
		if (!g_pStore->initializeContentStore())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "initializeContentStore() failed"));
			return S_FALSE;
		}

		if (supportNativeCS)
		{
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
				MOLOG(Log::L_INFO, CLOGFMT(MediaClusterCS, "init() fresh native contentstore succeed"));
			}
		}
	}
	catch(Ice::Exception& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(MediaClusterCS, "create content store object failed and exception is %s"), ex.ice_name().c_str());
		return S_FALSE;
	}

	return BaseZQServiceApplication::OnInit();
}

HRESULT MCCSEngineSvc::OnStart()
{
	try
	{
		g_adapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(MediaClusterCS, "activate g_adapter caught exception: %s"), ex.ice_name().c_str());
	}

	return BaseZQServiceApplication ::OnStart();
}

HRESULT MCCSEngineSvc::OnUnInit()
{
	try
	{
		if (g_pStore)
		{
			g_pStore= NULL;
		}
	}
	catch (...)
	{
	}

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
			_iceLogFile= NULL;
		}
	}
	catch (...)
	{
		_iceLogFile= NULL;
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

HRESULT MCCSEngineSvc::OnStop()
{
	return BaseZQServiceApplication::OnStop();	
}

void MCCSEngineSvc::OnSnmpSet( const char* varName)
{
	if(0 == strcmp(varName, "default/IceTrace/level"))
	{
		if(_iceLogFile)
		{
			_iceLogFile->setVerbosity(configGroup.iceTraceLevel);
		}
	}
}
/*
const ZQ::SNMP::ModuleMIB::MIBE gTblMib_mediaClusterCS[] = {
	{".2", "mediaClusterCSApp" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2)
	{".2.1", "mediaClusterCSAttr" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1)
	{".2.1.1", "mediaClusterCS-Version" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-Version(1)
	{".2.1.10", "mediaClusterCS-configDir" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-configDir(10)
	{".2.1.11", "mediaClusterCS-netId" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-netId(11)
	{".2.1.12", "mediaClusterCS-cacheMode" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-cacheMode(12)
	{".2.1.13", "mediaClusterCS-cacheLevel" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-cacheLevel(13)
	{".2.1.14", "mediaClusterCS-threads" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-threads(14)
	{".2.1.15", "mediaClusterCS-Bind-endpoint" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-Bind-endpoint(15)
	{".2.1.16", "mediaClusterCS-Bind-dispatchSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-Bind-dispatchSize(16)
	{".2.1.17", "mediaClusterCS-Bind-dispatchMax" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-Bind-dispatchMax(17)
	{".2.1.18", "mediaClusterCS-Provision-defaultBandwidth" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-Provision-defaultBandwidth(18)
	{".2.1.19", "mediaClusterCS-Provision-trickSpeeds" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-Provision-trickSpeeds(19)
	{".2.1.2", "mediaClusterCS-SnmpLoggingMask" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-SnmpLoggingMask(2)
	{".2.1.20", "mediaClusterCS-DatabaseCache-volumeSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-DatabaseCache-volumeSize(20)
	{".2.1.21", "mediaClusterCS-DatabaseCache-contentSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-DatabaseCache-contentSize(21)
	{".2.1.22", "mediaClusterCS-DatabaseCache-contentSavePeriod" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-DatabaseCache-contentSavePeriod(22)
	{".2.1.23", "mediaClusterCS-DatabaseCache-contentSaveSizeTrigger" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-DatabaseCache-contentSaveSizeTrigger(23)
	{".2.1.24", "mediaClusterCS-CPC-Bind-endpoint" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-CPC-Bind-endpoint(24)
	{".2.1.3", "mediaClusterCS-LogDir" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-LogDir(3)
	{".2.1.4", "mediaClusterCS-KeepAliveIntervals" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-KeepAliveIntervals(4)
	{".2.1.5", "mediaClusterCS-ShutdownWaitTime" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-ShutdownWaitTime(5)
	{".2.1.6", "mediaClusterCS-LogFileSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-LogFileSize(6)
	{".2.1.7", "mediaClusterCS-LogWriteTimeOut" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-LogWriteTimeOut(7)
	{".2.1.8", "mediaClusterCS-LogBufferSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-LogBufferSize(8)
	{".2.1.9", "mediaClusterCS-LogLevel" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).mediaClusterCS(1500).mediaClusterCSApp(2).mediaClusterCSAttr(1).mediaClusterCS-LogLevel(9)
	{NULL, NULL} };
	const ZQ::SNMP::ModuleMIB::MIBE* gSvcMib_mediaClusterCS = gTblMib_mediaClusterCS; // to export
*/
void MCCSEngineSvc::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-netId",           configGroup.mccsConfig.netId,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-cacheMode",           configGroup.mccsConfig.isCacheMode,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-cacheLevel",           configGroup.mccsConfig.cacheLevel,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-threads",           configGroup.mccsConfig.workerThreadSize,  "");

	SvcMIB_ExportReadOnlyVar("mediaClusterCS-Bind-endpoint",           configGroup.mccsConfig.clusterEndpoint,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-Bind-dispatchSize",           configGroup.mccsConfig.dispatchSize,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-Bind-dispatchMax",          configGroup.mccsConfig.dispatchMax,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-Provision-defaultBandwidth",           configGroup.mccsConfig.defaultProvisionBW,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-Provision-trickSpeeds",           configGroup.mccsConfig.strTrickSpeeds,  "");

	SvcMIB_ExportReadOnlyVar("mediaClusterCS-DatabaseCache-volumeSize",           configGroup.mccsConfig.volumeEvictorSize,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-DatabaseCache-contentSize",           configGroup.mccsConfig.contentEvictorSize,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-DatabaseCache-contentSavePeriod",           configGroup.mccsConfig.contentSavePeriod,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-DatabaseCache-contentSaveSizeTrigger",           configGroup.mccsConfig.contentSaveSizeTrigger,  "");
	SvcMIB_ExportReadOnlyVar("mediaClusterCS-CPC-Bind-endpoint",           configGroup.mccsConfig.cpcEndPoint, "");
}

}}
