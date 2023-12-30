// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.

// Branch: $Name:BcastPublishPointImpl.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastPublishPointImpl.cpp $
// 
// 22    3/03/16 2:12p Dejian.fei
// 
// 21    3/03/16 8:55a Li.huang
// 
// 20    2/26/16 5:00p Li.huang
// fix bug 22376
// 
// 19    1/11/16 5:21p Dejian.fei
// 
// 18    7/22/15 2:09p Li.huang
// 
// 17    7/14/15 10:28a Li.huang
// 
// 16    7/13/15 1:00p Li.huang
// 
// 15    7/07/15 4:37p Li.huang
// fix bug 21518
// 
// 14    3/05/15 10:35a Li.huang
// 
// 13    3/04/15 6:57p Li.huang
// 
// 12    11/19/14 10:01a Li.huang
// 
// 11    11/18/14 11:06a Li.huang
// 
// 10    11/18/14 10:29a Li.huang
// 
// 9     11/17/14 3:47p Li.huang
// 
// 8     11/14/14 9:42a Li.huang
// 
// 7     10/21/14 3:32p Li.huang
// 
// 6     10/17/14 3:37p Li.huang
// 
// 5     6/03/14 9:30a Li.huang
// 
// 4     5/30/14 4:43p Li.huang
// 
// 3     5/30/14 3:55p Li.huang
// 
// 2     12/12/13 1:35p Hui.shao
// %lld
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 6     10-03-19 11:46 Li.huang
// remove evictor lock
// 
// 5     09-07-06 9:48 Li.huang
// 
// 4     09-06-30 15:37 Li.huang
// 
// 3     09-06-30 11:31 Li.huang
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "BroadCastChannelEnv.h"
#include "BcastPublishPointImpl.h"
#include "BcastChDef.h"
#include "BcastChRequest.h"
#include "BroadcastChCfg.h"
#include "ChannelItemAssocImpl.h"
#include "TimeUtil.h"
#include "urlstr.h"
#include "IPathHelperObj.h"
#undef max
#include <boost/regex.hpp>
#include <algorithm>
#include "soapMRT.nsmap"
#include "MRTStreamService.h"
extern ZQ::common::Config::Loader<BroadcastChCfg> gBroadcastChCfg;
#define BcastChPPLog(_C, _X) CLOGFMT(_C, "[%s] " _X), ident.name.c_str()

extern ZQ::common::Config::Loader<BroadcastChCfg> gBroadcastChCfg;

Ice::Long UserGetCurrentTime()
{
  time_t lTime;
  time(&lTime);
  return (Ice::Long)lTime;
}
namespace ZQBroadCastChannel
{ 

BcastPublishPointImpl::BcastPublishPointImpl(BroadCastChannelEnv& bcastChenv):
_env(bcastChenv)
{
}

BcastPublishPointImpl::~BcastPublishPointImpl(void)
{
}
::std::string
BcastPublishPointImpl::getType(const Ice::Current& current)const
{
	RLock sync(*this);
	return type;
}

::std::string
BcastPublishPointImpl::getName(const Ice::Current& current)const
{
	RLock sync(*this);
	return ident.name;
}

::std::string
BcastPublishPointImpl::getDesc(const Ice::Current& current)const
{
	RLock sync(*this);
	return desc;
}

::Ice::Int
BcastPublishPointImpl::getMaxBitrate(const Ice::Current& current)const
{
	RLock sync(*this);

	return maxBitrate;
}

void
BcastPublishPointImpl::setMaxBitrate(::Ice::Int newMaxBitrate, const Ice::Current& current)
{
	WLock sync(*this);

	if (newMaxBitrate)
	{
		maxBitrate = newMaxBitrate;
	}
	else
	{
		maxBitrate = gBroadcastChCfg.DefaultChannelMaxBitrate;
	}

	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "setMaxBitrate to %d"), newMaxBitrate);	
}

void
BcastPublishPointImpl::setProperties(const TianShanIce::Properties& newProps, const Ice::Current& current)
{
	WLock sync(*this);
	properties.clear();
	properties = newProps;
}

void
BcastPublishPointImpl::setDesc(const ::std::string& description,
												 const Ice::Current& current)
{
	WLock sync(*this);
	desc = description;	
}

TianShanIce::Properties
BcastPublishPointImpl::getProperties(const Ice::Current& current)const
{
	RLock sync(*this);
	return properties;
}

void
BcastPublishPointImpl::destroy(const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "remove channel"));

	{  
		WLock sync(*this);

		///�Ƶ�MRTStream��Destory��
/*		if(gBroadcastChCfg.soapMRTCfg.enable && bInService && getStatusMRTStream())
		{
			tearDownMRTStream();
		}
*/
		try
		{
			_env._evitBcastChannelPublishPoint->remove(ident);
		}
		catch(const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_800, CLOGFMT(BcastPublishPointImpl, "remove channel [%s] from evictor caught %s: %s"), 
				ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}

		try
		{
			if(session && bInService)
				session->destroy();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,
				"destroy session caught %s:%s"),ex.ice_name().c_str(), ex.message.c_str() );
		}
		catch (Ice::Exception ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,"destroy session caught %s"), ex.ice_name().c_str() );
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,"destroy session caught unknown exception %d"), SYS::getLastErr());
		}
		session = NULL;
	}

	_env._watchDog.removeBcastChannel(ident);
	destroyChannelItemAssoc();

	if(!bIsNOVDSupCh)
		destroyChannelItem();

	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "channel removed"));
}

void
BcastPublishPointImpl::restrictReplica(const TianShanIce::StrValues& contentStoreNetIds,
														 const Ice::Current& current)
{
	WLock sync(*this);
	replicas = contentStoreNetIds;
	for (uint i = 0, count = contentStoreNetIds.size(); i < count; i ++)
	{
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "restrict channel on netId [%s]"), contentStoreNetIds[i].c_str());
	}
}

TianShanIce::StrValues
BcastPublishPointImpl::listReplica(const Ice::Current& current)const
{
	RLock sync(*this);
	return replicas;
}

::Ice::Long
BcastPublishPointImpl::requireResource(TianShanIce::SRM::ResourceType type,
										const TianShanIce::SRM::Resource& res,
										const Ice::Current& current)
{
	WLock sync(*this);
	if(bInService)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_701, CLOGFMT(BcastPublishPointImpl,
			"[%s]channel is already playing, please stop it first"),ident.name.c_str());  
	}
	//resources.insert(TianShanIce::SRM::ResourceMap::value_type(type, res));

	char szBuf[1024];
	memset(szBuf, 0, 1024);
	snprintf(szBuf, sizeof(szBuf) - 1, "[%s]requireResource ",ident.name.c_str());
	ZQTianShan::dumpValueMap(res.resourceData, szBuf, dumpLine);

	resources[type] = res;

	return res.resourceData.size();
}

TianShanIce::SRM::ResourceMap
BcastPublishPointImpl::getResourceRequirement(const Ice::Current& current)const
{
	RLock sync(*this);
	return resources;
}

void
BcastPublishPointImpl::withdrawResourceRequirement(TianShanIce::SRM::ResourceType type,
													const Ice::Current& current)
{
	WLock sync(*this);	
	TianShanIce::SRM::ResourceMap::iterator itor;
	itor = resources.find(type);
	if(itor != resources.end())
		resources.erase(itor);
}
void
BcastPublishPointImpl::setup(const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "enter setup Bcast PublishPoint"));

	int64 lstart = ZQ::common::now();
	if(!_env.connectWeiwoo())
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_330, 
			BcastChPPLog(BcastPublishPointImpl, "failed to connect weiwoo service"));
		return;
	}
	TianShanIce::Streamer::StreamPrx streamprx;
    std::string sessionId;
	try
	{			
		TianShanIce::SRM::Resource assetUriRes;
		TianShanIce::Variant varURI;

		///create weiwoo session
		assetUriRes.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
		assetUriRes.status = TianShanIce::SRM::rsRequested;
		varURI.bRange = false;
		varURI.type = TianShanIce::vtStrings;
		varURI.strs.push_back(gBroadcastChCfg.BcastRtspURL);
		assetUriRes.resourceData["uri"] = varURI;

		TianShanIce::SRM::SessionPrx sessionprx = _env._sessManager->createSession(assetUriRes);

		sessionId = sessionprx->getId();

		//add ServiceGroupID and deliveryId(for MRT)
		TianShanIce::Variant varSGId;
		varSGId.type = TianShanIce::vtInts;
		varSGId.bRange = false;
		varSGId.ints.push_back(gBroadcastChCfg.groupId);

		TianShanIce::Variant varDeliveryId;
		varDeliveryId.type = TianShanIce::vtStrings;
		varDeliveryId.bRange = false;
		varDeliveryId.strs.push_back(ident.name);

		if(resources.find(TianShanIce::SRM::rtServiceGroup) == resources.end())
		{
			TianShanIce::SRM::Resource resSG;
			resSG.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
			resSG.status = TianShanIce::SRM::rsRequested;
			resSG.resourceData["id"] = varSGId;

			if(gBroadcastChCfg.mrtStreamServiceCfg.enable)
			{
				resSG.resourceData["deliveryId"] = varDeliveryId;
			}
			MAPSET(TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtServiceGroup,resSG);
		}
		else
		{
			if(resources[TianShanIce::SRM::rtServiceGroup].resourceData.find("id") == resources[TianShanIce::SRM::rtServiceGroup].resourceData.end() ||
				resources[TianShanIce::SRM::rtServiceGroup].resourceData["id"].ints.empty())
			{
				resources[TianShanIce::SRM::rtServiceGroup].resourceData["id"] = varSGId;
			}

			if(gBroadcastChCfg.mrtStreamServiceCfg.enable)
			{
				
				resources[TianShanIce::SRM::rtServiceGroup].resourceData["deliveryId"] = varDeliveryId;
			}
		}

		//add rtTsDownstreamBandwidth
		TianShanIce::Variant varDB;
		varDB.type = TianShanIce::vtLongs;
		varDB.bRange = false;
		varDB.lints.push_back(maxBitrate);
		if(resources.find(TianShanIce::SRM::rtTsDownstreamBandwidth) == resources.end())
		{
			TianShanIce::SRM::Resource resDB;
			resDB.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
			resDB.status = TianShanIce::SRM::rsRequested;
			resDB.resourceData["bandwidth"] = varDB;
			MAPSET(TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtTsDownstreamBandwidth, resDB);
		}
		else if(resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.find("bandwidth") == resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.end() ||
			resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"].lints.empty())
		{
			resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"] = varDB;
		}

		if(gBroadcastChCfg.mrtStreamServiceCfg.enable)
		{
			std::string streams = gBroadcastChCfg.netId + "/*";
			TianShanIce::Variant varStreamers;
			varStreamers.type = TianShanIce::vtStrings;
			varStreamers.bRange = false;
			varStreamers.strs.clear();
			varStreamers.strs.push_back(streams);

			TianShanIce::SRM::ResourceMap::iterator itorResMap = resources.find(TianShanIce::SRM::rtStreamer);
			if(itorResMap == resources.end())
			{
				TianShanIce::SRM::Resource resStreamer;
				resStreamer.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
				resStreamer.status = TianShanIce::SRM::rsRequested;
				resStreamer.resourceData["NetworkId"] = varStreamers;
				MAPSET(TianShanIce::SRM::ResourceMap, resources, TianShanIce::SRM::rtStreamer, resStreamer);
			}
			else
			{
				TianShanIce::SRM::Resource& resStreamer = itorResMap->second;
				resStreamer.resourceData["NetworkId"] = varStreamers;
			}

			glog(ZQ::common::Log::L_INFO,BcastChPPLog(BcastPublishPointImpl, "setup() [%s]Restrict: streamerIds[%s]"), sessionId.c_str(),streams.c_str());
		}

		glog(ZQ::common::Log::L_INFO,BcastChPPLog(BcastPublishPointImpl, "setup()session [%s]add resource"), sessionId.c_str());

		TianShanIce::SRM::ResourceMap::iterator itorRsMap;
		for(itorRsMap = resources.begin(); itorRsMap != resources.end(); itorRsMap++)
		{
			sessionprx->addResourceEx(itorRsMap->first, itorRsMap->second);
		}

		///set privateData to weiwoo
		glog(ZQ::common::Log::L_INFO,BcastChPPLog(BcastPublishPointImpl, "setup()[%s]session add privateData"), sessionId.c_str());
		TianShanIce::Variant varBcastPPName;
		varBcastPPName.type = TianShanIce::vtStrings;
		varBcastPPName.bRange = false;
		varBcastPPName.strs.push_back(ident.name);
		sessionprx->setPrivateData(RESKEY_BcastPPName, varBcastPPName);

		/// session provison
		glog(ZQ::common::Log::L_INFO,BcastChPPLog(BcastPublishPointImpl, "setup()[%s]session provision"), sessionId.c_str());
		sessionprx->provision();

		glog(ZQ::common::Log::L_INFO,BcastChPPLog(BcastPublishPointImpl, "setup()[%s]session serve"), sessionId.c_str());
		sessionprx->serve();

		glog(ZQ::common::Log::L_INFO,BcastChPPLog(BcastPublishPointImpl, "setup()[%s]session renew(%d)ms"), sessionId.c_str(), _env._sessionRewTime);
		sessionprx->renew(_env._sessionRewTime); 
		_env._watchDog.watchBcastChannel(ident, _env._sessionRewTime - 10000);

		streamprx = sessionprx->getStream();
	    
		playlist = TianShanIce::Streamer::PlaylistPrx::checkedCast(streamprx);
		playlistId = playlist->getId();
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "setup()[%s]session associated playlist [%s]"),sessionId.c_str(), playlistId.c_str());
	    session = sessionprx;
	}
	catch(TianShanIce::BaseException&ex)
	{
		glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl,
			"setup Bcast PublishPoint caught error [%s,%s]"), ex.ice_name().c_str(), ex.message.c_str());

		_IceReThrow(TianShanIce::ServerError, ex);
	}
	catch (const ::Ice::Exception & ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", 840, 
			BcastChPPLog(BcastPublishPointImpl, "setup() caught ice exception '%s'"), ex.ice_name().c_str());
	}

	try
	{
		setupPlaylist();
	}   
	catch (TianShanIce::BaseException& ex)
	{
		_IceReThrow(TianShanIce::ServerError, ex);
	}
	catch (const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", 841, 
			BcastChPPLog(BcastPublishPointImpl, "setup()caught ice exception '%s'"), ex.ice_name().c_str());
	}

	bInService = true;
	glog(ZQ::common::Log::L_INFO , BcastChPPLog(BcastPublishPointImpl, "setup [%s]session successfully took %dms"), sessionId.c_str(), (int)(ZQ::common::now() - lstart));
}

