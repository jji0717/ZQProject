#include <boost/thread.hpp>
#include "CdnStreamerManager.h"
#include "CdnEnv.h"
#include "../CDNDefines.h"
#include "CdnSSConfig.h"
#include <TianShanIceHelper.h>
#include <C2StreamerLib.h>
#include <C2SessionHelper.h>
#include <CacheStoreImpl.h>
#include "IndexFileParser.h"
#include <ContentSysMD.h>
#include "CDNHttpBridge.h"
#include <urlstr.h>
#include <SystemInfo.h>
#include <strHelper.h>

#ifndef ZQ_CDN_UMG
#include <C2StreamerService.h>
#include <C2HttpHandler.h>
#include "C2Streamer/C2HttpLibAsyncBridge.h"


C2Streamer::C2StreamerEnv* 	gC2Env = NULL;
C2Streamer::C2Service*		gC2Svc = NULL;

extern ZQTianShan::ContentStore::CacheStoreImpl::Ptr cacheStore;

namespace C2Streamer
{
	C2StreamerEnv*	getEnvironment( )
	{
		return gC2Env;
	}
	C2Service* getC2StreamerService()
	{
		return gC2Svc;
	}
}
#else
#include "HttpC2Streamer.h"

#endif
namespace ZQ
{
namespace StreamService
{

CdnStreamerManager::CdnStreamerManager( CdnSsEnvironment* environment, SsServiceImpl& serviceImpl )
:env(environment),
ss(serviceImpl),
mbStarted(false)
#ifndef ZQ_CDN_UMG
,
mC2Service(NULL),
mC2HttpEngine(NULL)
#endif
{
	mbQuit = false;
}

CdnStreamerManager::~CdnStreamerManager( )
{

}

bool CdnStreamerManager::listStreamer( SsReplicaInfoS& infos ) const
{
	infos.clear();
	ZQ::common::MutexGuard gd(mStreamerLocker);
	StreamerAttrs::const_iterator it = mStreamers.begin();
	for( ; it != mStreamers.end() ; it ++ )
	{
		SsReplicaInfo info;

		info.bHasPorts		= false;
		info.ports.clear();
		info.replicaId		=	it->second.portName;
		info.replicaState	=	it->second.bUp ? TianShanIce::stInService : TianShanIce::stOutOfService;
		info.streamerType	=	"CdnStreamer";
		info.groupId		=	gCdnSSConfig.netId;

		ZQTianShan::Util::updatePropertyData(info.props, STREAMERPROP_CAPACITY , it->second.capacity );
		ZQTianShan::Util::updatePropertyData(info.props, STREAMERPROP_ACTIVETRANSFERCOUNT , it->second.activeBandwidth);
		ZQTianShan::Util::updatePropertyData(info.props, STREAMERPROP_ACTIVEBANDWIDTH , it->second.activeBandwidth );
		ZQTianShan::Util::updatePropertyData(info.props, STREAMERPROP_ADDRESSTCPPORT , it->second.transferTcpPort );
		ZQTianShan::Util::updatePropertyData(info.props, STREAMERPROP_VOLUMENETID , gCdnSSConfig.csStrReplicaGroupId );

		ZQTianShan::Util::updatePropertyData(info.props, STREAMERPROP_ADDRESSIPV4,
			ZQTianShan::Util::dumpTianShanIceStrValues(it->second.transferAddressIpv4) );
		ZQTianShan::Util::updatePropertyData(info.props, STREAMERPROP_ADDRESSIPV6,
			ZQTianShan::Util::dumpTianShanIceStrValues(it->second.transferAddressIpv6) );

		infos.insert( SsReplicaInfoS::value_type( info.replicaId, info ) );
		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(CdnStreamerManager,"listStreamer [%s] [%s/%s] capacity[%ld] found"),
			info.replicaId.c_str() ,
			ZQTianShan::Util::dumpTianShanIceStrValues(it->second.transferAddressIpv4).c_str() ,
			ZQTianShan::Util::dumpTianShanIceStrValues(it->second.transferAddressIpv6).c_str() ,
			it->second.capacity);

	}
	return (infos.size() > 0) ;
}

bool vectorChanged( const std::vector<std::string>& A, const std::vector<std::string>& B )
{
	if( A.size() != B.size() )
		return true;
	if(!std::equal( A.begin(),A.end(),B.begin() ))
		return true;

	return false;
}

bool StreamerAttrChanged( const CdnStreamerManager::StreamerAttr& attrA, const CdnStreamerManager::StreamerAttr& attrB)
{
	if( attrA.bUp != attrB.bUp )
		return true;

	if( attrA.capacity != attrB.capacity)
		return true;
	if( vectorChanged(attrA.transferAddressIpv4,attrB.transferAddressIpv4) )
	{
		return true;
	}
	if( vectorChanged(attrA.transferAddressIpv6,attrB.transferAddressIpv6) )
	{
		return true;
	}

	return false;
}
void CdnStreamerManager::reportStreamerState( const StreamerAttr& attr )
{
	bool bShouldReport = true;
	{
		ZQ::common::MutexGuard gd(mStreamerLocker);
		StreamerAttrs::iterator it = mStreamers.find(attr.portName);
		if( it != mStreamers.end() )
		{
			bShouldReport = StreamerAttrChanged(attr,it->second);
			it->second = attr;
		}
		else
		{
			bShouldReport = true;
			mStreamers.insert(StreamerAttrs::value_type( attr.portName , attr ) );

		}
		if(!bShouldReport)
		{
			return;
		}
	}

	ENVLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(CdnStreamerManager,"Streamer attribute changed name[%s] address[%s/%s] capacity[%ld] state[%s]	activeTransferCount[%d] activeBandwidth[%ld]"),
		attr.portName.c_str(),
		ZQTianShan::Util::dumpTianShanIceStrValues( attr.transferAddressIpv4 ).c_str(),
		ZQTianShan::Util::dumpTianShanIceStrValues( attr.transferAddressIpv6 ).c_str(),
		attr.capacity ,
		attr.bUp ?"UP":"DOWN",
		attr.activeTransferCount,
		attr.activeBandwidth );
	TianShanIce::Properties props;
	ZQTianShan::Util::updatePropertyData( props , STREAMERPROP_CAPACITY , attr.capacity );
	ZQTianShan::Util::updatePropertyData( props , STREAMERPROP_ACTIVETRANSFERCOUNT , attr.activeBandwidth);
	ZQTianShan::Util::updatePropertyData( props , STREAMERPROP_ACTIVEBANDWIDTH , attr.activeBandwidth );
	ZQTianShan::Util::updatePropertyData( props , STREAMERPROP_ADDRESSTCPPORT , attr.transferTcpPort );

	SsServiceImpl::SsReplicaEvent event = attr.bUp ? SsServiceImpl::sreInService : SsServiceImpl::sreOutOfService;

	bShouldReport = bShouldReport && mbStarted;

	if( bShouldReport )
	{
		ss.OnReplicaEvent( event ,attr.portName , props);
	}
}
#ifndef ZQ_CDN_UMG
void cleanupC2Resource( C2Streamer::C2StreamerEnv& env)
{
	if( env.mThreadPool )
	{
		delete env.mThreadPool;
		env.mThreadPool = NULL;
	}
	if( env.mAttrBridge )
	{
		delete env.mAttrBridge;
		env.mAttrBridge = NULL;
	}
}
#endif
void CdnStreamerManager::shutdown( )
{
	//mDelayDeleteQueue.stop();
	// Actually , we may encounter a problem here
	// If we stop the delay queue here , but in fact the deleting thread is running , we may encounter some access violation issues
	// so can we just stop thread pool before this method is invoked?

	mbQuit = true;
	mCond.signal();
	waitHandle(10000);
#ifndef ZQ_CDN_UMG

	if( mC2HttpEngine )
	{
		mC2HttpEngine->stop();
	}

	if(mLibAsyncHttpServer)
	{
		mLibAsyncHttpServer->stop();
		mLibAsyncHttpServer = NULL;
		LibAsync::HttpProcessor::teardown();
	}

	if( mC2Service )
	{
		mC2Service->stopService();
	}
	if( mC2Service)
	{
		delete mC2Service;
		mC2Service = NULL;
	}

	if( mC2HttpEngine )
	{
		delete mC2HttpEngine;
		mC2HttpEngine = NULL;
	}

	cleanupC2Resource(mC2Env);
#else
    C2Streamer::getEnvironment()->stop();
#endif
}

