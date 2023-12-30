#include "ContextImpl.h"
#include "NGODEnv.h"
#include "SessionRenewCmd.h"
#include <TianShanDefines.h>
#include <fstream>

#include <time.h>

#ifdef _DEBUG
	#include <iostream>
	using namespace std;
#endif

namespace NGODr2c1
{

	ContextImpl::ContextImpl(NGODEnv& env) : _env(env)
	{
	#ifdef _DEBUG
		cout<<"create context"<<endl;
	#endif
	}

	ContextImpl::~ContextImpl()
	{
	#ifdef _DEBUG
		cout<<"delete context"<<endl;
	#endif
	}

	void ContextImpl::addEventRecord(const ::NGODr2c1::SessionEventRecord& eventRecord, const ::Ice::Current& cur)
	{
		WLock lk(*this);
		data.sessionEvents.push_back(eventRecord);
	}

	void ContextImpl::getState(::NGODr2c1::ctxData& state, const ::Ice::Current& /*= ::Ice::Current()*/)  const
	{
		RLock lk(*this);	
		
		state = data;

		return;
	}

	void ContextImpl::renew(::Ice::Long ttl, const ::Ice::Current& c)
	{
		WLock lk(*this);
		data.expiration = ZQTianShan::now() + ttl;		
	}

	void ContextImpl::increaseAnnSeq(const ::Ice::Current& c)
	{
		WLock lk(*this);

		data.announceSeq++;
	}

