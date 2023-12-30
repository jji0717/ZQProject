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
// Name  : ChannelPublishPointImpl.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-21
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChannelPublishPointImpl.cpp $
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
// 36    10-03-17 17:17 Haoyuan.lu
// remove lock of evictor and add DB_CONFIG
// 
// 35    09-10-08 16:11 Haoyuan.lu
// 
// 34    09-07-27 16:34 Haoyuan.lu
// 
// 33    09-02-06 17:32 Haoyuan.lu
// 
// 32    08-12-26 18:22 Haoyuan.lu
// 
// 31    08-12-26 14:26 Haoyuan.lu
// 
// 30    08-11-18 10:46 Haoyuan.lu
// 
// 29    08-10-23 10:45 Haoyuan.lu
// 
// 28    08-04-24 18:00 Haoyuan.lu
// 
// 27    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 26    08-03-05 14:42 Guan.han
// 
// 25    08-03-05 12:15 Guan.han
// 
// 26    08-03-04 18:53 Guan.han
// 
// 21    08-02-02 12:10 Guan.han
// display the item details when append, insert, replace an item
// 
// 20    08-01-07 18:48 Guan.han
// 
// 19    08-01-02 11:42 Guan.han
// 
// 18    07-12-12 12:07 Guan.han
// 
// 17    07-08-15 14:43 Jie.zhang
// 
// 16    07-07-13 12:43 Jie.zhang
// sync
// 
// 15    07-07-12 20:02 Jie.zhang
// * RW lock seem must not used for dictionary, normal lock is ok,  *the
// evictor must inherited from a lock, or will cause program exit
// unexcpected.
// 
// 14    07-07-11 16:05 Jie.zhang
// sync with main tree
// 
// 15    07-07-11 15:53 Jie.zhang
// add lock to every evictor
// 
// 14    07-07-06 14:48 Jie.zhang
// add a confige: enablechannelmaxdurationcheck and add some logic to
// avoid null playlist handle
// 
// 13    06-12-28 13:57 Guan.han
// 
// 12    06-11-15 17:34 Jie.zhang
// 
// 11    06-10-24 18:43 Jie.zhang
// 
// 10    06-10-23 10:04 Jie.zhang
// 
// 9     06-09-27 10:33 Jie.zhang
// 
// 8     06-09-26 11:18 Jie.zhang
// 
// 7     06-09-20 14:32 Jie.zhang
// 
// 6     06-09-04 21:07 Bernie.zhao
// added notification callbacks
// 
// 5     06-09-04 12:30 Bernie.zhao
// modified findItem() to findItem() const
// 
// 4     06-08-29 13:58 Bernie.zhao
// added expiration item monitor
// 
// 3     06-08-28 12:40 Bernie.zhao
// fixed bug when appending an item early than existing items
// 
// 2     06-08-28 12:01 Bernie.zhao
// 1st draft done
// 
// 1     06-08-23 12:44 Bernie.zhao
// ===========================================================================

#include "stdafx.h"
#include "ChannelPublishPointImpl.h"
#include <time.h>
#include "CODConfig.h"


#define LOG_MODULE_NAME			"ChPubPoint"
#define err_300 300
#define err_301 301


#define err_400 400
#define err_401 401
#define err_402 402
#define err_403 403
#define err_404 404

#define err_500 500
#define err_501 501
#define err_502 502
#define err_503 503
#define err_504 504

#define err_600 600
#define err_601 601
#define err_602 602
#define err_603 603
#define err_604 604
#define err_605 605
#define err_606 606
#define err_607 607

#define err_700 700

#define err_800 800


#define NO_TRANSACTION

