
#include "StreamSmithService.h"
#include "StreamSmithConfig.h"
#include <embededContentStore.h>
#include "ServantFactory.h"
#include <TianShanIceHelper.h>
#include <MiniDump.h>

#include <memoryDebug.h>

namespace ZQ{
namespace StreamService{
ZQ::StreamService::SsServiceImpl* pServiceInstance = NULL;
SsEnvironment*	gSsEnvironment = NULL;
}}


DWORD gdwServiceType		=		1110;;
DWORD gdwServiceInstance	=		1;

StreamSmithService					gStreamSmithServiceInstance;
extern ZQ::common::BaseZQServiceApplication		*Application=&gStreamSmithServiceInstance;

#define SERVICELOG (*m_pReporter)

ZQ::common::Config::Loader<StreamSmithCfg> gStreamSmithConfig("StreamSmith.xml");

//StreamSmithConfig					gDummyConfig;
ZQ::common::Config::ILoader					*configLoader = &gStreamSmithConfig;


StreamSmithService::StreamSmithService( )
{
	mIc				=	NULL;
	mpServiceImpl	=	NULL;
}
StreamSmithService::~StreamSmithService( )
{

}
int StreamSmithService::run( )
{
	uint32 waitInterval = 1000;
	ZQ::StreamService::VstrmStreamerManager& manager	=	mStreamSmithEnv.getStreamerManager();
	while ( !manager.isVstrmNodeReady() )
	{
		SERVICELOG(ZQ::common::Log::L_WARNING,CLOGFMT(StreamSmithService,"node is not ready"));
		ZQ::common::delay( waitInterval );
		waitInterval = waitInterval << 1;
		if (waitInterval >= 60 * 1000)
		{
			waitInterval = ( 60*1000 );
		}
	};
	SERVICELOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamSmithService,"node is good to go"));
	//start node content store
	try
	{
		mStreamSmithEnv.mCsPrx	= TianShanIce::Storage::ContentStorePrx::checkedCast( ZQ::StreamSmith::NCSBridge::StartContentStore( mStreamSmithEnv.mainAdapter , "" , *mStreamSmithEnv.mainLogger, NULL ));
		
	}
	catch(const Ice::Exception& ex)
	{
		SERVICELOG( ZQ::common::Log::L_CRIT,CLOGFMT(StreamSmithService,"failed to start NodeContentstore because [%s]"),ex.ice_name().c_str() );
		return -1;
	}


	mpServiceImpl = new ZQ::StreamService::SsServiceImpl( &mStreamSmithEnv ,"StreamSmith" );
	ZQ::StreamService::pServiceInstance	= mpServiceImpl;

	mStreamSmithEnv.mStreamerManager.attachServiceInstance( mpServiceImpl );
	mStreamSmithEnv.mStreamerManager.initialize();	
	mStreamSmithEnv.mainScheduler.start();
	mStreamSmithEnv.mSessionScaner.attachServiceInstance( mpServiceImpl );
	mStreamSmithEnv.mSessionScaner.start( );
	mStreamSmithEnv.mIdxParserEnv.InitVstrmEnv();
	mStreamSmithEnv.mIdxParserEnv.AttchLogger(m_pReporter);

	
	mStreamSmithEnv.mIdxParserEnv.setUseVstrmIndexParseAPI(  gStreamSmithConfig.serverMode == SERVER_MODE_EDGE ); //use VstrmLoadAssetINfo only when in EdgeServer


	mpServiceImpl->strCheckpointPeriod			= gStreamSmithConfig.szIceEnvCheckPointPeriod;
	mpServiceImpl->strDbRecoverFatal			= "1";
	mpServiceImpl->strSavePeriod				= gStreamSmithConfig.szFreezePlaylistSavePeriod;
	mpServiceImpl->strSaveSizeTrigger			= gStreamSmithConfig.szFreezePlaylistSaveSizeTrigger;
	mpServiceImpl->iEvictorStreamSize			= gStreamSmithConfig.lEvictorPlaylistSize;
	mpServiceImpl->bShowDBOperationTimeCost		= true;

	assert( mpServiceImpl != NULL );
	assert( mStreamSmithEnv.mCsPrx != NULL );

// 	IceUtil::Handle<ZQ::StreamService::SsServiceImpl> svc= mpServiceImpl;
// 	mStreamSmithEnv.iceCommunicator->addObjectFactory( new ZQ::StreamService::ServantFactory( &mStreamSmithEnv , *mpServiceImpl ), TianShanIce::Streamer::SsPlaylist::ice_staticId() );
// 	mStreamSmithEnv.getMainAdapter()->ZQADAPTER_ADD(mStreamSmithEnv.iceCommunicator , svc , "StreamSmith");
	if(!mpServiceImpl->start( gStreamSmithConfig.szFailOverDatabasePath , gStreamSmithConfig.szEventChannelEndpoint,
						gStreamSmithConfig.spigotReplicaConfig.listenerEndpoint, gStreamSmithConfig.szServiceID,
						"StreamSmith"))
	{
		SERVICELOG(ZQ::common::Log::L_EMERG,CLOGFMT(StreamSmithService,"failed to start service instance"));
		return -1;
	}


	mStreamSmithEnv.getMainAdapter()->activate();

	ZQ::StreamSmith::NCSBridge::mountContentStore();

	SERVICELOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamSmithService,"Service start OK"));
	return 1;
}