int CdnStreamerManager::run( )
{
	int32 scanInterval = gCdnSSConfig.resourceScanInterval;
	scanInterval = scanInterval < 5000 ? 5000 : scanInterval;
	scanInterval = scanInterval > (10000 * 1000 )? (10000 * 1000 ) : scanInterval;
	std::string upstreamIp = gCdnSSConfig.cacheStoreConfig.upStreamBindip;

	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"start stream resource status scanning with scanInterval[%d]") , scanInterval);
	while ( !mbQuit )
	{
		{
			ZQ::common::MutexGuard gd(mMutex);
			mCond.wait(mMutex, scanInterval );
		}

		if( mbQuit) break;

		queryStreamersInfo();
		if( cacheStore )
		{
			C2Streamer::C2ServiceLoad load;
			if(!C2Streamer::getC2Load(upstreamIp, load))
			{
				ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CdnStreamerManager,"faile to query c2service load with upstream IP %s"),
						upstreamIp.c_str());
			}
			else
			{
				cacheStore->updateStreamLoad( load.localStreamBW/1000, load.localStreamBWMax/1000,
											load.natStreamBW/1000, load.natStreamBWMax/1000	);

				ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"update C2Service load: local streaming [%ld/%ld] pass through streaming [%ld/%ld], upStreaming ip [%s]"),
					load.localStreamBW, load.localStreamBWMax,
					load.natStreamBW, load.natStreamBWMax,
					upstreamIp.c_str() );
			}
		}
	}
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"stop stream resource status scanning"));
	return -1;
}
#ifndef ZQ_CDN_UMG
#if 0
bool CdnStreamerManager::setC2Conf(C2Streamer::C2StreamerEnv& c2env )
{
	C2Streamer::C2EnvConfig& conf = c2env.mConfig;

	C2Streamer::PORTSPEEDCONF& portSpeedMap	= conf.portSpeed;
	const std::vector<C2StreamerSvcConfig::PORTCONF>& portSpeedConf = gCdnSSConfig.c2StreamerConfig.portConf;
	std::vector<C2StreamerSvcConfig::PORTCONF>::const_iterator itPortSpeed = portSpeedConf.begin();
	for( ; itPortSpeed != portSpeedConf.end() ; itPortSpeed ++ )
	{
		if(! itPortSpeed->portName.empty())
		{
			C2Streamer::SERVEPORTATTR attr;
			attr.speed = itPortSpeed->speed;
			attr.natPortBase = itPortSpeed->natPortBase;
			attr.natPortCount = itPortSpeed->natPortCount;
			portSpeedMap[ itPortSpeed->portName ] = attr;
			(getSsEnv()->getMainLogger())(ZQ::common::Log::L_INFO,CLOGFMT(setC2Conf,"SET port[%s]'s speed to [%ld] nat[%d/%d] "),
										   itPortSpeed->portName.c_str() ,
										   attr.speed, attr.natPortBase, attr.natPortCount );
		}
	}

	///get mount doc path
	EdgeFEConfig::Volumes::const_iterator itDocPath = gCdnSSConfig.volumes.begin();
	bool bFoundDocPath =false;
	for( ; itDocPath != gCdnSSConfig.volumes.end() ; itDocPath ++)
	{
		if( itDocPath->isDefault )
		{
			c2env.mDocRootFolder  = itDocPath->path;
			env->mC2StreamerDocRoot = c2env.mDocRootFolder;
			bFoundDocPath = true;
			(getSsEnv()->getMainLogger())(ZQ::common::Log::L_INFO,CLOGFMT(setC2Conf,"set default doc path to [%s]"),
										   c2env.mDocRootFolder.c_str());
			break;
		}
	}
	if(!bFoundDocPath)
	{
		(getSsEnv()->getMainLogger())(ZQ::common::Log::L_ERROR, CLOGFMT(setC2Conf,"no doc path is found in configuration"));
		return false;
	}

	conf.readBufferCount	= gCdnSSConfig.c2StreamerConfig.readBufferCount;
	conf.readBufferCount	= MAX( conf.readBufferCount, 8 );
	conf.ioBlockSize 		= 4 * 1024;
	conf.minIdleInterval 	= 200;

	conf.mSocketKernelSndBuf = gCdnSSConfig.c2StreamerConfig.perfTune.socketSndBuf;
	if(conf.mSocketKernelSndBuf < 32 * 1024)
		conf.mSocketKernelSndBuf = 32 * 1024;
	conf.mSocketKernelSndTimeo = gCdnSSConfig.c2StreamerConfig.perfTune.socketSndTimeo;
	if( conf.mSocketKernelSndTimeo < 100 )
		conf.mSocketKernelSndTimeo = 100;

	conf.mFirstChunkWait = gCdnSSConfig.c2StreamerConfig.perfTune.firstChunkWait;
	if(conf.mFirstChunkWait < 5)
		conf.mFirstChunkWait = 5;
	conf.mbUseTcpCork = gCdnSSConfig.c2StreamerConfig.perfTune.useTcpCork >= 1;
	conf.mPendingSizeThreshold = gCdnSSConfig.c2StreamerConfig.perfTune.threadpoolwarningThreshold;
	conf.mSocketSendMonitorThreashold = gCdnSSConfig.c2StreamerConfig.perfTune.socketWriteMonitorThreshold;
	conf.mMaxIdleCountAtFileEnd = gCdnSSConfig.c2StreamerConfig.sessConf.maxIdleCountAtEnd;
	conf.mDefaultTransferBitrate = gCdnSSConfig.c2StreamerConfig.sessConf.defaultBitrate;
	conf.mFsReadThreshold	= gCdnSSConfig.c2StreamerConfig.perfTune.fsReadThreshold;
	conf.mbQuitPerConnectionLost = gCdnSSConfig.c2StreamerConfig.sessConf.quitPerConnectionLost >= 1;
	conf.mTimerThreadHold	= gCdnSSConfig.c2StreamerConfig.perfTune.timerMonitorThreshold;
	conf.mSendPacketSize	= gCdnSSConfig.c2StreamerConfig.perfTune.socketPacketSize;
	conf.mPacketsPerSend	= gCdnSSConfig.c2StreamerConfig.perfTune.packetsPerSend;
	conf.mLocalBindPort		= gCdnSSConfig.c2StreamerConfig.httpBindPort;
	if( conf.mPacketsPerSend < 1 )
		conf.mPacketsPerSend = 1;

	conf.maxRoundPerRun		= gCdnSSConfig.c2StreamerConfig.perfTune.maxRoundPerRun;
	if( conf.maxRoundPerRun < 1 )
		conf.maxRoundPerRun = 1;

	//HLS part
	conf.mKeyFile5I			= gCdnSSConfig.hlsServerConfig.keyfile;
	conf.mAquaRootUrl		= gCdnSSConfig.hlsServerConfig.rooturl;
	conf.mHomeContainer		= gCdnSSConfig.hlsServerConfig.homecontainer;
	conf.mServerHostUrl		= gCdnSSConfig.hlsServerConfig.serverHostUrl;
	conf.mLogFlag			= gCdnSSConfig.hlsServerConfig.logFlag;
	conf.mName2Bitrate		= gCdnSSConfig.hlsServerConfig.subname2bitrate;
	conf.mDefaultBitrate	= gCdnSSConfig.hlsServerConfig.defaultBitrate;

	//Cache part
	conf.mCacheDefaultBufferCount = gCdnSSConfig.c2StreamerConfig.perfTune.cacheBufferCount;
	conf.mCacheDefaultBufferCount = MAX(conf.mCacheDefaultBufferCount,10);
	conf.mCacheBufferSize = gCdnSSConfig.c2StreamerConfig.perfTune.cacheBufferSize;
	conf.mCacheReadaheadCount = gCdnSSConfig.c2StreamerConfig.perfTune.cacheReadAheadCount;
	conf.mUseBufferIo  = gCdnSSConfig.c2StreamerConfig.perfTune.useBufferIo != 0;
	conf.mIndexRecordCacheSize = gCdnSSConfig.c2StreamerConfig.perfTune.indexRecordCacheSize;
	if( conf.mIndexRecordCacheSize < 1)
		conf.mIndexRecordCacheSize = 1;
	conf.attributesTimeoutInPwe = gCdnSSConfig.c2StreamerConfig.perfTune.attrTimeout;
	conf.mBitrateInflationPercent = gCdnSSConfig.c2StreamerConfig.perfTune.bitrateInflation;

	//AquaReader part
	conf.aquaReaderRootUrl = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.rootUrl;
	conf.aquaReaderUserDomain = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.userDomain;
	conf.aquaReaderHomeContainer = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.homeContainer;
	conf.aquaReaderFlags = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.cdmiFlags;
	conf.aquaReaderIoThreadPoolSize = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.ioThreadPool;
	conf.aquaReaderCdmiOpsReadThreadPoolSize = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.cdmiOpsThreadPool;
	conf.aquaReaderCacheBlockSize = gCdnSSConfig.c2StreamerConfig.perfTune.cacheBufferSize;
	conf.aquaReaderCacheBlockCount = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.cacheBufferCount;
	conf.aquaReaderHlsEnableMDataQuery = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.aquaReaderHlsEnableMDataQuery;



	(getSsEnv()->getMainLogger())(ZQ::common::Log::L_INFO,CLOGFMT(setC2Conf,"set index record cache size: %d"),conf.mIndexRecordCacheSize);

	conf.clientType = gCdnSSConfig.c2StreamerConfig.clientType;
	if (conf.clientType == 1)
	{
		  (getSsEnv()->getMainLogger())(ZQ::common::Log::L_DEBUG,CLOGFMT(setC2Conf,"the c2streamer will be c2client"));
		  conf.C2ClientUpStreamIP 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientUpStreamIP;
		  conf.C2ClientURL 				  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientURL;
		  conf.C2ClientTransfer 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientTransfer;
		  conf.C2ClientHttpCRGAddr 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientHttpCRGAddr;
		  conf.C2ClientHttpCRGPort 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientHttpCRGPort;
		  conf.C2ClientTransferRate 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientTransferRate;
		  conf.C2ClientIngressCapacity 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientIngressCapacity;
		  conf.C2ClientExclusionList 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientExclusionList;
		  conf.C2ClientTransferDelay 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientTransferDelay;
		  conf.C2ClientDefaultGetPort 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientDefaultGetPort;
		  conf.C2ClientWaitBufferTime 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientWaitBufferTime;
		  conf.C2ClientIndexTimeout 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientIndexTimeout;
		  conf.C2ClientIndexRetryTimes 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientIndexRetryTimes;
		  conf.C2ClientMainfileTimeout 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientMainfileTimeout;
		  conf.C2ClientMainfileRetryTimes = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientMainfileRetryTimes;
	}
	else if (conf.clientType == 2  || conf.clientType == 4 )
	{
		  (getSsEnv()->getMainLogger())(ZQ::common::Log::L_DEBUG,CLOGFMT(setC2Conf,"the c2streamer will be httpclient"));
		  conf.httpProxyURL = gCdnSSConfig.c2StreamerConfig.httpclient.httpProxyURL;
		  conf.segmentURL = gCdnSSConfig.c2StreamerConfig.httpclient.segmentURL;
		  conf.httpTimeOut = gCdnSSConfig.c2StreamerConfig.httpclient.httpTimeOut;
		  conf.httpRetry = gCdnSSConfig.c2StreamerConfig.httpclient.httpRetry;
	}
	//disk cache 
	conf.diskCacheEnabled = false;
	if( 0 != gCdnSSConfig.c2StreamerConfig.diskcache.diskCacheEnabled )
	{	
		conf.diskCacheEnabled = true;
		conf.cacheLoaderTimeout = gCdnSSConfig.c2StreamerConfig.diskcache.cacheLoaderTimeout;
		conf.cacheIgnoreFiles = gCdnSSConfig.c2StreamerConfig.diskcache.cacheIgnoreFiles;
		C2Streamer::DISKCACHECON& dcc = conf.diskCacheConfig;
		const std::vector<DiskCache::CACHEDIR>& cacheDir = gCdnSSConfig.c2StreamerConfig.diskcache.cacheDir;
		std::vector<DiskCache::CACHEDIR>::const_iterator iterDiskCache = cacheDir.begin();
		int32 totalBufferCount = conf.mCacheDefaultBufferCount;
		for( ; iterDiskCache != cacheDir.end() ; iterDiskCache ++ )
		{
			if( ! iterDiskCache->homePath.empty() )
			{
				C2Streamer::DiskCacheCon  diskConfg;
				diskConfg.homePath = iterDiskCache->homePath;
				diskConfg.totalSize = iterDiskCache->totalSize;
				diskConfg.LRUSize = iterDiskCache->LRUSize;
				diskConfg.LRUSize = MAX(diskConfg.LRUSize,20);
				totalBufferCount += diskConfg.LRUSize;
				diskConfg.readThreadCount = iterDiskCache->readThreadCount;
				diskConfg.writeThreadCount = iterDiskCache->writeThreadCount;
				diskConfg.pendingsYield = iterDiskCache->pendingsYield;
				dcc.push_back(diskConfg);
			}

		}
		conf.mCacheDefaultBufferCount = totalBufferCount;
	}

	conf.defaultTimeOut = gCdnSSConfig.c2StreamerConfig.c2bufrm.defaultTimeOut;
	conf.fioErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2bufrm.fioErrTimeOut;
	conf.httpErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2bufrm.httpErrTimeOut;
	conf.socketErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2bufrm.socketErrTimeOut;
	conf.otherErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2bufrm.otherErrTimeOut;

	int32 serviceThreadCount = gCdnSSConfig.c2StreamerConfig.serviceThreadCount;
	serviceThreadCount = MAX( serviceThreadCount, 3 );
	int32 locateThreadCount = gCdnSSConfig.c2StreamerConfig.locateThreadCount;
	locateThreadCount = MAX( locateThreadCount, 3 );

	//ZQ::common::NativeThreadPool *pool = new ZQ::common::NativeThreadPool( serviceThreadCount );
	ZQ::common::NativeThreadPool *pool = new  C2ThreadPool( serviceThreadCount );
	ZQ::common::NativeThreadPool *locatePool = new ZQ::common::NativeThreadPool( locateThreadCount );
	c2env.mThreadPool			= pool;
	c2env.mLocateThreadPool		= locatePool;
	c2env.mLogger				= &getSsEnv()->getMainLogger();

	ContentFileAttributeBridge* pAttrBridge = new ContentFileAttributeBridge(env);
	c2env.mAttrBridge		= pAttrBridge;

	return true;
}
#endif //if 0