TianShanIce::SRM::SessionPrx
BcastPublishPointImpl::getSession(const Ice::Current& current)
{
	RLock sync(*this);
	return session;
}

void
BcastPublishPointImpl::start(const Ice::Current& current)
{	
	int64 lstart = ZQ::common::now();
	TianShanIce::Streamer::StreamPrx streamPrx = NULL;
	{
		WLock sync(*this);
		glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl,"enter start()"));
	// check if weiwoo live
	bool isWeiwooLive = true;

		if(session && bInService)
	{
		try
		{
			session->ice_ping();
		}
		catch (const Ice::ObjectNotExistException& ex)
		{
			isWeiwooLive = false;
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,"start() weiwoo session not exist"));
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "start() ice_ping weiwoo session caught %s"), ex.ice_name().c_str());
		}
		if(isWeiwooLive)
		{
			try
			{
				streamPrx = session->getStream();
			}
			catch(TianShanIce::BaseException&ex)
			{
				isWeiwooLive = false;
				resetchannel();
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,
					"start() broadcast PublishPoint caught error [%s,%s]"), ex.ice_name().c_str(), ex.message.c_str());

				}
				catch(Ice::Exception&ex)
			{
				isWeiwooLive = false;
				resetchannel();
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", 841, CLOGFMT(BcastPublishPointImpl,
					"[%s]start()caught ice exception '%s'"), ident.name.c_str(), ex.ice_name().c_str());
			}
		}
	}
	else
	{
		isWeiwooLive = false;
		resetchannel();
	}

		try
		{
			if(!isWeiwooLive)
				setup(current);
		}
		catch(TianShanIce::BaseException&ex)
		{
			resetchannel();
			_IceReThrow(TianShanIce::ServerError, ex);
		}
		catch(Ice::Exception&ex)
		{
		resetchannel();
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", 841, CLOGFMT(BcastPublishPointImpl,
				"[%s]start() caught ice exception [%s]"), ident.name.c_str(), ex.ice_name().c_str());
		}
	}

	std::string srmSessionId = "";
	try
	{
		srmSessionId = session->getId();
		streamPrx = session->getStream();

		if(gBroadcastChCfg.mrtStreamServiceCfg.enable)
		{
			// remove optimize 
            Ice::ObjectPrx objPrx;
            #if ICE_INT_VERSION / 100 >= 306
                objPrx      =  streamPrx->ice_collocationOptimized(false);
            #else
                objPrx      =  streamPrx->ice_collocationOptimization(false);
            #endif
			streamPrx =  TianShanIce::Streamer::StreamPrx::uncheckedCast(objPrx);
		}
		TianShanIce::Streamer::StreamState state = streamPrx->getCurrentState();

		if (state == TianShanIce::Streamer::stsStreaming) {
			glog( ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "start()srmSessionID[%s] stream already play"), srmSessionId.c_str());
			return; // already streaming
		}

		streamPrx->play();
		timestamp = ZQTianShan::now();
	}
	catch(TianShanIce::BaseException&ex)
	{
		glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl,
			"start() BcastPublishPoint caught error [%s,%s]"), ex.ice_name().c_str(), ex.message.c_str());
		resetchannel();
		_IceReThrow(TianShanIce::ServerError, ex);
	}
	catch(Ice::Exception&ex)
	{
		resetchannel();
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", 841, CLOGFMT(BcastPublishPointImpl,
			"[%s] play stream caught ice exception [%s]"), ident.name.c_str(), ex.ice_name().c_str());
	}
	glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl,"leave start(), srmSessionID[%s] play stream[%s] successfully took %dms"),
		srmSessionId.c_str(), playlistId.c_str(),(int)(ZQ::common::now() - lstart));
}

void
BcastPublishPointImpl::stop(const Ice::Current& current)
{
	Lock sync(*this);
	glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl,"enter stop()"));

/*	if(gBroadcastChCfg.soapMRTCfg.enable && bInService && getStatusMRTStream())
	{
		tearDownMRTStream();
	}
*/
	TianShanIce::Streamer::StreamPrx strmPrx;
	if(session)
	{	
		// check if weiwoo live
		bool isWeiwooLive = true;
		try
		{
			session->ice_ping();
		}
		catch (const Ice::ObjectNotExistException& ex)
		{
			isWeiwooLive = false;
			glog(ZQ::common::Log::L_WARNING,  BcastChPPLog(BcastPublishPointImpl,"stop() weiwoo session not exist"));
			resetchannel();
			return;
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING,  BcastChPPLog(BcastPublishPointImpl,"stop() ice_ping weiwoo session caught %s"), ex.ice_name().c_str());
		}
		if(isWeiwooLive)
		{	
			try
			{	
				session->destroy();
			}
			catch (TianShanIce::BaseException&ex)
			{

                resetchannel();
				_IceReThrow(TianShanIce::ServerError, ex);
			}
			catch(Ice::Exception&ex)
			{
				resetchannel();
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", 842, BcastChPPLog(BcastPublishPointImpl,
					"stop stream caught ice exception '%s'"), ex.ice_name().c_str());
			}
		}
	}
    resetchannel();
	destroyChannelItemAssoc();

	glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "stop stream successfully"));
}

::Ice::Long
BcastPublishPointImpl::getUpTime(const Ice::Current& current)
{
	RLock sync(*this);
	if(!bInService)
		return 0;
	else
		return ZQTianShan::now() - timestamp;
}
void BcastPublishPointImpl::renew(Ice::Long TTL, const Ice::Current &)
{
	{
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "BcastPublishPoint renew (%lld)ms"), TTL);
		WLock sync(*this);
		expiration = ZQTianShan::now() + TTL;
		_env._watchDog.watchBcastChannel(ident, 0);
	}

	try
	{
		if(gBroadcastChCfg.mrtStreamServiceCfg.enable && bInService)
		{
			GetStatusCmd* pThread = new GetStatusCmd(_env, ident);
			if(pThread)
				pThread->start();
		}
	}
	catch (...){
	}

}
::Ice::Long BcastPublishPointImpl::getExpiration(const Ice::Current &)
{
	RLock sync(*this);
	return expiration;
}
bool BcastPublishPointImpl::isPersistent(const Ice::Current &)
{
	RLock sync(*this);
	return persistent;
}

TianShanIce::StrValues
BcastPublishPointImpl::getItemSequence(const Ice::Current& current)const
{
	RLock sync(*this);
	///modify this;
	if(bIsNOVDSupCh) //indicates the NOVD supplemental Channel
	{
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastprx;
		bcastprx = getBcastPublishPointEx();
		if(bcastprx != NULL)
			return bcastprx->getItemSequence();	
	}
	else
	{
		return itemSequence;
	}
	return TianShanIce::StrValues();
}

TianShanIce::Application::ChannelItem
BcastPublishPointImpl::findItem(const ::std::string& itemName,
									  const Ice::Current& current)const
{
	RLock sync(*this);
//	std::string itemKey = ident.name + CHANNELITEM_KEY_SEPARATOR + itemName;
	std::string itemKey = mainChName + CHANNELITEM_KEY_SEPARATOR + itemName;

	// search the record in the dictionary	
	LockT<RecMutex> lk(_env._dictLock);
    ChannelItemDict::const_iterator it = _env._pChannelItemDict->find(itemKey);
	if(it == _env._pChannelItemDict->end())
	{ // not found
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_300, 
			CLOGFMT(BcastPublishPointImpl, "channel item [%s] not found"), itemKey.c_str());
	}
	return it->second.setupInfo;
}

void
BcastPublishPointImpl::appendItem(const TianShanIce::Application::ChannelItem& newItem,
										const Ice::Current& current)
{
	TianShanIce::Application::Broadcast::ChannelItemEx newItemCopy;
	{
		WLock sync(*this);

		newItemCopy.setupInfo = newItem;
		checkChannelItem(newItemCopy.setupInfo);
		newItemCopy.setupInfo.lastModified = ZQTianShan::now();
		newItemCopy.key = ident.name + CHANNELITEM_KEY_SEPARATOR + newItemCopy.setupInfo.contentName;
//		newItemCopy.key = mainChName + CHANNELITEM_KEY_SEPARATOR + newItem.contentName;
		newItemCopy.flags = 0;
		newItemCopy.isFilter = 0;

		STRTOLOWER(newItemCopy.key);
		STRTOLOWER(newItemCopy.contentName);

		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "append [%s] on channel [%s], broadcast[%s], expiration[%s], inOffset[%lld], outOffset[%lld], spliceIn[%d], spliceOut[%d]"), 
			newItemCopy.setupInfo.contentName.c_str(), ident.name.c_str(), newItemCopy.setupInfo.broadcastStart.c_str(), newItemCopy.setupInfo.expiration.c_str(), 
			newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset, 
			newItemCopy.setupInfo.spliceIn, newItemCopy.setupInfo.spliceOut);

		if (!gBroadcastChCfg.InputLocalTime)
		{
			if (!systemTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_402, CLOGFMT(BcastPublishPointImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!systemTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}
		else 
		{
			if (!localTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_403, CLOGFMT(BcastPublishPointImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!localTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}

		// check if the channel item already exists in this channel publish point
		const std::string& newItemName = newItemCopy.setupInfo.contentName;
		TianShanIce::StrValues::iterator it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), newItemName);
		if (it != itemSequence.end())
		{ // already exists
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_400, CLOGFMT(BcastPublishPointImpl, "item [%s] already exists in channel [%s]"), newItemName.c_str(), ident.name.c_str());
		}

		try
		{
			LockT<RecMutex> lk(_env._dictLock);

			// DO: check if the last item's broadcasttime is larger than current append item
			if (itemSequence.size() > 0)
			{
				it = itemSequence.end() - 1;
				const ::std::string& strKeyName = ident.name + CHANNELITEM_KEY_SEPARATOR + *it;
				ChannelItemDict::iterator dictIt;
				dictIt = _env._pChannelItemDict->find(strKeyName);
				if (dictIt != _env._pChannelItemDict->end() && dictIt->second.broadcastStart > newItemCopy.broadcastStart)
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_401, CLOGFMT(BcastPublishPointImpl, "the append channel item [%s]'s broadcast time is less than the last channel item"), newItemCopy.key.c_str());
				}
			}

			_env._pChannelItemDict->put(ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
			itemSequence.push_back(newItemName);
			glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "[%s] appended on channel [%s]"), newItemName.c_str(), ident.name.c_str());
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_404, CLOGFMT(BcastPublishPointImpl, "add [%s] into safestore caught %s: %s"), 
				newItemCopy.key.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			ex.ice_throw();
		}
	}

	_env.appendPlaylistItem(mainChName, newItemCopy);
}

