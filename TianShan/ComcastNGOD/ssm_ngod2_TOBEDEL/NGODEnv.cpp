#include "./NGODEnv.h"

#include "./SetupHandler.h"
#include "./PlayHandler.h"
#include "./PauseHandler.h"
#include "./TeardownHandler.h"
#include "./GetParamHandler.h"
#include "./SetParamHandler.h"
#include "./OptionHandler.h"
#include "./PingHandler.h"
#include "IceLog.h"
#include "urlstr.h"
#include "PenaltyManager.h"
#include "RtspRelevant.h"
#include <io.h>
#include <TianShanIceHelper.h>

#define INDEXFILENAME(_X) #_X "Idx"

#define SYSLOG _sysLog
#define NGODLOG _fileLog
#define NGODLOGFMT(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Mtd(%s) " _T), session.c_str(), sequence.c_str(), method.c_str()

// add by zjm to support ssm_ngod2_events.log
#define NGODEVENTLOG _sentryLog
NGODEnv::NGODEnv() 
	: _pSite(NULL)
	, _pThreadPool(NULL)
	, _pCommunicator(NULL)
	, _pEventAdapter(NULL)
	, _pSessionManager(NULL)
	, _pEventChannal(NULL)
	, _pSafeStoreConn(NULL)
	, _pContextEvtr(NULL)
	, _pStreamIdx(NULL)
	, _pGroupIdx(NULL)
	, _pFactory(NULL)
	, _pSessionWatchDog(NULL)
	, _penaltyManager(NULL)
	, _globalSequence(1)
	,mResManager(mSelectionEnv)
{
#pragma message(__MSGLOC__"not implemented yet")
	//throw "haha";
	bQuited = false;
	_sysLog.open("RtspProxy", ZQ::common::Log::L_WARNING);
}

NGODEnv::~NGODEnv()
{
	//doUninit();
}


int NGODEnv::doInit(IStreamSmithSite* pSite)
{
	counterMeasuredSince = ZQTianShan::now();

	_pSite = pSite;

	_fileLog.open(	_ngodConfig._pluginLog._path.c_str(), 
					ZQ::common::Log::L_DEBUG,
					_ngodConfig._pluginLog._maxCount,
					_ngodConfig._pluginLog._size,
					_ngodConfig._pluginLog._bufferSize,
					2,
					ZQLOG_DEFAULT_EVENTLOGLEVEL,
					"RtspProxy");

	ZQ::common::setGlogger(&_fileLog);
	_ngodConfig.setLogger(&_fileLog);

	// add by zjm to support ssm_ngod2_events.log
	try
	{
		char szIceLog[512];
		int nCopyLen = _ngodConfig._pluginLog._path.length() - 4;
		strncpy(szIceLog, _ngodConfig._pluginLog._path.c_str(), nCopyLen);
		strcpy(szIceLog + nCopyLen, "_events.log");
		_sentryLog.open(szIceLog, ZQ::common::Log::L_DEBUG);
	}
	catch (ZQ::common::FileLogException& ex)
	{
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "create log file caught [%s]"), ex.getString());
		return 1;
	}

	mSelectionEnv.mLogger = &_fileLog;


	// translate log into ssm_ngod2_events.log by zjm
	NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "NGOD initialize start"));
	NGODEVENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "NGOD initialize start"));
	if (false == loadConfig())
		return 1;

	_fileLog.setFileSize(_ngodConfig._pluginLog._size);
	_fileLog.setFileCount(_ngodConfig._pluginLog._maxCount);
	_fileLog.setLevel(_ngodConfig._pluginLog._level);
	_fileLog.setBufferSize(_ngodConfig._pluginLog._bufferSize);


	// add by zjm to support sentry log
	_sentryLog.setFileSize(_ngodConfig._pluginLog._size);
	_sentryLog.setFileCount(_ngodConfig._pluginLog._maxCount);
	_sentryLog.setLevel(_ngodConfig._pluginLog._level);
	_sentryLog.setBufferSize(_ngodConfig._pluginLog._bufferSize);
	try
	{
		char szIceLog[512];
		int nCopyLen = _ngodConfig._pluginLog._path.length() - 4;
		strncpy(szIceLog, _ngodConfig._pluginLog._path.c_str(), nCopyLen);
		strcpy(szIceLog + nCopyLen, "_iceTrace.log");
		//_iceLog.open(szIceLog, _ngodConfig._pluginLog._level, _ngodConfig._pluginLog._maxCount);
		//use debug level
		_iceLog.open(szIceLog, ZQ::common::Log::L_DEBUG, _ngodConfig._pluginLog._maxCount);
	}
	catch (ZQ::common::FileLogException& ex)
	{
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "create log file caught [%s]"), ex.getString());
		return 1;
	}



	if (false == initIceRunTime())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "Ice run-time initialization failed, exit!"));
		return 1;
	}

	if (false == initThreadPool())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "Thread pool initialization failed, exit!"));
		return 1;
	}

	if (false == openSessionWatchDog())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "Session watch dog initialization failed, exit!"));
		return 1;
	}

	//Initialize Penalty Manager
	_penaltyManager = new PenaltyManager(*this);
	if (!_penaltyManager) 
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "Create Penalty manager failed"));
		return 1;
	}
	_penaltyManager->start ();
	
	try
	{
		_streamerQuerier =  new streamerQuerier(*this);
		_streamerQuerier->start();
		
		_streamerReplciaSink = new streamerReplicaSink(*this,_penaltyManager);
		_streamerReplciaSink->start( );

		TianShanIce::ReplicaSubscriberPtr pSubscriber = _streamerReplciaSink;
		assert( _streamerQuerier != NULL );
		_pEventAdapter->ZQADAPTER_ADD(_pCommunicator, pSubscriber, "ReplicaSubscriber");

	}
	catch (const Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "create streamerQuerier caught %s"), ex.ice_name().c_str());
	}
	catch (...)
	{
		NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "create streamerQuerier caught unexpect exception"));
	}

	//warm up all ice interface
	warmUp();

	initSelectionManager( );
	
	if (false == openSafeStore())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "Open SafeStore initialization failed, exit!"));
		return 1;
	}


	_thrdConnService.open(this, &_fileLog);
	_thrdConnService.start();
	
	IClientSession* pSession = pSite->createClientSession(NULL, "rtsp://defaultSite/NGOD?asset=1");
	if (NULL != pSession && NULL != pSession->getSessionID())
		_globalSession = pSession->getSessionID();
	else 
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "global session create failed"));
		return 1;
	}	

	// -----------------------------------------------------------------------------
	// register sessionview servant
	// -----------------------------------------------------------------------------
	try
	{
		NGODr2c1::SessionViewImplPtr pSessionView = new NGODr2c1::SessionViewImpl(*this);
		_pEventAdapter->ZQADAPTER_ADD(_pCommunicator, pSessionView, "Ngod2View");
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "Register SessionView successfully"));
	}
	catch (const Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "Register SessionView caught %s"), ex.ice_name().c_str());
	}
	catch (...)
	{
		NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "Register SessionView caught unexpect exception"));
	}


	_ngodConfig.snmpRegister("ssm_NGOD2");

	mSnmpServant.setEndpoint( _ngodConfig._bind._endpoint );

	
	mSnmpServant.registerSnmpTable();

	// translate log into ssm_ngod2_sentry.log by zjm
	NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "NGOD initialize OK"));
	NGODEVENTLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "NGOD initialize OK"));

	return 0; // success to initialize
}

void NGODEnv::initSelectionManager( )
{
	{
		//update sops
		SOPS sops;
		const std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sopDatas = _sopConfig._sopRestrict._sopDatas;
		std::map< std::string , NGOD2::SOPRestriction::SopHolder >::const_iterator it = sopDatas.begin();
		for( ; it != sopDatas.end() ; it ++ )
		{//for every sop
			const std::vector<NGOD2::Sop::StreamerHolder>& streamers = it->second._streamerDatas;
			std::vector<NGOD2::Sop::StreamerHolder>::const_iterator itStreamer = streamers.begin();

			ResourceStreamerAttrMap streamerAttrs;
			for( ; itStreamer != streamers.end() ; itStreamer++ )
			{
				ResourceStreamerAttr attr;
				attr.streamServicePrx		= itStreamer->_streamServicePrx;
				attr.netId					= itStreamer->_netId;
				attr.endpoint				= itStreamer->_serviceEndpoint;
				attr.maxBw					= itStreamer->_totalBandwidth;
				attr.usedBw					= itStreamer->_usedBandwidth;
				attr.maxSessCount			= itStreamer->_maxStream;
				attr.usedSessCount			= itStreamer->_usedStream;
				attr.importChannelName		= itStreamer->_importChannel;
				attr.volumeNetId			= itStreamer->_storageNetId;
				attr.bAllPartitionAvail		= true;//hack for ssm_ngod2 current implementation
				streamerAttrs.insert( ResourceStreamerAttrMap::value_type( attr.netId , attr ) );
			}
			sops.insert( SOPS::value_type( it->first ,streamerAttrs ) );
		}
		mResManager.updateSopData( sops );
	}

	{//set max penalty value
		mSelectionEnv.mMaxPenaltyValue = _sopConfig._sopRestrict._maxPenaltyValue;
	}

	{
		ResourceVolumeAttrMap volumeAttrs;
		const NGOD2::LAMServer::ContentVolumeHolderS& volumes = _ngodConfig._lam._lamServer.contentVolumes;
		NGOD2::LAMServer::ContentVolumeHolderS::const_iterator it = volumes.begin() ;
		for( ; it != volumes.end() ; it++ )
		{
			ResourceVolumeAttr attr;
			attr.netId			=	it->second.netId;
			attr.bAllPartitions	=	true;//hack for ngod2 current implementation
			attr.level			=	it->second.cacheLevel;
			attr.bSupportNas	=	it->second.supportNasStreaming >= 1;
			attr.bSupportCache	=	it->second.cache >= 1;
			volumeAttrs.insert( ResourceVolumeAttrMap::value_type( it->first , attr) );
		}
		mResManager.updateVolumesData( volumeAttrs );
	}

	{
		ResourceImportChannelAttrMap icAttrs;
		const std::map<std::string,NGOD2::PassThruStreaming::ImportChannelHolder>& ics = _ngodConfig._passThruStreaming._importChannelDatas;
		std::map<std::string,NGOD2::PassThruStreaming::ImportChannelHolder>::const_iterator it = ics.begin();
		for( ; it != ics.end() ; it++ )
		{
			ResourceImportChannelAttr  attr;
			attr.netId				= it->second._name;
			attr.confMaxBw			= it->second._maxBandwidth;
			attr.confMaxSessCount	= it->second._maxImport;
			attr.usedBw				= it->second._usedBandwidth;
			attr.usedSessCount		= it->second._usedImport;
			attr.reportUsedBW		= it->second._reportUsedBandwidth;
			attr.reportMaxBW		= it->second._reportTotalBandwidth;
			attr.reportUsedSessCount= it->second._runningImportSessCount;
			icAttrs.insert( ResourceImportChannelAttrMap::value_type( attr.netId , attr) );
		}
		mResManager.updateImportChannelData( icAttrs );
	}
	mResManager.initResourceManager( _sopConfig._sopRestrict._enableWarmup > 0 );

	{//initialize content in test mode
		mSelectionEnv.mbContentTestMode = _ngodConfig._lam._lamTestMode._enabled >= 1;
		const NGOD2::LAMTestMode::LAMSubTestSetHolderS& testData= _ngodConfig._lam._lamTestMode.subTests;
		NGOD2::LAMTestMode::LAMSubTestSetHolderS::const_iterator it = testData.begin();
		for( ; it != testData.end() ; it++ )
		{
			using namespace  ::com::izq::am::facade::servicesForIce;
			AEInfo3 info;
			info.name		= it->contentName;
			info.bandWidth	= it->bandwidth;
			info.cueIn		= it->cueIn;
			info.cueOut		= it->cueOut;
			info.nasUrls	= it->urls;
			info.volumeList	= it->volumeList;
			mSelectionEnv.mTestModeContent.push_back( info );
		}
	}

	{
		mSelectionEnv.mbStreamerReplicaTestMode = _sopConfig._sopRestrict._replicaUpdateEnable <= 0;
	}

	{
		mSelectionEnv.mbIcReplicaTestMode = _ngodConfig._passThruStreaming._testMode >= 1;
	}
	mResManager.start();
}

