
#include "./SOPConfig.h"
#include "NgodService.h"

#include <ZQ_common_conf.h>
#include <assert.h>
#include <sstream>
#include <TianShanIceHelper.h>
#include <TsEvents.h>
#include "NgodConfig.h"
#include <IceLog.h>
#include "NgodSession.h"
#include "ZQResource.h"

#ifdef ZQ_OS_LINUX
#define _vsnprintf		vsnprintf
#define _vsnwprintf		vswprintf
#define _snprintf		snprintf
#define _snwprintf		vswprintf

#include <ctype.h>
#endif
#ifndef min
#define min(x,y) (x>y?y:x)
#endif

#ifndef max
#define max(x,y) (x>y?x:y)
#endif
namespace NGOD
{

NgodService::NgodService(void)
: mEnv(mMainLogger, mEventLogger),
mSessManager(mEnv),
mEventDispatcher(mEnv,mSessManager),
mbStopped(true),
mEventSinker(mEnv),
mD5Speaker(NULL)
{
}

NgodService::~NgodService(void)
{
}

void NgodService::setErroMsg( const char* fmt , ... )
{
	char szLocalBuffer[1024];
	va_list args;
	va_start(args, fmt);
	int nCount = _vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, fmt, args );
	va_end(args);
	if(nCount == -1)
	{
		szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	}
	else
	{
		szLocalBuffer[nCount] = '\0';
	}

	mErrMsg = szLocalBuffer;
}

const std::string& NgodService::getErrMsg( ) const
{
	return mErrMsg;
}

bool NgodService::initLogger( const char* logfolder )
{
	if( !( logfolder && logfolder[0] != 0 ) )
	{
		setErroMsg("null log folder passed in");
		return false;
	}

	try
	{
		std::string path = ZQTianShan::Util::fsConcatPath( logfolder,"ssm_NGOD2.log");
		mMainLogger.open( path.c_str() ,ZQ::common::Log::L_DEBUG );
	}
	catch( const ZQ::common::FileLogException& ex)
	{
		setErroMsg("failed to open main log due to [%s]", ex.what() );
		return false;
	}

	try
	{
		std::string path = ZQTianShan::Util::fsConcatPath( logfolder,"ssm_NGOD2_events.log");
		mEventLogger.open( path.c_str() , ZQ::common::Log::L_DEBUG );
	}
	catch( const ZQ::common::FileLogException& ex )
	{
		setErroMsg("failed to open events log due to [%s]",ex.what() );
		return false;
	}

	try
	{
		std::string path = ZQTianShan::Util::fsConcatPath( logfolder,"ssm_NGOD2_icetrace.log");
		mIceLogger.open( path.c_str() , ZQ::common::Log::L_DEBUG);
	}
	catch( const ZQ::common::FileLogException& ex )
	{
		setErroMsg("failed to open ice log due to [%s]",ex.what() );
		return false;
	}

	// mEnv.mMainLogger	= &mMainLogger;
	// mEnv.mEventLogger	= &mEventLogger;

	return true;
}

#define STRVALID(x) (x && x[0] != 0 )