void
BcastPublishPointImpl::insertItem(const ::std::string& atItemName,
										const TianShanIce::Application::ChannelItem& newItem,
										const Ice::Current& current)
{		
	TianShanIce::Application::Broadcast::ChannelItemEx newItemCopy; // new channel item context
	std::string istPosKey, istPosName; // the new item's insert position
	std::string newKey ;
	{
		WLock sync(*this);

		//	std::string newKey = mainChName + CHANNELITEM_KEY_SEPARATOR + newItem.contentName;

		newItemCopy.setupInfo = newItem;
		checkChannelItem(newItemCopy.setupInfo);
		newKey = ident.name + CHANNELITEM_KEY_SEPARATOR + newItemCopy.setupInfo.contentName;
		newItemCopy.setupInfo.lastModified = ZQTianShan::now();
		newItemCopy.key = newKey;
		newItemCopy.isFilter = 0;
		newItemCopy.flags = 0;
		STRTOLOWER(newItemCopy.key);
		STRTOLOWER(newItemCopy.setupInfo.contentName);

		istPosKey = ident.name + CHANNELITEM_KEY_SEPARATOR + atItemName;
		//		istPosKey = mainChName + CHANNELITEM_KEY_SEPARATOR + atItemName;
		istPosName = atItemName;
		STRTOLOWER(istPosKey);
		STRTOLOWER(istPosName);

		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "insert [%s] at [%s] on channel [%s], broadcast[%s], expiration[%s], inOffset[%lld], outOffset[%lld], spliceIn[%d], spliceOut[%d]"),
			newItemCopy.setupInfo.contentName.c_str(), istPosName.c_str(), ident.name.c_str(), newItemCopy.setupInfo.broadcastStart.c_str(), newItemCopy.setupInfo.expiration.c_str(), 
			newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset, 
			newItemCopy.setupInfo.spliceIn, newItemCopy.setupInfo.spliceOut);

		if (!gBroadcastChCfg.InputLocalTime)
		{
			if (!systemTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_500, CLOGFMT(BcastPublishPointImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!systemTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}
		else 
		{
			if (!localTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_501, CLOGFMT(BcastPublishPointImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}		
			if (!localTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}

		// check if this content already exists
		const std::string& newItemName = newItemCopy.setupInfo.contentName;
		TianShanIce::StrValues::iterator it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), newItemName);
		if (it != itemSequence.end())
		{ // insert item already exists
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_502, CLOGFMT(BcastPublishPointImpl, "item [%s] already exists on channel [%s]"), newItemName.c_str(), ident.name.c_str());
		}

		it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), istPosName);
		if(it == itemSequence.end())
		{ // insert at item not found
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_503, CLOGFMT(BcastPublishPointImpl, "insert position [%s] not found on channel [%s]"), istPosName.c_str(), ident.name.c_str());
		}

		try
		{
			LockT<RecMutex> lk(_env._dictLock);		
			_env._pChannelItemDict->put(ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
		}
		catch (const Freeze::DatabaseException& ex)
		{ // add channel item context to channel item dict failed
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_504, CLOGFMT(BcastPublishPointImpl, "add channel item context to channel item dict caught %s: %s"), 
				ex.ice_name().c_str(), ex.message.c_str());
		}

		// then update the item sequence list
		itemSequence.insert(it, newItemName);
		glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "item [%s] inserted at [%s] on channel [%s]"), 
			newItemName.c_str(), istPosName.c_str(), ident.name.c_str());
	}
    
	_env.insertPlaylistItem(mainChName,istPosKey, newItemCopy);
}

void
BcastPublishPointImpl::appendItemAs(const TianShanIce::Application::ChannelItem& newItem,
										  const ::std::string& newName,
										  const Ice::Current& current)
{
	ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_300, "[%s]not implement", ident.name.c_str());
}
void
BcastPublishPointImpl::insertItemAs(const ::std::string& atItemName,
										  const TianShanIce::Application::ChannelItem& newItem,
										  const ::std::string& newName,
										  const Ice::Current& current)
{
	ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_301, "[%s]not implement", ident.name.c_str());
}

void
BcastPublishPointImpl::replaceItem(const ::std::string& oldName,
										 const TianShanIce::Application::ChannelItem& newItem,
										 const Ice::Current& current)
{
	TianShanIce::Application::Broadcast::ChannelItemEx newItemCopy;
	std::string oldItemKey, oldItemName;
	std::string newKey;

	{
		WLock sync(*this);		
		newItemCopy.setupInfo = newItem;
		checkChannelItem(newItemCopy.setupInfo);
		newItemCopy.key= ident.name + CHANNELITEM_KEY_SEPARATOR + newItemCopy.setupInfo.contentName;
//		newItemCopy.key= mainChName + CHANNELITEM_KEY_SEPARATOR + newItem.contentName;
		newItemCopy.setupInfo.lastModified = ZQTianShan::now();
		newItemCopy.isFilter = 0;
		newItemCopy.flags = 0;
		STRTOLOWER(newItemCopy.key);
		STRTOLOWER(newItemCopy.setupInfo.contentName);

		oldItemKey = ident.name + CHANNELITEM_KEY_SEPARATOR + oldName;
//		oldItemKey = mainChName + CHANNELITEM_KEY_SEPARATOR + oldName;
		oldItemName = oldName;
		STRTOLOWER(oldItemKey);
		STRTOLOWER(oldItemName);

		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "replace [%s] with [%s] on channel [%s], broadcast[%s], expiration[%s], inOffset[%lld], outOffset[%lld], spliceIn[%d], spliceOut[%d]"),
			oldItemName.c_str(), newItemCopy.setupInfo.contentName.c_str(), ident.name.c_str(), newItemCopy.setupInfo.broadcastStart.c_str(), newItemCopy.setupInfo.expiration.c_str(), 
			newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset, 
			newItemCopy.setupInfo.spliceIn, newItemCopy.setupInfo.spliceOut);

		if (!gBroadcastChCfg.InputLocalTime)
		{
			if (!systemTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_600, CLOGFMT(BcastPublishPointImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!systemTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}
		else 
		{
			if (!localTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_601, CLOGFMT(BcastPublishPointImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!localTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}

		// check if this content already exists
		const std::string& newItemName = newItemCopy.setupInfo.contentName;
		TianShanIce::StrValues::iterator itNew = itemSequence.end(), itOld = itemSequence.end();

		// check if this old content exists
		itOld = ::std::find(itemSequence.begin(), itemSequence.end(), oldItemName);
		if(itOld == itemSequence.end())
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_605, CLOGFMT(BcastPublishPointImpl, "replace item [%s] with [%s] on channel [%s] caught old item not exist"), 
				oldItemName.c_str(), newItemName.c_str(), ident.name.c_str());
		}

		itNew = ::std::find(itemSequence.begin(), itemSequence.end(), newItemName);
		if (itNew != itemSequence.end())
		{ // if the new item's name cound be found in dict means modify the existing one's properties
			if (oldItemName != newItemName)
			{
				// channel item already exists but not modifing the properties
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublishPointImpl", err_602, CLOGFMT(BcastPublishPointImpl, "channel item [%s] already exists"), newItemCopy.key.c_str());
			}

			// channel item already exists but modifing the properties
			LockT<RecMutex> lk(_env._dictLock);
			ChannelItemDict::iterator dictIt;
			dictIt = _env._pChannelItemDict->find(oldItemKey);
			if (dictIt ==_env._pChannelItemDict->end())
			{
				itemSequence.erase(itNew); // not found in dict but in item sequence, so you have to erase it
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_603, CLOGFMT(BcastPublishPointImpl, "channel item [%s] not found in dict"), newItemCopy.key.c_str());
			}

			const TianShanIce::Application::Broadcast::ChannelItemEx& oldItemContx = dictIt->second;
			if (newItemCopy.setupInfo.inTimeOffset == oldItemContx.setupInfo.inTimeOffset && 
				newItemCopy.setupInfo.outTimeOffset == oldItemContx.setupInfo.outTimeOffset)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPublishPointImpl, "replace item but (cueIn and cueOut equal to old one), so ignore"));
				return;
			}

			try
			{
				// replace the record in the dictionary
				_env._pChannelItemDict->put(ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
			}
			catch(const Freeze::DatabaseException& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_606, CLOGFMT(BcastPublishPointImpl, "update [%s] properties on channel [%s] caught %s: %s"), 
					ident.name.c_str(),newItemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_603, CLOGFMT(BcastPublishPointImpl, "update [%s] properties on channel [%s] caught %s"), 
					ident.name.c_str(),newItemName.c_str(), ex.ice_name().c_str());
			}
			glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "item [%s] on channel [%s] replaced"), newItemName.c_str(), ident.name.c_str());
		}
		else // if the new item not found in dict means replace the old one with the new one
		{	
			try
			{
				LockT<RecMutex> lk(_env._dictLock);
				// replace the record in the dictionary
				_env._pChannelItemDict->erase(oldItemKey);
				_env._pChannelItemDict->put(ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
			}
			catch (const Freeze::DatabaseException& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_606, CLOGFMT(BcastPublishPointImpl, "update [%s] with [%s] on channel [%s] caught %s: %s"), 
					ident.name.c_str(), oldItemName.c_str(), newItemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_607, CLOGFMT(BcastPublishPointImpl, "update [%s] with [%s] on channel [%s] caught %s"), 
					ident.name.c_str(),oldItemName.c_str(), newItemName.c_str(), ex.ice_name().c_str());
			}
			// then update the item sequence list
			*itOld = newItemName;
			glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "replace [%s] with [%s] on channel [%s] successfully"), oldItemName.c_str(), newItemName.c_str(), ident.name.c_str());
		}
		//Playlist�е�Item��������oldItem������
		//����ChannelItem List: 1, 2,3,4,5,6,7,8,9,10
		//Playlist���� 3, 4,5,6,7 �����м�סcurrentItemλ���� 7
		//���ð�7�滻��.����� 1, 2,3,4,5,6,11,8,9,10
		//Playlist�б���� 3, 4,5,6,11����Ҫ��currentItemλ�ñ���11
		if(currentItem != "" && currentItem == oldName )
		{
			currentItem = newItemName;
		}
	}

	_env.replacePlaylistItem(mainChName, oldItemKey, newItemCopy);
}