namespace ZQChannelOnDemand {
		
//////////////////////////////////////////////////////////////////////////
// class ChannelPublishPointImpl
//////////////////////////////////////////////////////////////////////////

bool localTime2TianShanTime(const char* szTime, __int64& lTime)
{
	int nYear,nMon,nDay, nHour, nMin, nSec;
	if (sscanf(szTime, "%d-%d-%dT%d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec)<6)
		return false;
	
	if (nYear < 1970 || nYear > 2100 || nMon < 1 || nMon > 12 || nDay < 1 || nDay > 31 || 
		nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59)
		return false;
	
	// convert to system time
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	st.wYear = nYear;
	st.wMonth = nMon;
	st.wDay = nDay;
	st.wHour = nHour;
	st.wMinute = nMin;
	st.wSecond = nSec;
	
	FILETIME ft_local, ft_utc;
	SystemTimeToFileTime(&st, &ft_local);
	LocalFileTimeToFileTime(&ft_local, &ft_utc);
	
	memcpy(&lTime, &ft_utc, sizeof(lTime));
	lTime = lTime / 10000;  

	return true;
}

bool systemTime2TianShanTime(const char* szTime, __int64& lTime)
{
	int nYear,nMon,nDay, nHour, nMin, nSec;
	if (sscanf(szTime, "%d-%d-%dT%d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec)<6)
		return false;
	
	if (nYear < 1970 || nYear > 2100 || nMon < 1 || nMon > 12 || nDay < 1 || nDay > 31 || 
		nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59)
		return false;
	
	// convert to system time
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	st.wYear = nYear;
	st.wMonth = nMon;
	st.wDay = nDay;
	st.wHour = nHour;
	st.wMinute = nMin;
	st.wSecond = nSec;
	
	FILETIME ft_utc;
	SystemTimeToFileTime(&st, &ft_utc);	
	
	memcpy(&lTime, &ft_utc, sizeof(lTime));
	lTime = lTime / 10000;  

	return true;
}


ChannelPublishPointImpl::ChannelPublishPointImpl(ChODSvcEnv& env)
	: _env(env)
{
}

ChannelPublishPointImpl::~ChannelPublishPointImpl()
{
}

::std::string ChannelPublishPointImpl::getName(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

::std::string ChannelPublishPointImpl::getDesc(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return desc;
}

void ChannelPublishPointImpl::setDesc(const ::std::string& description, const ::Ice::Current&)
{
	RLock sync(*this);
	desc = description;	
}

::TianShanIce::StrValues ChannelPublishPointImpl::getItemSequence(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return itemSequence;
}

::TianShanIce::Properties ChannelPublishPointImpl::getProperties(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return properties;
}

void ChannelPublishPointImpl::setProperties(const ::TianShanIce::Properties& newProps, const ::Ice::Current& c)
{
	WLock sync(*this);
	properties.clear();
	properties = newProps;
}

CI_NS_PREFIX(ChannelItem) ChannelPublishPointImpl::findItem(const ::std::string& itemName, const ::Ice::Current& c) const
{
	RLock sync(*this);
	std::string itemKey = ident.name + CHANNELITEM_KEY_SEPARATOR + itemName;

	// search the record in the dictionary	
	LockT<RecMutex> lk(_env._dictLock);
	::ChannelOnDemand::ChannelItemDict::const_iterator it = _env._pChannelItemDict->find(itemKey);
	if(it == _env._pChannelItemDict->end())
	{ // not found
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_300, CLOGFMT(LOG_MODULE_NAME, "channel item [%s] not found"), itemKey.c_str());
	}

	return it->second.setupInfo;	
}

void ChannelPublishPointImpl::appendItem(const CI_NS_PREFIX(ChannelItem)& newItem, const ::Ice::Current& c)
{
	std::string signature = ZQChannelOnDemand::invokeSignature(c);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "%s"), signature.c_str());

	NS_PREFIX(ChannelOnDemand::ChannelItemEx) newItemCopy;

	{
		WLock sync(*this);

		//check whether items reach the max item number 
		if(_config.MaxItemNumber > 0)
		{
			if(itemSequence.size() >= _config.MaxItemNumber)
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "CodApp", err_301, CLOGFMT(LOG_MODULE_NAME, "channel [%s] with [%d] items, reach the max item number [%d]"), ident.name.c_str(), itemSequence.size(), _config.MaxItemNumber);
			}
		}

		newItemCopy.setupInfo = newItem;
		newItemCopy.setupInfo.lastModified = ZQTianShan::now();
		newItemCopy.key = ident.name + CHANNELITEM_KEY_SEPARATOR + newItem.contentName;
		newItemCopy.flags = 0;
		STRTOLOWER(newItemCopy.key);
		STRTOLOWER(newItemCopy.setupInfo.contentName);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "append [%s] on channel [%s], broadcast[%s], expiration[%s], inOffset[%lld], outOffset[%lld], spliceIn[%d], spliceOut[%d]"), 
			newItemCopy.setupInfo.contentName.c_str(), ident.name.c_str(), newItemCopy.setupInfo.broadcastStart.c_str(), newItemCopy.setupInfo.expiration.c_str(), 
			newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset, 
			newItemCopy.setupInfo.spliceIn, newItemCopy.setupInfo.spliceOut);