int32 readerTagToType( const std::string& readerTag ) {
	return (int32)C2Streamer::readerStr2Type(readerTag);
}

bool CdnStreamerManager::setC2Conf( C2Streamer::C2StreamerEnv& c2env ) {
	C2Streamer::C2EnvConfig& conf = c2env.mConfig;

	C2Streamer::NicsConf& portSpeedMap	= conf.nics.nics;
	const std::vector<C2StreamerSvcConfig::PORTCONF>& portSpeedConf = gCdnSSConfig.c2StreamerConfig.portConf;
	std::vector<C2StreamerSvcConfig::PORTCONF>::const_iterator itPortSpeed = portSpeedConf.begin();
	for( ; itPortSpeed != portSpeedConf.end() ; itPortSpeed ++ )
	{
		if(! itPortSpeed->portName.empty())
		{
			C2Streamer::NicAttr attr;
			attr.speed = itPortSpeed->speed;
			attr.natPortBase = itPortSpeed->natPortBase;
			attr.natPortCount = itPortSpeed->natPortCount;
			portSpeedMap[ itPortSpeed->portName ] = attr;
			(getSsEnv()->getMainLogger())(ZQ::common::Log::L_INFO,CLOGFMT(setC2Conf,"SET port[%s]'s speed to [%ld] nat[%d/%d] "),
										   itPortSpeed->portName.c_str() ,
										   attr.speed, attr.natPortBase, attr.natPortCount );
		}
	}

	///get mount doc path
	CdnSSConfig::Volumes::const_iterator itDocPath = gCdnSSConfig.volumes.begin();
	bool bFoundDocPath =false;
	for( ; itDocPath != gCdnSSConfig.volumes.end() ; itDocPath ++)
	{
		if( itDocPath->isDefault )
		{
			c2env.mDocRootFolder  = itDocPath->path;
			env->mC2StreamerDocRoot = c2env.mDocRootFolder;
			bFoundDocPath = true;
			(getSsEnv()->getMainLogger())(ZQ::common::Log::L_INFO,CLOGFMT(setC2Conf,"set default doc path to [%s]"),
										   c2env.mDocRootFolder.c_str());
			break;
		}
	}
	if(!bFoundDocPath)
	{
		(getSsEnv()->getMainLogger())(ZQ::common::Log::L_ERROR, CLOGFMT(setC2Conf,"no doc path is found in configuration"));
		return false;
	}

	//writers part
	//read url rule and establish a connection between urlrule and writer
	C2Streamer::URL2CONFS& c2streamerUrlRules = conf.urlRules;
	std::vector<C2StreamerSvcConfig::C2StreamerUrlRuleHolder>::const_iterator itUrlRule = gCdnSSConfig.c2StreamerConfig.urlRules.begin();
	for( ; itUrlRule != gCdnSSConfig.c2StreamerConfig.urlRules.end(); itUrlRule ++ ) {
		C2Streamer::ConfPerSessionConfig perSessConf;
		C2Streamer::ConfWriter& c2Writer = perSessConf.writer;

		std::map<std::string, C2StreamerSvcConfig::C2StreamerWriterHolder>& confWriters = gCdnSSConfig.c2StreamerConfig.writers;
		std::map<std::string, C2StreamerSvcConfig::C2StreamerWriterHolder>::const_iterator itWriter = confWriters.find(itUrlRule->writerTag);
		if( itWriter == confWriters.end())
			continue;
		const C2StreamerSvcConfig::C2StreamerWriterHolder& confWriter = itWriter->second;

		c2Writer.maxRoundPerRun = confWriter.maxRoundPerRun;
		c2Writer.minYieldInterval = confWriter.minYieldInterval;
		c2Writer.ioBlockSize = 4 * 1024;
		c2Writer.minIdleInterval = 200;
		c2Writer.mSocketKernelSndBuf = confWriter.socketSndBuf;
		c2Writer.mSocketKernelSndTimeo = confWriter.socketSndTimeo;
		c2Writer.mFirstChunkWait = confWriter.firstChunkWait;
		c2Writer.mSocketSendMonitorThreashold = confWriter.socketWriteMonitorThreshold;
		c2Writer.mMaxIdleCountAtFileEnd = confWriter.maxIdleCountAtEnd;
		c2Writer.mDefaultTransferBitrate = confWriter.defaultBitrate;
		c2Writer.mFsReadThreshold = confWriter.fsReadThreshold;
		c2Writer.mbQuitPerConnectionLost = confWriter.quitPerConnectionLost;
		c2Writer.mTimerThreadHold = confWriter.timerMonitorThreshold;
		c2Writer.mSendPacketSize = confWriter.socketPacketSize;
		c2Writer.mPacketsPerSend = confWriter.packetsPerSend;
		c2Writer.mDefaultTransferBitrate = confWriter.defaultBitrate;
		c2Writer.mBitrateInflationPercent = confWriter.bitrateInflation;

		C2Streamer::ConfUrlRule& c2Rule = perSessConf.urlRule;
		c2Rule.urlPrefix = itUrlRule->urlPrefix;
		c2Rule.readerType = readerTagToType( itUrlRule->readerTag );
		c2Rule.fsPath = itUrlRule->fsPath;
		if(c2Rule.readerType < 0 )
			continue;

		c2streamerUrlRules[c2Rule.urlPrefix] = perSessConf;
		getSsEnv()->getMainLogger().debug("urlRule: prefix[%s] writer[%s] reader[%s/%d]",
				itUrlRule->urlPrefix.c_str(), itUrlRule->writerTag.c_str(), 
				itUrlRule->readerTag.c_str(), c2Rule.readerType );
	}

	C2Streamer::ConfOldHls_obsolete& c2Hls = conf.oldHls;
	c2Hls.mKeyFile5I			= gCdnSSConfig.hlsServerConfig.keyfile;
	c2Hls.mAquaRootUrl		= gCdnSSConfig.hlsServerConfig.rooturl;
	c2Hls.mHomeContainer		= gCdnSSConfig.hlsServerConfig.homecontainer;
	c2Hls.mServerHostUrl		= gCdnSSConfig.hlsServerConfig.serverHostUrl;
	c2Hls.mLogFlag			= gCdnSSConfig.hlsServerConfig.logFlag;
	c2Hls.mName2Bitrate		= gCdnSSConfig.hlsServerConfig.subname2bitrate;
	c2Hls.mDefaultBitrate	= gCdnSSConfig.hlsServerConfig.defaultBitrate;

	//CacheCenter
	C2Streamer::ConfCacheCenter& c2CacheCenter = conf.cacheCenter;
	ZQ::common::Config::Holder<CacheCenterConfig>& confCacheCenter = gCdnSSConfig.c2StreamerConfig.c2CacheCenter;
	c2CacheCenter.mCacheDefaultBufferCount = confCacheCenter.cacheBufferCount;
	c2CacheCenter.mCacheDefaultBufferCount = MAX(c2CacheCenter.mCacheDefaultBufferCount,10);
	c2CacheCenter.mCacheBufferSize = confCacheCenter.cacheBufferSize;
	c2CacheCenter.mCacheReadaheadCount = confCacheCenter.cacheReadAheadCount;
	c2CacheCenter.mUseBufferIo  = confCacheCenter.useBufferIo != 0;
	c2CacheCenter.mIndexRecordCacheSize = confCacheCenter.indexRecordCacheSize;
	if( c2CacheCenter.mIndexRecordCacheSize < 1)
		c2CacheCenter.mIndexRecordCacheSize = 1;
	c2CacheCenter.assetAttributesTimeoutInPwe = confCacheCenter.attrTimeout;
	c2CacheCenter.defaultTimeOut = gCdnSSConfig.c2StreamerConfig.c2CacheCenter.defaultTimeOut;
	c2CacheCenter.fioErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2CacheCenter.fioErrTimeOut;
	c2CacheCenter.httpErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2CacheCenter.httpErrTimeOut;
	c2CacheCenter.socketErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2CacheCenter.socketErrTimeOut;
	c2CacheCenter.otherErrTimeOut = gCdnSSConfig.c2StreamerConfig.c2CacheCenter.otherErrTimeOut;



	//AquaReader part
	C2Streamer::ConfAquaReader& c2AquaReader = conf.aquaReader;
	c2AquaReader.aquaReaderRootUrl = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.rootUrl;
	c2AquaReader.aquaReaderUserDomain = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.userDomain;
	c2AquaReader.aquaReaderHomeContainer = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.homeContainer;
	c2AquaReader.aquaReaderMainFileExtension = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.mainFileExtension;
	c2AquaReader.aquaReaderContentNameFormat = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.contentNameFormat;
	c2AquaReader.aquaReaderFlags = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.cdmiFlags;
	c2AquaReader.aquaReaderIoThreadPoolSize = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.ioThreadPool;
	c2AquaReader.aquaReaderCdmiOpsReadThreadPoolSize = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.cdmiOpsThreadPool;
	c2AquaReader.aquaReaderCacheBlockSize = gCdnSSConfig.c2StreamerConfig.c2CacheCenter.cacheBufferSize;
	c2AquaReader.aquaReaderCacheBlockCount = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.cacheBufferCount;
	c2AquaReader.aquaReaderHlsEnableMDataQuery = gCdnSSConfig.c2StreamerConfig.aquaReaderConfig.aquaReaderHlsEnableMDataQuery;

	
	//C2Client Part
	C2Streamer::ConfC2Client& c2Client = conf.c2Client;
	c2Client.C2ClientUpStreamIP 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientUpStreamIP;
	c2Client.C2ClientURL 				  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientURL;
	c2Client.C2ClientTransfer 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientTransfer;
	c2Client.C2ClientHttpCRGAddr 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientHttpCRGAddr;
	c2Client.C2ClientHttpCRGPort 		  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientHttpCRGPort;
	c2Client.C2ClientTransferRate 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientTransferRate;
	c2Client.C2ClientIngressCapacity 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientIngressCapacity;
	c2Client.C2ClientExclusionList 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientExclusionList;
	c2Client.C2ClientTransferDelay 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientTransferDelay;
	c2Client.C2ClientDefaultGetPort 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientDefaultGetPort;
	c2Client.C2ClientWaitBufferTime 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientWaitBufferTime;
	c2Client.C2ClientIndexTimeout 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientIndexTimeout;
	c2Client.C2ClientIndexRetryTimes 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientIndexRetryTimes;
	c2Client.C2ClientMainfileTimeout 	  = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientMainfileTimeout;
	c2Client.C2ClientMainfileRetryTimes = gCdnSSConfig.c2StreamerConfig.c2client.C2ClientMainfileRetryTimes;


	//HttpClient Part
	C2Streamer::ConfHttpClient& c2HttpClient = conf.httpClient;
	c2HttpClient.httpProxyURL = gCdnSSConfig.c2StreamerConfig.httpclient.httpProxyURL;
	c2HttpClient.segmentURL = gCdnSSConfig.c2StreamerConfig.httpclient.segmentURL;
	c2HttpClient.httpTimeOut = gCdnSSConfig.c2StreamerConfig.httpclient.httpTimeOut;
	c2HttpClient.httpRetry = gCdnSSConfig.c2StreamerConfig.httpclient.httpRetry;

	//DiskCache Part
	C2Streamer::ConfDiskCache& c2DiskCache = conf.diskCache;
	c2DiskCache.diskCacheEnabled = true;
	c2DiskCache.cacheLoaderTimeout = gCdnSSConfig.c2StreamerConfig.diskcache.cacheLoaderTimeout;
	c2DiskCache.cacheIgnoreFiles = gCdnSSConfig.c2StreamerConfig.diskcache.cacheIgnoreFiles;
	C2Streamer::DISKCACHECON& dcc = c2DiskCache.diskCacheConfig;
	const std::vector<DiskCache::CACHEDIR>& cacheDir = gCdnSSConfig.c2StreamerConfig.diskcache.cacheDir;
	std::vector<DiskCache::CACHEDIR>::const_iterator iterDiskCache = cacheDir.begin();
	int32 totalBufferCount = c2CacheCenter.mCacheDefaultBufferCount;
	for( ; iterDiskCache != cacheDir.end() ; iterDiskCache ++ )
	{
		if( ! iterDiskCache->homePath.empty() )
		{
			C2Streamer::DiskCacheCon  diskConfg;
			diskConfg.homePath = iterDiskCache->homePath;
			diskConfg.totalSize = iterDiskCache->totalSize;
			diskConfg.LRUSize = iterDiskCache->LRUSize;
			diskConfg.LRUSize = MAX(diskConfg.LRUSize,20);
			totalBufferCount += diskConfg.LRUSize;
			diskConfg.readThreadCount = iterDiskCache->readThreadCount;
			diskConfg.writeThreadCount = iterDiskCache->writeThreadCount;
			diskConfg.pendingsYield = iterDiskCache->pendingsYield;
			dcc.push_back(diskConfg);
		}

	}
	c2CacheCenter.mCacheDefaultBufferCount = totalBufferCount;

	conf.mLocalBindPort     = gCdnSSConfig.c2StreamerConfig.httpBindPort;

	int32 serviceThreadCount = gCdnSSConfig.c2StreamerConfig.serviceThreadCount;
	serviceThreadCount = MAX( serviceThreadCount, 3 );
	int32 locateThreadCount = gCdnSSConfig.c2StreamerConfig.locateThreadCount;
	locateThreadCount = MAX( locateThreadCount, 3 );

	//ZQ::common::NativeThreadPool *pool = new ZQ::common::NativeThreadPool( serviceThreadCount );
	ZQ::common::NativeThreadPool *pool = new C2ThreadPool( serviceThreadCount );
	ZQ::common::NativeThreadPool *locatePool = new ZQ::common::NativeThreadPool( locateThreadCount );
	c2env.mThreadPool			= pool;
	c2env.mLocateThreadPool		= locatePool;
	c2env.mLogger				= &getSsEnv()->getMainLogger();

	ContentFileAttributeBridge* pAttrBridge = new ContentFileAttributeBridge(env);
	c2env.mAttrBridge		= pAttrBridge;

	return true;
}



