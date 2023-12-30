// CVSS.cpp : 定义控制台应用程序的入口点。
//

#include "VLCVSSSvc.h"
#include "CECommon.h"
#include <MiniDump.h>

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

extern int32 iTimeOut;

ZQTianShan::VSS::VLC::VLCVSS g_server;

Ice::CommunicatorPtr	_ic;					/*<!-ice communicator*/	

ZQ::common::BaseZQServiceApplication *Application = &g_server;
::ZQ::common::NativeThreadPool*	_pThreadPool=NULL;

::ZQ::common::NativeThreadPool*	_pCSThreadPool=NULL;

char *DefaultConfigPath = "VLCVSS.xml";
const int DefaultThreadPoolSize = 30;

ZQ::common::Config::Loader< ::ZQTianShan::VSS::VLC::VLCVSSCfg > pConfig(DefaultConfigPath);

ZQ::common::MiniDump			_crashDump;

ZQ::common::Config::ILoader		*configLoader=&pConfig;

//::ZQTianShan::NSS::NGODStreamServiceImplPtr _cps;
//::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;

#define MOLOG		glog
#define VLCVSSService "VLCVSS"
#define VLCVSSProduct "TianShan"

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();	
	glog( ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
	glog.flush();
}

void initWithConfig(Ice::PropertiesPtr proper )
{
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

	for (ZQTianShan::VSS::VLC::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
	{
		proper->setProperty((*iceIter).name.c_str(), (*iceIter).value.c_str());
		MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(VLCVSS, "Set ICE Property %s=%s"), (*iceIter).name.c_str(), (*iceIter).value.c_str());			
	}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern ::std::string getErrMsg();
extern ::std::string getErrMsg(DWORD dwErrCode);
extern bool validatePath( const char *     szPath );

namespace ZQTianShan{

namespace VSS{

namespace VLC{

VLCVSS::VLCVSS()
{
	strcpy(servname, VLCVSSService);
	strcpy(prodname, VLCVSSProduct);
}

VLCVSS::~VLCVSS()
{

}

HRESULT VLCVSS::OnInit()
{
	std::string			_strConfigFile;
	std::string			_strLogFolder;
	std::string			_strPluginFolder;

	//VSS log
	_iceFileLog.open(pConfig._VLCVSSIceLog.path.c_str(),
		pConfig._VLCVSSIceLog.level,
		pConfig._VLCVSSIceLog.maxCount,
		pConfig._VLCVSSIceLog.size,
		pConfig._VLCVSSIceLog.bufferSize,
		pConfig._VLCVSSIceLog.flushTimeout);

	_fileLog.open(pConfig._logFile.path.c_str(),
		pConfig._logFile.level,
		pConfig._logFile.maxCount,
		pConfig._logFile.size,
		pConfig._logFile.bufferSize,
		pConfig._logFile.flushTimeout);

	std::string CSLogPath = pConfig._logFile.path + "_CS";
	_fileLog_CS.open(CSLogPath.c_str(),
		pConfig._logFile.level,
		pConfig._logFile.maxCount,
		pConfig._logFile.size,
		pConfig._logFile.bufferSize,
		pConfig._logFile.flushTimeout);
	CSLogPath += "Event";
	_fileLog_CSEvent.open(CSLogPath.c_str(),
		pConfig._logFile.level,
		pConfig._logFile.maxCount,
		pConfig._logFile.size,
		pConfig._logFile.bufferSize,
		pConfig._logFile.flushTimeout);

	_strConfigFile = DefaultConfigPath;

	//get the install root
	char	szProgramRootPath[2048];
	ZeroMemory(szProgramRootPath,sizeof(szProgramRootPath));
	GetModuleFileNameA(NULL, szProgramRootPath, sizeof(szProgramRootPath)-1);
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
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "Can't get install root path"));
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get install root path"));
		return S_FALSE;
	}
	m_strProgramRootPath = szProgramRootPath;

	std::string	strCrashDumpPath ;
	strCrashDumpPath = pConfig._crashDump.path;

	if (!validatePath(strCrashDumpPath.c_str()))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "CrashDumpPath %s error"), strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR, "invalid mini-dump path %s",strCrashDumpPath.c_str());
		return S_FALSE;
	}	
	_crashDump.setDumpPath((char*)strCrashDumpPath.c_str());
	_crashDump.enableFullMemoryDump(TRUE);
	_crashDump.setExceptionCB(CrashExceptionCallBack);

	_pThreadPool=new ZQ::common::NativeThreadPool();

	if (!_pThreadPool)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "Can't create VSS thread pool instance"));
		logEvent(ZQ::common::Log::L_CRIT,"Can't create thread pool instance ,service down");
		return S_FALSE;
	}
	else
	{
		_pThreadPool->resize( pConfig._telnetProp.threadPoolSize> 10 ? pConfig._telnetProp.threadPoolSize : 10 );
	}

	_pCSThreadPool=new ZQ::common::NativeThreadPool();

	if (!_pCSThreadPool)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "Can't create VSS ContentStore thread pool instance"));
		logEvent(ZQ::common::Log::L_CRIT,"Can't create thread pool instance ,service down");
		return S_FALSE;
	}
	else
	{
		_pCSThreadPool->resize( pConfig._telnetProp.threadPoolSize> 10 ? pConfig._telnetProp.threadPoolSize : 10 );
	}

	TCHAR	szBuf[512];
	DWORD dwSize=sizeof(szBuf)/sizeof(szBuf[0]);
	ZeroMemory(szBuf,sizeof(szBuf));

	_strLogFolder=m_wsLogFolder;
	if(_strLogFolder.size()<=0)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "Invalid logFolder %s"),_strLogFolder.c_str() );
		logEvent(ZQ::common::Log::L_ERROR,_T("Invalid logFolder %s"),_strLogFolder.c_str() );
		return S_FALSE;
	}
	if(!( _strLogFolder[_strLogFolder.length()-1]=='\\' || _strLogFolder[_strLogFolder.length()-1]=='/'))
		_strLogFolder+="\\";

	{
		_strPluginFolder = szProgramRootPath;
		_strPluginFolder += "modules\\";
	}

	if(_strPluginFolder.size() >0 && !( _strPluginFolder[_strPluginFolder.length()-1]=='\\' || _strPluginFolder[_strPluginFolder.length()-1]=='/'))
		_strPluginFolder+="\\";

	int i=0;

	_iceLog = new ::TianShanIce::common::IceLogI(&_iceFileLog);