int NGODEnv::doUninit()
{
	mResManager.stop();

	if( bQuited )
	{
		return 1;
	}
	mSnmpServant.unregisterSnmpTable();
	bQuited = true;

	NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "do uninitialization [%s]"), ZQ_PRODUCT_NAME);

	assert( _streamerQuerier!= NULL );
	if( _streamerQuerier )
	{
		_streamerQuerier->stop();
		delete _streamerQuerier;
		_streamerQuerier = NULL;
	}
	
	assert( _streamerReplciaSink!= NULL );
	if(_streamerReplciaSink)
	{
		_streamerReplciaSink->stop( );
	}

	// destroy global session
	//do not destroy the global session , it's a in-memory-object
// 	if (NULL != _pSite)
// 		_pSite->destroyClientSession(_globalSession.c_str());

	if(_thrdConnService.isRunning())
		_thrdConnService.stop();

	closeSafeStore();

	closeSessionWatchDog();

	uninitThreadPool();

	uninitIceRunTime();



	if (_penaltyManager) 
	{
		_penaltyManager->Stop ();
		delete _penaltyManager;
		_penaltyManager = NULL;
	}

	NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "uninitialization [%s] successfully"), ZQ_PRODUCT_NAME);

	return 0;
}
void NGODEnv::warmUp()
{
	// warm up LAM if needed and test mode is disabled
	if ((_ngodConfig._lam._enableWarmup != 0) && (_ngodConfig._lam._lamTestMode._enabled == 0))
	{
		NGOD2::LAM::LAMServerHolder& lamServer = _ngodConfig._lam._lamServer;
		try
		{			
			NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "Warmup() connect to [%s]"), lamServer._endpoint.c_str());
			lamServer._lamPrx= LAMFacadePrx::uncheckedCast(_pCommunicator->stringToProxy( lamServer._endpoint.c_str() ));
			NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Warmup() connect to [%s] OK"), lamServer._endpoint.c_str());
		}
		catch(const Ice::Exception& ex)
		{
			NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "Warmup() connect to [%s] failed with exception[%s]"),
				lamServer._endpoint.c_str(),ex.ice_name().c_str());
			//lamServer._lamPrx = NULL;
		}
		catch(...)
		{				
			NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "Warmup() connect to [%s] failed "),
				lamServer._endpoint.c_str());
			//lamServer._lamPrx = NULL;
		}
		mSelectionEnv.lamProxy = lamServer._lamPrx;
	}
	
	TianShanIce::ReplicaQueryPrx replicaQuery = NULL;
	if (_sopConfig._sopRestrict._enableWarmup != 0) 
	{
		std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sops = _sopConfig._sopRestrict._sopDatas;
		std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator itSop = sops.begin();		
		for( ; itSop != sops.end() ; itSop++ )
		{
			NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;			
			std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;				
			std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin ();		
			for ( ; itStreamer != streamers.end() ; itStreamer ++ )
			{
				if (itStreamer->_enabled <= 0) // add by zjm
				{
					itStreamer->_bReplicaStatus = false;
				}

				try
				{
					//NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "Warmup() connect to [%s]"), itStreamer->_strEndpoint.c_str());
					itStreamer->_streamServicePrx = TianShanIce::Streamer::StreamSmithAdminPrx::checkedCast(_pCommunicator->stringToProxy(itStreamer->_serviceEndpoint));					
					itStreamer->_lastReplicaUpdate = 0;
					NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Warmup() connect to [%s] OK"), itStreamer->_serviceEndpoint.c_str());					
				}
				catch(const Ice::Exception& ex)
				{
					_streamerQuerier->pushStreamer( itSop->first , itStreamer->_netId , itStreamer->_serviceEndpoint );
					NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "Warmup() connect to [%s] failed with exception[%s]"),
						itStreamer->_serviceEndpoint.c_str(),ex.ice_name().c_str());
					//itStreamer->_streamServicePrx = NULL;
				}
				catch(...)
				{
					_streamerQuerier->pushStreamer( itSop->first , itStreamer->_netId , itStreamer->_serviceEndpoint );
					NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "Warmup() connect to [%s] failed "),
						itStreamer->_serviceEndpoint.c_str());
					//itStreamer->_streamServicePrx = NULL;
				}
				try
				{
					if( _sopConfig._sopRestrict._replicaUpdateEnable >=  1 )
					{
						typedef std::map< std::string, NGOD2::PassThruStreaming::ImportChannelHolder> ImportChannelMap;
						ImportChannelMap& importChannels = _ngodConfig._passThruStreaming._importChannelDatas;

						replicaQuery = TianShanIce::ReplicaQueryPrx::checkedCast( itStreamer->_streamServicePrx );
						std::string::size_type pos = itStreamer->_netId.find("/");
						std::string groupId = itStreamer->_netId ;
						if( pos != std::string::npos )
						{
							groupId = groupId.substr( 0 , pos );
						}
						TianShanIce::Replicas reps = replicaQuery->queryReplicas("Streamer", groupId, true );
						TianShanIce::Replicas::const_iterator itRep = reps.begin();
						for( ; itRep != reps.end() ; itRep ++ )
						{
							if( stricmp( itRep->category.c_str() , "Streamer" ) == 0 )
							{
								std::string strTempNetId = itRep->groupId + "/" + itRep->replicaId;								

								if( stricmp(strTempNetId.c_str() , itStreamer->_netId.c_str() ) == 0 )
								{
									itStreamer->_bReplicaStatus = itRep->replicaState == TianShanIce::stInService;
									NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Warmup() set streamer[%s] to [%s]"),
										itStreamer->_netId.c_str() , itStreamer->_bReplicaStatus ? "enable": "disable" );
								}
							}							
						}
						
						reps = replicaQuery->queryReplicas("BandwidthUsage", groupId, true );
						itRep = reps.begin();
						for( ; itRep != reps.end() ; itRep ++ )
						{
							if( stricmp( itRep->category.c_str() ,"BandwidthUsage" ) == 0 )
							{
								bool bUseReportBandwidth = _sopConfig._sopRestrict._enableReportedImportChannelBandWidth >= 1;
								const std::string& groupId = itRep->groupId;
								const TianShanIce::Properties& replicaProps = itRep->props;
								TianShanIce::Properties::const_iterator itMaxBandWidth = replicaProps.find("UsedReplicaImportBandwidth");
								if( bUseReportBandwidth && ( itMaxBandWidth != replicaProps.end() ) && (!itMaxBandWidth->second.empty()) )
								{
									//int64 maxBandwith = 0;	
									//sscanf( itMaxBandWidth->second.c_str(),"%lld",&maxBandwith );
									int64 usedImportBandwidth = 0;
									int64 totalImportBandwidth = 0;
									int32 runningImportSessCount = 0;

									ZQTianShan::Util::getPropertyDataWithDefault( replicaProps, "UsedReplicaImportBandwidth", 0, usedImportBandwidth);
									ZQTianShan::Util::getPropertyDataWithDefault( replicaProps, "TotalReplicaImportBandwidth",0, totalImportBandwidth);
									ZQTianShan::Util::getPropertyDataWithDefault( replicaProps, "runningImportSessionCount",  0, runningImportSessCount);


									ImportChannelMap::iterator itChannel = importChannels.find( groupId );
									if( itChannel == importChannels.end() )
									{
										NGOD2::PassThruStreaming::ImportChannelHolder holder;							
										holder._name				=	groupId;
										holder._bandwidth			=	static_cast<int32>(usedImportBandwidth/1000);
										holder._maxBandwidth		=	totalImportBandwidth;					
										holder._reportUsedBandwidth =	usedImportBandwidth;
										holder._reportTotalBandwidth=	totalImportBandwidth;
										holder._runningImportSessCount=	runningImportSessCount;
										holder._usedBandwidth		=	usedImportBandwidth;
										holder._maxImport			=	0x7FFFFFFF;
										holder._usedImport			=	0;
										holder._bConfiged			=	false;
										importChannels.insert(ImportChannelMap::value_type( groupId , holder ) );
										NGODLOG(ZQ::common::Log::L_INFO,
											CLOGFMT(updateReplica,"get new replica import channel bandwidth[%lld] with name[%s]"),
											holder._maxBandwidth,
											groupId.c_str()	);
									}
									else
									{
// 										if( (usedImportBandwidth != itChannel->second._reportUsedBandwidth) ||
// 											(runningImportSessCount != itChannel->second._runningImportSessCount) ||
// 											(totalImportBandwidth != itChannel->second._reportTotalBandwidth ) )
										{
											NGODLOG(ZQ::common::Log::L_INFO,
												CLOGFMT(updateReplica,"update replica import channel:name[%s] usedBandwidth[%lld] totalBandwidth[%lld] runningSess[%d] "),
												groupId.c_str(), 
												usedImportBandwidth,
												totalImportBandwidth,
												runningImportSessCount);											
											itChannel->second._reportUsedBandwidth		= usedImportBandwidth;
											itChannel->second._reportTotalBandwidth		= totalImportBandwidth;
											itChannel->second._runningImportSessCount	= runningImportSessCount;
										}
									}
								}
							}
						}
					}
					else
					{
						itStreamer->_bReplicaStatus = true;
						NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Warmup() disable streamer replica check, set streamer[%s] to [%s]"),
							itStreamer->_netId.c_str() , itStreamer->_bReplicaStatus ? "enable": "disable" );
					}
				}
				catch(const Ice::Exception& ex)
				{
					NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Warmup() can't get streamer[%s]'s replica information, set it to disable,Ice exception [%s]"),
						itStreamer->_netId.c_str() ,ex.ice_name().c_str() );
					itStreamer->_bReplicaStatus	=	false;
				}
				catch(...)
				{
					NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Warmup() can't get streamer[%s]'s replica information, set it to disable"),
						itStreamer->_netId.c_str() );
					itStreamer->_bReplicaStatus	=	false;
				}
			}
		}
	}
}

bool NGODEnv::openSessionWatchDog()
{
	_pSessionWatchDog = new SessionWatchDog(*this);

	if (NULL != _pSessionWatchDog)
	{
		_pSessionWatchDog->start();
		return true;
	}
	else 
		return false;
}

void NGODEnv::closeSessionWatchDog()
{
	if (NULL != _pSessionWatchDog)
		_pSessionWatchDog->quit();

	_pSessionWatchDog = NULL;
}

bool NGODEnv::initIceRunTime()
{
	try
	{
		// DO: initialize properties for ice run-time
		int i=0;
		Ice::PropertiesPtr props = Ice::createProperties(i, NULL);
		if (NULL != props)
		{
			std::vector<NGOD2::IceProperty::IcePropertyHolder>::iterator itIceProp;
			for (itIceProp = _ngodConfig._iceProps._propDatas.begin();
				itIceProp != _ngodConfig._iceProps._propDatas.end(); itIceProp ++)
			{
				props->setProperty(itIceProp->_name, itIceProp->_value);
			}
		}

		// DO: create communicator
		Ice::InitializationData idt;
		idt.properties = props;
		idt.logger = new TianShanIce::common::IceLogI(&_iceLog);
		_pCommunicator=Ice::initialize(i,NULL,idt);
	}
	catch(const Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "create Ice Communicator caught [%s]"), ex.ice_name().c_str());
		return false;
	}
	
	try
	{
		_pEventAdapter = ZQADAPTER_CREATE(_pCommunicator, "ListenEventAdapter", _ngodConfig._bind._endpoint.c_str(), NGODLOG);
#ifndef _INDEPENDENT_ADAPTER

		std::vector<NGOD2::PublishLogs::PublishLogHolder>& logSyntaxs = _ngodConfig._publishLogs._logDatas;        
		std::vector<NGOD2::PublishLogs::PublishLogHolder>::const_iterator iter = logSyntaxs.begin();
        for (; iter != logSyntaxs.end(); iter++ ) 
        {		
            if (!_pEventAdapter->publishLogger(iter->_path.c_str(), iter->_syntax.c_str(),
												iter->_key.c_str() , iter->_type.c_str() ))
            {			
                NGODLOG(ZQ::common::Log::L_ERROR, 
					CLOGFMT(NGODEnv,"Failed to publish logger name[%s] synax[%s] key[%s] type[%s]"),
                    iter->_path.c_str(), iter->_syntax.c_str(),iter->_key.c_str() , iter->_type.c_str() );
            }		
            else			
            {			
                NGODLOG(ZQ::common::Log::L_INFO,  
					CLOGFMT(NGODEnv,"Publish logger name[%s] synax[%s] key[%s] type[%s] successful"),
                    iter->_path.c_str(), iter->_syntax.c_str() , iter->_key.c_str() , iter->_type.c_str() );
            }		
        }	
#endif
		_pEventAdapter->activate();
	}
	catch(Ice::Exception& ex) 
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "create adapter: [%s] caught [%s]"), _ngodConfig._bind._endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	
	return true;
}

