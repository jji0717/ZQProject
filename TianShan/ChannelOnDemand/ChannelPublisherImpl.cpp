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
//
// Name  : ChannelPublisherImpl.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-23
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChannelPublisherImpl.cpp $
// 
// 2     12/12/13 1:38p Hui.shao
// %lld
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 26    10-03-17 17:17 Haoyuan.lu
// remove lock of evictor and add DB_CONFIG
// 
// 25    09-10-08 16:11 Haoyuan.lu
// 
// 24    09-07-27 16:40 Haoyuan.lu
// 
// 23    09-02-06 17:27 Haoyuan.lu
// 
// 22    08-04-22 18:07 Haoyuan.lu
// 
// 21    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 20    08-03-19 14:47 Haoyuan.lu
// 
// 19    08-03-05 12:15 Guan.han
// 
// 20    08-03-04 18:53 Guan.han
// 
// 18    08-01-15 15:52 Guan.han
// added listChannelInfo() shell
// 
// 17    08-01-09 13:32 Guan.han
// throw exception when publish a channel which already exists
// 
// 16    08-01-02 11:42 Guan.han
// 
// 15    07-12-24 13:51 Guan.han
// 
// 14    07-12-24 11:53 Guan.han
// 
// 13    07-12-12 12:07 Guan.han
// 
// 12    07-07-13 12:43 Jie.zhang
// sync
// 
// 11    07-07-12 20:02 Jie.zhang
// * RW lock seem must not used for dictionary, normal lock is ok,  *the
// evictor must inherited from a lock, or will cause program exit
// unexcpected.
// 
// 10    07-07-11 16:05 Jie.zhang
// sync with main tree
// 
// 11    07-07-11 15:53 Jie.zhang
// add lock to every evictor
// 
// 10    07-07-06 14:47 Jie.zhang
// add a confige: enablechannelmaxdurationcheck and add some logic to
// avoid null playlist handle
// 
// 9     06-11-17 19:38 Jie.zhang
// 
// 8     06-10-24 18:43 Jie.zhang
// 
// 7     06-10-23 10:04 Jie.zhang
// 
// 6     06-09-26 11:18 Jie.zhang
// 
// 5     06-09-20 14:32 Jie.zhang
// 
// 4     06-09-04 21:07 Bernie.zhao
// added notification callbacks
// 
// 3     06-08-29 13:58 Bernie.zhao
// added expiration item monitor
// 
// 2     06-08-28 12:01 Bernie.zhao
// 1st draft done
// ===========================================================================

#include "stdafx.h"
#include <ice/Ice.h>
#include "ChannelPublisherImpl.h"
#include "ChannelPublishPointImpl.h"
#include "ChODSvcEnv.h"
#include "CODConfig.h"
#include "PurchaseRequest.h"


#define LOG_MODULE_NAME			"CodPub"

#ifdef USE_OLD_NS
#define IdentityToObj(_CLASS, _ID) ::ChannelOnDemand::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#else
#define IdentityToObj(_CLASS, _ID) ::TianShanIce::Application::ChannelOnDemand::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#endif //USE_OLD_NS

