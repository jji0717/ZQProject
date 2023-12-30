#include "GBCSService.h"
#include "ServantFactory.h"
#include "GBCSCfgLoader.h"
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif
#include "TianShanIceHelper.h"
#include "IceLog.h"

GBCSService					gGBCSServiceInstance;
ZQ::common::BaseZQServiceApplication		*Application=&gGBCSServiceInstance;
ZQ::common::Config::Loader<ZQTianShan::GBCS::GBCSCfg> gGBCSConfig("GBCS.xml");

ZQ::common::Config::ILoader					*configLoader = &gGBCSConfig;
ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder	*pGBCSBaseConfig = NULL;
int gbcsContentInterfaceTestEnable = 0;
std::string gbcsContentInterfaceTestFolder;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

static ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog( ZQ::common::Log::L_CRIT, 
		_T("Crash exception callback called,ExceptionCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x"),
		ExceptionCode, ExceptionAddress, dwThreadID);
	glog.flush();	
}
#else
extern const char* DUMP_PATH;
#endif

bool GBCSService::initializeCrashDumpLocation( )
{
#ifdef ZQ_OS_MSWIN
	try
	{
		if(! ZQTianShan::Util::fsCreatePath( gGBCSConfig._crashDump.path) )
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(GBCSService,"can't create path [%s] for mini dump "),
				gGBCSConfig._crashDump.path);
			return false;
		}	
		_crashDump.setDumpPath((char *)gGBCSConfig._crashDump.path.c_str());
		_crashDump.enableFullMemoryDump(gGBCSConfig._crashDump.enabled);
		_crashDump.setExceptionCB(CrashExceptionCallBack);
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSService, "unexpected exception caught when initializeCrashDumpLocation"));
		return false;
	}
#else
	DUMP_PATH = gGBCSConfig._crashDump.path.c_str();
#endif
	return true;
}

std::string	GBCSService::getLoggerFolder( ) const
{
#ifdef ZQ_OS_MSWIN
	std::string folderPath = m_wsLogFolder;
#else
	std::string folderPath = _logDir;
#endif
	if ( folderPath.at(folderPath.length()-1) != FNSEPC)
	{
		folderPath += FNSEPS;
	}
	return folderPath;
}

bool GBCSService::InitializeIceRunTime( )
{
	try
	{
		Ice::InitializationData		iceInitData;
		int i = 0;
		iceInitData.properties =Ice::createProperties( i , NULL );

		std::stringstream ss;
		ss << pGBCSBaseConfig->_bind.dispatchSize;
		iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.Size", ss.str());
		ss << pGBCSBaseConfig->_bind.dispatchMax;
		iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.SizeMax",ss.str());
		glog(ZQ::common::Log::L_INFO,CLOGFMT(GBCSService,"set dispatchSize/dispatchSizeMax to [%d/%d]"),
			pGBCSBaseConfig->_bind.dispatchSize,
			pGBCSBaseConfig->_bind.dispatchMax);

		//set ice properties
		::ZQTianShan::GBCS::IceProperties::props::const_iterator it = pGBCSBaseConfig->_iceProperty._props.begin();
		for( ; it != pGBCSBaseConfig->_iceProperty._props.end() ; it ++ )
		{
			iceInitData.properties->setProperty( it->name , it->value );
			glog(ZQ::common::Log::L_INFO,CLOGFMT(GBCSService,"Set ice property [%s] \t\t\t= [%s]"),
				it->name.c_str() , it->value.c_str() );
		}

		mIc	=	Ice::initialize( i , NULL , iceInitData );
		return ( mIc );
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSService, "[%s] caught when InitializeIceRunTime"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSService, "unexpected exception caught when InitializeIceRunTime"));
		return false;
	}
}

bool GBCSService::initializeServiceParameter( )
{
	
	//
	// init adapter
	//	
	try
	{
		std::string serviceName("GBCS");//mGBCSEnv._strServiceName is not used as it is not been inited.
		mainAdapter = ZQADAPTER_CREATE( mIc, (serviceName.c_str()), pGBCSBaseConfig->_bind.endPoint.c_str(), *m_pReporter);
		assert(mainAdapter);
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSService, "Create adapter failed with endpoint=%s and exception is %s"),
			pGBCSBaseConfig->_bind.endPoint.c_str(), ex.ice_name().c_str());
		return false;
	}

	//
	// init contentstore
	//
#ifdef ZQ_OS_MSWIN
	mGBCSEnv.setServiceName(servname);
#else
	mGBCSEnv.setServiceName(getServiceName().c_str());
#endif
	if(!mGBCSEnv.setGBCSLog(m_pReporter))
		return false;

    mGBCSEnv.setLogPath(getLoggerFolder( ).c_str());
	mGBCSEnv.setDataPath(gGBCSConfig._dataBase.path.c_str());
	mGBCSEnv.setConfig(pGBCSBaseConfig);	
	mGBCSEnv.setThreadPool(&mContentStorethreadpool);
	mGBCSEnv.setIceAdapter(mainAdapter);
	if (!mGBCSEnv.initEnv())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSService, "failed to initialize ContentStore"));
		return false;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService, "ContentStore module initialized successfully"));

	gbcsContentInterfaceTestEnable = pGBCSBaseConfig->_videoServer.ContentInterfaceTestEnable;
	gbcsContentInterfaceTestFolder = pGBCSBaseConfig->_videoServer.ContentInterfaceTestFolder;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService, "GBCSService module initialized successfully"));

	return true;
}