void NGODEnv::uninitIceRunTime()
{
	try
	{
		_pEventChannal->clean();
		if (NULL != _pEventAdapter)
		{
			_pEventAdapter->deactivate();
		}
		_pEventAdapter = NULL;
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "deactive adapters caught [%s]"), ex.ice_name().c_str());
	}
	try
	{
		_pCommunicator->destroy();
	}
	catch(...)
	{
	}
	_pEventChannal = NULL;
}

bool NGODEnv::initThreadPool()
{
	_pThreadPool = new ZQ::common::NativeThreadPool(_ngodConfig._rtspSession._monitorThreads);
	return (NULL != _pThreadPool) ? true : false;
}

void NGODEnv::uninitThreadPool()
{
	if (NULL != _pThreadPool)
		delete _pThreadPool;

	_pThreadPool = NULL;
}

int NGODEnv::setLogDir(const char* pLogDir)
{
	if (NULL != pLogDir && strlen(pLogDir) > 0)
	{
		_ngodConfig._pluginLog._path = pLogDir;
		_ngodConfig._pluginLog._path += "\\ssm_NGOD2.log";
	}
	else 
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "log directory is empty"));
	return 0;
}

int NGODEnv::setConfigPath(const char* pConfigPath)
{
	if (NULL != pConfigPath && strlen(pConfigPath) > 0)
	{
		_configPath = pConfigPath;
		_configPath += "\\ssm_NGOD2.xml";
	}
	else 
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "configuration directory is empty"));
	return 0;
}

bool NGODEnv::loadConfig()
{
	NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "**************** [Load %s] ****************"), ZQ_PRODUCT_NAME);

    if(!_ngodConfig.load(_configPath.c_str()))
    {
        NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "load config failed"));
        return false;
    }
	if (!_sopConfig.load(_ngodConfig._sopProp.fileName.c_str()))
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "loadConfig() : failed to load sop config [%s]"), _ngodConfig._sopProp.fileName.c_str());
		return false;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage > 0 && _ngodConfig._protocolVersioning.enableVersioning > 0)
	{
		NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "loadConfig() : Enable versioning is ignored as rtsp NPT Usage is enabled"));
	}
	else
	{
		if (_ngodConfig._protocolVersioning.enableVersioning > 0)
		{
			NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NGODEnv, "loadConfig() : Enable protocol versioning"));
		}
	}
	//parse and adjust configuration

	
	//adjust configuration for SopRestriction

	//adjust max retry count
	_sopConfig._sopRestrict._retryCount = _sopConfig._sopRestrict._retryCount > 1000 ? 1000 : _sopConfig._sopRestrict._retryCount;
	_sopConfig._sopRestrict._retryCount = _sopConfig._sopRestrict._retryCount < 0 ? 0 : _sopConfig._sopRestrict._retryCount;
	
	//adjust max penalty value
	_sopConfig._sopRestrict._maxPenaltyValue = _sopConfig._sopRestrict._maxPenaltyValue > 10000 ? 10000 : _sopConfig._sopRestrict._maxPenaltyValue;
	_sopConfig._sopRestrict._maxPenaltyValue = _sopConfig._sopRestrict._maxPenaltyValue < 0 ? 0 : _sopConfig._sopRestrict._maxPenaltyValue;

	//adjust streamer query interval
	_sopConfig._sopRestrict._streamerQueryInterval = _sopConfig._sopRestrict._streamerQueryInterval < 100 ? 100  : _sopConfig._sopRestrict._streamerQueryInterval;

	// adjust timeout count by zjm
	if (_ngodConfig._rtspSession._timeoutCount < 3)
	{
		_ngodConfig._rtspSession._timeoutCount = 3;
	}
	if (_ngodConfig._rtspSession._timeoutCount > 100)
	{
		_ngodConfig._rtspSession._timeoutCount = 100;
	}
	std::map<std::string , NGOD2::SOPRestriction::SopHolder>::iterator itSop;
	for (itSop = _sopConfig._sopRestrict._sopDatas.begin();
		itSop != _sopConfig._sopRestrict._sopDatas.end(); itSop ++)
	{	
		NGOD2::SOPRestriction::SopHolder& sopConf = itSop->second;
		std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = sopConf._streamerDatas.begin();
		for ( itStreamer = sopConf._streamerDatas.begin();
				itStreamer != sopConf._streamerDatas.end(); 
				itStreamer ++)
		{
			itStreamer->_penalty		=	0;
			std::string& streamerEndpoint	= itStreamer->_serviceEndpoint;
			if( streamerEndpoint.find(":") == std::string::npos )
			{
				streamerEndpoint	=	"StreamSmith:" + streamerEndpoint;
			}
			//parse volume into netid and volume-name

			std::string associatedVolume = itStreamer->_volume;
			std::string::size_type pos = associatedVolume.find("/");
			if( pos != std::string::npos )
			{
#pragma message(__MSGLOC__"TODO: not implemented yet! need more thinking")
				itStreamer->_storageNetId	=	associatedVolume.substr( 0 , pos );
				std::string strTemp = associatedVolume.substr( pos + 1 );
				itStreamer->volumes.clear( );
				if( strTemp.find("*") != std::string::npos )
				{
					itStreamer->_bAllVolumeAvailble = true;
				}
			}
			//adjust total bandwidth 
			itStreamer->_totalBandwidth	=	static_cast<int64>(itStreamer->_totalBW) * 1000;
			itStreamer->_usedBandwidth	=	0;		

			itStreamer->_usedStream		=	0;
			
			itStreamer->_streamServicePrx = NULL;
			NGODLOG(ZQ::common::Log::L_NOTICE,
				CLOGFMT(NGODEnv,"Name[%s] ServiceGroup[%d] NetId[%s] endpoint[%s] volumn=[%s] TotalBW=[%lld] maxStream=[%d]"),
				itSop->first.c_str(),
				itSop->second._serviceGroup,
				itStreamer->_netId.c_str(),
				itStreamer->_serviceEndpoint.c_str(),
				itStreamer->_volume.c_str(),
				itStreamer->_totalBandwidth,
				itStreamer->_maxStream);
		}
	}
		
	
	std::map< std::string, NGOD2::PassThruStreaming::ImportChannelHolder >::iterator itImportChannel;
	for (itImportChannel = _ngodConfig._passThruStreaming._importChannelDatas.begin();
	itImportChannel != _ngodConfig._passThruStreaming._importChannelDatas.end(); itImportChannel ++)
	{
		itImportChannel->second._maxBandwidth	=	static_cast<int64>( itImportChannel->second._bandwidth ) * 1000;
		itImportChannel->second._usedBandwidth	=	0;
		itImportChannel->second._usedImport	=	0;		
		NGODLOG(ZQ::common::Log::L_INFO , 
			CLOGFMT(NGODEnv , "ImportChannel[%s] with MaxSessions[%d] maxBW[%lld]"),
			itImportChannel->first.c_str(),
			itImportChannel->second._maxImport,
			itImportChannel->second._maxBandwidth);
	}
	


	NGOD2::LAM::LAMServerHolder& lamServer	=	_ngodConfig._lam._lamServer;
	std::string& lamServerEndpoint = lamServer._endpoint;
	if( lamServerEndpoint.find(":") == std::string::npos )
	{
		lamServerEndpoint = "LAMFacade:" + lamServerEndpoint;
	}
	
	//check the volume
	NGOD2::LAMServer::ContentVolumeHolderS& storages = _ngodConfig._lam._lamServer.contentVolumes;
	NGOD2::LAMServer::ContentVolumeHolderS::iterator itStorage = storages.begin();
	NGOD2::LAMServer::ContentVolumeHolderS temp;
	for( ; itStorage != storages.end() ; itStorage ++ )
	{
		const std::string& storageName = itStorage->first;
		std::string::size_type posSlash = storageName.find("/");
		if( posSlash != std::string::npos )
		{
			itStorage->second.netId	=	storageName.substr(0,posSlash);
			itStorage->second.volumeName.clear();
#pragma message(__MSGLOC__"TODO: hard code here, need to be modified")
			itStorage->second.bAllVolumeAvailable = true;
			temp[itStorage->second.netId] = itStorage->second;
		}
	}
#pragma message(__MSGLOC__"TODO : I am not sure can I use a map in LAM volume config to record the volume's info")
	storages = temp;
// 	for( itStorage = temp.begin() ; itStorage != temp.end() ; itStorage ++ )
// 	{
// 		storages[itStorage->first] = itStorage->second;
// 	}

	return true;
}

bool NGODEnv::ConnectIceStorm()
{
#if defined _DEBUG || defined DEBUG
#pragma message(__MSGLOC__"TODO : remove this line in release version")
	return false;
#endif
	try
	{
		// translate log into ssm_ngod2_events.log by zjm

        NGODEVENTLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "do connect to EventChannel: [%s]"), _ngodConfig._iceStorm._endpoint.c_str());
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "do connect to EventChannel: [%s]"), _ngodConfig._iceStorm._endpoint.c_str());
		
		TianShanIce::Streamer::StreamEventSinkPtr _sEvent = new StreamEventSinkI(this);
		TianShanIce::Streamer::PlaylistEventSinkPtr _playlistEvent = new PlayListEventSinkI(this);
		_pEventChannal=new TianShanIce::Events::EventChannelImpl(_pEventAdapter, _ngodConfig._iceStorm._endpoint.c_str());
		TianShanIce::Properties qos;
		_pEventChannal->sink(_sEvent, qos);
		_pEventChannal->sink(_playlistEvent, qos);
		
		NGODEVENTLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "connect to EventChannel: [%s] successfully"), _ngodConfig._iceStorm._endpoint.c_str());
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "connect to EventChannel: [%s] successfully"), _ngodConfig._iceStorm._endpoint.c_str());
	}
	catch(const TianShanIce::BaseException& ex)
	{
		NGODEVENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "connect to EventChannel: [%s] caught %s:%s"), _ngodConfig._iceStorm._endpoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "connect to EventChannel: [%s] caught %s:%s"), _ngodConfig._iceStorm._endpoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		NGODEVENTLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "connect to EventChannel: [%s] caught an %s"), _ngodConfig._iceStorm._endpoint.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "connect to EventChannel: [%s] caught an %s"), _ngodConfig._iceStorm._endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}



bool NGODEnv::openSafeStore()
{
	///initialize random seed

	srand( (unsigned int)time(NULL) );
	// DO: create directories
	if (_ngodConfig._database._path[_ngodConfig._database._path.size() - 1] != '\\'
		&& _ngodConfig._database._path[_ngodConfig._database._path.size() - 1] != '/')
	{
		_ngodConfig._database._path += '\\';
	}
	_ngodConfig._database._path += "ssm_NGOD2\\";
	std::string pathStr(_ngodConfig._database._path);
	std::vector<std::string> paths;
	std::string tmp_path;
	if (pathStr[pathStr.size() - 1] == '\\' || pathStr[pathStr.size() - 1] == '/')
		pathStr[pathStr.size() - 1] = '\0';
	paths.push_back(pathStr);
	tmp_path = ZQ::StringOperation::getLeftStr(pathStr, "\\/", false);
	while(tmp_path.size())
	{
		paths.push_back(tmp_path);
		tmp_path = ZQ::StringOperation::getLeftStr(tmp_path, "\\/", false);
	}
	int paths_size = paths.size();
	for (int i = paths_size - 1; i >= 0; i--)
	{
		::CreateDirectoryA(paths[i].c_str(), NULL);
	}

	try
	{
		_pFactory = new NGODFactory(*this);
		_pCommunicator->addObjectFactory(_pFactory, NGODr2c1::Context::ice_staticId());
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "Catch [%s] when create and add object factory "), ex.ice_name().c_str());
		return false;
	}

	try
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "Do create freeze connection"));
		_pSafeStoreConn = ::Freeze::createConnection(_pCommunicator, _ngodConfig._database._path);
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(NGODEnv, "Create freeze connection successfully"));
	}
	catch (Freeze::DatabaseException& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "Catch [%s]:[%s] when create freeze connection"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "Catch [%s] when create freeze connection"), ex.ice_name().c_str());
		return false;
	}
	
	_pStreamIdx = new NGODr2c1::StreamIdx(INDEXFILENAME(StreamIdx));
	_pGroupIdx = new NGODr2c1::GroupIdx(INDEXFILENAME(GroupIdx));

	std::vector<Freeze::IndexPtr> indexs;
	indexs.push_back(_pStreamIdx);
	indexs.push_back(_pGroupIdx);