		if (!_config.InputLocalTime)
		{
			if (!systemTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_402, CLOGFMT(LOG_MODULE_NAME, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!systemTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}
		else 
		{
			if (!localTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_403, CLOGFMT(LOG_MODULE_NAME, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!localTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}

		// check if the channel item already exists in this channel publish point
		const std::string& newItemName = newItemCopy.setupInfo.contentName;
		::TianShanIce::StrValues::iterator it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), newItemName);
		if (it != itemSequence.end())
		{ // already exists
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_400, CLOGFMT(LOG_MODULE_NAME, "item [%s] already exists in channel [%s]"), newItemName.c_str(), ident.name.c_str());
		}

		try
		{
			LockT<RecMutex> lk(_env._dictLock);

			// DO: check if the last item's broadcasttime is larger than current append item
			if (itemSequence.size() > 0)
			{
				it = itemSequence.end() - 1;
				const ::std::string& strKeyName = ident.name + CHANNELITEM_KEY_SEPARATOR + *it;
				::ChannelOnDemand::ChannelItemDict::iterator dictIt;
				dictIt = _env._pChannelItemDict->find(strKeyName);
				if (dictIt != _env._pChannelItemDict->end() && dictIt->second.broadcastStart > newItemCopy.broadcastStart)
				{
					ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_401, CLOGFMT(LOG_MODULE_NAME, "the append channel item [%s]'s broadcast time is less than the last channel item"), newItemCopy.key.c_str());
				}
			}

			_env._pChannelItemDict->put(::ChannelOnDemand::ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
			itemSequence.push_back(newItemName);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "[%s] appended on channel [%s]"), newItemName.c_str(), ident.name.c_str());
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_404, CLOGFMT(LOG_MODULE_NAME, "add [%s] into safestore caught %s: %s"), 
				newItemCopy.key.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			ex.ice_throw();
		}
	}

	_env.appendPlaylistItem(ident.name, newItemCopy);
}