bool CdnStreamerManager::startupC2Streamer()
{
	  int cpuNum = ZQ::common::SystemInfo::getCpuCounts(&ENVLOG);
	  //cpuNum = cpuNum / 2;
	  if (cpuNum <= 0)
			cpuNum = 2;
	  std::vector<int> cores;
	  {
		  std::vector<std::string> cpuCores;
		  ZQ::common::stringHelper::SplitString(gCdnSSConfig.c2StreamerConfig.cpuCores, cpuCores,","," ,\t\v");
		  if( !cpuCores.empty() ) {
			  for( size_t i = 0 ; i < cpuCores.size(); i ++ ) {
				  std::vector<std::string> sects;
				  ZQ::common::stringHelper::SplitString( cpuCores[i], sects, "~","~ \t\v");
				  if( sects.size() == 2 ) {
					  int a = atoi( sects[0].c_str() );
					  int b = atoi( sects[1].c_str() );
					  if( a > b ) {
						  int t = a ;
						  a = b;
						  b = t;
					  }
					  for( int id = a; id <= b ; id ++ ) {
						  cores.push_back(id);
					  }
				  }  else {
					  int coreId = atoi( cpuCores[i].c_str() );
					  if( coreId >= cpuNum ) {
						  continue;
					  }
					  cores.push_back( coreId );
				  }
			  }
		  } else {
			  for( int i = 0; i < cpuNum; i ++ ) {
				  cores.push_back(i);
			  }
		  }
		  std::sort( cores.begin(), cores.end() );
		  std::vector<int> coreIds;
		  coreIds.swap( cores );
		  int cId = -1;
		  for( size_t i = 0 ; i < coreIds.size(); i ++ ) {
			  if( coreIds[i] != cId ) {
				  cId = coreIds[i];
				  cores.push_back( cId );
			  }
		  }
	  }
	  LibAsync::HttpProcessor::setup(ENVLOG , cores);
     //ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CdnStreamerManager,"startupC2Streamer() init eventloop[%d] successful."), cpuNum);
	if(!setC2Conf(mC2Env))
		return false;

	gC2Env = &mC2Env;


	mC2Service = new C2Streamer::C2Service(mC2Env);
	assert( mC2Service != 0 );
	gC2Svc = mC2Service;

	C2Streamer::C2EventSinkerPtr pSinker = new C2EventSinkerI( env , ss , *this);
	assert( pSinker != 0);

	C2Streamer::C2EventSinkerPtr p(pSinker);
	mC2Service->getEventPublisher().setSinker( pSinker , C2Streamer::METHOD_SESS_UPDATE | C2Streamer::METHOD_RES_UPDATE );
	//C2Streamer::updateEventReceiver( p , C2Streamer::METHOD_SESS_UPDATE );


	if( !mC2Service->startService() )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CdnStreamerManager,"failed to start c2 streamer"));
		return false;
	}

	mC2HttpEngine = new ZQHttp::Engine( env->getMainLogger() );
	C2Streamer::HttpHanlderFactory* fac = new C2Streamer::HttpHanlderFactory( mC2Env , *mC2Service );
	mC2HttpEngine->registerHandler( ".?scs.*" , fac );
	mC2HttpEngine->registerHandler( ".?assets.*",fac);

	//int32 httpThreadCount = gCdnSSConfig.c2StreamerConfig.httpThreadCount;
	//httpThreadCount = MAX(httpThreadCount , 3);

	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"Start Http Engine at[%s][%s]"),
		 gCdnSSConfig.c2StreamerConfig.httpBindIp .c_str(), gCdnSSConfig.c2StreamerConfig.httpBindPort.c_str() );

	//if( gCdnSSConfig.c2StreamerConfig.serverType == 1 ) {
	if(1) {
		mLibAsyncHttpServer = LibAsync::HttpServer::create(HttpServerConfig(), env->getMainLogger() );
		C2Streamer::C2AsyncHttpHandlerFactory::Ptr asyncfac = new C2Streamer::C2AsyncHttpHandlerFactory(*mLibAsyncHttpServer.get(),
															(env->getMainLogger()),fac, 1);

		if(!mLibAsyncHttpServer->addRule("/?vodadi.cgi", asyncfac) ) {
			ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CdnStreamerManager,"failed to register [ /?vodadi.cgi ] as http handler"));
		}
		if(!mLibAsyncHttpServer->addRule("/?cacheserver", asyncfac) ) {
			ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CdnStreamerManager,"failed to register [ /?cacheserver ] as http handler"));
		}
		if(!mLibAsyncHttpServer->addRule(".?scs.*", asyncfac) ) {
			ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CdnStreamerManager,"failed to register [ .?scs.* ] as http handler"));
		}
		if( !mLibAsyncHttpServer->addRule(".?assets.*", asyncfac) ) {
			ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CdnStreamerManager,"failed to register [ .?assets.* ] as http handler"));
		}
		if(!mLibAsyncHttpServer->startAt(gCdnSSConfig.c2StreamerConfig.httpBindIp, gCdnSSConfig.c2StreamerConfig.httpBindPort )){
			ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(CdnStreamerManager,"failed to start server at [%s:%s]"),
					gCdnSSConfig.c2StreamerConfig.httpBindIp.c_str(), gCdnSSConfig.c2StreamerConfig.httpBindPort.c_str());
			return false;
		}
		ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"start async server at[%s:%s]"),
					gCdnSSConfig.c2StreamerConfig.httpBindIp.c_str(), gCdnSSConfig.c2StreamerConfig.httpBindPort.c_str());