//	ZQ::common::MutexGuard lk(_contextEvtrLock);
	try
	{
		{
#define MAX_CONTENTS 100000
#define DEFAULT_CACHE_SIZE (16*1024*1024)
			::std::string dbConfFile = _ngodConfig._database._path + FNSEPS + "DB_CONFIG";
			if ( -1 == ::access(dbConfFile.c_str(), 0))
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "initializing %s"), dbConfFile.c_str());
				FILE* f = ::fopen(dbConfFile.c_str(), "w+");
				if (NULL != f)
				{
					::fprintf(f, "set_lk_max_locks %d\n",   MAX_CONTENTS);
					::fprintf(f, "set_lk_max_objects %d\n", MAX_CONTENTS);
					::fprintf(f, "set_lk_max_lockers %d\n", MAX_CONTENTS);
					::fprintf(f, "set_cachesize 0 %d 0\n",	DEFAULT_CACHE_SIZE);
					::fclose(f);
				}
			}
		}
#if ICE_INT_VERSION / 100 >= 303		
		_pContextEvtr = Freeze::createBackgroundSaveEvictor(_pEventAdapter, _ngodConfig._database._path.c_str(), "Contexts", 0, indexs);
#else
		_pContextEvtr = Freeze::createEvictor(_pEventAdapter, _ngodConfig._database._path.c_str(), "Contexts", 0, indexs);
#endif
		_pContextEvtr->setSize(_ngodConfig._rtspSession._cacheSize);
		_pEventAdapter->addServantLocator(_pContextEvtr, SERVANT_TYPE);
	}
	catch (Freeze::DatabaseException& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "create freeze evictor caught [%s]:[%s]"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(NGODEnv, "create freeze evictor caught [%s]"), ex.ice_name().c_str());
		return false;
	}

	unsigned int sessnum = 0;
	INOUTMAP inoutMap;
	inoutMap[MAP_KEY_METHOD] = "init";
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", MAX_SESSION_CONTEXT);
	while (tItor->hasNext())
	{
		Ice::Identity ident = tItor->next();
		NGODr2c1::ctxData NewContext;
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (openContext(ident.name, NewContext, pNewContextPrx, inoutMap))
		{
			NgodResourceManager::StreamerResourcePara para;
			para.identifier			= NewContext.sopname;
			para.requestBW			= (int32)NewContext.usedBandwidth;
			para.bNeedImportChannel	= !NewContext.importChannelName.empty();

			para.method				= "restore";
			para.sessionId			= NewContext.ident.name;
			para.cseq				= "1";
			
			mResManager.allocateResource(para,NewContext.streamNetId);
				
// 			//get record,calculate usedBandWidth
// 			{
// 				//初始化阶段，尽管lock
// 				ZQ::common::MutexGuard gd(_lockSopMap);
// 				std::string&	strSop				= NewContext.sopname;
// 				std::string&	strNetId			= NewContext.streamNetId;
// 				std::string&	strImportChannel	= NewContext.importChannelName;
// 				Ice::Long&		bandwidth			= NewContext.usedBandwidth;
// 				std::string&	strSess				= ident.name;
// 
// 				std::map< std::string , NGOD2::SOPRestriction::SopHolder>& sops = _sopConfig._sopRestrict._sopDatas;
// 				std::map< std::string , NGOD2::SOPRestriction::SopHolder>::iterator itSop = sops.find(strSop);
// 				if ( itSop == sops.end() ) 
// 				{
// 					NGODLOG(ZQ::common::Log::L_INFO,
// 						CLOGFMT(NGODEnv,"failOver() can't find the sop [%s] with netID[%s] usedBW[%lld]"),
// 							strSop.c_str(), strNetId.c_str(),bandwidth);
// 				}
// 				else
// 				{
// 					bool	bFound = false;
// 					std::vector<NGOD2::Sop::StreamerHolder>& streamers = itSop->second._streamerDatas;
// 					std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin();
// 					
// 					for( ; itStreamer != streamers.end() ; itStreamer ++  )					
// 					{
// 						if ( itStreamer->_netId  == strNetId ) 
// 						{
// 							bFound = true;
// 							itStreamer->_usedBandwidth					+= bandwidth; 
// 							itStreamer->_usedStream						+= 1;
// 
// 							
// 							NGODLOG(ZQ::common::Log::L_INFO,
// 								CLOGFMT(NGODEnv,"failOver() session[%s] used bandwidth[%lld] with SOP[%s] netID[%s],"
// 								" and now totalBW[%lld] usedBW[%lld] totalStreamCount[%d] usedStreamCount[%d]"),
// 								strSess.c_str(),bandwidth,
// 								strSop.c_str(), strNetId.c_str(),
// 								itStreamer->_totalBandwidth,
// 								itStreamer->_usedBandwidth,
// 								itStreamer->_maxStream,
// 								itStreamer->_usedStream );
// 							if ( !strImportChannel.empty() )
// 							{
// 								ZQ::common::MutexGuard lockImportChannel(_lockSopMap);
// 								std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder >& importChannles = _ngodConfig._passThruStreaming._importChannelDatas;
// 
// 								std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder >::iterator itImportChannel = importChannles.find(strImportChannel);
// 								if ( itImportChannel == importChannles.end() )
// 								{
// 									NGODLOG(ZQ::common::Log::L_INFO , 
// 										CLOGFMT( NGODEnv , "failOver() Session[%s] have importChannel[%s]"
// 										" but can't find the channel in ImportChannelMap" ),
// 										strSess.c_str() ,
// 										strImportChannel.c_str() );
// 								}
// 								else
// 								{
// 									itImportChannel->second._usedBandwidth			+= bandwidth;
// 									itImportChannel->second._usedImport				+= 1;
// 									NGODLOG(ZQ::common::Log::L_INFO ,
// 										CLOGFMT( NGODEnv , "failOver() Session[%s] with ImportChannel[%s] "
// 										"and now usedSessions[%d] totalSessions[%d] usedBW[%lld] totalBW[%lld]" ),
// 										strSess.c_str( ) ,
// 										strImportChannel.c_str(),
// 										itImportChannel->second._usedImport,
// 										itImportChannel->second._maxImport,
// 										itImportChannel->second._usedBandwidth,
// 										itImportChannel->second._maxBandwidth);
// 								}
// 								
// 							}
// 						}						
// 					}
// 				}
// 			}
			//do not care about the stream's status , leave it to ontimer checker
// 			bool bObjExist = true;
// 			try
// 			{
// 				//
// 				TianShanIce::Streamer::StreamPrx stream = 
// 					TianShanIce::Streamer::StreamPrx::uncheckedCast(_pCommunicator->stringToProxy( NewContext.streamFullID ) );
// 				if(stream)
// 				{
// 					try
// 					{
// 						stream->ice_ping();
// 					}
// 					catch( const Ice::ObjectNotExistException& )
// 					{
// 						_fileLog(ZQ::common::Log::L_WARNING,
// 							CLOGFMT(NGODEnv, "Session[%s];streamId[%s] failed to query object on stream service , session will be tearminated"),
// 							NewContext.ident.name.c_str(),
// 							NewContext.streamFullID.c_str() );
// 						bObjExist = false;
// 					}
// 					catch( const Ice::Exception& )
// 					{//nothing
// 					}
// 				}
// 			}
// 			catch( const Ice::Exception& ex )
// 			{
// 				_fileLog(ZQ::common::Log::L_WARNING,
// 					CLOGFMT(NGODEnv, "Session[%s];streamId[%s] failed to query object on stream service"),
// 					NewContext.ident.name.c_str(),
// 					NewContext.streamFullID.c_str() );
// 				bObjExist = false;
// 			}			
// 			if( !bObjExist )
// 			{
// 				INOUTMAP inoutMap;
// 				_fileLog(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"session[%s]'s stream is gone , destroy the session"),
// 					NewContext.ident.name.c_str() );
// 				removeContext( NewContext.ident.name , inoutMap );
// 			}
// 			else
			{
				
				//_pSessionWatchDog->watchSession(NewContext.ident, (long) (sessnum ++) * 20 + (long) (_ngodConfig._rtspSession._timeout) * 1000 , true );
				
				//wakeup the context in a random interval way ( not so random )	
				_pSessionWatchDog->watchSession( NewContext.ident, rand()% 20000 );

				if (NULL != _pSite->createClientSession(NewContext.ident.name.c_str(), NewContext.resourceURL.c_str()))
					NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "create client session: [%s] when initialize plug-in"), NewContext.ident.name.c_str());
			}
		}
	}

	return true;
}

void NGODEnv::closeSafeStore()
{
	if (NULL != _pSafeStoreConn)
	{
		try
		{
			_pSafeStoreConn->close();
		}
		catch (Freeze::DatabaseException& ex)
		{
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "Do close freeze connection caught [%s]:[%s]"), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "Do close freeze connection caught [%s]"), ex.ice_name().c_str());
		}
	}
	_pSafeStoreConn = NULL;
	_pContextEvtr	= NULL;
	_pGroupIdx		= NULL;
	_pStreamIdx		= NULL;
}

bool NGODEnv::openContext(const std::string& sessId, NGODr2c1::ctxData& context, NGODr2c1::ContextPrx& pContextPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE] = "\0";

	Ice::Identity ident;
	ident.category = SERVANT_TYPE;
	ident.name = sessId;
	if (ident.name.empty())
		ident.name = inoutMap[MAP_KEY_SESSION];

	std::string method, sequence, session;
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];
	session = ident.name;

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "openContext()"));

	std::string notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, 
		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);

	try
	{
		pContextPrx = NGODr2c1::ContextPrx::uncheckedCast(_pEventAdapter->createProxy(ident));
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "uncheckedCast to client session context proxy caught [%s]", ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	if (NULL == pContextPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result client session context proxy is NULL");
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(NGODEnv, "session context to getState()"));
	NGODr2c1::ctxData& ctxDt=context;
	try
	{
		pContextPrx->getState(ctxDt);
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "do getState() on session context: [%s] caught [%s]", ident.name.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}	


	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(NGODEnv, "session context gained successfully"));

	return true;
}

bool NGODEnv::addContext(const NGODr2c1::ctxData& context, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];


	DWORD dwT1 = GetTickCount();
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "to save session context"));

	// add by zjm 
	std::string notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, 
		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);


	::Ice::ObjectPrx basePrx = NULL;
	NGODr2c1::ContextPtr pContext= new NGODr2c1::ContextImpl(*this);
	try
	{
		ZQ::common::MutexGuard lk(_contextEvtrLock);		
		pContext->data = context;
		pContext->streamShortID =context.streamShortID;
		pContext->groupID =context.groupID;
		basePrx = _pContextEvtr->add(pContext, context.ident);
	}
	catch (Freeze::DatabaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "save context to evictor caught [%s]:[%s]", ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "save context to evictor caught [%s]", ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	if (NULL == basePrx)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1, "save context result is null");
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(NGODEnv, "session context saved successfully, used[%d]ms"), GetTickCount()-dwT1);

	return true;
}