void ChannelPublishPointImpl::insertItem(const ::std::string& atItemName, const CI_NS_PREFIX(ChannelItem)& newItem, const ::Ice::Current& c)
{
	std::string signature = ZQChannelOnDemand::invokeSignature(c);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "%s"), signature.c_str());

	NS_PREFIX(ChannelOnDemand::ChannelItemEx)	newItemCopy; // new channel item context
	std::string istPosKey, istPosName; // the new item's insert position

	{
		WLock sync(*this);

		//check whether items reach the max item number 
		if(_config.MaxItemNumber > 0)
		{
			if(itemSequence.size() >= _config.MaxItemNumber)
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "CodApp", err_301, CLOGFMT(LOG_MODULE_NAME, "channel [%s] with [%d] items, reach the max item number [%d]"), ident.name.c_str(), itemSequence.size(), _config.MaxItemNumber);
			}
		}

		newItemCopy.setupInfo = newItem;
		newItemCopy.key = ident.name + CHANNELITEM_KEY_SEPARATOR + newItem.contentName;
		newItemCopy.setupInfo.lastModified = ZQTianShan::now();
		STRTOLOWER(newItemCopy.key);
		STRTOLOWER(newItemCopy.setupInfo.contentName);

		istPosKey = ident.name + CHANNELITEM_KEY_SEPARATOR + atItemName;
		istPosName = atItemName;
		STRTOLOWER(istPosKey);
		STRTOLOWER(istPosName);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "insert [%s] at [%s] on channel [%s], broadcast[%s], expiration[%s], inOffset[%lld], outOffset[%lld], spliceIn[%d], spliceOut[%d]"),
			newItemCopy.setupInfo.contentName.c_str(), istPosName.c_str(), ident.name.c_str(), newItemCopy.setupInfo.broadcastStart.c_str(), newItemCopy.setupInfo.expiration.c_str(), 
			newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset, 
			newItemCopy.setupInfo.spliceIn, newItemCopy.setupInfo.spliceOut);

		if (!_config.InputLocalTime)
		{
			if (!systemTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_500, CLOGFMT(LOG_MODULE_NAME, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!systemTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}
		else 
		{
			if (!localTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_501, CLOGFMT(LOG_MODULE_NAME, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}		
			if (!localTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}

		// check if this content already exists
		const std::string& newItemName = newItemCopy.setupInfo.contentName;
		::TianShanIce::StrValues::iterator it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), newItemName);
		if (it != itemSequence.end())
		{ // insert item already exists
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_502, CLOGFMT(LOG_MODULE_NAME, "item [%s] already exists on channel [%s]"), newItemName.c_str(), ident.name.c_str());
		}

		it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), istPosName);
		if(it == itemSequence.end())
		{ // insert at item not found
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_503, CLOGFMT(LOG_MODULE_NAME, "insert position [%s] not found on channel [%s]"), istPosName.c_str(), ident.name.c_str());
		}

		try
		{
			LockT<RecMutex> lk(_env._dictLock);		
			_env._pChannelItemDict->put(::ChannelOnDemand::ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
		}
		catch (const Freeze::DatabaseException& ex)
		{ // add channel item context to channel item dict failed
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_504, CLOGFMT(LOG_MODULE_NAME, "add channel item context to channel item dict caught %s: %s"), 
				ex.ice_name().c_str(), ex.message.c_str());
		}

		// then update the item sequence list
		itemSequence.insert(it, newItemName);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "item [%s] inserted at [%s] on channel [%s]"), 
			newItemName.c_str(), istPosName.c_str(), ident.name.c_str());
	}

	_env.insertPlaylistItem(ident.name, istPosName, newItemCopy);
}

