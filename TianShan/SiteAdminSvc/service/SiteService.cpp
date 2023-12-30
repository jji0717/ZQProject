

#include "SiteService.h"

#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <MiniDump.h>
#include <SiteServiceConfig.h>
#include <time.h>



SiteAdminService gService;

ZQ::common::Config::Loader<SAConfig> gSiteAdminConfig("SiteAdminSvc.xml");
DWORD gdwServiceType =1;
DWORD gdwServiceInstance =1;

ZQ::common::BaseZQServiceApplication		*Application=&gService;

ZQ::common::Config::ILoader     *configLoader = &gSiteAdminConfig;
#ifdef _WITH_EVENTSENDER_
	ZQTianShan::Site::EventSenderManager*	g_pEventSinkMan=NULL;
    ZQTianShan::Site::EventTranslator* g_pEventTranslator = NULL;
#endif//_WITH_EVENTSENDER_


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

ZQ::common::NativeThreadPool threadpool;

static bool checkConf()
{
    if(0 == stricmp(gSiteAdminConfig.urlMode.c_str(), "Path"))
    { // Path mode
        gSiteAdminConfig.urlMode = "Path";
    }
    else
    { // default: DNS mode
        gSiteAdminConfig.urlMode = "DNS";
    }

    return true;
}
SiteAdminService::SiteAdminService()
{
	m_ic	=	NULL;
	m_pSvcEnv=	NULL;
	m_SiteSrvPtr=	NULL;
	m_pSvcLog=	NULL;
	m_pLogForIce =NULL;
#ifdef _WITH_EVENTSENDER_
	g_pEventSinkMan = NULL;
    g_pEventTranslator = NULL;
#endif//_WITH_EVENTSENDER_

}

SiteAdminService::~SiteAdminService()
{
}
HRESULT SiteAdminService::OnInit()
{
//	m_pSvcLog=new ZQ::common::FileLog((std::string(m_wsLogFolder)+gSiteAdminConfig.szSvcLogName).c_str(),
//															gSiteAdminConfig.lSvcLogLevel,
//															ZQLOG_DEFAULT_FILENUM,
//															gSiteAdminConfig.lSvcLogSize,
//															gSiteAdminConfig.lSvcLogBuffer,
//															gSiteAdminConfig.lSvcLogTimeout);
	//initialize random seed
	srand(time(NULL));
	m_pSvcLog = m_pReporter ;
    if(!checkConf())
    {
        glog(ZQ::common::Log::L_ERROR, "Illegal configuration detected!");
        return S_FALSE;
    }

	if (!validatePath(gSiteAdminConfig.szCrashDumpPath))
	{
		glog(ZQ::common::Log::L_ERROR, L"CrashDumpPath %s error", gSiteAdminConfig.szCrashDumpPath);
		logEvent(ZQ::common::Log::L_ERROR,_T("invalid minidump path %s"),gSiteAdminConfig.szCrashDumpPath);
		return S_FALSE;
	}	
	_crashDump.setDumpPath(gSiteAdminConfig.szCrashDumpPath);
	_crashDump.enableFullMemoryDump(gSiteAdminConfig.lCrashDumpEnable);
	_crashDump.setExceptionCB(CrashExceptionCallBack);

	
	int i = 0;
	Ice::PropertiesPtr proper= Ice::createProperties(i,NULL);
	//proper->setProperty("Ice.ThreadPerConnection","1");
	char	szBuf[128];
	proper->setProperty("BusinessRouter.ThreadPool.Size", itoa(gSiteAdminConfig.lIceAdapterThreadpool,szBuf,10));
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","20");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","20");
//	ZQ::common::ConfigLoader::VECKVMAP& iceProper=gSiteAdminConfig.getEnumValue("prop");
//	ZQ::common::ConfigLoader::VECKVMAP::iterator itProper=iceProper.begin();
    SAConfig::IceProps::iterator itProper = gSiteAdminConfig.iceProps.begin();
	for(;itProper!=gSiteAdminConfig.iceProps.end();itProper++)
	{		
		proper->setProperty(itProper->first, itProper->second); //((*itProper)["name"],(*itProper)["value"]);
	}
	
	std::string strLogFolder= m_wsLogFolder;
	if (strLogFolder.at(strLogFolder.length()-1)!='\\') 
	{
		strLogFolder+="\\";
	}


	::Ice::InitializationData initData;
	initData.properties = proper;	
	
	if (gSiteAdminConfig.lIceTraceEnable >=1)
	{
		m_pLogForIce = new ZQ::common::FileLog((std::string(strLogFolder)+"SiteAdminIceTrace.log").c_str(),
                                               gSiteAdminConfig.lIceTraceLogLevel,
                                               gSiteAdminConfig.lIceTraceCount,
                                               gSiteAdminConfig.lIceTraceLogSize);
		m_pIceLog =new  TianShanIce::common::IceLogI(m_pLogForIce);
		initData.logger = m_pIceLog;
	}

	m_ic = Ice::initialize(initData);	

	if(gSiteAdminConfig.lSvcThreadPoolSize > 3)
		threadpool.resize(gSiteAdminConfig.lSvcThreadPoolSize);
	else	
		threadpool.resize(3);
	try
	{
		m_pSvcEnv  =new ::ZQTianShan::Site::SiteAdminSvcEnv(*m_pSvcLog,
															threadpool,
															m_ic,
															(const char*)gSiteAdminConfig.szServiceEndpoint,
															(const char*)gSiteAdminConfig.szDatabaseFolder,
															(const char*)gSiteAdminConfig.szRuntimeDbFolder ) ;
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,("Catch an Ice exception [%s] when create SiteAdminEnviroment"),ex.ice_name().c_str());
		logEvent(ZQ::common::Log::L_ERROR,_T("Catch an Ice exception [%s] when create SiteAdminEnviroment"),ex.ice_name().c_str());
		m_pSvcEnv = NULL;
		return S_FALSE;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,"unexpect error,create SiteAdminEnviroment failed");
		logEvent(ZQ::common::Log::L_ERROR,_T("unexpect error,create SiteAdminEnviroment failed"));
		m_pSvcEnv = NULL;
		return S_FALSE;
	}

	try
	{
		m_SiteSrvPtr = new ZQTianShan::Site::SiteAdminImpl(*m_pSvcEnv);
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,("Catch an Ice exception [%s] when create SiteAdminImpl"),ex.ice_name().c_str());
		logEvent(ZQ::common::Log::L_ERROR,_T("Catch an Ice exception [%s] when create SiteAdminImpl"),ex.ice_name().c_str());
		m_pSvcEnv = NULL;
		return S_FALSE;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,"create SiteAdmin instance failed");
		logEvent(ZQ::common::Log::L_ERROR,_T("create SiteAdmin instance failed"));
		m_SiteSrvPtr = NULL;
		return S_FALSE;
	}

