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

// Branch: $Name:BcastPublisherImpl.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastPublisherImpl.cpp $
// 
// 8     11/25/14 2:18p Li.huang
// 
// 7     11/14/14 9:42a Li.huang
// 
// 6     10/22/14 10:56a Li.huang
// 
// 5     10/21/14 3:32p Li.huang
// 
// 4     10/17/14 3:37p Li.huang
// 
// 3     5/30/14 4:43p Li.huang
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
// 3     10-03-19 11:46 Li.huang
// remove evictor lock
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================

#include "BroadCastChannelEnv.h"
#include "BcastPublishPointImpl.h"
#include "NVODChannelPublishPointImpl.h"
#include "BcastPublisherImpl.h"
#include "BroadcastChCfg.h"
#include "BcastChDef.h"

extern ZQ::common::Config::Loader<BroadcastChCfg> gBroadcastChCfg;

#define BcastCh_ExpirationTime (3600*1000)
namespace ZQBroadCastChannel
{ 

BcastPublisherImpl::BcastPublisherImpl(BroadCastChannelEnv& bcastChenv):
_env(bcastChenv)
{
}

BcastPublisherImpl::~BcastPublisherImpl(void)
{
}
TianShanIce::Application::PublishPointPrx
BcastPublisherImpl::publish(const ::std::string& name,
												   ::Ice::Int maxBitrate,
												   const ::std::string& desc,
												   const Ice::Current& current)
{
	ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisher", 300, "not implement");
	return 0;
}

TianShanIce::Application::PublishPointPrx
BcastPublisherImpl::open(const ::std::string& name, const Ice::Current& current)
{
	CONN_TRACE(current, BcastPublisher, open);

	::std::string idName = name;
	STRTOLOWER(idName);	

	::Ice::Identity	ident;
	ident.name = idName;
	ident.category = ICE_BcastChannelPublishPoint;

	TianShanIce::Application::Broadcast::BcastPublishPointPrx pointPrx;
	try
	{
        if(_env._evitBcastChannelPublishPoint->hasObject(ident))
		{
			pointPrx = IdentityToObj2(BcastPublishPoint, ident);
		}
		else
		{
			ident.category = ICE_NVODChannelPublishPoint;
			if(_env._evitNOVDChannelPublishPoint->hasObject(ident))
				pointPrx = IdentityToObj2(BcastPublishPoint, ident);
		}
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisher", 300, "open channel[%s] caught %s:%s", 
			name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisher", 301, "open channel[%s] caught %s",
			name.c_str(), ex.ice_name().c_str());
	}
/*
	if (pointPrx == NULL)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisher", 302, "channel[%s] not exist",
			name.c_str());
	}
*/
	return pointPrx;
}

TianShanIce::StrValues
BcastPublisherImpl::list(const Ice::Current& current)
{
	CONN_TRACE(current, BcastPublisher, list);

	TianShanIce::StrValues	BcastchannelNames;
	try
	{
		///List all BroadCastChannel PublishPoint
		{
			// get the iterater of evictor and go thru each of the item
			::Freeze::EvictorIteratorPtr its;
			its = _env._evitBcastChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
			while(its->hasNext())
			{
				::Ice::Identity ident = its->next();
				TianShanIce::Application::Broadcast::BcastPublishPointExPrx pointPrx = 
					IdentityToObj(BcastPublishPointEx, ident);

				::std::string tmpName = pointPrx->getName();

				bool bIsNVODMainChannel = pointPrx->isNOVDMainCh();
				bool bIsNOVDSuppCh =  pointPrx->isNOVDSuppCh();
				if(bIsNVODMainChannel || bIsNOVDSuppCh)//not NVODMainChannel and NVODSupplementChannel
					continue;
				else
				{
                   BcastchannelNames.push_back(tmpName);
				}
			}
		}

		///List all NVODChannel PublishPoint
		{
			// get the iterater of evictor and go thru each of the item
			::Freeze::EvictorIteratorPtr its;
			its = _env._evitNOVDChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
			while(its->hasNext())
			{
				::Ice::Identity ident = its->next();
				TianShanIce::Application::Broadcast::BcastPublishPointPrx pointPrx = 
					IdentityToObj(BcastPublishPoint, ident);
				::std::string tmpName = pointPrx->getName();
				BcastchannelNames.push_back(tmpName);
			}
		}
	}
	catch(::Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublisherImpl, 
			"Caught DatabaseException while listing channels, %s"), ex.message.c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught DatabaseException: " + ex.message;
		throw ex1;
	}
	catch(::Freeze::EvictorDeactivatedException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublisherImpl, 
			"Caught EvictorDeactivatedException while listing channels, %s"), 
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught EvictorDeactivatedException: " + ex.ice_name();
		throw ex1;
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublisherImpl, 
			"Caught Ice Exception while listing channels, %s"), 
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught Ice Exception: " + ex.ice_name();
		throw ex1;
	}

	return BcastchannelNames;
}

