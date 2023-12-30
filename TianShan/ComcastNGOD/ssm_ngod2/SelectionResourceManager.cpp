
#include <ZQ_common_conf.h>
#include <math.h>
#include <TianShanDefines.h>
#include <strHelper.h>
#include <TimeUtil.h>
#include <TianShanIceHelper.h>
#include "SelectionResourceManager.h"

#define MLOG		(*mEnv.getMainLogger())
#define ELOG		(*mEnv.getEventLogger())


//////////////////////////////////////////////////////////////////////////
///
ResourceStreamerAttrEx::ResourceStreamerAttrEx()
{		
	streamerWeight			= MAX_WEIGHT;
	//importChannelWeight		= MAX_WEIGHT;
}
ResourceStreamerAttrEx::ResourceStreamerAttrEx( const ResourceStreamerAttrEx& a )
{
	netId				= a.netId;
	nodeId				= a.nodeId;
	endpoint			= a.endpoint;
	bReplicaStatus		= a.bReplicaStatus;
	lastReplicaUpdate	= a.lastReplicaUpdate;
	maxBw				= a.maxBw;				
	usedBw				= a.usedBw;
	maxSessCount		= a.maxSessCount;
	usedSessCount		= a.usedSessCount;
	streamServicePrx	= a.streamServicePrx;
	penalty				= a.penalty;
	importChannelName	= a.importChannelName;
	statisticsRemoteSessCount		= a.statisticsRemoteSessCount;
	maxSessCount		= a.maxSessCount;
	statisticsFailedSessCount		= a.statisticsFailedSessCount;
	volumeNetId			= a.volumeNetId;
	availPartition		= a.availPartition;
	bAllPartitionAvail	= a.bAllPartitionAvail;
	bMaintainEnable		= a.bMaintainEnable;
	streamerWeight		= a.streamerWeight;
}
ResourceStreamerAttrEx::ResourceStreamerAttrEx( const ResourceStreamerAttr& a)
{
	netId				= a.netId;
	nodeId				= a.nodeId;
	endpoint			= a.endpoint;
	bReplicaStatus		= a.bReplicaStatus;
	lastReplicaUpdate	= a.lastReplicaUpdate;
	maxBw				= a.maxBw;				
	usedBw				= a.usedBw;
	maxSessCount		= a.maxSessCount;
	usedSessCount		= a.usedSessCount;
	streamServicePrx	= a.streamServicePrx;
	penalty				= a.penalty;
	importChannelName		= a.importChannelName;
	statisticsRemoteSessCount		= a.statisticsRemoteSessCount;
	maxSessCount		= a.maxSessCount;
	statisticsFailedSessCount		= a.statisticsFailedSessCount;
	volumeNetId			= a.volumeNetId;
	availPartition		= a.availPartition;
	bAllPartitionAvail	= a.bAllPartitionAvail;
	bMaintainEnable		= a.bMaintainEnable;

	streamerWeight		= 0;
}
ResourceStreamerAttrEx& ResourceStreamerAttrEx::operator=( const ResourceStreamerAttrEx& a )
{
	netId				= a.netId;
	nodeId				= a.nodeId;
	endpoint			= a.endpoint;
	bReplicaStatus		= a.bReplicaStatus;
	lastReplicaUpdate	= a.lastReplicaUpdate;
	maxBw				= a.maxBw;				
	usedBw				= a.usedBw;
	maxSessCount		= a.maxSessCount;
	usedSessCount		= a.usedSessCount;
	streamServicePrx	= a.streamServicePrx;
	penalty				= a.penalty;
	importChannelName		= a.importChannelName;
	statisticsRemoteSessCount		= a.statisticsRemoteSessCount;
	maxSessCount		= a.maxSessCount;
	statisticsFailedSessCount		= a.statisticsFailedSessCount;
	volumeNetId			= a.volumeNetId;
	availPartition		= a.availPartition;
	bAllPartitionAvail	= a.bAllPartitionAvail;
	bMaintainEnable		= a.bMaintainEnable;
	streamerWeight		= a.streamerWeight;
	return  *this;
}
ResourceStreamerAttrEx& ResourceStreamerAttrEx::operator=( const ResourceStreamerAttr& a )
{
	netId				= a.netId;
	nodeId				= a.nodeId;
	endpoint			= a.endpoint;
	bReplicaStatus		= a.bReplicaStatus;
	lastReplicaUpdate	= a.lastReplicaUpdate;
	maxBw				= a.maxBw;				
	usedBw				= a.usedBw;
	maxSessCount		= a.maxSessCount;
	usedSessCount		= a.usedSessCount;
	streamServicePrx	= a.streamServicePrx;
	penalty				= a.penalty;
	importChannelName		= a.importChannelName;
	statisticsRemoteSessCount		= a.statisticsRemoteSessCount;
	maxSessCount		= a.maxSessCount;
	statisticsFailedSessCount		= a.statisticsFailedSessCount;
	volumeNetId			= a.volumeNetId;
	availPartition		= a.availPartition;
	bAllPartitionAvail	= a.bAllPartitionAvail;
	bMaintainEnable		= a.bMaintainEnable;

	streamerWeight		= 0;
	return *this;
}



