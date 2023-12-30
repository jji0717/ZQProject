
#include "SaService.h"
#include <IceLog.h>
#include <TianShanIceHelper.h>
#include "StreamEventReceiver.h"
#include "SiteAdminIceObjFactory.h"
#include <FileSystemOp.h>
#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType =1;
DWORD gdwServiceInstance =1;
#endif

SaService gService;
ZQ::common::BaseZQServiceApplication *Application=&gService;
ZQ::common::Config::Loader<SAConfig> gSaConfig("SiteAdminSvc.xml");
ZQ::common::Config::ILoader *configLoader = &gSaConfig;

extern const char* DUMP_PATH;

SaService::SaService(void)
:mDb(mEnv),
mTxnTransfer(mEnv,mDb),
mTxnWatchDog(mEnv,mDb),
mEventSinker(mEnv),
mEventSenderManager(mEnv)
{
}

SaService::~SaService(void)
{
}

void SaService::adjustConfig( )
{

}

bool SaService::initializeIceRuntime( )
{
	std::string logDir;
#ifdef ZQ_OS_MSWIN
	logDir = m_wsLogFolder;
#else
	logDir = _logDir;
#endif
	
	std::string iceLogPath = ZQTianShan::Util::fsConcatPath(logDir.c_str() , "SiteAdmin_IceTrace.log");
	int i = 0;
	Ice::PropertiesPtr props = Ice::createProperties( i , NULL );
	SAConfig::IceProps::const_iterator itProp = gSaConfig.iceProps.begin();
	for( ; itProp != gSaConfig.iceProps.end() ; itProp ++ )
	{
		props->setProperty(itProp->first , itProp->second);
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"initializeIceRuntime() apply ice property: %s -> %s"),
			itProp->first.c_str() , itProp->second.c_str() );
	}

#if ICE_INT_VERSION / 100 >= 303

	::Ice::InitializationData initData;
	//initData.properties = proper;	

	if ( gSaConfig.lIceTraceEnable >=1)
	{
		try
		{
			mIceLogger.open(iceLogPath.c_str() , gSaConfig.lIceTraceLogLevel , 5 , gSaConfig.lIceTraceLogSize , gSaConfig.lIceTraceLogBuffer  , gSaConfig.lIceTraceLogTimeout );
		}
		catch( const ZQ::common::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SaService,"initializeIceRuntime() failed to open ice trace at[%s] due to [%s]"),
				iceLogPath.c_str() , ex.what() );
			return false;
		}
		initData.logger =new  TianShanIce::common::IceLogI( &mIceLogger );		 
	}
	mEnv.mIc = Ice::initialize(initData);

#else //ICE_INT_VERSION / 100 >= 303
	if( gSaConfig.lIceTraceEnable >= 1)	
	{
		try
		{
			mIceLogger.open(iceLogPath.c_str() , gSaConfig.lIceTraceLogLevel , 5 , gSaConfig.lIceTraceLogSize , gSaConfig.lIceTraceLogBuffer  , gSaConfig.lIceTraceLogTimeout );
		}
		catch( const ZQ::common::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SaService,"initializeIceRuntime() failed to open ice trace at[%s] due to [%s]"),
				iceLogPath.c_str() , ex.what() );
			return false;
		}		
		int i=0;
		mEnv.mIc = Ice::initializeWithPropertiesAndLogger( i , NULL , props , new  TianShanIce::common::IceLogI( &mIceLogger ) );
	}
	else
	{
		int i=0;
		mEnv.mIc = Ice::initializeWithProperties(i,NULL, props );
	}
#endif//ICE_INT_VERSION / 100 >= 303

	try
	{
		mEnv.mObjAdapter = ZQADAPTER_CREATE(mEnv.mIc , "TianShan-SiteAdminSvc",gSaConfig.szServiceEndpoint,MLOG);
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SaService,"initializeIceRuntime() failed to create ice obj adapter at [%s] ue to [%s]"),
			gSaConfig.szServiceEndpoint, ex.ice_name().c_str() );
		return false;
	}

	return true;
}

