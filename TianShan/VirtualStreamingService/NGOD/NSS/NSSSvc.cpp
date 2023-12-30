#include <Log.h>
#include <filelog.h>
#include <NativeThread.h>
#include <NativeThreadPool.h>

#include "NSSSvc.h"
#include "NSSConfig.h"
#include "NSSCfgLoader.h"
#include "NSSImpl.h"
#include "CECommon.h"
#include <MiniDump.h>

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

extern int32 iTimeOut;

ZQTianShan::NSS::NSSSvc g_server;

Ice::CommunicatorPtr	_ic;					/*<!-ice communicator*/	

ZQ::common::BaseZQServiceApplication *Application = &g_server;
::ZQ::common::NativeThreadPool*	_pNSSThreadPool=NULL;
::ZQ::common::NativeThreadPool*	_pA3CSThreadPool=NULL;
::ZQTianShan::NSS::NSSBaseConfig::NSSHolder *pNSSBaseConfig = NULL;

char *DefaultConfigPath = "NSS.xml";
const int DefaultThreadPoolSize = 30;

//::ZQTianShan::NSS::NSSConfig pConfig(DefaultConfigPath);
ZQ::common::Config::Loader< ::ZQTianShan::NSS::NSSCfg > pConfig(DefaultConfigPath);

ZQ::common::MiniDump			_crashDump;

ZQ::common::Config::ILoader		*configLoader=&pConfig;

//::ZQTianShan::NSS::NGODStreamServiceImplPtr _cps;
//::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;

#define MOLOG		glog
#define NSSService	"NSS"

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	glog( ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
	glog.flush();
}

void initWithConfig(Ice::PropertiesPtr proper , ::ZQTianShan::NSS::IceProperties &configProper)
{
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

	for (::ZQTianShan::NSS::IceProperties::props::iterator iceIter = configProper._props.begin();
		iceIter != configProper._props.end(); iceIter++)
	{
		proper->setProperty((*iceIter).name.c_str(), (*iceIter).value.c_str());
		MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(NSSSvc, "Set ICE Property %s=%s"), (*iceIter).name.c_str(), (*iceIter).value.c_str());			
	}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern ::std::string getErrMsg();
extern ::std::string getErrMsg(DWORD dwErrCode);
extern bool validatePath( const char *     szPath );

namespace ZQTianShan{

namespace NSS{

NSSSvc::NSSSvc()
{
	//strcpy(servname, "NSS");
	//strcpy(prodname, "TianShan");
}

NSSSvc::~NSSSvc()
{

}

HRESULT NSSSvc::OnInit()
{
	int32 iLogLevel			= m_dwLogLevel;
	int32 iLogMaxCount		= m_dwLogFileCount;
	int32 iLogSize			= m_dwLogFileSize;
	int32 iLogBufferSize	= m_dwLogBufferSize;
	int32 iLogFlushTimeout	= m_dwLogWriteTimeOut;

	::std::string strLogBaseFolder = m_wsLogFolder;

	//construct log folder
	::std::string strNSSLog		= strLogBaseFolder + FNSEPS + servname + ".log";
	::std::string strIceLog		= strLogBaseFolder + FNSEPS + servname + "_ice.log";
	::std::string strA3Log		= strLogBaseFolder + FNSEPS + servname + "_A3.log";
	::std::string strA3EventLog = strLogBaseFolder + FNSEPS + servname + "_A3Event.log";

	//NSS log
	_iceFileLog.open(strIceLog.c_str(), iLogLevel, iLogMaxCount, iLogSize, iLogBufferSize, iLogFlushTimeout);

	_fileLog.open(strNSSLog.c_str(), iLogLevel, iLogMaxCount, iLogSize, iLogBufferSize, iLogFlushTimeout);

	//A3 log
	_csFileLog.open(strA3Log.c_str(), iLogLevel, iLogMaxCount, iLogSize, iLogBufferSize, iLogFlushTimeout);

	_csEventFileLog.open(strA3EventLog.c_str(), iLogLevel, iLogMaxCount, iLogSize, iLogBufferSize, iLogFlushTimeout);

	//get regedit argument
	std::string strNetId = "";
	if(m_argc == 1 && strlen(m_argv[0]))
	{
		strNetId = m_argv[0];
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSSvc, "netID='%s'"), strNetId.c_str());
	