//////////////////////////////////////////////////////////////////////////
///
bool parseVolumeName( const std::string& volumeName , VolumeNameAttr& attr)
{
	std::string::size_type slashPos = volumeName.find('/');

	attr.bDefaultPartition = false;//set it to false
	if( slashPos != std::string::npos )
	{
		attr.netId	= volumeName.substr( 0 , slashPos );
		std::string partition = volumeName.substr( slashPos+1 );
		ZQ::common::stringHelper::TrimExtra( partition );
		if( partition.empty() || ( partition.compare("$") ==  0 ) )
		{// '$' is used to identify the default volume
			attr.bDefaultPartition	= true;
		}
		else
		{
			attr.partition			= partition;
		}
	}
	else
	{//do not find slash mark, treat that whole storage can be used
		attr.bDefaultPartition		= true;
		attr.netId					= volumeName;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
///NgodResourceManager
NgodResourceManager::NgodResourceManager( SelectionEnv& env )
:mEnv(env),
mbCheckThreadQuit(true),
mResourceManagerEventSinker(NULL)
{
}

NgodResourceManager::~NgodResourceManager(void)
{
}
void NgodResourceManager::attachEventSinker(IResourceManagerEventSink* sinker)
{
	mResourceManagerEventSinker = sinker;
}
bool NgodResourceManager::initResourceManager( bool bNeedWarmup )
{
	ZQ::common::MutexGuard gd(*this);
	
	//collect available volumes for each sop
	//create streamer symbol link, so that we can access streamer quickly when applying penalty and updating replicas
	mAvailVolumesBySop.clear();
	mStreamerSymLinks.clear();
	mNasVolumes.clear();

	SOPS::iterator itSop = mSops.begin();
	for( ; itSop != mSops.end() ; itSop++ )
	{
		ResourceVolumeAttrMap volumeMapInSop;
		ResourceVolumeAttrMap nasVolumesInSop;

		ResourceStreamerAttrMap& streamers = itSop->second;

		ResourceStreamerAttrMap::iterator itStreamer = streamers.begin();
		for( ; itStreamer != streamers.end() ; itStreamer++ )
		{
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodResourceManager,"init() streamer info: groupName[%s] streamer[%s] volume[%s] ic[%s]"),
				itSop->first.c_str() , 
				itStreamer->second.netId.c_str() , 
				itStreamer->second.volumeNetId.c_str() , 
				itStreamer->second.importChannelName.c_str());
			//get nodeId from streamer's netid
			std::string::size_type posSlash = itStreamer->second.netId.find("/");
			if( posSlash != std::string::npos )
			{
				itStreamer->second.nodeId = itStreamer->second.netId.substr( 0,posSlash );
			}
			{//create streamer symbol link so that we can access the streamer easily if a streamer was configured in different group
				const std::string& streamerNetId = itStreamer->second.netId;
				StreamerIteratorSetMap::iterator itSymlink = mStreamerSymLinks.find( streamerNetId );
				if( itSymlink == mStreamerSymLinks.end() )
				{
					StreamerIteratorSet iterSet;
					iterSet.push_back( itStreamer );
					mStreamerSymLinks.insert( StreamerIteratorSetMap::value_type( streamerNetId , iterSet ) );
				}
				else
				{
					StreamerIteratorSet& iterSet = itSymlink->second;
					iterSet.push_back( itStreamer );
				}
			}

			{//collect available volumes
				//collect nas volumes
				const std::string& volumeNetId = itStreamer->second.volumeNetId;
				ResourceVolumeAttrMap::const_iterator itConfVolume = mVolumes.find( volumeNetId );
				if( itConfVolume == mVolumes.end() )
				{//invalid configuration, can't find the volume according to netId from volumes configuration
					continue;
				}
				

				if( ( itConfVolume->second.bSupportNas ) && ( nasVolumesInSop.find(volumeNetId) == nasVolumesInSop.end() ) )
				{
					nasVolumesInSop.insert( ResourceVolumeAttrMap::value_type(itConfVolume->first , itConfVolume->second) );
				}


				if( volumeMapInSop.find( volumeNetId ) == volumeMapInSop.end() )
				{//volume has already been recorded
					volumeMapInSop.insert( ResourceVolumeAttrMap::value_type(itConfVolume->first , itConfVolume->second) );
					
				}
			}
		}
		mAvailVolumesBySop.insert( VolumeMapMap::value_type( itSop->first , volumeMapInSop ) );
		mNasVolumes.insert( VolumeMapMap::value_type( itSop->first , nasVolumesInSop ) );	
	}

	resetStatisticsCounter();

	//warm up streamer by query its replica information
	if( bNeedWarmup )
	{
		StreamerIteratorSetMap streamerIters = mStreamerSymLinks;
		StreamerIteratorSetMap::iterator itIter = streamerIters.begin();
		for( ; itIter != streamerIters.end() ; itIter++ )
		{
			StreamerIteratorSet& streamers = itIter->second;
			
			if( streamers.empty() ) continue;

			StreamerIteratorSet::iterator itStreamer = streamers.begin();
			TianShanIce::Streamer::StreamSmithAdminPrx streamerPrx = (*itStreamer)->second.streamServicePrx;
			const std::string streamerNetId = (*itStreamer)->second.netId;
			std::string::size_type pos = streamerNetId.find("/");
			std::string groupId = streamerNetId ;
			if( pos != std::string::npos )
			{
				groupId = groupId.substr( 0 , pos );
			}
			try
			{
				TianShanIce::Replicas reps = streamerPrx->queryReplicas( "Streamer", groupId, true );
				updateReplica(reps);

				reps = streamerPrx->queryReplicas( "BandwidthUsage", groupId, true );
				updateReplica(reps);
			}
			catch( const Ice::Exception& ex )
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodResourceManager,"initResourceManager() got [%s] when query replica for streamer [%s], ednpoint[%s]"),
					ex.ice_name().c_str() , streamerNetId.c_str() , (*itStreamer)->second.streamServiceEndpoint.c_str());
			}
		}
	}

	return true;
}

void NgodResourceManager::updateSopData( const SOPS& sops )
{
	ZQ::common::MutexGuard gd(*this);
	mSops = sops;
}

void NgodResourceManager::updateVolumesData( const ResourceVolumeAttrMap& volumes )
{
	ZQ::common::MutexGuard gd(*this);
	mVolumes = volumes;
}

void NgodResourceManager::updateImportChannelData( const ResourceImportChannelAttrMap& ics )
{
	ZQ::common::MutexGuard gd(*this);
	mImportChannles = ics;
}

bool NgodResourceManager::getSopData( SOPS& sops ,std::string& measuredSince ) const
{
	ZQ::common::MutexGuard gd(*this);
	sops			=  mSops;
	measuredSince	= mStrMeasuredSince;
	return true;
}

bool NgodResourceManager::getImportChannelData( ResourceImportChannelAttrMap& ics ) const
{
	ZQ::common::MutexGuard gd(*this);
	ics =  mImportChannles;
	return true;
}

bool NgodResourceManager::hasAvailVolumes( const std::string& identifier ) const
{
	VolumeMapMap::const_iterator it = mAvailVolumesBySop.find( identifier );
	if( it == mAvailVolumesBySop.end() )
		return false;
	else
		return it->second.size() > 0 ;
}
const ResourceVolumeAttrMap& NgodResourceManager::getAvailVolumes( const std::string& identifier ) const
{
	//NOTICE: call hasAvailVolumes to detect if there is available volume for identifier before invoke this function

	//DO NOT lock this because no one would modify this member
	VolumeMapMap::const_iterator it = mAvailVolumesBySop.find( identifier );
	assert( it != mAvailVolumesBySop.end() );
	return it->second;
}

bool NgodResourceManager::getAvailVolumes( const std::string& identifier , ResourceVolumeAttrMap& volumes ) const
{
	//DO NOT lock this because no one would modify this member
	if( hasAvailVolumes(identifier))
	{
		volumes = getAvailVolumes(identifier);
		return true;
	}
	else
	{
		return false;
	}
}

bool NgodResourceManager::hasSupportNasVolumes( const std::string& identifier ) const
{
	//DO NOT lock this because no one would modify this member
	VolumeMapMap::const_iterator it = mNasVolumes.find( identifier );
	if( it == mNasVolumes.end() )
		return false;
	else
		return it->second.size() > 0 ;
}

const ResourceVolumeAttrMap& NgodResourceManager::getSupportNasVolumes( const std::string& identifier ) const
{
	//NOTICE: call hasSupportNasVolumes to detect if there is available volume for identifier before invoke this function
	//DO NOT lock this because no one would modify this member
	VolumeMapMap::const_iterator it = mNasVolumes.find( identifier );
	assert( it != mNasVolumes.end() );
	return it->second;
}

bool NgodResourceManager::getSupportNasVolumes( const std::string& identifier,ResourceVolumeAttrMap& volumes ) const
{
	//DO NOT lock this because no one would modify this member
	if( hasSupportNasVolumes(identifier) )
	{
		volumes = getSupportNasVolumes(identifier);
		return true;
	}
	else
	{
		return false;
	}
}