namespace ZQChannelOnDemand {


#ifdef ENABLE_CHANNEL_EXPIRE_MANAGER


//////////////////////////////////////////////////////////////////////////
// class ChannelItemMoniter
//////////////////////////////////////////////////////////////////////////
ChannelItemMoniter::ChannelItemMoniter(ChannelPublisherImpl& publisher)
:_publisher(publisher)
{
	_interval	= DEFAULT_MONITOR_INTERVAL;
	_bQuit		= false;
	_hSignal	= ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

ChannelItemMoniter::~ChannelItemMoniter()
{
	// stop thread
	if(isRunning())
	{
		_bQuit = true;
		signalStop();
		waitHandle(100);
	}

	// close signal handle
	if(_hSignal!=INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_hSignal);
		_hSignal = INVALID_HANDLE_VALUE;
	}
}

void ChannelItemMoniter::signalStop()
{
	_bQuit = true;
	if(_hSignal!=INVALID_HANDLE_VALUE)
		::SetEvent(_hSignal);
}

void ChannelItemMoniter::setInterval(DWORD interval)
{
	DWORD tmpInterval = interval<DEFAULT_MONITOR_INTERVAL? DEFAULT_MONITOR_INTERVAL : interval;
	if(tmpInterval!=_interval)
	{
		_interval = tmpInterval;
		::SetEvent(_hSignal);
	}
}
	
bool ChannelItemMoniter::init(void)
{
	return true;
}

int ChannelItemMoniter::run(void)
{
	while(!_bQuit)
	{
		DWORD retcode = ::WaitForSingleObject(_hSignal, _interval*1000);
		if(retcode == WAIT_OBJECT_0)
		{
			continue;
		}
		else if(retcode == WAIT_TIMEOUT)
		{
			int expiredCount = _publisher._scanForExpire();
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "::WaitForSingleObject() error : %d"), ::GetLastError());
		}
	}

	return 0;
}

void ChannelItemMoniter::final(void)
{
	return;
}

#endif
//////////////////////////////////////////////////////////////////////////
// class ChannelPublisherImpl
//////////////////////////////////////////////////////////////////////////


#ifdef ENABLE_CHANNEL_EXPIRE_MANAGER

void ChannelPublisherImpl::setMonitorInterval(DWORD interval)
{
	if(interval<DEFAULT_MONITOR_INTERVAL)
		_moniterInterval = DEFAULT_MONITOR_INTERVAL;
	
	_moniter.setInterval(_moniterInterval);
}

void ChannelPublisherImpl::setMonitorTraceFlag(bool traceflag)
{
	_monitorTraceFlag = traceflag;
}

int ChannelPublisherImpl::_scanForExpire()
{
	int expiredNum = 0;

	try
	{
		// get current system time
		::Ice::Long currTime = ::ZQChannelOnDemand::now();

		// since we want to dispatch the real expiration work to ChannelPublishPoint,
		// we do a 2-parse round to delete all expired items
		// 1. parse all program items and select the latest expired item in each channel
		// 2. for each item selected, call corresponding channel to expire them
		
		// lock dict

		::ChannelOnDemand::ChannelItemDict::const_iterator dictIt;
		::std::map<::std::string, ::ChannelOnDemand::ChannelItemEx> dyingItems;
		::std::map<::std::string, ::ChannelOnDemand::ChannelItemEx>::iterator dyingIt;
		dyingItems.clear();

		// parse 1
		{
			LockT<RecMutex> lk(_env._dictLock);
			for(dictIt=_env._pChannelItemDict->begin(); dictIt!=_env._pChannelItemDict->end(); dictIt++)
			{
				::ChannelOnDemand::ChannelItemEx tmpItem = dictIt->second;
				if(tmpItem.expiration != 0 && tmpItem.expiration <= currTime)
				{				
					::std::string chName = _getItemPrefix(tmpItem.key);
					if(chName.empty())
						continue;

					// add the item into the dying list if:
					// 1. if this channel is not in the dying list
					// 2. if this channel is in the list, but the item in the list expires early than current item
					dyingIt = dyingItems.find(chName);
					if(dyingIt==dyingItems.end() || dyingIt->second.broadcastStart<tmpItem.broadcastStart)
						dyingItems[chName] = tmpItem;

					expiredNum++;
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "now it is %lld, item [%s] should have expired at [%lld]"), currTime, tmpItem.key.c_str(), tmpItem.expiration);
				}
			}
		}
		
		// parse 2
		::Freeze::EvictorIteratorPtr its = _env._evitChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
		while(its->hasNext())
		{
			::Ice::Identity ident = its->next();
			::ChannelOnDemand::ChannelPublishPointPrx pointPrx = IdentityToObj(ChannelPublishPoint, ident);
			::std::string tmpName = pointPrx->getName();
			STRTOLOWER(tmpName);
			dyingIt = dyingItems.find(tmpName);
			if(dyingIt!=dyingItems.end())
			{
				// this channel is in the dying list, which means at least 1 item in this channel already expires
				pointPrx->removeItem(dyingIt->second.setupInfo.contentName);
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "ChannelPublisherImpl::_scanForExpire() Channel(%s) Item(%s) has been removed"), pointPrx->getName().c_str(), dyingIt->second.setupInfo.contentName.c_str());
			}
		}
		
	}
	catch(Freeze::DatabaseException e)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "got db exception when cleaning expired items, detail: %s"), e.message);
	}
	catch(Freeze::EvictorDeactivatedException e)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "got deactivated evictor exception when cleaning expired items"));
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "got unknown exception when cleaning expired items"));
	}

	if(expiredNum && _monitorTraceFlag)
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "monitor thread had cleaned %d expired items"), expiredNum);

	return expiredNum;
}