void
BcastPublishPointImpl::removeItem(const ::std::string& itemName,
										const Ice::Current& current)
{
	std::string rmvItemKey, rmvItemName;
	{
		WLock sync(*this);
        
		rmvItemKey = ident.name + CHANNELITEM_KEY_SEPARATOR + itemName;
//		rmvItemKey = mainChName + CHANNELITEM_KEY_SEPARATOR + itemName;
		rmvItemName = itemName;
		STRTOLOWER(rmvItemKey);
		STRTOLOWER(rmvItemName);

		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "remove item [%s] on channel [%s]"), rmvItemName.c_str(), ident.name.c_str());

		// remove from channel item dict
		try
		{
			LockT<RecMutex>  lk(_env._dictLock);
			_env._pChannelItemDict->erase(rmvItemKey);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_700, CLOGFMT(BcastPublishPointImpl, "remove item [%s] on channel [%s] caught %s: %s"), 
				rmvItemName.c_str(), ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPointImpl", err_700, CLOGFMT(BcastPublishPointImpl, "remove item [%s] on channel [%s] caught %s"), 
				rmvItemName.c_str(), ident.name.c_str(), ex.ice_name().c_str());
		}

		// remove from item sequence
		TianShanIce::StrValues::iterator it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), rmvItemName);
		if (it != itemSequence.end())
		{
			//���ɾ����Item������ currentItem.
			//���������: 1. BcastPublishPoint������ֻ��һ��Item, ��currentItem="",
			// 2.ɾ����Item������BcastPublishPoint Item List�е����һ�������м�һ��, ��currentItem = (rmvItemName��ǰһ��Item)
			// 3.ɾ������Item�еĵ�һ��,��currentItem=(BcastPublishPoint Item List�е����һ��Item)
			if(currentItem != ""  && currentItem == rmvItemName)
			{
				TianShanIce::StrValues::reverse_iterator rIt = itemSequence.rend();
				rIt = ::std::find(itemSequence.rbegin(), itemSequence.rend(), rmvItemName);
				if(rIt == itemSequence.rbegin())// �ڶ������.
				{
					if(itemSequence.size() == 1)//��һ�����
						 currentItem = "";
					else 
					{
						rIt++;
						currentItem = *rIt;
					}
				}
				else if(it == itemSequence.begin())//���������
				{
				  currentItem = *(itemSequence.rbegin());
				}
				else //�ڶ������
				{
					rIt++;
					currentItem = *rIt;
				}
			}
			// remove item name from sequence
			itemSequence.erase(it);
		}
		glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "item [%s] removed from channel [%s]"), rmvItemName.c_str(), ident.name.c_str());
	}
	_env.removePlaylistItem(mainChName, rmvItemKey);
}
bool BcastPublishPointImpl::addFilterItemToPlaylist()
{
    Lock lock(*this);

	//get all filter item info
	TianShanIce::StrValues filterItems;
	try
	{
		filterItems = _env._FilterItems->getFilterItemSequence();
	}
	catch (Ice::Exception&ex)
	{
		glog(ZQ::common::Log::L_WARNING,
			BcastChPPLog(BcastPublishPointImpl, "get filter item sequence caught %s"),
			ex.ice_name().c_str());
	}

	int filterItemSize = filterItems.size();

	//�����FilterItem,�����ѭ������FilterItem
	if(filterItemSize > 0)
	{
		TianShanIce::IValues plSequence;
		try
		{	
			plSequence = playlist->getSequence();
		}
		catch (TianShanIce::ServerError&ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"addFilterItemToPlaylist()get playlist caught [%d,%s]"),ex.errorCode, ex.message.c_str());
		}
		catch(Ice::Exception&ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"addFilterItemToPlaylist()get playlist caught [%s]"),ex.ice_name().c_str());
		}
        int plSize = plSequence.size();
		int windowsize = DEFAULTWINDOWSIZE;
		TianShanIce::StrValues::iterator filterItor;
		if(gBroadcastChCfg.windowsize > 0)
			windowsize = gBroadcastChCfg.windowsize;

		if(plSize >= DEFAULTWINDOWSIZE)
		{
			glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, 
				"addFilterItemToPlaylist()playlist item count [%d]"), plSize);
		}

		for(int i = 0;  i <  windowsize - plSize; i++)
		{
			TianShanIce::Application::Broadcast::ChannelItemEx filterItem;
			if(!getRandomFilterItem(filterItem))
				continue;

			std::string itemKey = filterItem.setupInfo.contentName;
			STRTOLOWER(itemKey);
			// initialize playlist item information according to channel item.
			TianShanIce::Streamer::PlaylistItemSetupInfo newItemInfo;
			copyChannelItemToSetupInfo(filterItem, newItemInfo);
			int userCtrlNum = _gUserCtrlNumGen.Generate();

			// ignore exception from calling pushBack() so that a failure on push item doesn't affect the whole setup.
			try
			{
				playlist->pushBack(userCtrlNum, newItemInfo);
				glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "filter item [%s] appended to playlist [%s], userctrlnum [%d]."), itemKey.c_str(), playlistId.c_str(), userCtrlNum);
			}
			catch(const TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append filter item [%s] on playlist[%s] caught %s:%s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				continue;
			}
			catch(const ::Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append filter item [%s] on playlist[%s] caught %s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str());
				continue;
			}
			// add an item to PurchaseItemAssoc
			ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
			pia->ident.name = IceUtil::generateUUID();
			pia->ident.category = ICE_ChannelItemAssoc;
			pia->bcastPPIdent = ident;
			pia->channelItemKey = itemKey;
			pia->playlistCtrlNum = userCtrlNum;
			pia->lastModified = filterItem.setupInfo.lastModified;
			try
			{
				_env._evitChannelItemAssoc->add(pia, pia->ident);
			}
			catch (const Freeze::DatabaseException& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 306, BcastChPPLog("BcastPublishPoint", "append filter item assoc[%s] caught  %s:%s"), pia->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 307, BcastChPPLog("BcastPublishPoint", "append filter item assoc[%s] caught %s"), pia->ident.name.c_str(), ex.ice_name().c_str());
			}
			glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "BcastPublishPoint item [%s] appended"), pia->ident.name.c_str());
		}
	}
	return true;
}
bool 
BcastPublishPointImpl::setupPlaylist()
{
//	Lock lock(*this);
	int64 lstart = ZQ::common::now();
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "setupPlaylist() enter"));

	ChannelItemDict::const_iterator dictIt;
	TianShanIce::StrValues::iterator sit;

	TianShanIce::StrValues chItemSequence;// = getItemSequence(Ice::Current());

	if(bIsNOVDSupCh) //indicates the NOVD supplemental Channel
	{
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastprx;
		bcastprx = getBcastPublishPointEx();
		if(bcastprx != NULL)
			chItemSequence = bcastprx->getItemSequence();	
	}
	else
	{
	   chItemSequence = itemSequence;
	}

	TianShanIce::StrValues filterItems ;
	//�õ����е�FilterItem��Ϣ
	try
	{
		filterItems = _env._FilterItems->getFilterItemSequence();
	}
	catch (Ice::Exception&ex)
	{
		glog(ZQ::common::Log::L_WARNING,
			BcastChPPLog(BcastPublishPointImpl, "fail to get filter item sequence caught %s"),
			ex.ice_name().c_str());
	}

	int filterItemSize = filterItems.size();

	//��ʾBcastPublishPoint��û��channelItem, ��һֱѭ������FilterItem
	if(chItemSequence.size() < 1 && filterItemSize > 0)
	{
       return addFilterItemToPlaylist();
	}
	else
	{
		int itemCount = 0;//��¼��ǰPlayList�в����˶��ٸ�ChannelItem. 
		                  //�����ֵΪ0��ʱ��˵��BcastPublishPoint�е�Item�������û����Ѿ�����

		Ice::Long currentBcastStrat = 0, previousBcastStart = UserGetCurrentTime();
		                  //previousBcastStart ��¼ǰһ��Item��BroadcastStratTime

		for(sit= chItemSequence.begin(); sit != chItemSequence.end(); sit ++)
		{
			//���֧����СWindow����ItemCount �Ѿ����������õ�Windown���ȣ��򷵻�
			if(gBroadcastChCfg.windowsize > 0 && itemCount >= gBroadcastChCfg.windowsize)
			{
				break;
			}

			std::string itemKey = mainChName + CHANNELITEM_KEY_SEPARATOR + *sit;
			STRTOLOWER(itemKey);
            
			//��ס��ǰChannelItem sequence���Ѿ���PlayList�в����Item��λ��
			currentItem = *sit;

			TianShanIce::Application::Broadcast::ChannelItemEx tmpItem;
			// find channel item according to the item key
			try
			{
				LockT<RecMutex> lk(_env._dictLock);
				dictIt = _env._pChannelItemDict->find(itemKey); 
				// if channel item not found in safestore, should throw server error
				if (dictIt == _env._pChannelItemDict->end())
				{
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 305, BcastChPPLog("BcastPublishPoint", "channel item [%s] not found in dict"), itemKey.c_str());
				}
				// initialize playlist item information according to channel item.
				tmpItem = dictIt->second;
			}
			catch (const Freeze::DatabaseException& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 303, BcastChPPLog("BcastPublishPoint", "find channel item [%s] caught %s:%s"), itemKey.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const TianShanIce::ServerError& ex)
			{
				ex.ice_throw();
			}
			catch (const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 304, BcastChPPLog("BcastPublishPoint", "find channel item [%s] caught %s"), itemKey.c_str(), ex.ice_name().c_str());
			}


			//ChannelItem �Ѿ�����, ���ٲ���
			if(tmpItem.expiration > 0 && tmpItem.expiration < UserGetCurrentTime())
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "Item [%s] on channel [%s], broadcast[%s], expiration[%s]"), 
					tmpItem.setupInfo.contentName.c_str(), ident.name.c_str(), tmpItem.setupInfo.broadcastStart.c_str(), tmpItem.setupInfo.expiration.c_str());
				continue;
			}

			//�����ǰItem��BroadcastStartTime > ǰһ��Item�� ������һ��δ��ʱ�䣬 ����Ҫ���м�� FilterItem��ȥ
			if(filterItemSize > 0 && tmpItem.broadcastStart > UserGetCurrentTime())
			{
				//��õ�ǰ��Ŀ����һ����Ŀ��ʱ���(second),��ʱ�������������FilterItem;
				Ice::Long timeInterval = tmpItem.broadcastStart - previousBcastStart;
				TianShanIce::Application::Broadcast::ChannelItemEx filterItem;

				//����������������,��ֹ��һ�����ѡ�񵽵�FilterItem������(�ǳ����������ֻ��һ��FilterItem,����)
				int randomCount = 5;
				bool bRet = false;
				//
				while(randomCount)
				{
					bRet = getRandomFilterItem(filterItem);
					if(bRet == false)
					{
						continue;
						randomCount--;
					}
                    // nInterval ��ʾ��ѡ�񵽵�FilterItem��Ƭ��(��λΪms)
					Ice::Long  nInterval = filterItem.setupInfo.outTimeOffset - filterItem.setupInfo.inTimeOffset;
					int nInsertFilterNumber = int(timeInterval *1000 / nInterval + 1);
					while(nInsertFilterNumber)
					{
						TianShanIce::Streamer::PlaylistItemSetupInfo newItemInfo;
						copyChannelItemToSetupInfo(filterItem, newItemInfo);
						int userCtrlNum = _gUserCtrlNumGen.Generate();
						// ignore exception from calling pushBack() so that a failure on push item doesn't affect the whole setup.
						try
						{
							playlist->pushBack(userCtrlNum, newItemInfo);
							glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "channel item [%s] appended to playlist [%s], userctrlnum [%d]."), itemKey.c_str(), playlistId.c_str(), userCtrlNum);
						}
						catch(const TianShanIce::BaseException& ex)
						{
							glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s:%s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
							break;
						}
						catch(const ::Ice::Exception& ex)
						{
							glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str());
							break;
						}
						// add an item to PurchaseItemAssoc
						ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
						pia->ident.name = IceUtil::generateUUID();
						pia->ident.category = ICE_ChannelItemAssoc;
						pia->bcastPPIdent = ident;
						pia->channelItemKey = itemKey;
						pia->playlistCtrlNum = userCtrlNum;
						pia->lastModified = tmpItem.setupInfo.lastModified;
						try
						{
							_env._evitChannelItemAssoc->add(pia, pia->ident);
						}
						catch (const Freeze::DatabaseException& ex)
						{
							ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 306, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught  %s:%s"), pia->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
						}
						catch (const Ice::Exception& ex)
						{
							ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 307, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught %s"), pia->ident.name.c_str(), ex.ice_name().c_str());
						}
						nInsertFilterNumber --;
					}
					if(nInsertFilterNumber == 0)
					      randomCount = 0;
					else
						randomCount--;
				}
			}

			TianShanIce::Streamer::PlaylistItemSetupInfo newItemInfo;
			copyChannelItemToSetupInfo(tmpItem, newItemInfo);
			int userCtrlNum = _gUserCtrlNumGen.Generate();

			// ignore exception from calling pushBack() so that a failure on push item doesn't affect the whole setup.
			try
			{
				playlist->pushBack(userCtrlNum, newItemInfo);
				glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "channel item [%s] appended to playlist [%s], userctrlnum [%d]."), itemKey.c_str(), playlistId.c_str(), userCtrlNum);
			}
			catch(const TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s:%s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				continue;
			}
			catch(const ::Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str());
				continue;
			}

			//��ס��ǰ�Ѿ����뵽PlayList��BroadStartTime,���������һ��Itemʱ����Ӧ�ò������FilterItem
			previousBcastStart = tmpItem.broadcastStart;

			// add an item to PurchaseItemAssoc
			ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
			pia->ident.name = IceUtil::generateUUID();
			pia->ident.category = ICE_ChannelItemAssoc;
			pia->bcastPPIdent = ident;
			pia->channelItemKey = itemKey;
			pia->playlistCtrlNum = userCtrlNum;
			pia->lastModified = tmpItem.setupInfo.lastModified;
			try
			{
				_env._evitChannelItemAssoc->add(pia, pia->ident);
			}
			catch (const Freeze::DatabaseException& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 306, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught  %s:%s"), pia->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 307, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught %s"), pia->ident.name.c_str(), ex.ice_name().c_str());
			}
			itemCount++;
			glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "BcastPublishPoint item [%s] appended"), pia->ident.name.c_str());
		}

        #pragma message ( __MSGLOC__ "Channel ��Item, �������е�Item���ǲ����õ�,�����쳣")
		if(itemCount == 0)
		{
			resetchannel();
			destroyChannelItemAssoc();
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 307, BcastChPPLog("BcastPublishPoint", 
				"Invalid channel item or all channel item already expiration or stream server error, please check"));
		}
	}	
	glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "setupPlaylist() took %dms"),(int)(ZQ::common::now()- lstart));
	return true;
}

void BcastPublishPointImpl::copyChannelItemToSetupInfo(const TianShanIce::Application::Broadcast::ChannelItemEx& chnlItemInfo, 
															 TianShanIce::Streamer::PlaylistItemSetupInfo& setupInfo)
{
	setupInfo.contentName = chnlItemInfo.setupInfo.contentName;
//	setupInfo.criticalStart = 0;
	if(chnlItemInfo.broadcastStart > UserGetCurrentTime())
	   setupInfo.criticalStart = chnlItemInfo.broadcastStart;
	else
    setupInfo.criticalStart = 0;
	setupInfo.forceNormal = chnlItemInfo.setupInfo.forceNormalSpeed;
	setupInfo.inTimeOffset = chnlItemInfo.setupInfo.inTimeOffset;
	setupInfo.outTimeOffset = chnlItemInfo.setupInfo.outTimeOffset;
	setupInfo.spliceIn = chnlItemInfo.setupInfo.spliceIn;
	setupInfo.spliceOut = chnlItemInfo.setupInfo.spliceOut;
	setupInfo.flags = chnlItemInfo.flags;
}

void BcastPublishPointImpl::OnEndOfStream(const ::std::string&playlistId, const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "end of stream, playlistId [%s]"), playlistId.c_str());
	/*	_env.removeSessionRenew(ident);
	playlist = NULL;
	playlistId = "";
	bInService = false;
	bNeedSyncChannel = false;
	timestamp = ZQTianShan::now();
	try
	{	
	if(session)
	session->destroy();

	}
	catch (TianShanIce::ServerError&ex)
	{
	glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "destroy session caught %s: %s"), 
	ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception&ex)
	{

	}
	session = NULL;*/

}

void BcastPublishPointImpl::OnStreamExit(const ::std::string&plid, const Ice::Current& current)
{
	resetchannel();
	glog(ZQ::common::Log::L_DEBUG,BcastChPPLog(BcastPublishPointImpl, "stream exit with playlistId [%s]"), plid.c_str());
}