bool NgodResourceManager::getVolumeAttr( const std::string& volumeName , ResourceVolumeAttr& attr ) const
{
	//DO NOT lock this because no one would modify this member
	ResourceVolumeAttrMap::const_iterator it = mVolumes.find( volumeName );
	if( it != mVolumes.end() )
	{
		attr = it->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool NgodResourceManager::isStreamHealthy(const std::string& streamerNetId, int *penalty ) {
	ZQ::common::MutexGuard gd(*this);
	StreamerIteratorSetMap::const_iterator it = mStreamerSymLinks.find( streamerNetId);
	if( it != mStreamerSymLinks.end() )
	{	
		const StreamerIteratorSet& streamers = it->second;
		if( streamers.size() > 0  )
		{
			int p = (*streamers.begin())->second.penalty;
			bool bEnable = (*streamers.begin())->second.bMaintainEnable;
			if(penalty)
				*penalty = p;
			return (p <= 0) && bEnable; 
		}
	}
	return false;
}

bool NgodResourceManager::isStreamerReplicaAvail( const std::string& streamerNetId ) const
{
	ZQ::common::MutexGuard gd(*this);
	StreamerIteratorSetMap::const_iterator it = mStreamerSymLinks.find( streamerNetId);
	if( it != mStreamerSymLinks.end() )
	{	
		const StreamerIteratorSet& streamers = it->second;
		if( streamers.size() > 0  )
		{
			return (*streamers.begin())->second.bReplicaStatus;
		}
	}
	return false;
}

void NgodResourceManager::maintainEnable( const std::string& groupName , const std::string& streamerNetId , bool enable )
{
	ZQ::common::MutexGuard gd(*this);
	SOPS::iterator itSop = mSops.find( groupName );
	if( itSop == mSops.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NgodResourceManager,"maintainEnable() can't find sop data for group[%s]"), groupName.c_str() );
		return;
	}
	ResourceStreamerAttrMap& streamers = itSop->second;
	ResourceStreamerAttrMap::iterator itStreamer = streamers.find( streamerNetId );
	if( itStreamer == streamers.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NgodResourceManager,"maintainEnable() failed to find streamer for group[%s] streamerNetId[%s]"),
			groupName.c_str() , streamerNetId.c_str() );
		return ;
	}
	itStreamer->second.bMaintainEnable = enable;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NgodResourceManager,"maintainEnable() set group[%s] streamerNetId[%s] to [%s]"),
		groupName.c_str() , streamerNetId.c_str() , enable ? "ENABLE" : "DISABLE");
	
}

void NgodResourceManager::applyIncreasablePenalty( const std::string& sessId, const std::string& streamerNetId , int32 penaltyValue , int32 maxPenaltyValue )
{
	bool bNeedReport = false;	
	{
		ZQ::common::MutexGuard gd(*this);

		StreamerIteratorSetMap::iterator itStreamerSet = mStreamerSymLinks.find( streamerNetId);
		if( itStreamerSet == mStreamerSymLinks.end() )
		{
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NgodResourceManager,"applyPenalty() sess[%s] can't find streamer with NetId[%s]"), 
			 sessId.c_str(), streamerNetId.c_str() );
			return;
		}

		StreamerIteratorSet& streamerSet = itStreamerSet->second;
		StreamerIteratorSet::iterator itStreamerSymLink = streamerSet.begin();
		for( ; itStreamerSymLink != streamerSet.end() ; itStreamerSymLink++ )
		{
			ResourceStreamerAttrMap::iterator& itStreamer = *itStreamerSymLink;

			if(itStreamer->second.penalty <= 0 && penaltyValue > 0 )
			{
				bNeedReport = true;
			}
			itStreamer->second.penalty += penaltyValue;
			itStreamer->second.penalty = MIN( itStreamer->second.penalty, maxPenaltyValue );

			//for log use
			penaltyValue = itStreamer->second.penalty;
		}
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NgodResourceManager,"applyPenalty() sess[%s] set penalty [%d] to streamer[%s]"),
		sessId.c_str(), penaltyValue , streamerNetId.c_str() );

	//report streamer down event
	if(bNeedReport && mResourceManagerEventSinker)
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodResourceManager,"applyPenalty() sess[%s] issue an event to show that streamer[%s] is DOWN"),
			sessId.c_str(), streamerNetId.c_str() );
		mResourceManagerEventSinker->onStreamerEvent( streamerNetId , false );
	}
}

void NgodResourceManager::applyPenalty( const std::string& sessId, const std::string& streamerNetId , int32 penaltyValue )
{
	applyIncreasablePenalty( sessId, streamerNetId , penaltyValue , penaltyValue );
}

void NgodResourceManager::updateStreamerReplica( const std::string& streamerNetId, bool status )
{
	bool bNeedLog = false;
	{
		ZQ::common::MutexGuard gd(*this);

		StreamerIteratorSetMap::iterator itStreamerSet = mStreamerSymLinks.find( streamerNetId);
		if( itStreamerSet == mStreamerSymLinks.end() )
		{
			if(strstr(streamerNetId.c_str(),"BoardNumber")==NULL)
				MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodResourceManager,"updateStreamerReplica() can't find streamer with NetId[%s]"), streamerNetId.c_str() );
			return;
		}

		Ice::Long curTime = ZQTianShan::now();

		StreamerIteratorSet& streamerSet = itStreamerSet->second;
		StreamerIteratorSet::iterator itStreamerSymLink = streamerSet.begin();
		for( ; itStreamerSymLink != streamerSet.end() ; itStreamerSymLink++ )
		{
			ResourceStreamerAttrMap::iterator& itStreamer = *itStreamerSymLink;
			if( status != itStreamer->second.bReplicaStatus )
			{
				bNeedLog = true;
			}
			itStreamer->second.bReplicaStatus		= status;
			if( status )
			{//only record last update time if status is OK
				itStreamer->second.lastReplicaUpdate	= curTime;
				if( itStreamer->second.penalty > 0 )
					itStreamer->second.penalty --;
				if(mEnv.mbPublishLog)
				{
					MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodResourceManager, "PenaltyManager streamer penalty[%d] netId[%s] endpoint[%s]"), 
						itStreamer->second.penalty, itStreamer->second.netId.c_str(), itStreamer->second.endpoint.c_str());					
				}
			}

		}
	}
	if ( bNeedLog )
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NgodResourceManager,"updateStreamerReplica() update status [%s] to streamer[%s]"), 
			status ? "UP":"DOWN" , streamerNetId.c_str() );
		if( mResourceManagerEventSinker )
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodResourceManager,"updateStreamerReplica() issue an event to show that streamer[%s] is [%s]"),
				streamerNetId.c_str() ,status ? "UP":"DOWN" );
			mResourceManagerEventSinker->onStreamerEvent( streamerNetId , status );
		}
	}
	
}