ChannelPublisherImpl::ChannelPublisherImpl(ChODSvcEnv& env)
	: _env(env), _moniter(*this)
{
	_moniter.start();
}

ChannelPublisherImpl::~ChannelPublisherImpl()
{
	_moniter.signalStop();
}

#else

ChannelPublisherImpl::ChannelPublisherImpl(ChODSvcEnv& env)
	: _env(env)
{
}

ChannelPublisherImpl::~ChannelPublisherImpl()
{
}

#endif

#ifdef USE_OLD_NS
::ChannelOnDemand::ChannelPublishPointPrx ChannelPublisherImpl::publishEx(const ::std::string& channelPubName, 
																		  const ::std::string& onDemandName, ::Ice::Int maxBitrate, const ::TianShanIce::Properties& props, const ::std::string& desc, const ::Ice::Current& c)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "publishEx channel [ChannelPubName=%s, onDemandName=%s, maxBitrate=%d, desc=%s]"), 
		channelPubName.c_str(), onDemandName.c_str(), maxBitrate, desc.c_str());

	CONN_TRACE(c, ChannelPublisher, publishEx);

	ChannelPublishPointImpl::Ptr pLink = new ChannelPublishPointImpl(_env);
	::std::string idName = channelPubName;	STRTOLOWER(idName);
	pLink->ident.name = idName;
	pLink->ident.category = ICE_ChannelPublishPoint;
	pLink->onDemandName = onDemandName; // use the input parameter onDemandName.
	pLink->desc = desc;
	pLink->properties.clear();
	pLink->itemSequence.clear();
	if (maxBitrate)
	{
		pLink->maxBitrate = maxBitrate;
	}
	else
	{
		pLink->maxBitrate = _config.DefaultChannelMaxBitrate;
	}

	// check if this ChannelPublishPoint exist
	::ChannelOnDemand::ChannelPublishPointPrx pointPrx;

	try
	{
		LockT<RecMutex> lk(_env._evitCPPLock);
		_env._evitChannelPublishPoint->add(pLink, pLink->ident);
		pointPrx = IdentityToObj(ChannelPublishPoint, pLink->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 400, "publishEx channel [%s] caught %s: %s"), channelPubName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 401, "publishEx channel [%s] caught %s"), channelPubName.c_str(), ex.ice_name().c_str());
	}

	return pointPrx;
}

