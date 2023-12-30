// File Name : CRGSessionImpl.cpp
#include <fstream>
#include "CRGSessionImpl.h"

#include "OpenVBOConfig.h"

#include "StreamersConfig.h"

#include "Environment.h"

#include "SelectionResourceManager.h"

#include "StreamEventSinkI.h"

#include <TianShanIceHelper.h>

#include "CRGSessionManager.h"



extern "C" {
#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif
}

#define TIMESTAMP_SESSION_FIRST_DESTROY SYS_PROP(SessFirstDestroy)

namespace EventISVODI5
{

CRGSessionImpl::CRGSessionImpl(ZQ::common::Log& fileLog, Environment& env)
:_fileLog(fileLog), _env(env)
{
	announceSeq = 1;
	sessStatusFlag = 0;
}

CRGSessionImpl::~CRGSessionImpl()
{

}

std::string CRGSessionImpl::getAnnounceSeq(const ::Ice::Current& current)
{
	WLock wLock(*this);
	if (INT_MAX == announceSeq)
	{
		announceSeq = 1;
	}
	char buff[20];
	buff[sizeof(buff) - 1] = '\0';
	snprintf(buff, sizeof(buff) - 1, "%d", announceSeq++);
	return buff;
}

void CRGSessionImpl::OnTimer(const ::Ice::Current& current)
{
	WLock wLock(*this);
	::Ice::Long timeout = expiration - ZQTianShan::now();
	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(CRGSessionImpl, "Session current expiration[%d]"), timeout);

	Ice::Long destroyTime = 0;
	ZQTianShan::Util::getPropertyDataWithDefault(metadata,TIMESTAMP_SESSION_FIRST_DESTROY,0,destroyTime);
	if( destroyTime > 0 )
	{//destroy the session and it's stream
		destroy(Ice::Current());
		return;
	}

	bool bExist = true;
	if (timeout > 0)
	{
		_env.watchSession(ident, (long)timeout);
		try
		{
			if (stream)
			{
				stream->ice_ping();
			}
		}
		catch(const Ice::ObjectNotExistException&)
		{
			_fileLog(ZQ::common::Log::L_WARNING, CLOGFMT(CRGSessionImpl, "Session[%s];streamId[%s] failed to query object on stream service , session will be tearminated"), ident.name.c_str(), streamId.c_str() );
			bExist = false;
		}
		catch( const Ice::Exception& )
		{
			//nothing
		}
		if (bExist)
		{
			return ;
		}
	}

	StreamEventSinkIPtr streamEvent = StreamEventSinkIPtr::dynamicCast(_env.getStreamEventSink());
	Ice::Long sessionTimeout = -(_openVBOConfig._rtspSession._timeout * 1000 * _openVBOConfig._rtspSession._timeoutCount);
	if (bExist && timeout > sessionTimeout)
	{
		// send session in progress to STB
		if (streamEvent)
		{
			streamEvent->sendANNOUNCE_SessionInProgress(*this);
		}
		_env.watchSession(ident);
	}
	else
	{
		if (streamEvent)
		{
			streamEvent->sendANNOUNCE_Terminated(*this);
		}
		destroy();		
	}
}

void CRGSessionImpl::renew(::Ice::Long ttl, const ::Ice::Current& current)
{
	WLock lk(*this);
	expiration = ZQTianShan::now() + ttl;
}

::TianShanIce::Properties CRGSessionImpl::getMetaData(const ::Ice::Current& current) const
{
	RLock rLock(*this);
	TianShanIce::Properties sessionContext = metadata;
	sessionContext.insert(std::make_pair(SESSION_META_DATA_REQUEST_URL, requestURL));
	sessionContext.insert(std::make_pair(SESSION_META_DATA_STREAMER_SOURCE, streamerSource));
	sessionContext.insert(std::make_pair(SESSION_META_DATA_STREAMER_NET_ID, streamerNetId));
	sessionContext.insert(std::make_pair(SESSION_META_DATA_STREAM_ID, streamId));
	sessionContext.insert(std::make_pair(SESSION_META_DATA_STB_CONNECTION_ID, STBConnectionID));
	return sessionContext;
}