//	}  else {
//		mC2HttpEngine->setEndpoint( gCdnSSConfig.c2StreamerConfig.httpBindIp , gCdnSSConfig.c2StreamerConfig.httpBindPort );
//		mC2HttpEngine->setCapacity( httpThreadCount );
//		if( !mC2HttpEngine->start() )
//		{
//			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CdnStreamerManager,"failed to start c2 http engine"));
//			return false;
//		}
//		ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"start httpengine server at[%s:%s]"),
//				gCdnSSConfig.c2StreamerConfig.httpBindIp.c_str(), gCdnSSConfig.c2StreamerConfig.httpBindPort.c_str());
//
	}
	return true;
}
#endif
bool CdnStreamerManager::startup()
{
	mbStarted = false;
	ZQ::common::MutexGuard gd(mStreamerLocker);
	mStreamers.clear();
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"streamer manager start up"));
	//get all streamer's attribute
#ifndef ZQ_CDN_UMG
	if( !startupC2Streamer() )
	{
		return false;
	}
#else
    C2Streamer::getEnvironment()->mgr = this;
    C2Streamer::getEnvironment()->start();
#endif
	queryStreamersInfo();

	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"Streamer manager start up [%s]") , mStreamers.size() > 0 ? "OK" : "FAIL");

	mbStarted = true;
	if( mStreamers.size() > 0 )
	{
		//mDelayDeleteQueue.start();
		return start();
	}
	else
	{
		return false;
	}
}
void CdnStreamerManager::reportStreamState( const std::string& sessId , SsServiceImpl::StreamEvent e )
{
	StreamParams currentParams;
	TianShanIce::Properties uparams;
	ss.OnStreamEvent( e , sessId , currentParams ,uparams );
}