void
BcastPublisherImpl::listPublishPointInfo_async(const TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr& listPublishPointInfoCB,
																		const TianShanIce::StrValues& paramNames,
																		const Ice::Current& current)const
{
	CONN_TRACE(current, BcastPublisher, listPublishPointInfo_async);
	try {
		(new ListChannelCmd(listPublishPointInfoCB, _env, paramNames))->execute();
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublisherImpl,"listChannelInfo_async() failed to initial ListChannelCmd"));
		listPublishPointInfoCB->ice_exception(TianShanIce::ServerError("BcastChannel", 500, "failed to generate ListChannelCmd"));
	}
}

TianShanIce::Application::Broadcast::BcastPublishPointPrx
BcastPublisherImpl::createBcastPublishPoint(const ::std::string& name,
										  const TianShanIce::SRM::ResourceMap& resourceRequirement,
										  const TianShanIce::Properties& props,
										  const ::std::string& desc,
										  const Ice::Current& current)
{
	CONN_TRACE(current, BcastPublisher, createBcastPublishPoint);

	for(TianShanIce::SRM::ResourceMap::const_iterator itorRM = resourceRequirement.begin();
		itorRM != resourceRequirement.end(); itorRM++)
	{
		char szBuf[1024];
		memset(szBuf, 0, 1024);
		snprintf(szBuf, sizeof(szBuf) - 1, "[%s]resource requirement: ",name.c_str());
		ZQTianShan::dumpValueMap(itorRM->second.resourceData, szBuf, dumpLine);
	}

	BcastPublishPointImpl::Ptr BcastchPtr = new BcastPublishPointImpl(_env);
	::std::string idName = name;	
	STRTOLOWER(idName);
	BcastchPtr->ident.name = idName;
	BcastchPtr->ident.category = ICE_BcastChannelPublishPoint;
	BcastchPtr->desc = desc;
	BcastchPtr->properties.clear();
	BcastchPtr->properties = props;
    BcastchPtr->resources.clear();
	BcastchPtr->resources = resourceRequirement;
	BcastchPtr->itemSequence.clear();
	BcastchPtr->bInService = false;
	BcastchPtr->bNeedSyncChannel = false;
	BcastchPtr->timestamp = 0;
	BcastchPtr->currentItem ="";
	BcastchPtr->maxBitrate = gBroadcastChCfg.DefaultChannelMaxBitrate;

	BcastchPtr->bIsNVODMainCh = false;
    BcastchPtr->bIsNOVDSupCh = false;
    BcastchPtr->mainChName = idName;
    BcastchPtr->type = BcastChannel_Type;
	BcastchPtr->expiration = ZQ::common::now() + gBroadcastChCfg.expirationTime;

//	BcastchPtr->expiration = ZQ::common::now() + 120000;
    BcastchPtr->persistent = true;
	TianShanIce::Properties::const_iterator itor;

	itor = props.find("Persistent_PublistPoint");
	if(itor != props.end())
	{
       if(atoi(itor->second.c_str()) == 0)
		   BcastchPtr->persistent = false;
	}

	itor = props.find(NVODMAINCHNAME);
	if(itor == props.end())
	{
		BcastchPtr->bIsNVODMainCh = false;
	    BcastchPtr->mainChName = idName;
	}
	else
	{
		BcastchPtr->bIsNVODMainCh = true;
		BcastchPtr->mainChName = itor->second;
		BcastchPtr->type = NVODChannel_Type;
	}

	itor = props.find(NVODSUPPLEMENTAL);
	/// if not have key "nvod_supplemental"
	if(itor == props.end()) 
	{
       BcastchPtr->bIsNOVDSupCh = false;
	}
	else
	{
       BcastchPtr->bIsNOVDSupCh = true;
	   BcastchPtr->bIsNVODMainCh = false;
	   BcastchPtr->type = NVODSupplChannel_Type;
	}

	// check if this BcastChannelPublishPoint exist
	TianShanIce::Application::Broadcast::BcastPublishPointExPrx pointPrx;

	try
	{
		_env._evitBcastChannelPublishPoint->add(BcastchPtr, BcastchPtr->ident);
		pointPrx = IdentityToObj(BcastPublishPointEx, BcastchPtr->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			EXPFMT("BcastPublisherImpl", 400, "create BcastPublishPoint [%s] caught %s: %s"),
			name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, 
			EXPFMT("BcastPublisherImpl", 401, "create BcastPublishPoint [%s] caught %s"),
			name.c_str(), ex.ice_name().c_str());
	}
    _env._watchDog.watchBcastChannel(BcastchPtr->ident, _env._sessionRewTime -10000);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPublisherImpl,"create broadcast publishpoint [%s] successfully"), name.c_str());

	return pointPrx;
}