::TianShanIce::Streamer::StreamPrx CRGSessionImpl::getStream(::std::string& strStreamerNetId, ::std::string& strStreamId, const Ice::Current &current) const
{
	RLock rLock(*this);
	strStreamerNetId = streamerNetId;
	strStreamId = streamId;
	return stream;
}


void CRGSessionImpl::setMetaData(const TianShanIce::Properties &newMetaData, const Ice::Current &current)
{
	WLock wLock(*this);
	TianShanIce::Properties::iterator iter;
	TianShanIce::Properties::const_iterator constIter = newMetaData.begin();
	for (; constIter != newMetaData.end(); constIter++)
	{
		iter = metadata.find(constIter->first);
		if (iter != metadata.end())
		{
			// update 
			iter->second = constIter->second;
		}
		else
		{
			// insert
			metadata.insert(std::make_pair(constIter->first, constIter->second));
		}
	} // end for
}

void CRGSessionImpl::attachStream(const ::TianShanIce::Streamer::StreamPrx& streamPrx, const ::Ice::Current& current)
{
	WLock wLock(*this);
	stream = streamPrx;
}

bool CRGSessionImpl::destroy(const Ice::Current &current)
{
	WLock wLock(*this);
	try
	{
		if (stream)
		{
			stream->destroy();
			stream = NULL;
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessionImpl, "successfully destroyed stream[%s]"), streamId.c_str());
		}
		else
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessionImpl, "stream[%s] NULL"), streamId.c_str());
	}
	catch( const Ice::ObjectNotExistException& )
	{
		_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(CRGSessionImpl, "stream[%s] caught ObjectNotExistException"), streamId.c_str());
		stream = NULL;
	}
	catch (const Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessionImpl, "caught %s when destroying stream[%s]"), ex.ice_name().c_str(), streamId.c_str());
		Ice::Long destroyTime = 0 ;
		ZQTianShan::Util::getPropertyDataWithDefault(metadata, TIMESTAMP_SESSION_FIRST_DESTROY, 0, destroyTime);		
		if (destroyTime <= 0 )
		{
			destroyTime = ZQ::common::now();
			ZQTianShan::Util::updatePropertyData(metadata, TIMESTAMP_SESSION_FIRST_DESTROY, destroyTime);
		}

		Ice::Long delta = ZQ::common::now() - destroyTime;
		if( delta < _openVBOConfig._rtspSession._maxDestroyInterval * 1000 )
		{
			_env.watchSession(ident.name, 10000);
			return false;
		}

		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessionImpl, "destroying stream has retried [%lld]ms, continue to terminate the session"), delta);
	}

	// delete session in storage
	std::string _strLastError;
	_env.getSessionManager().removeSession( ident.name, _strLastError);

	// destroy client session
	if (_env.getStreamSmithSite().destroyClientSession(ident.name.c_str()))
		_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessionImpl, "rtsp sess[%s] destroyed"), ident.name.c_str());

	return true;
}

int CRGSessionImpl::getSessStatusFlags(const Ice::Current &current)
{
	RLock rLock(*this); 
	return sessStatusFlag;
}

void CRGSessionImpl::setSessStatusFlags(int flag, const Ice::Current &current)
{
	WLock lk(*this);
	sessStatusFlag |= flag;
}

void CRGSessionImpl::clearSessStatusFlags( const Ice::Current &current)
{
	WLock lk(*this);
	sessStatusFlag &= ~sessStatusFlag;
}

OpenVBOServantImpl::OpenVBOServantImpl(ZQ::common::Log& fileLog, Environment& env)
:_fileLog(fileLog), _env(env)
{

}

OpenVBOServantImpl::~OpenVBOServantImpl()
{

}