bool NgodService::loadConfig( const char* confPath )
{
	if(!STRVALID(confPath) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"null configuration path passed in"));
		return false;
	}

	ngodConfig.setLogger( &mMainLogger );
	sopConfig.setLogger( &mMainLogger );

	std::string ngodConfPath = ZQTianShan::Util::fsConcatPath(confPath,"ssm_NGOD2.xml");
	if( !ngodConfig.load( ngodConfPath.c_str() ) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"failed to load configuration from [%s]"),ngodConfPath.c_str() );
		return false;
	}

	std::string sopConfFileName = ngodConfig.sopProp.fileName;
	if(sopConfFileName.empty())
	{
		sopConfFileName = ZQTianShan::Util::fsConcatPath(confPath,"ssm_NGOD2_SOP.xml");
	}
	if( !sopConfig.load( sopConfFileName.c_str() ) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"failed to load configuration from [%s]") , sopConfFileName.c_str() );
		return false;
	}
	
	{
		//calculate ads restriction
		ngodConfig.adsReplacment.adsRestricts = 0;

		if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "F"))
			ngodConfig.adsReplacment.adsRestricts |= TianShanIce::Streamer::PLISFlagNoFF;

		if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "R"))
			ngodConfig.adsReplacment.adsRestricts |= TianShanIce::Streamer::PLISFlagNoRew;

		if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "P"))
			ngodConfig.adsReplacment.adsRestricts |= TianShanIce::Streamer::PLISFlagNoPause;

		//fdj
		if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "S"))
			ngodConfig.adsReplacment.adsRestricts |= TianShanIce::Streamer::PLISFlagNoSeek;
		
		if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "K"))
			ngodConfig.adsReplacment.adsRestricts |= TianShanIce::Streamer::PLISFlagSkipAtFF;

		if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "W"))
			ngodConfig.adsReplacment.adsRestricts |= TianShanIce::Streamer::PLISFlagSkipAtRew;

		int playtimes = 0;
			sscanf(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(),"%*[^0-9]%d",&playtimes);
		if( playtimes > 0 && playtimes < 10)
			ngodConfig.adsReplacment.adsRestricts |=  (playtimes << 4);

	} 

	//adjust log file parameter

	mMainLogger.setFileSize( ngodConfig.pluginLog.size);
	mMainLogger.setFileCount( ngodConfig.pluginLog.maxCount);
	mMainLogger.setLevel( ngodConfig.pluginLog.level);
	mMainLogger.setBufferSize( ngodConfig.pluginLog.bufferSize);

	mEventLogger.setFileSize( ngodConfig.eventLog.size);
	mEventLogger.setFileCount( ngodConfig.eventLog.maxCount);
	mEventLogger.setLevel( ngodConfig.eventLog.level );
	mEventLogger.setBufferSize( ngodConfig.eventLog.bufferSize );

	//StreamFailover part
	std::map<std::string, std::string >& failoerTestStreamers = mEnv.getFailoverTestStreamers();
	std::map< std::string , SOPRestriction::SopHolder >::const_iterator itSop = sopConfig.sopRestrict.sopDatas.begin();
	for( ; itSop != sopConfig.sopRestrict.sopDatas.end() ; itSop++ ) 
	{
		typedef std::vector<ZQ::common::Config::Holder<NGOD::Streamer> >::const_iterator roItType;
		roItType itStreamer = itSop->second.streamerDatas.begin();
		for( ; itStreamer != itSop->second.streamerDatas.end() ; itStreamer ++ ) 
		{
			if( !itStreamer->enabled ) 
				continue;
			if( !itStreamer->usedInStreamFailoverTest) 
				continue;
			failoerTestStreamers[itStreamer->netId] = itStreamer->serviceEndpoint;
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodService,"mark streamer[%s]  endpoint [%s] as stream failover member"),
				itStreamer->netId.c_str(), itStreamer->serviceEndpoint.c_str());
		}
	}

	if( failoerTestStreamers.size() > 0 && failoerTestStreamers.size() != 2 ) 
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(NgodService,"bad fail over test  configuration , only 2 streamer can be set in this configuration. disable stream failover"));
		failoerTestStreamers.clear();
	}

	return true;
}

bool NgodService::initIceRuntime( )
{
	int i = 0;

	Ice::InitializationData initData;
	
	initData.properties = Ice::createProperties(i, NULL);
	
	assert( initData.properties != NULL );
	///initialize ice properties	
	const std::vector<NGOD::IceProperty::IcePropertyHolder>& iceProprs = ngodConfig.iceProps.propDatas;
	std::vector<NGOD::IceProperty::IcePropertyHolder>::const_iterator itIceProp = iceProprs.begin();
	for( ; itIceProp != iceProprs.end() ; itIceProp++ )
	{
		initData.properties->setProperty(itIceProp->name, itIceProp->value);
	}
	
	initData.logger = new TianShanIce::common::IceLogI( &mIceLogger );
	assert( initData.logger != NULL );
	
	try
	{
		mIc =	Ice::initialize( i , NULL , initData );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"failed to initialize ice runtime due to [%s]"),ex.ice_name().c_str() );
		return false;
	}

	mIc->addObjectFactory( new SessionFactory(mEnv,mSessManager) , NGOD::NgodSession::ice_staticId() );

	return true;
}