RequestProcessResult NGODEnv::doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Req(%p), enter FixupRequest"), pReq);

	char szBuf[MY_BUFFER_SIZE * 2];
	uint16 szBufLen;
	memset(szBuf, 0, sizeof(szBuf));
	// char szBuf[MY_BUFFER_SIZE * 2] = {'\0'};

	IServerResponse* pResponse = NULL;
	pResponse = pReq->getResponse();
	if (NULL == pResponse)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "Req(%p), response object is null"), pReq);
		return RequestError;
	}

	std::string session, method, sequence;
	const char* pHeaderStr = NULL;
	szBufLen = sizeof(szBuf) - 1;
	pHeaderStr = pReq->getHeader(NGOD_HEADER_SESSION, szBuf, &szBufLen);
	if (NULL != pHeaderStr)
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Req(%p), session: [%s]"), pReq, session.c_str());
		session = pHeaderStr;
	}
	else 
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Req(%p), No session id, use global session: [%s]"), pReq, _globalSession.c_str());
		session = _globalSession;
		pReq->setHeader(NGOD_HEADER_SESSION, (char*) session.c_str());
	}

	pHeaderStr = NULL;
	szBufLen = sizeof(szBuf) - 1;
	pHeaderStr = pReq->getHeader(NGOD_HEADER_SEQ, szBuf, &szBufLen);
	sequence = (NULL != pHeaderStr) ? pHeaderStr : "";

	// DO: get value of server header
	if (true == _serverHeader.empty())
	{
		_serverHeader = ZQ_PRODUCT_NAME_SHORT;
		/*szBufLen = sizeof(szBuf) - 1;
		const char* pServer = pReq->getHeader(NGOD_HEADER_SERVER, szBuf, &szBufLen);
		if (pServer != NULL && strlen(pServer) != 0)
		{
			_serverHeader = pServer;
			_serverHeader += " ";
			_serverHeader += ZQ_PRODUCT_NAME_SHORT;
		}
		else 
			_serverHeader = ZQ_PRODUCT_NAME_SHORT;*/
	}

	RTSP_VerbCode retVerbCode = pReq->getVerb();
	
	if (retVerbCode == RTSP_MTHD_SETUP)
	{
		// add by zjm to get require header
		pHeaderStr = pReq->getHeader(NGOD_HEADER_REQUIRE, szBuf, &szBufLen);
		std::string strRequire = (NULL != pHeaderStr) ? pHeaderStr : "";

		// add by zjm to support mutilple require options
		int ngodVersion = NgodVerCode_UNKNOWN;
		std::vector<std::string> requires;
		ZQ::common::stringHelper::SplitString(strRequire, requires);
		std::vector<std::string>::iterator strIter = requires.begin();
		for (; strIter != requires.end(); strIter++)
		{
			if (*strIter == "com.comcast.ngod.r2" && ngodVersion == NgodVerCode_UNKNOWN)
			{
				ngodVersion = NgodVerCode_R2;
			}
			if (*strIter == "com.comcast.ngod.r2.decimal_npts")
			{
				ngodVersion = NgodVerCode_R2_DecNpt;
			}
		}

		std::string originalURI, strResourceURI;
		// DO: get url from ODRM request, and format it to "rtsp://<server>:<port>
		pReq->getUri(szBuf, sizeof(szBuf) - 1);
		originalURI = "rtsp://";
		originalURI += szBuf;
		
		// DO: get content body form request, and then take out the value of providerId and AssetId
		std::string strContent;
		szBufLen = sizeof(szBuf) - 1;
		const char* pContLen = pReq->getHeader(HeaderContentLength, szBuf, &szBufLen);
		unsigned __int32 cntLen = atoi(pContLen?pContLen:"");
		cntLen ++;
		unsigned char* cntBuff = new unsigned char[cntLen];
		cntBuff[cntLen - 1] = '\0';
		const char* pRetStr = pReq->getContent(cntBuff, &cntLen);
		strContent = (NULL != pRetStr) ? pRetStr : "";
		delete []cntBuff;

		// parse a=X-playlist-item: <provider-id> <asset-id>[ <range>][ tricks/[F][R][P] ]
		int asset_count = 0;
		const char* next_content = strContent.c_str();
		const char* pTemp = NULL;
		std::vector<std::string> assets;
		pTemp = strstr(next_content, "a=X-playlist-item:");
		while (pTemp != NULL)
		{
			pTemp += strlen("a=X-playlist-item:");
			next_content = pTemp;
			std::vector<std::string> temp_strs;
			ZQ::StringOperation::splitStr(pTemp, " \r\n\t", temp_strs);
			if (temp_strs.size() >= 2)
			{
				// if temp_str.size() == 2
				// a=X-playlist-item: <provider-id> <asset-id>
				std::string provider_encode, asset_encode;
				int outlen = MY_BUFFER_SIZE - 1;
				ZQ::common::URLStr::encode(temp_strs[0].c_str(), szBuf, outlen);
				provider_encode = szBuf;
				outlen = MY_BUFFER_SIZE - 1;
				ZQ::common::URLStr::encode(temp_strs[1].c_str(), szBuf, outlen);
				asset_encode = szBuf;
				snprintf(szBuf, MY_BUFFER_SIZE - 1, "item%d=%s#%s", asset_count, provider_encode.c_str(), asset_encode.c_str());
				// if temp_str.size() == 3
				// a=X-playlist-item: <provider-id> <asset-id> <range>
				if (temp_strs.size() >= 3) {
					std::string queIn, queOut, tStr(szBuf);
					int tPos;
					if ( true == ZQ::StringOperation::hasChar(temp_strs[2], '-', tPos) ) 
					{
						queIn = ZQ::StringOperation::midStr(temp_strs[2], -1, tPos);
						queOut = ZQ::StringOperation::rightStr(temp_strs[2], tPos);
						if (false == queIn.empty())
						{
							int cuein = 0;
							if( _ngodConfig._MessageFmt.rtspNptUsage >= 1)//enable rtsp npt usage
							{
								float ftmp = 0.0f;
								sscanf(queIn.c_str(),"%f",&ftmp);
								cuein = (int)(ftmp*1000);
							}
							else
							{
								
								if (_ngodConfig._protocolVersioning.enableVersioning > 0 &&
									ngodVersion == NgodVerCode_R2_DecNpt)
									//strRequire == "com.comcast.ngod.r2.decimal_npts")
								{
									// enable versioning
									float ftmp = 0.0f;
									sscanf(queIn.c_str(),"%f",&ftmp);
									cuein = (int)(ftmp*1000);

								}
								else
								{ 
									// disable versioning
									sscanf(queIn.c_str() ,"%x",&cuein);
								}
							}
							snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&cueIn%d=%d", tStr.c_str(), asset_count, cuein);
						}
						if (false == queOut.empty())
						{
							tStr = szBuf;
							int cueout = 0;
							if( _ngodConfig._MessageFmt.rtspNptUsage >= 1)//enable rtsp npt usage
							{
								float ftmp = 0.0f;
								sscanf(queOut.c_str(),"%f",&ftmp);
								cueout = (int)(ftmp*1000);
							}
							else
							{
								if (_ngodConfig._protocolVersioning.enableVersioning > 0 &&
									ngodVersion == NgodVerCode_R2_DecNpt)
									//strRequire == "com.comcast.ngod.r2.decimal_npts")
								{
									float ftmp = 0.0f;
									sscanf(queOut.c_str(),"%f",&ftmp);
									cueout = (int)(ftmp*1000);
								}
								else
								{
									sscanf(queOut.c_str() ,"%x",&cueout);
								}
							}
							snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&cueOut%d=%d", tStr.c_str(), asset_count, cueout );
						}
					}
					else //if (ZQ::StringOperation::isInt(temp_strs[2])) 
					{
						int cuein = 0;
						if( _ngodConfig._MessageFmt.rtspNptUsage >= 1)//enable rtsp npt usage
						{
							double ftmp = 0.0f;
							sscanf(queIn.c_str(),"%f",&ftmp);
							cuein = (int)(ftmp*1000);
						}
						else
						{
							if (_ngodConfig._protocolVersioning.enableVersioning > 0 &&
								ngodVersion == NgodVerCode_R2_DecNpt)
								//strRequire == "com.comcast.ngod.r2.decimal_npts")
							{
								double ftmp = 0.0f;
								sscanf(queIn.c_str(),"%f",&ftmp);
								cuein = (int)(ftmp*1000);

							}
							else
							{
								sscanf(queIn.c_str() ,"%x",&cuein);
							}
						}
						snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&cueIn%d=%d", tStr.c_str(), asset_count, cuein);
					}
					NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "The request %s"), szBuf);
				}
				// parse a=X-playlist-item: <provider-id> <asset-id>[ <range>][ tricks/[F][R][P]]
				if (temp_strs.size() >= 4) {
					int tPos;
					if (true == ZQ::StringOperation::hasChar(temp_strs[3], '/', tPos)
						&& stricmp(ZQ::StringOperation::getLeftStr(temp_strs[3], "/", true).c_str(), "tricks") == 0)
					{
						const std::string& controlStr = ZQ::StringOperation::getRightStr(temp_strs[3], "/", true);
						if (strstr(controlStr.c_str(), "F") != NULL)
						{
							std::string tStr(szBuf);
							snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&DisableF%d=1", tStr.c_str(), asset_count);
						}
						if (strstr(controlStr.c_str(), "R") != NULL)
						{
							std::string tStr(szBuf);
							snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&DisableR%d=1", tStr.c_str(), asset_count);
						}
						if (strstr(controlStr.c_str(), "P") != NULL)
						{
							std::string tStr(szBuf);
							snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&DisableP%d=1", tStr.c_str(), asset_count);
						}
					}
				}
				assets.push_back(szBuf);
				asset_count++;
			}
			pTemp = strstr(next_content, "a=X-playlist-item:");
		}

		// DO: format the strResourceURI to "rtsp://<server>:<port>/NGOD?asset=<providerId>#<assetId>"
		strResourceURI = originalURI + "NGOD?compatible=Comcast.ngod&";
		int assets_size = assets.size();
		for (int tmp_int = 0; tmp_int < assets_size; tmp_int++)
		{
			strResourceURI += assets[tmp_int];
			if (tmp_int < assets_size - 1)
				strResourceURI += '&';
		}
		
		pReq->setHeader(ORIGINAL_URI, (char*)originalURI.c_str());
		pReq->setHeader(RESOURCE_URI, (char*)strResourceURI.c_str());
		pReq->getProtocol(szBuf, MY_BUFFER_SIZE - 1);
		pReq->setArgument(pReq->getVerb(), strResourceURI.c_str(), szBuf);
	}
	else if (retVerbCode == RTSP_MTHD_RESPONSE)
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Req(%p), CLIENT RESPONSE"), pReq);
		
		//NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*this);
		NGODr2c1::ctxData NewContext;
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_SESSION] = session;
		inoutMap[MAP_KEY_SEQUENCE] = sequence;

		if (true == openContext(session, NewContext, pNewContextPrx, inoutMap))
		{
			std::string url_str;
			url_str = NewContext.resourceURL;
			memset(szBuf, 0, sizeof(szBuf));
			pReq->getStartline(szBuf, sizeof(szBuf) - 1);
			std::vector<std::string> starts;
			ZQ::StringOperation::splitStr(szBuf, " \r\n\t", starts);
			if (starts.size() >= 2 && (strcmp(starts[1].c_str(), "200") == 0 || strcmp(starts[1].c_str(), "454") == 0))
			{
				if (_ngodConfig._MessageFmt.rtspNptUsage <= 0 && _ngodConfig._protocolVersioning.enableVersioning > 0)
				{
					int requireVerCode = atoi(NewContext.prop["RequireR2"].c_str());
					char requireProtocol[64];
					if (0 == requireVerCode)
					{
						sprintf(requireProtocol, "com.comcast.ngod.r2");
					}
					else
					{
						sprintf(requireProtocol, "com.comcast.ngod.r2.decimal_npts");
					}
					pReq->setHeader(NGOD_HEADER_REQUIRE, requireProtocol);
				}
				if (strcmp(starts[1].c_str(), "200") == 0)
				{
					pReq->setArgument(RTSP_MTHD_PING, url_str.c_str(), starts[0].c_str());
				}
				else
				{
					pReq->setArgument(RTSP_MTHD_TEARDOWN, url_str.c_str(), starts[0].c_str());
				}
			}
			pReq->setHeader("NeedResponse", "no");
		}
	}

	NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Req(%p), leave FixupRequest"), pReq);
	return RequestProcessed;
}

RequestProcessResult NGODEnv::doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Req(%p), enter ContentHandler"), pReq);

	RequestHandler::Ptr pRequestHandler = NULL;
	SetupHandler::Ptr pSetupHandler = NULL;
	//SmartRequestHandler smartHandler(pRequestHandler);
	switch(pReq->getVerb())
	{
	case RTSP_MTHD_SET_PARAMETER:
		{
			pRequestHandler = new SetParamHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_SETUP:
		{
			pSetupHandler = new SetupHandler(*this, pSite, pReq);
			pRequestHandler =  pSetupHandler;
		}
		break;
	case RTSP_MTHD_PLAY:
		{
			pRequestHandler = new PlayHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_PAUSE:
		{
			pRequestHandler = new PauseHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_TEARDOWN:
		{
			pRequestHandler = new TeardownHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_GET_PARAMETER:
		{
			pRequestHandler = new GetParamHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_OPTIONS:
		{
			pRequestHandler = new OptionHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_PING:
		{
			pRequestHandler = new PingHandler(*this, pSite, pReq);
		}
		break;
	default:
		{
		}
		break;
	}

	if (NULL == pRequestHandler)
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "request(%p) is not acceptable request"));
		return RequestError;
	}

	try
	{
		::Ice::Long timeUsed = ZQTianShan::now();
		RequestProcessResult ret = pRequestHandler->process();
		timeUsed = ZQTianShan::now() - timeUsed;
		if( pRequestHandler->getReturnType() == RequestHandler::RETURN_ASYNC)
		{
			//do nothing
		}
		else if ( RequestProcessed == ret )
		{
			NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Sess(%s)Seq(%s)[success]process[%s]request, used [%lld]ms"), 
			pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(), pRequestHandler->getRequestType().c_str(), timeUsed);
			if(pRequestHandler)
			{
// 				delete pRequestHandler;
// 				pRequestHandler = NULL;
			}
		}
		else 
		{
			NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "Sess(%s)Seq(%s)[failed]process[%s]request, used [%lld]ms"), 
			pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(), pRequestHandler->getRequestType().c_str(), timeUsed);
			if ( pReq->getVerb() == RTSP_MTHD_SETUP) 
			{
				pSite->destroyClientSession(pRequestHandler->getSession().c_str());
			//	pSetupHandler->updateFailSessions();
			}
			if(pRequestHandler)
			{
// 				delete pRequestHandler;
// 				pRequestHandler = NULL;
			}
		}
	}
	catch (...)
	{
		if ( pReq->getVerb() == RTSP_MTHD_SETUP) 
		{
			//pSetupHandler->updateFailSessions();
		}

		char szBuf[MY_BUFFER_SIZE];
		szBuf[sizeof(szBuf) - 1] = '\0';
		snprintf(szBuf, sizeof(szBuf) - 1, "Req(%p)Sess(%s)Seq(%s)Mtd(%s) caught an unexpect exception.", pReq, pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(), pRequestHandler->getRequestType().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "%s"), szBuf);
		IServerResponse* pResponse = NULL;
		pResponse = pReq->getResponse();
		if (NULL != pResponse)
		{
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
			if (_globalSession != pRequestHandler->getSession())
				pResponse->setHeader(NGOD_HEADER_SESSION, pRequestHandler->getSession().c_str());
			pResponse->setHeader(NGOD_HEADER_SERVER, _serverHeader.c_str());
			pResponse->setHeader(NGOD_HEADER_SERVER, pRequestHandler->getRequestType().c_str());

			std::string notice_str;
			notice_str = NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR " \"" NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING "\" " "event-date=";
			SYSTEMTIME time;
			GetLocalTime(&time);
			char t[50];
			memset(t, 0, 50);
			snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
				time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
			notice_str += t;
			pResponse->setHeader(NGOD_HEADER_NOTICE, notice_str.c_str());
			pResponse->post();
		}
		if(pRequestHandler)
		{
// 			delete pRequestHandler;
// 			pRequestHandler = NULL;
		}
	}
	
	NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "Req(%p), leave ContentHandler"), pReq);
	return RequestDone;
}

