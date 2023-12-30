
#include "NgodEnv.h"
#include "NgodConfig.h"
#include "SOPConfig.h"
#include "SessionViewImpl.h"
#include "NgodDatabase.h"
#include "NgodSessionManager.h"
#include "SelectionResourceManager.h"

namespace NGOD
{

SessionViewImpl::SessionViewImpl( NgodEnv& env , NgodSessionManager& sessManager )
:mEnv(env),
mSessManager(sessManager)
{
}

SessionViewImpl::~SessionViewImpl(void)
{
	clearResource();
}

void SessionViewImpl::clearResource()
{
	IceUtil::RecMutex::Lock lk(_lockMap);
	std::map<Ice::Int, ViewData*>::iterator itor;
	for (itor = _viewDataMap.begin(); itor != _viewDataMap.end(); itor ++)
	{
		try {delete (itor->second);} catch (...) {}
	}
	_viewDataMap.clear();
}

::Ice::Int SessionViewImpl::getTimeoutValue(const ::Ice::Current& c)
{
	return ngodConfig.rtspSession.timeout/1000;//count in millisecond , so translate it to second
}
CtxDatas SessionViewImpl::getRangeBySG(::Ice::Int from, ::Ice::Int to, const ::std::string& sessionGroup, const ::Ice::Current& c)
{
	IceUtil::RecMutex::Lock lk(*this);
	ViewData* pViewData = NULL;
	CtxDatas retCtxs;

	Ice::Context::const_iterator ctxItor = c.ctx.end();
	ctxItor = c.ctx.find("ctx#ClientId");
	if (ctxItor == c.ctx.end())
	{
		TianShanIce::ClientError ex;
		ex.category = "SessionView";
		ex.message = "No ClientId in method call";
		ex.ice_throw();
	}
	Ice::Int lClientId = atoi(ctxItor->second.c_str());

	{
		IceUtil::RecMutex::Lock lk(_lockMap);
		std::map<Ice::Int, ViewData*>::const_iterator viewDataItor = _viewDataMap.end();
		viewDataItor = _viewDataMap.find(lClientId);
		if (viewDataItor == _viewDataMap.end())
		{
			TianShanIce::ClientError ex;
			ex.category = "SessionView";
			ex.message = "ClientId not found, client maybe timeout";
			ex.ice_throw();
		}
		pViewData = viewDataItor->second;
	}

	if (!pViewData->_someGroup.size() || pViewData->_someGroup[0].groupID != sessionGroup)
		getContextsBySG(sessionGroup,c);

	CtxDatas& groupCtxs = pViewData->_someGroup;
	if (from < 1)
		from = 1;
	while (from <= to && from <= (int) groupCtxs.size())
	{
		retCtxs.push_back(groupCtxs[from - 1]);
		from ++;
	}

	pViewData->_lastAccessTime = ZQTianShan::now();

	return retCtxs;
}

CtxDatas SessionViewImpl::getRange(::Ice::Int from, ::Ice::Int to, const ::Ice::Current& c)
{
	IceUtil::RecMutex::Lock lk(*this);
	ViewData* pViewData = NULL;
	CtxDatas retCtxs;

	Ice::Context::const_iterator ctxItor = c.ctx.end();
	ctxItor = c.ctx.find("ctx#ClientId");
	if (ctxItor == c.ctx.end())
	{
		TianShanIce::ClientError ex;
		ex.category = "SessionView";
		ex.message = "No ClientId in method call";
		ex.ice_throw();
	}
	Ice::Int lClientId = atoi(ctxItor->second.c_str());

	{
		IceUtil::RecMutex::Lock lk(_lockMap);
		std::map<Ice::Int, ViewData*>::const_iterator viewDataItor = _viewDataMap.end();
		viewDataItor = _viewDataMap.find(lClientId);
		if (viewDataItor == _viewDataMap.end())
		{
			TianShanIce::ClientError ex;
			ex.category = "SessionView";
			ex.message = "ClientId not found, client maybe timeout";
			ex.ice_throw();
		}
		pViewData = viewDataItor->second;
	}

	CtxDatas& allCtxs = pViewData->_all;
	if (from < 1)
		from = 1;
	while (from <= to && from <= (int) allCtxs.size())
	{
		retCtxs.push_back(allCtxs[from - 1]);
		from ++;
	}

	pViewData->_lastAccessTime = ZQTianShan::now();

	return retCtxs;
}

::Ice::Int SessionViewImpl::getAllContexts(::Ice::Int& clientId, const ::Ice::Current& c)
{
	IceUtil::RecMutex::Lock lk(*this);

	ViewData* pViewData = new ViewData();
	pViewData->_lastAccessTime = ZQTianShan::now();
	pViewData->_all.clear();
	pViewData->_someGroup.clear();

	{
		NgodSessionPrxS allSess = mSessManager.getDatabase().openAllSessions();
		for( NgodSessionPrxS::iterator it = allSess.begin() ; it != allSess.end() ;it++)
		{
			NGOD::ctxData data;
			(*it)->convertToCtxData( data );
			pViewData->_all.push_back( data );
		}		
	}

	// _nextClientId初始值为0,所以第一个clientId为1
	// clientId作为Client的后续请求标识
	clientId = ++ _nextClientId;

	// 加入map
	{
		IceUtil::RecMutex::Lock lk(_lockMap);
		_viewDataMap[clientId] = pViewData;
	}

	// 返回session context的数目
	return (int)pViewData->_all.size();
}

::Ice::Int SessionViewImpl::getContextsBySG(const ::std::string& sessionGroup, const ::Ice::Current& c)
{
	IceUtil::RecMutex::Lock lk(*this);
	ViewData* pViewData = NULL;

	Ice::Context::const_iterator ctxItor = c.ctx.end();
	ctxItor = c.ctx.find("ctx#ClientId");
	if (ctxItor == c.ctx.end())
	{
		TianShanIce::ClientError ex;
		ex.category = "SessionView";
		ex.message = "No ClientId in method call";
		ex.ice_throw();
	}
	Ice::Int lClientId = atoi(ctxItor->second.c_str());

	{
		IceUtil::RecMutex::Lock lk(_lockMap);
		std::map<Ice::Int, ViewData*>::const_iterator viewDataItor = _viewDataMap.end();
		viewDataItor = _viewDataMap.find(lClientId);
		if (viewDataItor == _viewDataMap.end())
		{
			TianShanIce::ClientError ex;
			ex.category = "SessionView";
			ex.message = "ClientId not found, client maybe timeout";
			ex.ice_throw();
		}
		pViewData = viewDataItor->second;
	}

	pViewData->_someGroup.clear();
	std::vector< ::NGOD::ctxData>::const_iterator itor = pViewData->_all.begin();
	for (; itor != pViewData->_all.end(); itor ++)
	{
		const ::NGOD::ctxData& data = *itor;
		if (data.groupID == sessionGroup)
		{
			pViewData->_someGroup.push_back(data);
		}
	}

	pViewData->_lastAccessTime = ZQTianShan::now();

	return (int)pViewData->_someGroup.size();
}

void SessionViewImpl::resetCounters(const ::Ice::Current& ) 
{
	mEnv.getSelResManager().resetStatisticsCounter();	
}

void SessionViewImpl::getImportChannelUsage( NGOD::ImportChannelUsageS& usages, const ::Ice::Current& )
{
	//NGODEnv::getImportChannelUsage( usages );
	ResourceImportChannelAttrMap ics;
	mEnv.getSelResManager().getImportChannelData( ics );
	
	ResourceImportChannelAttrMap::const_iterator it = ics.begin();
	for( ; it != ics.end() ; it ++ )
	{
		NGOD::ImportChannelUsage usage;
		usage.channelName				= it->first;
		usage.runningSessCount			= (Ice::Int)it->second.reportUsedSessCount;
		usage.totalImportBandwidth		= MIN(it->second.reportMaxBW,it->second.confMaxBw);
		usage.usedImportBandwidth		= it->second.reportUsedBW;
		usages.push_back( usage );
	}
}

void SessionViewImpl::getNgodUsage(::NGOD::NgodUsage& usage, ::std::string& strMeasuredSince, const ::Ice::Current& /*= ::Ice::Current()*/)
{
	SOPS sopRes;

	mEnv.getSelResManager().getSopData( sopRes , strMeasuredSince );	

	usage.clear( );
	SOPS::const_iterator itSop = sopRes.begin( );
	for (  ; itSop != sopRes.end() ; itSop++  )
	{
		NGOD::SopUsage	outSopUsage;
		//typedef std::map< std::string , ResourceStreamerAttr > ResourceStreamerAttrMap
		const ResourceStreamerAttrMap& sopData		= itSop->second;

		NGOD::StreamerUsageS& outStreamerInfos		= outSopUsage.streamerUsageInfo;
		outSopUsage.servieGroupId					= 0; //sopData.serviceGroup;

		ResourceStreamerAttrMap::const_iterator itStreamerInfo = sopData.begin( );
		for (  ; itStreamerInfo != sopData.end() ; itStreamerInfo ++ ) 
		{
			const ResourceStreamerAttr& streamer = itStreamerInfo->second;
			
			::NGOD::StreamerUsage strmerUsage;

			strmerUsage.penaltyValue		=	streamer.penalty;
			strmerUsage.streamerNetId		=	streamer.netId;
			strmerUsage.streamerEndpoint	=	streamer.endpoint;
			strmerUsage.attachedVolumeName	=	streamer.volumeNetId;
			strmerUsage.importChannelName	=	streamer.importChannelName;
			strmerUsage.totalBandwidth		=	streamer.maxBw;
			strmerUsage.usedBandwidth		=	streamer.usedBw;
			strmerUsage.maxStreamCount		=	(Ice::Int)streamer.maxSessCount;
			strmerUsage.usedStreamCount		=	(Ice::Int)streamer.usedSessCount;
			strmerUsage.available			=	streamer.bReplicaStatus ? 1 : 0;
			strmerUsage.maintenanceEnable   =   streamer.bMaintainEnable;

			strmerUsage.usedSession         =   streamer.statisticsUsedSessCount;
			strmerUsage.failedSession       =   streamer.statisticsFailedSessCount;
			strmerUsage.histCountRemoteSess	=	streamer.statisticsRemoteSessCount;
			strmerUsage.histCountTotalSess	=	streamer.statisticsTotalSessCount;

			outStreamerInfos.push_back(strmerUsage);
		}
		usage.insert(NGOD::NgodUsage::value_type( itSop->first ,outSopUsage ));
	}
}

void SessionViewImpl::groupBySop(const TianShanIce::StrValues& streamerNames, SopMap& streamersOfSop)
{
	char delimeter = 0x06;

	SopMap::iterator sopMapIter;
	std::string strFullName;
	std::string sopName;
	std::string streamerId;
	size_t position;

	TianShanIce::StrValues::const_iterator streamerNameIter = streamerNames.begin();
	for (; streamerNameIter != streamerNames.end(); streamerNameIter++)
	{
		strFullName = *streamerNameIter;
		position = strFullName.find(delimeter);
		sopName = strFullName.substr(0, position);
		streamerId = strFullName.substr(position+1);
		sopMapIter = streamersOfSop.find(sopName);
		if (sopMapIter != streamersOfSop.end())
		{
			(sopMapIter->second).push_back(streamerId);
		}
		else
		{
			std::vector<std::string> initVec;
			initVec.push_back(streamerId);
			streamersOfSop.insert(std::make_pair(sopName, initVec));
		}
	}
}

void SessionViewImpl::enableStreamers(const TianShanIce::StrValues &streamerNames, bool enable, const Ice::Current &cur)
{
	IceUtil::Mutex::Lock lock(_fileLock);
	if (streamerNames.empty())
	{
		return;
	}
	std::map< std::string , NGOD::SOPRestriction::SopHolder >& sops = sopConfig.sopRestrict.sopDatas;
	std::map< std::string , NGOD::SOPRestriction::SopHolder >::iterator itSop;

	SopMap streamersOfSop;
	groupBySop(streamerNames, streamersOfSop);
	SopMap::iterator sopMapIter = streamersOfSop.begin();
	for (; sopMapIter != streamersOfSop.end(); sopMapIter++) 
	{
		itSop = sops.find(sopMapIter->first);
		if (itSop != sops.end())
		{
			NGOD::SOPRestriction::SopHolder& sopData = itSop->second;
			std::vector<NGOD::Sop::StreamerHolder>& streamers = sopData.streamerDatas;

			std::vector<std::string>::iterator streamerIdIter = sopMapIter->second.begin();
			for (; streamerIdIter != sopMapIter->second.end(); streamerIdIter++) 
			{
				std::vector<NGOD::Sop::StreamerHolder>::iterator itStreamer = streamers.begin();
				for (; itStreamer != streamers.end(); itStreamer++)
				{
					if (itStreamer->netId == *streamerIdIter)
					{
						itStreamer->enabled = (enable == true ? 1 : 0);
						mEnv.getSelResManager().maintainEnable(itSop->first,itStreamer->netId,enable);
						MLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionViewImpl, "enableStreamers() : streamer[%s] is enabled=[%d]"), itStreamer->netId.c_str(), itStreamer->enabled);
					}
				} // end for (streamer)
			} // end for (stream names)
		} // check if sop exist
	} // end for (sop)
	updateSOPXML( ngodConfig.sopProp.fileName);
}