bool NgodService::initNgodEnv( )
{
	//TODO: initialize module name
	mEnv.mModuleName = ZQ_PRODUCT_NAME_SHORT;

	//create object adapter
	try
	{
		//set dispatch thread size
		Ice::PropertiesPtr proper = mIc->getProperties();
		int		iThread		= ngodConfig.bind.dispatchSize;
		int		iThreadMax	= ngodConfig.bind.dispatchMax;
		iThread		= max( iThread , 3 );
		iThreadMax	= max( iThreadMax , 5 );
		std::ostringstream oss;oss.str("");
		oss<<iThread;
		proper->setProperty("NgodService.ThreadPool.Size", oss.str() );
		oss.str("");
		oss<<iThreadMax;
		proper->setProperty("NgodService.ThreadPool.SizeMax", oss.str() );
		
		mEnv.mAdapter = ZQADAPTER_CREATE( mIc, "NgodService", ngodConfig.bind.endpoint.c_str(), mMainLogger);
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodService,"create adapter on[%s] with dispatchSize[%d] disptachMax[%d]"),
					ngodConfig.bind.endpoint.c_str() , iThread , iThreadMax	);
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"failed to create adapter on [%s] due to [%s]"),
				ngodConfig.bind.endpoint.c_str() , ex.ice_name().c_str() );
		return false;
	}

	mEnv.mIc = mIc;

	std::transform(ngodConfig.protocolVersioning.customer.begin(), ngodConfig.protocolVersioning.customer.end(), ngodConfig.protocolVersioning.customer.begin(), tolower);

	return true;
}

bool NgodService::initSelectionEnv( )
{
	SelectionEnv&	selEnv = mEnv.mSelEnv;
	

	//initialize asset stack configuration
	selEnv.mbEnableAssetStack			= ngodConfig.assetStack.enable >= 1;
//	selEnv.mAssetStackTimeShiftWindow	= ngodConfig.assetStack.windowSize ;
	selEnv.mAssetStackAdjustWeight		= ngodConfig.assetStack.adjustWeight;

	selEnv.mAssetStackAdjustWeight		= max( selEnv.mAssetStackAdjustWeight , 100 );
	selEnv.mAssetStackAdjustWeight		= min( selEnv.mAssetStackAdjustWeight , 9999 );
	selEnv.mAssetStackStartMode			= ngodConfig.assetStack.startMode;

	selEnv.mLogger						= &mMainLogger;
	selEnv.mEventLogger					= &mEventLogger;
	selEnv.mbContentTestMode			= false;
	selEnv.mbContentLibMode				= false;

	selEnv.mbPublishLog					= ngodConfig.publishLogs.enabled;

	bool lamTestMode = ngodConfig.lam.lamTestMode.enabled >= 1;
	if ( ! lamTestMode )
	{
	
		if(ngodConfig.lam.contentLibMode.enabled >= 1)
		{
			selEnv.mbContentLibMode = true;
			std::string contentLibEndpoint = ngodConfig.lam.contentLibMode.endpoint;
			if( contentLibEndpoint.find(':') == std::string::npos )
			{
				contentLibEndpoint = std::string("ContentLibApp:") + ngodConfig.lam.contentLibMode.endpoint;
			}
			try
			{

				selEnv.contentLibPrxoy = TianShanIce::Repository::ContentLibPrx::uncheckedCast( STR2PROXY(contentLibEndpoint) );
			}
			catch( const Ice::Exception& ex)
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"failed to turn ContentLib endpoint[%s] to proxy due to [%s]"),
					contentLibEndpoint.c_str() , ex.ice_name().c_str() );
				return false;
			}		
		}
		else
		{
			std::string lamEndpoint = ngodConfig.lam.lamServer.endpoint;
			if( lamEndpoint.find(':') == std::string::npos )
			{
				lamEndpoint = std::string("LAMFacade:") + ngodConfig.lam.lamServer.endpoint;
			}
			try
			{

				selEnv.lamProxy			= com::izq::am::facade::servicesForIce::LAMFacadePrx::uncheckedCast( STR2PROXY(lamEndpoint) );
			}
			catch( const Ice::Exception& ex)
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"failed to turn lam endpoint[%s] to proxy due to [%s]"),
					lamEndpoint.c_str() , ex.ice_name().c_str() );
				return false;
			}
		}
	}
	else
	{
		selEnv.mbContentTestMode = true;
		selEnv.lamProxy			= NULL;
		selEnv.contentLibPrxoy	= NULL;

		//check the test mode content settings
		com::izq::am::facade::servicesForIce::AEInfo3Collection& assetInfo = selEnv.mTestModeContent;
		PID2ELEMAP& pid2ele = selEnv.mPid2Elements;
		NGOD::LAMTestMode::LAMSubTestSetHolderS& testData = ngodConfig.lam.lamTestMode.subTests;
		NGOD::LAMTestMode::LAMSubTestSetHolderS::const_iterator it = testData.begin();
		for( ; it != testData.end() ; it++ )
		{
			com::izq::am::facade::servicesForIce::AEInfo3 aeinfo;
			aeinfo.name			= it->contentName;
			aeinfo.bandWidth	= it->bandwidth;
			aeinfo.cueIn		= it->cueIn;
			aeinfo.cueOut		= it->cueOut;
			aeinfo.nasUrls		= it->urls;
			aeinfo.volumeList	= it->volumeList;
			aeinfo.attributes	= it->attrs;
			assetInfo.push_back( aeinfo );
			pid2ele[ it->pid ] = aeinfo;
		}
	}

	selEnv.mbIcReplicaTestMode			= ngodConfig.passThruStreaming.testMode >= 1;
	selEnv.mbStreamerReplicaTestMode	= sopConfig.sopRestrict.replicaUpdateEnable <= 0 ;
	selEnv.mReplicaUpdateInterval		= sopConfig.sopRestrict.replicaUpdateInterval * 1000;//convert second to millisecond
	selEnv.mMaxPenaltyValue				= sopConfig.sopRestrict.maxPenaltyValue;
	
	selEnv.mbExcludeC2Linkdown			= ngodConfig.passThruStreaming.excludeC2Down >= 1;

	return initSelManager();
}