void CdnStreamerManager::queryStreamersInfo()
{
	//query all port
	C2Streamer::ResourceStatusRequestParamPtr request = new C2Streamer::ResourceStatusRequestParam(*getC2Env(), URLRULE_C2RESSTAT );
	request->method		= C2Streamer::METHOD_RESOURCE_STATUS;
	request->portNames.clear();
	C2Streamer::ResourceStatusResponseParamPtr response = new C2Streamer::ResourceStatusResponseParam();

	if( C2Streamer::cResourceStatus( request , response ) == C2Streamer::errorCodeOK )
	{
		StreamerAttr attr;
		const std::vector<C2Streamer::ResourceStatusInfo>& retInfos = response->portInfos;
		std::vector<C2Streamer::ResourceStatusInfo>::const_iterator itPortInfo = retInfos.begin();
		for( ; itPortInfo != retInfos.end() ; itPortInfo++ )
		{
			attr.portName				=	itPortInfo->portName;
			attr.transferAddressIpv4	=	itPortInfo->portAddressIpv4;
			attr.transferAddressIpv6	=	itPortInfo->portAddressIpv6;
			attr.capacity				=	itPortInfo->capacity;
			attr.bUp					=	( itPortInfo->portState == C2Streamer::PORT_STATE_UP );
			attr.activeBandwidth		=	itPortInfo->activeBandwidth;
			attr.activeTransferCount	=	itPortInfo->activeTransferCount;
			attr.transferTcpPort		=	gCdnSSConfig.c2StreamerConfig.httpBindPort;
			reportStreamerState( attr );
		}
	}
}

void CdnStreamerManager::c2shadowIndexComplete( const std::string& name ,
		const std::string& pid, const std::string& paid,
		const std::string& subtype )
{
	std::string key = pid+paid;
	ZQ::common::MutexGuard gd(mC2ShadowIndexMutex);
	mC2ShadowIndexMap.erase(key);
}

void CdnStreamerManager::getShadowIndex(const std::string& upstreamUrl, const std::string& name,
		const std::string& pid, const std::string& paid,
		const std::string& subtype  )
{
	static int clientType = gCdnSSConfig.c2StreamerConfig.defaultReaderType;
	if( clientType == 2 ) { //http fetcher
		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(CdnStreamerManager,"getShadowIndex(), trying to get asset information via httpfetcher for: %s"),
				name.c_str() );
		C2Streamer::C2SessionManager& c2sessmanager = mC2Service->getSessManager();
		std::string filename = name;
		if( subtype.length() > 0 &&subtype.at(0) != '.')
			filename = filename + ".";
		filename += subtype;
		c2sessmanager.getAssetAttribute("", filename, gCdnSSConfig.c2StreamerConfig.defaultReaderType);
		return;
	}
	std::string key = pid+paid;
	ZQ::common::MutexGuard gd(mC2ShadowIndexMutex);
	if( mC2ShadowIndexMap.find(key) != mC2ShadowIndexMap.end())
		return;
	C2ShadowIndexGetterInfo info;
	info.name = name;
	info.pid = pid;
	info.paid = paid;
	info.subtype = subtype;
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(CdnStreamerManager,"getShadowIndex() issue a shadow index download task for [%s] : url[%s]"),
			name.c_str(), upstreamUrl.c_str());
	C2ShadowIndexGetter* getter = new C2ShadowIndexGetter( env,*this, mC2ShadowIndexGetterPool,upstreamUrl, name,pid,paid,subtype);
	getter->start();

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


IngressCapacityUpdateEvent::IngressCapacityUpdateEvent( CdnSsEnvironment* environment )
:env(environment)
{
}
IngressCapacityUpdateEvent::~IngressCapacityUpdateEvent( )
{

}
void IngressCapacityUpdateEvent::post( const std::string& clientAddress , int64 ingressCapacity )
{
	SESSLOG(ZQ::common::Log::L_INFO, EventFMT( (env->getNetId().c_str()) ,IngressCapacity, update , 0, "client[%s] update ingressCapacity[%ld]") ,
		clientAddress.c_str() , ingressCapacity );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//TransferSessionStateUpdateEvent
TransferSessionStateUpdateEvent::TransferSessionStateUpdateEvent( CdnSsEnvironment* environment )
:env(environment)
{

}

TransferSessionStateUpdateEvent::~TransferSessionStateUpdateEvent( )
{
}

void TransferSessionStateUpdateEvent::post(const std::string &client, const std::string &transferId, const std::string &state)
{
	SESSLOG(ZQ::common::Log::L_INFO,EventFMT((env->getNetId().c_str()) ,
		TransferSession,update,0,"client[%s] transferId[%s] state changed to [%s]"),
		client.c_str(), transferId.c_str(),state.c_str()	);
	if( stricmp(state.c_str(),"DELETED") == 0)
	{
		std::string id = /*"/session/" + */transferId;
		env->getStreamerManager().reportStreamState( id , SsServiceImpl::seGone );
	}
}


C2EventSinkerI::C2EventSinkerI( CdnSsEnvironment* environment ,SsServiceImpl& serviceImpl , CdnStreamerManager& manager)
:env(environment),
mSvc(serviceImpl),
mStreamerManager(manager)
{
}
C2EventSinkerI::~C2EventSinkerI()
{
}

int32 C2EventSinkerI::publish(const C2Streamer::C2EventPtr request)
{
	switch ( request->eventmethod )
	{
	case C2Streamer::METHOD_SESS_UPDATE:
		{
			C2Streamer::TransferStateUpdateEventPtr e = C2Streamer::TransferStateUpdateEventPtr::dynamicCast( request );
			if(e)
			{
				return onSessEvent( e );
			}
		}
		break;
	case C2Streamer::METHOD_RES_UPDATE:
		{
			C2Streamer::PortStateUpdateEventPtr e = C2Streamer::PortStateUpdateEventPtr::dynamicCast( request );
			if(e)
			{
				return onStreamerEvent(e);
			}
		}
		break;
	default:
		///FIXME: not finished yet
		return C2Streamer::errorCodeOK;
	}
	return C2Streamer::errorCodeOK;
}

bool emptyStringArray( const std::vector<std::string>& strs ) {
	std::vector<std::string>::const_iterator it = strs.begin();
	for (; it != strs.end() ; it ++ ) {
		if( it->length() > 0 ){
			return false;
		}
	}
	return true;
}

int32 C2EventSinkerI::onStreamerEvent( const C2Streamer::PortStateUpdateEventPtr event)
{
	if( event->portAddressV4.empty() || emptyStringArray( event->portAddressV4 ) ) {
		ENVLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2EventSinkerI,"onStreamerEvent() comes a new streamer event but with empty IPv4 address, ignore"));
		return C2Streamer::errorCodeOK;
	}
	CdnStreamerManager::StreamerAttr attr;

	attr.activeBandwidth = event->activeBandiwidth;
	attr.activeTransferCount = event->activeTransferCount;
	attr.bUp = event->portState == C2Streamer::PORT_STATE_UP;
	attr.capacity = event->capacity;
	attr.portName = event->portName;
	attr.transferAddressIpv4 = event->portAddressV4;
	attr.transferAddressIpv6 = event->portAddressV6;
	attr.transferTcpPort = event->tcpPortNumber;


	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2EventSinkerI,"onStreamerEvent() streamer[%s] status[%s]"),
		attr.portName.c_str() , attr.bUp ? "UP":"DOWN");
	mStreamerManager.reportStreamerState(attr);

	return C2Streamer::errorCodeOK;
}