void StreamSmithService::adjustConfiguration( )
{
	if( stricmp(gStreamSmithConfig.strServerMode.c_str() , "nPVRServer") == 0 )
	{
		logEvent(ZQ::common::Log::L_INFO,_T("Run as nPVR Server"));
		gStreamSmithConfig.serverMode = SERVER_MODE_NPVR;
	}
	else if( stricmp(gStreamSmithConfig.strServerMode.c_str() , "EdgeServer") == 0 )
	{
		logEvent(ZQ::common::Log::L_INFO,_T("Run as Edge Server"));
		gStreamSmithConfig.serverMode = SERVER_MODE_EDGE;
	}
	else
	{
		logEvent(ZQ::common::Log::L_INFO,_T("Run as Normal Server"));
		gStreamSmithConfig.serverMode = SERVER_MODE_NORMAL;
	}
}

void StreamSmithService::uninitializeLogger( )
{

}

bool StreamSmithService::initializeLogger( )
{
	try
	{
		mSessionLogger.open((getLoggerFolder()+"StreamSmith.Sess.log").c_str(),
							gStreamSmithConfig.lSessMonLogLevel,
							ZQLOG_DEFAULT_FILENUM,
							gStreamSmithConfig.lSessMonLogSize,
							gStreamSmithConfig.lSessMonLogBuffer,
							gStreamSmithConfig.lSessMonLogTimeout);
	}
	catch( const ZQ::common::FileLogException& ex)
	{
		SERVICELOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(StreamSmithService,"failed to open session log file[%s] because [%s]"),
			(getLoggerFolder()+"StreamSmith.Sess.log").c_str(),
			ex.what() );
		return false;
	}
	return true;
}

#define ADAPTERNAME_STREAM_SERVICE "StreamService"

bool StreamSmithService::InitializeIceRunTime( )
{
	Ice::InitializationData		iceInitData;
	int i = 0;
	iceInitData.properties =Ice::createProperties( i , NULL );

	iceInitData.properties->setProperty( ADAPTERNAME_STREAM_SERVICE".ThreadPool.Size", gStreamSmithConfig.szAdapterThreadpoolSize);
	iceInitData.properties->setProperty( ADAPTERNAME_STREAM_SERVICE".ThreadPool.SizeMax",gStreamSmithConfig.szAdapterThreadpoolSizeMax);
	SERVICELOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamSmithService,"set dispatchSize/dispatchSizeMax to [%s/%s]"),
		gStreamSmithConfig.szAdapterThreadpoolSize,
		gStreamSmithConfig.szAdapterThreadpoolSizeMax);
	//StreamService

	//set ice properties
	std :: map<std::string, std::string>::const_iterator it = gStreamSmithConfig.iceProperties.begin();
	for( ; it != gStreamSmithConfig.iceProperties.end() ; it ++ )
	{
		iceInitData.properties->setProperty( it->first , it->second );
		SERVICELOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamSmithService,"Set ice property [%s] \t\t\t= [%s]"),
				it->first.c_str() , it->second.c_str() );
	}
	if( gStreamSmithConfig.lEnableIceTrace >= 1 )
	{
		try
		{
			mIceFileLogger.open( (getLoggerFolder()+"StreamSmith.IceTrace.log").c_str() ,
				gStreamSmithConfig.lIceLogLevel,
				ZQLOG_DEFAULT_FILENUM,
				gStreamSmithConfig.lIceLogSize,
				gStreamSmithConfig.lIceLogBuffer,
				gStreamSmithConfig.lIceLogTimeout);
		}
		catch( const ZQ::common::FileLogException& ex)
		{
			SERVICELOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(StreamSmithService,"failed to open ice trace log file[%s] because [%s]"),
				(getLoggerFolder()+"StreamSmith.IceTrace.log").c_str(),
				ex.what() );
			return false;
		}
		iceInitData.logger = new TianShanIce::common::IceLogI( &mIceFileLogger );
		assert( iceInitData.logger != NULL );
		
	}
	
	mIc	=	Ice::initialize( i , NULL , iceInitData );
	return ( mIc != NULL );
}

void StreamSmithService::UninitializeIceRunTime()
{
	try
	{
		mStreamSmithEnv.iceCommunicator = NULL;
		if(mIc)
		{
			mIc->destroy();
			mIc = NULL;
		}
	}
	catch(...)	{}
}

std::string	StreamSmithService::getLoggerFolder( ) const
{
	std::string folderPath = m_wsLogFolder;
	if ( folderPath.at(folderPath.length()-1) != '\\' ||
		folderPath.at(folderPath.length()-1) != '/') 
	{
#ifdef ZQ_OS_MSWIN
		folderPath	+=	"\\";
#else //
		folderPath	+=	"/";
#endif//ZQ_OS
	}
	return folderPath;
}

