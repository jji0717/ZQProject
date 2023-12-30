// CVSS.cpp : 定义控制台应用程序的入口点。
//

#include "CVSService.h"
#include "CVSSCfgLoader.h"
#include "CiscoAIMSoap11Impl.h"
#include "filelog.h"
#include <NativeThread.h>
#include <NativeThreadPool.h>
#include "CECommon.h"
#include <MiniDump.h>

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

extern int32 iTimeOut;

ZQTianShan::CVSS::CVSSvc g_server;

Ice::CommunicatorPtr	_ic;					/*<!-ice communicator*/	

ZQ::common::BaseZQServiceApplication *Application = &g_server;
::ZQ::common::NativeThreadPool*	_pCVSSThreadPool=NULL;
::ZQ::common::NativeThreadPool*	_pA3CSThreadPool=NULL;

char *DefaultConfigPath = "CVSS.xml";
const int DefaultThreadPoolSize = 30;
int32 iDefaultBufferSize = 16*1024;

//::ZQTianShan::NSS::NSSConfig pConfig(DefaultConfigPath);
ZQ::common::Config::Loader< ::ZQTianShan::CVSS::CVSSCfg > pConfig(DefaultConfigPath);

ZQ::common::MiniDump			_crashDump;

ZQ::common::Config::ILoader		*configLoader=&pConfig;

//::ZQTianShan::NSS::NGODStreamServiceImplPtr _cps;
//::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;

#define MOLOG		glog
#define CVSSService	"CVSS"
#define CVSSProduct "TianShan"

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

	for (ZQTianShan::CVSS::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
	{
		proper->setProperty((*iceIter).name.c_str(), (*iceIter).value.c_str());
		MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(CVSSvc, "Set ICE Property %s=%s"), (*iceIter).name.c_str(), (*iceIter).value.c_str());			
	}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
extern ::std::string getErrMsg();
extern ::std::string getErrMsg(DWORD dwErrCode);
extern bool validatePath( const char *     szPath );

namespace ZQTianShan{

namespace CVSS{

CVSSvc::CVSSvc()
{
	strcpy(servname, CVSSService);
	strcpy(prodname, CVSSProduct);
}

CVSSvc::~CVSSvc()
{

}

HRESULT CVSSvc::OnInit()
{
	std::string			_strConfigFile;
	std::string			_strLogFolder;
	std::string			_strPluginFolder;

	//CVSS log
	_iceFileLog.open(pConfig._iceLog.path.c_str(),
		pConfig._iceLog.level,
		pConfig._iceLog.logNum,
		pConfig._iceLog.size,
		pConfig._iceLog.buffer,
		pConfig._iceLog.flushTimeout);

	_fileLog.open(pConfig._logFile.path.c_str(),
		pConfig._logFile.level,
		pConfig._logFile.maxCount,
		pConfig._logFile.size,
		pConfig._logFile.bufferSize,
		pConfig._logFile.flushTimeout);

	//A3 log
	_csFileLog.open(pConfig._soapLog.path.c_str(),
		pConfig._soapLog.level,
		pConfig._soapLog.logNum,
		pConfig._soapLog.size,
		pConfig._soapLog.buffer,
		pConfig._soapLog.flushTimeout);

	iDefaultBufferSize = pConfig._rtspProp.bufferMaxSize * 1024;
	iTimeOut = pConfig._rtspProp.timeOut * 1000;
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
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CVSSvc, "Can't get install root path"));
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get install root path"));
		return S_FALSE;
	}
	m_strProgramRootPath = szProgramRootPath;

	std::string	strCrashDumpPath ;
	strCrashDumpPath = pConfig._crashDump.path;

	if (!validatePath(strCrashDumpPath.c_str()))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CVSSvc, "CrashDumpPath %s error"), strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR, "invalid mini-dump path %s",strCrashDumpPath.c_str());
		return S_FALSE;
	}	
	_crashDump.setDumpPath((char*)strCrashDumpPath.c_str());
	_crashDump.enableFullMemoryDump(TRUE);
	_crashDump.setExceptionCB(CrashExceptionCallBack);

	_pCVSSThreadPool=new ZQ::common::NativeThreadPool();

	if (!_pCVSSThreadPool)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CVSSvc, "Can't create NSS thread pool instance"));
		logEvent(ZQ::common::Log::L_CRIT,"Can't create thread pool instance ,service down");
		return S_FALSE;
	}

	if (_pCVSSThreadPool) 
	{
		_pCVSSThreadPool->resize( pConfig._rtspProp.threadPoolSize> 10 ? pConfig._rtspProp.threadPoolSize : 10 );
	}

	TCHAR	szBuf[512];
	DWORD dwSize=sizeof(szBuf)/sizeof(szBuf[0]);
	ZeroMemory(szBuf,sizeof(szBuf));

	_strLogFolder=m_wsLogFolder;
	if(_strLogFolder.size()<=0)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CVSSvc, "Invalid logFolder %s"),_strLogFolder.c_str() );
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
	string strServerPath;
	uint16 uServerPort;
	strServerPath = pConfig._streamingServer.ip;
	uServerPort =  pConfig._streamingServer.port;


	_pCVSSEnv = new ::ZQTianShan::CVSS::CVSSEnv(_fileLog, *_pCVSSThreadPool, _ic,
		pConfig._iceStorm.endPoint.c_str(),
		pConfig._bind.endPoint.c_str(),
		pConfig._dataBase.path.c_str(),
		pConfig._dataBase.runtimePath.c_str());

	_cps = new ::ZQTianShan::CVSS::CiscoVirtualStreamServiceImpl(_fileLog, *_pCVSSThreadPool, *_pCVSSThreadPool, _ic, strServerPath,uServerPort, pConfig._bind.evictorSize, *_pCVSSEnv);

	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + CVSSService;
	_contentStore = new ::ZQTianShan::ContentStore::ContentStoreImpl(_csFileLog,_csEventFileLog, *_pCVSSThreadPool, _pCVSSEnv->_adapter, csDataPath.c_str());

	try{
		::std::string CSPath = pConfig._storeInfo._volumeInfo.path + FNSEPS;
		_contentStore->mountStoreVolume(pConfig._storeInfo._volumeInfo.name.c_str(), CSPath.c_str(), true);
	}
	catch(...)
	{
		_csFileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CVSSvc, "mount store volume name (%s) path (%s) catch an exception\n"),pConfig._storeInfo._volumeInfo.name.c_str(),pConfig._storeInfo._volumeInfo.path.c_str());
		return -1;
	}

	MOLOG(::ZQ::common::Log::L_INFO, CLOGFMT(CVSSvc, "CISCO Stream Service and ContentStore Service created"));

	pConfig.snmpRegister(::std::string(""));
	return BaseZQServiceApplication::OnInit();
}
HRESULT CVSSvc::OnStart()
{	
	if(_pCVSSEnv)
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
		MOLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(CVSSvc, "No enviroment is setuped"));
		return S_FALSE;
	}

	return BaseZQServiceApplication ::OnStart();	
}
HRESULT CVSSvc::OnStop()
{	
	if (_cps)
	{
		try
		{
			_cps = NULL;
		}
		catch(...){ }		
	}

	if(_pCVSSEnv)
	{
		try
		{
			delete _pCVSSEnv;
			_pCVSSEnv = NULL;
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

HRESULT CVSSvc::OnUnInit()
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

	if(_pCVSSThreadPool)
	{
		try
		{
			delete _pCVSSThreadPool;
			_pCVSSThreadPool = NULL;
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