void BcastPublishPointImpl::OnEndOfItem(Ice::Int userCtrlNum, const Ice::Current& current)
{	
	Lock lock(*this);
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "End-of-Item [%d] enter"),userCtrlNum);

	//�����Item�������,��PlayList��ɾ�����Item
    if(userCtrlNum > 0)
	    removeChItemAssocByCtrlNumber(userCtrlNum);

    #pragma message ( __MSGLOC__ "TODO: ��Ҫ���PlayList�е�Item�����Ƿ�����ָ���ĸ���")

	//��õ�ǰPlaylist��Item�ĸ���, ��������������Item����, ���Բ�������, ���� �ع�Playlist.
	int currentCtrlNum = 0;
	TianShanIce::IValues plSequence;
	try
	{	
		currentCtrlNum = playlist->current();
		plSequence = playlist->getSequence();
	}
	catch (TianShanIce::ServerError&ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
			"End-of-Item()get playlist caught [%d,%s]"),ex.errorCode, ex.message.c_str());
	}
	catch(Ice::Exception&ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
			"End-of-Item()get playlist caught [%s]"),ex.ice_name().c_str());
	}
	//����ɾ��playlist��������item(��������û�б�ɾ���Ļ�����FilterItem��������);
	TianShanIce::IValues::iterator itorItem = plSequence.begin();
	while(itorItem != plSequence.end())
	{
		if(*itorItem != currentCtrlNum)
		{
			removeChItemAssocByCtrlNumber(*itorItem);
			plSequence.erase(itorItem);
			itorItem = plSequence.begin();
		}
		else
		{
			break;
		}
	}
    #pragma message (__MSGLOC__ "TODO: check  playlist, rebuild playlist when the playlist item is less than configration windownItem")
    
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, 
		"End-of-Item() IsRepeat [%d], windowsize [%d], minimum playlist item count [%d], current playlist item count [%d] "),
		gBroadcastChCfg.IsRepeat, gBroadcastChCfg.windowsize > DEFAULTWINDOWSIZE? gBroadcastChCfg.windowsize:DEFAULTWINDOWSIZE,
		gBroadcastChCfg.miniPLcount > MINIMUMPLITEMCOUNT ? gBroadcastChCfg.miniPLcount:MINIMUMPLITEMCOUNT,
		plSequence.size());
	TianShanIce::StrValues chItemSequence;// = getItemSequence(Ice::Current());

	if(bIsNOVDSupCh) //indicates the NOVD supplemental Channel
	{
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastprx;
		bcastprx = getBcastPublishPointEx();
		if(bcastprx != NULL)
			chItemSequence = bcastprx->getItemSequence();	
	}
	else
	{
		chItemSequence = itemSequence;
	}

	//channel��û��Item, ѭ������FilterItem
	if(chItemSequence.size() == 0)
	{
		currentItem = "";
		try
		{  
			addFilterItemToPlaylist();
		}
		catch(TianShanIce::ServerError&ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"End-of-Item() addfilter Item to play list caught [%d,%s]"),ex.errorCode, ex.message.c_str());
		}
		catch(Ice::Exception&ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"End-of-Item()addfilter Item to play list caught ice exception[%s]"),ex.ice_name().c_str());
		}
	}
	else
	{	
		Ice::Long previousBcastStart = UserGetCurrentTime();

		int plSize = plSequence.size();

		//��ż���Ҫ����PlayList�е�ItemSequence
		TianShanIce::StrValues useChannelItems;

        #pragma message (__MSGLOC__ "TODO:����ʲôCase���������������")

		TianShanIce::StrValues::iterator itorchItem; 
		itorchItem  = std::find(chItemSequence.begin(), chItemSequence.end(), currentItem);

		//���Currentitem������channelItem sequence���ҵ�,���������,
		//һ����ItemSequence�����һ��Item,��һ���ǵ�һ�������м��Item
		if(itorchItem == chItemSequence.end())
		{
		   #pragma message (__MSGLOC__ "TODO:����α�ǵ�Item����Channel Item Sequence��")
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"End-of-Item()indicated currentItem [%s] not exists in channel ItemSequence"),currentItem.c_str());
			return;
		}
        
		//�ҵ�currentItem
		{
			ChannelItemDict::const_iterator dictIt;
			std::string itemKey = mainChName + CHANNELITEM_KEY_SEPARATOR + currentItem;
			STRTOLOWER(itemKey);
			// find channel item according to the item key
			try
			{
				
				LockT<RecMutex> lk(_env._dictLock);
				dictIt = _env._pChannelItemDict->find(itemKey); 
				// if channel item not found in safestore, should throw server error
				if (dictIt == _env._pChannelItemDict->end())
				{
					glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
						"End-of-Item() channel item [%s] not found in dict"), itemKey.c_str());
				}
			}
			catch (const Freeze::DatabaseException& ex)
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
					"End-of-Item() find channel item [%s] caught %s:%s"),
					itemKey.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const TianShanIce::ServerError& ex)
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
					"End-of-Item() find channel item [%s] caught %d:%s"),
					itemKey.c_str(), ex.errorCode, ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
					"End-of-Item()find channel item [%s] caught %s"),
					itemKey.c_str(), ex.ice_name().c_str());
			}

			// initialize playlist item information according to channel item.
			const TianShanIce::Application::Broadcast::ChannelItemEx& tmpItem = dictIt->second;

			previousBcastStart = tmpItem.broadcastStart;
		}

	   TianShanIce::StrValues::reverse_iterator ritor = std::find(chItemSequence.rbegin(), chItemSequence.rend(), currentItem);
	   if(ritor == chItemSequence.rbegin()) //ChannelItem�����һ��Ƭ��
	   {
		   if(!gBroadcastChCfg.IsRepeat)//�������Repeatģʽ, ˵��PlayList�Ľ�β�Ѿ���ChannelItem�Ľ�β
		   {
			   glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, 
				   "End-of-Item() the last channelItem %s already append on playlist %s"),currentItem.c_str(), playlistId.c_str());
			   return;
		   }
		   else//�����Repeatģʽ,����Ҫ����Ƿ�ChannelItem�ٴμ��뵽Playlist��(windowsize �Ƿ�����)
		   {
			   useChannelItems = chItemSequence;
		   }
	   }  
	   else  
	   {
		   //��currentItem ����һ��������channelItem sequenceβ��
		   itorchItem++;
		   useChannelItems.assign(itorchItem, chItemSequence.end());
	   }

      //if support the window
	   if(gBroadcastChCfg.windowsize > 0)
	   {
		   //���playListSize���� WindowLength, ����Ҫ�ж�playlist�����һ��Item�ǲ���FilterItem(������������ԭ��, ���ð�PlayList�����һ��Remove��, ��RemoveItem��Ӧ��������)
		   //�����, 
		   //����Ҫ���һ��Item��ȥ,�Ա�֤PlayList����channelItem������
		   if(plSize > gBroadcastChCfg.windowsize)
		   {
			   glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, 
				   "End-of-Item() playlist [%s]Item count is %d, configration windowlength is %d"), 
				   playlistId.c_str(), plSize, gBroadcastChCfg.windowsize);
		   }
		   else
		   {	
			   int needAddItemCount = gBroadcastChCfg.windowsize - plSize;
			   rebuildPlaylist(useChannelItems, needAddItemCount, previousBcastStart);
		   }
	  }
	  else
	  {
		  //�����֧��WindowsLength,���ڵ�һ�ε�ʱ��Ͱ����е�ChannelItem���뵽PlayList����.
		  //�������ж��Ƿ���ҪRepeat,�����Ҫ,���жϵ�ǰPlayList�л��ж��ٸ�Item,
		  //���С��MINIMUMPLAYLISTSIZE,����Ҫ����PlayList.�����е�ChannelItem���뵽PlayList��
         if(gBroadcastChCfg.IsRepeat)
		 {
			 Ice::Long miniPLcount = gBroadcastChCfg.miniPLcount;
			 if(chItemSequence.size() < miniPLcount)
				 miniPLcount = chItemSequence.size();

			 if(plSize < miniPLcount || plSize == 1)
			 {
               rebuildPlaylist(chItemSequence, chItemSequence.size(),previousBcastStart);
			 }
		 }
	  }
	}

   	//add sync; 
/*	SyncPlaylistRequest* pRequest = new SyncPlaylistRequest(_env, ident);
	if (pRequest)
	{
		pRequest->start();
	}
	else 
	{
		glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "create broadcastpublishpoint request for syncplaylist failed"));
	}*/

	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "End-of-Item [%d] leave"),userCtrlNum);
}
bool 
BcastPublishPointImpl::rebuildPlaylist(TianShanIce::StrValues& useChannelItems, int needAddItemCount, Ice::Long previousBcastStart)
{
//	Lock lock(*this);
	int64 lstart = ZQ::common::now();
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "rebuildPlaylist() enter, need insert playlist itemcount [%d]"), needAddItemCount);
	TianShanIce::StrValues filterItems ;
	ChannelItemDict::const_iterator dictIt;
	//�õ����е�FilterItem��Ϣ
	try
	{
		filterItems = _env._FilterItems->getFilterItemSequence();
	}
	catch (Ice::Exception&ex)
	{
		glog(ZQ::common::Log::L_WARNING,
			BcastChPPLog(BcastPublishPointImpl, "fail to get filter item sequence caught %s"),
			ex.ice_name().c_str());
	}
	int filterItemSize = filterItems.size();

	TianShanIce::StrValues::iterator sit;
	for(sit = useChannelItems.begin(); sit != useChannelItems.end() && needAddItemCount > 0; sit++)
	{
		std::string itemKey = mainChName + CHANNELITEM_KEY_SEPARATOR + *sit;
		STRTOLOWER(itemKey);

		// find channel item according to the item key
		// initialize playlist item information according to channel item.
		TianShanIce::Application::Broadcast::ChannelItemEx tmpItem;
		try
		{
			LockT<RecMutex> lk(_env._dictLock);
			dictIt = _env._pChannelItemDict->find(itemKey); 
			// if channel item not found in safestore, should throw server error
			if (dictIt == _env._pChannelItemDict->end())
			{
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
					"rebuildPlaylist() channel item [%s] not found in dict"), itemKey.c_str());
				continue;
			}
			tmpItem = dictIt->second;
		}
		catch (const Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"rebuildPlaylist() find channel item [%s] caught %s:%s"),
				itemKey.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			continue;
		}
		catch (const TianShanIce::ServerError& ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"rebuildPlaylist() find channel item [%s] caught %d:%s"),
				itemKey.c_str(), ex.errorCode, ex.message.c_str());
			continue;
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"rebuildPlaylist()find channel item [%s] caught %s"),
				itemKey.c_str(), ex.ice_name().c_str());
			continue;
		}


		//ChannelItem �Ѿ�����, ���ٲ���
		if(tmpItem.expiration > 0 && tmpItem.expiration < UserGetCurrentTime())
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "Item [%s] on channel [%s]��already expiration, broadcast[%s], expiration[%s]"), 
				tmpItem.setupInfo.contentName.c_str(), ident.name.c_str(), tmpItem.setupInfo.broadcastStart.c_str(), tmpItem.setupInfo.expiration.c_str());
			continue;
		}

		//�����ǰItem��BroadcastStartTi me > ǰһ��Item�� ������һ��δ��ʱ�䣬 ����Ҫ���м�� FilterItem��ȥ
		if(filterItemSize > 0 && tmpItem.broadcastStart > UserGetCurrentTime())
		{
			//��õ�ǰ��Ŀ����һ����Ŀ��ʱ���(ms),��ʱ�������������FilterItem;
			Ice::Long timeInterval = tmpItem.broadcastStart - previousBcastStart;
			TianShanIce::Application::Broadcast::ChannelItemEx filterItem;

			//����������������,��ֹ��һ�����ѡ�񵽵�FilterItem������(�ǳ����������ֻ��һ��FilterItem,����)
			int randomCount = 5;
			bool bRet = false;
			//
			while(randomCount)
			{
				bRet = getRandomFilterItem(filterItem);
				if(bRet == false)
				{
					continue;
					randomCount--;
				}
				// nInterval ��ʾ��ѡ�񵽵�FilterItem��Ƭ��(��λΪms)
				Ice::Long  nInterval = filterItem.setupInfo.outTimeOffset - filterItem.setupInfo.inTimeOffset;
				int nInsertFilterNumber = int(timeInterval / nInterval + 1);
				while(nInsertFilterNumber)
				{
					TianShanIce::Streamer::PlaylistItemSetupInfo newItemInfo;
					copyChannelItemToSetupInfo(tmpItem, newItemInfo);
					int userCtrlNum = _gUserCtrlNumGen.Generate();
					// ignore exception from calling pushBack() so that a failure on push item doesn't affect the whole setup.
					try
					{
						playlist->pushBack(userCtrlNum, newItemInfo);
						glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "channel item [%s] appended to playlist [%s], userctrlnum [%d]."), itemKey.c_str(), playlistId.c_str(), userCtrlNum);
					}
					catch(const TianShanIce::BaseException& ex)
					{
						glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s:%s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
						break;
					}
					catch(const ::Ice::Exception& ex)
					{
						glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str());
						break;
					}
					// add an item to PurchaseItemAssoc
					ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
					pia->ident.name = IceUtil::generateUUID();
					pia->ident.category = ICE_ChannelItemAssoc;
					pia->bcastPPIdent = ident;
					pia->channelItemKey = itemKey;
					pia->playlistCtrlNum = userCtrlNum;
					pia->lastModified = tmpItem.setupInfo.lastModified;
					try
					{
						_env._evitChannelItemAssoc->add(pia, pia->ident);
					}
					catch (const Freeze::DatabaseException& ex)
					{
						ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 306, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught  %s:%s"), pia->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
					}
					catch (const Ice::Exception& ex)
					{
						ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 307, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught %s"), pia->ident.name.c_str(), ex.ice_name().c_str());
					}
					nInsertFilterNumber --;
				}
				if(nInsertFilterNumber == 0)
					randomCount = 0;
				else
					randomCount--;
			}
		}

		TianShanIce::Streamer::PlaylistItemSetupInfo newItemInfo;
		copyChannelItemToSetupInfo(tmpItem, newItemInfo);
		int userCtrlNum = _gUserCtrlNumGen.Generate();
		previousBcastStart = tmpItem.broadcastStart;

		// ignore exception from calling pushBack() so that a failure on push item doesn't affect the whole setup.
		try
		{
			playlist->pushBack(userCtrlNum, newItemInfo);
			glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "channel item [%s] appended to playlist [%s], userctrlnum [%d]."), itemKey.c_str(), playlistId.c_str(), userCtrlNum);
		}
		catch(const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s:%s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			continue;
		}
		catch(const ::Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "append channel item [%s] on playlist[%s] caught %s"), itemKey.c_str(), playlistId.c_str(), ex.ice_name().c_str());
			continue;
		}
		// add an item to PurchaseItemAssoc
		ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
		pia->ident.name = IceUtil::generateUUID();
		pia->ident.category = ICE_ChannelItemAssoc;
		pia->bcastPPIdent = ident;
		pia->channelItemKey = itemKey;
		pia->playlistCtrlNum = userCtrlNum;
		pia->lastModified = tmpItem.setupInfo.lastModified;
		try
		{
			_env._evitChannelItemAssoc->add(pia, pia->ident);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 306, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught  %s:%s"), pia->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 307, BcastChPPLog("BcastPublishPoint", "append channel item assoc[%s] caught %s"), pia->ident.name.c_str(), ex.ice_name().c_str());
		}
		needAddItemCount--;
		currentItem = *sit;
	}
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, 
		"rebuildPlaylist() took %dms"), (int)(ZQ::common::now()- lstart));

	return true;
}