void ChannelPublishPointImpl::replaceItem(const ::std::string& oldName, const CI_NS_PREFIX(ChannelItem)& newItem, const ::Ice::Current& c)
{
	std::string signature = ZQChannelOnDemand::invokeSignature(c);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "%s"), signature.c_str());

	NS_PREFIX(ChannelOnDemand::ChannelItemEx) newItemCopy;
	std::string oldItemKey, oldItemName;

	{
		WLock sync(*this);

		newItemCopy.setupInfo = newItem;
		newItemCopy.key = ident.name + CHANNELITEM_KEY_SEPARATOR + newItem.contentName;
		newItemCopy.setupInfo.lastModified = ZQTianShan::now();
		STRTOLOWER(newItemCopy.key);
		STRTOLOWER(newItemCopy.setupInfo.contentName);

		oldItemKey = ident.name + CHANNELITEM_KEY_SEPARATOR + oldName;
		oldItemName = oldName;
		STRTOLOWER(oldItemKey);
		STRTOLOWER(oldItemName);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "replace [%s] with [%s] on channel [%s], broadcast[%s], expiration[%s], inOffset[%lld], outOffset[%lld], spliceIn[%d], spliceOut[%d]"),
			oldItemName.c_str(), newItemCopy.setupInfo.contentName.c_str(), ident.name.c_str(), newItemCopy.setupInfo.broadcastStart.c_str(), newItemCopy.setupInfo.expiration.c_str(), 
			newItemCopy.setupInfo.inTimeOffset, newItemCopy.setupInfo.outTimeOffset, 
			newItemCopy.setupInfo.spliceIn, newItemCopy.setupInfo.spliceOut);

		if (!_config.InputLocalTime)
		{
			if (!systemTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_600, CLOGFMT(LOG_MODULE_NAME, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!systemTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}
		else 
		{
			if (!localTime2TianShanTime(newItem.broadcastStart.c_str(), newItemCopy.broadcastStart))
			{
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_601, CLOGFMT(LOG_MODULE_NAME, "invalid time format [%s]"), newItem.broadcastStart.c_str());
			}
			if (!localTime2TianShanTime(newItem.expiration.c_str(), newItemCopy.expiration))
				newItemCopy.expiration = 0;
		}

		// check if this content already exists
		const std::string& newItemName = newItemCopy.setupInfo.contentName;
		::TianShanIce::StrValues::iterator itNew = itemSequence.end(), itOld = itemSequence.end();

		// check if this old content exists
		itOld = ::std::find(itemSequence.begin(), itemSequence.end(), oldItemName);
		if(itOld == itemSequence.end())
		{
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_605, CLOGFMT(LOG_MODULE_NAME, "replace item [%s] with [%s] on channel [%s] caught old item not exist"), 
				oldItemName.c_str(), newItemName.c_str(), ident.name.c_str());
		}

		itNew = ::std::find(itemSequence.begin(), itemSequence.end(), newItemName);
		if (itNew != itemSequence.end())
		{ // if the new item's name cound be found in dict means modify the existing one's properties
			if (oldItemName != newItemName)
			{
				// channel item already exists but not modifing the properties
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_602, CLOGFMT(LOG_MODULE_NAME, "channel item [%s] already exists"), newItemCopy.key.c_str());
			}

			// channel item already exists but modifing the properties
			LockT<RecMutex> lk(_env._dictLock);
			::ChannelOnDemand::ChannelItemDict::iterator dictIt;
			dictIt = _env._pChannelItemDict->find(oldItemKey);
			if (dictIt ==_env._pChannelItemDict->end())
			{
				itemSequence.erase(itNew); // not found in dict but in item sequence, so you have to erase it
				ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_603, CLOGFMT(LOG_MODULE_NAME, "channel item [%s] not found in dict"), newItemCopy.key.c_str());
			}

			const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& oldItemContx = dictIt->second;
			if (newItemCopy.setupInfo.inTimeOffset == oldItemContx.setupInfo.inTimeOffset && 
				newItemCopy.setupInfo.outTimeOffset == oldItemContx.setupInfo.outTimeOffset)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "replace item but (cueIn and cueOut equal to old one), so ignore"));
				return;
			}

			try
			{
				// replace the record in the dictionary
				_env._pChannelItemDict->put(::ChannelOnDemand::ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
			}
			catch(const Freeze::DatabaseException& ex)
			{
				ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_606, CLOGFMT(LOG_MODULE_NAME, "update [%s] properties on channel [%s] caught %s: %s"), 
					newItemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_603, CLOGFMT(LOG_MODULE_NAME, "update [%s] properties on channel [%s] caught %s"), 
					newItemName.c_str(), ex.ice_name().c_str());
			}
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "item [%s] on channel [%s] replaced"), newItemName.c_str(), ident.name.c_str());
		}
		else // if the new item not found in dict means replace the old one with the new one
		{	
			try
			{
				LockT<RecMutex> lk(_env._dictLock);
				// replace the record in the dictionary
				_env._pChannelItemDict->erase(oldItemKey);
				_env._pChannelItemDict->put(::ChannelOnDemand::ChannelItemDict::value_type(newItemCopy.key, newItemCopy));
			}
			catch (const Freeze::DatabaseException& ex)
			{
				ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_606, CLOGFMT(LOG_MODULE_NAME, "update [%s] with [%s] on channel [%s] caught %s: %s"), 
					oldItemName.c_str(), newItemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_607, CLOGFMT(LOG_MODULE_NAME, "update [%s] with [%s] on channel [%s] caught %s"), 
					oldItemName.c_str(), newItemName.c_str(), ex.ice_name().c_str());
			}
			// then update the item sequence list
			*itOld = newItemName;
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "replace [%s] with [%s] on channel [%s] successfully"), oldItemName.c_str(), newItemName.c_str(), ident.name.c_str());
		}
	}

	_env.replacePlaylistItem(ident.name, oldItemKey, newItemCopy);
}