void GBCSService::UninitializeIceRunTime()
{
	try
	{
		if(mIc)
		{
			mIc->destroy();
			mIc = NULL;
		}
	}
	catch(...)	{}
}


GBCSService::GBCSService()
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "GBCS");
	strcpy(prodname, "TianShan");
#endif
	mIc				=	NULL;
	// GBCSSnmpAgent    =   NULL;
}

GBCSService::~GBCSService()
{

}

HRESULT GBCSService::OnInit( )
{
	//get regedit argument
	std::string strNetId = "";
	if(m_argc == 1 && strlen(m_argv[0]))
	{
		strNetId = m_argv[0];
	}	

	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService, "netID='%s'"), strNetId.c_str());

	for (::ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolderVec::iterator iter = gGBCSConfig._gbcsBaseConfig.GBCSVec.begin(); iter != gGBCSConfig._gbcsBaseConfig.GBCSVec.end(); iter++)
	{
		if (strNetId == (*iter).netId)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService, "find NetID=%s"), strNetId.c_str());
			pGBCSBaseConfig = &(*iter);
		}
	}

	if (NULL == pGBCSBaseConfig)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSService, "couldn't find specified netID[%s]"), strNetId.c_str());
		return S_FALSE;
	}

	mContentStorethreadpool.resize( pGBCSBaseConfig->_bind.contentStoreThreadPoolSize );
	if ( !initializeCrashDumpLocation() )
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize crash dump location"));
#endif
		return S_FALSE;
	}

	// initialize ice run time
	if( !InitializeIceRunTime() )
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR, _T("failed to initialize ice run time"));
#endif
		return S_FALSE;
	}	

	if (!initializeServiceParameter())
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR, _T("failed to initialize service parameter"));
#endif
		return S_FALSE;
	}	
	
//	if(NULL == GBCSSnmpAgent)
//	{
//#pragma message("TODO:gbcs snmp oid is not 3000, redefine it")
//		GBCSSnmpAgent = new ZQ::Snmp::Subagent(3000, 5, Application->getInstanceId());
//		GBCSSnmpAgent->setLogger(m_pReporter);
//	}

	return ZQ::common::BaseZQServiceApplication::OnInit();
}

HRESULT GBCSService::OnStart()
{
	if (gGBCSConfig._dataBase.runtimePath.empty())
		gGBCSConfig._dataBase.runtimePath = gGBCSConfig._dataBase.path;

	//if (NULL != GBCSSnmpAgent)
	//{
	//	GBCSSnmpAgent->start();
	//	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService, "start(), snmp agent succeed"));
	//}
	//else
	//	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService, "start(), snmp agent failed"));

#ifndef _INDEPENDENT_ADAPTER
    { // publish logs
        for(size_t i = 0; i < pGBCSBaseConfig->_publishedLog._logDatas.size(); ++i) {
            const ZQTianShan::GBCS::PublishLog& pl= pGBCSBaseConfig->_publishedLog._logDatas[i];
            if(pl._path.empty() || pl._syntax.empty() || pl._key.empty()) {
                glog(ZQ::common::Log::L_WARNING, CLOGFMT(GBCSService,"publishLogger() Bad logger configuration. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
                continue;
            }
            if(mainAdapter->publishLogger(pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str())) {
                glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService,"publishLogger() Successfully publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
            } else {
                glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSService,"publishLogger() Failed to publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
            }
        }
    }
#endif

	try
	{
		mainAdapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCS, "activate adapter caught exception: %s"), ex.ice_name().c_str());
	}

	return ZQ::common::BaseZQServiceApplication::OnStart();
}

HRESULT GBCSService::OnStop()
{
	mGBCSEnv.uninitEnv();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService,"ContentStore uninitialized"));

	try
	{
		mainAdapter->deactivate();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBCS, "deactivate adapter caught exception: %s"), ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService,"Adapter deactivated"));

	mContentStorethreadpool.stop();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService,"ThreadPool stopped"));
	//if (NULL != GBCSSnmpAgent)
	//	GBCSSnmpAgent->stop();

	return ZQ::common::BaseZQServiceApplication::OnStop();
}

HRESULT GBCSService::OnUnInit()
{
	UninitializeIceRunTime();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSService, "Ice runtime destroyed"));
	//if (NULL != GBCSSnmpAgent)
	//{
	//	ZQ::Snmp::Subagent*  tmpAgent = GBCSSnmpAgent;
	//	GBCSSnmpAgent = NULL;
	//	delete tmpAgent;
	//	glog(ZQ::common::Log::L_INFO, CLOGFMT(GBCSSvcEnv, "OnUnInit() snmp agent released"));
	//}

	return ZQ::common::BaseZQServiceApplication::OnUnInit();
}