TianShanIce::Application::Broadcast::NVODChannelPublishPointPrx
BcastPublisherImpl::createNVODPublishPoint(const ::std::string& name,
                                         const TianShanIce::SRM::ResourceMap& resourceRequirement,
                                         ::Ice::Short iteration,
                                         ::Ice::Int interval,
                                         const TianShanIce::Properties& props,
                                         const ::std::string& desc,
                                         const Ice::Current& current)
{
	CONN_TRACE(current, BcastPublisher, createNVODPublishPoint);

	for(TianShanIce::SRM::ResourceMap::const_iterator itorRM = resourceRequirement.begin();
		itorRM != resourceRequirement.end(); itorRM++)
	{
		char szBuf[1024];
		memset(szBuf, 0, 1024);
		snprintf(szBuf, sizeof(szBuf) - 1, "[%s]resource requirement: ",name.c_str());
		ZQTianShan::dumpValueMap(itorRM->second.resourceData, szBuf, dumpLine);
	}

	//get port information
    int ndestPort = 0;	
	TianShanIce::SRM::ResourceMap resourceRequirementCopy = resourceRequirement;
	TianShanIce::SRM::ResourceMap::const_iterator resItor;
	resItor = resourceRequirement.find(TianShanIce::SRM::rtEthernetInterface);
	if(resItor == resourceRequirement.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", 755, CLOGFMT(BcastPublisherImpl,
			"miss resource type='rtEthernetInterface'"));
	}
	TianShanIce::SRM::Resource resource = resItor->second;
	TianShanIce::ValueMap resourceData = resource.resourceData;
	TianShanIce::ValueMap::iterator vmItor = resourceData.find("destPort");
	if(vmItor == resourceData.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", 756, CLOGFMT(BcastPublisherImpl,
			"missed destport infomation"));
	}
	else
	{
		TianShanIce::Variant vardestport = vmItor->second;

		if(vardestport.type == TianShanIce::vtInts && vardestport.ints.size() > 0)
		{
			ndestPort = vardestport.ints[0];
		}
		else
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", 756, CLOGFMT(BcastPublisherImpl,
				"missed destport infomation"));
		}
	}

	if((ndestPort + iteration * gBroadcastChCfg.portIncreaseBase)> MAX_PORT)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", 757, CLOGFMT(BcastPublisherImpl,
			"destport can not be greater than %d (iteration=%d, portIncreasebase=%d)"),
			MAX_PORT, iteration, gBroadcastChCfg.portIncreaseBase);
	}

	NVODChannelPublishPointImpl::Ptr NVODchPtr = new NVODChannelPublishPointImpl(_env);
	::std::string idName = name;	
	STRTOLOWER(idName);
	NVODchPtr->ident.name = idName;
	NVODchPtr->ident.category = ICE_NVODChannelPublishPoint;
	NVODchPtr->desc = desc;
	NVODchPtr->properties.clear();
	NVODchPtr->properties = props;
	NVODchPtr->resources.clear();
	NVODchPtr->resources = resourceRequirement;
	NVODchPtr->itemSequence.clear();

	NVODchPtr->interval = interval;
	NVODchPtr->iteration = iteration;
	NVODchPtr->type = NVODChannel_Type;

	std::string mainChName = idName+"#0";
	TianShanIce::Properties propsMain = props;
	propsMain[NVODMAINCHNAME] = mainChName; 
   
	for(unsigned int i = 0; i <= iteration; i++)
	{
		try
		{	
			char strname[256]="";
			sprintf(strname, "%s#%d\0", idName.c_str(), i);

			if(i != 0)
			{
				ndestPort += gBroadcastChCfg.portIncreaseBase;
				TianShanIce::Variant varDestPort;
				varDestPort.bRange = false;
				varDestPort.type = TianShanIce::vtInts;
				varDestPort.ints.clear();
				varDestPort.ints.push_back(ndestPort);
				resource.resourceData["destPort"] = varDestPort;
				resourceRequirementCopy[TianShanIce::SRM::rtEthernetInterface] = resource;
				propsMain[NVODSUPPLEMENTAL] = NVODSUPPLEMENTAL;
			}

		   TianShanIce::Application::Broadcast::BcastPublishPointPrx bcastprx;
		    TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastExprx;
		   bcastprx = createBcastPublishPoint(strname, resourceRequirementCopy, propsMain, desc, current);
		   bcastExprx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(bcastprx);
		   if(i == 0)
		   {
               NVODchPtr->mainCh = bcastExprx;
		   }
		   else
		   {
			   NVODchPtr->supplementalChannels.push_back(bcastprx);
		   }
		}
		catch(TianShanIce::ServerError& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, 
				EXPFMT("BcastPublisherImpl", 401, "create NOVDChPublishPoint supplemental Channels [%s] caught %s"),
				name.c_str(), ex.ice_name().c_str());
		}
	}

	// check if this BcastChannelPublishPoint exist
	TianShanIce::Application::Broadcast::NVODChannelPublishPointExPrx pointPrx;

	try
	{
		_env._evitNOVDChannelPublishPoint->add(NVODchPtr, NVODchPtr->ident);
		pointPrx = IdentityToObj(NVODChannelPublishPointEx, NVODchPtr->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,
			EXPFMT("BcastPublisherImpl", 400, "create NOVDChPublishPoint [%s] caught %s: %s"),
			name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, 
			EXPFMT("BcastPublisherImpl", 401, "create NOVDChPublishPoint [%s] caught %s"),
			name.c_str(), ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublisherImpl,"create NVOD channel publishpoint [%s] successfully"), name.c_str());

	return pointPrx;
}