	void ContextImpl::onTimer(const ::Ice::Current& c)
	{
		::NGODr2c1::ctxData tmp;

		{
			RLock lk(*this);
			tmp = data;
		}

		::Ice::Long timeout = tmp.expiration - ZQTianShan::now();
		_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContextImpl, "onTimer() Session[%s]; timeout=%lld"), tmp.ident.name.c_str(), timeout);

		bool bObjExist = true;
		if (timeout > 0)			
		{			
			_env._pSessionWatchDog->watchSession(tmp.ident, (long) timeout);
			//ping stream
			try
			{
				//
				TianShanIce::Streamer::StreamPrx stream = TianShanIce::Streamer::StreamPrx::uncheckedCast(_env._pCommunicator->stringToProxy(tmp.streamFullID));
				if(stream)
				{
					try
					{
						stream->ice_ping();
					}
					catch( const Ice::ObjectNotExistException& )
					{
						_env._fileLog(ZQ::common::Log::L_WARNING,
							CLOGFMT(ContextImpl, "Session[%s];streamId[%s] failed to query object on stream service , session will be tearminated"),
							tmp.ident.name.c_str(),
							tmp.streamFullID.c_str() );
						bObjExist = false;
					}
					catch( const Ice::Exception& )
					{//nothing
					}
				}
			}
			catch( const Ice::Exception&  )
			{
				_env._fileLog(ZQ::common::Log::L_WARNING,
					CLOGFMT(ContextImpl, "Session[%s];streamId[%s] failed to query object on stream service"),
					tmp.ident.name.c_str(),
					tmp.streamFullID.c_str() );
				bObjExist = false;
			}
			if(bObjExist)
				return;
		}

		// increase announce sequence number
		if (_ngodConfig._announce._useGlobalCSeq == 0)
		{
			WLock lk(*this);
			data.announceSeq++;
			tmp.announceSeq++;
		}
		
		if ( bObjExist && timeout > - (Ice::Long) (_ngodConfig._rtspSession._timeout) * 1000 * (_ngodConfig._rtspSession._timeoutCount))
		{			
			_env.sessionInProgressAnnounce(tmp);
			_env._pSessionWatchDog->watchSession(tmp.ident, (long) (_ngodConfig._rtspSession._timeout) * 1000);
		}		
		else // ??óDrenewê±??ì???á?￡?Dèòa?ú?ù2￠·￠?íSession Terminated announcement
		{		
			_env.terminatAndAnnounce(tmp); //!! Rename this func to terminateAndAnnounce()			
		}			
	}

	::TianShanIce::Streamer::StreamPrx ContextImpl::getStream(const ::Ice::Current& c/*= ::Ice::Current()*/) const
	{
		RLock lk(*this);
		if (NULL != _streamPrx)
		{		
			return _streamPrx;
		}		
		
		((ContextImpl*)this)->_streamPrx = ::TianShanIce::Streamer::StreamPrx::uncheckedCast(_env._pCommunicator->stringToProxy(data.streamFullID));			
		
		return _streamPrx;
	}

	void ContextImpl::updateCtxProp(const ::std::string& key, const ::std::string& val, const ::Ice::Current& c)
	{
		WLock lk(*this);
		data.prop[key] = val;
	}

	::std::string ContextImpl::getCtxPropItem(const ::std::string& key, const ::Ice::Current& c) const
	{
		RLock lk(*this);
		::TianShanIce::Properties::const_iterator iter = data.prop.find(key);
		if (iter != data.prop.end())
			return iter->second;
		else
			return "";
	}


	// --------------------------------------------------------------------------------------
	// SessionView implementation
	// --------------------------------------------------------------------------------------
	SessionViewImpl::SessionViewImpl(NGODEnv& env) : _env(env), _nextClientId(0)
	{
	}

	SessionViewImpl::~SessionViewImpl()
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
		return _ngodConfig._rtspSession._timeout;
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
			ZQ::common::MutexGuard lk(_env._contextEvtrLock);
			INOUTMAP inoutMap;
			::Freeze::EvictorIteratorPtr tItor = _env._pContextEvtr->getIterator("", MAX_SESSION_CONTEXT);
			while (tItor->hasNext())
			{
				Ice::Identity ident = tItor->next();
				NGODr2c1::ctxData NewContext;
				NGODr2c1::ContextPrx pNewContextPrx = NULL;
				if (_env.openContext(ident.name, NewContext, pNewContextPrx, inoutMap))
				{
					pViewData->_all.push_back(NewContext);
				}
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
		return pViewData->_all.size();
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
		std::vector< ::NGODr2c1::ctxData>::const_iterator itor = pViewData->_all.begin();
		for (; itor != pViewData->_all.end(); itor ++)
		{
			const ::NGODr2c1::ctxData& data = *itor;
			if (data.groupID == sessionGroup)
			{
				pViewData->_someGroup.push_back(data);
			}
		}

		pViewData->_lastAccessTime = ZQTianShan::now();

		return pViewData->_someGroup.size();
	}
	
	void SessionViewImpl::resetCounters(const ::Ice::Current& ) 
	{
		NGODEnv::resetCounters();
	}
	
	void SessionViewImpl::getImportChannelUsage( NGODr2c1::ImportChannelUsageS& usages, const ::Ice::Current& )
	{
		//NGODEnv::getImportChannelUsage( usages );
		extern NGODEnv ssmNGOD;
		ResourceImportChannelAttrMap ics;
		ssmNGOD.mResManager.getImportChannelData(ics);
		ResourceImportChannelAttrMap::const_iterator itIc = ics.begin();
		for( ; itIc != ics.end() ; itIc++ )
		{
			 const ResourceImportChannelAttr& attr = itIc->second;
			 ::NGODr2c1::ImportChannelUsage usage;
			 usage.channelName			= attr.netId;
			 usage.runningSessCount		= (Ice::Int)attr.reportUsedSessCount;
			 usage.totalImportBandwidth	= attr.reportMaxBW;
			 usage.usedImportBandwidth	= attr.reportUsedBW;
			 
			 usages.push_back(usage);
		}
	}

	void SessionViewImpl::getNgodUsage(::NGODr2c1::NgodUsage& usage, ::std::string& strMeasuredSince, const ::Ice::Current& /*= ::Ice::Current()*/)
	{
		SOPS sops;
// 		std::map< std::string , NGOD2::SOPRestriction::SopHolder > sopRes;
// 		sopRes.clear( );
// 		NGODEnv::getNgodUsage( sopRes , strMeasuredSince );
		extern NGODEnv ssmNGOD;
		ssmNGOD.mResManager.getSopData( sops , strMeasuredSince );
		
		usage.clear( );

		SOPS::const_iterator itSop = sops.begin( );
		for (  ; itSop != sops.end() ; itSop++  )
		{
			NGODr2c1::SopUsage					outSopUsage;
			
			NGODr2c1::StreamerUsageS& outStreamerInfos = outSopUsage.streamerUsageInfo;
			
			outSopUsage.servieGroupId			= 0;//not used any longer

			const ResourceStreamerAttrMap& streamerInfos = itSop->second;

			ResourceStreamerAttrMap::const_iterator itStreamerInfo = streamerInfos.begin( );
			for (  ; itStreamerInfo != streamerInfos.end() ; itStreamerInfo ++ ) 
			{
				::NGODr2c1::StreamerUsage strmerUsage;
				strmerUsage.penaltyValue		=	itStreamerInfo->second.penalty;
				strmerUsage.streamerNetId		=	itStreamerInfo->second.netId;
				strmerUsage.streamerEndpoint	=	itStreamerInfo->second.endpoint;
				strmerUsage.attachedVolumeName	=	itStreamerInfo->second.volumeNetId;
				strmerUsage.importChannelName	=	itStreamerInfo->second.importChannelName;
				strmerUsage.totalBandwidth		=	itStreamerInfo->second.maxBw;
				strmerUsage.usedBandwidth		=	itStreamerInfo->second.usedBw;
				strmerUsage.maxStreamCount		=	(Ice::Int)itStreamerInfo->second.maxSessCount;
				strmerUsage.usedStreamCount		=	(Ice::Int)itStreamerInfo->second.usedSessCount;
				strmerUsage.available			=	itStreamerInfo->second.bReplicaStatus;
				strmerUsage.maintenanceEnable   =   itStreamerInfo->second.bMaintainEnable;

				strmerUsage.usedSession         =   itStreamerInfo->second.statisticsUsedSessCount;
				strmerUsage.failedSession       =   itStreamerInfo->second.statisticsFailedSessCount;
				strmerUsage.histCountRemoteSess	=	itStreamerInfo->second.statisticsRemoteSessCount;
				strmerUsage.histCountTotalSess	=	itStreamerInfo->second.statisticsTotalSessCount;
				
				outStreamerInfos.push_back(strmerUsage);
			}
			usage.insert(NGODr2c1::NgodUsage::value_type( itSop->first ,outSopUsage ));
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

		extern NGODEnv ssmNGOD;
		
		ZQ::common::MutexGuard gd(ssmNGOD.mResManager);

		SopMap streamersOfSop;
		groupBySop(streamerNames, streamersOfSop);
		SopMap::const_iterator itSop = streamersOfSop.begin();
		for( ; itSop != streamersOfSop.end() ; itSop++ )
		{
			 const std::vector<std::string>& streamerNetIds = itSop->second;
			 std::vector<std::string>::const_iterator itStreamer = streamerNetIds.begin();
			 for( ;itStreamer != streamerNetIds.end() ; itStreamer++ )
			 {
				 ssmNGOD.mResManager.maintainEnable( itSop->first , *itStreamer  ,enable );
			 }
		}
//		std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sops = _sopConfig._sopRestrict._sopDatas;
//		std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator itSop;
//		SopMap streamersOfSop;
//		groupBySop(streamerNames, streamersOfSop);
//		SopMap::iterator sopMapIter = streamersOfSop.begin();
//		for (; sopMapIter != streamersOfSop.end(); sopMapIter++) 
//		{
//			itSop = sops.find(sopMapIter->first);
//			if (itSop != sops.end())
//			{
//				NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;			
//				std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;
//
//				std::vector<std::string>::iterator streamerIdIter = sopMapIter->second.begin();
//				for (; streamerIdIter != sopMapIter->second.end(); streamerIdIter++) 
//				{
//					std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin();
//					for (; itStreamer != streamers.end(); itStreamer++)
//					{
//						if (itStreamer->_netId == *streamerIdIter)
//						{
//#pragma message(__MSGLOC__"TODO:Why set replicaStatus here????")
//							//itStreamer->_bReplicaStatus = enable;
//							itStreamer->_enabled = (enable == true ? 1 : 0);
//							_env._fileLog(ZQ::common::Log::L_INFO, CLOGFMT(SessionViewImpl, "enableStreamers() : streamer[%s] is enabled=[%d]"), itStreamer->_netId.c_str(), itStreamer->_enabled);
//						}
//					} // end for (streamer)
//				} // end for (stream names)
//			} // check if sop exist
//		} // end for (sop)
		updateSOPXML(_ngodConfig._sopProp.fileName);
	}

	void SessionViewImpl::updateSOPXML(const std::string strFile)
	{
		std::ofstream sopOut;
		sopOut.open(strFile.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!sopOut)
		{
			_env._fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionViewImpl, "updateSOPXML() : Fail to open sop file at [%s] for updating"), strFile.c_str());
			return;
		}
		sopOut << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		       << "<!-- Configurations on SOP restrictions:\n"
               << "       attributes:\n"
		       << "         enableWarmup - if this configuration is turned on(set it to 1) ,NGOD2 will connect to streamer when it startup\n"
			   << "                        turn it off if you do not want it to connect to streamer when plugin startup\n"
			   << "         retryCount - the max number of the retries if failed to setup streams\n"
			   << "         maxPenaltyValue - the penalty to apply if an SOP failed to setup a stream\n"
			   << "-->\n"
			   << "<SOPRestriction enableWarmup=\"" << _sopConfig._sopRestrict._enableWarmup << "\" retryCount=\"" << _sopConfig._sopRestrict._retryCount << "\" maxPenaltyValue=\"" << _sopConfig._sopRestrict._maxPenaltyValue << "\" streamerQueryInterval=\"" << _sopConfig._sopRestrict._streamerQueryInterval << "\" replicaUpdateInterval=\"" << _sopConfig._sopRestrict._replicaUpdateInterval << "\" >\n"
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
               << "         totalBW : total bandwidth can be used by this streamer , and counted in Kbps\n"
               << "         maxStream : max srteam count can be used by this streamer\n"
			   << "         import channel: name of import channel bind to this streamer\n"
			   << "-->\n";
		extern NGODEnv ssmNGOD;
		
		SOPS sopDatas;
		std::string measureSince;

		ssmNGOD.mResManager.getSopData( sopDatas , measureSince );
		SOPS::const_iterator itSop = sopDatas.begin();
		for( ; itSop != sopDatas.end() ; itSop++ )
		{
			sopOut << "  <sop name=\"" << itSop->first << "\" >\n";
			const ResourceStreamerAttrMap& streamers = itSop->second;
			ResourceStreamerAttrMap::const_iterator itStreamer = streamers.begin();
			for( ; itStreamer != streamers.end() ; itStreamer++ )
			{
				sopOut	<< "    <streamer netId=\"" << itStreamer->second.netId
						<< "\" serviceEndpoint=\"" << itStreamer->second.endpoint 
						<<  "\" volume=\"" << itStreamer->second.volumeNetId
						<< "\" totalBW=\"" << itStreamer->second.maxBw
						<< "\" maxStream=\"" << itStreamer->second.maxSessCount
						<< "\" importChannel=\"" << itStreamer->second.importChannelName 
						<< "\" enabled=\"" << (itStreamer->second.bMaintainEnable ? 1 : 0) << "\" />\n";
			}
		}

		
// 		std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sops = _sopConfig._sopRestrict._sopDatas;
// 		std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator itSop = sops.begin();		
// 		for( ; itSop != sops.end() ; itSop++ )
// 		{
// 			sopOut << "  <sop name=\"" << itSop->second._name << "\" serviceGroup=\"" << itSop->second._serviceGroup << "\" >\n";
// 			NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;			
// 			std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;				
// 			std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin ();		
// 			for ( ; itStreamer != streamers.end() ; itStreamer ++ )
// 			{
// 				sopOut << "    <streamer netId=\"" << itStreamer->_netId << "\" serviceEndpoint=\"" << itStreamer->_serviceEndpoint <<  "\" volume=\"" << itStreamer->_volume << "\" totalBW=\"" << itStreamer->_totalBW << "\" maxStream=\"" << itStreamer->_maxStream << "\" importChannel=\"" << itStreamer->_importChannel << "\" enabled=\"" << itStreamer->_enabled << "\" />\n";
// 			}
// 			sopOut << "  </sop>\n";
// 		}
		sopOut << "</SOPRestriction>\n";
		sopOut.flush();
		sopOut.close();
	}
}