::ChannelOnDemand::ChannelPublishPointPrx ChannelPublisherImpl::publish(const ::std::string& onDemandName, ::Ice::Int maxBitrate, const ::std::string& desc, const ::Ice::Current& c)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "publish channel [ChannelPubName=%s, maxBitrate=%d, desc=%s]"), 
		onDemandName.c_str(), maxBitrate, desc.c_str());

	CONN_TRACE(c, ChannelPublisher, publish);

	ChannelPublishPointImpl::Ptr pLink = new ChannelPublishPointImpl(_env);
	::std::string idName = onDemandName;	STRTOLOWER(idName);
	pLink->ident.name = idName;
	pLink->ident.category = ICE_ChannelPublishPoint;
	pLink->onDemandName = onDemandName; // by default using name as onDemandName.
	pLink->desc = desc;
	pLink->properties.clear();
	pLink->itemSequence.clear();
	if (maxBitrate)
	{
		pLink->maxBitrate = maxBitrate;
	}
	else
	{
		pLink->maxBitrate = _config.DefaultChannelMaxBitrate;
	}

	// check if this ChannelPublishPoint exist
	::ChannelOnDemand::ChannelPublishPointPrx pointPrx;

	try
	{
		LockT<RecMutex> lk(_env._evitCPPLock);
		_env._evitChannelPublishPoint->add(pLink, pLink->ident);
		pointPrx = IdentityToObj(ChannelPublishPoint, pLink->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 400, "publish channel [%s] caught %s: %s"), onDemandName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 401, "publish channel [%s] caught %s"), onDemandName.c_str(), ex.ice_name().c_str());
	}

	return pointPrx;
}

::ChannelOnDemand::ChannelPublishPointPrx ChannelPublisherImpl::open(const ::std::string& chlPubName, const ::Ice::Current& c)
{
	CONN_TRACE(c, ChannelPublisher, open);

	::std::string idName = chlPubName;
	STRTOLOWER(idName);	

	::Ice::Identity	ident;
	ident.name = idName;
	ident.category = ICE_ChannelPublishPoint;

	::ChannelOnDemand::ChannelPublishPointPrx chlPubPrx;
	try
	{
		chlPubPrx = IdentityToObj(ChannelPublishPoint, ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "CodPub", 300, "open channel[%s] caught %s:%s", chlPubName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "CodPub", 301, "open channel[%s] caught %s", chlPubName.c_str(), ex.ice_name().c_str());
	}

	if (chlPubPrx == NULL)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "CodPub", 302, "channel[%s] not exist", chlPubName.c_str());
	}

	return chlPubPrx;
}

::TianShanIce::StrValues ChannelPublisherImpl::list(const ::Ice::Current& c)
{
	CONN_TRACE(c, ChannelPublisher, list);

	::TianShanIce::StrValues	channelNames;
	try
	{
		// get the iterater of evictor and go thru each of the item
		LockT<RecMutex> lk(_env._evitCPPLock);

		::Freeze::EvictorIteratorPtr its;
		its = _env._evitChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
		while(its->hasNext())
		{
			::Ice::Identity ident = its->next();
			::ChannelOnDemand::ChannelPublishPointPrx pointPrx = IdentityToObj(ChannelPublishPoint, ident);
			::std::string tmpName = pointPrx->getName();
			channelNames.push_back(tmpName);
		}
	}
	catch(::Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught DatabaseException while listing channels, %s"), ex.message.c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught DatabaseException: " + ex.message;
		throw ex1;
	}
	catch(::Freeze::EvictorDeactivatedException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught EvictorDeactivatedException while listing channels, %s"), 
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught EvictorDeactivatedException: " + ex.ice_name();
		throw ex1;
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught Ice Exception while listing channels, %s"), 
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught Ice Exception: " + ex.ice_name();
		throw ex1;
	}

	return channelNames;
}

