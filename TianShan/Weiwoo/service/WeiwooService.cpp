
#include "WeiwooService.h"
#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Log.h>
#include <FileLog.h>
#include <WeiwooConfig.h>
#include "FileSystemOp.h"
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif

#define CANDIDATE_TICKETS_MIN (1)

#ifndef S_FALSE
	//work around for linux base service wrapper
	#define S_FALSE false
#endif//

#ifndef S_OK
	//work around for linux base service wrapper
	#define S_OK true
#endif

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
#endif

WeiwooService g_server;

ZQ::common::BaseZQServiceApplication *Application = &g_server;

::ZQ::common::NativeThreadPool *threadpoolWeiwoo=NULL;
::ZQ::common::NativeThreadPool *threadpoolPath=NULL;


ZQ::common::Config::Loader<WeiwooCfg> gWeiwooServiceConfig("Weiwoo.xml");
//WeiwooConfig					gDummyConfig;
ZQ::common::Config::ILoader		*configLoader = &gWeiwooServiceConfig;

TianShanIce::SRM::SessionManagerPtr _gSessionManager = NULL;

extern const char* DUMP_PATH;

#ifdef ZQ_OS_MSWIN
ZQ::common::MiniDump			_crashDump;

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();	
	glog( ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
	glog.flush();
}
bool validatePath( const char *     szPath )
{
    if (-1 != ::GetFileAttributesA(szPath))
        return true;
	
    DWORD dwErr = ::GetLastError();
    if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
    {
        if (!::CreateDirectoryA(szPath, NULL))
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
#endif

void initWithConfig(Ice::PropertiesPtr proper )
{
	proper->setProperty("PathManager.ThreadPool.Size",gWeiwooServiceConfig.szPathManagerAdapterThreadPoolSize);	
	proper->setProperty("PathManager.ThreadPool.SizeMax",gWeiwooServiceConfig.szPathManagerAdapterThreadPoolSizeMax);		
	proper->setProperty("Weiwoo.ThreadPool.Size",gWeiwooServiceConfig.szWeiwooAdpaterThreadpool);
	proper->setProperty("Weiwoo.ThreadPool.SizeMax",gWeiwooServiceConfig.szWeiwooAdpaterThreadpoolMax);
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");
    
    std::map<std::string, std::string>::iterator itProper;
	for( itProper = gWeiwooServiceConfig.iceProperties.begin(); 
				itProper!=gWeiwooServiceConfig.iceProperties.end();
				itProper++)
	{		
		proper->setProperty(itProper->first, itProper->second);//((*itProper)["name"],(*itProper)["value"]);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
WeiwooService::WeiwooService()
{
	
}
WeiwooService::~WeiwooService()
{

}
#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp(varName , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }
/*
void WeiwooService::OnSnmpSet(const char *varName)
{
	if ( !varName || strlen(varName) <= 0 ) 
	{
		glog(ZQ::common::Log::L_INFO , "WeiwooService Invalid varName in OnSnmpSet , just return directly ");
		return ; 
	}
	STRSWITCH()
	STRCASE("Weiwoo/BusinessRouter/mixedTeardownReason")
		glog(ZQ::common::Log::L_INFO , 
					CLOGFMT( WeiwooService, "Set Weiwoo/BusinessRouter/mixedTeardownReason to %d"),
					gWeiwooServiceConfig.lMixTeardownReasonAndTerminateReason);
	STRCASE("Weiwoo/PathSelection/maxTicketCount")
		if( gWeiwooServiceConfig.lMaxSelectTickets < CANDIDATE_TICKETS_MIN)
		{			
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(WeiwooService, "Weiwoo/PathSelection/maxTicketCount was %d, adjusted to %d"),
					gWeiwooServiceConfig.lMaxSelectTickets, CANDIDATE_TICKETS_MIN);
			gWeiwooServiceConfig.lMaxSelectTickets = CANDIDATE_TICKETS_MIN;
		}	

	STRENDCASE()
}
*/


HRESULT WeiwooService::OnInit()
{
	std::string			_strConfigFile;
	std::string			_strLogFolder;
	std::string			_strPluginFolder;

	m_pLogWeiwoo = m_pReporter;

	_strConfigFile=gWeiwooServiceConfig.getConfigFilePath();
	//get the install root
	std::string szProgramRootPath = FS::getImagePath();
	std::string::size_type pos = szProgramRootPath.rfind(FNSEPC);

	if (pos == std::string::npos) 
	{
		glog(ZQ::common::Log::L_ERROR,"Can't get install root path");
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get install root path"));
#endif
		return S_FALSE;
	}
	szProgramRootPath = szProgramRootPath.substr(0, pos+1);
	
	std::string	strCrashDumpPath ;
	if (strstr(gWeiwooServiceConfig.szCrashDumpPath,":")!=NULL) 
	{
		strCrashDumpPath = gWeiwooServiceConfig.szCrashDumpPath;
	}
	else
	{
		strCrashDumpPath=szProgramRootPath;
		strCrashDumpPath+=gWeiwooServiceConfig.szCrashDumpPath;		
	}
	
#ifdef ZQ_OS_MSWIN
	if (!validatePath(strCrashDumpPath.c_str()))
	{
		glog(ZQ::common::Log::L_ERROR, L"CrashDumpPath %s error",strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR,_T("invalid minidump path %s"),strCrashDumpPath.c_str());
		return S_FALSE;
	}	
	_crashDump.setDumpPath((char*)strCrashDumpPath.c_str());
	_crashDump.enableFullMemoryDump(gWeiwooServiceConfig.lCrashDumpEnable);
	_crashDump.setExceptionCB(CrashExceptionCallBack);
#else
	DUMP_PATH = gWeiwooServiceConfig.szCrashDumpPath;
#endif


	if(m_pLogPath!=NULL)
	{
		try{delete m_pLogPath;m_pLogPath=NULL;}catch(...){ }
	}
	if(wenv!=NULL)
	{
		try{delete wenv;wenv=NULL;}catch(...){ }
	}
	if(penv!=NULL)
	{
		try{delete penv;penv=NULL;}catch(...){ }
	}
	if(threadpoolWeiwoo!=NULL)
	{
		try{delete threadpoolWeiwoo;threadpoolWeiwoo=NULL;}catch(...){ }
	}
	if(threadpoolPath!=NULL)
	{
		try{delete threadpoolPath;threadpoolPath=NULL;}catch(...){ }
	}

	threadpoolWeiwoo=new ZQ::common::NativeThreadPool();
	threadpoolPath = new ZQ::common::NativeThreadPool();

	if ( !threadpoolWeiwoo || !threadpoolPath ) 
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_CRIT,"Can't create threadpool instance ,service down");
#endif
		return S_FALSE;
	}

	if ( threadpoolWeiwoo ) 
	{
		threadpoolWeiwoo->resize( gWeiwooServiceConfig.lServiceThreadPoolSize > 5 ? 
								gWeiwooServiceConfig.lServiceThreadPoolSize : 5 );
	}
	if ( threadpoolPath ) 
	{
		long pathThreadpoolSize = gWeiwooServiceConfig.lServiceThreadPoolSize+5;
		threadpoolPath->resize(  pathThreadpoolSize> 5 ? 
								pathThreadpoolSize : 5 );
	}

	

// 	TCHAR	szBuf[512];
// 	DWORD dwSize=sizeof(szBuf)/sizeof(szBuf[0]);
// 	ZeroMemory(szBuf,sizeof(szBuf));

	//_strLogFolder=gWeiwooServiceConfig.szLogFolder;
#ifdef ZQ_OS_MSWIN
	_strLogFolder=m_wsLogFolder;
#else
	_strLogFolder=_logDir;
#endif
	if(_strLogFolder.size()<=0)
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("Invalid logFolder %s"),_strLogFolder.c_str() );
#endif
		return S_FALSE;
	}
	if( _strLogFolder[_strLogFolder.length()-1]!=FNSEPC) 
		_strLogFolder+=FNSEPS;

	{
		std::string pathParent = ZQTianShan::Util::fsGetParentFolderPath(szProgramRootPath);
		_strPluginFolder +=  ZQTianShan::Util::fsConcatPath( pathParent , "modules"FNSEPS );
	}

	
	if(_strPluginFolder.size() >0 && ( _strPluginFolder[_strPluginFolder.length()-1]!=FNSEPC))
		_strPluginFolder+=FNSEPS;

	//step 3.create log file

	m_pLogPath=new ZQ::common::FileLog( (_strLogFolder+"Path.log").c_str(),
												gWeiwooServiceConfig.lPathLogLevel,
												ZQLOG_DEFAULT_FILENUM,
												gWeiwooServiceConfig.lPathLogSize,
												gWeiwooServiceConfig.lPathLogBuffer,
												gWeiwooServiceConfig.lPathLogTimeOut );

	m_pIceLog=new ZQ::common::FileLog( (_strLogFolder+"WeiwooSRMIceTrace.log").c_str(),
												gWeiwooServiceConfig.lIceTraceLogLevel,
												ZQLOG_DEFAULT_FILENUM,
												gWeiwooServiceConfig.lIceTraceLogSize);
												
	m_iceLogger=new IceLogger(*m_pIceLog);

	int i=0;

#if ICE_INT_VERSION / 100 >= 303

	::Ice::InitializationData initData;
	initData.properties = Ice::createProperties(i, NULL);
	initWithConfig(initData.properties);
	
	if (gWeiwooServiceConfig.lIcetraceLogEnable >=1)
		initData.logger = m_iceLogger;

	m_icWeiwoo = Ice::initialize(initData);
	m_icPath =   Ice::initialize(initData);

#else
	Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);

	initWithConfig(proper);

	
	if(gWeiwooServiceConfig.lIcetraceLogEnable>=1)
	{
		m_icWeiwoo=Ice::initializeWithPropertiesAndLogger(i,NULL,proper,m_iceLogger);
	}
	else
	{
		m_icWeiwoo=Ice::initializeWithProperties(i,NULL,proper);
	}
	if(gWeiwooServiceConfig.lIcetraceLogEnable>=1)
		m_icPath = Ice::initializeWithPropertiesAndLogger(i,NULL,proper,m_iceLogger);
	else
		m_icPath = Ice::initializeWithProperties(i,NULL,proper);
