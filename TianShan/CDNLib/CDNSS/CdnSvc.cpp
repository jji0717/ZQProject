#include <boost/thread.hpp>
#include "CdnSvc.h"
#include "CdnSSConfig.h"
#include "TianShanIceHelper.h"
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif

#ifndef ZQ_CDN_UMG
extern Ice::ObjectPrx StartContentStore(ZQADAPTER_DECLTYPE& adapter, const std::string&, const std::string&, ZQ::common::Log& log); 
extern bool StopContentStore (ZQ::common::Log& log);
#define ZQ_App_Name getServiceName()
#else
#include "HttpC2Streamer.h"
#define ZQ_App_Name getServiceName()
#endif

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType		=		1110;;
DWORD gdwServiceInstance	=		1;
static ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
    DWORD dwThreadID = GetCurrentThreadId();

    glog( ZQ::common::Log::L_CRIT, 
        _T("Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x"),
        ExceptionCode, ExceptionAddress, dwThreadID);
    glog.flush();	
}
#else
extern const char* DUMP_PATH;
#endif

//extern int  RegisterCacheStoreSnmp( );
extern void cache_resetSnmpStat(ZQ::common::ServiceMIB::Ptr pMib);
extern void cache_refreshSnmpStat(ZQ::common::ServiceMIB::Ptr pMib, int iStep);

namespace ZQ{
namespace StreamService{
	SsEnvironment*	gSsEnvironment = NULL;
}}

ZQ::StreamService::CdnSSSerice					gCdnSSServiceInstance;
ZQ::common::BaseZQServiceApplication *Application= &gCdnSSServiceInstance;

#define SERVICELOG (glog)
ZQ::common::Config::Loader< ZQ::StreamService::CdnSSConfig> gCdnSSConfig("CdnSS" ".xml");

ZQ::common::Config::ILoader			*configLoader = &gCdnSSConfig;

#define SERVICE_ADAPTER_NAME "StreamService"

namespace ZQ {
namespace StreamService {

CdnSSSerice::CdnSSSerice()
	:mCdnEnv( NULL ),
	mbOnStopInvoked(false)
{
	mIc				=	NULL;
	mpServiceImpl	=	NULL;
}

CdnSSSerice::~CdnSSSerice( )
{
	if(mCdnEnv)
	{
		delete mCdnEnv;
		mCdnEnv = 0;
	}
}

void CdnSSSerice::uninitializeLogger( )
{

}

bool CdnSSSerice::initializeLogger( )
{
	try
	{
		mSessionLogger.open((getLoggerFolder()+ ZQ_App_Name + ".Sess.log").c_str(),
			gCdnSSConfig.sessionLogLevel,
			ZQLOG_DEFAULT_FILENUM,
			gCdnSSConfig.sessionLogFileSize,
			gCdnSSConfig.sessionLogBufferSize,
			gCdnSSConfig.sessionLogTimeout);
	}
	catch( const ZQ::common::FileLogException& ex)
	{
		SERVICELOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(CdnSSSerice,"failed to open session log file[%s] because [%s]"),
			(getLoggerFolder()+ ZQ_App_Name + ".Sess.log").c_str(),
			ex.what() );
		return false;
	}
	
	return true;
}

bool CdnSSSerice::InitializeIceRunTime() {
	Ice::InitializationData		iceInitData;
	int i = 0;
	iceInitData.properties =Ice::createProperties( i , NULL );

	//set ice properties
	int threadSize = 10;
	int threadSizeMax = 20;
	
	ZQTianShan::Util::getPropertyDataWithDefault( gCdnSSConfig.iceProperties,"Ice.ThreadPool.Server.Size",10,threadSize);
	ZQTianShan::Util::getPropertyDataWithDefault( gCdnSSConfig.iceProperties,"Ice.ThreadPool.Server.SizeMax",20,threadSizeMax);

	threadSize = MAX(threadSize , atoi(gCdnSSConfig.dispatchThreadCount.c_str()) );
	threadSizeMax = MAX(threadSizeMax , atoi(gCdnSSConfig.dispatchThreadCountMax.c_str()) );

	gCdnSSConfig.iceProperties.erase("Ice.ThreadPool.Server.Size");
	gCdnSSConfig.iceProperties.erase("Ice.ThreadPool.Server.SizeMax");

	ZQTianShan::Util::updatePropertyData(gCdnSSConfig.iceProperties,SERVICE_ADAPTER_NAME".ThreadPool.Size",threadSize);
	ZQTianShan::Util::updatePropertyData(gCdnSSConfig.iceProperties,SERVICE_ADAPTER_NAME".ThreadPool.SizeMax",threadSizeMax);

	std :: map<std::string, std::string>::const_iterator it = gCdnSSConfig.iceProperties.begin();
	for( ; it != gCdnSSConfig.iceProperties.end() ; it ++ )
	{		
		iceInitData.properties->setProperty( it->first , it->second );
		SERVICELOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnSSSerice,"Set ice property [%s] \t\t\t= [%s]"),
			it->first.c_str() , it->second.c_str() );
	}


	if( gCdnSSConfig.enableIceTrace >= 1 )
	{
		try
		{
			mIceFileLogger.open( (getLoggerFolder()+ ZQ_App_Name + ".IceTrace.log").c_str() ,
									gCdnSSConfig.iceTraceLevel,
									ZQLOG_DEFAULT_FILENUM,
									gCdnSSConfig.iceTraceLogSize,
									10240,
									2);
		}
		catch( const ZQ::common::FileLogException& ex)
		{
			SERVICELOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(CdnSSSerice,"failed to open ice trace log file[%s] because [%s]"),
				(getLoggerFolder()+ ZQ_App_Name + ".IceTrace.log").c_str(),
				ex.what() );
			return false;
		}
		iceInitData.logger = new TianShanIce::common::IceLogI( &mIceFileLogger );
		assert( iceInitData.logger );

	}

	mIc	=	Ice::initialize( i , NULL , iceInitData );
	return ( mIc != 0 );
}