SsmOpenVBO::StreamersStatisticsWithStamp OpenVBOServantImpl::getStreamerInfos(const ::Ice::Current& current)
{
	SOPS sops;
	SsmOpenVBO::StreamersStatisticsWithStamp statistics;
	_env.getResourceManager().getSopData(sops, statistics.stampMeasuredSince);
	for (SOPS::iterator itSop = sops.begin(); itSop != sops.end(); itSop++)
	{
		ResourceStreamerAttrMap& streamerInfos = itSop->second;
		ResourceStreamerAttrMap::iterator itStreamerInfo = streamerInfos.begin();
		for (; itStreamerInfo != streamerInfos.end(); itStreamerInfo++)
		{
			SsmOpenVBO::StreamerStatistics streamerInfo;
			streamerInfo.penaltyValue		= itStreamerInfo->second.penalty;
            streamerInfo.streamerSource     = itStreamerInfo->second.sourceAddr;
			streamerInfo.streamerNetId		= itStreamerInfo->second.netId;
			streamerInfo.streamerEndpoint	= itStreamerInfo->second.endpoint;
			streamerInfo.attachedVolumeName	= itStreamerInfo->second.volumeNetId;
			streamerInfo.importChannelName	= itStreamerInfo->second.importChannelName;
			streamerInfo.totalBandwidth		= itStreamerInfo->second.maxBw;
			streamerInfo.usedBandwidth      = itStreamerInfo->second.usedBw;
			streamerInfo.maxStreamCount     = (Ice::Int)itStreamerInfo->second.maxSessCount;
			streamerInfo.usedStreamCount    = (Ice::Int)itStreamerInfo->second.usedSessCount;
			streamerInfo.available          = itStreamerInfo->second.bReplicaStatus;
			streamerInfo.adminEnabled       = itStreamerInfo->second.bMaintainEnable;
			streamerInfo.usedSessions       = itStreamerInfo->second.statisticsUsedSessCount;
			streamerInfo.failedSessions     = itStreamerInfo->second.statisticsFailedSessCount;
			streamerInfo.remoteSessions     = itStreamerInfo->second.statisticsRemoteSessCount;
            streamerInfo.localSessions = itStreamerInfo->second.statisticsTotalSessCount - streamerInfo.remoteSessions;
			statistics.streamersInfos.push_back(streamerInfo);
		}
	}
	return statistics;
}

SsmOpenVBO::ImportChannelsStatisticsWithStamp OpenVBOServantImpl::getImportChannelStat(const ::Ice::Current& current)
{
	SsmOpenVBO::ImportChannelsStatisticsWithStamp statistics;
	ResourceImportChannelAttrMap ics;
	_env.getResourceManager().getImportChannelData(ics);
	ResourceImportChannelAttrMap::const_iterator itIc = ics.begin();
	for (; itIc != ics.end(); itIc++)
	{
		SsmOpenVBO::ImportChannelStatistics importChannelInfos;
		const ResourceImportChannelAttr& attr = itIc->second;
		importChannelInfos.channelName			= attr.netId;
		importChannelInfos.runningSessCount		= (Ice::Int)attr.reportUsedSessCount;
		importChannelInfos.totalImportBandwidth	= attr.reportMaxBW;
		importChannelInfos.usedImportBandwidth	= attr.reportUsedBW;
		statistics.importChannelsInfos.push_back(importChannelInfos);
	}
	return statistics;
}

void OpenVBOServantImpl::enableStreamers(const ::TianShanIce::StrValues& streamerNames, bool adminEnabled, const ::Ice::Current& current)
{
	if (streamerNames.empty())
	{
		return;
	}
	NgodResourceManager& resourceManager = _env.getResourceManager();
	ZQ::common::MutexGuard gd(resourceManager);
	TianShanIce::StrValues::const_iterator iter = streamerNames.begin();
	for (; iter != streamerNames.end(); iter++)
	{
		resourceManager.maintainEnable("test", *iter, adminEnabled);
	}
	updateSourceXML(_openVBOConfig._sourceStreamers._fileName);
}