bool BcastPublishPointImpl::appendPlaylistItem(
	const TianShanIce::Application::Broadcast::ChannelItemEx& appendChnlItem, 
	const ::Ice::Current& c)
{
	Lock lock(*this);

	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "appendPlaylistItem(%s) enter"),
		appendChnlItem.setupInfo.contentName.c_str());

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, 
			"appendPlaylistItem() channel is not in service mode"));
		return false;
	}
	TianShanIce::StrValues chItemSequence;// = getItemSequence(Ice::Current());
	if(bIsNOVDSupCh) //indicates the NOVD supplemental Channel
	{
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastprx;
		bcastprx = getBcastPublishPointEx();
		if(bcastprx != NULL)
			chItemSequence = bcastprx->getItemSequence();	
	}
	else
	{
		chItemSequence = itemSequence;
	}
   
	//���֧��Window, ����ChannelItem�ĸ�������PlayList��Сֵ,��Append
    if(gBroadcastChCfg.windowsize > 0 && chItemSequence.size() > MINIMUMPLITEMCOUNT)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
			"BcastPublishpoint is support window, Ingore append"));
		return false;
	}
	// the playlist item info
	TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
	copyChannelItemToSetupInfo(appendChnlItem, setupInfo);
	const std::string& appendItemKey = appendChnlItem.key;

	bool isCalledFromSync = false;
	std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
	if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
		isCalledFromSync = true;

	bool bNeedSyncChannelTemp = bNeedSyncChannel;
	if (!isCalledFromSync)
		bNeedSyncChannel = true;

	int newCtrlNum = _gUserCtrlNumGen.Generate();
	try
	{
		playlist->pushBack(newCtrlNum, setupInfo);
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "item [%s: %d] appended on playlist [%s]"), 
			appendItemKey.c_str(), newCtrlNum, playlistId.c_str());
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "append item [%s: %d] on playlist [%s] caught %s: %s"), 
			appendItemKey.c_str(), newCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "append item [%s: %d] on playlist [%s] caught %s"), 
			appendItemKey.c_str(), newCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
		return false;
	}

	// 2. record the playlist item information in a purchase item
	ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
	pia->ident.name = IceUtil::generateUUID();
	pia->ident.category = ICE_ChannelItemAssoc;
	pia->bcastPPIdent = ident;
	pia->channelItemKey = appendChnlItem.key;
	pia->playlistCtrlNum = newCtrlNum;
	pia->lastModified = appendChnlItem.setupInfo.lastModified;
	try
	{
		_env._evitChannelItemAssoc->add(pia, pia->ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 306, BcastChPPLog("BcastPublishPoint", "append channel item assoc [%s] caught  %s:%s"), pia->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublishPoint", 307, BcastChPPLog("BcastPublishPoint", "append channel item assoc [%s] caught %s"), pia->ident.name.c_str(), ex.ice_name().c_str());
		return false;
	}

	// set bNeedSyncChannel = false to indicates operation successfully
	if (!isCalledFromSync)
	{
		bNeedSyncChannel = bNeedSyncChannelTemp;
	}
	return true;
}

bool BcastPublishPointImpl::insertPlaylistItem(
	const ::std::string& insertPosKey, 
	const TianShanIce::Application::Broadcast::ChannelItemEx& insertChnlItem, 
	const ::Ice::Current& c)
{
	Lock lock(*this);

	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "insertPlaylistItem(%s before %s) enter"), 
		insertChnlItem.key.c_str(), insertPosKey.c_str());

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "insertPlaylistItem() channel is not in service mode"));
		return false;
	}
	// the playlist item info
	TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
	copyChannelItemToSetupInfo(insertChnlItem, setupInfo);

	const std::string& insertItemKey = insertChnlItem.key;

	bool isCalledFromSync = false;
	std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
	if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
		isCalledFromSync = true;

	// set bNeedSyncChannel to true now, suppose the operation will be failed
	if (!isCalledFromSync)
		bNeedSyncChannel = true;

	// find the purchase item' identity before which will be inserted
	/*Playlist�п��ܻ����ظ�ChannelItem
	 ���� 1 , 2, 3 ,4, 5,1 , 2, 3 ,4, 5 ��3�ĺ������һ��6 
	 ���Ӧ���� 1 , 2, 3 ,6, 4, 5,1 , 2, 3 ,6, 4,5*/
	std::vector<Ice::Identity> insertPosIdents;
	if(!ChnlItem2ChItemAssocIdent(insertPosKey, insertPosIdents))
		return false;
	std::vector<Ice::Identity>::iterator itorInsertPosIdent;
	//�����λ����Playlist����
	if(insertPosIdents.size() > 0)
	{
		for(itorInsertPosIdent = insertPosIdents.begin(); itorInsertPosIdent != insertPosIdents.end(); itorInsertPosIdent++)
		{
			Ice::Identity insertPosIdent = *itorInsertPosIdent;
			int insertPosCtrlNum;
			try
			{
				TianShanIce::Application::Broadcast::ChannelItemAssocPrx insertPosPrx = NULL;
				insertPosPrx = TianShanIce::Application::Broadcast::ChannelItemAssocPrx::checkedCast(_env._adapter->createProxy(insertPosIdent));
				insertPosCtrlNum = insertPosPrx->getCtrlNum();
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "get insert position caught %s"), 
					ex.ice_name().c_str());
				return false;
			}

			// the ctrlnum for new playlist item
			Ice::Int newCtrlNum = _gUserCtrlNumGen.Generate();
			try
			{
				playlist->insert(newCtrlNum, setupInfo, insertPosCtrlNum);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s:%s"), 
					insertItemKey.c_str(), newCtrlNum, insertPosKey.c_str(), insertPosCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				return false;
			}
			catch(const ::Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s"), 
					insertItemKey.c_str(), newCtrlNum, insertPosKey.c_str(), insertPosCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
				return false;
			}
			glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "item [%s: %d] inserted before [%s: %d] on playlist [%s]"), 
				insertItemKey.c_str(), newCtrlNum, insertPosKey.c_str(), insertPosCtrlNum, playlistId.c_str());

			// save purchase item
			ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
			pia->ident.name = IceUtil::generateUUID();
			pia->ident.category = ICE_ChannelItemAssoc;
			pia-> bcastPPIdent= ident;
			pia->channelItemKey = insertItemKey;
			pia->playlistCtrlNum = newCtrlNum;
			pia->lastModified = insertChnlItem.setupInfo.lastModified;
			try
			{
				_env._evitChannelItemAssoc->add(pia, pia->ident);
			}
			catch (const Freeze::DatabaseException& ex)
			{
				// if save purchase item caught exception, you should remove purchase here, because no purchase item records
				// the playlist list item information, it will affect the streaming.
				char szBuf[MAX_PATH];
				szBuf[MAX_PATH - 1] = '\0';
				snprintf(szBuf, MAX_PATH - 1, "save channel item assoc caught %s: %s, freeze db exception to destroy purchase", 
					ex.ice_name().c_str(), ex.message.c_str());
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "%s"), szBuf);

				return false;
			}
		}

		bNeedSyncChannel = false;
	}
/*	//Insert��λ����Playlist��û��,����Insert֮����Ǹ�Item������, ��Ҫ��ͬ��
	else
	{
      bNeedSyncChannel = true;  
	}*/
	return true;
}

bool BcastPublishPointImpl::removePlaylistItem(
	const ::std::string& removeItemKey, 
	const ::Ice::Current& c)
{
	{
		Lock lock(*this);

		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "removePlaylistItem(%s) enter"), 
			removeItemKey.c_str());

		// check whether purchase is in service mode
		if(!bInService)
		{
			glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "removePlaylistItem() channel is not in service mode"));
			return false;
		}

		bool isCalledFromSync = false;
		std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
		if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
			isCalledFromSync = true;

		// find the remove purchase item identity

		/*Playlist�п��ܻ����ظ�ChannelItem
		���� 1 , 2, 3 ,4, 5,1 , 2, 3 ,4, 5 ��3ɾ�� 
		���Ӧ���� 1 , 2, 4, 5,1 , 2, 4, 5*/

		std::vector<Ice::Identity> removeItemIdents;
		if(!ChnlItem2ChItemAssocIdent(removeItemKey, removeItemIdents))
			return false;
		std::vector<Ice::Identity>::iterator itorRemovePosIdent;
		for(itorRemovePosIdent = removeItemIdents.begin(); itorRemovePosIdent != removeItemIdents.end(); itorRemovePosIdent++)
		{
			Ice::Identity removeItemIdent = *itorRemovePosIdent;
			TianShanIce::Application::Broadcast::ChannelItemAssocPrx removeItemPrx = NULL;
			Ice::Int removeCtrlNum;
			try
			{
				removeItemPrx = TianShanIce::Application::Broadcast::ChannelItemAssocPrx::checkedCast(_env._adapter->createProxy(removeItemIdent));
				removeCtrlNum = removeItemPrx->getCtrlNum();
			}
			catch(const ::Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "get remove position caught %s"), 
					ex.ice_name().c_str());
				continue;
			}

			try
			{
				playlist->erase(removeCtrlNum);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "remove [%d] from playlist [%s] caught %s: %s"), 
					removeCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "remove [%d] from playlist [%s] caught %s"), 
					removeCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
			}
			glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "item [%s: %d] removed form playlist [%s]"), 
				removeItemKey.c_str(), removeCtrlNum, playlistId.c_str());

			try
			{
				removeItemPrx->destroy();
			}
			catch (const Ice::Exception& ex)
			{
				// if destroy purchase item caught exception, just ignore with a warning message
				// because a excrescent purchase item in db doesn't affect streaming
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "remove channel item assoc caught %s"), 
					ex.ice_name().c_str());
			}
		}
	}
	 OnEndOfItem(-2, c);
	 glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "removePlaylistItem(%s) leave"), 
		 removeItemKey.c_str());
	return true;
}

