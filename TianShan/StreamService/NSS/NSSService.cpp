#include "NSSService.h"
#include "ServantFactory.h"
#include "NSSCfgLoader.h"
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif
#include "TianShanIceHelper.h"
#include "IceLog.h"

namespace ZQ{
	namespace StreamService{
		ZQ::StreamService::SsServiceImpl* pServiceInstance = NULL;
		SsEnvironment*	gSsEnvironment = NULL;
	}}


NSSService					gNSSServiceInstance;
ZQ::common::BaseZQServiceApplication		*Application=&gNSSServiceInstance;

ZQ::common::Config::Loader<ZQTianShan::NSS::NSSCfg> gNSSConfig("NSS.xml");

ZQ::common::Config::ILoader					*configLoader = &gNSSConfig;
ZQTianShan::NSS::NSSBaseConfig::NSSHolder	*pNSSBaseConfig = NULL;

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

bool NSSService::initializeCrashDumpLocation( )
{
#ifdef ZQ_OS_MSWIN
	try
	{
		if(! ZQTianShan::Util::fsCreatePath( gNSSConfig._crashDump.path) )
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(NSSService,"can't create path [%s] for mini dump "),
				gNSSConfig._crashDump.path);
			return false;
		}	
		_crashDump.setDumpPath((char *)gNSSConfig._crashDump.path.c_str());
		_crashDump.enableFullMemoryDump(gNSSConfig._crashDump.enabled);
		_crashDump.setExceptionCB(CrashExceptionCallBack);
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "unexpected exception caught when initializeCrashDumpLocation"));
		return false;
	}
#else
	DUMP_PATH = gNSSConfig._crashDump.path.c_str();
#endif
	return true;
}

std::string	NSSService::getLoggerFolder( ) const
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

void NSSService::uninitializeLogger( )
{

}

bool NSSService::initializeLogger( )
{
#ifdef ZQ_OS_MSWIN
	::std::string strLogBaseFolder = m_wsLogFolder;
	std::string svc_name = servname;
#else
	::std::string strLogBaseFolder = _logDir;
	std::string svc_name = getServiceName();
#endif
	::std::string strNSSLog	= strLogBaseFolder + FNSEPS + svc_name + ".sess.log";

	try
	{
// 		mSessionLogger.open(gNSSConfig._logFile.path.c_str(),
// 			gNSSConfig._logFile.level,
// 			ZQLOG_DEFAULT_FILENUM,
// 			gNSSConfig._logFile.size,
// 			gNSSConfig._logFile.bufferSize,
// 			gNSSConfig._logFile.flushTimeout);
#ifdef ZQ_OS_MSWIN
		int32 iLogLevel			= m_dwLogLevel;
		int32 iLogMaxCount		= m_dwLogFileCount;
		int32 iLogSize			= m_dwLogFileSize;
		int32 iLogBufferSize	= m_dwLogBufferSize;
		int32 iLogFlushTimeout	= m_dwLogWriteTimeOut;
#else
		int32 iLogLevel			= _logLevel;
		int32 iLogMaxCount		= ZQLOG_DEFAULT_FILENUM;
		int32 iLogSize			= _logSize;
		int32 iLogBufferSize	= _logBufferSize;
		int32 iLogFlushTimeout	= _logTimeout;
#endif

		mSessionLogger.open(strNSSLog.c_str(),
							iLogLevel,
							iLogMaxCount,
							iLogSize,
							iLogBufferSize,
							iLogFlushTimeout);

		if( gNSSConfig._iceTrace.enabled >= 1 )
		{
			strNSSLog = strLogBaseFolder + FNSEPS + svc_name + "_ice.log";
			mIceFileLogger.open( strNSSLog.c_str() ,
				iLogLevel,
				iLogMaxCount,
				iLogSize,
				iLogBufferSize,
				iLogFlushTimeout);
		}
	}
	catch( const ZQ::common::FileLogException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(NSSService,"failed to open log file[%s] because [%s]"),
			strNSSLog.c_str(),
			ex.what() );
		return false;
	}
	return true;
}

#define ADAPTERNAME_STREAM_SERVICE "StreamService"

