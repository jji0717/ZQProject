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

// Branch: $Name:NVODChannelPublishPointImpl.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/NVODChannelPublishPointImpl.cpp $
// 
// 5     11/03/14 3:30p Li.huang
// 
// 4     10/21/14 3:32p Li.huang
// 
// 3     10/17/14 3:37p Li.huang
// 
// 2     5/30/14 4:43p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 4     10-03-19 11:46 Li.huang
// remove evictor lock
// 
// 3     09-07-06 9:48 Li.huang
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "NVODChannelPublishPointImpl.h"
#include "BroadcastChCfg.h"

#define NOVDChPPLog(_C, _X) CLOGFMT(_C, "[%s] " _X), ident.name.c_str()

extern ZQ::common::Config::Loader<BroadcastChCfg> gBroadcastChCfg;

namespace ZQBroadCastChannel
{ 

NVODChannelPublishPointImpl::NVODChannelPublishPointImpl(BroadCastChannelEnv& bcastChenv):
_env(bcastChenv)
{
}

NVODChannelPublishPointImpl::~NVODChannelPublishPointImpl(void)
{
}

TianShanIce::Application::Broadcast::NVODSupplementalChannels
NVODChannelPublishPointImpl::getSupplementalChannels(const Ice::Current& current)const
{
	RLock sync(*this);
	return supplementalChannels;
}

::std::string
NVODChannelPublishPointImpl::getType(const Ice::Current& current)const
{
	RLock sync(*this);
	return type;
}

::std::string
NVODChannelPublishPointImpl::getName(const Ice::Current& current)const
{
	RLock sync(*this);
	return ident.name;
}

::std::string
NVODChannelPublishPointImpl::getDesc(const Ice::Current& current)const
{
	RLock sync(*this);
	return desc;
}

::Ice::Int
NVODChannelPublishPointImpl::getMaxBitrate(const Ice::Current& current)const
{
	RLock sync(*this);
   
	return maxBitrate;
}

void
NVODChannelPublishPointImpl::setMaxBitrate(::Ice::Int newMaxBitrate,
									 const Ice::Current& current)
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

	try
	{
		mainCh->setMaxBitrate(newMaxBitrate);

		TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itor;
		for(itor = supplementalChannels.begin(); itor != supplementalChannels.end(); itor++)
		{
			(*itor)->setMaxBitrate(newMaxBitrate);
		}
	}
	catch (...)
	{		
	}
	glog(ZQ::common::Log::L_DEBUG, 
		CLOGFMT(NVODChannelPublishPointImpl, "[%s]setMaxBitrate to %d"), ident.name.c_str(), newMaxBitrate);	
}

void
NVODChannelPublishPointImpl::setProperties(const TianShanIce::Properties& newProps,
									 const Ice::Current& current)
{
	WLock sync(*this);

	glog(ZQ::common::Log::L_DEBUG, 
		CLOGFMT(NVODChannelPublishPointImpl, "[%s]set Properties"), ident.name.c_str());	

	properties.clear();
	properties = newProps;

	try
	{
		mainCh->setProperties(newProps);
		TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itor;
		for(itor = supplementalChannels.begin(); itor != supplementalChannels.end(); itor++)
		{
			(*itor)->setProperties(newProps);
		}
	}
	catch (...)
	{		
	}
}

void
NVODChannelPublishPointImpl::setDesc(const ::std::string& description,
							   const Ice::Current& current)
{
	WLock sync(*this);
    desc = description;	
	try
	{
		mainCh->setDesc(description);
		TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itor;
		for(itor = supplementalChannels.begin(); itor != supplementalChannels.end(); itor++)
		{
			(*itor)->setDesc(description);
		}
	}
	catch (...)
	{		
	}  
}

TianShanIce::Properties
NVODChannelPublishPointImpl::getProperties(const Ice::Current& current)const
{
	RLock sync(*this);
	return properties;
}