void CdnSSSerice::UninitializeIceRunTime()
{
	SERVICELOG(ZQ::common::Log::L_INFO, CLOGFMT(CdnSSSerice,"uninit ice runtime"));
	try
	{
		mCdnEnv->iceCommunicator = NULL;
		mCdnEnv->mainAdapter = NULL;
		if(mIc)
		{
			mIc->destroy();
			mIc = NULL;
		}
	}
	catch(const std::exception& ex)
	{
		SERVICELOG(ZQ::common::Log::L_ERROR, CLOGFMT(CdnSSSerice,"uninit ice runtime, got an exceptino: %s"),ex.what());
	}

	SERVICELOG(ZQ::common::Log::L_INFO, CLOGFMT(CdnSSSerice,"uninit ice runtime, done"));
}

std::string	CdnSSSerice::getLoggerFolder( ) const 
{
	std::string folderPath = ZQ_App_LogDir;
	if (folderPath.at(folderPath.length()-1) != '/') 
	{
		folderPath	+=	"/";
	}
	return folderPath;
}

bool CdnSSSerice::initializeServiceParameter( ) 
{
	try
	{
		mCdnEnv->mainAdapter = ZQADAPTER_CREATE( mIc , SERVICE_ADAPTER_NAME , gCdnSSConfig.serviceEndpoint.c_str() , *m_pReporter );
		assert(mCdnEnv->mainAdapter);
	}
	catch( const Ice::Exception& ex)
	{
		SERVICELOG(ZQ::common::Log::L_ERROR,CLOGFMT(CdnSSSerice,"initializeServiceParameter() failed to create adapter at [%s] due to [%s]"),
			gCdnSSConfig.serviceEndpoint.c_str(), ex.ice_name().c_str() );
		return false;
	}

	mCdnEnv->mTransferServerHttpIp					=	gCdnSSConfig.c2StreamerConfig.httpBindIp;
	mCdnEnv->mTransferServerHttpPort				=	gCdnSSConfig.c2StreamerConfig.httpBindPort;

	mCdnEnv->streamsmithConfig.iUseMemoryDB			=	1;
	mCdnEnv->streamsmithConfig.iRenewTicketInterval	=	gCdnSSConfig.ticketRenewInterval;

	mCdnEnv->streamsmithConfig.iPreloadTimeInMS		=	0;
	mCdnEnv->streamsmithConfig.iSupportPlaylist		=	ZQ::StreamService::LIB_SUPPORT_NORMAL_STREAM;
	mCdnEnv->streamsmithConfig.iPlaylistTimeout		=	gCdnSSConfig.sessionTimeout;
	mCdnEnv->iceCommunicator						=	mIc;

	mCdnEnv->mNetId									=	gCdnSSConfig.netId;
	mCdnEnv->mHttpLog								=	m_pReporter;
	return true;
}