void NgodResourceManager::updateImportChannelReplica( const std::string& importChannelName , int64 bwUsed , int64 bwTotal ,int32 runningSess )
{
	ZQ::common::MutexGuard gd(*this);
	ResourceImportChannelAttrMap::iterator itImportChannel = mImportChannles.find( importChannelName );
	if( itImportChannel == mImportChannles.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT( NgodResourceManager,"updateImportChannelReplica() failed to find importChannelName with name[%s]"),importChannelName.c_str() );
		return;
	}
	bool bLog = (itImportChannel->second.reportUsedBW	!= bwUsed) ||
		(itImportChannel->second.reportMaxBW		!= bwTotal ) ||
		(itImportChannel->second.reportUsedSessCount != runningSess);
	
	itImportChannel->second.reportUsedBW	= bwUsed;
	itImportChannel->second.reportMaxBW		= bwTotal;
	itImportChannel->second.reportUsedSessCount = runningSess;

	if( bLog)
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT( NgodResourceManager,"updateImportChannelReplica() update importChannle[%s] .Import channel status: bwUsed[%lld] bwMax[%lld] runningSess[%d] maxSess[%lld]"),
				importChannelName.c_str() , bwUsed, bwTotal , runningSess , itImportChannel->second.confMaxSessCount);
	}
}

#if defined ZQ_OS_MSWIN
	#define	REQFMT(x,y) 	"%s/%s/%s/%06X/[%8s]\t"##y,para.sessionId.c_str(),para.cseq.c_str(),para.method.c_str(),GetCurrentThreadId(),#x
#else
	#define	REQFMT(x,y) 	"%s/%s/%s/%06X/[%8s]\t"y,para.sessionId.c_str(),para.cseq.c_str(),para.method.c_str(),pthread_self(),#x
#endif	


NgodResourceManager::StreamerIterMap NgodResourceManager::findByIdentifier( const std::string& identifier ) 
{
	StreamerIterMap rets;
	SOPS::iterator it = mSops.find( identifier );
	if( it != mSops.end() )
	{
		ResourceStreamerAttrMap& attrs = it->second;
		ResourceStreamerAttrMap::iterator itAttr = attrs.begin();
		for( ;itAttr != attrs.end() ; itAttr++ )
		{
			rets.insert( StreamerIterMap::value_type( itAttr->second.netId , itAttr ) );
		}
		return rets;
	}
	else
	{
		return rets;
	}
}

bool NgodResourceManager::getStreamersFromSopAndVolume( const StreamerResourcePara& para , ResourceStreamerAttrExS& rets )
{
	ZQ::common::MutexGuard gd(*this);

	//collect streamer iterators from record with identifier 
	const StreamerIterMap streamers = findByIdentifier( para.identifier );

	if( streamers.empty() )
	{//no streamer iterator was found, return false
		MLOG(ZQ::common::Log::L_ERROR,REQFMT(NgodResourceManager,"getStreamersFromSopAndVolume() no sop with identifier[%s] is available"), para.identifier.c_str() );
		return false;
	}

	StreamerIterMap::const_iterator itStreamerIter = streamers.begin();
	for( ; itStreamerIter != streamers.end() ; itStreamerIter++ )
	{// walk streamers and try to find the available streamer to create stream on
		
		ResourceStreamerAttrMap::const_iterator itStreamer = itStreamerIter->second;

		if( para.volumeNetIds.find( itStreamer->second.volumeNetId) == para.volumeNetIds.end() )
		{//if the streamer is not relative to our candidate volumes , discard it
			continue;
		}

		const ResourceStreamerAttr& streamerAttr = itStreamer->second;
		//check if the streamer is able to hold a stream or not
		if( !checkStreamerConfiguration( para , streamerAttr ) )
			continue;

		//evaluate the streamer to see if the streamer is able to hold a stream
		int32 streamerWeight = evaluateStreamer( para , streamerAttr );
		if( streamerWeight <= 0 )
		{
			continue;//do not select this streamer because streamer is not available
		}
		
		if( mEnv.excludeC2Linkdown() )
		{
			if( !isImportChannelAvailable( para , streamerAttr.importChannelName) )
			{
				MLOG(ZQ::common::Log::L_WARNING,REQFMT(NgodResourceManager,"getStreamersFromSopAndVolume() skip streamer[%s] because it's import channel is not available"),
					streamerAttr.netId.c_str() );
				continue;
			}
		}

		int32 importChannelWeight = MAX_WEIGHT;
		
		if( para.bNeedImportChannel )		
		{//evaluate importchannel's weight
			importChannelWeight = evaludateImportChannel( para , streamerAttr );			
			if( importChannelWeight <= 0)
				continue;
		}
		else
		{
			MLOG(ZQ::common::Log::L_DEBUG, REQFMT(NgodResourceManager, "getStreamersFromSopAndVolume() no import channel needed, set importChannelWeight to MAX_WEIGHT"));
		}

		int32 weight = streamerWeight > importChannelWeight ? importChannelWeight : streamerWeight;
		ResourceStreamerAttrEx ret = itStreamer->second;		
		MLOG(ZQ::common::Log::L_INFO,REQFMT(NgodResourceManager,"getStreamersFromSopAndVolume() select streamer[%s] as candidate: rquestBW[%d]  usedBW[%lld] totalBW[%lld] usedSess[%lld] totalSess[%lld] weight[%d], streaming weight[%d] importing channel weight [%d] "),
				ret.netId.c_str(),para.requestBW , streamerAttr.usedBw , streamerAttr.maxBw , streamerAttr.usedSessCount, streamerAttr.maxSessCount , weight, streamerWeight, importChannelWeight );

		ret.streamerWeight = weight;
		//get a available streamer which can be used to create a stream on	

		rets.push_back( ret );
	}
	return rets.size() > 0 ;
}