bool BcastPublishPointImpl::replacePlaylistItem(
	const ::std::string& oldItemKey, 
	const TianShanIce::Application::Broadcast::ChannelItemEx& replaceChnlItem, 
	const ::Ice::Current& c)
{
	Lock lock(*this);
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "replacePlaylistItem(%s -> %s) enter"), 
		oldItemKey.c_str(), replaceChnlItem.key.c_str());

	// check whether purchase is in service mode
	if(!bInService)
	{
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "removePlaylistItem() channel is not in service mode"));
		return false;
	}

	// the playlist item info
	TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
	copyChannelItemToSetupInfo(replaceChnlItem, setupInfo);

	const std::string& newItemKey = replaceChnlItem.key;

	bool isCalledFromSync = false;
	std::map<std::string, std::string>::const_iterator ctx_itor = c.ctx.find(SYS_PROP(SyncPlaylistKey));
	if (c.ctx.end() != ctx_itor && ctx_itor->second == SyncPlaylistValue)
		isCalledFromSync = true;
	// set bNeedSyncChannel to true now, suppose the operation will be failed
	bool bNeedSyncChannelTemp = bNeedSyncChannel;
	if (!isCalledFromSync)
		bNeedSyncChannel = true;
  
	// find the purchase item' identity before which will be inserted
	/*Playlist�п��ܻ����ظ�ChannelItem
	���� 1 , 2, 3 ,4, 5,1 , 2, 3 ,4, 5 ��3�滻Ϊ6 
	���Ӧ���� 1 , 2, 6, 4, 5,1 , 2, 6, 4,5*/

	std::vector<Ice::Identity> oldPosIdents;
	if(!ChnlItem2ChItemAssocIdent(oldItemKey, oldPosIdents))
		return false;

	std::vector<Ice::Identity>::iterator itoroldPosIdent;
	for(itoroldPosIdent = oldPosIdents.begin(); itoroldPosIdent != oldPosIdents.end(); itoroldPosIdent++)
	{
		Ice::Identity oldPosIdent = *itoroldPosIdent;
		// 1. to insert a new item before the old item
		TianShanIce::Application::Broadcast::ChannelItemAssocPrx oldChAssocItemPrx = NULL;
		Ice::Int oldCtrlNum;
		try
		{
			oldChAssocItemPrx = TianShanIce::Application::Broadcast::ChannelItemAssocPrx::checkedCast(_env._adapter->createProxy(oldPosIdent));
			oldCtrlNum = oldChAssocItemPrx->getCtrlNum();
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "get old item position information caught %s"), 
				ex.ice_name().c_str());
			return false;
		}

		Ice::Int newCtrlNum = _gUserCtrlNumGen.Generate();
		try
		{
			playlist->insert(newCtrlNum, setupInfo, oldCtrlNum);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s:%s"), 
				newItemKey.c_str(), newCtrlNum, oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			continue;
		}
		catch(const ::Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "insert [%s: %d] before [%s: %d] on playlist [%s] caught %s"), 
				newItemKey.c_str(), newCtrlNum, oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
			continue;
		}
		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "item [%s: %d] inserted before [%s: %d] on playlist [%s]"), 
			newItemKey.c_str(), newCtrlNum, oldItemKey.c_str(), oldCtrlNum, playlistId.c_str());

		// 2. to remove the old item
		bool bOldPIRemoved = false;
		try
		{
			playlist->erase(oldCtrlNum);
			bOldPIRemoved = true;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "remove [%s: %d] from playlist [%s] caught %s: %s"), 
				oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "remove [%s: %d] from playlist [%s] caught %s"), 
				oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
		}

		if (bOldPIRemoved)
		{
			glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "item [%s: %d] removed form playlist [%s]"), 
				oldItemKey.c_str(), oldCtrlNum, playlistId.c_str());
			try
			{
				oldChAssocItemPrx->destroy();
			}
			catch (const Ice::Exception& ex)
			{
				// if destroy purchase item caught exception, just ignore with a warning message
				// because a excrescent purchase item in db doesn't affect streaming
				glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "remove purchase item caught %s"), 
					ex.ice_name().c_str());
			}

			// save purchase item
			ChannelItemAssocImplPtr pia = new ChannelItemAssocImpl(_env);
			pia->ident.name = IceUtil::generateUUID();
			pia->ident.category = ICE_ChannelItemAssoc;
			pia->bcastPPIdent = ident;
			pia->channelItemKey = newItemKey;
			pia->playlistCtrlNum = newCtrlNum;
			pia->lastModified = replaceChnlItem.setupInfo.lastModified;
			try
			{
				_env._evitChannelItemAssoc->add(pia, pia->ident);
			}
			catch (const Freeze::DatabaseException& ex)
			{
				char szBuf[MAX_PATH];
				szBuf[MAX_PATH - 1] = '\0';
				snprintf(szBuf, MAX_PATH - 1, "save channel item assoc  caught %s: %s, freeze db exception to destroy purchase", 
					ex.ice_name().c_str(), ex.message.c_str());
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "%s"), szBuf);
				continue;
			}
		}
		else 
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "replacePlaylistItem(), caught remove old item [%s: %d] failed so we have to remove the newly inserted item [%s: %d]"), 
				oldItemKey.c_str(), oldCtrlNum, newItemKey.c_str(), newCtrlNum);
			try
			{
				playlist->erase(newCtrlNum);
				glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "newly inserted item [%s: %d] has been removed"), 
					newItemKey.c_str(), newCtrlNum);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "remove newly inserted item [%s: %d] from playlist [%s] caught %s: %s"), 
					oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "remove newly inserted item [%s: %d] from playlist [%s] caught %s"), 
					oldItemKey.c_str(), oldCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
			}
		}
    }
	bNeedSyncChannel = false;
    return true;
}
bool BcastPublishPointImpl::ChnlItem2ChItemAssocIdent(const ::std::string& channelItemKey, std::vector<Ice::Identity>& chItemassociIdents) const
{
	// find the purchase item' identity according to position Key
	try
	{
		std::vector<Ice::Identity> idents;
		idents = _env._idxBcastPublishPoint2ItemAssoc->find(ident);

		if (idents.size() < 1)
		{
			glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "there is no ChannelItemAsssoc item relating to the channel item [%s]"), 
				channelItemKey.c_str());
			return false;
		}
		std::vector<Ice::Identity>::iterator identItor;
		bool bFound = false;
		for(identItor = idents.begin(); identItor != idents.end(); identItor++)
		{
			std::string channelItemkey;
			TianShanIce::Application::Broadcast::ChannelItemAssocPrx chItemAssocPrx;
			chItemAssocPrx = TianShanIce::Application::Broadcast::ChannelItemAssocPrx::checkedCast(_env._adapter->createProxy(*identItor));
			std::string ChannelItemKey = chItemAssocPrx->getChannelItemKey();
			if(channelItemKey.compare(ChannelItemKey) == 0)
			{
				bFound = true;
				chItemassociIdents.push_back(*identItor);
			}
		}
		if (!bFound)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "there is no purchase item relating to the channel item [%s]"), 
				channelItemKey.c_str());
			return false;
		}

	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "find position caught %s:%s"), 
			ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "get position caught %s"), 
			ex.ice_name().c_str());
		return false;
	}
	return true;
}
bool  BcastPublishPointImpl::destroyChannelItemAssoc()
{
	Lock lock(*this);
	try
	{
		std::vector<Ice::Identity> idents;
		idents = _env._idxBcastPublishPoint2ItemAssoc->find(ident);

		if (idents.size() < 1)
		{
			return false;
		}
		std::vector<Ice::Identity>::iterator identItor;
		bool bFound = false;
		for(identItor = idents.begin(); identItor != idents.end(); identItor++)
		{
			try
			{
				TianShanIce::Application::Broadcast::ChannelItemAssocPrx chItemAssocPrx;
				chItemAssocPrx = TianShanIce::Application::Broadcast::ChannelItemAssocPrx::checkedCast(_env._adapter->createProxy(*identItor));
				chItemAssocPrx->destroy();
			}
			catch(const ::Ice::Exception&)
			{
			}
		}
	}
	catch (const Freeze::DatabaseException&)
	{
		return false;
	}
	catch (const Ice::Exception&)
	{
		return false;
	}
	return true;
}
bool  BcastPublishPointImpl::destroyChannelItem()
{
	Lock lock(*this);
	TianShanIce::StrValues::iterator chitemItor;
	for(chitemItor = itemSequence.begin(); chitemItor != itemSequence.end(); chitemItor++)
	{
		std::string rmvItemKey, rmvItemName;

		rmvItemKey = ident.name + CHANNELITEM_KEY_SEPARATOR + *chitemItor;
		rmvItemName = *chitemItor;
		STRTOLOWER(rmvItemKey);
		STRTOLOWER(rmvItemName);

		glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "remove item [%s]"), rmvItemName.c_str());

		// remove from channel item dict
		try
		{
			LockT<RecMutex>  lk(_env._dictLock);
			_env._pChannelItemDict->erase(rmvItemKey);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "remove item [%s] on channel caught %s: %s"), 
				rmvItemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "remove item [%s] on channel  caught %s"), 
				rmvItemName.c_str(),  ex.ice_name().c_str());
		}
	}
	return true;
}

bool BcastPublishPointImpl::syncPlaylist(const ::Ice::Current& c)
{
/*	Lock lock(*this);

	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "syncPlaylist() enter"));

	// check whether purchase is in service mode
	if(!bInService)
	{
	glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "bInService = false, channel is not in service mode"));
	return false;
	}

	#if !TestSyncPlaylist
	if (!bNeedSyncChannel)
	{
	glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "syncPlaylist() found bNeedSyncChannel = false"));
	return true;
	}
	#endif

	// 1. get channel item list from channel db
	TianShanIce::StrValues itemsInChannel;// = getItemSequence(c);
	if(bIsNOVDSupCh) //indicates the NOVD supplemental Channel
	{
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastprx;
		bcastprx = getBcastPublishPointEx();
		if(bcastprx != NULL)
			itemsInChannel = bcastprx->getItemSequence();	
	}
	else
	{
		itemsInChannel = itemSequence;
	}

	if(gBroadcastChCfg.IsRepeat)
	{
		int currentCtrlNum = 0;
		TianShanIce::IValues plSequence;
		try
		{	
			currentCtrlNum = playlist->current();
		}
		catch (TianShanIce::ServerError&ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"syncPlaylist()get playlist caught [%d,%s]"),ex.errorCode, ex.message.c_str());
			return false;
		}
		catch(Ice::Exception&ex)
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, 
				"syncPlaylist()get playlist caught [%s]"),ex.ice_name().c_str());
			return false;
		}
		std::vector<Ice::Identity> identsByCtrlNum;
		TianShanIce::Application::Broadcast::ChannelItemEx currentChItem;
		identsByCtrlNum = _env._idxCtrlNumber2ChannelItemAssoc->find(currentCtrlNum);
		if (identsByCtrlNum.size() != 1)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublishPointImpl, "ctrlNum [%d] in playlist [%s] have [%d] corresponding purchase item"), 
				currentCtrlNum, playlistId.c_str(), identsByCtrlNum.size());
			return false;
		}
		TianShanIce::Application::Broadcast::ChannelItemAssocData data;
		try
		{
			TianShanIce::Application::Broadcast::ChannelItemAssocPrx pPurcharseItemPrx = 
				TianShanIce::Application::Broadcast::ChannelItemAssocPrx::checkedCast(_env._adapter->createProxy(identsByCtrlNum[0]));
			currentChItem = pPurcharseItemPrx->getChannelItem();
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublishPointImpl, "get purchase item [%s] context caught %s"), 
				identsByCtrlNum[0].name.c_str(), ex.ice_name().c_str());
			return false;
		}

		TianShanIce::StrValues::iterator itorCurrent;
		itorCurrent = std::find(itemsInChannel.begin(), itemsInChannel.end(),currentChItem.setupInfo.contentName);
		if(itorCurrent == itemsInChannel.end()) //�����ǰ�����ChannelItem���Ҳ���,�����ͬ��.
			return false;	
		
		TianShanIce::StrValues itemsInRepeatChannel;		
		if(itorCurrent == itemsInChannel.end() - 1)//����ҵ�����ChannelItem�����һ��,����Ҫ����PlayList
		{
			itemsInRepeatChannel.push_back(*itorCurrent);
            itemsInRepeatChannel.resize(itemsInChannel.size() + 1);
			std::copy(itemsInChannel.begin(), itemsInChannel.end(),itemsInRepeatChannel.begin()+1);
		}
		else
		{
			itemsInRepeatChannel.assign(itorCurrent, itemsInChannel.end());
		}
		itemsInChannel.clear();
		itemsInChannel=itemsInRepeatChannel;
	}
	int i = 0, count = 0;
	// 2. get all channel item's context
	std::vector< TianShanIce::Application::Broadcast::ChannelItemEx> chnlItemCtxs;
	chnlItemCtxs.reserve(itemsInChannel.size());

	for (i = 0, count = itemsInChannel.size(); i < count; i ++)
	{
		LockT<RecMutex> lk(_env._dictLock);

		std::string chnlItemKey = mainChName + CHANNELITEM_KEY_SEPARATOR + itemsInChannel[i];

		ZQBroadCastChannel::ChannelItemDict::iterator dictIt = _env._pChannelItemDict->find(chnlItemKey);
		if (_env._pChannelItemDict->end() == dictIt)
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPublishPointImpl, "channel item [%s]'s context could not be found"), 
				chnlItemKey.c_str());
		}
		chnlItemCtxs.push_back(dictIt->second);
	}

	// 3. get playlist item ctrl number vector
	TianShanIce::IValues ctrlNums;
	ctrlNums.clear();
	try
	{
		ctrlNums = playlist->getSequence();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublishPointImpl, "get playlist [%s] sequence caught %s"), 
			playlistId.c_str(), ex.ice_name().c_str());
		return false;
	}

	// store all purchase item context
	std::vector<TianShanIce::Application::Broadcast::ChannelItemAssocData> chItemAssocDatas;
	chItemAssocDatas.reserve(ctrlNums.size());

	// notice you must add purchase item follow the order of ctrlnum
	for (i = 0, count = ctrlNums.size(); i < count; i ++)
	{
		std::vector<Ice::Identity> identsByCtrlNum;
		identsByCtrlNum = _env._idxCtrlNumber2ChannelItemAssoc->find(ctrlNums[i]);
		if (identsByCtrlNum.size() != 1)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublishPointImpl, "ctrlNum [%d] in playlist [%s] have [%d] corresponding purchase item"), 
				ctrlNums[i], playlistId.c_str(), identsByCtrlNum.size());
			return false;
		}
		TianShanIce::Application::Broadcast::ChannelItemAssocData data;
		try
		{
			TianShanIce::Application::Broadcast::ChannelItemAssocPrx pPurcharseItemPrx = 
				TianShanIce::Application::Broadcast::ChannelItemAssocPrx::checkedCast(_env._adapter->createProxy(identsByCtrlNum[0]));
			data = pPurcharseItemPrx->getData();
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublishPointImpl, "get purchase item [%s] context caught %s"), 
				identsByCtrlNum[0].name.c_str(), ex.ice_name().c_str());
			return false;
		}
		chItemAssocDatas.push_back(data);
	}

	bool isAllSyncOK = true;

	// start the sync process
	int j = 0, k = 0;
	while (j < (int)chnlItemCtxs.size() && k < (int)chItemAssocDatas.size())
	{
		// if name equal, further check if the item has been replaced
		// �жϵ�ǰ�±�ָ���ChannelItem name �� PlaylistItem name��ͬ����
		// �����ж�lastModified�Ƿ���ͬ
		if (chnlItemCtxs[j].key == chItemAssocDatas[k].channelItemKey)
		{
			if (chnlItemCtxs[j].setupInfo.lastModified != chItemAssocDatas[k].lastModified)
			{
				// lastModified����ͬ˵��ChannelItem�����Ա��޸���
				if (!replacePlaylistItem(chItemAssocDatas[k].channelItemKey, chnlItemCtxs[j], c))
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPublishPointImpl, "syncPlaylist(%s -> %s) caught replacePlaylistItem(%s -> %s) error, need further syncPlaylist()"), 
						chItemAssocDatas[k].channelItemKey.c_str(), chnlItemCtxs[j].key.c_str(), chItemAssocDatas[k].channelItemKey.c_str(), chnlItemCtxs[j].key.c_str());
					isAllSyncOK = false;
				}
			}
			j ++;
			k ++;
			continue;
		}

		// ���Playlist item����Ӧ��channel item name�뵱ǰ�Ĳ���
		int i, count;
		bool plInChannel = false; // to indicates whether or not the playlist item is in channel item list
		for (i = j, count = chnlItemCtxs.size(); i < count; i ++)
		{	// notice here the initail value of i is j
			// means the channel item list is a sub list which after the position of current channel item
			if (chItemAssocDatas[k].channelItemKey == chnlItemCtxs[i].key)
			{
				// ����Playlist item����Ӧ��channel item nameȷʵ������channel item list��
				// ��˵����ǰ��ָ���channel item������ӵ�
				plInChannel = true;
				break;
			}
		}

		if (plInChannel)
		{
			// current channel item is a new item, so we have to insert it to playlist
			if (!insertPlaylistItem(chItemAssocDatas[k].channelItemKey, chnlItemCtxs[j], c))
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPublishPointImpl, "syncPlaylist() caught insertPlaylistItem(%s before %s) error, need further syncPlaylist()"), 
					chnlItemCtxs[j].key.c_str(), chItemAssocDatas[k].channelItemKey.c_str());
				isAllSyncOK = false;
			}
			j ++;
		}
		else 
		{
			// current playlist item is not in channel item list, so we have to erase it
			if (!removePlaylistItem(chItemAssocDatas[k].channelItemKey, c))
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPublishPointImpl, "syncPlaylist() caught removePlaylistItem(%s) error, need further syncPlaylist()"), 
					chItemAssocDatas[k].channelItemKey.c_str());
				isAllSyncOK = false;
			}
			k ++;
		}
	}

	while (j < (int)chnlItemCtxs.size())
	{
		if (!appendPlaylistItem(chnlItemCtxs[j], c))
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPublishPointImpl, "syncPlaylist() caught appendPlaylistItem(%s) error, need further syncPlaylist()"), 
				chnlItemCtxs[j].key.c_str());
			isAllSyncOK = false;
		}
		j ++;
	}

	while (k < (int)chItemAssocDatas.size())
	{
		if (!removePlaylistItem(chItemAssocDatas[k].channelItemKey, c))
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPublishPointImpl, "syncPlaylist() caught removePlaylistItem(%s) error, need further syncPlaylist()"), 
				chItemAssocDatas[k].channelItemKey.c_str());
			isAllSyncOK = false;
		}
		k ++;
	}

	if (isAllSyncOK)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPublishPointImpl, "syncPlaylist() OK"));
		bNeedSyncChannel = false;
		return true;
	}
	else 
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastPublishPointImpl, "some error occured when doing current syncPlaylist()"));
		bNeedSyncChannel = true;
		return false;
	}*/
	return true;
}
TianShanIce::Application::Broadcast::BcastPublishPointExPrx 
 BcastPublishPointImpl::getBcastPublishPointEx()const
{
	Lock lock(*this);
	Ice::Identity ident;
	ident.name = mainChName;
	ident.category = ICE_BcastChannelPublishPoint;

	TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastPPPrx = NULL;
	try
	{	
		Ice::ObjectPrx prx = _env._adapter->createProxy(ident);
		if (!prx)
		{
			return NULL;
		}

		bcastPPPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(prx);
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "fail to get bcastPublishPointEx proxy, ice exception '%s'"),
			ex.ice_name().c_str());

		bcastPPPrx = NULL;
	}
	return bcastPPPrx;
}