int32 C2EventSinkerI::onSessEvent( const C2Streamer::TransferStateUpdateEventPtr event )
{
	switch( event->sessionState )
	{
	case  C2Streamer::SESSION_STATE_DELETED:
		{
			ENVLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2EventSinkerI,"onSessEvent() Session [%s] is gone"),event->transferId.c_str() );
			StreamParams paras;
			TianShanIce::Properties props;
			mSvc.OnStreamEvent( SsServiceImpl::seGone, event->transferId , paras , props );
		}
		break;
	default:
		break;
	}
	return C2Streamer::errorCodeOK;
}

///////////////////////////////////////////////////////////////////////////
/// class C2ShadowIndexGetter
C2ShadowIndexGetter::C2ShadowIndexGetter( CdnSsEnvironment* environment,
										 CdnStreamerManager& manager,
										 ZQ::common::NativeThreadPool& pool,
										 const std::string& upstreamUrl,
										 const std::string& name,
										 const std::string& pid,
										 const std::string& paid,
										 const std::string& subtype)
:ZQ::common::ThreadRequest(pool),
env(environment),
mStreamerManager(manager),
mUpstreamUrl(upstreamUrl),
mContentName(name),
mPid(pid),
mPaid(paid),
mSubtype(subtype)
{
}

C2ShadowIndexGetter::~C2ShadowIndexGetter( )
{
}
void C2ShadowIndexGetter::final(int retcode /* =0 */, bool bCancelled /* =false */){
	mStreamerManager.c2shadowIndexComplete(mContentName,mContentName, mPid, mPaid);
	delete this;
}

void fillMetadatForContent( TianShanIce::Storage::UnivContentPrx content,
		const std::string& mainFilePathname,
		ZQ::IdxParser::IndexData& idxData)
{
	TianShanIce::Properties props;

	if(idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VVX ) {
		ZQTianShan::Util::updatePropertyData(props,METADATA_SubType,TianShanIce::Storage::subctVVX );
	}
	else if( idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VV2 ) {
		ZQTianShan::Util::updatePropertyData(props,METADATA_SubType,TianShanIce::Storage::subctVV2 );
	}
	else if(idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VVC) {
		ZQTianShan::Util::updatePropertyData(props,METADATA_SubType,"VVC");
	}
	else {
		return ;
	}
	ZQ::IdxParser::IndexData::SubFileInformation subinfo;
	if(	idxData.getSubFileInfo(0,subinfo) )
	{
		ZQTianShan::Util::updatePropertyData(props,METADATA_EXTNAME_MAIN,subinfo.fileExtension );
	}

	ZQTianShan::Util::updatePropertyData(props, METADATA_BitRate , idxData.getMuxBitrate() );
	ZQTianShan::Util::updatePropertyData(props, METADATA_FILENAME_MAIN , idxData.getMainFilePathName() );
	ZQTianShan::Util::updatePropertyData(props, METADATA_PlayTime , idxData.getPlayTime() );
	ZQTianShan::Util::updatePropertyData(props, METADATA_FileSize , idxData.getMainFileSize() );
	ZQTianShan::Util::updatePropertyData(props, METADATA_IDX_GENERIC_INFO, idxData.baseInfoToXML() );
	ZQTianShan::Util::updatePropertyData(props, METADATA_IDX_SUBFULE_INFO, idxData.memberFileToXML() );

	int32 iSubFileCount = idxData.getSubFileCount();
	int iSubFileIndex = 0;
	char buf[1024];
	std::string strAllExts;
	for(int i =0 ;  i< iSubFileCount ;i ++ ) {
		const std::string& strExtension =  idxData.getSubFileName( i ) ;
		if( !strExtension.empty() ) {
			std::string	subFileName = mainFilePathname + strExtension;
			sprintf(buf , "%s%d" , METADATA_SUBFILENAME, iSubFileIndex++ );
			ZQTianShan::Util::updatePropertyData(props, buf , subFileName );

			//record subfile data start-byte end-byte
			ZQ::IdxParser::IndexData::SubFileInformation sfinfo;
			idxData.getSubFileInfo(i,sfinfo);

			sprintf(buf,"%s%s.%s", "Sys.SubFile.",strExtension.c_str(),"start");
			ZQTianShan::Util::updatePropertyData(props, buf, sfinfo.startingByte );

			sprintf(buf,"%s%s.%s", "Sys.SubFile.",strExtension.c_str(),"end");
			ZQTianShan::Util::updatePropertyData(props, buf, sfinfo.endingByte );

			strAllExts = strAllExts + strExtension;
			strAllExts = strAllExts + ";";
		}
		else {
			sprintf(buf , "%s%d" , METADATA_SUBFILENAME, iSubFileIndex++ );
			ZQTianShan::Util::updatePropertyData(props, buf , mainFilePathname );
			strAllExts = strAllExts + ";";
		}
	}
	ZQTianShan::Util::updatePropertyData(props, std::string("sys.allextentionnames"), strAllExts);
	const std::string& tmpIndexFileName = idxData.getIndexFilePathName();
	sprintf(buf , "%s%d" , METADATA_SUBFILENAME, iSubFileIndex++ );
	ZQTianShan::Util::updatePropertyData(props, buf , tmpIndexFileName );
	ZQTianShan::Util::updatePropertyData(props, METADATA_FILENAME_INDEX , tmpIndexFileName);

	content->setMetaData(props);
}