bool NgodResourceManager::allocateResource( const StreamerResourcePara& para, const std::string& streamerNetId )
{
	ZQ::common::MutexGuard	gd(*this);
	
	StreamerIterMap streamers = findByIdentifier( para.identifier );
	if( streamers.empty() )
	{
		MLOG(ZQ::common::Log::L_ERROR,REQFMT(NgodResourceManager,"getStreamersFromSopAndVolume() no sop with identifier[%s] is available"), para.identifier.c_str() );
		return false;
	}

	StreamerIterMap::iterator itStreamerIter = streamers.find( streamerNetId )		 ;
	if( itStreamerIter == streamers.end() )
	{
		MLOG(ZQ::common::Log::L_ERROR,REQFMT(NgodResourceManager,"allocateResource()  no streamer with netId[%s] identifier[%s] is found"),
			streamerNetId.c_str() , para.identifier.c_str()	);
		return false;
	}

	ResourceStreamerAttrMap::iterator itStreamer = itStreamerIter->second;	
	ResourceStreamerAttr& streamerAttr = itStreamer->second;

	if( !checkStreamerConfiguration( para , streamerAttr ))
	{
		MLOG(ZQ::common::Log::L_ERROR, REQFMT(NgodResourceManager,"allocateResource() failed to allocate resource from netId[%s] identifier[%s] due to streamer not avaialable"),
			streamerNetId.c_str() , para.identifier.c_str()	);
		return false;
	}
	if( evaluateStreamer( para, streamerAttr ) <= 0 )
	{		
		return false;
	}

	if( !para.bRestore && mEnv.excludeC2Linkdown() )
	{
		if( !isImportChannelAvailable( para , streamerAttr.importChannelName) )
		{
			MLOG(ZQ::common::Log::L_ERROR, REQFMT(NgodResourceManager,"allocateResource() do not select streamer[%s] because it's import channel is not available"),
				streamerAttr.netId.c_str() );
			return false;
		}
	}

	if( para.bNeedImportChannel)
	{
		if( evaludateImportChannel( para, streamerAttr ) <= 0 )
		{			
			return false;
		}
	}
	
	//do not add request bandwidth onto import channel bandwidth
	//because we use report bandwidth to evaluate
	streamerAttr.usedBw				+= para.requestBW;
	streamerAttr.usedSessCount		+= 1;

	std::string icStatus;
	if( para.bNeedImportChannel )
	{
		const std::string& icName = streamerAttr.importChannelName;
		ResourceImportChannelAttrMap::iterator itIc = mImportChannles.find( icName );
		if( itIc == mImportChannles.end() )
		{
			return false;//should never happen		
		}
		ResourceImportChannelAttr& icAttr = itIc->second;		
		if( mEnv.importChannelReplicaInTestMode() )
		{//count on import channel in test mode

			icAttr.usedSessCount ++;
			icAttr.usedBw	+= para.requestBW;	
		}
		else if( para.bNeedImportChannel )
		{
			icAttr.reportUsedBW += para.requestBW;
			icAttr.reportUsedSessCount ++;
			std::ostringstream oss;
			oss<<"Import channel status: usedBW["<<icAttr.reportUsedBW <<"] maxBW["<<MIN(icAttr.reportMaxBW, icAttr.confMaxBw)<<"] usedSess["<<icAttr.reportUsedSessCount<<"] maxSess["<<icAttr.confMaxSessCount<<"]";
			icStatus = oss.str();
		}
	}

	MLOG(ZQ::common::Log::L_INFO, REQFMT(NgodResourceManager,"allocateResource() allocate resource for netId[%s] identifier[%s] requestBW[%ld], "
		"now usedBW[%lld] maxBW[%lld] usedSess[%lld] maxSess[%lld] %s"),
		streamerNetId.c_str() , para.identifier.c_str() , para.requestBW , 
		streamerAttr.usedBw ,streamerAttr.maxBw, streamerAttr.usedSessCount, streamerAttr.maxSessCount ,
		icStatus.c_str() );

	return true;
}
void NgodResourceManager::confirmResource( const StreamerResourcePara& para, const std::string& streamerNetId ,bool bLocalPlaylist )
{
	ZQ::common::MutexGuard gd(*this);

	StreamerIterMap streamers = findByIdentifier( para.identifier );

	if( streamers.empty() )
	{
		MLOG(ZQ::common::Log::L_WARNING,REQFMT(NgodResourceManager,"releaseResource() no sop with identifier[%s] is available"), para.identifier.c_str() );
		return ;
	}

	StreamerIterMap::iterator itStreamerIter = streamers.find( streamerNetId );
	if( itStreamerIter == streamers.end() )
	{
		MLOG(ZQ::common::Log::L_ERROR,REQFMT(NgodResourceManager,"releaseResource()  no streamer with netId[%s] identifier[%s] is found"),
			streamerNetId.c_str() , para.identifier.c_str()	);
		return ;
	}

	ResourceStreamerAttrMap::iterator itStreamer = itStreamerIter->second;

	ResourceStreamerAttr& streamerAttr = itStreamer->second;
	streamerAttr.statisticsTotalSessCount ++;
	streamerAttr.statisticsUsedSessCount ++;

	streamerAttr.statisticsRemoteSessCount += ( bLocalPlaylist ? 0 : 1 );
}

void NgodResourceManager::resetStatisticsCounter()
{
	ZQ::common::MutexGuard gd(*this);
	SOPS::iterator itSop = mSops.begin();
	for( ; itSop != mSops.end() ; itSop++ )
	{
		ResourceStreamerAttrMap& streamers = itSop->second;
		ResourceStreamerAttrMap::iterator itStreamer = streamers.begin();
		for( ; itStreamer != streamers.end() ; itStreamer++ )
		{
			ResourceStreamerAttr& attr = itStreamer->second;
			attr.statisticsRemoteSessCount	= 0;
			attr.statisticsFailedSessCount	= 0;
			attr.statisticsTotalSessCount	= 0;
			attr.statisticsUsedSessCount	= 0;
		}
	}

	{
		char szTimeBuffer[128];
		const char* pTime = ZQTianShan::TimeToUTC( ZQTianShan::now() , szTimeBuffer, sizeof(szTimeBuffer) -1 , true );
		mStrMeasuredSince = pTime ? pTime : "";
	}	
}

void NgodResourceManager::releaseResource( const StreamerResourcePara& para , const std::string& streamerNetId  , bool bFailed , int32 penaltyValue )
{
	ZQ::common::MutexGuard gd(*this);

	StreamerIterMap streamers = findByIdentifier( para.identifier );

	if( streamers.empty() )
	{
		MLOG(ZQ::common::Log::L_WARNING,REQFMT(NgodResourceManager,"releaseResource() no sop with identifier[%s] is available"), para.identifier.c_str() );
		return ;
	}

	StreamerIterMap::iterator itStreamerIter = streamers.find( streamerNetId );
	if( itStreamerIter == streamers.end() )
	{
		MLOG(ZQ::common::Log::L_ERROR,REQFMT(NgodResourceManager,"releaseResource() sess[%s] no streamer with netId[%s] identifier[%s] is found"),
			para.sessionId.c_str(), streamerNetId.c_str() , para.identifier.c_str()	);
		return ;
	}

	ResourceStreamerAttrMap::iterator itStreamer = itStreamerIter->second;

	ResourceStreamerAttr& streamerAttr = itStreamer->second;

	streamerAttr.usedBw			-= para.requestBW;
	streamerAttr.usedSessCount  --;
	
	streamerAttr.usedSessCount	= max( streamerAttr.usedSessCount ,  0 );
	streamerAttr.usedBw			= max( streamerAttr.usedBw , 0 );

	if( bFailed )
	{
		streamerAttr.statisticsFailedSessCount ++;
		if( penaltyValue > 0 )
		{
			streamerAttr.penalty = penaltyValue;
			MLOG(ZQ::common::Log::L_INFO,REQFMT(NgodResourceManager,"releaseResource() release resource for netId[%s] identifier[%s] , and apply penalty value to [%ld]"),
				streamerAttr.netId.c_str() , para.identifier.c_str() , penaltyValue );
		}
	}	

	std::string icStatus;
	if( para.bNeedImportChannel  )
	{
		if( mEnv.importChannelReplicaInTestMode() )
		{
			const std::string& icName = streamerAttr.importChannelName;
			ResourceImportChannelAttrMap::iterator itIc = mImportChannles.find( icName );
			if( itIc != mImportChannles.end() )
			{
				ResourceImportChannelAttr& icAttr = itIc->second;
				icAttr.usedSessCount --;
				icAttr.usedBw			-= para.requestBW;;
				icAttr.usedSessCount	= max( icAttr.usedSessCount , 0 );
				icAttr.usedBw			= max( icAttr.usedBw , 0 );				
			}
		}
		else
		{
			const std::string& icName = streamerAttr.importChannelName;
			ResourceImportChannelAttrMap::iterator itIc = mImportChannles.find( icName );
			if( itIc != mImportChannles.end() )
			{
				ResourceImportChannelAttr& icAttr = itIc->second;
				icAttr.reportUsedSessCount  --;
				icAttr.reportUsedBW			-= para.requestBW;;
				icAttr.reportUsedSessCount	= max( icAttr.reportUsedSessCount , 0 );
				icAttr.reportUsedBW			= max( icAttr.reportUsedBW , 0 );
				std::ostringstream oss;
				oss << "Import channel status: usedBW["<<icAttr.reportUsedBW<<"] maxBW["<<icAttr.reportMaxBW<<"] runningSess["<<icAttr.reportUsedSessCount<<"] maxCount["<<icAttr.confMaxSessCount<<"]";
				icStatus = oss.str();
			}
		}
	}

	MLOG(ZQ::common::Log::L_INFO,REQFMT(NgodResourceManager,"releaseResource() release resource to netId[%s] identifier[%s] bandwidth[%ld], now usedBW[%lld] maxBW[%lld] usedSess[%lld] maxSess[%lld] %s"),
		streamerAttr.netId.c_str() , para.identifier.c_str() , para.requestBW ,
		streamerAttr.usedBw , streamerAttr.maxBw , streamerAttr.usedSessCount , streamerAttr.maxSessCount, icStatus.c_str() );

}

