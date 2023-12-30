// CVSS.cpp : 定义控制台应用程序的入口点。
//

#include "HSN.h"
#include "CECommon.h"
#include <MiniDump.h>

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

extern int32 iTimeOut;

ZQTianShan::VSS::TM::HSN g_server;

Ice::CommunicatorPtr	_ic;					/*<!-ice communicator*/	

ZQ::common::BaseZQServiceApplication *Application = &g_server;
::ZQ::common::NativeThreadPool*	_pThreadPool=NULL;

::ZQ::common::NativeThreadPool*	_pCSThreadPool=NULL;

char *DefaultConfigPath = "TMVSS.xml";
const int DefaultThreadPoolSize = 30;

ZQ::common::Config::Loader< ::ZQTianShan::VSS::TM::TMVSSCfg > pConfig(DefaultConfigPath);

ZQ::common::MiniDump			_crashDump;

ZQ::common::Config::ILoader		*configLoader=&pConfig;

//::ZQTianShan::NSS::NGODStreamServiceImplPtr _cps;
//::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;

#define MOLOG		glog
#define HSNVSSService "TMVSS"
#define HSNVSSProduct "TianShan"

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

	for (ZQTianShan::VSS::TM::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
	{
		proper->setProperty((*iceIter).name.c_str(), (*iceIter).value.c_str());
		MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(HSN, "Set ICE Property %s=%s"), (*iceIter).name.c_str(), (*iceIter).value.c_str());			
	}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern ::std::string getErrMsg();
extern ::std::string getErrMsg(DWORD dwErrCode);
extern bool validatePath( const char *     szPath );

namespace ZQTianShan{

namespace VSS{

namespace TM{

HSN::HSN()
{
	strcpy(servname, HSNVSSService);
	strcpy(prodname, HSNVSSProduct);
}

HSN::~HSN()
{

}

HRESULT HSN::OnInit()
{
	std::string			_strConfigFile;
	std::string			_strLogFolder;
	std::string			_strPluginFolder;

	//VSS log
	_iceFileLog.open(pConfig._tmvssIceLog.path.c_str(),
		pConfig._tmvssIceLog.level,
		pConfig._tmvssIceLog.maxCount,
		pConfig._tmvssIceLog.size,
		pConfig._tmvssIceLog.bufferSize,
		pConfig._tmvssIceLog.flushTimeout);

	_fileLog.open(pConfig._logFile.path.c_str(),
		pConfig._logFile.level,
		pConfig._logFile.maxCount,
		pConfig._logFile.size,
		pConfig._logFile.bufferSize,
		pConfig._logFile.flushTimeout);

	//contentstore log
	/*_csFileLog.open(pConfig._soapLog.path.c_str(),
		pConfig._soapLog.level,
		pConfig._soapLog.maxCount,
		pConfig._soapLog.size,
		pConfig._soapLog.buffer,
		pConfig._soapLog.flushTimeout);*/

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
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "Can't get install root path"));
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get install root path"));
		return S_FALSE;
	}
	m_strProgramRootPath = szProgramRootPath;

	std::string	strCrashDumpPath ;
	strCrashDumpPath = pConfig._crashDump.path;

	if (!validatePath(strCrashDumpPath.c_str()))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "CrashDumpPath %s error"), strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR, "invalid mini-dump path %s",strCrashDumpPath.c_str());
		return S_FALSE;
	}	
	_crashDump.setDumpPath((char*)strCrashDumpPath.c_str());
	_crashDump.enableFullMemoryDump(TRUE);
	_crashDump.setExceptionCB(CrashExceptionCallBack);

	_pThreadPool=new ZQ::common::NativeThreadPool();

	if (!_pThreadPool)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "Can't create VSS thread pool instance"));
		logEvent(ZQ::common::Log::L_CRIT,"Can't create thread pool instance ,service down");
		return S_FALSE;
	}
	else
	{
		_pThreadPool->resize( pConfig._rtspProp.threadPoolSize> 10 ? pConfig._rtspProp.threadPoolSize : 10 );
	}

	_pCSThreadPool=new ZQ::common::NativeThreadPool();

	if (!_pCSThreadPool)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "Can't create VSS ContentStore thread pool instance"));
		logEvent(ZQ::common::Log::L_CRIT,"Can't create thread pool instance ,service down");
		return S_FALSE;
	}
	else
	{
		_pCSThreadPool->resize( pConfig._rtspProp.threadPoolSize> 10 ? pConfig._rtspProp.threadPoolSize : 10 );
	}

	TCHAR	szBuf[512];
	DWORD dwSize=sizeof(szBuf)/sizeof(szBuf[0]);
	ZeroMemory(szBuf,sizeof(szBuf));

	_strLogFolder=m_wsLogFolder;
	if(_strLogFolder.size()<=0)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "Invalid logFolder %s"),_strLogFolder.c_str() );
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


	//get media server config
	std::string strServerPath = pConfig._soapServerInfo.ip;
	uint16 uServerPort = pConfig._soapServerInfo.port;
	std::string strLocalPath = pConfig._localInfo.ip;
	uint16 uLocalPort = pConfig._localInfo.port;


	_pEnv = new ::ZQTianShan::VSS::TM::TMVSSEnv(_fileLog, *_pThreadPool, 
												strServerPath, uServerPort,
												strLocalPath, uLocalPort,
												_ic,
												pConfig._iceStorm.endPoint.c_str(),
												pConfig._bind.endPoint.c_str(),
												pConfig._dataBase.path.c_str(),
												pConfig._dataBase.runtimePath.c_str());

	_cps = new ::ZQTianShan::VSS::TM::TMVStreamServiceImpl(_fileLog, _ic, strServerPath,uServerPort, strLocalPath, uLocalPort, pConfig._bind.evictorSize, *_pEnv);
	_cps->start();

	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + HSNVSSService;
	_contentStore = new ::ZQTianShan::ContentStore::ContentStoreImpl(_csFileLog,_csEventFileLog, *_pThreadPool, _pEnv->_adapter, csDataPath.c_str());

	try{
		::std::string CSPath =  std::string("HSNCS") + FNSEPS;
		_contentStore->mountStoreVolume("70001", CSPath.c_str(), true);
	}
	catch(...)
	{
		_csFileLog(ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "mount store volume name (%s) path (%s) catch an exception\n"), "70001", "HSNCS");
		return -1;
	}

	MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(HSN, "HSN Virtual Stream Service and ContentStore Service created"));

	pConfig.snmpRegister(::std::string(""));
	return BaseZQServiceApplication::OnInit();
}
HRESULT HSN::OnStart()
{	
	if(_pEnv)
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
		MOLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "No enviroment is setuped"));
		return S_FALSE;
	}

	return BaseZQServiceApplication ::OnStart();	
}
HRESULT HSN::OnStop()
{	
	if (_cps)
	{
		try
		{
			_cps = NULL;
		}
		catch(...){ }		
	}

	if(_pEnv)
	{
		try
		{
			delete _pEnv;
			_pEnv = NULL;
		}
		catch(...){ }		
	}

	if(_contentStore)
	{
		try
		{
			_contentStore = NULL;
		}
		catch(...){ }		
	}

	return BaseZQServiceApplication::OnStop();	
}

HRESULT HSN::OnUnInit()
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

}//namespace TM

}//namespace VSS

}//namespace ZQTianShan