#endif // ICE_INT_VERSION

	std::string	strDbPath ;
#ifdef ZQ_OS_MSWIN
	if (strstr(gWeiwooServiceConfig.szIceDbFolder,":")!=NULL) 
	{
		strDbPath = gWeiwooServiceConfig.szIceDbFolder;
	}
	else
	{
		strDbPath = szProgramRootPath;
		strDbPath += gWeiwooServiceConfig.szIceDbFolder;
	}
#else
		strDbPath = gWeiwooServiceConfig.szIceDbFolder;
#endif

	try
	{
		std::string dbRuntimePath ;
		if ( gWeiwooServiceConfig.szIceDbRuntimeFolder&&strlen(gWeiwooServiceConfig.szIceDbRuntimeFolder)>1 ) 
		{
#ifdef ZQ_OS_MSWIN
			//dbRuntimePath = gWeiwooServiceConfig.szIceDbRuntimeFolder;
			if (strstr(gWeiwooServiceConfig.szIceDbRuntimeFolder,":")!=NULL) 
			{
				dbRuntimePath = gWeiwooServiceConfig.szIceDbRuntimeFolder;
			}
			else
			{
				dbRuntimePath = szProgramRootPath;
				dbRuntimePath += gWeiwooServiceConfig.szIceDbRuntimeFolder;
			}
#else
			dbRuntimePath = gWeiwooServiceConfig.szIceDbRuntimeFolder;
#endif
		}		
		


		wenv=new WeiWooEnvEx(*m_pLogWeiwoo,*threadpoolWeiwoo,
								m_icWeiwoo,gWeiwooServiceConfig.szWeiwooEndpoint);
		if(!wenv->init(strDbPath.c_str(),dbRuntimePath.c_str()))
		{
			(*m_pLogWeiwoo)(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to init weiwoo enviroment"));
			return S_FALSE;
		}
		wenv->_maxTickets = (gWeiwooServiceConfig.lMaxSelectTickets > 5 ) ? gWeiwooServiceConfig.lMaxSelectTickets : 5;
	}
	catch (...)
	{
		(*m_pLogWeiwoo)(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to create weiwoo enviroment"));
		return S_FALSE;
	}	
	

	try
	{
		penv= new ZQTianShan::AccreditedPath::PathSvcEnv(*wenv,m_icPath,*m_pLogPath,
														*threadpoolPath,
														_strPluginFolder.c_str(),
														_strConfigFile.c_str(),
														_strLogFolder.c_str() );

		penv->_streamLinksByMaxTicket = max(0, gWeiwooServiceConfig.lStrmLinksByTicket);
	}
	catch( ... )
	{
		(*m_pLogPath)(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to create path enviroment"));
		return S_FALSE;
	}

//	penv->doValidation();
	
#ifdef WITH_ICESTORM
	if (strlen(gWeiwooServiceConfig.szIceStormEndPoint)>0)
	{
		(*m_pLogWeiwoo)(ZQ::common::Log::L_DEBUG, CLOGFMT(Weiwoo, "opening TianShan topic manager at \"%s\""), gWeiwooServiceConfig.szIceStormEndPoint);
		try 
		{
			IceStorm::TopicManagerPrx topicManager = IceStorm::TopicManagerPrx::checkedCast(m_icWeiwoo->stringToProxy(std::string(SERVICE_NAME_TopicManager ":") + gWeiwooServiceConfig.szIceStormEndPoint));
			wenv->openPublisher(topicManager);
		}
		catch(Ice::Exception& e)
		{
			(*m_pLogWeiwoo)(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to init event publisher at \"%s\": %s"), gWeiwooServiceConfig.szIceStormEndPoint, e.ice_name().c_str());
		}
		catch(...)
		{
			(*m_pLogWeiwoo)(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to init event publisher at \"%s\": unknown exception"),  gWeiwooServiceConfig.szIceStormEndPoint);
		}
	}
#endif // WITH_ICESTORM

//	brouter = new ZQTianShan::Weiwoo::BusinessRouterImpl(*wenv);
	sessmgr = new ZQTianShan::Weiwoo::SessionManagerImpl(*wenv);
	
	_gSessionManager = sessmgr;

	ADPaths = new ZQTianShan::AccreditedPath::AccreditedPathsImpl(*penv);

    gWeiwooServiceConfig.snmpRegister("");

	try
	{
		penv->doValidation();
	}
	catch( const Ice::Exception& ex)
	{
		(*m_pLogPath)(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to validate path environment due to [%s]"), ex.ice_name().c_str() );
		return S_FALSE;
	}
	catch(...)
	{
		(*m_pLogPath)(ZQ::common::Log::L_ERROR, CLOGFMT(Weiwoo, "failed to validate path environment"));
		return S_FALSE;
	}

	return BaseZQServiceApplication::OnInit();
}

void WeiwooService::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();

	//{".2", "weiwooSvcApp" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2)
	//{".2.1", "weiwooAttr" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2).weiwooAttr(1)
	//{".2.1.100", "weiwooStat" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2).weiwooAttr(1).weiwooStat(100)
	//{".2.1.100.2", "weiwooStat-cSessions" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2).weiwooAttr(1).weiwooStat(100).weiwooStat-cSessions(2)
	//{".2.1.100.3", "weiwooStat-cPending" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2).weiwooAttr(1).weiwooStat(100).weiwooStat-cPending(3)
	//{".2.1.100.4", "weiwooStat-BusyThreads" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2).weiwooAttr(1).weiwooStat(100).weiwooStat-BusyThreads(4)
	//{".2.1.100.5", "weiwooStat-ThreadPoolSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2).weiwooAttr(1).weiwooStat(100).weiwooStat-ThreadPoolSize(5)
	//{".2.1.41", "weiwooDumpSgUsage" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).weiwoo(200).weiwooSvcApp(2).weiwooAttr(1).weiwooDumpSgUsage(41)
	ServiceMIB_ExportByAPI(_pServiceMib, "weiwooStat-cSessions", WeiwooService, *this, uint32, AsnType_Int32, &WeiwooService::getSessionCount, NULL, "");
	ServiceMIB_ExportByAPI(_pServiceMib, "weiwooStat-cPending", WeiwooService, *this, uint32, AsnType_Int32, &WeiwooService::getPendingSize, NULL, "");
	ServiceMIB_ExportByAPI(_pServiceMib, "weiwooStat-BusyThreads", WeiwooService, *this, uint32, AsnType_Int32, &WeiwooService::getBusyThreads, NULL, "");
	ServiceMIB_ExportByAPI(_pServiceMib, "weiwooStat-ThreadPoolSize", WeiwooService, *this, uint32, AsnType_Int32, &WeiwooService::getThreads, NULL, "");
	ServiceMIB_ExportByAPI(_pServiceMib, "weiwooDumpSgUsage", WeiwooService, *this, uint32, AsnType_Int32, &WeiwooService::dummyGet, &WeiwooService::dumpSgUsage, "");
}