void
NVODChannelPublishPointImpl::destroy(const Ice::Current& current)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NVODChannelPublishPointImpl, "remove channel [%s]"), ident.name.c_str());

	WLock sync(*this);

	try
	{
		_env._evitNOVDChannelPublishPoint->remove(ident);
	}
	catch(const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "NVODChannelPublishPointImpl", err_800, CLOGFMT(NVODChannelPublishPointImpl, "remove channel [%s] from evictor caught %s: %s"), 
			ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(NVODChannelPublishPointImpl, "channel [%s] removed"), ident.name.c_str());

    _env.removeNVODSupplMgr(ident);

	try
	{
		mainCh->destroy();
	}
	catch (const TianShanIce::BaseException& ex)
	{
		_IceReThrow(TianShanIce::ServerError, ex);
	}

	TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itor;
	for(itor = supplementalChannels.begin(); itor != supplementalChannels.end(); itor++)
	{
		try
		{
			(*itor)->destroy();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_IceReThrow(TianShanIce::ServerError, ex);
		}
		
	}
	supplementalChannels.clear();
}

void
NVODChannelPublishPointImpl::restrictReplica(const TianShanIce::StrValues& contentStoreNetIds,
									   const Ice::Current& current)
{
	WLock sync(*this);

	replicas = contentStoreNetIds;
	for (uint i = 0, count = contentStoreNetIds.size(); i < count; i ++)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(NVODChannelPublishPointImpl, "restrict channel [%s] on netId [%s]"), ident.name.c_str(), contentStoreNetIds[i].c_str());
	}

	try
	{
		return mainCh->restrictReplica(contentStoreNetIds);
	}
	catch (...)
	{		
	}
}

TianShanIce::StrValues
NVODChannelPublishPointImpl::listReplica(const Ice::Current& current)const
{
	RLock sync(*this);
	return replicas;
}

::Ice::Long
NVODChannelPublishPointImpl::requireResource(TianShanIce::SRM::ResourceType type,
									   const TianShanIce::SRM::Resource& res,
									   const Ice::Current& current)
{
	WLock sync(*this);
	if(isInService(current))
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "NVODChannelPublishPointImpl", err_701, CLOGFMT(NVODChannelPublishPointImpl,
			"[%s]channel is already playing, please stop it first"),ident.name.c_str());  
	}

	char szBuf[1024];
	memset(szBuf, 0, 1024);
	snprintf(szBuf, sizeof(szBuf) - 1, "[%s]requireResource ",ident.name.c_str());
	ZQTianShan::dumpValueMap(res.resourceData, szBuf, dumpLine);

	try
	{
		//get port information
		int ndestPort = 0;	
		bool bFindPort = false;
		if(type == TianShanIce::SRM::rtEthernetInterface)
		{	
			TianShanIce::ValueMap resourceData = res.resourceData;
			TianShanIce::ValueMap::iterator vmItor = resourceData.find("destPort");
			if(vmItor == resourceData.end())
			{
				bFindPort = false;
			}
			else
			{
				TianShanIce::Variant vardestport = vmItor->second;

				if(vardestport.type == TianShanIce::vtInts && vardestport.ints.size() > 0)
				{
					ndestPort = vardestport.ints[0];
					bFindPort = true;
				}
				else
				{
					bFindPort = false;
				}
			}
			if((ndestPort + iteration * gBroadcastChCfg.portIncreaseBase)> MAX_PORT)
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "NVODChannelPublishPointImpl", 757, CLOGFMT(NVODChannelPublishPointImpl,
					"[%s]destport can not be greater than %d (iteration=%d, portIncreasebase=%d)"),
					ident.name.c_str(), MAX_PORT, iteration, gBroadcastChCfg.portIncreaseBase); 
				bFindPort = false;
			}
		}

//		resources.insert(TianShanIce::SRM::ResourceMap::value_type(type, res));
		resources[type] = res;
		mainCh->requireResource(type, res);
		TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itor;
		for(itor = supplementalChannels.begin(); itor != supplementalChannels.end(); itor++)
		{
			TianShanIce::SRM::Resource resource = res;
			if(bFindPort)
			{
				ndestPort += gBroadcastChCfg.portIncreaseBase;
				TianShanIce::Variant varDestPort;
				varDestPort.bRange = false;
				varDestPort.type = TianShanIce::vtInts;
				varDestPort.ints.clear();
				varDestPort.ints.push_back(ndestPort);
				resource.resourceData["destPort"] = varDestPort;
			}			
			(*itor)->requireResource(type, resource);
		}		
	}
	catch (...)
	{		
	}

	return res.resourceData.size();
}