void 
BcastPublisherImpl::addFilterItem(const TianShanIce::Application::ChannelItem& newItem, 
						   const Ice::Current&)
{
	TianShanIce::Application::Broadcast::ChannelItemEx newItemCopy;
	{
		WLock sync(*this);

		newItemCopy.setupInfo = newItem;
		checkChannelItem(newItemCopy.setupInfo);
		newItemCopy.setupInfo.lastModified = ZQTianShan::now();
		newItemCopy.key = newItemCopy.setupInfo.contentName;
		newItemCopy.flags = 0;
		newItemCopy.isFilter = 1;

		STRTOLOWER(newItemCopy.key);
		STRTOLOWER(newItemCopy.contentName);

		glog(ZQ::common::Log::L_INFO,  CLOGFMT(BcastPublisherImpl, "add filter item [%s], expiration[%s], inOffset[%lld], outOffset[%lld], spliceIn[%d], spliceOut[%d]"), 
			newItemCopy.setupInfo.contentName.c_str(), newItemCopy.setupInfo.broadcastStart.c_str(), newItemCopy.setupInfo.expiration.c_str(), 
			newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset, 
			newItemCopy.setupInfo.spliceIn, newItemCopy.setupInfo.spliceOut);

		if(newItemCopy.setupInfo.inTimeOffset < 0 || newItemCopy.setupInfo.outTimeOffset <= newItemCopy.setupInfo.inTimeOffset )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", 823, CLOGFMT(BcastPublisherImpl, "outTimeOffset[%d] should be greater than inTimeOffset[%d] or inTimeOffset must be greater than -1"), 
			                                                       newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset);
		}

        newItemCopy.broadcastStart = 0;

		if (!gBroadcastChCfg.InputLocalTime)
		{
			if (!systemTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", err_402, CLOGFMT(BcastPublisherImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!systemTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}
		else 
		{
			if (!localTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", err_403, CLOGFMT(BcastPublisherImpl, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!localTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}

		newItemCopy.broadcastStart = 0;

		// check if the channel item already exists in this channel publish point
		const std::string& newItemName = newItemCopy.setupInfo.contentName;
		TianShanIce::StrValues itemSequence;
		try
		{
			itemSequence = _env._FilterItems->getFilterItemSequence();
		}
		catch (Ice::Exception&ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisherImpl", err_402,
				    CLOGFMT(BcastPublisherImpl, "fail to get filter item sequence"));

		}
		TianShanIce::StrValues::iterator it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), newItemName);
		if (it != itemSequence.end())
		{ // already exists
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", err_400, CLOGFMT(BcastPublisherImpl, "item [%s] already exists "), newItemName.c_str());
		}

		try
		{
			LockT<RecMutex> lk(_env._dictLock);
			_env._pChannelItemDict->put(ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
			_env._FilterItems->addFilterItem(newItemName);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastPublisherImpl, "[%s] appended filter item"), newItemName.c_str());
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisherImpl", err_404, CLOGFMT(BcastPublisherImpl, "add [%s] into safestore caught %s: %s"), 
				newItemCopy.key.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			ex.ice_throw();
		}
	}
}