HRESULT WeiwooService::OnStart()
{
	
	if(wenv)
	{
#ifndef _INDEPENDENT_ADAPTER

        std::vector<MonitoredLog>::iterator iter;
		for (iter = gWeiwooServiceConfig.monitoredLogs.begin(); iter != gWeiwooServiceConfig.monitoredLogs.end(); ++iter) 
		{			
			if (!wenv->_adapter->publishLogger(iter->name.c_str(), iter->syntax.c_str(),
										iter->key.c_str() , iter->type.c_str() ))				
			{				
				glog(ZQ::common::Log::L_ERROR,
					"Failed to publish logger name[%s] synax[%s] key[%s] type[%s]",
					iter->name.c_str(), iter->syntax.c_str() ,iter->key.c_str() ,iter->type.c_str() );
			}			
			else				
			{				
				glog(ZQ::common::Log::L_INFO,
					"Publish logger name[%s] synax[%s] key[%s] type[%s] successful",
					iter->name.c_str(), iter->syntax.c_str() , iter->key.c_str() , iter->type.c_str() );
			}			
		}		
#endif

        wenv->_adapter->activate();
        penv->_adapter->activate();
		wenv->_serviceState = ::TianShanIce::stInService;
		penv->_serviceState = ::TianShanIce::stInService;
	}
	else
	{
		(*m_pLogWeiwoo)(ZQ::common::Log::L_CRIT,"No weiwoo enviroment is setuped");
		return S_FALSE;
	}

	return BaseZQServiceApplication ::OnStart();
}