TianShanIce::SRM::ResourceMap
NVODChannelPublishPointImpl::getResourceRequirement(const Ice::Current& current)const
{
	RLock sync(*this);
    return resources;
}

void
NVODChannelPublishPointImpl::withdrawResourceRequirement(TianShanIce::SRM::ResourceType type,
												   const Ice::Current& current)
{
	WLock sync(*this);

	TianShanIce::SRM::ResourceMap::iterator itor;
	itor = resources.find(type);
	if(itor != resources.end())
		resources.erase(itor);

	try
	{
		mainCh->withdrawResourceRequirement(type);

		TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itor;
		for(itor = supplementalChannels.begin(); itor != supplementalChannels.end(); itor++)
		{
			(*itor)->withdrawResourceRequirement(type);
		}
	}
	catch (...)
	{		
	}
}

void
NVODChannelPublishPointImpl::setup(const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO , NOVDChPPLog(NVODChannelPublishPointImpl, "Enter setup NVOD Channel PublishPoint"));

	///setup nvod broadcastPublishpoint
	try
	{
		mainCh->setup();
	}
	catch (TianShanIce::BaseException&ex)
	{
		_IceReThrow(TianShanIce::ServerError, ex);
	}

	TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itorSupCh;
	for(itorSupCh = supplementalChannels.begin() ; itorSupCh != supplementalChannels.end(); itorSupCh++)
	{
		try
		{	
			(*itorSupCh)->setup();
		}
		catch (TianShanIce::BaseException&ex)
		{
			_IceReThrow(TianShanIce::ServerError, ex);
		}
	}

	glog(ZQ::common::Log::L_INFO , NOVDChPPLog(NVODChannelPublishPointImpl, "setup NVODChannelPublishPoint successfully"));
}

TianShanIce::SRM::SessionPrx
NVODChannelPublishPointImpl::getSession(const Ice::Current& current)
{
	RLock sync(*this);
	try
	{
		return mainCh->getSession();
	}
	catch (...)
	{

	}
	return NULL;
}

void
NVODChannelPublishPointImpl::start(const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl,
		"Enter start()"));

	try
	{
		mainCh->start();
	}
	catch (TianShanIce::BaseException&ex)
	{
		_IceReThrow(TianShanIce::ServerError, ex);
	}

    _env.addNVODSupplMgr(ident, interval, true);

	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl,
		"play stream successfully"));

	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl,
		"Leave start()"));
}

void
NVODChannelPublishPointImpl::stop(const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl,
		"Enter stop()"));
	try
	{
		mainCh->stop();
	}
	catch (TianShanIce::BaseException&ex)
	{
		_IceReThrow(TianShanIce::ServerError, ex);
	}
	TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator itorSupCh;
	for(itorSupCh = supplementalChannels.begin() ; itorSupCh != supplementalChannels.end(); itorSupCh++)
	{
		try
		{	
			(*itorSupCh)->stop();
		}
		catch (TianShanIce::BaseException&ex)
		{
			_IceReThrow(TianShanIce::ServerError, ex);
		}
	}

    _env.removeNVODSupplMgr(ident);

	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl, "stop stream successfully"));

	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl,
		"Leave stop()"));
}

::Ice::Long
NVODChannelPublishPointImpl::getUpTime(const Ice::Current& current)
{
	RLock sync(*this);
	try
	{
		return mainCh->getUpTime();
	}
	catch (...)
	{

	}
	return 0;
}
void NVODChannelPublishPointImpl::renew(Ice::Long TTL, const Ice::Current &)
{
	RLock sync(*this);
	try
	{
		return mainCh->renew(TTL);
	}
	catch (...)
	{

	}
}
::Ice::Long NVODChannelPublishPointImpl::getExpiration(const Ice::Current &)
{
	RLock sync(*this);
	try
	{
		return mainCh->getExpiration();
	}
	catch (...)
	{

	}
	return 0;
}