bool NgodService::initPublishedLogs( )
{
	if(ngodConfig.publishLogs.enabled)
	{
		try
		{
			std::vector<NGOD::PublishLogs::PublishLogHolder>& logSyntaxs = ngodConfig.publishLogs.logDatas;        
			std::vector<NGOD::PublishLogs::PublishLogHolder>::const_iterator iter = logSyntaxs.begin();
			for (; iter != logSyntaxs.end(); iter++ ) 
			{		
				if (!mEnv.mAdapter->publishLogger(iter->path.c_str(), iter->syntax.c_str(),
					iter->key.c_str() , iter->type.c_str() ))
				{			
					MLOG(ZQ::common::Log::L_ERROR, 
						CLOGFMT(NgodService,"Failed to publish logger name[%s] syntax[%s] key[%s] type[%s]"),
						iter->path.c_str(), iter->syntax.c_str(),iter->key.c_str() , iter->type.c_str() );
				}		
				else			
				{			
					MLOG(ZQ::common::Log::L_INFO,  
						CLOGFMT(NgodService,"Publish logger name[%s] syntax[%s] key[%s] type[%s] successful"),
						iter->path.c_str(), iter->syntax.c_str() , iter->key.c_str() , iter->type.c_str() );
				}		
			}	
		}
		catch(...) 
		{
			MLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NgodService, "publishLogger caught unexpected error"));
			return false;
		}
	}
	return true;
}

std::string fixupNgodPartitionConf( const std::string& volumeName)
{
	std::string::size_type pos = volumeName.find("/");
	if(pos != std::string::npos)
	{
		return volumeName.substr( 0, pos);
	}
	else
	{
		return volumeName;
	}
}