bool CdnSSSerice::startServer( )
{
#ifndef ZQ_CDN_UMG
    Ice::ObjectPrx prx = StartContentStore( mCdnEnv->mainAdapter, ZQ_App_Name, ZQ_App_LogDir, SERVICELOG);
    if(!prx) {
		SERVICELOG(ZQ::common::Log::L_ERROR,CLOGFMT(CdnSSSerice,"failed to start contentstore"));
		return false;
	}
    mCdnEnv->mCsPrx = TianShanIce::Storage::ContentStorePrx::uncheckedCast(prx);
#else
    C2Streamer::getEnvironment()->logger = m_pReporter;
#endif
	mpServiceImpl = new ZQ::StreamService::SsServiceImpl( mCdnEnv, ZQ_App_Name );

	ZQ::StreamService::CdnStreamerManager* streamerManager = new ZQ::StreamService::CdnStreamerManager( mCdnEnv,*mpServiceImpl);
	mCdnEnv->mStreamerManager = streamerManager;
	if(!streamerManager->startup()) {
       return false; 
    }

	int32 threadCount = gCdnSSConfig.mainThreadCount;
	threadCount = threadCount < 3 ? 3 : threadCount;

	mCdnEnv->getMainThreadPool().resize(threadCount);

	mpServiceImpl->strCheckpointPeriod		= gCdnSSConfig.perfCheckpointPeriod;
	mpServiceImpl->strDbRecoverFatal		= gCdnSSConfig.perfDbRecoverFatal;
	mpServiceImpl->strSavePeriod			= gCdnSSConfig.perfSavePeriod ;
	mpServiceImpl->strSaveSizeTrigger		= gCdnSSConfig.perfSaveSizeTrigger ;
	mpServiceImpl->iEvictorStreamSize		= gCdnSSConfig.perfSessionCacheSize;
	mpServiceImpl->iReplicaReportInterval	= gCdnSSConfig.streamerReplicaConfig.defaultUpdateInterval;
	mpServiceImpl->bShowDBOperationTimeCost	= true;

	assert( mpServiceImpl != NULL );
	//	assert( mCdnEnv->mCsPrx != NULL );

	IceUtil::Handle<ZQ::StreamService::SsServiceImpl> svc= mpServiceImpl;
	//mCdnEnv->getCommunicator()->addObjectFactory( new ZQ::StreamService::StreamFactory( mCdnEnv , *mpServiceImpl ), 
	//	TianShanIce::Streamer::SsPlaylist::ice_staticId() );

	//mCdnEnv->getMainAdapter()->ZQADAPTER_ADD(mCdnEnv->getCommunicator() , svc , ZQ_App_Name);
	
	std::string cdnDbPath = ZQTianShan::Util::fsConcatPath(gCdnSSConfig.dbPath, ZQ_App_SvcName);
	
	if(!mpServiceImpl->start( cdnDbPath ,
		gCdnSSConfig.eventChannelEndpoint,
		gCdnSSConfig.streamerReplicaConfig.listenerEndpoint,
		gCdnSSConfig.netId,
		ZQ_App_Name))
	{
		SERVICELOG(ZQ::common::Log::L_ERROR,CLOGFMT(CdnSSSerice,"failed to start service instance"));
		return false;
	}

	//publish log
	{
		//std::vector<MonitoredLog>	monitoredLogs;
		const std::vector< MonitoredLog >& monLogs =  gCdnSSConfig.monitoredLogs;
		std::vector< MonitoredLog >::const_iterator itLogPub = monLogs.begin();
		for( ; itLogPub != monLogs.end(); ++itLogPub )
		{
			ZQTianShan::Adapter::publishLogger( itLogPub->name.c_str(), itLogPub->syntax.c_str() , itLogPub->key.c_str(), itLogPub->type.c_str());
			SERVICELOG(ZQ::common::Log::L_DEBUG,CLOGFMT(CdnSSService,"publish log [%s],[%s],[%s],[%s]"),
				itLogPub->name.c_str(), itLogPub->syntax.c_str() , itLogPub->key.c_str(),itLogPub->type.c_str());
		}
	}
	
	mCdnEnv->getMainAdapter()->activate();

	gCdnSSConfig.snmpRegister("");

	SERVICELOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnSSSerice,"Service start OK"));
	return true;

}
HRESULT CdnSSSerice::OnInit( )
{
#ifdef ZQ_OS_MSWIN
    if(! ZQTianShan::Util::fsCreatePath( gCdnSSConfig.crashDumpPath) )
    {
        SERVICELOG(ZQ::common::Log::L_ERROR,CLOGFMT(CdnSSService,"can't create path [%s] for mini dump "),
            gCdnSSConfig.crashDumpPath.c_str());
        return S_FALSE;
    }	
    _crashDump.setDumpPath( (char*)gCdnSSConfig.crashDumpPath.c_str() );
    _crashDump.enableFullMemoryDump( gCdnSSConfig.enableCrashDump );
    _crashDump.setExceptionCB(CrashExceptionCallBack);
#else
    DUMP_PATH = gCdnSSConfig.crashDumpPath.c_str();

    // set ulimit, bug#21095
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = 10000;
    int rc = setrlimit(RLIMIT_NOFILE, &rt);
#endif

	if(!initializeLogger()) {
		SERVICELOG( ZQ::common::Log::L_ERROR,_T("failed to open log"));
		return S_FALSE;
	}

	if(mCdnEnv) { delete mCdnEnv;mCdnEnv = 0 ;}
	mCdnEnv = new CdnSsEnvironment(*m_pReporter, mSessionLogger , mThreadPool );

	//assign global environment to StreamSmith environment
	ZQ::StreamService::gSsEnvironment	=	mCdnEnv;

	//ticketRenewThreadpoolSize
	gCdnSSConfig.ticketRenewThreadpoolSize = MAX(gCdnSSConfig.ticketRenewThreadpoolSize,3);
	mCdnEnv->getRenewTicketThreadPool().resize(gCdnSSConfig.ticketRenewThreadpoolSize);

	if(!InitializeIceRunTime()) {
		SERVICELOG(ZQ::common::Log::L_ERROR,_T("failed to initialize ice run time"));
		return S_FALSE;
	}

	if(!initializeServiceParameter()) {
		SERVICELOG(ZQ::common::Log::L_ERROR,_T("failed to initialize service parameter"));
		return S_FALSE;
	}

    return ZQ::common::BaseZQServiceApplication::OnInit();
}