	for (::ZQTianShan::NSS::NSSBaseConfig::NSSHolderVec::iterator iter = pConfig._nssBaseConfig.NSSVec.begin(); iter != pConfig._nssBaseConfig.NSSVec.end(); iter++)
	{
		if (strNetId == (*iter).netId)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSSvc, "find NetID=%s"), strNetId.c_str());
			pNSSBaseConfig = &(*iter);
		}
	}
	if (NULL == pNSSBaseConfig)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "couldn't find specified netID(%s)"), strNetId.c_str());
		return S_FALSE;
	}
	iTimeOut = pNSSBaseConfig->_videoServer.SessionInterfaceRequestTimeout;
	pNSSBaseConfig->_videoServer.parseSpeedStr(pNSSBaseConfig->_videoServer.FixedSpeedSetForward, pNSSBaseConfig->_videoServer.FixedSpeedSetForwardSet);
	pNSSBaseConfig->_videoServer.parseSpeedStr(pNSSBaseConfig->_videoServer.FixedSpeedSetBackward, pNSSBaseConfig->_videoServer.FixedSpeedSetBackwardSet);

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
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "Can't get install root path"));
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get install root path"));
		return S_FALSE;
	}
	m_strProgramRootPath = szProgramRootPath;
	
	std::string	strCrashDumpPath ;
	strCrashDumpPath = pConfig._crashDump.path;

	if (!validatePath(strCrashDumpPath.c_str()))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "CrashDumpPath %s error"), strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR, "invalid mini-dump path %s",strCrashDumpPath.c_str());
		return S_FALSE;
	}	
	_crashDump.setDumpPath((char*)strCrashDumpPath.c_str());
	_crashDump.enableFullMemoryDump(TRUE);
	_crashDump.setExceptionCB(CrashExceptionCallBack);

	_pNSSThreadPool=new ZQ::common::NativeThreadPool();

	if (!_pNSSThreadPool)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "Can't create NSS thread pool instance"));
		logEvent(ZQ::common::Log::L_CRIT,"Can't create thread pool instance ,service down");
		return S_FALSE;
	}

	if (_pNSSThreadPool) 
	{
		_pNSSThreadPool->resize( pNSSBaseConfig->_bind.threadPoolSize> 10 ? pNSSBaseConfig->_bind.threadPoolSize : 10 );
	}

	
	TCHAR	szBuf[512];
	DWORD dwSize=sizeof(szBuf)/sizeof(szBuf[0]);
	ZeroMemory(szBuf,sizeof(szBuf));
	
	_strLogFolder=m_wsLogFolder;
	if(_strLogFolder.size()<=0)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "Invalid logFolder %s"),_strLogFolder.c_str() );
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
	initWithConfig(initData.properties, pNSSBaseConfig->_iceProperty);

	initData.logger = _iceLog;

	_ic = Ice::initialize(initData);

#else
	Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);
	initWithConfig(proper, pNSSBaseConfig->_iceProperty);	

	_ic=Ice::initializeWithPropertiesAndLogger(i,NULL,proper, _iceLog);