bool NgodResourceManager::checkStreamerConfiguration( const StreamerResourcePara& para , const ResourceStreamerAttr& streamerAttr  )
{
	bool bOk = (streamerAttr.streamServicePrx != NULL) ;
	bOk = bOk && ( streamerAttr.bReplicaStatus || mEnv.streamerReplicaInTestMode() ) ;
	bOk = bOk && streamerAttr.bMaintainEnable;
	bOk = bOk && (streamerAttr.penalty <= 0);
	if( para.bRestore )
		bOk = true;//if it is in restore stage , just think the streamer is OK
	
	if( mEnv.streamerReplicaInTestMode() )
	{
		MLOG(ZQ::common::Log::L_WARNING, REQFMT(NgodResourceManager,"checkStreamerConfiguration() test mode" ));
	}
	if( !bOk )
	{
		MLOG(ZQ::common::Log::L_WARNING, REQFMT(NgodResourceManager,"checkStreamerConfiguration() do not select streamer [%s] due to : interface[%s], replicaStatus[%s] maintainStatus[%s] penalty[%d]"),
			streamerAttr.netId.c_str(),
			streamerAttr.streamServicePrx != NULL ? "GOOD":"NULL",
			streamerAttr.bReplicaStatus ? "UP":"DOWN",
			streamerAttr.bMaintainEnable ? "ENABLE":"DISABLE",
			streamerAttr.penalty );
	}
	return bOk;
}

int32 NgodResourceManager::evaluateStreamer( const StreamerResourcePara& para , const ResourceStreamerAttr& streamerAttr )
{
	//calculate streamer weight
	if( (streamerAttr.maxBw < ( streamerAttr.usedBw + para.requestBW )) || (streamerAttr.maxSessCount < (streamerAttr.usedSessCount + 1 )) )
	{
		MLOG(ZQ::common::Log::L_WARNING, REQFMT(NgodResourceManager,"do not select due to insufficient maxbandiwdth: usedBW[%lld] totalBW[%lld] requestBW[%ld] ; usedSess[%lld] maxSess[%lld] "),
			streamerAttr.usedBw , streamerAttr.maxBw, para.requestBW , streamerAttr.usedSessCount , streamerAttr.maxSessCount );
		return 0;
	}
	int streamerBwWeight		= (int)( ( streamerAttr.maxBw - streamerAttr.usedBw ) * MAX_WEIGHT / streamerAttr.maxBw);
	int streamerSessCountWeight	= (int)( ( streamerAttr.maxSessCount - streamerAttr.usedSessCount ) * MAX_WEIGHT / streamerAttr.maxSessCount );
	//take the smaller one
	int streamerWeight			= (int)( streamerBwWeight > streamerSessCountWeight ? streamerSessCountWeight : streamerBwWeight);
	streamerWeight = MAX( streamerWeight , 1 );
	return streamerWeight;
}

bool NgodResourceManager::isImportChannelAvailable( const StreamerResourcePara& para, const std::string& importChannelName )
{
	ZQ::common::MutexGuard gd(*this);
	ResourceImportChannelAttrMap::const_iterator it = mImportChannles.find( importChannelName );
	if( it == mImportChannles.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING, REQFMT(NgodResourceManager,"isImportChannelAvailable() can't find import channel according to name[%s]" ),
			importChannelName.c_str() );
		return false;
	}
	if( mEnv.importChannelReplicaInTestMode() )
	{
		return true;
	}
	else
	{
		const ResourceImportChannelAttr& icAttr = it->second;
		if( icAttr.reportUsedBW >= icAttr.reportMaxBW )
		{
			MLOG(ZQ::common::Log::L_WARNING,REQFMT(NgodResourceManager,"isImportChannelAvailable() import channel[%s], reportUsedBW[%lld] reportMaxBW[%lld] which means import channel is not available"),
				importChannelName.c_str() , icAttr.reportUsedBW , icAttr.reportMaxBW );
			return false;
		}
		else
		{
			return true;
		}
	}
}

int32 NgodResourceManager::evaludateImportChannel( const StreamerResourcePara& para , const ResourceStreamerAttr& streamerAttr )
{
	//do not lock here, outer code must lock *this
	ResourceImportChannelAttrMap::const_iterator it = mImportChannles.find( streamerAttr.importChannelName );
	if( it == mImportChannles.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING, REQFMT(NgodResourceManager,"evaludateImportChannel() can't find import channel according to name[%s]" ),
			streamerAttr.importChannelName.c_str() );
		return 0;
	}
	
	const ResourceImportChannelAttr& icAttr = it->second;
	int64 maxBw			= 0;
	int64 usedBw		= 0;
	int64 maxSess		= 0;
	int64 usedSess		= 0;
	if( mEnv.importChannelReplicaInTestMode() )
	{
		maxBw		= icAttr.confMaxBw ;
		usedBw		= icAttr.usedBw;

		maxSess		= icAttr.confMaxSessCount;
		usedSess	= icAttr.usedSessCount;
	}
	else
	{
		maxBw		= MIN(icAttr.reportMaxBW ,icAttr.confMaxBw );
		usedBw		= icAttr.reportUsedBW;

		maxSess		= icAttr.confMaxSessCount;
		usedSess	= icAttr.reportUsedSessCount;
	}
	if( para.bRestore )
	{
		return MAX_WEIGHT;// in restore stage, take MAX_WEIGHT
	}
	else if( ( maxBw < ( usedBw + para.requestBW )) || ( usedSess >= maxSess) )
	{
		MLOG(ZQ::common::Log::L_WARNING, REQFMT(NgodResourceManager,"evaludateImportChannel() insufficient bandwidth to apply more stream: requestBW[%ld] usedBW[%lld] maxBW[%lld/(report)%lld/(conf)%lld] usedSess[%lld] maxSess[%lld] "),
			para.requestBW , usedBw, maxBw, icAttr.reportMaxBW ,icAttr.confMaxBw,
			usedSess , maxSess );
		return 0;
	}
	int bwWeight	= (int)( ( maxBw-usedBw ) * MAX_WEIGHT / maxBw);
	int sessWeight	= (int)( ( maxSess-usedSess) * MAX_WEIGHT / maxSess );
	int icWeight	= bwWeight > sessWeight ? sessWeight : bwWeight;
	icWeight = MAX( icWeight , 1 );

	return icWeight;
}