bool NSSService::InitializeIceRunTime( )
{
	try
	{
		Ice::InitializationData		iceInitData;
		int i = 0;
		iceInitData.properties =Ice::createProperties( i , NULL );

		std::stringstream ss;
		ss << pNSSBaseConfig->_bind.dispatchSize;
		iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.Size", ss.str());
		ss << pNSSBaseConfig->_bind.dispatchMax;
		iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.SizeMax",ss.str());
		glog(ZQ::common::Log::L_INFO,CLOGFMT(NSSService,"set dispatchSize/dispatchSizeMax to [%d/%d]"),
			pNSSBaseConfig->_bind.dispatchSize,
			pNSSBaseConfig->_bind.dispatchMax);
		//StreamService

		//set ice properties
		::ZQTianShan::NSS::IceProperties::props::const_iterator it = pNSSBaseConfig->_iceProperty._props.begin();
		for( ; it != pNSSBaseConfig->_iceProperty._props.end() ; it ++ )
		{
			iceInitData.properties->setProperty( it->name , it->value );
			glog(ZQ::common::Log::L_INFO,CLOGFMT(NSSService,"Set ice property [%s] \t\t\t= [%s]"),
				it->name.c_str() , it->value.c_str() );
		}
		if( gNSSConfig._iceTrace.enabled >= 1 )
		{
			iceInitData.logger = new TianShanIce::common::IceLogI( &mIceFileLogger );
			assert( iceInitData.logger );
		}

		mIc	=	Ice::initialize( i , NULL , iceInitData );
		return ( mIc );
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "[%s] caught when InitializeIceRunTime"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "unexpected exception caught when InitializeIceRunTime"));
		return false;
	}
}

#define StrToLower(_STR) std::transform(_STR.begin(), _STR.end(), _STR.begin(), (int(*)(int)) tolower);

bool NSSService::initializeServiceParameter( )
{
	
	//
	// init adapter
	//	
	try
	{
		mainAdapter = ZQADAPTER_CREATE( mIc,ADAPTER_NAME_NSS ,pNSSBaseConfig->_bind.endPoint.c_str(), *m_pReporter);
		assert(mainAdapter);
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "Create adapter failed with endpoint=%s and exception is %s"),
			pNSSBaseConfig->_bind.endPoint.c_str(), ex.ice_name().c_str());
		return false;
	}

	//
	// init contentstore
	//
#ifdef ZQ_OS_MSWIN
	mNGODCSEnv.setServiceName(servname);
#else
	mNGODCSEnv.setServiceName(getServiceName().c_str());
#endif

	// fixup spelling in the loaded configuration
	StrToLower(pNSSBaseConfig->_videoServer.customer);

	mNGODCSEnv.setLogPath(getLoggerFolder( ).c_str());
	mNGODCSEnv.setDataPath(gNSSConfig._dataBase.path.c_str());
	mNGODCSEnv.setConfig(pNSSBaseConfig);	
	mNGODCSEnv.setThreadPool(&mContentStorethreadpool);
	mNGODCSEnv.setIceAdapter(mainAdapter);
	if (!mNGODCSEnv.initEnv())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "failed to initialize ContentStore"));
		return false;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService, "ContentStore module initialized successfully"));

	//
	// init streamservice
	//
	mpNSSEnv->mainAdapter		= mainAdapter;
	mpNSSEnv->iceCommunicator	= mIc;

	try
	{
		mpNSSEnv->init();
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "failed to initialize StreamService, caught[%s]"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "failed to initialize StreamService, caught runtime exception"));
		return false;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService, "StreamService module initialized successfully"));

	return true;
}

void NSSService::UninitializeIceRunTime()
{
	try
	{
		mpNSSEnv->iceCommunicator = NULL;
		if(mIc)
		{
			mIc->destroy();
			mIc = NULL;
		}
	}
	catch(...)	{}
}


NSSService::NSSService()
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "NSS");
	strcpy(prodname, "TianShan");
#endif
	mIc				=	NULL;
	mpServiceImpl	=	NULL;
}