bool SaService::initializeSiteAdmin( )
{
	try
	{
		mEnv.mMainThreadpool = new ZQ::common::NativeThreadPool();
		assert(mEnv.mMainThreadpool != NULL);
		int svcThreadSize = mEnv.getConfig().lSvcThreadPoolSize;
		svcThreadSize = MIN(svcThreadSize,1000);
		svcThreadSize = MAX(svcThreadSize,3);
		mEnv.mMainThreadpool->resize(svcThreadSize);

		if( !mDb.openDb( gSaConfig.szDatabaseFolder , gSaConfig.szRuntimeDbFolder, mEnv.getAdapter() ) )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SaService,"initializeSiteAdmin() failed to open db at [%s] [%s]"),
				gSaConfig.szDatabaseFolder , gSaConfig.szRuntimeDbFolder );
			return false;
		}

		mSaImpl = new SiteAdminImpl(mEnv,mDb,mTxnWatchDog,mTxnTransfer);
		assert( mSaImpl );

		mObjFactory = new SiteAdminIceObjFactory(mEnv,mDb,mTxnWatchDog,mTxnTransfer,*mSaImpl.get() );

		mEnv.getAdapter()->ZQADAPTER_ADD( mEnv.getIc() , mSaImpl , SERVICE_NAME_BusinessRouter );

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"initializeSiteAdmin() adding %s onto object adapter"), SERVICE_NAME_BusinessRouter );

		mSaImpl->init();

		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"initializeSiteAdmin() service instance initialized"));

		mEventSenderManager.SetupEventSenderEnvironment();

		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"initializeSiteAdmin() plugin loaded"));

		startEventSinker();

		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"initializeSiteAdmin() event sinker started"));
		
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SaService,"initializeSiteAdmin() failed to create siteadmin instance: %s"), ex.ice_name().c_str() );
		return false;
	}
	return true;
}

void SaService::startEventSinker()
{
	if( strlen( mEnv.getConfig().szIceStormEndpoint ) <= 0 )
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"startEventSinker() event channel endpoint is not present, do not sink event"));
		return;
	}
	mEventSinker.addEventHandler( new ZQTianShan::Site::StreamEventSinkI(mEventSenderManager) );
	mEventSinker.addEventHandler( new ZQTianShan::Site::PlaylistEventSinkI(mEventSenderManager) );
	mEventSinker.start( mEnv.getConfig().szIceStormEndpoint );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"startEventSinker() start to sink event"));
}

HRESULT SaService::OnInit(void)
{
	mEnv.mLogger				= m_pReporter;
	mEnv.mSaConfig				= &gSaConfig;
	
#ifdef ZQ_OS_MSWIN
	if( mEnv.getConfig().lCrashDumpEnable >= 1)
	{
		FS::FileAttributes fa(mEnv.getConfig().szCrashDumpPath);
		if(fa.exists() && fa.isDirectory())
		{
			static ZQ::common::MiniDump			_crashDump;
			_crashDump.setDumpPath( mEnv.getConfig().szCrashDumpPath );
			_crashDump.enableFullMemoryDump( true );
			//_crashDump.setExceptionCB(CrashExceptionCallBack);
		}
	}
#else//
	DUMP_PATH = mEnv.getConfig().szCrashDumpPath;
#endif

	adjustConfig();

	if( !initializeIceRuntime() )
		return S_FALSE;

	if( !initializeSiteAdmin() )
		return S_FALSE;

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnInit() siteAdminService initialized"));
	return BaseZQServiceApplication::OnInit();
}

HRESULT SaService::OnStart(void)
{
	try
	{	
		mTxnWatchDog.start();
		mTxnTransfer.start();
		mEnv.getAdapter()->activate();
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnStart() siteAdminService started"));
		//return S_OK;
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SaService,"OnStart() failed to start siteAdminService due to %s"), ex.ice_name().c_str() );
		return S_FALSE;
	}
	return BaseZQServiceApplication::OnStart();
}

void SaService::stopEventSinker()
{
	if(strlen(mEnv.getConfig().szIceStormEndpoint) > 0 )
		mEventSinker.stop();
}

HRESULT SaService::OnStop(void)
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"OnStop() trying to stop event sinker"));
	stopEventSinker();
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"OnStop() event sinker stopped"));

	mEventSenderManager.DestroyEventSenderEnvironment();

	try
	{
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"OnStop() trying to deactivate object adapter"));
		mEnv.getAdapter()->deactivate();
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnStop() object adapter deactivated"));		

		mDb.closeDb();
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnStop() db closed"));

	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SaService,"OnStop() caught exception %s"), ex.ice_name().c_str() );
	}
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"OnStop() trying to stop watch dog"));
	mTxnWatchDog.stop();
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnStop() watchdog stopped"));

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"OnStop() trying to stop txn transfer"));
	mTxnTransfer.stop();
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnStop() txn transfer stopped"));
	return BaseZQServiceApplication::OnStop();
}

HRESULT SaService::OnUnInit(void)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SaService,"OnUnInit() entering"));
	try
	{
		mEnv.mObjAdapter = NULL;
	}
	catch(...){}
	try
	{
		mSaImpl = NULL;
	}
	catch(...){}
	try
	{
		mObjFactory = NULL;

		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(SaService,"OnUnInit() trying to destroy ice communicator"));
		mEnv.getIc()->destroy();
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnUnInit(0 ice communicator destroyed"));
	}
	catch(...){}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SaService,"OnUnInit() siteAdminService stopped"));

	return BaseZQServiceApplication::OnUnInit();
}