void ChannelPublisherImpl::listChannelInfo_async(const ::ChannelOnDemand::AMD_ChannelPublisher_listChannelInfoPtr& amdCB, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNames, const ::Ice::Current& c) const
{
	try {
		(new ListChannelCmd(amdCB, _env, onDemandName, paramNames))->execute();
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("ChannelPublisher","listChannelInfo_async() failed to initial ListChannelCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("COD", 500, "failed to generate ListChannelCmd"));
	}
}

#else
::TianShanIce::Application::OnDemandPublishPointPrx ChannelPublisherImpl::publishEx(const ::std::string& channelPubName, 
				const ::std::string& onDemandName, ::Ice::Int maxBitrate, const ::TianShanIce::Properties& props, const ::std::string& desc, const ::Ice::Current& c)
{
	std::string caller = (c.con)->toString();
	caller = caller.substr(caller.find_last_of("=")+1, caller.size());
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "publishEx channel [ChannelPubName=%s, onDemandName=%s, maxBitrate=%d, desc=%s] from %s"), 
		channelPubName.c_str(), onDemandName.c_str(), maxBitrate, desc.c_str(), caller.c_str());

	std::string signature = ZQChannelOnDemand::invokeSignature(c);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "%s"), signature.c_str());
	
	CONN_TRACE(c, ChannelPublisher, publishEx);

	ChannelPublishPointImpl::Ptr pLink = new ChannelPublishPointImpl(_env);
	::std::string idName = channelPubName;	STRTOLOWER(idName);
	pLink->ident.name = idName;
	pLink->ident.category = ICE_ChannelPublishPoint;
	pLink->onDemandName = onDemandName; // use the input parameter onDemandName.
	pLink->desc = desc;
	pLink->properties.clear();
	pLink->itemSequence.clear();
	if (maxBitrate)
	{
		pLink->maxBitrate = maxBitrate;
	}
	else
	{
		pLink->maxBitrate = _config.DefaultChannelMaxBitrate;
	}

	// check if this ChannelPublishPoint exist
	::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx pointPrx;

	try
	{
//		LockT<RecMutex> lk(_env._evitCPPLock);
		_env._evitChannelPublishPoint->add(pLink, pLink->ident);
		pointPrx = IdentityToObj(ChannelPublishPoint, pLink->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 400, "publishEx channel [%s] caught %s: %s"), channelPubName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 401, "publishEx channel [%s] caught %s"), channelPubName.c_str(), ex.ice_name().c_str());
	}

	return pointPrx;
}

::TianShanIce::Application::PublishPointPrx ChannelPublisherImpl::publish(const ::std::string& onDemandName, ::Ice::Int maxBitrate, const ::std::string& desc, const ::Ice::Current& c)
{
	std::string caller = (c.con)->toString();
	caller = caller.substr(caller.find_last_of("=")+1, caller.size());
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "publish channel [ChannelPubName=%s, maxBitrate=%d, desc=%s] from %s"), 
		onDemandName.c_str(), maxBitrate, desc.c_str(), caller.c_str());

	std::string signature = ZQChannelOnDemand::invokeSignature(c);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "%s"), signature.c_str());

	CONN_TRACE(c, ChannelPublisher, publish);
	
	ChannelPublishPointImpl::Ptr pLink = new ChannelPublishPointImpl(_env);
	::std::string idName = onDemandName;	STRTOLOWER(idName);
	pLink->ident.name = idName;
	pLink->ident.category = ICE_ChannelPublishPoint;
	pLink->onDemandName = onDemandName; // by default using name as onDemandName.
	pLink->desc = desc;
	pLink->properties.clear();
	pLink->itemSequence.clear();
	if (maxBitrate)
	{
		pLink->maxBitrate = maxBitrate;
	}
	else
	{
		pLink->maxBitrate = _config.DefaultChannelMaxBitrate;
	}

	// check if this ChannelPublishPoint exist
	::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx pointPrx;

	try
	{
//		LockT<RecMutex> lk(_env._evitCPPLock);
		_env._evitChannelPublishPoint->add(pLink, pLink->ident);
		pointPrx = IdentityToObj(ChannelPublishPoint, pLink->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 400, "publish channel [%s] caught %s: %s"), onDemandName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(LOG_MODULE_NAME, 401, "publish channel [%s] caught %s"), onDemandName.c_str(), ex.ice_name().c_str());
	}

	return pointPrx;
}