#ifdef ZQ_OS_MSWIN
static ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog( ZQ::common::Log::L_CRIT, 
		_T("Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x"),
		ExceptionCode, ExceptionAddress, dwThreadID);
	glog.flush();	
}
#endif//OS_ZQ_MSWIN

bool StreamSmithService::initializeCrashDumpLocation( )
{
#ifdef ZQ_OS_MSWIN
	if(! ZQTianShan::Util::fsCreatePath( gStreamSmithConfig.szMiniDumpPath) )
	{
		SERVICELOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamSmithService,"can't create path [%s] for mini dump "),
			gStreamSmithConfig.szMiniDumpPath);
		return false;
	}	
	_crashDump.setDumpPath(gStreamSmithConfig.szMiniDumpPath);
	_crashDump.enableFullMemoryDump(gStreamSmithConfig.lEnableMiniDump);
	_crashDump.setExceptionCB(CrashExceptionCallBack);
	return true;
#else//OS
	return true;
#endif
}


bool StreamSmithService::initializeServiceParameter( )
{
	mStreamSmithEnv.mainLogger				= m_pReporter;
	mStreamSmithEnv.sessLogger				= &mSessionLogger;
	
	mStreamSmithEnv.mainAdapter = ZQADAPTER_CREATE( mIc,ADAPTERNAME_STREAM_SERVICE ,gStreamSmithConfig.szSvcEndpoint , *m_pReporter);
	assert(mStreamSmithEnv.mainAdapter);

	mStreamSmithEnv.streamsmithConfig.iPreloadTimeInMS		= gStreamSmithConfig.lPreloadTime;
	mStreamSmithEnv.streamsmithConfig.iSupportPlaylist		= ZQ::StreamService::LIB_SUPPORT_NORMAL_PLAYLIST;
	mStreamSmithEnv.streamsmithConfig.iPlaylistTimeout		= gStreamSmithConfig.lPlaylistTimeout;
	mStreamSmithEnv.streamsmithConfig.iRenewTicketInterval	= gStreamSmithConfig.lTicketRenewInterval ;//need a configuration
	mStreamSmithEnv.streamsmithConfig.iEOTSize				= gStreamSmithConfig.lEOTProtectionTime;
	mStreamSmithEnv.iceCommunicator							= mIc;

	return true;
}

HRESULT StreamSmithService::OnInit( )
{
#ifdef REG_DEBUG_NEW
	REG_DEBUG_NEW
#endif

	//assign global environment to StreamSmith environment
	ZQ::StreamService::gSsEnvironment	=	&mStreamSmithEnv;

	ZQ::common::NativeThreadPool* pool = new ZQ::common::NativeThreadPool();
	assert( pool != NULL );
	mStreamSmithEnv.mainThreadPool	= pool;

	if( !initializeCrashDumpLocation() )
	{
		logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize crash dump location"));
		return S_FALSE;
	}

	if(!initializeLogger())
	{
		logEvent( ZQ::common::Log::L_ERROR,_T("failed to open log"));
		return S_FALSE;
	}
	//initialize ice run time
	if( !InitializeIceRunTime() )
	{
		logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize ice run time"));
		return S_FALSE;
	}

	if(!initializeServiceParameter())
	{
		logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize service parameter"));
	}
	
	adjustConfiguration();

	start();//start thread to check if node is ready
	return ZQ::common::BaseZQServiceApplication::OnInit();
}

HRESULT StreamSmithService::OnStart()
{
	return ZQ::common::BaseZQServiceApplication::OnStart();
}

HRESULT StreamSmithService::OnStop()
{
	if( mpServiceImpl )
	{
		mpServiceImpl->stop();
		//should I delete service instance or just leave it to Ice runtime ?
		mpServiceImpl = NULL;
		ZQ::StreamSmith::NCSBridge::StopContentStore(*mStreamSmithEnv.mainLogger);
		
		if( mStreamSmithEnv.mainAdapter )
			mStreamSmithEnv.mainAdapter->deactivate();	

		mStreamSmithEnv.mSessionScaner.stop();
		mStreamSmithEnv.mainScheduler.stop();
		mStreamSmithEnv.mStreamerManager.uninitialize();
		
		if( mStreamSmithEnv.mainThreadPool )
			mStreamSmithEnv.mainThreadPool->stop();

	}
	return ZQ::common::BaseZQServiceApplication::OnStop();
}

HRESULT StreamSmithService::OnUnInit()
{
	UninitializeIceRunTime();
	uninitializeLogger();
	if( mStreamSmithEnv.mainThreadPool )
	{		
		delete mStreamSmithEnv.mainThreadPool;
	}
	return ZQ::common::BaseZQServiceApplication::OnUnInit();
}

void	StreamSmithService::OnSnmpSet(const char *varName)
{

}