RequestProcessResult NGODEnv::doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	IServerResponse* pResponse = pReq->getResponse( );
	if(pResponse)
	{
		pResponse->post();
	}
	return RequestDone;
}

bool NGODEnv::removeContext(const std::string& sessId, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE] = "\0";

	Ice::Identity ident;
	ident.category = SERVANT_TYPE;
	ident.name = sessId;
	if (ident.name.empty())
		ident.name = inoutMap[MAP_KEY_SESSION];

	std::string method, sequence, session;
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];
	session = ident.name;

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "removeContext()"));

	// add by zjm 
	std::string notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, 
		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);

	NGODr2c1::ContextImplPtr pContextForReturnBandwidth = NULL;
	try
	{
		ZQ::common::MutexGuard lk(_contextEvtrLock);
		Ice::ObjectPtr objPtr = _pContextEvtr->remove(ident);
		pContextForReturnBandwidth= NGODr2c1::ContextImplPtr::dynamicCast(objPtr);
		
	}
	catch (const Freeze::DatabaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform remove() session[%s] caught [%s]:%s", ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform remove() session[%s] caught [%s]", ident.name.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}
	catch (...)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform remove() session[%s] caught unknown exception", ident.name.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	_pSessionWatchDog->removeSession(ident);
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "removeContext() record removed, free resources"));
	
	//release resource allocated by resource manager
	{
		NgodResourceManager::StreamerResourcePara para;
		para.requestBW				= (int32)pContextForReturnBandwidth->data.usedBandwidth;
		para.identifier				= pContextForReturnBandwidth->data.sopname;
		para.bNeedImportChannel		= !pContextForReturnBandwidth->data.importChannelName.empty();
		para.method					= "destroy";
		para.cseq					= "0";
		para.sessionId				= ident.name;

		mResManager.releaseResource( para , pContextForReturnBandwidth->data.streamNetId );
	}

// 	{
// 		//return the used bandwidth from session
// 		std::string& strImportChannelName = pContextForReturnBandwidth->data.importChannelName;
// 		std::string& strSop = pContextForReturnBandwidth->data.sopname;
// 		std::string& strNetId = pContextForReturnBandwidth->data.streamNetId;
// 		Ice::Long	UsedBandwidth = pContextForReturnBandwidth->data.usedBandwidth;
// 		ZQ::common::MutexGuard gd(_lockSopMap);
// 
// 		std::map< std::string ,NGOD2::SOPRestriction::SopHolder >& sopMap = _sopConfig._sopRestrict._sopDatas;
// 		std::map< std::string ,NGOD2::SOPRestriction::SopHolder >::iterator itSop = sopMap.find(strSop);
// 		if ( sopMap.end() == itSop ) 
// 		{//no matched sop
// 			snprintf(szBuf, sizeof(szBuf) - 1, "free bandwidth with Sop[%s] netId[%s] UsedBanwidth[%lld] failed because no Sop is found ",
// 					strSop.c_str(),strNetId.c_str(),UsedBandwidth);
// 			NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
// 			//inoutMap[MAP_KEY_LASTERROR] = szBuf;
// 			inoutMap[MAP_KEY_LASTERROR] = notice_str;
// 			return false;		
// 		}
// 		NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;
// 		//search the netID
// 		std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;
// 		std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin();
// 		for( ; itStreamer != streamers.end() ; itStreamer ++  )		
// 		{
// 			if ( strNetId == itStreamer->_netId  ) 
// 			{
// 				//free the bandwidth				
// 				{
// 					itStreamer->_usedBandwidth				-= UsedBandwidth;
// 					itStreamer->_usedStream					--;					
// 					NGODLOG(ZQ::common::Log::L_INFO, 
// 						NGODLOGFMT(NGODEnv, "Free bandwidth ok and Sop[%s] netID[%s] "
// 						"usedBW[%lld] totalBW[%lld] usedStreamCount[%d] totalStreamCount[%d] freedBW[%lld]"),
// 						pContextForReturnBandwidth->data.sopname.c_str(),
// 						pContextForReturnBandwidth->data.streamNetId.c_str(),
// 						itStreamer->_usedBandwidth,
// 						itStreamer->_totalBandwidth,
// 						itStreamer->_usedStream,
// 						itStreamer->_maxStream,
// 						UsedBandwidth );				
// 				}
// 				break;				
// 			}
// 		}//没有成功free 不需要返回错误??
// 		if ( !strImportChannelName.empty() )
// 		{
// 			ZQ::common::MutexGuard gd(_importChannelMapLocker);
// 			
// 			std::map<std::string,NGOD2::PassThruStreaming::ImportChannelHolder>& importChannels = _ngodConfig._passThruStreaming._importChannelDatas;
// 			std::map<std::string,NGOD2::PassThruStreaming::ImportChannelHolder>::iterator it = importChannels.find( strImportChannelName );
// 			if ( it == importChannels.end( ) )
// 			{
// 				NGODLOG(ZQ::common::Log::L_INFO , 
// 					NGODLOGFMT(NGODEnv,"Can't find import Channel with name[%s]"),
// 					strImportChannelName.c_str( ) );
// 				return true;
// 			}
// 			NGOD2::PassThruStreaming::ImportChannelHolder& info = it->second;
// 			info._usedBandwidth			-= UsedBandwidth;
// 			info._usedImport			--;
// 			if ( info._usedBandwidth < 0 )
// 			{
// 				NGODLOG(ZQ::common::Log::L_INFO , 
// 					NGODLOGFMT(NGODEnv,"Adjust usedBW to[0] with released BW[%lld]"
// 					" because it is [%lld] for importChannel[%s]"),
// 					 UsedBandwidth,
// 					 info._usedBandwidth,
// 					 strImportChannelName.c_str());
// 				info._usedBandwidth = 0;
// 			}
// 			if ( info._usedImport <0 )
// 			{
// 				NGODLOG(ZQ::common::Log::L_INFO , 
// 					NGODLOGFMT(NGODEnv,"Adjust usedSessions to[0]"
// 					" because it is [%d] for importChannel[%s]"),
// 					info._usedImport,
// 					strImportChannelName.c_str());
// 				info._usedImport = 0;
// 			}
// 			NGODLOG(ZQ::common::Log::L_INFO , 
// 				NGODLOGFMT(NGODEnv , "release ImportChannel with name[%s] bandwidth[%lld] and "
// 				"now totalBW[%lld] usedBW[%lld] totalSessCount[%d] usedSessionCount[%d]"),
// 				strImportChannelName.c_str() , UsedBandwidth ,
// 				info._maxBandwidth , info._usedBandwidth,
// 				info._maxImport, info._usedImport);
// 		}
// 				
// 	}
	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(NGODEnv, "context removed session[%s] successfully"), ident.name.c_str());
	return true;
}

std::string NGODEnv::getCurrentConnID(NGODr2c1::ctxData& context)
{
	ZQ::common::MutexGuard lk(_connIDGroupsLock);
	
	int size = _connIDGroups.size();
	int i = 0;
	
	for (i = 0; i < size; i++)
	{
		ConnIDGroupPair& _pair = _connIDGroups[i];
		if (_pair._sessionGroup == context.groupID)
		{
			return _pair._connectionID;
		}
	}

	return context.groupID;
}

bool NGODEnv::getStream(::TianShanIce::Streamer::StreamPrx& streamPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string streamid = inoutMap[MAP_KEY_STREAMFULLID];

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "to gain stream: [%s]"), streamid.c_str());

	// add by zjm 
	std::string notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, 
		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);

	try
	{
		streamPrx = ::TianShanIce::Streamer::StreamPrx::uncheckedCast(_pCommunicator->stringToProxy(streamid));
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform uncheckedCast to stream: [%s] caught [%s]", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	if (NULL == streamPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result stream proxy: [%s] is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(NGODEnv, "stream: [%s] gained successfully"), streamid.c_str());

	return true;
}



bool NGODEnv::getStreamState(TianShanIce::Streamer::StreamState& strmState
								 , const TianShanIce::Streamer::StreamPrx& strmPrx
								 , INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string streamid = inoutMap[MAP_KEY_STREAMFULLID];


	std::string sopName ;
	if( inoutMap.find(MAP_KEY_SOPNAME) != inoutMap.end() )
		sopName = inoutMap[MAP_KEY_SOPNAME];
	std::string streamerNetId ;
	if( inoutMap.find(MAP_KEY_STREMAERNETID) != inoutMap.end() )
		streamerNetId = inoutMap[MAP_KEY_STREMAERNETID];

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "to gain stream: [%s] state"), streamid.c_str());

	// add by zjm 
	std::string notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, 
		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);

	if (NULL == strmPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "stream: [%s] proxy is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
	//	inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	try
	{
		strmState=strmPrx->getCurrentState();
	}
	catch( const Ice::TimeoutException&)
	{
		NGODLOG(ZQ::common::Log::L_ERROR,NGODLOGFMT(NGODEnv,"caught [Ice::TimeoutException] when perform getCurrentState"));
		if( ( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR ) && 
			!sopName.empty() &&
			!streamerNetId.empty() )
		{
			addPenaltyToStreamer(sopName,streamerNetId);
		}
		return false;
	}
	catch( const Ice::ConnectionRefusedException& )
	{
		NGODLOG(ZQ::common::Log::L_ERROR,NGODLOGFMT(NGODEnv,"caught [Ice::ConnectionRefusedException] when perform getCurrentState"));
		if( ( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR ) && 
			!sopName.empty() &&
			!streamerNetId.empty() )
		{
			addPenaltyToStreamer(sopName,streamerNetId);
		}
		return false;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform getCurrentState() on stream: [%s] caught %s:%s", streamid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform getCurrentState() on stream: [%s] caught %s", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	switch(strmState)
	{
	case TianShanIce::Streamer::stsSetup:
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "init";
		}
		break;
	case TianShanIce::Streamer::stsStreaming: 
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "play";
		}
		break;
	case TianShanIce::Streamer::stsPause: 
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "pause";
		}
		break;
	case TianShanIce::Streamer::stsStop:
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "ready";
		}
		break;
	default: 
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "unknown";
		}
		break;
	}
	
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "stream: [%s], state: [%s] gained successfully"), streamid.c_str(), inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION].c_str());
	return true;
}