void NgodResourceManager::updateReplica(const TianShanIce::Replicas &reps)
{//update replica reported from stream service for streamer and import channel

	//should I lock NgodResourceManager? 
	ZQ::common::MutexGuard gd(*this);
	
	TianShanIce::Replicas::const_iterator itRep = reps.begin();
	
	for( ; itRep != reps.end() ; itRep++ )
	{
		if( stricmp( itRep->category.c_str() , "BandwidthUsage" ) == 0 )
		{
			processImportChannelReplica( *itRep );
		}
		else if ( stricmp( itRep->category.c_str() , "streamer" ) == 0 )
		{
			processStreamerReplica( *itRep );
		}
		else if( stricmp(itRep->category.c_str() , "StreamService") == 0 )
		{
			processStreamServiceReplica(*itRep);
		}
	}
}
void NgodResourceManager::processImportChannelReplica( const TianShanIce::Replica& rep )
{
	int64 usedImportBandwidth = 0;
	int64 totalImportBandwidth = 0;
	int32 runningImportSessCount = 0;

	ZQTianShan::Util::getPropertyDataWithDefault( rep.props, "UsedReplicaImportBandwidth", 0, usedImportBandwidth);
	ZQTianShan::Util::getPropertyDataWithDefault( rep.props, "TotalReplicaImportBandwidth",0, totalImportBandwidth);
	ZQTianShan::Util::getPropertyDataWithDefault( rep.props, "runningImportSessionCount",  0, runningImportSessCount);

	updateImportChannelReplica( rep.groupId , usedImportBandwidth , totalImportBandwidth, runningImportSessCount );
}

void NgodResourceManager::processStreamerReplica( const TianShanIce::Replica& rep )
{
	std::string		streamerId = rep.groupId + "/" + rep.replicaId;
	bool status = rep.replicaState == TianShanIce::stInService;

	if (status)
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodResourceManager,"processStreamerReplica() update streamer[%s]'s status[UP]"), streamerId.c_str());
	else
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NgodResourceManager,"processStreamerReplica() update streamer[%s]'s status: [DOWN]"), streamerId.c_str());

	updateStreamerReplica( streamerId, status);
}

void NgodResourceManager::processStreamServiceReplica( const TianShanIce::Replica& rep )
{
	
	std::string reportingNode;
	ZQTianShan::Util::getPropertyDataWithDefault(rep.props,"reportingNode","",reportingNode);

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(processStreamServiceReplica,"trying to update node[%s] state [%s] from node[%s]"),
		rep.groupId.c_str() , rep.replicaState == TianShanIce::stInService ? "UP":"DOWN" , reportingNode.c_str() );

	std::string groupName = rep.groupId;
	transform(groupName.begin(),groupName.end(), groupName.begin(), tolower); 
		
	ZQ::common::MutexGuard gd(*this);
	StreamerIteratorSetMap::iterator itStreamerSet = mStreamerSymLinks.begin();
	for( ; itStreamerSet != mStreamerSymLinks.end() ; itStreamerSet++ )
	{
		std::string streamerNetId = itStreamerSet->first;
		transform(streamerNetId.begin(),streamerNetId.end(), streamerNetId.begin(), tolower);
		if( streamerNetId.find(groupName) != 0 )
		{
			continue;
		}		
		updateStreamerReplica( itStreamerSet->first , rep.replicaState == TianShanIce::stInService );
	}
}

int NgodResourceManager::run( )
{
	
	mbCheckThreadQuit = false;
	
	int32 updateInterval		= mEnv.replicaUpdateInterval();//convert second to millisecond
	updateInterval				= max(10,updateInterval);

	int32 defaultWaitInterval	= 10 * 60 * 1000;
	int32 waitInterval			= defaultWaitInterval;

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodResourceManager,"run() replica checker start with interval[%d]"),updateInterval);

	while( !mbCheckThreadQuit )
	{
		waitInterval = defaultWaitInterval;

		{
			ZQ::common::MutexGuard gd(*this);
			
			Ice::Long curTime = ZQTianShan::now();

			StreamerIteratorSetMap::iterator itSymLink = mStreamerSymLinks.begin();
			for( ; itSymLink != mStreamerSymLinks.end() ; itSymLink++ )
			{
				StreamerIteratorSet& iterSet = itSymLink->second;
				if( iterSet.size() > 0 )
				{
					ResourceStreamerAttrMap::iterator itStreamerAttr = *iterSet.begin();
					Ice::Long lastUpdate = itStreamerAttr->second.lastReplicaUpdate;
					bool bStatus = itStreamerAttr->second.bReplicaStatus;
					if( bStatus && ( lastUpdate + updateInterval ) < curTime ) 
					{
						char szTimeBuf[128];
						szTimeBuf[sizeof(szTimeBuf)-1] = 0;
						ZQ::common::TimeUtil::TimeToUTC( lastUpdate, szTimeBuf, sizeof(szTimeBuf)-1 , true );
						MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NgodResourceManager,"streamer[%s] has not been updated since [%s], mark its status to [DOWN], interval[%d]"),
							itSymLink->first.c_str(), szTimeBuf, updateInterval);
						updateStreamerReplica( itSymLink->first , false );
					}
					else
					{
						waitInterval = (int32)(( lastUpdate + updateInterval ) - curTime);
					}
				}
			}
			waitInterval = waitInterval < 10 ? 10 :waitInterval;
			mCond.wait( *this, (timeout_t)waitInterval);
		}
		
	}
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodResourceManager,"run() replica checker stopped"),updateInterval);
	return 0;
}