bool NgodService::initSelManager()
{
	NgodResourceManager& manager = mEnv.getSelResManager();

	{///initialize sop
		SOPS sops;
		std::map< std::string , NGOD::SOPRestriction::SopHolder > confSops = sopConfig.sopRestrict.sopDatas;
		std::map< std::string , NGOD::SOPRestriction::SopHolder >::const_iterator itSop = confSops.begin();
		for( ; itSop != confSops.end() ; itSop++ )
		{
			ResourceStreamerAttrMap attrmap;
			const std::vector<NGOD::Sop::StreamerHolder>& streamers = itSop->second.streamerDatas;
			std::vector<NGOD::Sop::StreamerHolder>::const_iterator itStreamer = streamers.begin();
			for( ; itStreamer != streamers.end() ; itStreamer ++ )
			{
				 ResourceStreamerAttr attr;
				 const NGOD::Streamer& streamer = *itStreamer;
				 attr.netId					= streamer.netId;
				 attr.endpoint				= streamer.serviceEndpoint;
				 attr.maxBw					= ((int64)streamer.totalBW) * 1000;
				 attr.maxSessCount			= streamer.maxStream;
				 attr.importChannelName		= streamer.importChannel;
				 attr.bMaintainEnable		= streamer.enabled >= 1;
				 attr.volumeNetId			= fixupNgodPartitionConf(streamer.volume);
				 attr.bAllPartitionAvail	= true;//hard code now because all partitions must be available for the streamer
				 try
				 {
					 std::string endpoint = streamer.serviceEndpoint;
					 if( streamer.serviceEndpoint.find(":") == std::string::npos )
					 {
						 endpoint = std::string("StreamSmith:") + streamer.serviceEndpoint;
					 }					
					 attr.streamServicePrx	= TianShanIce::Streamer::StreamSmithAdminPrx::uncheckedCast( STR2PROXY( endpoint ) );
					 attr.streamServiceEndpoint = endpoint;
				 }
				 catch( const Ice::Exception& ex)
				 {
					 MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodService,"failed to convert endpoint[%s] to proxy due to [%s]"),
						 streamer.serviceEndpoint.c_str() , ex.ice_name().c_str() );
					 return false;
				 }
				 attrmap.insert( ResourceStreamerAttrMap::value_type( attr.netId, attr ) );
			}
			sops.insert( SOPS::value_type(itSop->first , attrmap) );
		}
		manager.updateSopData( sops );
	}

	{
		ResourceVolumeAttrMap volumes;
		const NGOD::LAMServer::ContentVolumeHolderS& confVols = ngodConfig.lam.lamServer.contentVolumes;
		//LAMServer::ContentVolumeHolder
		NGOD::LAMServer::ContentVolumeHolderS::const_iterator it = confVols.begin();
		for( ; it != confVols.end() ; it ++ )
		{
			const ContentVolume::ContentVolumeHolder& vol = it->second;
			ResourceVolumeAttr attr;
			attr.netId						= fixupNgodPartitionConf( vol.name);
			attr.bAllPartitions				= true;//hard code now because all partitions must be available for a streamer
			attr.level						= vol.cacheLevel;
			attr.bSupportNas				= vol.supportNasStreaming >= 1;
			attr.bSupportCache				= vol.cache >= 1;
			volumes.insert( ResourceVolumeAttrMap::value_type(attr.netId , attr) );
		}
		manager.updateVolumesData( volumes );
	}

	{
		ResourceImportChannelAttrMap ics;
		const std::map<std::string,PassThruStreaming::ImportChannelHolder>& confIc = ngodConfig.passThruStreaming.importChannelDatas;
		std::map<std::string,PassThruStreaming::ImportChannelHolder>::const_iterator it = confIc.begin();
		for( ; it != confIc.end() ; it ++ )
		{
			ResourceImportChannelAttr attr;
			const PassThruStreaming::ImportChannelHolder& ic = it->second;
			attr.netId				= ic.name;
			attr.confMaxBw			= ((int64)ic.bandwidth)*1000;
			attr.confMaxSessCount	= ic.maxImport;
			ics.insert( ResourceImportChannelAttrMap::value_type( attr.netId , attr) );
		}
		manager.updateImportChannelData( ics );
	}

	if(!manager.initResourceManager( sopConfig.sopRestrict.enableWarmup >= 1))
		return false;
	return manager.start();
}

bool NgodService::initEventSinker( )
{
	mEventDispatcher.start();

	bool bOk = false;

	bOk = mEventSinker.addEventHandler( new StreamEventI( mEnv , mSessManager ,mEventDispatcher) );
	if( !bOk )	return false;
	bOk = mEventSinker.addEventHandler( new PlaylistEventI( mEnv , mSessManager ,mEventDispatcher) );
	if( !bOk )	return false;
	bOk = mEventSinker.addEventHandler( new RepositionEventSinkI( mEnv , mSessManager , mEventDispatcher) , TianShanIce::Events::TopicStreamRepositionEvent );
	if( !bOk )	return false;
	bOk = mEventSinker.addEventHandler( new PauseTimeoutEventSinkI(mEnv , mSessManager , mEventDispatcher) , TianShanIce::Events::TopicStreamPauseTimeoutEvent );
	if( !bOk )	return false;

	return true;
}

#define XLOGFMT(_C, _X) "%-18s " _X, #_C