::TianShanIce::Application::PublishPointPrx ChannelPublisherImpl::open(const ::std::string& chlPubName, const ::Ice::Current& c)
{
	CONN_TRACE(c, ChannelPublisher, open);

	::std::string idName = chlPubName;
	STRTOLOWER(idName);	

	::Ice::Identity	ident;
	ident.name = idName;
	ident.category = ICE_ChannelPublishPoint;

	::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx chlPubPrx;
	try
	{
		chlPubPrx = IdentityToObj(ChannelPublishPoint, ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "CodPub", 300, "open channel[%s] caught %s:%s", chlPubName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "CodPub", 301, "open channel[%s] caught %s", chlPubName.c_str(), ex.ice_name().c_str());
	}

	if (chlPubPrx == NULL)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, "CodPub", 302, "channel[%s] not exist", chlPubName.c_str());
	}

	return chlPubPrx;
}

::TianShanIce::StrValues ChannelPublisherImpl::list(const ::Ice::Current& c)
{
	CONN_TRACE(c, ChannelPublisher, list);

	::TianShanIce::StrValues	channelNames;
	try
	{
		// get the iterater of evictor and go thru each of the item
//		LockT<RecMutex> lk(_env._evitCPPLock);

		::Freeze::EvictorIteratorPtr its;
		its = _env._evitChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
		while(its->hasNext())
		{
			::Ice::Identity ident = its->next();
			::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx pointPrx = IdentityToObj(ChannelPublishPoint, ident);
			::std::string tmpName = pointPrx->getName();
			channelNames.push_back(tmpName);
		}
	}
	catch(::Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught DatabaseException while listing channels, %s"), ex.message.c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught DatabaseException: " + ex.message;
		throw ex1;
	}
	catch(::Freeze::EvictorDeactivatedException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught EvictorDeactivatedException while listing channels, %s"), 
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught EvictorDeactivatedException: " + ex.ice_name();
		throw ex1;
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught Ice Exception while listing channels, %s"), 
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught Ice Exception: " + ex.ice_name();
		throw ex1;
	}
	
	return channelNames;
}

void ChannelPublisherImpl::listOnDemandPointInfo_async(const ::TianShanIce::Application::AMD_OnDemandPublisher_listOnDemandPointInfoPtr& amdCB, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNames, const ::Ice::Current& c) const
{
	try {
		(new ListChannelCmd(amdCB, _env, onDemandName, paramNames))->execute();
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("ChannelPublisher","listChannelInfo_async() failed to initial ListChannelCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("COD", 500, "failed to generate ListChannelCmd"));
	}
}

void ChannelPublisherImpl::listPublishPointInfo_async(const ::TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr& amdCB, const ::TianShanIce::StrValues& paramNames, const ::Ice::Current& c) const
{
}
#endif //USE_OLD_NS

std::string	ChannelPublisherImpl::_getItemPrefix(std::string itemkey)
{
	if(itemkey.empty())
		return "";

	size_t separator = itemkey.find_first_of(CHANNELITEM_KEY_SEPARATOR);
	if(separator==itemkey.npos)
		return "";

	return itemkey.substr(0, separator);
}

std::string	ChannelPublisherImpl::getChannelByStream(const std::string& streamID, const ::Ice::Current& c)
{
	if(streamID.empty())
		return "";

	glog(ZQ::common::Log::L_DEBUG, "getChannelByStream [%s]", streamID.c_str());

	std::vector<Ice::Identity> idents;
	try
	{
		idents = _env._idxPlaylistId2Purchase->find(streamID);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "stream [%s] to purchase caught %s: %s"), 
			streamID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return "";
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "stream [%s] to purchase caught %s"), 
			streamID.c_str(), ex.ice_name().c_str());
		return "";
	}

	if (idents.size() == 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "no purchase associated with stream [%s]"), 
			streamID.c_str());
		return "";
	}

	NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx) purchasePrx;

	Ice::ObjectPrx prx = _env._adapter->createProxy(idents[0]);
	if (!prx)
	{
		return "";
	}

	purchasePrx = NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx)::uncheckedCast(prx);
	
	return purchasePrx->getChannelName();
}

}	// ZQChannelOnDemand