HRESULT WeiwooService::OnStop()
{	
	if(wenv)
	{
		try{ wenv->_watchDog.quit(); } catch(...){ }
		try{ wenv->_adapter->deactivate(); wenv->_serviceState = ::TianShanIce::stOutOfService; }catch(...){ }	

#ifndef _INDEPENDENT_ADAPTER
		//unpublish log
		std::vector<MonitoredLog>::iterator iter;
		for (iter = gWeiwooServiceConfig.monitoredLogs.begin(); iter != gWeiwooServiceConfig.monitoredLogs.end(); ++iter) 
		{			
			if (!wenv->_adapter->unpublishLogger(iter->name.c_str()))				
			{				
				(*m_pLogWeiwoo)(ZQ::common::Log::L_ERROR, "Failed to Unpublish logger name[%s] synax[%s] key[%s] type[%s]",
					iter->name.c_str(), iter->syntax.c_str() ,iter->key.c_str() ,iter->type.c_str() );
			}			
			else				
			{				
				(*m_pLogWeiwoo)(ZQ::common::Log::L_DEBUG, "Unpublish logger name[%s] synax[%s] key[%s] type[%s] successful",
					iter->name.c_str(), iter->syntax.c_str() , iter->key.c_str() , iter->type.c_str() );
			}			
		}		
#endif
	}

	if (penv) 
	{
		try{ penv->_adapter->deactivate(); penv->_serviceState = ::TianShanIce::stOutOfService; } catch(...){ }
	}

	return BaseZQServiceApplication::OnStop();	
}