bool NGODEnv::getPositionAndScale(const TianShanIce::Streamer::StreamPrx& strmPrx, INOUTMAP& inoutMap, int ngodVersion)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string sopName ;
	if( inoutMap.find(MAP_KEY_SOPNAME) != inoutMap.end() )
		sopName = inoutMap[MAP_KEY_SOPNAME];
	std::string streamerNetId ;
	if( inoutMap.find(MAP_KEY_STREMAERNETID) != inoutMap.end() )
		streamerNetId = inoutMap[MAP_KEY_STREMAERNETID];

	std::string streamid = inoutMap[MAP_KEY_STREAMFULLID];
	
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(NGODEnv, "to gain stream: [%s] position and scale"), streamid.c_str());

	// add by zjm 
	std::string notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, 
		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);

	if (NULL == strmPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "stream: [%s] proxy is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	TianShanIce::Streamer::PlaylistPrx playlist = NULL;
	try
	{
		playlist = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(strmPrx);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "checkedCast stream: [%s] to playlist caught [%s]:[%s]", streamid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//noutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "checkedCast stream: [%s] to playlist caught [%s]", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}

	if (NULL == playlist)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "checkedCast stream: [%s] to playlist, but result is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}
	
	TianShanIce::ValueMap valMap;
	try
	{
		playlist->getInfo(::TianShanIce::Streamer::infoSTREAMNPTPOS, valMap);		
	}
	catch( const Ice::TimeoutException&)
	{
		NGODLOG(ZQ::common::Log::L_ERROR,NGODLOGFMT(NGODEnv,"caught [Ice::TimeoutException] when perform getinfo"));
		if( ( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR ) && 
				!sopName.empty() &&
				!streamerNetId.empty() )
		{
			addPenaltyToStreamer(sopName,streamerNetId);
		}
		return false;
	}
	catch( const Ice::ConnectionRefusedException& )
	{
		NGODLOG(ZQ::common::Log::L_ERROR,NGODLOGFMT(NGODEnv,"caught [Ice::ConnectionRefusedException] when perform getinfo"));
		if( ( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR ) && 
			!sopName.empty() &&
			!streamerNetId.empty() )
		{
			addPenaltyToStreamer(sopName,streamerNetId);
		}
		return false;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getInfo() on stream: [%s] caught [%s]:[%s]", streamid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getInfo() on stream: [%s] caught [%s]", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(NGODEnv, "%s"), szBuf);
		//inoutMap[MAP_KEY_LASTERROR] = szBuf;
		inoutMap[MAP_KEY_LASTERROR] = notice_str;
		return false;
	}
	
	TianShanIce::ValueMap::iterator it_cp, it_tp, it_scale;
	it_cp = valMap.find("playposition");
	it_tp = valMap.find("totalplaytime");

	//// add by zjm to support session history
	//if (it_tp != valMap.end()  && it_tp->second.ints.size())
	//{
	//	inoutMap["stopNPT"] = it_tp->second.ints[0];
	//}

	it_scale = valMap.find("scale");
	if (it_cp != valMap.end() && it_cp->second.ints.size()
		&& it_tp != valMap.end()  && it_tp->second.ints.size()
		&& it_scale != valMap.end() && it_scale->second.strs.size())
	{
		int cur, total;
		cur = it_cp->second.ints[0];
		total = it_tp->second.ints[0];
		inoutMap[MAP_KEY_STREAMSCALE] = it_scale->second.strs[0];
		if(total != 0)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "%d.%d-%d.%d", cur/1000, cur%1000, total/1000, total%1000);
			inoutMap[MAP_KEY_STREAMPOSITION] = szBuf;

			//only current position is need
			snprintf(szBuf, sizeof(szBuf) - 1, "%x", cur);
			inoutMap[MAP_KEY_STREAMPOSITION_HEX] = szBuf;
		}
		else
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "%d.%d-", cur/1000, cur%1000 );
			inoutMap[MAP_KEY_STREAMPOSITION] = szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "%x", cur );
			inoutMap[MAP_KEY_STREAMPOSITION_HEX] = szBuf;
		}
	}

	TianShanIce::ValueMap::iterator it_index, it_itemOffset;
	it_index		= valMap.find("index");

	// add by zjm to support session history
	if (it_index != valMap.end() && it_index->second.ints.size()) 
	{
		std::stringstream ss;
		ss << it_index->second.ints[0];
		inoutMap["stopIndex"] = ss.str();
	}

	it_itemOffset	= valMap.find("itemOffset");

	// add by zjm to support session history
	if (it_itemOffset != valMap.end() && it_itemOffset->second.ints.size() > 0)
	{
		int iTimeOffset	= it_itemOffset->second.ints[0];
		int main = iTimeOffset / 1000 ;
	    int fraction = iTimeOffset % 1000;
		szBuf[sizeof(szBuf)-1] = 0;
		snprintf( szBuf , sizeof(szBuf)-1 ,"%d.%d", main, fraction );
		inoutMap["SessionAssetNPT"] = szBuf;
	}

	if( it_index != valMap.end() && it_index->second.ints.size() > 0 
		&& it_itemOffset != valMap.end() && it_itemOffset->second.ints.size() > 0 )
	{
		int iIndex,iTimeOffset;
		iIndex		= it_index->second.ints[0];
		iTimeOffset	= it_itemOffset->second.ints[0];
		szBuf[sizeof(szBuf)-1] = 0;
		if(_ngodConfig._MessageFmt.rtspNptUsage >= 1 )
		{
			int main = 0,fraction=0;
			main = iTimeOffset / 1000 ;
			fraction = iTimeOffset % 1000;
			if( fraction != 0)
			{
				snprintf( szBuf , sizeof(szBuf)-1 ,"%d %d.%d",iIndex, main,fraction );
			}
			else
			{
				snprintf( szBuf , sizeof(szBuf)-1 ,"%d %d",iIndex, main );
			}
		}
		else
		{
			if( _ngodConfig._MessageFmt.rtspNptUsage >= 1)
			{
				int main = 0,fraction=0;
				main = iTimeOffset / 1000 ;
				fraction = iTimeOffset % 1000;
				if( fraction != 0)
				{
					snprintf( szBuf , sizeof(szBuf)-1 ,"%d %d.%d",iIndex, main,fraction );
				}
				else
				{
					snprintf( szBuf , sizeof(szBuf)-1 ,"%d %d",iIndex, main );
				}
			}
			else
			{
				if (_ngodConfig._protocolVersioning.enableVersioning > 0 &&
					(ngodVersion == NgodVerCode_C1_DecNpt || ngodVersion == NgodVerCode_R2_DecNpt))
				{
					int main = 0,fraction=0;
					main = iTimeOffset / 1000 ;
					fraction = iTimeOffset % 1000;
					if( fraction != 0)
					{
						snprintf( szBuf , sizeof(szBuf)-1 ,"%d %d.%d",iIndex, main,fraction );
					}
					else
					{
						snprintf( szBuf , sizeof(szBuf)-1 ,"%d %d",iIndex, main );
					}
				}
				else
				{
					snprintf( szBuf , sizeof(szBuf)-1 ,"%d %x",iIndex, iTimeOffset );
				}
			}
		}
		NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(NGODEnv, "getPositionAndScale() : NPT[%s]"), szBuf);
		inoutMap[MAP_KEY_STREAMPOSITION_STOPPOINT] = szBuf;
	}


	
	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(NGODEnv, "stream: [%s]'s position: [%s] and scale: [%s] gained successfully")
		, streamid.c_str(), inoutMap[MAP_KEY_STREAMPOSITION].c_str(), inoutMap[MAP_KEY_STREAMSCALE].c_str());

	return true;
}

bool NGODEnv::sessionInProgressAnnounce(NGODr2c1::ctxData& context)
{

	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "sessionInProgressAnnounce() Session[%s]"), context.ident.name.c_str());
	try
	{
		STRINGMAP inoutMap;
		inoutMap[MAP_KEY_SESSION] = context.ident.name;
		inoutMap[MAP_KEY_METHOD] = "InProgressAnnounce";
		
		std::string sequence;

		if (0 != _ngodConfig._announce._useGlobalCSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", context.announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		std::string currentConnID;
		currentConnID = getCurrentConnID(context);
		
		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm);
		pOdrm = _pSite->newServerRequest(context.ident.name.c_str(), currentConnID);
		if(NULL != pOdrm)
		{
			std::string responseHead = "ANNOUNCE " + context.normalURL + " RTSP/1.0";
			
			std::string notice_str;
			notice_str = NGOD_ANNOUNCE_SESSIONINPROGRESS " \"" NGOD_ANNOUNCE_SESSIONINPROGRESS_STRING "\" " "event-date=";
			SYSTEMTIME time;
			GetLocalTime(&time);
			char t[50];
			memset(t, 0, 50);
			snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
				time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
			notice_str += t;

			// add by zjm to fix up bug 10466
			::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
			std::string pos_str;
			inoutMap[MAP_KEY_STREAMFULLID] = context.streamFullID;
			if (true == getStream(streamPrx, inoutMap))
			{
				NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Scale Changed : The latest C1 reqire is [%s]"), context.prop["RequireC1"].c_str());
				getPositionAndScale(streamPrx, inoutMap, atoi(context.prop["RequireR2"].c_str()));
				if ( atoi( context.prop["RequireR2"].c_str() ) == NgodVerCode_R2_DecNpt)
				{
					pos_str = inoutMap[MAP_KEY_STREAMPOSITION];
					pOdrm->printHeader("Require" , "com.comcast.ngod.r2,com.comcast.ngod.r2.decimal_npts");
				}
				else
				{
					pos_str = inoutMap[MAP_KEY_STREAMPOSITION_HEX];
					pOdrm->printHeader("Require", "com.comcast.ngod.r2");
				}
				size_t nPos = pos_str.find("-");
				if (nPos != std::string::npos)
				{
					pos_str = pos_str.substr(0, nPos);
				}
			}

			notice_str += pos_str;
			
			// TODO: send session timeout announce
			pOdrm->printCmdLine(responseHead.c_str());
			//pOdrm->printHeader("Require", "com.comcast.ngod.r2");
			/*if (atoi(context.prop["RequireC1"].c_str()) == NgodVerCode_C1)
			{
				pOdrm->printHeader("Require" , "com.comcast.ngod.c1");
			}
			else
			{
				pOdrm->printHeader("Require", "com.comcast.ngod.c1.decimal_npts");
			}*/
			pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) context.ident.name.c_str());
			pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) context.onDemandID.c_str());
			pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
			pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _serverHeader.c_str());
			pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
			pOdrm->post();

			NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "[Session In Progress] has been sent out, session: [%s]"),context.ident.name.c_str());
		}
		else 
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "create server request failed, session: [%s][%s]"), context.ident.name.c_str() ,currentConnID.c_str() );
	}
	catch (...)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "process [Session In Progress] caught unexpect exception, session: [%s]"), context.ident.name.c_str());
		return false;
	}

	return true;
}

bool NGODEnv::terminatAndAnnounce(NGODr2c1::ctxData& context)
{

	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "terminatAndAnnounce() Session[%s]"), context.ident.name.c_str());

	try
	{
		STRINGMAP inoutMap;
		inoutMap[MAP_KEY_SESSION] = context.ident.name;
		inoutMap[MAP_KEY_METHOD] = "SessionTerminated";
	
		std::string sequence;

		if (0 != _ngodConfig._announce._useGlobalCSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", context.announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		std::string currentConnID;
		currentConnID = getCurrentConnID(context);
		
//		inoutMap[MAP_KEY_WEIWOOSESSIONFULLID] = pContext->weiwooFullID;
		inoutMap[MAP_KEY_WEIWOOSESSIONDESTROYREASON] = "TerminateAnnounce";
		
		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm);
		pOdrm = _pSite->newServerRequest(context.ident.name.c_str(), currentConnID);
		if(NULL != pOdrm)
		{
			std::string responseHead = "ANNOUNCE " + context.normalURL + " RTSP/1.0";
			
			std::string notice_str;
			notice_str = NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED " \"" NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED_STRING "\" " "event-date=";
			SYSTEMTIME time;
			GetLocalTime(&time);
			char t[50];
			memset(t, 0, 50);
			snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
				time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
			notice_str += t;
			
			inoutMap[MAP_KEY_STREAMFULLID] = context.streamFullID;
			::TianShanIce::Streamer::StreamPrx strmPrx = NULL;
			if (true == getStream(strmPrx, inoutMap))
			{
				if (true == getPositionAndScale(strmPrx, inoutMap, atoi(context.prop["RequireR2"].c_str()) ))
					notice_str += inoutMap[MAP_KEY_STREAMPOSITION];
			}
			
			// TODO: send session timeout announce
			pOdrm->printCmdLine(responseHead.c_str());
			pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) context.ident.name.c_str());
			pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) context.onDemandID.c_str());
			pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
			pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _serverHeader.c_str());
			pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
			pOdrm->post();

			NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODEnv, "[Client Session Terminated] has been sent out , session: [%s]"), context.ident.name.c_str());
		}
		else 
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "create server request failed, session: [%s]"), context.ident.name.c_str());
		
		try
		{
			TianShanIce::Streamer::StreamPrx stream  = TianShanIce::Streamer::StreamPrx::checkedCast(_pCommunicator->stringToProxy(context.streamFullID));
			if (stream) 
			{
				stream->destroy();
			}
		}
		catch (...) 
		{
		}
		
		// DO: remove session context
		//wiped by andy:		removeContextByIdentity(pContext->ident, inoutMap);
		removeContext(context.ident.name, inoutMap);
		
		
		// DO: destroy rtsp client session
		if (true == _pSite->destroyClientSession(context.ident.name.c_str()))
			NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(NGODEnv, "rtspProxy session destroyed, session: [%s]"), context.ident.name.c_str());
	}
	catch (...)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NGODEnv, "process [Client Session Terminated] caught unexpect exception, session: [%s]"), context.ident.name.c_str());
		return false;
	}

	return true;
}
bool NGODEnv::AddBackupLAM(IN const std::string& volumnName, IN const std::string& lamEndpoint ,bool bWarmUp)
{
	LAMFacadePrx	prx = NULL;
	std::string	endPoint = lamEndpoint;
	if ( endPoint.find(":") != std::string::npos ) 
	{//do not have adapter name
		endPoint = "LAMFacade:";
		endPoint +=lamEndpoint;
		NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"adjust lam endpoint from [%s] to [%s]"),
													lamEndpoint.c_str(),endPoint.c_str());
	}
	if ( bWarmUp ) 
	{
		//connect to LAM
		NGODLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODEnv,"connect to LAM:%s"),lamEndpoint.c_str());
		try
		{
			prx =LAMFacadePrx::checkedCast(_pCommunicator->stringToProxy(endPoint));
		}
		catch (Ice::Exception& ex) 
		{
			prx = NULL;
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Catch an Ice exception [%s] when connect to [%s]"),
												ex.ice_name().c_str() , endPoint.c_str());
			return false;
		}
		catch (...) 
		{
			prx = NULL;
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Catch unknown exception when connect to [%s]"),endPoint.c_str());
			return false;
		}
	}
	LAMInfo info;
	info.lamEndpoint	= endPoint ; 
	info.lamPrx			= prx;
	
	if (_lamMap.find(volumnName)!=_lamMap.end()) 
	{
		NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"There is already a LAM with volumnName [%s]"),volumnName.c_str());
		return false;
	}
	else
	{
		_lamMap[volumnName] = info;
	}
	return true;
}