void 
BcastPublisherImpl::removeFilterItem(const ::std::string& itemName, 
							  const Ice::Current&)
{
  
	std::string rmvItemKey, rmvItemName;
	{
		WLock sync(*this);

		rmvItemKey = itemName;
		rmvItemName = itemName;
		STRTOLOWER(rmvItemKey);
		STRTOLOWER(rmvItemName);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPublisherImpl, "remove item [%s]"), rmvItemName.c_str());

		// remove from channel item dict
		try
		{
			LockT<RecMutex>  lk(_env._dictLock);
			_env._pChannelItemDict->erase(rmvItemKey);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisherImpl", err_700, CLOGFMT(BcastPublisherImpl, "remove item [%s] caught %s: %s"), 
				rmvItemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisherImpl", err_700, CLOGFMT(BcastPublisherImpl, "remove item [%s] caught %s"), 
				rmvItemName.c_str(), ex.ice_name().c_str());
		}

		// remove from item sequence
		try
		{
			_env._FilterItems->removeFilterItem(rmvItemName);
		}
		catch (Ice::Exception&ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BcastPublisherImpl", err_402,
				CLOGFMT(BcastPublisherImpl, "fail to remove filter from item sequence"));
		}
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastPublisherImpl, "item [%s] removed"), rmvItemName.c_str());
	}
}

void
BcastPublisherImpl::listFilterItems_async(const TianShanIce::Application::Broadcast::AMD_BcastPublisher_listFilterItemsPtr& listFilterItemsCB,
											const Ice::Current& current)const
{
	CONN_TRACE(current, BcastPublisher, listPublishPointInfo_async);
	try {
		(new ListFilterItemsCmd(listFilterItemsCB, _env))->execute();
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastPublisherImpl,"listFilterItems_async() failed to initial ListFilterItemsCmd"));
		listFilterItemsCB->ice_exception(TianShanIce::ServerError("BcastChannel", 500, "failed to generate ListFilterItemsCmd"));
	}
}
bool BcastPublisherImpl::checkChannelItem(TianShanIce::Application::ChannelItem& channelItem)
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
//////////////////////////////
/// Class FilterItemsImpl/////
//////////////////////////////
FilterItemsImpl::FilterItemsImpl(BroadCastChannelEnv& bcastChenv):_env(bcastChenv)
{

}
FilterItemsImpl::~FilterItemsImpl()
{

}
void
FilterItemsImpl::addFilterItem(const ::std::string& itemName,
							   const Ice::Current& current)
{
	WLock sync(*this);
	itemSequence.push_back(itemName);
}

void
FilterItemsImpl::removeFilterItem(const ::std::string& itemName,
								  const Ice::Current& current)
{ 
	WLock sync(*this);
	TianShanIce::StrValues::iterator itor;
	itor = std::find(itemSequence.begin(), itemSequence.end(), itemName);
	if(itor != itemSequence.end())
		itemSequence.erase(itor);	
}
TianShanIce::Application::ChannelItem
FilterItemsImpl::findFilterItem(const ::std::string& itemName,
								  const Ice::Current& current)
{ 
	RLock sync(*this);
	std::string itemKey = itemName;

	// search the record in the dictionary	
	LockT<RecMutex> lk(_env._dictLock);
	ChannelItemDict::const_iterator it = _env._pChannelItemDict->find(itemKey);
	if(it == _env._pChannelItemDict->end())
	{ // not found
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "BcastPublisherImpl", err_300, 
			CLOGFMT(BcastPublisherImpl, "channel item [%s] not found"), itemKey.c_str());
	}
	return it->second.setupInfo;
}

TianShanIce::StrValues
FilterItemsImpl::getFilterItemSequence(const Ice::Current& current)
{
	return itemSequence;
}
}