#if ICE_INT_VERSION / 100 >= 303

	::Ice::InitializationData initData;
	initData.properties = Ice::createProperties(i, NULL);
	initWithConfig(initData.properties);

	initData.logger = _iceLog;

	_ic = Ice::initialize(initData);

#else
	Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);
	initWithConfig(proper);	

	_ic=Ice::initializeWithPropertiesAndLogger(i,NULL,proper, _iceLog);
#endif // ICE_INT_VERSION

	std::string	strDbPath ;
	strDbPath = pConfig._dataBase.path;


	//get telnet server config
	std::string strServerPath = pConfig._telnetServerInfo.ip;
	uint16 uServerPort = pConfig._telnetServerInfo.port;
	std::string strPassword = pConfig._telnetServerInfo.password;
	uint16 uTelnetPoolSize = pConfig._telnetProp.threadPoolSize;

	_pVSSEnv = new ZQTianShan::VSS::VLC::VLCVSSEnv(_fileLog, *_pThreadPool,
		strServerPath, uServerPort, strPassword, uTelnetPoolSize,
		_ic,
		pConfig._iceStorm.endPoint.c_str(),
		pConfig._VLCServiceProp.szServiceName, 
		pConfig._VLCServiceProp.synInterval,
		pConfig._bind.endPoint.c_str(),
		pConfig._dataBase.path.c_str(),
		pConfig._dataBase.runtimePath.c_str());

	_cps = new ::ZQTianShan::VSS::VLC::VLCStreamServiceImpl(_fileLog, _ic, pConfig._bind.evictorSize, *_pVSSEnv);
	_cps->start();

	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + VLCVSSService;
	_contentStore = new ZQTianShan::ContentStore::ContentStoreImpl(_fileLog_CS,_fileLog_CSEvent, *_pCSThreadPool, _pVSSEnv->_adapter, csDataPath.c_str());

	try{
		::std::string CSPath =  std::string("VLCCS") + FNSEPS;
		_contentStore->initializeContentStore();
		if (pConfig._volumnInfo._volumns.empty())
			_contentStore->mountStoreVolume("70001", CSPath.c_str(), true);
		else
		{
			for (::ZQTianShan::VSS::VLC::VolumnInfo::Volumns::iterator iter = pConfig._volumnInfo._volumns.begin(); iter != pConfig._volumnInfo._volumns.end(); iter++)
				_contentStore->mountStoreVolume(iter->name, iter->path);
		}
		_pVSSEnv->connectToContentStore();

		//initialize content store basic info
		//config for contentstore information
		_contentStore->_netId = pConfig._storeInfo.netId;
		if(_contentStore->_netId.length() == 0)
		{
			char chHost[256] = {0};
			gethostname(chHost,sizeof(chHost));
			_contentStore->_netId = chHost;
		}
		_contentStore->_storeType = pConfig._storeInfo.type;
		_contentStore->_streamableLength = atol(pConfig._storeInfo.streamableLength.c_str());

		_contentStore->_replicaGroupId = pConfig._storeInfo.groupId;
		_contentStore->_replicaId = pConfig._storeInfo.replicaId;
		if(_contentStore->_replicaId.length() == 0)
			_contentStore->_replicaId = _contentStore->_netId;

		_contentStore->_replicaPriority = pConfig._storeInfo.replicaPriority;
		_contentStore->_replicaTimeout = pConfig._storeInfo.timeout;
		_contentStore->_contentEvictorSize = pConfig._storeInfo.contentSize;
		_contentStore->_volumeEvictorSize = pConfig._storeInfo.volumeSize;

	}
	catch(...)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "mount store volume name (%s) path (%s) catch an exception\n"), "70001", "VLCCS");
		return -1;
	}
	MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(VLCVSS, "VLCVSS Virtual Stream Service and ContentStore Service created"));

	pConfig.snmpRegister(::std::string(""));
	return BaseZQServiceApplication::OnInit();
}
HRESULT VLCVSS::OnStart()
{	
	if(_pVSSEnv)
	{
#ifndef _INDEPENDENT_ADAPTER			
		/*		if (!_pNSSEnv->_adapter->publishLogger(pConfig._publishedLogs.path.c_str(), pConfig._publishedLogs.syntax.c_str()))				
		{				
		glog(::ZQ::common::Log::L_ERROR, "Failed to publish logger name[%s] syntax[%s]", 					
		pConfig._publishedLogs.path.c_str(), pConfig._publishedLogs.syntax.c_str());				
		}			
		else				
		{				
		glog(::ZQ::common::Log::L_INFO, "Publish logger name[%s] syntax[%s] successful", 					
		pConfig._publishedLogs.path.c_str(), pConfig._publishedLogs.syntax.c_str());				
		}	*/			
#endif

		//_pNSSEnv->start();        
	}
	else
	{
		MOLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "No enviroment is setuped"));
		return S_FALSE;
	}

	return BaseZQServiceApplication ::OnStart();	
}
HRESULT VLCVSS::OnStop()
{	
	if (_cps)
	{
		try
		{
			_cps->uninitializeService();
			_cps = NULL;
		}
		catch(...){ }		
	}

	if(_pVSSEnv)
	{
		try
		{
			delete _pVSSEnv;
			_pVSSEnv = NULL;
		}
		catch(...){ }		
	}

	if(_contentStore)
	{
		try
		{
			_contentStore->unInitializeContentStore();
			_contentStore = NULL;
		}
		catch(...){ }		
	}

	return BaseZQServiceApplication::OnStop();	
}

HRESULT VLCVSS::OnUnInit()
{
	try
	{
		_csAdapter->deactivate();
		_csAdapter = NULL;
	}
	catch (...)
	{
	}

	try
	{
		_ic->destroy();
	}
	catch (...)
	{
	}	

	if(_pThreadPool)
	{
		try
		{
			delete _pThreadPool;
			_pThreadPool = NULL;
		}
		catch(...){ }
	}

	if(_pCSThreadPool)
	{
		try
		{
			delete _pCSThreadPool;
			_pCSThreadPool=NULL;
		}
		catch(...){ }
	}

	return BaseZQServiceApplication ::OnUnInit();	
}

}//namespace VLC

}//namespace VSS

}//namespace ZQTianShan