NSSService::~NSSService()
{

}
/*

class nssSessionCount: public ZQ::Snmp::IVariable
{
public:
	nssSessionCount(ZQ::StreamService::SsServiceImpl* serviceInstance)
		:_serviceInstance(serviceInstance){};

	~nssSessionCount()
	{
		_serviceInstance = NULL;
	}

	virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
	{
		int sessionCount = _serviceInstance->sessionCount();
		return smivalFrom(val, sessionCount, desiredType);
	}

	virtual bool set(const ZQ::Snmp::SmiValue& val)
	{
		return true;// read only, not set
	}

	virtual bool validate(const ZQ::Snmp::SmiValue& val) const
	{
		return true;
	}

private:
	ZQ::StreamService::SsServiceImpl* _serviceInstance;
};
*/

HRESULT NSSService::OnInit( )
{
	//get regedit argument
	std::string strNetId = "";
	if(m_argc == 1 && strlen(m_argv[0]))
		strNetId = m_argv[0];
	
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSService, "looking for netID[%s] in configuration"), strNetId.c_str());

	for (::ZQTianShan::NSS::NSSBaseConfig::NSSHolderVec::iterator iter = gNSSConfig._nssBaseConfig.NSSVec.begin(); iter != gNSSConfig._nssBaseConfig.NSSVec.end(); iter++)
	{
		if (strNetId == (*iter).netId)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService, "found NetID=%s in configuration"), strNetId.c_str());
			pNSSBaseConfig = &(*iter);
			break;
		}
	}

	if (NULL == pNSSBaseConfig)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService, "couldn't find netID[%s] in configuration"), strNetId.c_str());
		return S_FALSE;
	}

	mStreamServiceThreadpool.resize( pNSSBaseConfig->_bind.threadPoolSize );
	mContentStorethreadpool.resize( pNSSBaseConfig->_bind.contentStoreThreadPoolSize );
	
	_thrdPoolRTSP.resize(max(pNSSBaseConfig->_bind.threadPoolSize/4, 20));

	// fix up some configurations for case-insensive
	std::transform(pNSSBaseConfig->_videoServer.vendor.begin(), pNSSBaseConfig->_videoServer.vendor.end(), pNSSBaseConfig->_videoServer.vendor.begin(), (int(*)(int)) tolower);
	std::transform(pNSSBaseConfig->_videoServer.customer.begin(), pNSSBaseConfig->_videoServer.customer.end(), pNSSBaseConfig->_videoServer.customer.begin(), (int(*)(int)) tolower);
	
	if ( !initializeCrashDumpLocation() )
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize crash dump location"));
#endif
		return S_FALSE;
	}

	if (!initializeLogger())
	{
#ifdef ZQ_OS_MSWIN
		logEvent( ZQ::common::Log::L_ERROR,_T("failed to open log"));
#endif
		return S_FALSE;
	}

	mpNSSEnv = new ZQTianShan::NGODSS::NSSEnv(*m_pReporter, mSessionLogger, _thrdPoolRTSP, mStreamServiceThreadpool);
	mpNSSEnv->setNetId(strNetId);

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
	
	ZQ::StreamService::gSsEnvironment	=	mpNSSEnv;
	mpServiceImpl = new ZQ::StreamService::SsServiceImpl( mpNSSEnv , "NSS" );	
	ZQ::StreamService::pServiceInstance	= mpServiceImpl;
	mpServiceImpl->strCheckpointPeriod	=	pNSSBaseConfig->_videoServer.streamDbPerfTune.databaseStreamCheckpointPeriod;
	mpServiceImpl->strSavePeriod		=	pNSSBaseConfig->_videoServer.streamDbPerfTune.databaseStreamSavePeriod;
	mpServiceImpl->strSaveSizeTrigger	=	pNSSBaseConfig->_videoServer.streamDbPerfTune.databaseStreamSaveSizeTrigger;

	return ZQ::common::BaseZQServiceApplication::OnInit();
}