HRESULT WeiwooService::OnUnInit()
{
	try
	{
		ADPaths=NULL;
	}
	catch (...) 
	{
	}
	try
	{
		sessmgr=NULL;
	}
	catch (...) 
	{
	}
	if(penv)
	{
		try
		{
			delete penv;
			penv=NULL;
		}
		catch (...) {}
	}

	if(wenv)
	{
		try
		{
			delete wenv;
			wenv=NULL;
		}
		catch(...){}
	}
	try
	{
		m_icWeiwoo->destroy();
	}
	catch (...)
	{
	}	
	try
	{
		m_icPath->destroy();
	}
	catch (...)
	{
	}

	if(m_pLogPath)
	{
		try
		{
			delete m_pLogPath;
			m_pLogPath=NULL;
		}
		catch(...){
		}
	}

//	if(m_pLogWeiwoo)
//	{
//		try
//		{
//			delete m_pLogWeiwoo;
//			m_pLogWeiwoo=NULL;
//		}
//		catch(...){
//		}
//	}
	if (m_pIceLog) 
	{
		try
		{
			delete m_pIceLog;
			m_pIceLog = NULL;
		}
		catch(...)
		{
		}
	}
	if(threadpoolWeiwoo)
	{
		try
		{
			delete threadpoolWeiwoo;
			threadpoolWeiwoo=NULL;
		}
		catch(...){ }
	}
	
	if(threadpoolPath)
	{
		try
		{
			delete threadpoolPath;
			threadpoolPath=NULL;
		}
		catch(...){ }
	}
	
	return BaseZQServiceApplication ::OnUnInit();	
}

// vim: ts=4 sw=4 bg=dark nu