TianShanIce::StrValues
NVODChannelPublishPointImpl::getItemSequence(const Ice::Current& current)const
{
	RLock sync(*this);
	try
	{
		return mainCh->getItemSequence();
	}
	catch (...)
	{
	}
    return TianShanIce::StrValues();
}

TianShanIce::Application::ChannelItem
NVODChannelPublishPointImpl::findItem(const ::std::string& itemName,
								const Ice::Current& current)const
{
	RLock sync(*this);
	try
	{
		return mainCh->findItem(itemName);
	}
	catch (...)
	{
	}
	return TianShanIce::Application::ChannelItem();
}

void
NVODChannelPublishPointImpl::appendItem(const TianShanIce::Application::ChannelItem& newItem,
								  const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl, "append [%s] on channel [%s]"), 
		newItem.contentName.c_str(), ident.name.c_str());
	try
	{	
		return	mainCh->appendItem(newItem);
	}
	catch (...)
	{	
	}
}

void
NVODChannelPublishPointImpl::insertItem(const ::std::string& atItemName,
								  const TianShanIce::Application::ChannelItem& newItem,
								  const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl, "insert [%s] at [%s] on channel [%s]"),
		newItem.contentName.c_str(), atItemName.c_str(), ident.name.c_str());
	try
	{	
		return	mainCh->insertItem(atItemName, newItem);
	}
	catch (...)
	{	 
        
	}
}

void
NVODChannelPublishPointImpl::appendItemAs(const TianShanIce::Application::ChannelItem& newItem,
									const ::std::string& newName,
									const Ice::Current& current)
{
	ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "NVODChannelPublishPointImpl", err_300, 
		"not implement");
}
void
NVODChannelPublishPointImpl::insertItemAs(const ::std::string& atItemName,
									const TianShanIce::Application::ChannelItem& newItem,
									const ::std::string& newName,
									const Ice::Current& current)
{
	ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "NVODChannelPublishPointImpl", err_300, 
		"not implement");

}

void
NVODChannelPublishPointImpl::replaceItem(const ::std::string& oldName,
								   const TianShanIce::Application::ChannelItem& newItem,
								   const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl, "replace [%s] with [%s] on channel [%s]"),
		 oldName.c_str(), newItem.contentName.c_str(),ident.name.c_str());
	try
	{	
		return	mainCh->replaceItem(oldName, newItem);
	}
	catch (...)
	{	

	}
}

void
NVODChannelPublishPointImpl::removeItem(const ::std::string& itemName,
								  const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, NOVDChPPLog(NVODChannelPublishPointImpl, "remove [%s on channel [%s]"),
         itemName.c_str(), ident.name.c_str());
	try
	{	
		return	mainCh->removeItem(itemName);
	}
	catch (...)
	{
	}
}

::Ice::Int NVODChannelPublishPointImpl::getInterval(const Ice::Current&)
{
	RLock sync(*this);
	return interval;
}
::Ice::Short NVODChannelPublishPointImpl::getIteration(const Ice::Current&)
{
	RLock sync(*this);
	return iteration;
}


bool  NVODChannelPublishPointImpl::isInService(const ::Ice::Current&)
{
	try
	{	
		return	mainCh->isInService();
	}
	catch (...)
	{
	}
	return false;
}

bool 
NVODChannelPublishPointImpl::activate(const ::Ice::Current& c)
{
//	Lock sync(*this);
	glog(ZQ::common::Log::L_DEBUG, NOVDChPPLog(NVODChannelPublishPointImpl, "Enter activate()"), ident.name.c_str());

	bool bisInService = isInService(c);

	if(bisInService)
	{
		_env.addNVODSupplMgr(ident, interval, true);
	}
	glog(ZQ::common::Log::L_DEBUG, NOVDChPPLog(NVODChannelPublishPointImpl, "Leave activate()"), ident.name.c_str());
	return true;
}
}///end namespace ZQBroadCastChannel