void ChannelPublishPointImpl::removeItem(const ::std::string& itemName, const ::Ice::Current& c)
{
	std::string signature = ZQChannelOnDemand::invokeSignature(c);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "%s"), signature.c_str());

	std::string rmvItemKey, rmvItemName;
	{
		WLock sync(*this);

		rmvItemKey = ident.name + CHANNELITEM_KEY_SEPARATOR + itemName;
		rmvItemName = itemName;
		STRTOLOWER(rmvItemKey);
		STRTOLOWER(rmvItemName);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "remove item [%s] on channel [%s]"), rmvItemName.c_str(), ident.name.c_str());

		// remove from channel item dict
		try
		{
			LockT<RecMutex>  lk(_env._dictLock);
			_env._pChannelItemDict->erase(rmvItemKey);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_700, CLOGFMT(LOG_MODULE_NAME, "remove item [%s] on channel [%s] caught %s: %s"), 
				rmvItemName.c_str(), ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_700, CLOGFMT(LOG_MODULE_NAME, "remove item [%s] on channel [%s] caught %s"), 
				rmvItemName.c_str(), ident.name.c_str(), ex.ice_name().c_str());
		}

		// remove from item sequence
		::TianShanIce::StrValues::iterator it = itemSequence.end();
		it = ::std::find(itemSequence.begin(), itemSequence.end(), rmvItemName);
		if (it != itemSequence.end())
		{
			// remove item name from sequence
			itemSequence.erase(it);
		}

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "item [%s] removed from channel [%s]"), rmvItemName.c_str(), ident.name.c_str());
	}

	_env.removePlaylistItem(ident.name, rmvItemName);
}

void ChannelPublishPointImpl::destroy(const ::Ice::Current& c)
{
	std::string caller = (c.con)->toString();
	caller = caller.substr(caller.find_last_of("=")+1, caller.size());
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "remove channel [%s] from %s"), ident.name.c_str(), caller.c_str());

	std::string signature = ZQChannelOnDemand::invokeSignature(c);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "%s"), signature.c_str());

	//
	// must call destroy before removing from evictor, because the index will be removed if channel removed from evictor
	//
	OnChannelDestroyed(); // to destroy all purchases on the channel

	{
	WLock sync(*this);
	try
	{
//		LockT<RecMutex> lk(_env._evitCPPLock);
		_env._evitChannelPublishPoint->remove(ident);
	}
	catch(const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, LOG_MODULE_NAME, err_800, CLOGFMT(LOG_MODULE_NAME, "remove channel [%s] from evictor caught %s: %s"), 
			ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "channel [%s] removed"), ident.name.c_str());
	}	
}

::Ice::Int ChannelPublishPointImpl::getMaxBitrate(const ::Ice::Current& c) const
{
	RLock sync(*this);

	return maxBitrate;
}

void ChannelPublishPointImpl::setMaxBitrate(::Ice::Int newMaxBitrate, const ::Ice::Current& c)
{
	WLock sync(*this);

	if (newMaxBitrate)
	{
		maxBitrate = newMaxBitrate;
	}
	else
	{
		maxBitrate = _config.DefaultChannelMaxBitrate;
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "setMaxBitrate to %d"), newMaxBitrate);	
}


void ChannelPublishPointImpl::OnChannelDestroyed()
{
	_env.removePurchases(ident.name);
}

::std::string ChannelPublishPointImpl::getOnDemandName(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return onDemandName;
}

void ChannelPublishPointImpl::setOnDemandName(const ::std::string& name, const ::Ice::Current&)
{
	WLock sync(*this);
	onDemandName = name;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "set channel [%s]'s on demand name [%s]"), ident.name.c_str(), name.c_str());
}