int C2ShadowIndexGetter::run( )
{
	static int clientType = gCdnSSConfig.c2StreamerConfig.defaultReaderType;
	static const std::string& proxyUrl =  gCdnSSConfig.c2StreamerConfig.httpclient.httpProxyURL;
	static const std::string& segmentUrl = gCdnSSConfig.c2StreamerConfig.httpclient.segmentURL;
	if( clientType == 2 && mUpstreamUrl.empty()  ) {
		std::string::size_type posSlash = mContentName.rfind('/');
		std::string rawName;
		if( posSlash != std::string::npos ) {
			rawName = mContentName.substr(posSlash+1);
		} else {
			rawName= mContentName;
		}
		rawName = rawName + "." + mSubtype;
		//assemble upStream Url
		mUpstreamUrl = segmentUrl + rawName+"-0L4096";
		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2ShadowIndexGetter,"assembly UpStreamUrl to :%s , and set Proxy to:%s"),
				mUpstreamUrl.c_str(), proxyUrl.c_str() );
	}

	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2ShadowIndexGetter,"tring to get index content for [%s], url[%s]"),
			mContentName.c_str(), mUpstreamUrl.c_str());
	CDNIndexGetter getter(env,mContentName, mUpstreamUrl );
	if( clientType == 2 ) {
		getter.setProxyUrl( proxyUrl);
	}

	int32 errCode = getter.invoke( mUpstreamUrl );
	if( errCode /100 != 2 )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2ShadowIndexGetter,"failed to download index for [%s]"),
			mContentName.c_str());
		return -1;
	}
	size_t dataSize = 0;
	const char* indexData = getter.getIndexData(dataSize);

	ZQ::IdxParser::IdxParserEnv			idxParserEnv;
	idxParserEnv.AttchLogger(&env->getMainLogger());
	ZQ::IdxParser::IndexFileParser		idxParser(idxParserEnv);
	ZQ::IdxParser::IndexData			idxData;


	if(!idxParser.ParseIndexFromMemory( mContentName, idxData, indexData, dataSize ) )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2ShadowIndexGetter,"failed to parse index data for[%s], data size[%u]"),
			mContentName.c_str(), (uint32)dataSize);
		return -1;
	}

	try
	{
		TianShanIce::Storage::UnivContentPrx contentPrx = 0;
		std::string::size_type posSlash = mContentName.rfind('/');
		if( posSlash == std::string::npos )
		{
			contentPrx = TianShanIce::Storage::UnivContentPrx::checkedCast(
				env->getCsPrx()->openContentByFullname( mContentName) );
		}
		else
		{
			std::string foldername = mContentName.substr(0,posSlash);
			std::string contentName = mContentName.substr(posSlash+1);
			TianShanIce::Storage::ContentStoreExPrx csExPrx = TianShanIce::Storage::ContentStoreExPrx::checkedCast(env->getCsPrx());
			contentPrx = TianShanIce::Storage::UnivContentPrx::checkedCast(
				csExPrx->openFolderEx(foldername,true,0)->openContent(contentName,"",true)
				);
		}
		assert( contentPrx != 0 );
		fillMetadatForContent(contentPrx, mContentName, idxData);
	}
	catch( const TianShanIce::BaseException& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2ShadowIndexGetter,"failed to create content for [%s], got[%s]"),
			mContentName.c_str(), ex.message.c_str());
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2ShadowIndexGetter,"failed to create content for [%s], got[%s]"),
			mContentName.c_str(), ex.ice_name().c_str());
	}

	return 0;
}

#ifndef ZQ_CDN_UMG
ContentFileAttributeBridge::ContentFileAttributeBridge( CdnSsEnvironment* environment)
:env(environment)
{
	mStorePrx = env->mCsPrx;
}

const char* contentStateToString( const TianShanIce::Storage::ContentState& state )
{
	switch(state)
	{
	case TianShanIce::Storage::csNotProvisioned:
		return "NOTPROVISIONED";
	case TianShanIce::Storage::csProvisioning:
		return "PROVISIONING";
	case TianShanIce::Storage::csProvisioningStreamable:
		return "STREAMABLE";
	case TianShanIce::Storage::csInService:
		return "INSERVICE";
	case TianShanIce::Storage::csOutService:
		return "OUTSERVICE";
	case TianShanIce::Storage::csCleaning:
		return "CLEAN";
	default:
		return "UNKNOWN";
	}

}

bool ContentFileAttributeBridge::getFileDataRange( const std::string& filename , const std::string& sessionId , int64& startByte , int64& endByte )
{
	std::string contentName = std::string("/$/") + filenameToContentName(filename);
	std::string extname = filenameToExtname(filename);
	try
	{
		C2Streamer::StopWatch openWatch;openWatch.start();
		::TianShanIce::Storage::ContentPrx content = mStorePrx->openContentByFullname( contentName );
		if( !content )
		{
			ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentFileAttributeBridge,"Session[%s]: content[%s] for file[%s] is not found"),
				sessionId.c_str(), contentName.c_str()  , filename.c_str() );
			return false;
		}
		openWatch.stop();

		C2Streamer::StopWatch getStateWatch; getStateWatch.start();
		TianShanIce::Storage::ContentState state =  content->getState();
		getStateWatch.stop();

		if(state != TianShanIce::Storage::csInService)
		{
			ENVLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ContentFileAttributeBridge,"Session[%s], file[%s] is not at InService state"),
				sessionId.c_str() ,filename.c_str() );
			return false;
		}

		C2Streamer::StopWatch getMetaWatch; getMetaWatch.start();
		TianShanIce::Properties metadata = content->getMetaData();
		getMetaWatch.stop();
		std::string startbyteKey = "Sys.SubFile." + extname + ".start";
		std::string endbyteKey = "Sys.SubFile." + extname + ".end";
		ZQTianShan::Util::getPropertyData(metadata,startbyteKey,startByte);
		ZQTianShan::Util::getPropertyData(metadata,endbyteKey,endByte);
		endByte += 1;//end byte offset + 1 = file size

		IceUtil::Time stoptime = IceUtil::Time::now();
		ENVLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ContentFileAttributeBridge,"Session[%s]: get file[%s] data range[%ld - %ld], time cost[%ld/%ld/%ld]us"),
			sessionId.c_str() , filename.c_str() , startByte , endByte , openWatch.span() , getStateWatch.span() , getMetaWatch.span() );
		return true;
	}
	catch( const TianShanIce::BaseException& ex)
	{
		ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentFileAttributeBridge, "Session[%s]: caught [%s] while getting data range for content[%s] file[%s]"),
			sessionId.c_str(), ex.message.c_str(), contentName.c_str() , filename.c_str() );
	}
	catch(const Ice::Exception& ex)
	{
		ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentFileAttributeBridge, "Session[%s]: caught [%s] hile getting data range for content[%s] file[%s]"),
			sessionId.c_str(), ex.ice_name().c_str(), contentName.c_str() , filename.c_str() );
	}
	return false;
}

bool ContentFileAttributeBridge::isFileBeingWritten( const std::string& filename , const std::string& sessionId)
{
	std::string contentName = std::string("/$/") + filenameToContentName(filename);
	try
	{
		C2Streamer::StopWatch openWatch; openWatch.start();
		::TianShanIce::Storage::ContentPrx content = mStorePrx->openContentByFullname( contentName );
		if( !content )
		{
			openWatch.stop();
			ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentFileAttributeBridge,"Session[%s]: content[%s] for file[%s] is not found, time cost[%ld]us"),
				 sessionId.c_str(), contentName.c_str()  , filename.c_str(), openWatch.span() );
			return false;
		}
		openWatch.stop();

		C2Streamer::StopWatch getStateWatch; getStateWatch.start();
		TianShanIce::Storage::ContentState state =  content->getState();
		getStateWatch.stop();
 		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentFileAttributeBridge,"Session[%s]: got content[%s] for file[%s], state[%s] , time cost[%ld/%ld]us"),
 			 sessionId.c_str(), contentName.c_str() , filename.c_str() , contentStateToString(state), openWatch.span() , getStateWatch.span() );
		return ( state >= TianShanIce::Storage::csProvisioning && state <= TianShanIce::Storage::csProvisioningStreamable );
	}
	catch( const TianShanIce::BaseException& ex)
	{
		ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentFileAttributeBridge, "Session[%s]: caught [%s] while check content[%s] for file[%s]"),
			   sessionId.c_str(), ex.message.c_str(), contentName.c_str() , filename.c_str() );
	}
	catch(const Ice::Exception& ex)
	{
		ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentFileAttributeBridge, "Session[%s]: caught [%s] while check content[%s] for file[%s]"),
			   sessionId.c_str(), ex.ice_name().c_str(), contentName.c_str() , filename.c_str() );
	}
	return false;
}

std::string ContentFileAttributeBridge::filenameToContentName( const std::string& filename ) const
{
	std::string name = filename;
	std::string::size_type posSlash = name.find_first_not_of('/');
	if(posSlash != std::string::npos ) {
		name = name.substr(posSlash);
	}
	std::string::size_type pos = name.find_last_of('.');
	if(pos == std::string::npos) {  return name;  }

	std::string ext1 = name.substr(pos+1);
	std::transform(ext1.begin(), ext1.end(), ext1.begin(), (int (*)(int))(toupper));
	if(ext1.find("0X") == 0 || ext1 == "INDEX")
	{
		return name.substr(0, pos);
	}
	std::string ext = name.substr(pos+1, 2);
	std::transform(ext.begin(), ext.end(), ext.begin(), (int (*)(int))(toupper));
	if(ext == "VV" || ext == "FF" || ext == "FR")
	{
		return name.substr(0, pos);
	}
	else
	{
		return name;
	}
}

std::string ContentFileAttributeBridge::filenameToExtname( const std::string& filename ) const
{
	/*std::string name = filename;
	while(name.length() >0 && name.at(0)=='/') {
		name =name.substr(1)
	}*/

	std::string::size_type pos = filename.rfind('.');
	if( pos == std::string::npos )
	{
		return filename;
	}
	else
	{
		return filename.substr(pos);
	}
}
#endif

}}//namespace ZQ::StreamService