bool  BcastPublishPointImpl::isNOVDMainCh(const Ice::Current&)
{
	RLock sync(*this);
	return bIsNVODMainCh;
}
bool  BcastPublishPointImpl::isNOVDSuppCh(const Ice::Current&)
{
	RLock sync(*this);
	return bIsNOVDSupCh;
}

bool BcastPublishPointImpl::isInService(const ::Ice::Current&)
{
	RLock sync(*this);
	return bInService;
}
void BcastPublishPointImpl::resetchannel()
{
	playlist = NULL;
		playlistId = "";
		bInService = false;
		bNeedSyncChannel = false;
		timestamp = 0;
		session = NULL;
		currentItem ="";
	TianShanIce::Properties::iterator itorProps;
	itorProps = properties.find(SOAP_SessionID);
	if(itorProps != properties.end())
	{
		properties.erase(itorProps);
	}

	itorProps = properties.find(SOAP_StreamNetID);
	if(itorProps != properties.end())
	{
		properties.erase(itorProps);
	}
	_env._watchDog.watchBcastChannel(ident, 0);
}
bool BcastPublishPointImpl::checkChannelItem(TianShanIce::Application::ChannelItem& channelItem)
{
	std::string contentname = channelItem.contentName;

	int nposB, nPosE;
	nposB = contentname.find('/');
	if(nposB < 0)//Like "test"
	{
		contentname = DEFAULTVOLUME + contentname;
	}
	else
	{
		int nlen = contentname.size();
		std::string subContentName = contentname.substr(nposB + 1, nlen - nposB -1);
		nPosE = subContentName.find('/');
		if(nPosE < 0)//like "test/, /test, te/st"
		{
           contentname = DEFAULTVOLUME + contentname;
		}
		else
		{
			if(nPosE == 0)//like "test//, //test, te//st"
			{
               contentname = DEFAULTVOLUME + contentname;
			}
			/*
			else // like "/volume/test"
			{}
			*/
		}
	}
    channelItem.contentName = contentname;
	return true;
}
bool BcastPublishPointImpl::removeChItemAssocByCtrlNumber(int removeCtrlNum)
{

	//ɾ��Playlist��Item, ���ֿ��ܣ�
	//1. �ϴ���ɾ��Item��ʱ�����Item�ڲ�,�������ݿ��ChannelItemAssoc����ɾ,����PlayList�е�û�б�ɾ��
	//2. BcastChannel Serviceͣ������������ʱ��StreamSmith�Ѿ���������Item,����BcastChannel Serviceû���������Ϣ
	try
	{
		playlist->erase(removeCtrlNum);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "OnEndOfItem() remove [%d] from playlist [%s] caught %s: %s"), 
			removeCtrlNum, playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  BcastChPPLog(BcastPublishPointImpl, "OnEndOfItem()remove [%d] from playlist [%s] caught %s"), 
			removeCtrlNum, playlistId.c_str(), ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_INFO, BcastChPPLog(BcastPublishPointImpl, "OnEndOfItem() item [%d] removed from playlist [%s]"), 
		removeCtrlNum, playlistId.c_str());

	std::vector<Ice::Identity> idents;
	try
	{
		idents = _env._idxCtrlNumber2ChannelItemAssoc->find(removeCtrlNum);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "CtrlNumber [%d] to ChannelItem caught %s: %s"), 
			removeCtrlNum, ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "CtrlNumber [%d] to ChannelItem caught %s"), 
			removeCtrlNum, ex.ice_name().c_str());
		return false;
	}

	if (idents.size() == 0)
	{
//		glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "no ChannelItem associated with CtrlNumber [%d]"), 
//			removeCtrlNum);
		return false;
	}
	else
	{
		//ɾ��ChannelItemAssoc
		TianShanIce::Application::Broadcast::ChannelItemAssocPrx chItemprx;

		Ice::ObjectPrx prx = _env._adapter->createProxy(idents[0]);
		if (!prx)
		{
			return false;
		}
		try
		{
			chItemprx = TianShanIce::Application::Broadcast::ChannelItemAssocPrx::uncheckedCast(prx);
			chItemprx->destroy();
		}
		catch (Ice::Exception&)
		{
			return false;
		}
	}
	return true;
}

bool 
BcastPublishPointImpl::activate(const ::Ice::Current& c)
{
//  Lock sync(*this);
  glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "enter activate()"));

  // check if weiwoo live
  bool isWeiwooLive = true;
  TianShanIce::Streamer::StreamPrx streamPrx;

  if(session)
  {
	  try
	  {
		  session->ice_ping();
		  session->renew(_env._sessionRewTime);
	  }
	  catch (const Ice::ObjectNotExistException& ex)
	  {
		  isWeiwooLive = false;
		  glog(ZQ::common::Log::L_WARNING, "activate() ice_ping weiwoo session caught %s", ex.ice_name().c_str());
	  }
	  catch (const Ice::Exception& ex)
	  {
		  glog(ZQ::common::Log::L_WARNING, "activate() ice_ping weiwoo session caught %s", ex.ice_name().c_str());
	  }
	  if(isWeiwooLive)
	  {
		  try
		  {
			  streamPrx = session->getStream();
		  }
		  catch(TianShanIce::BaseException&ex)
		  {
			  isWeiwooLive = false;
			  glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,
				  "activate() broadcast PublishPoint caught error [%s,%s]"), ex.ice_name().c_str(), ex.message.c_str());

		  }
		  catch(Ice::Exception&ex)
		  {
			  isWeiwooLive = false;
			  glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,
				  "activate() '%s' caught ice exception '%s'"), ident.name.c_str(), ex.ice_name().c_str());
		  }
	  }
  }
  else
  {
	  isWeiwooLive = false;
  }

  // ��־ΪIn Service,������ Repeatģʽ, ����Weiwoo Session�ֲ�������,
  // ˵��֮ǰBroadcastChannel״̬���ڻ״̬, ��Ҫ������������
  if(bInService && gBroadcastChCfg.IsRepeat && !isWeiwooLive)
  {
	  try
	  {
		  start(c);
	  }
	  catch(TianShanIce::ServerError&ex)
	  {
		  glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,
			  "activate() broadcast PublishPoint caught error [%s,%s]"), ex.ice_name().c_str(), ex.message.c_str());
	  }
	  catch (Ice::Exception&ex)
	  {
		  glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl,
			  "activate() '%s' caught ice exception '%s'"), ident.name.c_str(), ex.ice_name().c_str());
	  }
  }
  //���Streamer is playing states, ����һ��Playlist��ͬ��.
  if(isWeiwooLive && (gBroadcastChCfg.windowsize > 0 || gBroadcastChCfg.IsRepeat)) 
  {
	  bNeedSyncChannel = true;
	  OnEndOfItem(-2, c);
  }

  _env._watchDog.watchBcastChannel(ident, _env._sessionRewTime - 10000);

  glog(ZQ::common::Log::L_DEBUG, BcastChPPLog(BcastPublishPointImpl, "Leave activate()"));
  return true;
}

bool 
BcastPublishPointImpl::getRandomFilterItem(TianShanIce::Application::Broadcast::ChannelItemEx& filterItem)
{
	//�õ����е�FilterItem��Ϣ
	TianShanIce::StrValues filterItems;
	ChannelItemDict::const_iterator dictIt;
	int nRandom;
	srand( (unsigned)time(NULL)); 
	try
	{
		filterItems = _env._FilterItems->getFilterItemSequence();
	}
	catch (Ice::Exception&ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "failed to get filter item sequence caught %s"),
			ex.ice_name().c_str());
		return false;
	}
    int filterSize = filterItems.size();
	if(filterSize < 1)
		return false;

	nRandom = rand()%filterSize;
	std::string itemKey = filterItems[nRandom];

	// find channel item according to the item key.
	try
	{
		LockT<RecMutex> lk(_env._dictLock);
		dictIt = _env._pChannelItemDict->find(itemKey); 
		// if channel item not found in safestore, should throw server error
		if (dictIt == _env._pChannelItemDict->end())
		{
			glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "channel item [%s] not found in dict"), itemKey.c_str());
			return false;
		}
		filterItem = dictIt->second;
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "find channel item [%s] caught %s:%s"), itemKey.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const TianShanIce::ServerError& ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "find channel item [%s] caught %d:%s"), itemKey.c_str(), ex.errorCode, ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "find channel item [%s] caught %s"), itemKey.c_str(), ex.ice_name().c_str());
		return false;
	}
    return true;
}
void  
BcastPublishPointImpl::pingStream(const ::Ice::Current& c)
{
	try
	{
		if(playlist)
			playlist->ice_ping();
	}
	catch (Ice::ObjectNotExistException& ex)
	{
	  glog(ZQ::common::Log::L_ERROR, BcastChPPLog(BcastPublishPointImpl, "playlist [%s] ping caught ice exception %s'"), 
		  playlistId.c_str(), ex.ice_name().c_str());
	  resetchannel();	 
	}
	catch(Ice::Exception& ex)
	{
	   glog(ZQ::common::Log::L_WARNING, BcastChPPLog(BcastPublishPointImpl, "playlist [%s] ping caught ice exception %s'"), 
			playlistId.c_str(),ex.ice_name().c_str());
	}
}

}