void ChannelPublishPointImpl::restrictReplica(const ::TianShanIce::StrValues& reps, const ::Ice::Current& c)
{
	WLock sync(*this);
	replicas = reps;
	for (uint i = 0, count = reps.size(); i < count; i ++)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "restrict channel [%s] on netId [%s]"), ident.name.c_str(), reps[i].c_str());
	}
}

::TianShanIce::StrValues ChannelPublishPointImpl::listReplica(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return replicas;
}

#ifdef USE_OLD_NS

void ChannelPublishPointImpl::enable(bool status, const ::Ice::Current& c)
{
	WLock sync(*this);
	bEnable = status;
}

bool ChannelPublishPointImpl::ifEnable(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return bEnable;
}

#else

::std::string ChannelPublishPointImpl::getType(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return type;
}

void ChannelPublishPointImpl::allowDemand(bool status, const ::Ice::Current& c)
{
	WLock sync(*this);
	bAllowDemand = status;
}

bool ChannelPublishPointImpl::isAvailableOnDemand(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return bAllowDemand;
}

void ChannelPublishPointImpl::appendItemAs(const ::TianShanIce::Application::ChannelItem& newItem, const ::std::string& newName, const ::Ice::Current& c)
{
}

void ChannelPublishPointImpl::insertItemAs(const ::std::string& atItemName, const ::TianShanIce::Application::ChannelItem& newItem, const ::std::string& newName, const ::Ice::Current& c)
{
}

#endif //USE_OLD_NS
//void ChannelPublishPointImpl::checkChannelDuration(LONGLONG lastStartTime)
//{
//	std::vector<std::string> toRemove;
//
//	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Check max-duration for channel [%s] enter"), ident.name.c_str());
//
//	int nMinutes = 10;
//	LONGLONG maxDuration = _config.MaxChannelDurationInMinutes * 60 * 1000; 
//
//	::TianShanIce::StrValues::iterator it;
//
//	{
//		WLock sync(*this);
//
//		LockT<RecMutex> lk(_env._dictLock);
//		while(itemSequence.size()>1)
//		{
//			it = itemSequence.begin();
//
//			::std::string strKeyName = ident.name + CHANNELITEM_KEY_SEPARATOR + *it;
//			::ChannelOnDemand::ChannelItemDict::iterator dictIt;
//			
//			try
//			{
//				dictIt = _env._pChannelItemDict->find(strKeyName);
//				if(dictIt!=_env._pChannelItemDict->end())
//				{
//					if(dictIt->second.broadcastStart + maxDuration >= lastStartTime)
//					{
//						// no more than max duration, so leave
//						break;	
//					}
//				}			
//			}
//			catch(const TianShanIce::InvalidParameter&)
//			{
//				glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "InvalidParameter exception caught during find item [%s]"), 
//					strKeyName.c_str());
//				break;
//			}
//			catch(...)
//			{
//				glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Unknown exception caught during find item [%s]"), 
//					strKeyName.c_str());
//				break;
//			}
//
//			try
//			{
//				_env._pChannelItemDict->erase(strKeyName);				
//				
//				// then update the item sequence list
//				itemSequence.erase(it);
//			}
//			catch(const Freeze::DatabaseException& ex)
//			{
//				glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught DatabaseException while item [%s] removing operation, %s"),
//					strKeyName.c_str(), ex.message.c_str());
//				break;
//			}
//			catch(...)
//			{
//				glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught unknown exception while item [%s] removing operation"),
//					strKeyName.c_str());
//				break;
//			}
//			
//			toRemove.push_back(strKeyName);		
//		}
//	}
//
//	for(unsigned int k=0;k<toRemove.size();k++)
//	{
//		//do remove
//		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "remove item [%s] because of exceed max duration"), toRemove[k].c_str());
//		OnItemRemoved(toRemove[k]);
//	}
//
//	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Check max-duration for channel [%s] leave"), ident.name.c_str());
//}

}	// end of namespace ZQChannelOnDemand