void NSSService::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();

	//{".2", "nssSvcApp" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).nss(1400).nssSvcApp(2)
	//{".2.1", "nssAttr" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).nss(1400).nssSvcApp(2).nssAttr(1)
	//{".2.1.100", "nssStat" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).nss(1400).nssSvcApp(2).nssAttr(1).nssStat(100)
	//{".2.1.100.2", "nssStat-cSessions" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).nss(1400).nssSvcApp(2).nssAttr(1).nssStat(100).nssStat-SessionCount(2)
	ServiceMIB_ExportByAPI(_pServiceMib, "nssStat-cSessions", NSSService, *this, uint32, AsnType_Int32, &NSSService::getSessionCount, NULL, "");
}

HRESULT NSSService::OnStart()
{
	if (gNSSConfig._dataBase.runtimePath.empty())
		gNSSConfig._dataBase.runtimePath = gNSSConfig._dataBase.path;
	
	std::string dbPath = ZQTianShan::Util::fsConcatPath( gNSSConfig._dataBase.runtimePath, mNGODCSEnv._strServiceName);

	if (!mpServiceImpl->start(dbPath, gNSSConfig._iceStorm.endPoint, "", mpNSSEnv->_netId, mNGODCSEnv._strServiceName))
	{
		glog(ZQ::common::Log::L_EMERG,CLOGFMT(NSSService,"failed to start service instance"));
		return -1;
	}

	ZQ::common::RTSPSession::startDefaultSessionManager();

#ifndef _INDEPENDENT_ADAPTER
    { // publish logs
        for(size_t i = 0; i < pNSSBaseConfig->_publishedLog._logDatas.size(); ++i) {
            const ZQTianShan::NSS::PublishLog& pl= pNSSBaseConfig->_publishedLog._logDatas[i];
            if(pl._path.empty() || pl._syntax.empty() || pl._key.empty()) {
                glog(ZQ::common::Log::L_WARNING, CLOGFMT(NSSService,"publishLogger() Bad logger configuration. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
                continue;
            }
            if(mainAdapter->publishLogger(pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str())) {
                glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService,"publishLogger() Successfully publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
            } else {
                glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService,"publishLogger() Failed to publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
            }
        }
    }
#endif

	mpNSSEnv->start();
    mpNSSEnv->getMainAdapter()->activate();
	return ZQ::common::BaseZQServiceApplication::OnStart();
}

HRESULT NSSService::OnStop()
{
#ifndef _INDEPENDENT_ADAPTER
	{ // unpublish logs
		for(size_t i = 0; i < pNSSBaseConfig->_publishedLog._logDatas.size(); ++i) {
			const ZQTianShan::NSS::PublishLog& pl= pNSSBaseConfig->_publishedLog._logDatas[i];
			if(pl._path.empty() || pl._syntax.empty() || pl._key.empty()) {
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(NSSService,"OnStop() Bad logger configuration. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
				continue;
			}
			if(mainAdapter->unpublishLogger(pl._path.c_str())) {
				glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService,"unpublishLogger() Successfully unpublish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
			} else {
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(NSSService,"unpublishLogger() Failed to unpublish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
			}
		}
	}
#endif

	if (mpServiceImpl )
	{
		mpServiceImpl->stop();
		//should I delete service instance or just leave it to Ice runtime ?
		glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService,"NSSService stopped"));
	}

	mNGODCSEnv.uninitEnv();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService,"ContentStore uninitialized"));

	ZQ::common::RTSPSession::stopDefaultSessionManager();

	if( mpNSSEnv->mainAdapter )
		mpNSSEnv->mainAdapter->deactivate();	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService,"Adapter deactivated"));

	mStreamServiceThreadpool.stop();
	mContentStorethreadpool.stop();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService,"ThreadPool stopped"));

	return ZQ::common::BaseZQServiceApplication::OnStop();
}

HRESULT NSSService::OnUnInit()
{
	UninitializeIceRunTime();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService, "Ice runtime destroyed"));
	
	mpNSSEnv->uninit();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService, "NSS enviroment uninitialized"));
	if(mpNSSEnv)
	{		
		delete mpNSSEnv;
		mpNSSEnv = NULL;
	}

	if (mpServiceImpl )
	{
		delete mpServiceImpl;
		mpServiceImpl = NULL;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(NSSService,"NSSService uninitialized"));
	}

	uninitializeLogger();
	return ZQ::common::BaseZQServiceApplication::OnUnInit();
}