#endif // ICE_INT_VERSION
	
	::std::string strDbPath = pConfig._dataBase.path;
	::std::string strDbRuntimePath = pConfig._dataBase.runtimePath;
	
	//get media server config
	::std::string strServerPath;
	uint16 uServerPort;
	

	strServerPath = pNSSBaseConfig->_videoServer.SessionInterfaceIp.c_str();
	uServerPort = pNSSBaseConfig->_videoServer.SessionInterfacePort;

	_pNSSEnv = new ::ZQTianShan::NSS::NSSEnv(_fileLog, *_pNSSThreadPool, _ic,
											 pConfig._iceStorm.endPoint.c_str(),
											 pNSSBaseConfig->_bind.endPoint.c_str(),
										 	 strDbPath.c_str(),
											 strDbRuntimePath.c_str());
	if (_pNSSEnv)
	{
		for (::ZQTianShan::NSS::PublishedLogs::PublishLogHolderVec::iterator iter = pNSSBaseConfig->_publishedLog._logDatas.begin(); iter != pNSSBaseConfig->_publishedLog._logDatas.end(); iter++)
		{
			if (!_pNSSEnv->_adapter->publishLogger(iter->_path.c_str(), iter->_syntax.c_str(), iter->_key.c_str()))	
			{				
				glog(::ZQ::common::Log::L_ERROR, "Failed to publish logger name[%s] syntax[%s] key[%s]", 			
					iter->_path.c_str(), iter->_syntax.c_str(), iter->_key.c_str());		
			}
			else				
			{				
				glog(::ZQ::common::Log::L_INFO, "Publish logger name[%s] syntax[%s] key[%s] successful", 			
					iter->_path.c_str(), iter->_syntax.c_str(), iter->_key.c_str());	
			}
		}
	}
	
	int32 iMaxSessionGroupNumer = pNSSBaseConfig->_videoServer.SessionInterfaceMaxSessionGroup;
	int32 iMaxSessionPerGroup = pNSSBaseConfig->_videoServer.SessionInterfaceMaxSessionsPerGroup;;
	//initialize sessiongroup
	if (iMaxSessionGroupNumer < 1)
	{
		glog(::ZQ::common::Log::L_WARNING, "invalide MasSessionGroup number(%d), reset to 2", iMaxSessionGroupNumer);			
	}
	for (int i = 0; i < iMaxSessionGroupNumer; i++)
	{
		SessionGroup sessionGroup;
		sessionGroup.maxSession = iMaxSessionPerGroup;
		::std::stringstream ss;
		ss << (i+1);
		sessionGroup.name = strNetId + "." + ss.str();
		_sessionGroup.push_back(sessionGroup);
	}

	_cps = new ::ZQTianShan::NSS::NGODStreamServiceImpl(_fileLog, *_pNSSThreadPool, _ic, strServerPath, 
		uServerPort,_sessionGroup, pNSSBaseConfig->_bind.threadPoolSize, *_pNSSEnv);
	//add for A3 content store
	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + NSSService;
	_contentStore = new ::ZQTianShan::ContentStore::ContentStoreImpl(_csFileLog,_csEventFileLog, *_pNSSThreadPool, _pNSSEnv->_adapter, csDataPath.c_str());

	::std::string A3CSPath;
	::std::string mountPath;
	try{
		_contentStore->initializeContentStore();
		for (int i = 0; i < pNSSBaseConfig->_videoServer.vols.size(); i++)
		{
			A3CSPath = pNSSBaseConfig->_videoServer.vols[i].targetName + FNSEPS;
			bool bDefaultVolume = false;
			if (pNSSBaseConfig->_videoServer.vols[i].defaultVal == 1)
				bDefaultVolume = true;
			mountPath = pNSSBaseConfig->_videoServer.vols[i].mount;
			_contentStore->mountStoreVolume(mountPath.c_str(), A3CSPath.c_str(), bDefaultVolume);
		}
	}
	catch(...)
	{
		_csFileLog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "mount store volume name (%s) path (%s) catch an exception\n"), mountPath.c_str(), A3CSPath.c_str());
		//printf("fail to mount Store Volume!\n");
		return -1;
	}

	MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(NSSSvc, "NGOD Stream Service and A3ContentStore Service created"));

	pConfig.snmpRegister(::std::string(""));
	return BaseZQServiceApplication::OnInit();
}
HRESULT NSSSvc::OnStart()
{	
	if(_pNSSEnv)
	{
//#ifndef _INDEPENDENT_ADAPTER			
		_pNSSEnv->_adapter->activate();
//#endif

		//_pNSSEnv->start();        
	}
	else
	{
		MOLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "No enviroment is setuped and adapter fail to activate"));
		return S_FALSE;
	}

	return BaseZQServiceApplication ::OnStart();	
}
HRESULT NSSSvc::OnStop()
{	
	if (_cps)
	{
		try
		{
			_cps = NULL;
		}
		catch(...){ }		
	}

	if(_pNSSEnv)
	{
		try
		{
			delete _pNSSEnv;
			_pNSSEnv = NULL;
		}
		catch(...){ }		
	}

	if(_contentStore)
	{
		try
		{
			//delete _contentStore;
			_contentStore->unInitializeContentStore();
			_contentStore = NULL;
		}
		catch(...){ }		
	}

	return BaseZQServiceApplication::OnStop();	
}

HRESULT NSSSvc::OnUnInit()
{
	try
	{
		_csAdapter->deactivate();
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

	if(_pNSSThreadPool)
	{
		try
		{
			delete _pNSSThreadPool;
			_pNSSThreadPool=NULL;
		}
		catch(...){ }
	}

	if(_pA3CSThreadPool)
	{
		try
		{
			delete _pA3CSThreadPool;
			_pA3CSThreadPool=NULL;
		}
		catch(...){ }
	}
	
	return BaseZQServiceApplication ::OnUnInit();	
}

}

}