void OpenVBOServantImpl::updateSourceXML(const std::string& strFile)
{
	IceUtil::Mutex::Lock lock(_fileLock);
	std::ofstream out;
	out.open(strFile.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!out)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(OpenVBOServantImpl, "updateSourceXML() : Fail to open sop file at [%s] for updating"), strFile.c_str());
		return;
	}
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "	<!-- Configurations on StreamingResource: \n"
        << "		attributes: \n"
	    << "			enableWarmup - if this configuration is turned on(set it to 1), \n"
		<< "			ssm_OpenVBO will connect to streamer when it startup\n"
		<< "			turn it off if you do not want it to connect to streamer when plugin startup\n"
		<< "			retryCount - [0, 100] the max number of the retries if failed to setup streams\n"
		<< "			maxPenaltyValue - [0, 10000] the penalty to apply if failed to setup a stream\n"
		<< "			replicaUpdateTimeout - in msec, to specify how often the StreamService should report the status of replica\n"
		<< "	-->\n"
	    << "<StreamingResource enableWarmup=\"" << _streamersConfig._streamingResource._enableWarmup << "\" retryCount=\"" << _streamersConfig._streamingResource._retryCount << "\" maxPenaltyValue=\"" << _streamersConfig._streamingResource._maxPenaltyValue << "\" replicaUpdateTimeout=\"" << _streamersConfig._streamingResource._replicaUpdateTimeout << "\" >\n";

	out << "	<!-- Configurations on streamer: \n "
        << "		attributes:\n"
	    << "		netId            - the netId of streamer aggregated by the ssm_OpenVBO \n"
		<< "		serviceEdnpoint  - endpoint to the StreamsService that controls this Streamer\n"
		<< "		volume           - to indicate the local volume that this \n"
		<< "		totalBW          - total bandwidth in Kbps that this Streamer can supply \n"
		<< "		maxStream        - max stream count that this Streamer can supply \n"
		<< "		importChannel    - name of importChannel thru that this streamer can cache the contents from a remote storage\n"
		<< "	-->\n";

	SOPS sopDatas;
	std::string measureSince;
	_env.getResourceManager().getSopData( sopDatas , measureSince );
	SOPS::const_iterator itSop = sopDatas.begin();
	for( ; itSop != sopDatas.end() ; itSop++ )
	{
		const ResourceStreamerAttrMap& streamers = itSop->second;
		ResourceStreamerAttrMap::const_iterator itStreamer = streamers.begin();
		for( ; itStreamer != streamers.end() ; itStreamer++ )
		{
//     <Streamer source="10.15.10.50" netId="SEACnnnnn-N0/Spigot00" serviceEndpoint="StreamSmith:tcp -h SEACnnnnn-N0 -p 10700" volume="CL2/*" totalBW="700000000" maxStream="10000" importChannel="SEACnnnnn-Ny_PG" adminEnabled ="1" />
			out << "	<Streamer netId=\"" << itStreamer->second.netId
				<< "\" source=\"" << itStreamer->second.sourceAddr
				<< "\" serviceEndpoint=\"" << itStreamer->second.endpoint 
				<<  "\" volume=\"" << itStreamer->second.volume
				<< "\" totalBW=\"" << itStreamer->second.maxBw/1000
				<< "\" maxStream=\"" << itStreamer->second.maxSessCount
				<< "\" importChannel=\"" << itStreamer->second.importChannelName 
				<< "\" adminEnabled=\"" << (itStreamer->second.bMaintainEnable ? 1 : 0) << "\" />\n";
		}
	}
	out << "</StreamingResource>\n";
	out.flush();
	out.close();
}

void OpenVBOServantImpl::resetCounters(const ::Ice::Current& current)
{
	_env.getResourceManager().resetStatisticsCounter();
}

void OpenVBOServantImpl::updateReplica_async(const TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr &callback, 
											 const TianShanIce::Replicas &reps, 
											 const Ice::Current &current)
{
	_env.getResourceManager().updateReplica(reps);
	try
	{
		callback->ice_response(_streamersConfig._streamingResource._replicaUpdateTimeout); // unit is second
	}
	catch(...)
	{

	}
}

} // end EventISVODI5