#ifdef _WITH_EVENTSENDER_
	try
	{
		g_pEventSinkMan = new ZQTianShan::Site::EventSenderManager(*m_pSvcEnv);
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,("Catch an Ice exception [%s] when create EventSenderManager"),ex.ice_name().c_str());
		logEvent(ZQ::common::Log::L_ERROR,_T("Catch an Ice exception [%s] when create EventSenderManager"),ex.ice_name().c_str());
		g_pEventSinkMan=NULL;
		return S_FALSE;
	}
	catch (...) 
	{
		g_pEventSinkMan=NULL;
		glog(ZQ::common::Log::L_ERROR,"unexpect error when create eventSendermanager");
		logEvent(ZQ::common::Log::L_ERROR,_T("unexpect error when create eventSendermanager"));
		return S_FALSE;
	}
	
    g_pEventTranslator = new ZQTianShan::Site::EventTranslator(*m_pSvcEnv, *g_pEventSinkMan);

	if(!g_pEventSinkMan->SetupEventSenderEnvironment())
	{
		g_pEventSinkMan->DestroyEventSenderEnvironment();
		glog(ZQ::common::Log::L_ERROR,"Setup EventSender Environment failed");
		return S_FALSE;
	}

#endif //_WITH_EVENTSENDER_	
    
    gSiteAdminConfig.snmpRegister("");
	return BaseZQServiceApplication::OnInit();
}
HRESULT SiteAdminService::OnStart()
{
	if(m_pSvcEnv)
	{
		m_pSvcEnv->_adapter->activate();
		m_pSvcEnv->_txnWatchDog->start();
		m_pSvcEnv->_liveTxnTransfer->start();
#ifdef _WITH_EVENTSENDER_
        g_pEventTranslator->start();
#endif
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,"invalid SiteAdmin enviroment");
		logEvent(ZQ::common::Log::L_ERROR,_T("invalid SiteAdmin enviroment"));
		return S_FALSE;
	}
	return BaseZQServiceApplication::OnStart();
}
HRESULT  SiteAdminService::OnStop()
{
#ifdef _WITH_EVENTSENDER_
    if (g_pEventTranslator)
    { // must delete the object before the adapter's deactive
        delete g_pEventTranslator;
        g_pEventTranslator = NULL;
    }
	if (g_pEventSinkMan) 
	{
		try
		{
			g_pEventSinkMan->DestroyEventSenderEnvironment();
		}
		catch (...) 
		{
		}
	}
#endif//_WITH_EVENTSENDER_

	if(m_pSvcEnv)
	{
		m_pSvcEnv->_adapter->deactivate();
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,"invalid SiteAdmin enviroment");
		logEvent(ZQ::common::Log::L_ERROR,_T("invalid SiteAdmin enviroment"));
		//return S_FALSE;
	}
	if( m_pSvcEnv )
	{
		 m_pSvcEnv->_thpool.stop();
	}
	return BaseZQServiceApplication::OnStop();
}
HRESULT SiteAdminService::OnUnInit()
{
	try
	{
		
#ifdef _WITH_EVENTSENDER_
		if(g_pEventSinkMan)
		{
			delete g_pEventSinkMan;
			g_pEventSinkMan = NULL;
		}
#endif //_WITH_EVENTSENDER_

//		if(m_pSvcLog)
//			delete m_pSvcLog;
		if(m_SiteSrvPtr)
			m_SiteSrvPtr =NULL;		
		try
		{
			m_ic->destroy();
		}
		catch (...) 
		{
		}
		if(m_pSvcEnv)
			delete m_pSvcEnv;
		if(m_pIceLog)
		{
			m_pIceLog = NULL;
		}
		if(m_pLogForIce)
		{
			delete m_pLogForIce;
			m_pLogForIce = NULL;
		}
	}
	catch (...)
	{
	}
	return BaseZQServiceApplication::OnUnInit();
}

void SiteAdminService::OnSnmpSet(const char *varName)
{
    if(0 == strcmp(varName, "default/IceTrace/level"))
    {
        if(m_pLogForIce)
        {
            m_pLogForIce->setVerbosity(gSiteAdminConfig.lIceTraceLogLevel);
        }
    }
}