void NGODEnv::addPenaltyToStreamer( const std::string& sopName , const std::string& streamerNetId )
{
	ZQ::common::MutexGuard gd( _lockSopMap );
	ZQ::common::Config::Holder<NGOD2::SOPRestriction>&	sopRes = _sopConfig._sopRestrict;
	std::map<std::string , NGOD2::SOPRestriction::SopHolder>::iterator itSop = sopRes._sopDatas.find( sopName );
	if( itSop != sopRes._sopDatas.end() )
	{
		NGOD2::SOPRestriction::SopHolder& sop = itSop->second;
		std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = sop._streamerDatas.begin();
		for( ; itStreamer != sop._streamerDatas.end() ; itStreamer++ )
		{
			if( streamerNetId == itStreamer->_netId )
			{
				{
					NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"add penalty [%d] to streamer[%s] for sop[%s]"),
						_sopConfig._sopRestrict._maxPenaltyValue,
						itStreamer->_netId.c_str(),
						sopName.c_str() );
					itStreamer->_penalty = _sopConfig._sopRestrict._maxPenaltyValue;
				}
				break;
			}
		}
	}
}

void NGODEnv::dummyGetAeList(OUT AssetElementCollection& aeInfos,OUT long& maxBW)
{
// 	for (int i = 0; i < _ngodConfig._lam._lamTestMode._playlistSize; i ++)
// 	{
// 		AEInfo eleInfo;
// 		eleInfo.aeUID = _ngodConfig._lam._lamTestMode._contentName;
// 		eleInfo.cueIn = 0;
// 		eleInfo.cueOut = 0;
// 		maxBW = (long) 4000 * 1000;
// 		aeInfos.push_back(eleInfo);
// 	}
}
bool NGODEnv::GetAeList(IN const std::string& volumnName ,  
						IN const std::string& strProviderID,
						IN const std::string& strAssetID,
						IN int cueIn , IN int cueOut,
						OUT AssetElementCollection& aeInfos,
						OUT long& maxBW,
						RequestHandler* pRequestHandler )
{
	if (_ngodConfig._lam._lamTestMode._enabled != 0)
	{
		dummyGetAeList(aeInfos,maxBW);
		return true;
	}
	LAMInfo info;
	{
		ZQ::common::MutexGuard gd(_lockLammap);
		if (_lamMap.find(volumnName) == _lamMap.end()) 
		{
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Can't find lam through volumnName[%s]"),volumnName.c_str());
			return false;
		}
		info = _lamMap[volumnName];
	}
	if ( info.lamPrx == NULL ) 
	{
		//connect to LAM
		NGODLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODEnv,"LAM [%s][%s] is not connected,connect it now"),
					volumnName.c_str(),info.lamEndpoint.c_str());
		try
		{
			info.lamPrx =LAMFacadePrx::checkedCast(_pCommunicator->stringToProxy(info.lamEndpoint));
			NGODLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODEnv,"connect to LAM [%s][%s] OK"),
					volumnName.c_str(),info.lamEndpoint.c_str());
		}
		catch (Ice::Exception& ex) 
		{
			info.lamPrx = NULL;
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Catch an Ice exception [%s] when connect to [%s]"),
												ex.ice_name().c_str() , info.lamEndpoint.c_str());
			return false;
		}
		catch (...) 
		{
			info.lamPrx = NULL;
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Catch unknown exception when connect to [%s]"),info.lamEndpoint.c_str());
			return false;
		}
		{
			ZQ::common::MutexGuard gd(_lockLammap);
			_lamMap[volumnName] = info;
		}
	}

	//Get ae list from LAM
	{
		NGODLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) Get aeList with ProviderID [%s] assetId[%s] volumnName[s]"),
													pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
													strProviderID.c_str(),strAssetID.c_str(),volumnName.c_str());
		AssetElementCollection aCl;
		DWORD dwGetAeListTime = GetTickCount();
		try
		{
			aCl = info.lamPrx->getAEListByProviderIdAndPAssetId(strProviderID,strAssetID);
		}
		catch (Ice::Exception& ex) 
		{
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) Catch an Ice exception [%s] when get aelist from [%s] with providerID[%s] assetId[%s]"),
									pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
									ex.ice_name().c_str(),volumnName.c_str(),strProviderID.c_str(),strAssetID.c_str());
			return false;
		}
		catch (...) 
		{
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) Catch an unknow exception when get aelist from [%s] with providerID[%s] assetId[%s]"),
								pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
								volumnName.c_str(),strProviderID.c_str(),strAssetID.c_str());
			return false;
		}
		if (aCl.size() == 0) 
		{
			NGODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) No element is found through providerID[%s] AssetID[%s] volumnName[%s]"),
								pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
								strProviderID.c_str(),strAssetID.c_str(),volumnName.c_str());
			return false;
		}
		
		NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) get [%d] elements from LAM and use time [%d]"),
							pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
							aCl.size(),GetTickCount()-dwGetAeListTime);

		int tempCuein =0;
		int tempCueout =0;
		maxBW = -1000;

		//adjust cue in and cue out
		for(int i=0;i<(int)aCl.size();i++)
		{				
			if(aCl[i].cueIn <= 0 && cueIn <=0 )
			{
				NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) item [%s] no cuein value,leave it as 0"),
													pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
													aCl[i].aeUID.c_str());
			}
			else
			{
				tempCuein = cueIn > aCl[i].cueIn ? cueIn : aCl[i].cueIn;
				NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) Adjust item [%s] cuein to [%d] ,cuein from URI is [%d] and cuein from LAM is [%d]"),
											pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
											aCl[i].aeUID.c_str(), tempCuein,cueIn,aCl[i].cueIn);
				aCl[i].cueIn = tempCuein;
			}
			
			if(aCl[i].cueOut <= 0 && cueOut <=0 )
			{
				NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) item [%s] no cueout value,leave it as 0"),
												pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
												aCl[i].aeUID.c_str());
			}
			else
			{
				tempCueout =  0;
				if ( cueOut <= 0 ) 
				{
					tempCueout = aCl[i].cueOut;
				}
				else if ( aCl[i].cueOut<=0 ) 
				{
					tempCueout = cueOut;
				}
				else
				{
					tempCueout = cueOut < aCl[i].cueOut ? cueOut : aCl[i].cueOut;
				}
				
				NGODLOG(ZQ::common::Log::L_INFO,CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) Adjust item [%s] cueout to [%d] ,cueout from URI is [%d] and cueout from LAM is [%d]"),
								pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
								aCl[i].aeUID.c_str(),tempCueout,cueOut,aCl[i].cueOut);
				aCl[i].cueOut = tempCueout;
			}	
			if (maxBW < aCl[i].bandWidth) 
			{
				maxBW = aCl[i].bandWidth;
			}
			aeInfos.push_back(aCl[i]);				
		}
		NGODLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(NGODEnv,"Sess(%s) Seq(%s) maxBW to [%d] with ProviderID[%s] assetID[%s] volumnName[%s], itemCount[%d]"),
					pRequestHandler->getSession().c_str(), pRequestHandler->getSequence().c_str(),
					maxBW, strProviderID.c_str(), strAssetID.c_str(), volumnName.c_str(), aeInfos.size());

	}
	return true;
}

void	NGODEnv::resetCounters( )
{
	extern NGODEnv ssmNGOD;
	ZQ::common::MutexGuard gd(ssmNGOD._lockSopMap);
	/*sopRestriction = _ngodConfig._sopRestrict._sopDatas;*/
	std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sopDatas = _sopConfig._sopRestrict._sopDatas;
	std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator it = sopDatas.begin();
	for( ; it != sopDatas.end() ; it ++ )
	{
		NGOD2::SOPRestriction::SopHolder& sopHolder = it->second;
		std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopHolder._streamerDatas;
		std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin();
		for( ; itStreamer != streamers.end() ; itStreamer++)
		{
			itStreamer->_setupCountersForRemote =	0;
			itStreamer->_setupCountersTotal		=	0;
			itStreamer->_failedSession = 0;
			itStreamer->_usedSession = 0;
		}
	}
	counterMeasuredSince =ZQTianShan::now();
}

Ice::Long	NGODEnv::counterMeasuredSince = 0;
void	NGODEnv::getNgodUsage( std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sopRestriction , std::string& strMesureSince )
{
	extern NGODEnv ssmNGOD;
	ZQ::common::MutexGuard gd(ssmNGOD._lockSopMap);
	sopRestriction = _sopConfig._sopRestrict._sopDatas;
	char szBuf[1024] ={0};
	const char* pTime = ZQTianShan::TimeToUTC( counterMeasuredSince , szBuf, sizeof(szBuf) -1 , true );
	strMesureSince = pTime ? pTime : "";
}

void NGODEnv::getImportChannelUsage( NGODr2c1::ImportChannelUsageS& usages )
{
	extern NGODEnv ssmNGOD;
	usages.clear();

	ZQ::common::MutexGuard gd(ssmNGOD._lockSopMap);
	if( _ngodConfig._passThruStreaming._muteStat >= 1 )
	{
		ssmNGOD._fileLog(ZQ::common::Log::L_WARNING,CLOGFMT(NGODEnv, "do not return any ImportChannel record due to muteStat[%d]"),
			_ngodConfig._passThruStreaming._muteStat );
		return;
	}
	
	std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder>& ics = _ngodConfig._passThruStreaming._importChannelDatas;
	std::map< std::string , NGOD2::PassThruStreaming::ImportChannelHolder>::const_iterator it = ics.begin();
	for( ; it != ics.end() ; it ++ )
	{	
		if( it->second._bConfiged )
		{
			NGODr2c1::ImportChannelUsage	usage;
			const NGOD2::PassThruStreaming::ImportChannelHolder& ic = it->second;
			usage.channelName				= it->first;
			usage.runningSessCount			= ic._runningImportSessCount;
			//use reported totalbandwidth instead of configured bandwidth
			usage.totalImportBandwidth		= ic._reportTotalBandwidth;//ic._reportTotalBandwidth >0 ? ic._reportTotalBandwidth : ic._maxBandwidth;
			usage.usedImportBandwidth		= ic._reportUsedBandwidth;

			usages.push_back(usage);
		}
	}
}

void NGODEnv::getCoreInfo( int32 type , ZQ::common::Variant& var)
{
	var = _pSite->getInfo(type);
}

SmartServerRequest::SmartServerRequest(IServerRequest*& pServerRequest) : _pServerRequest(pServerRequest)
{
}

SmartServerRequest::~SmartServerRequest()
{
	if (NULL != _pServerRequest)
		_pServerRequest->release();
	_pServerRequest = NULL;
}


