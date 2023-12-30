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

void initWithConfig(Ice::PropertiesPtr proper )
{
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

	for (ZQTianShan::NSS::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
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
	strcpy(servname, "NSS");
	strcpy(prodname, "TianShan");
}

NSSSvc::~NSSSvc()
{

}

HRESULT NSSSvc::OnInit()
{
	std::string			_strConfigFile;
	std::string			_strLogFolder;
	std::string			_strPluginFolder;

	//NSS log
	_iceFileLog.open(pConfig._nssIceLog.path.c_str(),
					 pConfig._nssIceLog.level,
					 pConfig._nssIceLog.maxCount,
					 pConfig._nssIceLog.size,
					 pConfig._nssIceLog.bufferSize,
					 pConfig._nssIceLog.flushTimeout);

	_fileLog.open(pConfig._logFile.path.c_str(),
				  pConfig._logFile.level,
				  pConfig._logFile.maxCount,
				  pConfig._logFile.size,
				  pConfig._logFile.bufferSize,
				  pConfig._logFile.flushTimeout);

	//A3 log
	_csFileLog.open(pConfig._a3Log.path.c_str(),
					pConfig._a3Log.level,
					pConfig._a3Log.logNum,
					pConfig._a3Log.size,
					pConfig._a3Log.buffer,
					pConfig._a3Log.flushTimeout);

	_csEventFileLog.open(pConfig._a3EventLog.path.c_str(),
						 pConfig._a3EventLog.level,
						 pConfig._a3EventLog.logNum,
						 pConfig._a3EventLog.size,
						 pConfig._a3EventLog.buffer,
						 pConfig._a3EventLog.flushTimeout);

	iTimeOut = pConfig._timeOut.time * 1000;
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
		_pNSSThreadPool->resize( pConfig._stBind.threadPoolSize> 10 ? pConfig._stBind.threadPoolSize : 10 );
	}


	//set A3 thread pool
	//_pA3CSThreadPool=new ZQ::common::NativeThreadPool();

	//if (!_pA3CSThreadPool)
	//{
	//	MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "Can't create A3 thread pool instance"));
	//	logEvent(ZQ::common::Log::L_CRIT,"Can't create thread pool instance ,service down");
	//	return S_FALSE;
	//}

	//if (_pA3CSThreadPool) 
	//{
	//	_pA3CSThreadPool->resize( pConfig._stBind.threadPoolSize> 10 ? pConfig._stBind.threadPoolSize : 10 );
	//}
	
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
	/*::ZQ::common::FileLog iceLogFile(pConfig._nssIceLog.path.c_str(),
									 pConfig._nssIceLog.level,
									 pConfig._nssIceLog.size,
									 pConfig._nssIceLog.buffer,
									 pConfig._nssIceLog.flushTimeout);*/

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

	/*::ZQ::common::FileLog logFile(pConfig._nssLog.path.c_str(),
								  pConfig._nssLog.level,
								  pConfig._nssLog.size,
								  pConfig._nssLog.buffer,
								  pConfig._nssLog.flushTimeout);*/
	
	//get media server config
	string strServerPath;
	uint16 uServerPort;
	for (ZQTianShan::NSS::MediaCluster::Servers::const_iterator MCiter = pConfig._mediaCluster._servers.begin();
	MCiter != pConfig._mediaCluster._servers.end(); MCiter++)
	{
		
		strServerPath = MCiter->address.c_str();
		uServerPort = MCiter->port;
		break;
	}

	_pNSSEnv = new ::ZQTianShan::NSS::NSSEnv(_fileLog, *_pNSSThreadPool, _ic,
											 pConfig._iceStorm.endPoint.c_str(),
											 pConfig._stBind.endPoint.c_str(),
										 	 pConfig._dataBase.path.c_str(),
											 pConfig._dataBase.runtimePath.c_str());

	_cps = new ::ZQTianShan::NSS::NGODStreamServiceImpl(_fileLog, *_pNSSThreadPool, _ic, strServerPath, 
		uServerPort,pConfig._sessionGroup, pConfig._stBind.evictorSize, *_pNSSEnv);
	//add for A3 content store

	//try
	//{
	//	_csAdapter = ZQADAPTER_CREATE(_ic, "A3CS",  pConfig._stBind.endPoint.c_str(), _csFileLog);	 
	//}
	//catch(Ice::Exception& ex)
	//{
	//	_csFileLog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEnv,"Create adapter failed with endpoint=%s and exception is %s"),pConfig._stBind.endPoint.c_str(), ex.ice_name().c_str());
	//}
	
	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + NSSService;
	_contentStore = new ::ZQTianShan::ContentStore::ContentStoreImpl(_csFileLog,_csEventFileLog, *_pNSSThreadPool, _pNSSEnv->_adapter, csDataPath.c_str());

	try{
		//if (_contentStore)
		//	_csAdapter->activate();
		::std::string A3CSPath = pConfig._a3VolumeInfo.path + FNSEPS;
		_contentStore->mountStoreVolume(pConfig._a3VolumeInfo.name.c_str(),A3CSPath.c_str(),true);
	}
	catch(...)
	{
		_csFileLog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "mount store volume name (%s) path (%s) catch an exception\n"),pConfig._a3VolumeInfo.name.c_str(),pConfig._a3VolumeInfo.path.c_str());
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
		MOLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(NSSSvc, "No enviroment is setuped"));
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