void SessionViewImpl::updateSOPXML(const std::string strFile)
{
	std::ofstream sopOut;
	sopOut.open(strFile.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!sopOut)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SessionViewImpl, "updateSOPXML() : Fail to open sop file at [%s] for updating"), strFile.c_str());
		return;
	}

#define XMLSS_ATTR_EXPRESSION(_VAR, _VAL) #_VAR "=\"" << _VAL << "\" "
#define XMLSS_SOPREST_EXPRESSION(_VAR)    XMLSS_ATTR_EXPRESSION(_VAR, sopConfig.sopRestrict._VAR)
	sopOut << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<!-- Configurations on SOP restrictions:\n"
		<< "       attributes:\n"
		<< "         enableWarmup - if this configuration is turned on(set it to 1) ,NGOD2 will connect to streamer when it startup\n"
		<< "                        turn it off if you do not want it to connect to streamer when plugin startup\n"
		<< "         retryCount - the max number of the retries if failed to setup streams\n"
		<< "         maxPenaltyValue - the penalty to apply if an SOP failed to setup a stream\n"
        << "         penaltyOfTimeout	- penalty value to be used when encounter an icetimeout exception, this value shoud <= maxPenaltyValue\n"
		<< "         penaltyMask		- this mask is used to define which operation may cause a penalty\n"
		<< "         					The Mask value are list below:\n"
		<< "         					  PLAY			:	1\n"
		<< "         					  PAUSE			:	2\n"
		<< "         					  GET_PARAMETER	:	4\n"    							
		<< "         					These values can be combined , for example , if you want to enable penalty in PLAY and PAUSE, \n"
		<< "         					set penaltyEnableMask to 3 which is 1 + 2\n"
		<< "-->\n"
		<< "<SOPRestriction " 
		<< XMLSS_SOPREST_EXPRESSION(enableWarmup)
		<< XMLSS_SOPREST_EXPRESSION(retryCount)
		<< XMLSS_SOPREST_EXPRESSION(maxPenaltyValue)
		<< XMLSS_ATTR_EXPRESSION(penaltyOfTimeout, sopConfig.sopRestrict.timeoutPenalty)
		<< XMLSS_ATTR_EXPRESSION(penaltyMask,      sopConfig.sopRestrict.penaltyEnableMask)
		<< XMLSS_SOPREST_EXPRESSION(execMask)
		<< XMLSS_SOPREST_EXPRESSION(streamerQueryInterval)
		<< XMLSS_SOPREST_EXPRESSION(replicaUpdateInterval)
		<< ">\n"

		//<< "\" checkReplicaUpdateInfo=\"" << _sopConfig._sopRestrict._replicaUpdateEnable << "\" enableReportedImportChannel=\"" << _sopConfig._sopRestrict._enableReportedImportChannelBandWidth << "\" >\n"
		<< "<!-- Configurations on a SOP:\n"
		<< "       attributes:\n"
		<< "         name - the name of SOP that refer to the NGOD ODRM's configuration\n"
		<< "         serviceGroup - to specify a service group if this SOP is picked\n"
		<< "-->\n"
		<< "<!-- Configurations on streamer:\n"
		<< "       attributes:\n"
		<< "         netId : target streamer's netid\n"
		<< "         serviceEdnpoint: endpoint of streamservice bind to this streamer\n"
		<< "         volume : volume name \n"
		<< "         totalBW : total abandwidth can be used by thi streamer , and counted in Kbps\n"
		<< "         maxStream : max srtea count can be used by this streamer\n"
		<< "         import channel: name of import channel bind to this streamer\n"
		<< "-->\n";
	std::map< std::string , NGOD::SOPRestriction::SopHolder >& sops = sopConfig.sopRestrict.sopDatas;
	std::map< std::string , NGOD::SOPRestriction::SopHolder >::iterator itSop = sops.begin();		
	for( ; itSop != sops.end() ; itSop++ )
	{
		sopOut	<< "  <sop "
			<< XMLSS_ATTR_EXPRESSION(name,         itSop->second.name)
			<< XMLSS_ATTR_EXPRESSION(serviceGroup, itSop->second.serviceGroup)
			<< XMLSS_ATTR_EXPRESSION(sopGroup,     itSop->second.sopGroupName)
			<< ">\n";

		NGOD::SOPRestriction::SopHolder& sopData = itSop->second;			
		std::vector<NGOD::Sop::StreamerHolder>& streamers = sopData.streamerDatas;				
		std::vector<NGOD::Sop::StreamerHolder>::iterator itStreamer = streamers.begin ();		
		for ( ; itStreamer != streamers.end() ; itStreamer ++ )
		{
			sopOut << "    <streamer "
				<< XMLSS_ATTR_EXPRESSION(netId,           itStreamer->netId)
				<< XMLSS_ATTR_EXPRESSION(serviceEndpoint, itStreamer->serviceEndpoint)
				<< XMLSS_ATTR_EXPRESSION(volume,          itStreamer->volume)
				<< XMLSS_ATTR_EXPRESSION(totalBW,         itStreamer->totalBW)
				<< XMLSS_ATTR_EXPRESSION(maxStream,       itStreamer->maxStream)
				<< XMLSS_ATTR_EXPRESSION(importChannel,   itStreamer->importChannel)
				<< XMLSS_ATTR_EXPRESSION(enabled,         itStreamer->enabled)
				<< "/>\n";
		}

		sopOut << "  </sop>\n";
	}
	sopOut << "</SOPRestriction>\n";
	sopOut.flush();
	sopOut.close();

}
}