bool NgodService::start( IStreamSmithSite* pSite , const char* pConfPath ,const char* logfolder )
{	
	if( !initLogger(logfolder) )
	{
		return false;
	}
	
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodService,"====================== NGOD service is starting ======================"));

	//load configuration
	if(!loadConfig(pConfPath) )
	{
		return false;
	}

	if(ngodConfig.publishLogs.enabled)
		ELOG(ZQ::common::Log::L_INFO, XLOGFMT(NgodService, "NGOD initialize start"));
	
	if(!initIceRuntime())
		return false;
	
	if( !initNgodEnv() )
		return false;

	if( !initSelectionEnv() )
		return false;
	
	if( !initEventSinker( ) )
		return false;
	
	if( !initPublishedLogs( ) )
		return false;

	if(!mEventSinker.start(ngodConfig.iceStorm.endpoint))
	{
		ELOG(ZQ::common::Log::L_ERROR, XLOGFMT(NgodService, "failed to start EventSinker"));
		return false;
	}

	//TODO: start the service now
    if (ngodConfig.database.runtimePath.empty())
        ngodConfig.database.runtimePath = ngodConfig.database.path;

	const std::string dbPath = ngodConfig.database.runtimePath;
	int32 evictorSize = ngodConfig.rtspSession.cacheSize;
	if (!mSessManager.start(dbPath, evictorSize, pSite))
	{
		ELOG(ZQ::common::Log::L_ERROR, XLOGFMT(NgodService, "failed to start SessManager"));
		return false;
	}

	mbStopped = false;
	mEnv.getObjAdapter()->activate();

	ngodConfig.snmpRegister("ssm_NGOD");

	// mSnmpServant.setEndpoint( ngodConfig.bind.endpoint );
	// mSnmpServant.registerSnmpTable(pSite);
	mEnv._snmpSA.start();

	mD5Speaker = new D5Speaker(mEnv, mEnv.getSelResManager(), mEnv.getThreadPool());
	mD5Speaker->start();
#define SSMNGODVER __N2S__(ZQ_PRODUCT_VER_MAJOR) "." __N2S__(ZQ_PRODUCT_VER_MINOR) "." __N2S__(ZQ_PRODUCT_VER_PATCH) "." __N2S__(ZQ_PRODUCT_VER_BUILD)
    //bug#20449 ZQ_PRODUCT_VER_* is used just for windows, the following log will be wrong at linux, so comment it
	//MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodService,"************************ NGOD %s service is running ************************"), SSMNGODVER);
	if(ngodConfig.publishLogs.enabled)
		ELOG(ZQ::common::Log::L_INFO, XLOGFMT(NgodService, "NGOD initialize OK"));

	return true;
}

void NgodService::uninitNgodEnv()
{
	mEnv._snmpSA.stop();
	mEnv.mThreadPool.stop();//stop thread pool so that no more requests can be executed
	mEventDispatcher.stop();
}

void NgodService::uninitSelectionEnv()
{
	mEnv.getSelResManager().stop();
}
void NgodService::uninitIceRuntime()
{
	try
	{
		mEnv.getObjAdapter()->deactivate();
	}
	catch( const Ice::Exception& )
	{
	}
	mEnv.mAdapter = NULL;
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService,"stop() objAdapter deactivated"));
	try
	{
		mEnv.getCommunicator()->destroy();

	}
	catch( const Ice::Exception& )
	{

	}
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService,"stop() communicator destroyed"));
	mEnv.mIc	= NULL;
	mIc			= NULL;
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService,"stop() ice runtime destroyed"));

}
void NgodService::uninitLogger()
{

}

void NgodService::stop( )
{	
	if(mD5Speaker)
	{
		mD5Speaker->stop();
		delete mD5Speaker;
	}

	// mSnmpServant.unregisterSnmpTable();

	mbStopped = true;
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService," NGOD service is quiting "));
	
	mEventSinker.stop();
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService," event sinker stopped "));

	mSessManager.stop();
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService," session manager stopped "));

	uninitNgodEnv();
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService," ngod environment uninitialized "));

	uninitSelectionEnv();
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService," selection envrionment uninitialized "));

	uninitIceRuntime();
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodService," ice runtime destroyed "));

	uninitLogger();	

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodService,"NGOD service is stopped"));
}

RequestProcessResult NgodService::processRequest( IClientRequest* clireq )
{
	if( mbStopped )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(NgodService,"processRequest() service is not running, reject request"));
		return RequestDone;
	}
	
	mSessManager.processRequest( clireq );
	

	return RequestDone;
}

}//namespace NGOD