void NgodResourceManager::stop( )
{
	mbCheckThreadQuit = true;
	mCond.signal();
	waitHandle(100*1000);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
ElementInfo::ElementInfo()
{
	cueIn			= 0;
	cueOut			= 0;		
	primaryElement	= false;
}

ContentCachingStatus ElementInfo::getStatus( const std::string& volumeNetId ) const
{
	VolumeNameAttr attr ;
	attr.netId = volumeNetId;
	std::map<VolumeNameAttr,ContentCachingStatus>::const_iterator it = mContentStatus.find( attr );
	if( it == mContentStatus.end() )
		return STATUS_CONTENT_CACHING_NULL;
	return it->second;
}

std::string	ElementInfo::getImportingNode( const std::string& volumeNetId ) const
{
	VolumeNameAttr attr ;
	attr.netId = volumeNetId;
	std::map<VolumeNameAttr,std::string>::const_iterator it = mImportingNode.find( attr );
	if( it == mImportingNode.end() )
		return std::string("");
	return it->second;
}

bool ElementInfo::isImportingInVolume( const std::string& volumeNetId ) const
{
	VolumeNameAttr attr ;
	attr.netId = volumeNetId;
	std::map<VolumeNameAttr , std::string>::const_iterator it = mImportingNode.find( attr );
	if( it == mImportingNode.end() )
		return false;
	return !it->second.empty();
}

std::string ElementInfo::fullContentName( const std::string& volumeName ) const
{
	VolumeNameAttrS::const_iterator it = volumeAttr.begin();
	for( ; it != volumeAttr.end() ; it++ )
	{
		if( it->netId == volumeName )
		{
			return std::string("/") + ( it->partition.empty() ? "$" : it->partition ) + "/" + name;
		}
	}
	return std::string("/$/") + name;
}

#ifdef ZQ_OS_LINUX
	#define  stricmp strcasecmp
#endif

#define CONTENTSTATUSSWITCH()		if(0){
#define CONTENTSTATUSCASE(x)		} else if( stricmp(x,str.c_str() ) == 0 ) {
#define CONTENTSTATUSENDSWITCH()	}

ContentCachingStatus parseContentCachingStatus( const std::string& str )
{
	CONTENTSTATUSSWITCH()
		CONTENTSTATUSCASE("NotProvisioned")
			return STATUS_CONTENT_CACHING_NONE;
		CONTENTSTATUSCASE("Provisioning")
			return STATUS_CONTENT_CACHING_NONE;
		CONTENTSTATUSCASE("ProvisioningStreamable")
			return STATUS_CONTENT_CACHING_IMPORTING;
		CONTENTSTATUSCASE("InService")
			return STATUS_CONTENT_CACHING_LOCAL;
		CONTENTSTATUSCASE("OutService")
			return STATUS_CONTENT_CACHING_LOCAL;
	CONTENTSTATUSENDSWITCH()
	return STATUS_CONTENT_CACHING_NULL;
}
void ElementInfo::updateContentStatus( const std::map<std::string,std::string>& contentAttributes )
{
	static const char* strReplicaState		= "replicaStateOfVol_";
	static const char* strImportEdgeNode	= "ImportEdgeNodeOfVol_";
	static size_t iSizeReplicaState			= strlen( strReplicaState );
	static size_t iSizeImportNode			= strlen( strImportEdgeNode );
	static size_t sizeArray[]				= { iSizeReplicaState , iSizeImportNode };
	static const char* strArray[]			= { strReplicaState , strImportEdgeNode };
	static int count = sizeof(strArray)/sizeof(strArray[0]);

	std::map<std::string,std::string>::const_iterator it = contentAttributes.begin();
	for( ; it != contentAttributes.end() ; it ++ )
	{
		const char* p = NULL;
		int iSelect = -1;
		 int i = 0;
		for( i = 0 ;i < count ; i ++ )
		{
			p = strstr( it->first.c_str() , strArray[i]);
			
			if( p == NULL ) 
				continue;
			iSelect = i;
			p += sizeArray[i];
			break;
		}
		if( !(p && p[0]) || ( i >= count) ) 
			continue;

		VolumeNameAttr name;
		parseVolumeName( p , name);	
		switch ( iSelect )
		{
		case 0://replica state
			{				
				mContentStatus[ name ] = parseContentCachingStatus( it->second );
			}
			break;
		case 1://import edge node
			{
				mImportingNode[ name ] = it->second;
			}
			break;
		default:
			//ignore
			break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
/// SessionStatCache
SessionStatCache::SessionStatCache( ) {
}

SessionStatCache::~SessionStatCache( ) {
	//do not free memory, let os reclaim it
}

bool SessionStatCache::addStat( SessionStat* stat ) {

	ZQ::common::MutexGuard gd(mLocker);

	if( mStreamSess2Stats.find(stat->streamSessId) != mStreamSess2Stats.end() )
		return false;
	if( mOndemand2Stats.find( stat->ondemandSessId) != mOndemand2Stats.end() )
		return false;
	if( mRtspSess2Stats.find( stat->rtspSessId) != mRtspSess2Stats.end() )
		return false;

	mStreamSess2Stats[stat->streamSessId] = stat;
	mOndemand2Stats[stat->ondemandSessId] = stat;
	mRtspSess2Stats[stat->rtspSessId] = stat;
	if( mStreamer2Stat.find(stat->streamerNetId) == mStreamer2Stat.end() ) {
		STATSET ss;
		ss.insert( stat );
		mStreamer2Stat[stat->streamerNetId] = ss;
	} else {
		mStreamer2Stat[stat->streamerNetId].insert(stat);
	}
	stat->lastTouch = ZQ::common::now();

	return true;
}
void SessionStatCache::removeStat( const std::string& rtspSess ) {
	ZQ::common::MutexGuard gd(mLocker);
	SESS2STATS::iterator it = mRtspSess2Stats.find( rtspSess);
	if( it == mRtspSess2Stats.end() )
		return;
	SessionStat* stat = it->second;
	if(!stat)
		return;
	mRtspSess2Stats.erase(it);
	mStreamSess2Stats.erase(stat->streamSessId);
	mOndemand2Stats.erase(stat->ondemandSessId);
	STREAMER2STAT::iterator itStreamer2Stat = mStreamer2Stat.find(stat->streamerNetId);
	if( itStreamer2Stat != mStreamer2Stat.end() ) {
		itStreamer2Stat->second.erase(stat);		
	}
	delete stat;
}
void SessionStatCache::statChanged( const std::string& streamSess, int64 offset, float scale, TianShanIce::Streamer::StreamState state ) {
	ZQ::common::MutexGuard gd(mLocker);
	SESS2STATS::iterator it = mStreamSess2Stats.find(streamSess);
	if( it == mStreamSess2Stats.end() )
		return;
	SessionStat* stat = it->second;
	stat->timeOffset = offset;
	stat->scale = scale;
	stat->state = state;
	stat->lastTouch = ZQ::common::now();
}

SessionStatCache::STATSET SessionStatCache::getCachedStatusForStreamer( const std::string& streamer ) {
	STATSET ss;
	{
		ZQ::common::MutexGuard gd(mLocker);
		STREAMER2STAT::const_iterator it = mStreamer2Stat.find( streamer );
		if( it == mStreamer2Stat.end() )
			return ss;
		ss= it->second; // make a copy, this will reduce the block time on other thread which is accessing the stat cache
	}
	int64 timenow = ZQ::common::now();
	STATSET::iterator itStat = ss.begin();
	for( ; itStat != ss.end() ; itStat++ ) {
		SessionStat* stat = *itStat;
		//assert( stat ! = 0 );
		if( stat->lastTouch == 0 )
			continue;
		if( stat->state != TianShanIce::Streamer::stsStreaming )
			continue;// nothing should be ajusted
		if( stat->lastTouch >= timenow )
			continue;
		if( fabs(stat->scale - 0.01) <= 0.01 ) 
			continue;
		stat->timeOffset += (int64)(stat->scale * ( timenow - stat->lastTouch ));
		stat->lastTouch = timenow;
	}
	return ss;
}