HRESULT CdnSSSerice::OnStart()
{
    SERVICELOG(ZQ::common::Log::L_INFO,_T("OnStart() enter"));

	if(!startServer()) {
		SERVICELOG(ZQ::common::Log::L_ERROR,_T("failed to start service"));
		return S_FALSE;
	}

	return ZQ::common::BaseZQServiceApplication::OnStart();
}

HRESULT CdnSSSerice::OnStop()
{
	mbOnStopInvoked = true;
    SERVICELOG(ZQ::common::Log::L_INFO,_T("OnStop() enter"));

#ifndef ZQ_CDN_UMG
	StopContentStore(glog);
	mCdnEnv->mCsPrx = NULL;
	SERVICELOG(ZQ::common::Log::L_INFO,_T("OnStop() contentstore stopped"));
#endif

	mCdnEnv->mainAdapter->deactivate();
	SERVICELOG(ZQ::common::Log::L_INFO,_T("OnStop() adapter deactivate"));
    
	mCdnEnv->getRenewTicketThreadPool().stop();

	mCdnEnv->getMainThreadPool().stop();

	if(mpServiceImpl) 
	{
		mpServiceImpl->stop();
		//should I delete service instance or just leave it to Ice runtime ?
		mpServiceImpl = NULL;
    }
    SERVICELOG(ZQ::common::Log::L_INFO,_T("OnStop() service stopped"));

	
	if(mCdnEnv->mStreamerManager)
	{ 
        mCdnEnv->mStreamerManager->shutdown();
    }
    SERVICELOG(ZQ::common::Log::L_INFO,_T("OnStop() streamer manager stopped"));

    return ZQ::common::BaseZQServiceApplication::OnStop();
}


HRESULT CdnSSSerice::OnUnInit() {
	if(!mbOnStopInvoked)
	{
		OnStop();
	}
    SERVICELOG(ZQ::common::Log::L_INFO,_T("OnUnInit() enter"));

	UninitializeIceRunTime();
	uninitializeLogger();
	if(mCdnEnv->mStreamerManager)
	{
		delete mCdnEnv->mStreamerManager;
		mCdnEnv->mStreamerManager = NULL;
	}

	return ZQ::common::BaseZQServiceApplication::OnUnInit();
}

void CdnSSSerice::snmpResetCacheStat( const uint32& )
{ 
	cache_resetSnmpStat(_pServiceMib);
}

void CdnSSSerice::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();
	ServiceMIB_ExportByAPI(_pServiceMib, "cdnssStat-cache-Measure-Reset", CdnSSSerice, *this, uint32, AsnType_Int32, &CdnSSSerice::snmpDummyGet, &CdnSSSerice::snmpResetCacheStat, "");

	snmpResetCacheStat(0);
	//cache_refreshSnmpStat(_pServiceMib);
}

bool CdnSSSerice::isHealth(void)
{
	static int64 stampLastRefreshCacheStat = 0;
	int64 stampNow = ZQ::common::now();
	if (stampNow - stampLastRefreshCacheStat > 30000)
	{
		static int iStep =0;
		cache_refreshSnmpStat(_pServiceMib, iStep++);

		if (iStep>10)
		{
			stampLastRefreshCacheStat = stampNow;
			iStep = 0;
		}
	}

	return BaseZQServiceApplication::isHealth();
}

}}
