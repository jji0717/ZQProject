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
// Ident : $Id: PathManagerImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathManagerImpl.cpp $
// 
// 42    3/22/17 6:29p Hui.shao
// CLOGFMT
// 
// 41    2/23/16 10:38a Hui.shao
// include rtServiceGroup in the PathTicket
// 
// 40    2/22/16 2:09p Hui.shao
// added pattern on restriction matching with wildcast '?' and '*'
// 
// 36    6/17/15 7:00p Hui.shao
// 
// 35    5/20/15 11:41a Hui.shao
// merged from main tree about AliveStreamerCollector
// 
// 38    5/06/15 6:12p Hui.shao
// 
// 37    5/06/15 6:09p Hui.shao
// 
// 34    3/23/15 11:06a Hui.shao
// 
// 33    3/23/15 10:12a Hui.shao
// 
// 32    3/16/15 5:36p Hui.shao
// 
// 31    1/23/15 6:27p Hongquan.zhang
// 
// 30    1/23/15 5:07p Hongquan.zhang
// 
// 29    1/23/15 3:29p Hui.shao
// lock dict before use
// 
// 27    8/11/14 6:15p Hui.shao
// 
// 26    7/14/14 1:32p Zhiqiang.niu
// add function importStatus to set the flags of the link
// 
// 25    7/09/14 10:44a Zhiqiang.niu
// 
// 25    7/01/14 5:03p Zhiqiang.niu
// add attribute ?ˉstatus?? at StreamLink/StorageLink
// 
// 24    6/06/14 4:11p Hui.shao
// 
// 23    6/05/14 11:08a Hui.shao
// throw exception 1180 at invalid servicegroup id
// 
// 21    3/11/14 12:28p Hui.shao
// corrected erase() call
// 22    5/13/14 9:39a Li.huang
// fix bug18996
// 
// 20    2/20/14 5:35p Hui.shao
// 
// 20    2/20/14 5:29p Hui.shao
// 
// 19    2/20/14 5:25p Hui.shao
// merged from V1.16
// 
// 19    2/14/14 11:52a Hui.shao
// added config  _streamLinksByMaxTicket
// 
// 18    2/14/14 10:35a Hui.shao
// 
// 17    2/13/14 4:09p Hui.shao
// declare Link::status() as readonly to reduce disk io
// 
// 16    8/28/13 3:22p Hui.shao
// about pathticket restoring
// 
// 15    4/02/13 5:45p Hui.shao
// make standby threshold 0~0 as involve instantly and forever
// 
// 14    3/26/13 3:30p Hui.shao
// 
// 13    3/26/13 2:41p Hui.shao
// 
// 12    3/26/13 10:09a Hui.shao
// 
// 11    3/25/13 6:00p Hui.shao
// 
// 10    3/05/13 11:52a Hui.shao
// enh#17725 Weiwoo to specially deal with "$Standby" StorageLinks
// 
// 9     1/16/13 1:32p Hui.shao
// 
// 8     12/21/12 11:25a Hui.shao
// enable/disable streamlink
// 
// 7     12/21/12 11:08a Hui.shao
// log params
// 
// 6     12/20/12 11:17a Hui.shao
// 
// 5     6/12/12 10:59a Hongquan.zhang
// 
// 4     3/10/11 1:51p Hongquan.zhang
// 
// 3     3/09/11 4:26p Hongquan.zhang
// 
// 2     3/07/11 4:55p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 112   10-06-24 14:00 Build
// 
// 111   10-06-23 16:59 Hui.shao
// 
// 110   10-02-05 13:58 Yixin.tian
// change atol to atoi64
// 
// 109   09-12-29 16:46 Yixin.tian
// dumpToXmlFile() add code to catch exception and throw servererror
// exception
// 
// 108   09-12-28 11:44 Xiaohui.chai
// 
// 107   09-09-23 17:48 Xiaohui.chai
// 
// 106   09-09-23 12:27 Xiaohui.chai
// detail error message
// 
// 105   09-09-23 10:54 Xiaohui.chai
// 
// 104   09-09-22 17:15 Xiaohui.chai
// implement the importXml2()
// 
// 103   09-09-17 17:04 Xiaohui.chai
// extract the xml parsing function
// 
// 102   09-09-07 13:58 Xiaohui.chai
// add error detail for import failure
// 
// 101   09-09-07 11:32 Xiaohui.chai
// extend the operation of import xml file to Modify/Append/Delete
// 
// 100   09-06-18 11:40 Build
// 
// 99    09-06-17 16:42 Xiaoming.li
// add random seed for shuffle
// 
// 98    09-05-22 15:55 Jie.zhang
// 
// 97    09-05-20 10:32 Yixin.tian
// 
// 96    09-03-05 14:55 Hongquan.zhang
// 
// 95    09-02-25 11:30 Hongquan.zhang
// receive and update replicas information
// 
// 94    09-01-15 15:14 Hui.shao
// rollback to version 92 to wipe out the log statement
// 
// 92    08-12-29 17:50 Yixin.tian
// add log parameter
// 
// 91    08-12-29 15:23 Xiaohui.chai
// tune log level
// 
// 90    08-12-29 15:06 Xiaohui.chai
// 
// 89    08-12-29 12:18 Hui.shao
// 
// 88    08-12-23 10:12 Yixin.tian
// doImportXml() add catch TianShanIce::BaseException
// 
// 87    08-11-24 15:29 Hongquan.zhang
// 
// 86    08-11-21 14:07 Hongquan.zhang
// assign a value to attr and status when add a new resource into
// resourceMap
// 
// 85    08-10-09 16:52 Xiaohui.chai
// update error handling in importXml()
// 
// 84    08-08-14 14:52 Hui.shao
// merged from 1.7.10
// 
// 83    08-07-08 13:37 Hui.shao
// fixed for ICE 3.3 new syntax
// 
// 82    08-05-28 17:12 Hongquan.zhang
// 
// 81    08-03-28 16:25 Yixin.tian
// add nodename to use XMLPreferenceEx member firstchild 
// 
// 80    08-03-24 17:46 Build
// check in for 64bit build
// 
// 79    08-03-18 14:50 Hongquan.zhang
// must lock servant when using it
// 
// 78    08-01-03 15:53 Hui.shao
// 
// 77    07-12-21 16:11 Xiaohui.chai
// 
// 76    07-12-14 18:04 Hongquan.zhang
// Use ZQTianShan::Adapter
// 
// 75    07-12-14 11:37 Hongquan.zhang
// Check in for updating ErrorCode
// 
// 74    07-12-07 13:56 Hui.shao
// 
// 73    07-12-07 13:54 Xiaohui.chai
// 
// 72    07-11-27 13:36 Xiaohui.chai
// 
// 71    07-11-16 17:06 Xiaohui.chai
// 
// 70    07-10-16 14:23 Yixin.tian
// 
// 69    07-10-09 12:06 Yixin.tian
// 
// 68    07-09-27 16:26 Yixin.tian
// 
// 67    07-09-26 10:34 Yixin.tian
// 
// 66    07-09-21 13:02 Yixin.tian
// 
// 65    07-09-18 12:55 Hongquan.zhang
// 
// 61    07-08-30 15:39 Hongquan.zhang
// 
// 60    07-08-15 17:03 Hongquan.zhang
// // 59    07-08-15 16:58 Hui.shao
// added dumpDot(), dumpXml(), importXml()
// 
// 58    07-06-27 10:01 Hongquan.zhang
// 
// 57    07-06-06 16:07 Hongquan.zhang
// 
// 56    07-05-24 11:18 Hongquan.zhang
// 
// 55    07-05-16 16:55 Hongquan.zhang
// 
// 54    07-05-09 17:43 Hongquan.zhang
// 
// 53    07-04-09 17:15 Hongquan.zhang
// 
// 52    07-03-22 18:05 Hongquan.zhang
// 
// 51    07-03-22 17:41 Hongquan.zhang
// 
// 50    07-03-22 17:24 Hongquan.zhang
// 
// 49    07-03-20 10:58 Hongquan.zhang
// 
// 48    07-03-14 12:33 Hongquan.zhang
// 
// 47    07-03-01 15:27 Hongquan.zhang
// 
// 46    07-01-11 16:09 Hongquan.zhang
// 
// 45    07-01-09 15:15 Hongquan.zhang
// 
// 44    07-01-05 18:13 Hongquan.zhang
// 
// 43    07-01-05 10:59 Hongquan.zhang
// 
// 42    06-12-25 16:58 Hui.shao
// fixed glog to envlog; _throw with envlog
// 
// 41    06-12-25 12:23 Hongquan.zhang
// 
// 40    06-12-18 14:02 Hongquan.zhang
// 
// 39    06-12-13 18:47 Hongquan.zhang
// 
// 38    9/21/06 4:34p Hui.shao
// batch checkin on 20060921
// 
// 37    06-09-05 12:53 Hui.shao
//
// 29    06-08-10 12:46 Hui.shao
// moved bandwidth into the privateData
// 
// 25    06-08-04 17:05 Hui.shao
// defined the PathHelper interface for plugin
// 
// 24    06-07-26 20:13 Hui.shao
// 
// 23    06-07-26 15:22 Hui.shao
// modified the list link apis
// 13    06-06-14 11:06 Hui.shao
// added PathHelperMgr to eval the ticket cost
// 
// 12    06-06-13 22:12 Hui.shao
// initial draft of ticket eval
// 
// 11    06-06-12 15:56 Hui.shao
// added file header
// ===========================================================================
// #define NEWLOGFMT
#include "PathManagerImpl.h"
#include "PathCommand.h"
#include "Log.h"

#include <set>
#include <vector>
#include <algorithm>
#include <functional>
#include <sstream>
#include <time.h>

#include <TianShanIceHelper.h>
#include <strHelper.h>

#include "ServiceCommonDefintion.h"

namespace ZQTianShan {
namespace AccreditedPath {

#define TRY_BEGIN()			try {
#define TRY_END(_PROMPT)	} catch(const Ice::Exception& ex) { ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,_PROMPT,1001,  "exception: %s", ex.ice_name().c_str()} \
	catch(...)	{ ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog,_PROMPT,1002, " unkown exception"); }


//#define OBJLOGFMT(_CLASS, _X) CLOGFMT(_CLASS, "[%s] " _X), c.id.name.c_str()

extern ZQ::common::AtomicInt _gTotalTickets;

// -----------------------------
// class StorageLinkImpl
// -----------------------------
StorageLinkImpl::StorageLinkImpl(PathSvcEnv& env)
: _env(env)
{
	WLock sync(*this);
}

void StorageLinkImpl::destroy(const Ice::Current& c)
{
	WLock sync(*this);
	try
	{
		envlog(ZQ::common::Log::L_INFO, OBJLOGFMT(StorageLink, "destroy() with id is %s"),c.id.name.c_str());
		_env._cachedIdxStreamerToStorageLink.erase(streamerId, ident);
		_env._eStorageLink->remove(c.id);
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StorageLink, "object already gone, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,"StorageLink",1011,OBJLOGFMT(StorageLink, "destroy() caught exception: %s"), ex.ice_name().c_str());
	}
}

::Ice::Identity StorageLinkImpl::getIdent(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return ident;
}

::std::string StorageLinkImpl::getType(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return type;
}

::TianShanIce::Transport::Storage StorageLinkImpl::getStorageInfo(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StorageLink, "getStorageInfo()"));
    RLock sync(*this);
	static ::TianShanIce::Transport::Storage Nil;
	ZQ::common::MutexGuard gd(_env._lockStorageDict);
	
	TianShanIce::Transport::StorageDict::iterator it = _env._pStorageDict->find(storageId);
	if (_env._pStorageDict->end() == it)
		return Nil;
	return it->second;
}

::TianShanIce::Transport::Streamer StorageLinkImpl::getStreamerInfo(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StorageLink, "getStreamerInfo()"));

    RLock sync(*this);
	static ::TianShanIce::Transport::Streamer Nil;
	ZQ::common::MutexGuard gd(_env._lockStreamerDict);
	
	TianShanIce::Transport::StreamerDict::iterator it = _env._pStreamerDict->find(streamerId);
	if (_env._pStreamerDict->end() == it)
		return Nil;
	return it->second;
}

::TianShanIce::ValueMap StorageLinkImpl::getPrivateData(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StorageLink, "getPrivateData()"));

	::TianShanIce::Variant v;
	v.type = ::TianShanIce::vtInts;
	v.bRange = false;

	RLock sync(*this);
	::TianShanIce::ValueMap pd = privateData;
	v.ints.push_back(((uint32)revision) >>24);
	MAPSET(::TianShanIce::ValueMap, pd, "CtrlStatus", v);
	return pd;
}

bool StorageLinkImpl::setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& val, const ::Ice::Current& c)
{
	if (key.empty())
		return false;

	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StorageLink, "setPrivateData with key=%s"), key.c_str());

	::TianShanIce::ValueMap newConfig;
	{
		RLock sync(*this);		
		newConfig = privateData;
		newConfig[key] = val;
		_env.pathHelperMgr().validateStorageLinkConfig(type.c_str(), ident.name.c_str(), newConfig);
	}

	{
		WLock sync(*this);
		privateData[key] = val;		
	}
	return true;
}

#define REVISION_MASK (0x00ffffff)
#define LINK_IS_ENA(_LINK)  (0 == (_LINK).revision & (1<<24))
#define LINK_ENABLE(_LINK)  ((_LINK).revision &= ~((uint32)(1<<24)))
#define LINK_DISABLE(_LINK) ((_LINK).revision |= (1<<24))

::Ice::Int StorageLinkImpl::getRevision(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return (revision & REVISION_MASK);
}

void StorageLinkImpl::updateRevision(::Ice::Int newRevision, const ::Ice::Current& c)
{
    WLock sync(*this);
	revision &= 0xff000000;
	revision |= (newRevision & REVISION_MASK);
}

int  StorageLinkImpl::status(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return (((uint32)revision) >>24);
}

void StorageLinkImpl::enableLink(bool enable, const ::Ice::Current& c)
{
    WLock sync(*this);
	if (enable)
		LINK_ENABLE(*this);
	else
		LINK_DISABLE(*this);
}

void StorageLinkImpl::importStatus(::Ice::Int status, const ::Ice::Current& c)
{
	WLock sync(*this);
	revision &= 0x00ffffff;
	revision |= (((uint32)status) << 24);
}

static void dumpLine(const char* line, void* pCtx)
{
	if (line && pCtx)
		((PathSvcEnv*)pCtx)->_log(ZQ::common::Log::L_DEBUG, line);
}

bool StorageLinkImpl::updatePrivateData(const ::TianShanIce::ValueMap& newValues, const ::Ice::Current& c)
{
//	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StorageLink, "updatePrivateData()"));
	char hint[120];
	snprintf(hint, sizeof(hint)-2, OBJLOGFMT(StorageLink, "updatePrivateData()"));
	::ZQTianShan::dumpValueMap(newValues, hint, dumpLine, &_env);

	::TianShanIce::ValueMap newConfig = newValues;

	_env.pathHelperMgr().validateStorageLinkConfig(type.c_str(), ident.name.c_str(), newConfig);
	if (! _env._cachedIdxStreamerToStorageLink.has(streamerId, ident))
		_env._cachedIdxStreamerToStorageLink.insert(streamerId, ident);

    WLock sync(*this);
	privateData = newConfig;
	::TianShanIce::ValueMap::iterator itRev = newConfig.find("revision");
	if (newConfig.end() != itRev)
	{
		// found the revision code from the PHO impl, set it into link.revision
		uint32 newRevision = revision;
		if (::TianShanIce::vtInts ==  itRev->second.type && itRev->second.ints.size() >0)
		{
			newRevision = (revision & ~((uint) REVISION_MASK)) | (itRev->second.ints[0] & REVISION_MASK);
			revision = newRevision; 
		}
	}

	snprintf(hint, sizeof(hint)-2, OBJLOGFMT(StorageLink, "updatePrivateData() submitted: rev[0x%x]"), revision);
	::ZQTianShan::dumpValueMap(newConfig, hint, dumpLine, &_env);
	return true;
}


// -----------------------------
// class StreamLinkImpl
// -----------------------------
StreamLinkImpl::StreamLinkImpl(PathSvcEnv& env)
: _env(env)
{}

void StreamLinkImpl::destroy(const Ice::Current& c)
{
	RLock sync(*this);
	try
	{
		envlog(ZQ::common::Log::L_INFO, OBJLOGFMT(StreamLink, "destroy()"));
		_env._cachedIdxSvcGrpToStreamLink.erase(servicegroupId, ident);
		_env._eStreamLink->remove(c.id);
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StreamLink, "object already gone, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,"StreamLink",1021, OBJLOGFMT(StreamLink, "destroy() caught exception: %s"), ex.ice_name().c_str());
	}
}

::Ice::Identity StreamLinkImpl::getIdent(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return ident;
}

::std::string StreamLinkImpl::getType(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return type;
}

::TianShanIce::Transport::ServiceGroup StreamLinkImpl::getServiceGroupInfo(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StreamLink, "getServiceGroupInfo()"));

    RLock sync(*this);
	static ::TianShanIce::Transport::ServiceGroup Nil;
	ZQ::common::MutexGuard gd(_env._lockServiceGroupDict);
	
	TianShanIce::Transport::ServiceGroupDict::iterator it = _env._pServiceGroupDict->find(servicegroupId);
	if (_env._pServiceGroupDict->end() == it)
		return Nil;
	return it->second;
}

::TianShanIce::Transport::Streamer StreamLinkImpl::getStreamerInfo(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StreamLink, "getStreamerInfo()"));

    RLock sync(*this);
	static ::TianShanIce::Transport::Streamer Nil;
	ZQ::common::MutexGuard gd(_env._lockStreamerDict);
	
	TianShanIce::Transport::StreamerDict::iterator it = _env._pStreamerDict->find(streamerId);
	if (_env._pStreamerDict->end() == it)
	{
		envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(StreamLink, "getStreamerInfo() no record found"));
		return Nil;
	}

	return it->second;
}

bool StreamLinkImpl::setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& val, const ::Ice::Current& c)
{
	if (key.empty())
		return false;

	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StreamLink, "setPrivateData(%s)"), key.c_str());

	::TianShanIce::ValueMap newConfig;
	{
		RLock sync(*this);		
		newConfig = privateData;
		newConfig[key] = val;
		_env.pathHelperMgr().validateStreamLinkConfig(type.c_str(), ident.name.c_str(), newConfig);
	}
	{
		WLock sync(*this);
		privateData[key] = val;
		newConfig = privateData;
	}
	
	
	return true;
}

::Ice::Int StreamLinkImpl::getRevision(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return (revision & REVISION_MASK);
}

void StreamLinkImpl::updateRevision(::Ice::Int newRevision, const ::Ice::Current& c)
{
    WLock sync(*this);
	revision &= 0xff000000;
	revision |= (newRevision & REVISION_MASK);
}

bool StreamLinkImpl::updatePrivateData(const ::TianShanIce::ValueMap& newValues, const ::Ice::Current& c)
{
	char hint[120];
	snprintf(hint, sizeof(hint)-2, OBJLOGFMT(StreamLink, "updatePrivateData()"));
	::ZQTianShan::dumpValueMap(newValues, hint, dumpLine, &_env);

	::TianShanIce::ValueMap newConfig = newValues;

	_env.pathHelperMgr().validateStreamLinkConfig(type.c_str(), ident.name.c_str(), newConfig);
	if (! _env._cachedIdxSvcGrpToStreamLink.has(servicegroupId, ident))
		_env._cachedIdxSvcGrpToStreamLink.insert(servicegroupId, ident);

    WLock sync(*this);
	privateData = newConfig;
	::TianShanIce::ValueMap::iterator itRev = newConfig.find("revision");
	if (newConfig.end() != itRev)
	{
		// found the revision code from the PHO impl, set it into link.revision
		uint32 newRevision = revision;
		if (::TianShanIce::vtInts ==  itRev->second.type && itRev->second.ints.size() >0)
		{
			newRevision = (revision & ~((uint) REVISION_MASK)) | (itRev->second.ints[0] & REVISION_MASK);
			revision = newRevision; 
		}
	}

	snprintf(hint, sizeof(hint)-2, OBJLOGFMT(StreamLink, "updatePrivateData() submitted: rev[0x%x]"), revision);
	::ZQTianShan::dumpValueMap(newConfig, hint, dumpLine, &_env);

	return true;
}

int StreamLinkImpl::status(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return (((uint32)revision) >>24);
}

void StreamLinkImpl::enableLink(bool enable, const ::Ice::Current& c)
{
    WLock sync(*this);
	if (enable)
		LINK_ENABLE(*this);
	else
		LINK_DISABLE(*this);
}

void StreamLinkImpl::importStatus(::Ice::Int status, const ::Ice::Current& c)
{
	WLock sync(*this);
	revision &= 0x00ffffff;
	revision |= (((uint32)status) << 24);
}

::TianShanIce::ValueMap StreamLinkImpl::getPrivateData(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(StreamLink, "getPrivateData()"));

	::TianShanIce::Variant v;
	v.type = ::TianShanIce::vtInts;
	v.bRange = false;

    RLock sync(*this);
	::TianShanIce::ValueMap pd = privateData;
	v.ints.push_back(((uint32)revision) >>24);
	MAPSET(::TianShanIce::ValueMap, pd, "CtrlStatus", v);
	return pd;
}

// -----------------------------
// class ADPathTicketImpl
// -----------------------------
ADPathTicketImpl::ADPathTicketImpl(PathSvcEnv& env)
: _env(env)
{
	WLock sync(*this);
//	printf("create ticket %s/%s\n", id.category.c_str(), id.name.c_str());
	costWhenAllocated =0; // start the cost with 0
}
ADPathTicketImpl::~ADPathTicketImpl()
{
	WLock sync(*this);
}

bool ADPathTicketImpl::setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& val, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	char szLocalBuffer[1024] = {0};
	char* p = szLocalBuffer;
	size_t sz = sizeof(szLocalBuffer);
	ZQTianShan::Util::dumpTianShanVariant( val , p ,sz );	
	envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(AccreditedPaths,"set privData [%s]=[%s]"),key.c_str() , szLocalBuffer );
	
	{
		WLock sync(*this);
		privateData[key] = val;
	}

	return true;
}

//virtual void commit_async(const ::TianShanIce::Transport::AMD_PathTicket_commitPtr&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current()) = 0;
void ADPathTicketImpl::commit_async(const ::TianShanIce::Transport::AMD_PathTicket_commitPtr& callback, 
									const ::TianShanIce::SRM::SessionPrx& sessPrx,
									const ::Ice::Current&)
{
	SessCtx sessCtx;

	if (sessPrx)
	{
		try
		{
			sessCtx.sessId = sessPrx->getId();
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "commit() ticket for session[%s]"), sessCtx.sessId.c_str());
			
			// start validate the session context here
			
			// v.1 validate the session state
			sessCtx.state = sessPrx->getState();
			
			sessCtx.strPrx = _env._communicator->proxyToString(sessPrx);
			
			sessCtx.resources = sessPrx->getReources();
			sessCtx.privdata  = sessPrx->getPrivateData();
#ifdef _DEBUG
			::ZQTianShan::dumpResourceMap(sessCtx.resources);
#endif //_DEBUG
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPaths, "commit() failed to retrieve session context for committng"));
			callback->ice_exception(::TianShanIce::ServerError("AccreditedPaths", 1031, "commit() failed to retrieve session context for committng"));
			return;
		}
	}
			
	std::string errMsg;
	char buf[1024] ="\0";

	try
	{
		//lock path ticket in PathHelperMgr
		if(! _env.pathHelperMgr().doCommit(this, sessCtx))
		{
			snprintf(buf, sizeof(buf)-2, CLOGFMT(AccreditedPaths, "doCommit() false for session[%s]"), sessCtx.sessId.c_str());
			errMsg = buf;
		}
		else if (stampComitted >0)
			state = TianShanIce::stInService;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(buf, sizeof(buf)-2, CLOGFMT(AccreditedPaths, "doCommit() session[%s] threw exception[%s]: %s"), sessCtx.sessId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		errMsg = buf;
	}
	catch(const Ice::Exception& ex)
	{
		snprintf(buf, sizeof(buf)-2, CLOGFMT(AccreditedPaths, "doCommit() session[%s] threw exception[%s]"), sessCtx.sessId.c_str(), ex.ice_name().c_str());
		errMsg = buf;
	}
	catch (...)
	{
		snprintf(buf, sizeof(buf)-2, CLOGFMT(AccreditedPaths, "doCommit() session[%s] threw unknown exception"), sessCtx.sessId.c_str());
		errMsg = buf;
	}

	if (errMsg.empty())
	{
		callback->ice_response();
		return;
	}

	envlog(ZQ::common::Log::L_ERROR, buf);
	callback->ice_exception(::TianShanIce::ServerError("AccreditedPaths", 1032, buf));

}

::TianShanIce::Transport::StorageLinkPrx ADPathTicketImpl::getStorageLink(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathTicket, "ticket[%s] getStorageLink(): %s"), ident.name.c_str(), storageLinkIden.name.c_str());

    RLock sync(*this);
	return IdentityToObj(StorageLink, storageLinkIden);
}

::TianShanIce::Transport::StreamLinkPrx ADPathTicketImpl::getStreamLink(const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathTicket, "ticket[%s] getStreamLink(): %s"), ident.name.c_str(), streamLinkIden.name.c_str());

    RLock sync(*this);
	return IdentityToObj(StreamLink, streamLinkIden);
}

::Ice::Identity ADPathTicketImpl::getIdent(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return ident;
}

::Ice::Int ADPathTicketImpl::getCost(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return costWhenAllocated;
}

::Ice::Int ADPathTicketImpl::getLeaseLeft(const ::Ice::Current&) const
{
    RLock sync(*this);
#pragma message(__MSGLOC__"TODO:how many milliseconds should be used here if no client renew the ticket")
	::Ice::Long nowstamp = now();
	::Ice::Long diff = expiration - nowstamp;
	return (::Ice::Int) (diff <=0 ? 0 : diff);
}

::TianShanIce::ValueMap ADPathTicketImpl::getPrivateData(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return privateData;
}
TianShanIce::SRM::ResourceMap ADPathTicketImpl::getResources(const ::Ice::Current& ) const
{
	RLock sync(*this);
	return resources;
}
::TianShanIce::State ADPathTicketImpl::getState(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return state;
}

void ADPathTicketImpl::renew(::Ice::Int newLeaseTerm, const ::Ice::Current& c)
{
	//envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(PathTicket, "renew() newLeaseTerm = %d"), newLeaseTerm);
	
    WLock sync(*this);

	if (newLeaseTerm < 0 || newLeaseTerm > MAX_TICKET_LEASETERM)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,"ADPathTicket",1041,"PathTicket::renew() out of range of 0-6hour");
	}	
	expiration = now() + newLeaseTerm;
}

::Ice::Byte ADPathTicketImpl::getPenalty(const ::Ice::Current&/* = ::Ice::Current()*/) const 
{
// 	RLock sync(*this);
// 	//privateData
// 	::TianShanIce::ValueMap::const_iterator it = privateData.find("StreamerPenalty_value");
// 	if( it == privateData.end() )
// 	{
// 		return 0;
// 	}
// 	else
// 	{
// 		TianShanIce::Variant var = it->second;
// 		if( var.type == TianShanIce::vtInts  && var.ints.size() > 0 )
// 		{
// 			::Ice::Byte penalty = static_cast<Ice::Byte>(var.ints[0]);
// 			return penalty;
// 		}
// 		else
// 		{
// 			return 0;
// 		}
// 	}
	return 0;
}

void ADPathTicketImpl::setPenalty(::Ice::Byte penalty, const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	TianShanIce::Variant varPenalty;
	varPenalty.type = TianShanIce::vtInts;
	varPenalty.bRange = false;
	varPenalty.ints.clear();
	varPenalty.ints.push_back(penalty);
	WLock sync(*this);	
	privateData["StreamerPenalty_value"] = varPenalty;
}

void ADPathTicketImpl::narrow_async(const ::TianShanIce::Transport::AMD_PathTicket_narrowPtr& amdCB,
									const TianShanIce::SRM::SessionPrx& sess,
									const ::Ice::Current& c)
{
	std::string	sessID = sess->getId();
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(PathTicket, "Session[%s] enter narrow_async()"),sessID.c_str());

	// extend the lease if necessary
	if (getLeaseLeft(c) < DEFAULT_ALLOCATE_TICKET_LEASETERM *10)
		renew(DEFAULT_ALLOCATE_TICKET_LEASETERM *10, c);

	if (TianShanIce::stProvisioned != state)
		amdCB->ice_exception(TianShanIce::InvalidStateOfArt("ADPathTicket",1051,"ADPathTicketImpl::narrow() can not be applied onto non-\"stProvisioned\" ticket"));

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPath,"narrow_async() session[%s] pending threadPool[%d/%d] pendingRequest[%d]"),
		sessID.c_str(),_env._thpool.activeCount(),_env._thpool.size(),_env._thpool.pendingRequestSize());
	(new TicketNarrowCommand(amdCB, _env, *this, sess))->execute();
}

 
void ADPathTicketImpl::destroy(const ::Ice::Current& c)
{
	
	try
	{
		::Ice::Context ctx=c.ctx;
		
		envlog(ZQ::common::Log::L_INFO, OBJLOGFMT(PathTicket, "destroy() called by %s"), ctx["caller"].c_str());
		_env.pathHelperMgr().doFreeResources(this);		
		{
			WLock sync(*this);
			_env._ePathTicket->remove(c.id); if (_gTotalTickets.dec()<0) _gTotalTickets.set(0);
			privateData.clear();
			resources.clear();
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathTicket, "destroy() ticket %s was deleted"),c.id.name.c_str());
		}
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathTicket, "destroy() object already gone, ignore"));
	}
	catch (const ::Ice::NotRegisteredException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathTicket, "destroy() object already gone, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,"ADPathTicket",1061, CLOGFMT(PathTicket, "destroy() caught exception: %s"), ex.ice_name().c_str());
	}
}

void ADPathTicketImpl::destroyEx(const ::Ice::Identity& id)
{
	WLock sync(*this);
	try
	{
		_env._ePathTicket->remove(id); _gTotalTickets.dec();
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathTicket, "destroyEx() object already gone, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog,"ADPathTicket",1071, CLOGFMT(PathTicket, "destroyEx() caught exception: %s"), ex.ice_name().c_str());
	}
}

// -----------------------------
// service AccreditedPathsImpl
// -----------------------------
#define PATH_ALLOC_PREFIX AccreditedPathsImpl.alloc
#define PATH_ALLOC_FIELD(_F)	PD_FIELD(PATH_ALLOC_PREFIX, _F)
AccreditedPathsImpl::AccreditedPathsImpl(PathSvcEnv& env)
: _env(env), _pathHelperMgr(env.pathHelperMgr())
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "add the interface \"%s\" on to the adapter"), SERVICE_NAME_PathManager);
	_localId=_env._communicator->stringToIdentity(SERVICE_NAME_PathManager);
    //_env._adapter->add(this, _localId);
	_env._adapter->ZQADAPTER_ADD(_env._communicator, this, SERVICE_NAME_PathManager);
}

AccreditedPathsImpl::~AccreditedPathsImpl()
{
	try{	
		if(_env._adapter->find(_localId))
			_env._adapter->remove(_localId);
	}
	catch(...){}
}

// impls of Service

// implemenations of PathManager
::TianShanIce::StrValues AccreditedPathsImpl::listSupportedStorageLinkTypes(const ::Ice::Current& c) const
{
	return _pathHelperMgr.listSupportedStorageLinkTypes();
}

::TianShanIce::StrValues AccreditedPathsImpl::listSupportedStreamLinkTypes(const ::Ice::Current& c) const
{
	return _pathHelperMgr.listSupportedStreamLinkTypes();
}

::TianShanIce::PDSchema AccreditedPathsImpl::getStorageLinkSchema(const ::std::string& type, const ::Ice::Current& c) const
{
	::TianShanIce::PDSchema schema;
	if (!_pathHelperMgr.getStorageLinkSchema(type, schema))
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths",1081, "getStorageLinkSchema(%s) no schema definition for such a linktype"), type.c_str());

	return schema;
}

::TianShanIce::PDSchema AccreditedPathsImpl::getStreamLinkSchema(const ::std::string& type, const ::Ice::Current& c) const
{
	::TianShanIce::PDSchema schema;
	if (!_pathHelperMgr.getStreamLinkSchema(type, schema))
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths",1091, "getStreamLinkSchema(%s) no schema definition for such a linktype"), type.c_str());
	
	return schema;
}

// record operations
#define IMPL_ListRecords(_COLLECTION, _DICT) \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_DICT, "list records")); \
	::TianShanIce::Transport::_COLLECTION collection; \
	{ \
		ZQ::common::MutexGuard gd(_env._lock##_DICT); \
		for (TianShanIce::Transport::_DICT::const_iterator it = _env._p##_DICT->begin(); it != _env._p##_DICT->end(); it++) \
			collection.push_back(it->second); \
	} \
	return collection;

#define IMPL_UpdateRecord(_DICT, _KEY, _REC)  { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_DICT, "update record")); \
	ZQ::common::MutexGuard gd(_env._lock##_DICT); \
	Freeze::TransactionHolder txHolder(_env._conn); \
	_env._p##_DICT->put(TianShanIce::Transport::_DICT::value_type(_KEY, _REC)); \
	txHolder.commit(); }

#define IMPL_RemoveRecord(_DICT, _KEY)  { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_DICT, "remove record")); \
	ZQ::common::MutexGuard gd(_env._lock##_DICT); \
	Freeze::TransactionHolder txHolder(_env._conn); \
	_env._p##_DICT->erase(_KEY); \
	txHolder.commit(); }

#define IMPL_CleanLinkByRecord(_LINK, _IDX, _KEY) { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_LINK, "clean up links by " #_IDX)); \
	IdentCollection linkIds = _env._idx##_IDX->find(_KEY); \
	for (IdentCollection::iterator it = linkIds.begin(); it < linkIds.end(); it++) \
	{ \
		try	{ \
			::TianShanIce::Transport::_LINK##Prx link = IdentityToObj(_LINK, *it); \
			if (link) link->destroy(); \
		} catch(...) {} \
	} }

// must make a copy of the cached record then save back to the berkeley db -andy 
#define IMPL_UpdatePrivateData(_DICT, _RECKEY, _RECTYPE, _PDKEY, _PD)  { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_DICT, "update private data")); \
	::TianShanIce::Transport::_DICT::iterator it; \
	::TianShanIce::Transport::_RECTYPE rec; \
	{ \
		ZQ::common::MutexGuard gd(_env._lock##_DICT); \
		it = _env._p##_DICT->find(_RECKEY); \
		if (_env._p##_DICT->end() == it) return false; \
		rec = it->second; \
		rec.privateData[_PDKEY]= _PD; \
	} \
	IMPL_UpdateRecord(_DICT, _RECKEY, rec); }

#define IMPL_GetPrivateData(_DICT, _RECKEY)  { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_DICT, "get private data")); \
	static ::TianShanIce::ValueMap Nil; \
	ZQ::common::MutexGuard gd(_env._lock##_DICT); \
	::TianShanIce::Transport::_DICT::iterator it = _env._p##_DICT->find(_RECKEY); \
	if (_env._p##_DICT->end() == it) return Nil; \
	return it->second.privateData; }

#define IMPL_listLinks(_LINK, _RECTYPE, _RECKEY)  { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_LINK, "list links by " #_RECTYPE)); \
	::TianShanIce::Transport::_LINK##s links; \
	IdentCollection identities; \
	{ \
		ZQ::common::MutexGuard gd(_env._lock##_LINK); \
		identities = _env._idx##_RECTYPE##To##_LINK->find(_RECKEY); \
	} \
	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++) \
	{ \
		::TianShanIce::Transport::_LINK##Prx linkprx = (IdentityToObj(_LINK, *it)); \
		if (linkprx) links.push_back(linkprx); \
	} \
	return links; }

#define CREATE_Link(_LINK, _SRC_DICT, _SRC_ID, _DEST_DICT, _DEST_ID, _TYPE, _PD)  \
	::TianShanIce::Transport::_LINK##Prx linkPrx; _LINK##Impl::Ptr link; { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(_LINK, "link records")); \
	link = new _LINK##Impl(_env); link->revision=0; \
	link->ident.name = IceUtil::generateUUID(); link->ident.category = #_LINK; \
	link->_SRC_ID = _SRC_ID; link->_DEST_ID = _DEST_ID; \
	link->type = _TYPE; link->privateData = _PD; \
	::TianShanIce::Variant val;	val.bRange = false;	val.type   = ::TianShanIce::vtStrings; \
	{  \
		ZQ::common::MutexGuard gd(_env._lock##_SRC_DICT);  \
		TianShanIce::Transport::_SRC_DICT::iterator sit = _env._p##_SRC_DICT->find(_SRC_ID); \
		if (_env._p##_SRC_DICT->end() == sit) \
			ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog,#_LINK,1111, CLOGFMT(_LINK, "No such record in " #_SRC_DICT)); \
		val.strs.push_back(sit->second.type); link->privateData[".$$srcType"] = val; val.strs.clear();\
	} { \
		ZQ::common::MutexGuard gd2(_env._lock##_DEST_DICT); \
		TianShanIce::Transport::_DEST_DICT::iterator dit = _env._p##_DEST_DICT->find(_DEST_ID); \
		if (_env._p##_DEST_DICT->end() == dit) \
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog,#_LINK,1112, CLOGFMT(_LINK, "No such record in " #_DEST_DICT)); \
		val.strs.push_back(dit->second.type); link->privateData[".$$destType"] = val; \
	} \
	_env.pathHelperMgr().validate##_LINK##Config(link->type.c_str(), link->ident.name.c_str(), link->privateData); \
	link->privateData.erase(".$$srcType"); link->privateData.erase(".$$destType"); \
	ZQ::common::MutexGuard gd(_env._lock##_LINK); \
	_env._e##_LINK->add(link, link->ident); \
    linkPrx =IdentityToObj(_LINK, link->ident); }

::TianShanIce::Transport::ServiceGroups AccreditedPathsImpl::listServiceGroups(const ::Ice::Current& c)
{
	IMPL_ListRecords(ServiceGroups, ServiceGroupDict);
}

bool AccreditedPathsImpl::updateServiceGroup(::Ice::Int id, const ::std::string& desc, const ::Ice::Current& c)
{
	TianShanIce::Transport::ServiceGroup sg;

	try {
		ZQ::common::MutexGuard gd(_env._lockServiceGroupDict);
		::TianShanIce::Transport::ServiceGroupDict::iterator it = _env._pServiceGroupDict->find(id);
		if (_env._pServiceGroupDict->end() != it)
			sg = it->second;
	}
	catch(...) {}

	sg.id = id;
	sg.desc = desc;

	IMPL_UpdateRecord(ServiceGroupDict, id, sg);
	return true;
}

bool AccreditedPathsImpl::removeServiceGroup(::Ice::Int id, const ::Ice::Current& c)
{
	IMPL_RemoveRecord(ServiceGroupDict, id);
	IMPL_CleanLinkByRecord(StorageLink, ServiceGroupToStreamLink, id);

	return true;
}

::TianShanIce::Transport::Storages AccreditedPathsImpl::listStorages(const ::Ice::Current& c)
{
	IMPL_ListRecords(Storages, StorageDict);
}

bool AccreditedPathsImpl::updateStorage(const ::std::string& netId, const ::std::string& type, const ::std::string& endpoint, const ::std::string& desc, const ::Ice::Current& c)
{
	TianShanIce::Transport::Storage store;

	try
	{
		ZQ::common::MutexGuard gd(_env._lockStorageDict);
		::TianShanIce::Transport::StorageDict::iterator it = _env._pStorageDict->find(netId);
		if (_env._pStorageDict->end() != it)
			store = it->second;
	}
	catch(...) {}

	store.netId = netId;
	store.type = type;
	store.ifep = endpoint;
	store.desc = desc;

	IMPL_UpdateRecord(StorageDict, netId, store);

	return true;
}

bool AccreditedPathsImpl::removeStorage(const ::std::string& netId, const ::Ice::Current& c)
{
	IMPL_RemoveRecord(StorageDict, netId);
	IMPL_CleanLinkByRecord(StorageLink, StorageToStorageLink, netId);

	return true;
}

bool AccreditedPathsImpl::setStoragePrivateData(const ::std::string& netId, const ::std::string& key, const ::TianShanIce::Variant& val, const ::Ice::Current& c)
{
	IMPL_UpdatePrivateData(StorageDict, netId, Storage, key, val);
	return true;
}

::TianShanIce::ValueMap AccreditedPathsImpl::getStoragePrivateData(const ::std::string& netId, const ::Ice::Current& c)
{
	IMPL_GetPrivateData(StorageDict, netId);
}

::TianShanIce::Transport::Streamers AccreditedPathsImpl::listStreamers(const ::Ice::Current& c)
{
	IMPL_ListRecords(Streamers, StreamerDict);
}

bool AccreditedPathsImpl::updateStreamer(const ::std::string& netId, const ::std::string& type, const ::std::string& endpoint, const ::std::string& desc, const ::Ice::Current& c)
{
	TianShanIce::Transport::Streamer streamer;
	
	try {
		ZQ::common::MutexGuard gd(_env._lockStreamerDict);
		::TianShanIce::Transport::StreamerDict::iterator it = _env._pStreamerDict->find(netId);
		if (_env._pStreamerDict->end() != it)
			streamer = it->second;
	}
	catch(...) {}

	streamer.netId = netId;
	streamer.type = type;
	streamer.ifep = endpoint;
	streamer.desc = desc;

	IMPL_UpdateRecord(StreamerDict, netId, streamer);
	// _env.mStreamerreplicaUpdater->getAllStreamerReplica( );
	return true;
}

bool AccreditedPathsImpl::removeStreamer(const ::std::string& netId, const ::Ice::Current& c)
{
	IMPL_RemoveRecord(StreamerDict, netId);
	IMPL_CleanLinkByRecord(StorageLink, StreamerToStorageLink, netId);
	IMPL_CleanLinkByRecord(StreamLink, StreamerToStreamLink, netId);

	return true;
}

bool AccreditedPathsImpl::setStreamerPrivateData(const ::std::string& netId, const ::std::string& key, const ::TianShanIce::Variant& val, const ::Ice::Current& c)
{
	IMPL_UpdatePrivateData(StreamerDict, netId, Streamer, key, val);
	return true;
}

::TianShanIce::ValueMap AccreditedPathsImpl::getStreamerPrivateData(const ::std::string& netId, const ::Ice::Current& c)
{
	IMPL_GetPrivateData(StreamerDict, netId);
}

::TianShanIce::Transport::StorageLinks AccreditedPathsImpl::listStorageLinksByStorage(const ::std::string& storageId, const ::Ice::Current&c)
{
	IMPL_listLinks(StorageLink, Storage, storageId);
}

::TianShanIce::Transport::StorageLinks AccreditedPathsImpl::listStorageLinksByStreamer(const ::std::string& streamerId, const ::Ice::Current&c)
{
	IMPL_listLinks(StorageLink, Streamer, streamerId);
}

::TianShanIce::Transport::StreamLinks AccreditedPathsImpl::listStreamLinksByStreamer(const ::std::string& streamerId, const ::Ice::Current&c)
{
	IMPL_listLinks(StreamLink, Streamer, streamerId);
}

::TianShanIce::Transport::StreamLinks AccreditedPathsImpl::listStreamLinksByServiceGroup(::Ice::Int servicegroup, const ::Ice::Current&c)
{
	IMPL_listLinks(StreamLink, ServiceGroup, servicegroup);
}

::TianShanIce::Transport::StorageLinkPrx AccreditedPathsImpl::linkStorage(const ::std::string& storageId, const ::std::string& streamerId, const ::std::string& type, const ::TianShanIce::ValueMap& linkPD, const ::Ice::Current& c)
{
	CREATE_Link(StorageLink, StorageDict, storageId, StreamerDict, streamerId, type, linkPD);

	// TODO: bug, this func didn't validate the configuration of link private data
	ZQ::common::MutexGuard gd(_env._lockStreamerDict); 
	if (linkPrx && !_env._cachedIdxStreamerToStorageLink.has(streamerId, link->ident))
		_env._cachedIdxStreamerToStorageLink.insert(streamerId, link->ident);

	return linkPrx;
}

::TianShanIce::Transport::StreamLinkPrx AccreditedPathsImpl::linkStreamer(::Ice::Int servicegroupId, const ::std::string& streamerId, const ::std::string& type, const ::TianShanIce::ValueMap& linkPD, const ::Ice::Current& c)
{
	CREATE_Link(StreamLink, StreamerDict, streamerId, ServiceGroupDict, servicegroupId, type, linkPD);

	// TODO: bug, this func didn't validate the configuration of link private data
	ZQ::common::MutexGuard gd(_env._lockServiceGroupDict); 
	if (linkPrx && !_env._cachedIdxSvcGrpToStreamLink.has(servicegroupId, link->ident))
		_env._cachedIdxSvcGrpToStreamLink.insert(servicegroupId, link->ident);

	return linkPrx;
}

::TianShanIce::Transport::PathTickets AccreditedPathsImpl::listTickets(const ::Ice::Current& c)
{
	::TianShanIce::Transport::PathTickets tickets;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathTicket, "list tickets"));

	// found all the object in the ev
	IdentCollection identities;
	try
	{
		ZQ::common::MutexGuard gd(_env._lockPathTicket);
		::Freeze::EvictorIteratorPtr itptr = _env._ePathTicket->getIterator("", 1);
		if (!itptr)
			return tickets;
		
		while (itptr->hasNext())
			identities.push_back(itptr->next());

		_gTotalTickets.set(identities.size());

	} catch(...) { envlog(ZQ::common::Log::L_WARNING, CLOGFMT(PathTicket, "listTickets() met a DB exception")); }

	// validate the ticket, flush out those expired
	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++)
	{
		try {
			::TianShanIce::Transport::PathTicketPrx ticket = (IdentityToObj(PathTicket, *it));
			if (!ticket)
				continue;

			if (ticket->getLeaseLeft() <= 0)
			{
				Ice::Context ctx;
				ctx["caller"]=__MSGLOC__;
				ticket->destroy(ctx);
				continue;
			}
			
			tickets.push_back(ticket);
		} catch(...) { envlog(ZQ::common::Log::L_WARNING, CLOGFMT(PathTicket, "listTickets() caught an ingorable exception")); }
	}

	return tickets;
}

::std::string AccreditedPathsImpl::getAdminUri(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQTianShan::_IceThrow< TianShanIce::NotImplemented>("AccreditedPaths",1121,__MSGLOC__ "TODO: impl here");
	return "";
}

::TianShanIce::State AccreditedPathsImpl::getState(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQTianShan::_IceThrow< TianShanIce::NotImplemented>("AccreditedPaths",1131,__MSGLOC__ "TODO: impl here");
	return ::TianShanIce::State();
}

::TianShanIce::Transport::PathTickets AccreditedPathsImpl::reservePathsByStorage(const ::TianShanIce::Variant& storages, ::Ice::Int serviceGroup, ::Ice::Long bandwidth, ::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintedLease, const TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePathsByStorage()"));

	if (::TianShanIce::vtStrings != storages.type || storages.strs.empty())
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1141, "AccreditedPathsImpl::reservePathsByStorage() Invalid parameter to find storage");

	if (storages.bRange)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1151, "AccreditedPathsImpl::reservePathsByStorage() does not support a range of storages");

	::TianShanIce::ValueMap context;
	context[PATH_ALLOC_FIELD(storages)] = storages;

	::TianShanIce::Variant var;
	var.bRange =false;

	var.type = ::TianShanIce::vtInts;
	var.ints.push_back(serviceGroup);
	context[PATH_ALLOC_FIELD(svcgrp)] =var;
	var.ints.clear();

	var.type = ::TianShanIce::vtLongs;
	var.lints.push_back(bandwidth);
	context[PathTicketPD_Field(bandwidth)] =var;
	var.lints.clear();

	return reservePathsEx(maxCost, maxTickets, hintedLease, context, sess, context, c);
}

::TianShanIce::Transport::PathTickets AccreditedPathsImpl::reservePathsByStreamer(const ::TianShanIce::Variant& streamers, ::Ice::Int serviceGroup, ::Ice::Long bandwidth, ::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintedLease, const TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePathsByStreamer()"));

	if (::TianShanIce::vtStrings != streamers.type || streamers.strs.empty())
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1161, "AccreditedPathsImpl::reservePathsByStreamer() Invalid parameter to find streamers");

	if (streamers.bRange)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1162, "AccreditedPathsImpl::reservePathsByStreamer() does not support a range of streamers");

	::TianShanIce::ValueMap context;
	context[PATH_ALLOC_FIELD(streamers)] = streamers;

	::TianShanIce::Variant var;
	var.bRange =false;

	var.type = ::TianShanIce::vtInts;
	var.ints.push_back(serviceGroup);
	context[PATH_ALLOC_FIELD(svcgrp)] =var;
	var.ints.clear();

	var.type = ::TianShanIce::vtLongs;
	var.lints.push_back(bandwidth);
	context[PathTicketPD_Field(bandwidth)] =var;
	var.lints.clear();

	return reservePathsEx(maxCost, maxTickets, hintedLease, context, sess, context, c);
}

typedef struct _LinkInfoEx : public LinkInfo
{
	TianShanIce::ValueMap ticketPrivateData;
} LinkInfoEx;

typedef ::std::vector< LinkInfoEx > LinkInfos;

class MyRandom 
{
public:
	ptrdiff_t operator() (ptrdiff_t max) 
	{
		static bool bInit = false;
		if( !bInit )
		{
			srand((uint)time(NULL));
		}
		if( max  > 0 )
			return rand()%max;
		else 
			return 0;
	}
};

void AccreditedPathsImpl::convertSetToRandomVector(std::set<std::string>& sets , 
												   std::vector<std::string>& vectors )
{
	vectors.clear();
	std::set<std::string>::const_iterator  it = sets.begin();
	for ( ; it != sets.end() ; it++ ) 
	{
		vectors.push_back(*it);
	}
	
	MyRandom my_random;
	std::random_shuffle(vectors.begin(),vectors.end(), my_random);
		
}

#define LINK_STATUS_BIT_DISABLED (1<<0)
// #define LINK_STATUS_BIT_STANDBY  (1<<1)

// enh#17727 Weiwoo to specially deal with "$Standby" StorageLinks
typedef struct _StandbyLink
{
	uint32 sessThresholdHigh, sessThresholdLow;
	uint64 kbpsThresholdHigh, kbpsThresholdLow;
	int64 stampCached, stampActive;
} StandbyLink;

typedef std::map < std::string, StandbyLink > StandbyLinkMap;

// enh#17727 Weiwoo to specially deal with "$Standby" StorageLinks
bool AccreditedPathsImpl::validateStandbyStoreLink(::TianShanIce::Transport::StorageLinkExPrx& storeLink, const std::string& linkId, const std::string& clientdesc)
{
	static StandbyLinkMap     _standbyStorageLinks;
	static ZQ::common::Mutex  _lkStandbyStoreLinks;

#define SESSHIGH_INFINITE (999999999) // a really large number

	ZQ::common::MutexGuard g(_lkStandbyStoreLinks);
	int64 stampNow = ZQ::common::now();

	StandbyLinkMap::iterator itSbLink = _standbyStorageLinks.find(linkId);
	if (_standbyStorageLinks.end() == itSbLink || itSbLink->second.stampCached < (stampNow - 5*60*1000))
	{
		// read the attributes of StorageLink into the cache map
		try {
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "%s reading standby StorageLink[%s]" ), clientdesc.c_str(), linkId.c_str());

			StandbyLink sbLink;
			sbLink.stampActive =0;
			sbLink.sessThresholdHigh = SESSHIGH_INFINITE; 
			sbLink.sessThresholdLow  = 0; // initial with a dummy 0
			sbLink.stampCached = stampNow;
			::TianShanIce::ValueMap pd = storeLink->getPrivateData();
			::TianShanIce::ValueMap::iterator itPD = pd.find("SessionThreshold");
			if (pd.end() != itPD)
			{
				if (itPD->second.ints.size() >0)
					sbLink.sessThresholdLow = itPD->second.ints[0];

				if (itPD->second.ints.size() >1)
					sbLink.sessThresholdHigh = itPD->second.ints[1];

				if (sbLink.sessThresholdHigh < sbLink.sessThresholdLow)
				{
					uint32 tmp = sbLink.sessThresholdHigh;
					sbLink.sessThresholdHigh = sbLink.sessThresholdLow;
					sbLink.sessThresholdLow = tmp;
				}

				if (sbLink.sessThresholdLow == sbLink.sessThresholdHigh)
					sbLink.sessThresholdLow = (uint32) (sbLink.sessThresholdHigh *0.7);
			}

			if (_standbyStorageLinks.end() == itSbLink)
				_standbyStorageLinks.insert(StandbyLinkMap::value_type(linkId, sbLink));
			else
			{
				sbLink.stampActive = itSbLink->second.stampActive;
				_standbyStorageLinks[linkId] = sbLink;
			}
		}
		catch(...) { return false; }
	}

	itSbLink = _standbyStorageLinks.find(linkId);
	if (_standbyStorageLinks.end() == itSbLink || itSbLink->second.sessThresholdLow <0)
		return false;

	uint32 totalTickets = _gTotalTickets.get();
	if (itSbLink->second.sessThresholdHigh >=0 && totalTickets >= itSbLink->second.sessThresholdHigh)
	{
		if (itSbLink->second.stampActive <=0)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPaths, "%s start involving standby StorageLink[%s] per active tickets[%d] is greater than threshold-high[%d]" ), 
				clientdesc.c_str(), linkId.c_str(), totalTickets, itSbLink->second.sessThresholdHigh);

			itSbLink->second.stampActive = stampNow;
		}
	}
	else if (totalTickets < itSbLink->second.sessThresholdLow)
	{
		if (itSbLink->second.stampActive >0)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPaths, "%s stop involving standby StorageLink[%s] per active tickets[%d] is less than threshold-low[%d]" ), 
				clientdesc.c_str(), linkId.c_str(), totalTickets, itSbLink->second.sessThresholdLow);

			itSbLink->second.stampActive = 0;
		}
	}

	if (itSbLink->second.stampActive >0)
		return true;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "%s excludes inactive standby StorageLink[%s]: tickets[%d] threshold[%d ~ %d]" ),
		clientdesc.c_str(), linkId.c_str(), totalTickets, itSbLink->second.sessThresholdLow, itSbLink->second.sessThresholdHigh);
	return false;
}

// simple pattern matching utility on wildcasts '?' and '*'
static bool PatternMatch(const char *pattern, const char *str)
{
	const char *s=NULL;
	const char *p=NULL;
	bool bWildcastSubStr=false;
	bool bNextLoop=false;

	do 
	{
		bNextLoop=false;
		for(s=str,p=pattern; *s; ++s,++p)
		{
			switch(*p)
			{
			case '?': // wildcast any char, stay in this loop
				break;

			case '*':
				bWildcastSubStr =true; // wildcast any string
				str=s;
				pattern=p;
				if (!*++pattern) // reached the pattern end with wildcast
					return true;

				bNextLoop=true; // let next loop to handle, primarily thru the 'case default'
				break;

			default:
				if (*s!=*p)
				{
					if(!bWildcastSubStr) 
						return false;

					str++;
					bNextLoop=true;
				}

				break;
			}

			if (bNextLoop) // go next loop
				break;
		}

		if (!bNextLoop)
		{
			if(*p=='*')
				++p;

			return (!*p);
		}

	} while(true);
}

typedef ::std::set< ::std::string > StringSet;
typedef ::TianShanIce::StrValues    StringList;

static bool StringPatternsMatch(const StringList& patterns, const std::string& value)
{
	if (patterns.empty()) // true if not specified any patterns
		return true;

	if (value.empty())
		return false;

	for (StringList::const_iterator itPat = patterns.begin(); itPat < patterns.end(); itPat++)
	{
		if (PatternMatch(itPat->c_str(), value.c_str()))
			return true;
	}

	return false;
}

::TianShanIce::Transport::PathTickets AccreditedPathsImpl::reservePathsEx(
												::Ice::Int maxCost,
												::Ice::Int maxTickets, 
												::Ice::Int hintedLease,
												const ::TianShanIce::ValueMap& contextIn, 
												const TianShanIce::SRM::SessionPrx& sess,
												::TianShanIce::ValueMap& contextOut,
												const ::Ice::Current& c)
{
	contextOut = contextIn;

	::Ice::Int serviceGroup =0;
	::TianShanIce::Variant storages, streamers;
	storages.bRange = streamers.bRange = false;
	storages.type = streamers.type = ::TianShanIce::vtStrings;

	// std::string sessid;

	SessCtx sessCtx;
	StringList Restriction_storeLinkTypes, Restriction_streamLinkTypes;
	std::string RstnList_storeLinkTypes, RstnList_streamLinkTypes;

	StringList Restriction_storeNetIds, Restriction_streamerNetIds;
	std::string RstnList_storeNetIds, RstnList_streamerNetIds;

	TianShanIce::SRM::ResourceAttribute resAttrStorage = TianShanIce::SRM::raNonMandatoryNegotiable;

	TianShanIce::SRM::Resource resServiceGroup;

	if (sess)
	{		
		try
		{			
			sessCtx.sessId = sess->getId();
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePathsEx() session[%s] maxCost=%d; maxTickets=%d; hintedLease=%dms"),
				sessCtx.sessId.c_str(), maxCost, maxTickets, hintedLease);

			// start validate the session context here

			// v.1 validate the session state
			//其实这个地方可以去掉
			//因为session的状态的完全可以在inservice里面判断，而且这个也不是外界可以调用的函数
			sessCtx.state = sess->getState();
			if (::TianShanIce::stProvisioned != sessCtx.state)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPaths, "reservePathsEx() session[%s] state(%d) is not stProvisioned, rejected"), sessCtx.sessId.c_str(), sessCtx.state);
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog,"AccreditedPaths",1171, "AccreditedPaths::reservePaths() session[%s] state(%d) is not stProvisioned, rejected", sessCtx.sessId.c_str(), sessCtx.state);
			}

			sessCtx.strPrx = _env._communicator->proxyToString(sess);
			
			sessCtx.resources = sess->getReources();
			sessCtx.privdata  = sess->getPrivateData();
#ifdef _DEBUG
			::ZQTianShan::dumpResourceMap(sessCtx.resources);
#endif //_DEBUG
			
			// v.2 prepare the servicegroup to allocate for
			if (sessCtx.resources.end() == sessCtx.resources.find(TianShanIce::SRM::rtServiceGroup))
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1172, "AccreditedPaths::reservePaths() session[%s] rtServiceGroup has not been specified", sessCtx.sessId.c_str());
			}
			else
			{
				resServiceGroup = sessCtx.resources[TianShanIce::SRM::rtServiceGroup];
				if (resServiceGroup.resourceData.end() == resServiceGroup.resourceData.find("id") ||
											resServiceGroup.resourceData["id"].bRange || 
											resServiceGroup.resourceData["id"].ints.size() ==0 )
					ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1173, "AccreditedPaths::reservePaths() session[%s] \"id\" of resource \"rtServiceGroup\" is a must, one value can be taken", sessCtx.sessId.c_str());
				else
					serviceGroup = resServiceGroup.resourceData["id"].ints[0];
			}

			// v.2 prepare the specified storages
			
			if (sessCtx.resources.end() != sessCtx.resources.find(TianShanIce::SRM::rtStorage))
			{
				TianShanIce::SRM::Resource& res = sessCtx.resources[TianShanIce::SRM::rtStorage];
				resAttrStorage = res.attr;

				// read the Storage NetIds
				if (res.resourceData.end() == res.resourceData.find("NetworkId"))
				{
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePaths() session[%s] resource rtStorage doesn't has feild \"NetworkId\""), sessCtx.sessId.c_str());
				}
				else
				{
					if (res.resourceData["NetworkId"].bRange)
						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1174, "AccreditedPaths::reservePaths() session[%s] a range of storage are unacceptable, use an enumeration instead", sessCtx.sessId.c_str());
					storages.strs = res.resourceData["NetworkId"].strs;
				}

				// read the StorageLink types
				if (res.resourceData.end() == res.resourceData.find("LinkType"))
				{
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePaths() session[%s] resource rtStorage doesn't has feild \"LinkType\""), sessCtx.sessId.c_str());
				}
				else
				{
					if (res.resourceData["LinkType"].bRange)
						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1174, "AccreditedPaths::reservePaths() session[%s] a range of StorageLink types is illegal, must be an enumeration instead", sessCtx.sessId.c_str());

					Restriction_storeLinkTypes = res.resourceData["LinkType"].strs;
					for (size_t i=0; i < Restriction_storeLinkTypes.size(); i++)
						RstnList_storeLinkTypes += Restriction_storeLinkTypes[i] + ",";
				}
			}
			
			// v.2 prepare the specified streamers
			if (sessCtx.resources.end() != sessCtx.resources.find(TianShanIce::SRM::rtStreamer))
			{
				TianShanIce::SRM::Resource& res = sessCtx.resources[TianShanIce::SRM::rtStreamer];
				if (res.resourceData.end() == res.resourceData.find("NetworkId"))
					envlog(ZQ::common::Log::L_WARNING, 
					CLOGFMT(AccreditedPaths, "reservePaths() session[%s] resource rtStreamer doesn't has attribute \"NetworkId\""), sessCtx.sessId.c_str());
				else
				{
					if (res.resourceData["NetworkId"].bRange)
						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1175, "AccreditedPaths::reservePaths() session[%s] a range of streamer are unacceptable, use a enumeration instead", sessCtx.sessId.c_str());
					streamers.strs = res.resourceData["NetworkId"].strs;
				}

				// read the StreamLink types
				if (res.resourceData.end() == res.resourceData.find("LinkType"))
				{
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePaths() session[%s] resource rtStreamer doesn't has feild \"LinkType\""), sessCtx.sessId.c_str());
				}
				else
				{
					if (res.resourceData["LinkType"].bRange)
						ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1174, "AccreditedPaths::reservePaths() session[%s] a range of StreameLink types is illegal, must be an enumeration instead", sessCtx.sessId.c_str());

					Restriction_streamLinkTypes = res.resourceData["LinkType"].strs;
					for (size_t i=0; i < Restriction_streamLinkTypes.size(); i++)
						RstnList_streamLinkTypes += Restriction_streamLinkTypes[i] + ",";
				}
			}
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			ex.ice_throw();
		}
		catch(const Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPaths, "reservePaths() validate session context caught %s"), ex.ice_name().c_str());
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1176,"reservePaths() validate session context caught %s", ex.ice_name().c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPaths, "reservePaths() validate session context caught exception"));
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1176,"reservePaths() validate session context caught exception");
		}
	}
	else
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(AccreditedPaths, "reservePaths() no session specified, use the allocation context instead"));

		// copy parameters from the context

		if (contextOut.end() != contextOut.find(PATH_ALLOC_FIELD(storages)))
		{
			::TianShanIce::Variant& v = contextOut[PATH_ALLOC_FIELD(storages)];
			if (v.bRange || ::TianShanIce::vtStrings != v.type)
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1177, "AccreditedPaths::reservePaths() illegal parameters on storages: range=%d, type=%d", v.bRange, v.type);
			else storages.strs = v.strs;
			contextOut.erase(PATH_ALLOC_FIELD(storages));
		}
		
		if (contextOut.end() != contextOut.find(PATH_ALLOC_FIELD(streamers)))
		{
			::TianShanIce::Variant& v = contextOut[PATH_ALLOC_FIELD(streamers)];
			if (v.bRange || ::TianShanIce::vtStrings != v.type)
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1178 ,"AccreditedPaths::reservePaths() illegal parameters on streamers: range=%d, type=%d", v.bRange, v.type);
			else streamers.strs = v.strs;
			contextOut.erase(PATH_ALLOC_FIELD(streamers));
		}

		if (contextOut.end() != contextOut.find(PATH_ALLOC_FIELD(svcgrp)))
		{
			::TianShanIce::Variant& v = contextOut[PATH_ALLOC_FIELD(svcgrp)];
			if (v.bRange || ::TianShanIce::vtInts != v.type || v.ints.size() !=1)
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog,"AccreditedPaths",1179, "AccreditedPaths::reservePaths() illegal parameters on servicegroup: range=%d, type=%d, size=%d", v.bRange, v.type, v.ints.size());
			else serviceGroup = v.ints[0];
			contextOut.erase(PATH_ALLOC_FIELD(svcgrp));
		}
	}

	typedef ::std::set< Ice::Int > LongIdSet;

	Restriction_storeNetIds = storages.strs;
	Restriction_streamerNetIds =streamers.strs;

	::TianShanIce::IValues::size_type idx = 0;
	for (idx =0; idx < Restriction_storeNetIds.size(); idx++)
	{
		RstnList_storeNetIds += Restriction_storeNetIds[idx] + ",";
	}

	for (idx =0; idx < Restriction_streamerNetIds.size(); idx++)
	{
		RstnList_streamerNetIds += Restriction_streamerNetIds[idx] + ",";
	}

	envlog(ZQ::common::Log::L_DEBUG,  CLOGFMT(AccreditedPaths, "reservePathsEx() session[%s] start reserving with restrictions: storages[%s] thru types[%s], streamers[%s]  thru types[%s], serviceGroup[%d]"), 
		sessCtx.sessId.c_str(), RstnList_storeNetIds.c_str(), RstnList_storeLinkTypes.c_str(), RstnList_streamerNetIds.c_str(), RstnList_streamLinkTypes.c_str(), serviceGroup);

	typedef ::std::set< Ice::Identity > IdentSet;

	// step 1. find all the StreamLinks by serviceGroup, then filter out those not associated with the streamers
	LinkInfos streamerLinkInfos;
	StringSet associatedStreamerIdSet;

	// step 1.1 find all the streamlink by the service group
	IdentCollection streamLinkIds;
	if (_env._cachedIdxSvcGrpToStreamLink.size()>0)
		streamLinkIds = _env._cachedIdxSvcGrpToStreamLink.find(serviceGroup);
	else streamLinkIds = _env._idxServiceGroupToStreamLink->find(serviceGroup);

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePathsEx() session[%s] found %d streamLinks connected to serviceGroup=%d"), sessCtx.sessId.c_str(), streamLinkIds.size(), serviceGroup);
	if (streamLinkIds.size() <=0)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(AccreditedPaths, 1180, "serviceGroup[%d] with no streamer associated"), serviceGroup);

	::Ice::Long stampLast =0;

	// step 1.2 random_shuffle the stream links out
	if (maxTickets >0)
	{
		MyRandom my_random;
		std::random_shuffle( streamLinkIds.begin(), streamLinkIds.end(), my_random);
		
		// to speed up the selection if there are too many streamlinks to a same servicegroup
		if (_env._streamLinksByMaxTicket >0)
		{
			int takesize = maxTickets * _env._streamLinksByMaxTicket;
			if (streamLinkIds.size() > takesize)
			{
				// to speed up the selection if there are a lot of streamlinks to a same servicegroup
				int oldsize = streamLinkIds.size();
				streamLinkIds.erase(streamLinkIds.begin() +takesize, streamLinkIds.end()); 
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPaths, "session[%s] taking [%d/%d]streamLinks linked to serviceGroup[%d] per maxTickets[%d]x%d"),
					sessCtx.sessId.c_str(), streamLinkIds.size(), oldsize, serviceGroup, maxTickets, _env._streamLinksByMaxTicket);
			}
		}
	}
	
	AliveStreamerCollector::StreamerAttrMap aliveStreamers;
	_env._aliveStreamerCol->getAliveStreamers(aliveStreamers);

	// step 1.3 filter the invalid stream link out
	for (IdentCollection::iterator itLinkId = streamLinkIds.begin();
									itLinkId < streamLinkIds.end(); 
									itLinkId++)
	{
		try
		{
			LinkInfoEx link;
			link.ticketPrivateData = contextOut;
			link.otherIntId	= serviceGroup;
			link.linkIden = *itLinkId;
			link.isStreamLink = true;
			
			// open the link
			::TianShanIce::Transport::StreamLinkExPrx strmlink = IdentityToObj(StreamLinkEx, link.linkIden);
			
			// eliminate those streamers not in selection
			link.linkPrx    = strmlink;
			link.streamerId = strmlink->getStreamerId();
			link.linkType = strmlink->getType();
			if (strmlink->status() & LINK_STATUS_BIT_DISABLED)
			{
				// ignore the streamlink that has been marked to disabled
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths , "session[%s] ignore streamLink[%s] that was disabled" ),
					sessCtx.sessId.c_str() , link.linkIden.name.c_str());
				continue;  
			}

			// step 2.3 eliminate the streamer not allowed by the restrictions
			bool bRestrictionValid_Streamers = StringPatternsMatch(Restriction_streamerNetIds, link.streamerId); // (!Restriction_streamerNetIds.empty() && Restriction_streamerNetIds.end() == Restriction_streamerNetIds.find(link.streamerId))
			bool bRestrictionValid_StreameLinkTypes = StringPatternsMatch(Restriction_streamLinkTypes, link.linkType);

			if (!bRestrictionValid_Streamers || !bRestrictionValid_StreameLinkTypes) // (!Restriction_streamerNetIds.empty() && Restriction_streamerNetIds.end() == Restriction_streamerNetIds.find(link.streamerId))
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths , "session[%s] strmlink[%s:%s] from[%s] skipped due to restrictions: streamers[%s] and/or linktypes[%s]" ),
					sessCtx.sessId.c_str(), link.linkType.c_str(), link.linkIden.name.c_str(), link.streamerId.c_str(), 
					RstnList_streamerNetIds.c_str(), RstnList_streamLinkTypes.c_str());

				continue;
			}
			TianShanIce::Transport::Streamer strmInfo = strmlink->getStreamerInfo();

			// validate if the streamer is in the alive map
			AliveStreamerCollector::StreamerAttrMap::iterator itAlive = aliveStreamers.find(link.streamerId);
			if (aliveStreamers.end() != itAlive) // we only exclude those explict "disabled"
			{
				if (!itAlive->second.bEnable)
				{
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths , "session[%s] execluded streamer[%s] due to it was reported as disabled" ),
						sessCtx.sessId.c_str() , link.streamerId.c_str());
					continue;
				}
			}
			
			// comment it
// 			Ice::Int streamerStatus = 0;
// 			
// 			ZQTianShan::Util::getValueMapDataWithDefault(strmInfo.privateData , REPLICA_STATUS() , 0, streamerStatus );
// 			if( streamerStatus <= 0 )
// 			{
// 				envlog(ZQ::common::Log::L_WARNING,CLOGFMT(AccreditedPaths,
// 					"session[%s] streamer[%s] with endpoint[%s] is disabled, ignore it"),
// 					sessCtx.sessId.c_str() ,
// 					strmInfo.netId.c_str() ,
// 					strmInfo.ifep.c_str() );
// 			}
			
			// evaluate the link cost
			stampLast = ZQTianShan::now();
			link.cost = _pathHelperMgr.doEvaluation(link, sessCtx, link.ticketPrivateData, 0);

			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePaths() strmlink[%s]: stream(%s)->svcgrp(%d), cost=%d with sessionID=%s, latency=%lldms"), 
				link.linkIden.name.c_str(), link.streamerId.c_str(), link.otherIntId, link.cost, sessCtx.sessId.c_str(), ZQTianShan::now() - stampLast);

			// eliminate those link already exceed the allowed cost
			if (link.cost > ::TianShanIce::Transport::MaxCost || maxCost>0 && link.cost > maxCost) 
				continue;

			streamerLinkInfos.push_back(link);
			associatedStreamerIdSet.insert(link.streamerId);
			
		}
		catch (...) {} // always ignore the problem link and continue scanning
	}

	Restriction_streamerNetIds.clear(); // this container is useless now

	::TianShanIce::Transport::PathTickets tickets;

	if (hintedLease<=0)
		hintedLease = DEFAULT_ALLOCATE_TICKET_LEASETERM;
	else if (hintedLease>MAX_ALLOCATE_TICKET_LEASETERM)
		hintedLease = MAX_ALLOCATE_TICKET_LEASETERM;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePaths() session[%s] find the storagelink by the associated streamers"), sessCtx.sessId.c_str());
	
	StringList associatedStreamerIdVector;

	convertSetToRandomVector( associatedStreamerIdSet , associatedStreamerIdVector );
	
	// step 2. find the storagelink by the associated streamers
	for (StringList::iterator itlidSet = associatedStreamerIdVector.begin();
		itlidSet != associatedStreamerIdVector.end(); itlidSet++)
	{
		if (maxTickets>0 && tickets.size() >= (std::size_t) maxTickets)
			break;

		// step 2.1 find all the storage link by the associated streamers
		IdentCollection storageLinkIds;
		try
		{
			if (_env._cachedIdxStreamerToStorageLink.size()>0)
				storageLinkIds = _env._cachedIdxStreamerToStorageLink.find(*itlidSet);
			else storageLinkIds = _env._idxStreamerToStorageLink->find(*itlidSet);
		}
		catch (...) { continue; } // always ignore the problem link and continue scanning
			
		// step 2.2 filter out the invalid storage links
		for (IdentCollection::iterator itLinkId = storageLinkIds.begin();
			itLinkId < storageLinkIds.end(); itLinkId++)
		{
			if (maxTickets>0 && tickets.size() >= (std::size_t) maxTickets)
				break;
			
			try
			{
				LinkInfoEx linkInfo;
				linkInfo.linkIden = *itLinkId;
				linkInfo.isStreamLink = false;
				::TianShanIce::Transport::StorageLinkExPrx storeLink = IdentityToObj(StorageLinkEx, linkInfo.linkIden);
				
				linkInfo.linkPrx    = storeLink;
				linkInfo.streamerId = storeLink->getStreamerId();
				linkInfo.otherNetId = storeLink->getStorageId();
				linkInfo.linkType   = storeLink->getType();
				Ice::Int storeLinkStatus = storeLink->status();

				if (storeLinkStatus & LINK_STATUS_BIT_DISABLED)
				{
					// ignore the streamlink that has been marked to disabled
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths , "session[%s] ignore StorageLink[%s] that was disabled" ),
						sessCtx.sessId.c_str() , linkInfo.linkIden.name.c_str());
					continue;  
				}

				// enh#17727 Weiwoo to specially deal with "$Standby" StorageLinks
				if (std::string::npos != linkInfo.linkType.find("$Standby") && !validateStandbyStoreLink(storeLink, linkInfo.linkIden.name, std::string("sess[")+sessCtx.sessId+"]"))
					continue;

				std::string linkDesc = linkInfo.linkType +"/" + linkInfo.linkIden.name + ":" + linkInfo.otherNetId + "->" + linkInfo.streamerId;
				
				// step 2.3 eliminate the unassociated storage link by streamer
				if (associatedStreamerIdSet.end() == associatedStreamerIdSet.find(linkInfo.streamerId))
					continue;
				
				// step 2.3 eliminate the storages not in selection if there is any
				bool bRestrictionValid_Storages = StringPatternsMatch(Restriction_storeNetIds, linkInfo.otherNetId);
				bool bRestrictionValid_StoreLinkTypes = StringPatternsMatch(Restriction_storeLinkTypes, linkInfo.linkType);
				
				if (Restriction_storeNetIds.empty() && Restriction_storeLinkTypes.empty())
				{
					if (TianShanIce::SRM::raNonMandatoryNonNegotiable == resAttrStorage)
					{
						envlog(ZQ::common::Log::L_WARNING, CLOGFMT( AccreditedPaths , "session[%s] no acceptable range of storages or storageLink types specified although it is raNonMandatoryNonNegotiable, treat it as raNonMandatoryNegotiable instead"), sessCtx.sessId.c_str());
						resAttrStorage = TianShanIce::SRM::raNonMandatoryNegotiable;
					}
					else if (TianShanIce::SRM::raMandatoryNonNegotiable == resAttrStorage)
					{
						envlog(ZQ::common::Log::L_WARNING, CLOGFMT( AccreditedPaths , "session[%s] no acceptable range of storages or storageLink types specified although it is raMandatoryNonNegotiable, treat it as raMandatoryNegotiable instead"), sessCtx.sessId.c_str());
						resAttrStorage = TianShanIce::SRM::raMandatoryNegotiable;
					}
				}
				
				switch (resAttrStorage)
				{
				case TianShanIce::SRM::raMandatoryNonNegotiable:
					if (!bRestrictionValid_Storages || !bRestrictionValid_StoreLinkTypes)
					{
						envlog(ZQ::common::Log::L_DEBUG, CLOGFMT( AccreditedPaths , "session[%s] ignore the link[%s] that is not in pre-set storageId[%s] and linktype[%s] per raMandatoryNonNegotiable"),
							sessCtx.sessId.c_str() , linkDesc.c_str(), RstnList_storeNetIds.c_str(), RstnList_storeLinkTypes.c_str());
						continue;
					}
					break;

				case TianShanIce::SRM::raMandatoryNegotiable:
					if (!bRestrictionValid_Storages && !bRestrictionValid_StoreLinkTypes) // && ( && (!Restriction_streamLinkTypes.empty() && !inPresetStreamLinkTypes))
					{
						envlog(ZQ::common::Log::L_DEBUG, CLOGFMT( AccreditedPaths , "session[%s] ignore the link[%s] that is not in pre-set storageId[%s] or linktype[%s] per raMandatoryNegotiable"),
							sessCtx.sessId.c_str() , linkDesc.c_str(), RstnList_storeNetIds.c_str(), RstnList_storeLinkTypes.c_str());
						continue;
					}
					
					break;

				case TianShanIce::SRM::raNonMandatoryNonNegotiable:
					if ( (!Restriction_storeNetIds.empty() && !bRestrictionValid_Storages ) && (!Restriction_storeLinkTypes.empty() && !bRestrictionValid_StoreLinkTypes))
					{
						envlog(ZQ::common::Log::L_DEBUG, CLOGFMT( AccreditedPaths , "session[%s] ignore the link[%s] that is not in pre-set either storageId[%s] or linktype[%s] per raNonMandatoryNonNegotiable"),
							sessCtx.sessId.c_str() , linkDesc.c_str(), RstnList_storeNetIds.c_str(), RstnList_storeLinkTypes.c_str());
						continue;
					}
					
					break;

				case TianShanIce::SRM::raNonMandatoryNegotiable:
					// always accept in this case
					break;
				}
				
				// step 3. connect the two links, allocate the ticketing
				stampLast = now();
				::Ice::Long exp = stampLast + hintedLease;
				for (LinkInfos::iterator itStreamLink = streamerLinkInfos.begin();
					itStreamLink != streamerLinkInfos.end(); itStreamLink ++)
				{
					if (maxTickets>0 && tickets.size() >= (std::size_t) maxTickets)
						break;

					try 
					{
						if (linkInfo.streamerId != itStreamLink->streamerId)
							continue;
						
						// step 3.1 evaluate the storage link cost
						linkInfo.ticketPrivateData = itStreamLink->ticketPrivateData; // make a copy from the previous eval
						linkInfo.rcMap= itStreamLink->rcMap;
						stampLast = now();
						linkInfo.cost = _pathHelperMgr.doEvaluation(linkInfo, sessCtx, linkInfo.ticketPrivateData, itStreamLink->cost);

						envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePaths() session[%s] storelink[%s]: store(%s)->streamer(%s), cost=%d, dur=%lld"), 
							sessCtx.sessId.c_str(), linkInfo.linkIden.name.c_str(), linkInfo.otherNetId.c_str(), linkInfo.streamerId.c_str(), linkInfo.cost, now() -stampLast);

						// step 3.2 eliminate those links already exceed the expected cost
						if (linkInfo.cost > ::TianShanIce::Transport::MaxCost || maxCost>0 && linkInfo.cost > maxCost) 
							continue;
						
						// step 3.3 okay, we found a path met the requirement, create a ticket on the path
						envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "reservePaths() session[%s] found a valid path, create a ticket on it"), sessCtx.sessId.c_str()); 

						ADPathTicketImpl::Ptr pTicket = new ADPathTicketImpl(_env);
						pTicket->ident.name = IceUtil::generateUUID(); pTicket->ident.category = "PathTicket"; // ADPathTicketImpl::ice_staticId();
						pTicket->state	 = TianShanIce::stProvisioned;
						pTicket->storageLinkIden = linkInfo.linkIden;
						pTicket->streamLinkIden  = itStreamLink->linkIden;
						
						pTicket->privateData     = sessCtx.privdata;

						//add session ident to ticket's private data
						ZQTianShan::Util::updateValueMapData(pTicket->privateData,WEIWOO_SESS_IDENT_NAME,sessCtx.sessId);
						ZQTianShan::Util::updateValueMapData(pTicket->privateData,WEIWOO_SESS_IDENT_CATE,"Session");

						{
							TianShanIce::ValueMap::const_iterator it=linkInfo.ticketPrivateData.begin();
							for( ; it != linkInfo.ticketPrivateData.end() ; it++ )
							{
								pTicket->privateData[it->first]=it->second;
							}
						}
					
						pTicket->resources		 = sessCtx.resources;
						{
							TianShanIce::SRM::ResourceMap::const_iterator it=linkInfo.rcMap.begin();
							for( ; it != linkInfo.rcMap.end() ; it++ )
							{
								//pTicket->resources.insert(*it);
								pTicket->resources[it->first]=it->second;
							}

							// TianShanIce::SRM::rtStorage
							TianShanIce::SRM::Resource rc;
							TianShanIce::Variant varStorageID;
							varStorageID.type = TianShanIce::vtStrings;
							varStorageID.bRange = false;
							varStorageID.strs.clear();
							varStorageID.strs.push_back(linkInfo.otherNetId);
							rc.resourceData["NetworkId"]=varStorageID;
							rc.status = TianShanIce::SRM::rsAssigned;
							rc.attr = TianShanIce::SRM::raMandatoryNegotiable;
							pTicket->resources[TianShanIce::SRM::rtStorage]=rc;

							// TianShanIce::SRM::rtStreamer
							TianShanIce::Variant varStreamerID;
							varStreamerID.type = TianShanIce::vtStrings;
							varStreamerID.bRange = false;
							varStreamerID.strs.clear();
							varStreamerID.strs.push_back(linkInfo.streamerId);
							rc.resourceData["NetworkId"] = varStreamerID;
							rc.status = TianShanIce::SRM::rsAssigned;
							rc.attr = TianShanIce::SRM::raMandatoryNegotiable;

							if(pTicket->resources.find(TianShanIce::SRM::rtStreamer) != pTicket->resources.end())
							{
								pTicket->resources[TianShanIce::SRM::rtStreamer].resourceData["NetworkId"] = varStreamerID;
							}
							else
								pTicket->resources[TianShanIce::SRM::rtStreamer]=rc;	

							if (pTicket->resources.find(TianShanIce::SRM::rtServiceGroup) == pTicket->resources.end())
								pTicket->resources[TianShanIce::SRM::rtServiceGroup] = resServiceGroup;	
						}

						pTicket->stampAllocated	= stampLast;
						pTicket->stampComitted  = 0;
						pTicket->expiration		= exp;
						pTicket->costWhenAllocated = linkInfo.cost;

						// add the object into the ev
						stampLast = now();
						_env._ePathTicket->add(pTicket, pTicket->ident); _gTotalTickets.inc();
						::TianShanIce::Transport::PathTicketPrx ticket = IdentityToObj(PathTicket, pTicket->ident);
						// add the new ticket into the returning array
						if (!ticket)
							continue;
						envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(AccreditedPaths,"session[%s] create a path ticket %s with cost=%d, add to DB took %lldmsec"),
													sessCtx.sessId.c_str(), pTicket->ident.name.c_str(), pTicket->costWhenAllocated, now() -stampLast);

						tickets.push_back(ticket);
					}
					catch(const Ice::Exception& ex) 
					{
						envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(AccreditedPaths,"session[%s] create a path ticket failed with Ice Exception [%s]"),
													sessCtx.sessId.c_str(), ex.ice_name().c_str());
					}
					catch(...) 
					{
						envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(AccreditedPaths,"session[%s] create a path ticket failed with unknown exception"),	sessCtx.sessId.c_str());
					} // always ignore the problem link and continue scanning
				}
			}
			catch(...) {} // always ignore the problem link and continue scanning
		}
	}

	if (tickets.size() <=0)
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(AccreditedPaths, "reservePathsEx() session[%s] no appreciated ticket reserved"), sessCtx.sessId.c_str()); 
	else
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPaths, "reservePathsEx() session[%s] %d tickets reserved"), sessCtx.sessId.c_str(),tickets.size()); 

	return tickets;
}

::TianShanIce::Transport::PathTickets AccreditedPathsImpl::reservePaths(
											::Ice::Int maxCost, ::Ice::Int maxTickets, 
											::Ice::Int hintedLease, const TianShanIce::SRM::SessionPrx& sess, 
											const ::Ice::Current& c)
{
	::TianShanIce::ValueMap context;
	return reservePathsEx(maxCost, maxTickets, hintedLease, context, sess, context, c);
}
void AccreditedPathsImpl::reservePaths_async(const ::TianShanIce::Transport::AMD_PathManager_reservePathsPtr& rPathasyncPtr,
											 ::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintedLease, 
											 const TianShanIce::SRM::SessionPrx& sess,
											 const ::Ice::Current& c)
{	
	envlog(ZQ::common::Log::L_DEBUG,
		CLOGFMT(AccreditedPath,"reservePaths_async() pending threadPool[%d/%d] pendingRequest[%d]"),
		_env._thpool.activeCount(),_env._thpool.size(),_env._thpool.pendingRequestSize());
	(new ZQTianShan::AccreditedPath::ReserveTicketCommand(rPathasyncPtr,*this,maxCost,maxTickets,hintedLease,sess,c))->execute();
}

struct LessStreamer
{
     bool operator()(const ::TianShanIce::Transport::Storage& A, const ::TianShanIce::Transport::Storage& B)
     {
          return A.netId.compare(B.netId) <0;
     }
};

void AccreditedPathsImpl::dumpDot(const ::TianShanIce::ValueMap& context, ::std::string& storageGraph, ::std::string& streamGraph, ::std::string& sgGraph, ::std::string& storageLinksGraph, ::std::string& streamLinksGraph, const ::Ice::Current& c)
{
	storageGraph = streamGraph = sgGraph = storageLinksGraph = streamLinksGraph = "";
	typedef ::std::set< Ice::Int > IntIdSet;
	StringSet storeNetIdSet, stmrNetIdSet;
	IntIdSet sgIdSet;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "dumpDot() reading context"));

	::TianShanIce::ValueMap::const_iterator it = context.find(PATH_ALLOC_FIELD(storages));
	if (context.end() != it)
	{
		if (it->second.bRange || ::TianShanIce::vtStrings != it->second.type)
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths",1181, "dumpDot() illegal parameters on storages: range=%d, type=%d"), it->second.bRange, it->second.type);

		for (size_t i =0; i< it->second.strs.size(); i++)
			storeNetIdSet.insert(it->second.strs[i]);
	}
		
	it = context.find(PATH_ALLOC_FIELD(streamers)); 
	if (context.end() != it)
	{
		if (it->second.bRange || ::TianShanIce::vtStrings != it->second.type)
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths",1182, "dumpDot() illegal parameters on streamers: range=%d, type=%d"), it->second.bRange, it->second.type);

		for (size_t i =0; i< it->second.strs.size(); i++)
			stmrNetIdSet.insert(it->second.strs[i]);
	}

	it = context.find(PATH_ALLOC_FIELD(svcgrp));
	if (context.end() != it)
	{
		if (it->second.bRange || ::TianShanIce::vtInts != it->second.type)
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths",1183, "dumpDot() illegal parameters on servicegroup: range=%d, type=%d, size=%d"), it->second.bRange, it->second.type, it->second.ints.size());
		
		for (size_t i =0; i< it->second.ints.size(); i++)
			sgIdSet.insert(it->second.ints[i]);
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "dumpDot() list all entities"));

	::TianShanIce::Transport::Storages		storages	= listStorages(c);
	::TianShanIce::Transport::Streamers		streamers	= listStreamers(c);
	::TianShanIce::Transport::ServiceGroups sgs			= listServiceGroups(c);

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPaths, "dumpDot() filter entities based on context"));

#define FILTER_BY_SET(_FILTER, _TYPE, _COLLECTION, _FIELD) \
	if (_FILTER.size() >0) \
	{ ::TianShanIce::Transport::_TYPE tmp; \
		for (::TianShanIce::Transport::_TYPE::iterator it = _COLLECTION.begin(); _FILTER.size() >0 && it < _COLLECTION.end(); it++) \
		   if (_FILTER.end() != _FILTER.find(it->_FIELD))	{ tmp.push_back(*it); _FILTER.erase(it->_FIELD); } \
		_COLLECTION = tmp;	}

	FILTER_BY_SET(storeNetIdSet,    Storages, storages, netId);
	FILTER_BY_SET(stmrNetIdSet,		Streamers, streamers, netId);
	FILTER_BY_SET(sgIdSet,			ServiceGroups, sgs, id);

	// step 1. generate the dot definition for Storage records
	for (::TianShanIce::Transport::Storages::iterator stit = storages.begin(); stit < storages.end(); stit++)
	{
        storageGraph += std::string("\"") + stit->netId + "\"[label=\"" + stit->netId + "\"];\n";
		// ::std::string type;
		// ::std::string desc;
		// ::std::string ifep;
	}

	// step 2. generate the dot definition for ServiceGroup records
	for (::TianShanIce::Transport::ServiceGroups::iterator sgit = sgs.begin(); sgit < sgs.end(); sgit++)
	{
		char buf[64];
		sgGraph += std::string("sg") + itoa(sgit->id, buf, 10) + ";\n";
	}

	// step 3. generate the dot definition for Streamer records
	std::sort(storages.begin(), storages.end(), LessStreamer());
	std::string latestSS;
	for (::TianShanIce::Transport::Streamers::iterator smit = streamers.begin(); smit < streamers.end(); smit++)
	{
		// streamer netId = "<StreamService netId>/<deviceId>", chop it
		size_t pos = smit->netId.find_first_of("/");
		std::string crntSS = (pos >0) ? smit->netId.substr(0, pos) : smit->netId;
		std::string stmr = (pos >0) ? smit->netId.substr(pos+1) : "";

		if (0 != crntSS.compare(latestSS))
		{
			if (!latestSS.empty())
				streamGraph = streamGraph.substr(0, streamGraph.length() -1) + "}}\"];\n";
            streamGraph += std::string("\"") + crntSS + "\"[shape=record,label=\"{<ss>" + crntSS + "|{";
		}

		streamGraph += "<" + stmr + ">" + stmr + "|";
		latestSS = crntSS;

		// step 3.2 list the storagelinks by this streamer
		::TianShanIce::Transport::StorageLinks stlinks = listStorageLinksByStreamer(smit->netId, c);
		for (::TianShanIce::Transport::StorageLinks::iterator itStLnk = stlinks.begin(); itStLnk < stlinks.end(); itStLnk ++)
		{
			try {
				::TianShanIce::Transport::Storage store = ::TianShanIce::Transport::StorageLinkExPrx::checkedCast(*itStLnk)->getStorageInfo();
                storageLinksGraph += std::string("\"") + store.netId + "\" -> \"" + crntSS + "\":\"" + stmr + "\";\n";
			}
			catch(...) {}
		}

		// step 3.3 list the streamlinks by this streamer
		::TianShanIce::Transport::StreamLinks smlinks = listStreamLinksByStreamer(smit->netId, c);
		for (::TianShanIce::Transport::StreamLinks::iterator itSmLnk = smlinks.begin(); itSmLnk < smlinks.end(); itSmLnk ++)
		{
			try {
				::Ice::Int svcgrpId = ::TianShanIce::Transport::StreamLinkExPrx::checkedCast(*itSmLnk)->getServiceGroupId();
				std::ostringstream oss;
				oss << svcgrpId;
                streamLinksGraph += std::string("\"") + crntSS + "\":\"" + stmr + "\" -> " + std::string("sg") + oss.str() + ";\n";
			}
			catch(...) {}
		}
	}

	if (!latestSS.empty())
		streamGraph = streamGraph.substr(0, streamGraph.length() -1) + "}}\"];\n";
}

void AccreditedPathsImpl::dumpToXmlFile(const ::std::string& filename, const ::Ice::Current& c)
{
	std::string tempfile = filename + "~";
	::std::ofstream file(tempfile.c_str());
	if (!file.is_open())
		ZQTianShan::_IceThrow<TianShanIce::NotSupported> (_env._log, EXPFMT(AccreditedPaths, 1191, "dumpToXmlFile failed to write temp file %s"), tempfile.c_str());

	file << "<TianShanTransport>\n";

	::std::stringstream storelinks, streamlinks;
	storelinks << "<StorageLinks>\n";
	streamlinks << "<StreamLinks>\n";

	::TianShanIce::Transport::Storages		storages;
	::TianShanIce::Transport::Streamers		streamers;
	::TianShanIce::Transport::ServiceGroups sergroups;
	try
	{	
		storages = listStorages(c);
		streamers = listStreamers(c);
		sergroups = listServiceGroups(c);
	}
	catch(::TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a TianShanIce::BaseException name[%s] message[%s] when get list data"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a Ice::exception name[%s] when get list data"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a unknown exception when get list data about listStorages , listStreamers and listServiceGroups"));
	}

	//storage records
	file << "<Storages>\n";
	for (::TianShanIce::Transport::Storages::iterator stoit = storages.begin(); stoit < storages.end(); stoit++)
	{
		try
		{		
			::TianShanIce::ValueMap stomap = getStoragePrivateData(stoit->netId,c);
			file << "<Storage netid=\"" + stoit->netId + "\" type=\"" + stoit->type + "\" desc=\"" + stoit->desc + "\" ifep=\"" + stoit->ifep + "\" ";
			file <<  (stomap.size() > 0 ? ">" : "/>\n");		
			
			if(stomap.size() > 0)
				file << "<PrivateData>" << privDataToXml(stomap) << "</PrivateData></Storage>\n";
		}
		catch(::TianShanIce::BaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a TianShanIce::BaseException name[%s] message[%s] when dump storage netID[%s]"),
				ex.ice_name().c_str(), ex.message.c_str(), stoit->netId.c_str());
		}
		catch(Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a Ice::exception name[%s] when dump storage netID[%s]"),
				ex.ice_name().c_str(), stoit->netId.c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a unknown exception when dump storage netID[%s]"), 
				stoit->netId.c_str());
		}
	}
	file << "</Storages>\n";
	
	//servicegroup records
	file << "<ServiceGroups>\n";
	char chId[50] = {0};
	for(::TianShanIce::Transport::ServiceGroups::iterator sergit = sergroups.begin(); sergit < sergroups.end(); sergit++)
	{
		memset(chId,0,sizeof(chId));
		sprintf(chId,"%d",sergit->id);
		file << "<Group id=\"" << chId << "\" type=\"" + sergit->type + "\" desc=\"" + sergit->desc + "\" />\n";
	}
	file << "</ServiceGroups>\n";

	//stream  streamlink storagelink records
	file << "<Streamers>\n";
	for (::TianShanIce::Transport::Streamers::iterator smit = streamers.begin(); smit < streamers.end(); smit++)
	{
		//privateData if hase
		::TianShanIce::ValueMap streammap = getStreamerPrivateData(smit->netId,c);
		file << "<Streamer netid=\"" + smit->netId + "\" type=\"" + smit->type + "\" desc=\"" + smit->desc + "\" ifep=\"" + smit->ifep + "\" ";
		file <<  (streammap.size() > 0 ? ">" : "/>\n");

		if(streammap.size() > 0)
			file << "<PrivateData>" << privDataToXml(streammap) << "</PrivateData></Streamer>\n";
		
		//list the storagelinks by this streamer
		std::string strLinkID;
		std::string strProxy;
		::TianShanIce::Transport::StorageLinks stlinks = listStorageLinksByStreamer(smit->netId, c);
		for (::TianShanIce::Transport::StorageLinks::iterator itStLnk = stlinks.begin(); itStLnk < stlinks.end(); itStLnk ++)
		{
			strLinkID = "";
			strProxy = "";
			try {
				strProxy = _env._communicator->proxyToString(*itStLnk);
				strLinkID = _env._communicator->identityToString((*itStLnk)->getIdent());
				::TianShanIce::Transport::StorageLinkExPrx storeLinkExPrx = ::TianShanIce::Transport::StorageLinkExPrx::checkedCast(*itStLnk);
				::TianShanIce::Transport::Storage store = storeLinkExPrx->getStorageInfo();
				::TianShanIce::ValueMap stolinmap = ::TianShanIce::Transport::StorageLinkExPrx::checkedCast(*itStLnk)->getPrivateData();				

				int status = storeLinkExPrx->status();
				
				storelinks << "<Link"
						   << " status=\"" << status << "\""
                           << " linkId=\"" << strLinkID << "\""
                           << " storageId=\"" << store.netId << "\" streamerId=\"" << smit->netId << "\" type=\""
				<< ::TianShanIce::Transport::StorageLinkExPrx::checkedCast(*itStLnk)->getType() << "\" " << (stolinmap.size() > 0 ? ">" : "/>\n");

				//private data
				if(stolinmap.size() > 0)
					storelinks << "<PrivateData>" << privDataToXml(stolinmap) << "</PrivateData>\n" << "</Link>\n";
			}
			catch(::TianShanIce::BaseException& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a TianShanIce::BaseException name[%s] message[%s] when dump storagelinks proxy string[%s]"),
					ex.ice_name().c_str(), ex.message.c_str(), strProxy.c_str());
			}
			catch(Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a Ice::exception name[%s] when dump storagelinks proxy string[%s]"),
					ex.ice_name().c_str(),strProxy.c_str());
			}
			catch(...)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a unknown exception when dump storagelinks proxy string[%s]"), 
					strProxy.c_str());
			}
		}

		//list the streamlinks by this streamer
		::TianShanIce::Transport::StreamLinks smlinks = listStreamLinksByStreamer(smit->netId, c);
		for (::TianShanIce::Transport::StreamLinks::iterator itSmLnk = smlinks.begin(); itSmLnk < smlinks.end(); itSmLnk ++)
		{
			strLinkID = "";
			strProxy = "";
			memset(chId,0,sizeof(chId));
			try {
				strProxy = _env._communicator->proxyToString(*itSmLnk);
				strLinkID = _env._communicator->identityToString((*itSmLnk)->getIdent());
				::TianShanIce::Transport::StreamLinkExPrx streamLinkExPrx = ::TianShanIce::Transport::StreamLinkExPrx::checkedCast(*itSmLnk);
				::Ice::Int svcgrpId = streamLinkExPrx->getServiceGroupId();
				sprintf(chId,"%d",svcgrpId);
				::TianShanIce::ValueMap streamlinmap = ::TianShanIce::Transport::StreamLinkExPrx::checkedCast(*itSmLnk)->getPrivateData();
				
				int status = streamLinkExPrx->status();

				streamlinks << "<Link"
							<< " status=\"" << status << "\""
                            << " linkId=\"" << strLinkID << "\""
                            << " streamerId=\"" << smit->netId << "\" servicegroupId=\"" << chId << "\" type=\"" << ::TianShanIce::Transport::StreamLinkExPrx::checkedCast(*itSmLnk)->getType() << "\" "
					<< (streamlinmap.size() > 0 ? ">" : "/>\n");
				
				//private data
				if(streamlinmap.size() > 0)
					streamlinks << "<PrivateData>" << privDataToXml(streamlinmap) << "</PrivateData>\n" << "</Link>\n";
			}
			catch(::TianShanIce::BaseException& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a TianShanIce::BaseException name[%s] message[%s] when dump streamlinks proxy string[%s]"),
					ex.ice_name().c_str(), ex.message.c_str(), strProxy.c_str());
			}
			catch (Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a Ice::exception name[%s] when dump streamlinks proxy string[%s]"),
					ex.ice_name().c_str(), strProxy.c_str());
			}
			catch(...) 
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (_env._log, EXPFMT(AccreditedPaths, 1190, "dumpToXmlFile catch a unknown exception when dump streamlinks proxy string[%s]"),
					strProxy.c_str());
			}
		}		
	}
	file << "</Streamers>\n";
	storelinks << "</StorageLinks>\n";
	streamlinks << "</StreamLinks>\n";

	file << storelinks.str() << streamlinks.str() << "</TianShanTransport>";
	storelinks.str("");
	streamlinks.str("");
	file.flush();
	file.close();

	::unlink(filename.c_str());
	::rename(tempfile.c_str(), filename.c_str());
	::unlink(tempfile.c_str());
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "dumpXml() has get the database record and transform to xml string"));
}

::std::string AccreditedPathsImpl::dumpXml(const ::Ice::Current& c)
{
	::std::string filename = _env._dbRuntimePath.empty() ? _env._dbPath : _env._dbRuntimePath;
	filename += "ts.xml";

	dumpToXmlFile(filename, c);

	::std::ifstream file(filename.c_str());
	if (!file.is_open())
		return "";

	std::ostringstream strXml;
	// Read to EOF
	for (bool done = file.eof(); !done; done = file.eof())
	{
		char szBuffer[8192];
		// Read data from the input file; store the bytes read
		file.read(szBuffer, sizeof(szBuffer));
		strXml << std::string(szBuffer, file.gcount());
	}

	return strXml.str();
}

struct TransportData
{ // define the internal data type
    // servicegroup
    struct ServiceGroupData
    {
        std::string id;
        std::string desc;
        std::string type;
    };
    typedef std::list<ServiceGroupData> ServiceGroupDataList;
    ServiceGroupDataList servicegroupList;

    // storage
    typedef TianShanIce::Transport::Storage StorageData;
    typedef std::list<StorageData> StorageDataList;
    StorageDataList storageList;

    // streamer
    typedef TianShanIce::Transport::Streamer StreamerData;
    typedef std::list<StreamerData> StreamerDataList;
    StreamerDataList streamerList;

    // storagelink
    struct StorageLinkData
    {
		std::string status;
        std::string ident;

        std::string	storageId;
        std::string streamerId;

        std::string type; // transport type, i.e. ftp protocol
        TianShanIce::ValueMap privateData;
    };
    typedef std::list<StorageLinkData> StorageLinkDataList;
    StorageLinkDataList storagelinkList;

    // streamlink
    struct StreamLinkData
    {
		std::string status;
        std::string ident;

        std::string streamerId;
        std::string servicegroupId;

        std::string type; // transport type, i.e. IP or DVBC connection
        TianShanIce::ValueMap privateData;
    };
    typedef std::list<StreamLinkData> StreamLinkDataList;
    StreamLinkDataList streamlinkList;

    std::string operation; // operation

    void clear()
    {
        servicegroupList.clear();
        storageList.clear();
        streamerList.clear();
        storagelinkList.clear();
        streamlinkList.clear();
        operation.clear();
    }
};

static ::TianShanIce::ValueMap convertXmlToPrivData(ZQ::common::Log& log, ZQ::common::XMLPreferenceEx* pPre);
static bool parseTransportXml(ZQ::common::Log& log, const std::string& xmlData, TransportData& td, std::string& err)
{
    err.clear();
    td.clear();
	//parse xml file
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	try
	{
		if(!xmlDoc.read(xmlData.c_str(), xmlData.size(), 1))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "parseTransportXml() read xml data failed"));
            err = "Can't parse the xml data";
			return false;
		}
	}
	catch(...)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "parseTransportXml() catch a exception when read xml data"));
        err = "Not a valid xml file";
		return false;
	}

	ZQ::common::XMLPreferenceEx* preRoot = (ZQ::common::XMLPreferenceEx*)xmlDoc.getRootPreference();
	if(!preRoot)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "parseTransportXml() xml parse error,getRootPreference function failed"));
        err = "Bad xml format";
		return false;
	}

    // get the operation
    ZQ::common::XMLPreferenceEx* pOpNode = preRoot->firstChild("Operation");
    if(pOpNode)
    {
        char opBuf[128];
        memset(opBuf, 0, sizeof(opBuf));
        pOpNode->getPreferenceText(opBuf, sizeof(opBuf) - 1);

        td.operation = opBuf;
        ZQ::common::stringHelper::TrimExtra(td.operation);
        pOpNode->free();
    }

    // gather the data
	char value[1024] = {0}; // buf

    //parse storages
    td.storageList.clear();

	ZQ::common::XMLPreferenceEx* pStoPre = (ZQ::common::XMLPreferenceEx*)preRoot->firstChild("Storages");
	if(pStoPre)
	{
        for(ZQ::common::XMLPreferenceEx* pStorNode=(ZQ::common::XMLPreferenceEx*)pStoPre->firstChild("Storage"); pStorNode; pStorNode=(ZQ::common::XMLPreferenceEx*)pStoPre->nextChild())
        {
            TransportData::StorageData data;
            if(pStorNode->get("netid",value,"",sizeof(value)))
            {
                data.netId = value;
            }
            if(pStorNode->get("type",value,"",sizeof(value)))
            {
                data.type = value;
            }
            if(pStorNode->get("ifep",value,"",sizeof(value)))
            {
                data.ifep = value;
            }
            if(pStorNode->get("desc",value,"",sizeof(value)))
            {
                data.desc = value;
            }

            ZQ::common::XMLPreferenceEx* pPriv = (ZQ::common::XMLPreferenceEx*)pStorNode->firstChild("PrivateData");
            if(pPriv)//has private data
            {
                data.privateData = convertXmlToPrivData(log, pPriv);

                pPriv->free();
            }
            td.storageList.push_back(data);

            pStorNode->free();
        }
        pStoPre->free();
    }
	//parse streamers
    td.streamerList.clear();

	ZQ::common::XMLPreferenceEx* pStreamPre = (ZQ::common::XMLPreferenceEx*)preRoot->firstChild("Streamers");
	if(pStreamPre)
	{
        for(ZQ::common::XMLPreferenceEx* pStreamNode=(ZQ::common::XMLPreferenceEx*)pStreamPre->firstChild("Streamer");pStreamNode; pStreamNode=(ZQ::common::XMLPreferenceEx*)pStreamPre->nextChild())
        {
            TransportData::StreamerData data;
            if(pStreamNode->get("netid",value,"",sizeof(value)))
            {
                data.netId = value;
            }
            if(pStreamNode->get("type",value,"",sizeof(value)))
            {
                data.type = value;
            }
            if(pStreamNode->get("ifep",value,"",sizeof(value)))
            {
                data.ifep = value;
            }
            if(pStreamNode->get("desc",value,"",sizeof(value)))
            {
                data.desc = value;
            }

            ZQ::common::XMLPreferenceEx* pPriv = (ZQ::common::XMLPreferenceEx*)pStreamNode->firstChild("PrivateData");
            if(pPriv)//has private data
            {
                data.privateData = convertXmlToPrivData(log, pPriv);

                pPriv->free();
            }
            td.streamerList.push_back(data);

            pStreamNode->free();
        }
        pStreamPre->free();
    }
	//parse servicegroups
    td.servicegroupList.clear();
	ZQ::common::XMLPreferenceEx* pSerGroupPre = (ZQ::common::XMLPreferenceEx*)preRoot->firstChild("ServiceGroups");
	if(pSerGroupPre)
	{
        for(ZQ::common::XMLPreferenceEx* pSerGroupNode=(ZQ::common::XMLPreferenceEx*)pSerGroupPre->firstChild("Group"); pSerGroupNode; pSerGroupNode=(ZQ::common::XMLPreferenceEx*)pSerGroupPre->nextChild())
        {
            TransportData::ServiceGroupData data;
            if(pSerGroupNode->get("id",value,"",sizeof(value)))
            {
                data.id = value;
            }

            if(pSerGroupNode->get("desc",value,"failure",sizeof(value)))
            {
                data.desc = value;
            }
            td.servicegroupList.push_back(data);

            pSerGroupNode->free();
        }
        pSerGroupPre->free();
    }
	//parse storagelink
    td.storagelinkList.clear();
	ZQ::common::XMLPreferenceEx* pStoLinkPre = (ZQ::common::XMLPreferenceEx*)preRoot->firstChild("StorageLinks");
	if(pStoLinkPre)
	{
        for(ZQ::common::XMLPreferenceEx* pStoLinkNode=(ZQ::common::XMLPreferenceEx*)pStoLinkPre->firstChild("Link"); pStoLinkNode; pStoLinkNode=(ZQ::common::XMLPreferenceEx*)pStoLinkPre->nextChild())
        {
            TransportData::StorageLinkData data;

			if(pStoLinkNode->get("status",value,"",sizeof(value)))
			{
				data.status = value;
			}

            if(pStoLinkNode->get("linkId",value,"",sizeof(value)))
            {
                data.ident = value;
            }

            if(pStoLinkNode->get("storageId",value,"",sizeof(value)))
            {
                data.storageId = value;
            }

            if(pStoLinkNode->get("streamerId",value,"",sizeof(value)))
            {
                data.streamerId = value;
            }

            if(pStoLinkNode->get("type",value,"",sizeof(value)))
            {
                data.type = value;
            }

            ZQ::common::XMLPreferenceEx* pPriv = (ZQ::common::XMLPreferenceEx*)pStoLinkNode->firstChild("PrivateData");
            if(pPriv)//has private data
            {
                data.privateData = convertXmlToPrivData(log, pPriv);

                pPriv->free();
            }
            td.storagelinkList.push_back(data);

            pStoLinkNode->free();
        }
        pStoLinkPre->free();
    }	
	//parse streamlink
    td.streamlinkList.clear();
	ZQ::common::XMLPreferenceEx* pStreamLinkPre = (ZQ::common::XMLPreferenceEx*)preRoot->firstChild("StreamLinks");
	if(pStreamLinkPre)
	{
        for(ZQ::common::XMLPreferenceEx* pStreamLinkNode=(ZQ::common::XMLPreferenceEx*)pStreamLinkPre->firstChild("Link"); pStreamLinkNode; pStreamLinkNode=(ZQ::common::XMLPreferenceEx*)pStreamLinkPre->nextChild())
        {
            TransportData::StreamLinkData data;

			if(pStreamLinkNode->get("status",value,"",sizeof(value)))
			{
				data.status = value;
			}
            if(pStreamLinkNode->get("linkId",value,"",sizeof(value)))
            {
                data.ident = value;
            }
            if(pStreamLinkNode->get("streamerId",value,"",sizeof(value)))
            {
                data.streamerId = value;
            }
            if(pStreamLinkNode->get("servicegroupId",value,"",sizeof(value)))
            {
                data.servicegroupId = value;
            }
            if(pStreamLinkNode->get("type",value,"",sizeof(value)))
            {
                data.type = value;
            }

            ZQ::common::XMLPreferenceEx* pPriv = (ZQ::common::XMLPreferenceEx*)pStreamLinkNode->firstChild("PrivateData");
            if(pPriv)
            {
                data.privateData = convertXmlToPrivData(log, pPriv);
                pPriv->free();
            }
            td.streamlinkList.push_back(data);

            pStreamLinkNode->free();
        }
        pStreamLinkPre->free();
    }
	preRoot->free();

    return true;
}
// the implemention of importXml()
bool AccreditedPathsImpl::doImportXml(const std::string& xmlData, bool cleanup, std::string& err, const ::Ice::Current& c)
{
    err.clear();
    if(cleanup)
	{
		try
		{
			//cleanup storage
			::TianShanIce::Transport::Storages		storages = listStorages(c);
			for(::TianShanIce::Transport::Storages::iterator stoit=storages.begin(); stoit<storages.end(); stoit++)
				removeStorage(stoit->netId,c);

			//cleanup streamer
			::TianShanIce::Transport::Streamers		streamers = listStreamers(c);
			for(::TianShanIce::Transport::Streamers::iterator streamit=streamers.begin(); streamit<streamers.end(); streamit++)
				removeStreamer(streamit->netId,c);

			//cleanup servicegroup
			::TianShanIce::Transport::ServiceGroups sergroups = listServiceGroups(c);
			for(::TianShanIce::Transport::ServiceGroups::iterator sergit=sergroups.begin(); sergit<sergroups.end(); sergit++)
				removeServiceGroup(sergit->id,c);		
		}
		catch(::TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch a exception name[%s]  message[%s] when remove database"),ex.ice_name().c_str(),ex.message.c_str());
            err = ex.ice_name() + " during the cleanup";
			return false;
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch a exception when remove datebase"));
            err = "Unknown exception during the cleanup";
			return false;
		}
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() clean up old data befor import new data"));
	}

	//parse xml file
    TransportData td;
    if(!parseTransportXml(envlog, xmlData, td, err))
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() Failed to parse the xml data. reason:%s"), err.c_str());
        return false;
    }

    // get the operation
#pragma message(__MSGLOC__"The <Operation> interface is hidden here, and is known by AdminCtrl_web only")
    enum {Unknown, Modify, Append, Delete} op = Unknown;
    std::string opDesc = "Unknown";
    {
        std::string opLiteral = td.operation;
        ZQ::common::stringHelper::TrimExtra(opLiteral);
        std::transform(opLiteral.begin(), opLiteral.end(), opLiteral.begin(), tolower);
        if(0 == opLiteral.compare("modify")){
            op = Modify;
            opDesc = "Modify";
        }else if(0 == opLiteral.compare("append")){
            op = Append;
            opDesc = "Append";
        }else if(0 == opLiteral.compare("delete")){
            op = Delete;
            opDesc = "Delete";
        }else{
            op = Unknown;
            opDesc = "Unknown";
        }
    }

    if(Unknown == op)
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() no proper operation to apply.op=[%s]"), td.operation.c_str());
        err = "No proper operation to apply";
        return false;
    }

    envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPathsImpl, "doImportXml() importing transport data with operation %s."), opDesc.c_str());

    // apply the operation

    // the storage, streamer and servicegroup treat the Append and Modify the same way
    { // storage
        TransportData::StorageDataList::iterator it = td.storageList.begin();
        for(; it != td.storageList.end(); ++it)
        {
            if(it->netId.empty() || it->ifep.empty() || it->type.empty())
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() bad attribute of storage: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = "Must supply ifep and type for storage " + it->netId;
                return false;
            }
            try
            {
                if(Delete == op)
                {
                    if(!removeStorage(it->netId, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to remove storage: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                        err = "Failed to remove storage " + it->netId;
                        return false;
                    }
                }
                else
                {
                    if(!updateStorage(it->netId, it->type, it->ifep, it->desc, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to update storage: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                        err = "Failed to update storage " + it->netId;
                        return false;
                    }
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch TianShan exception name[%s] message[%s] when %s storage with netId=[%s] ifep=[%s] type=[%s]"),ex.ice_name().c_str(),ex.message.c_str(), opDesc.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating storage " + it->netId;
                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Ice exception name[%s] when %s storage with netId=[%s] ifep=[%s] type=[%s]"),ex.ice_name().c_str(), opDesc.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating storage " + it->netId;
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Unknown exception when %s storage with netId=[%s] ifep=[%s] type=[%s]"), opDesc.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = "Unknown exception during updating storage " + it->netId;
                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() import storage data is successful. %u items"), td.storageList.size());
    }

    { // streamer
        TransportData::StreamerDataList::iterator it = td.streamerList.begin();
        for(; it != td.streamerList.end(); ++it)
        {
            if(it->netId.empty() || it->ifep.empty() || it->type.empty())
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() bad attribute of streamer: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = "Must supply ifep and type for streamer " + it->netId;
                return false;
            }
            try
            {
                if(Delete == op)
                {
                    if(!removeStreamer(it->netId, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to remove steamer: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                        err = "Failed to remove streamer " + it->netId;
                        return false;
                    }
                }
                else
                {
                    if(!updateStreamer(it->netId, it->type, it->ifep, it->desc, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to update streamer: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                        err = "Failed to update streamer " + it->netId;
                        return false;
                    }
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch TianShan exception name[%s] message[%s] when %s streamer with netId=[%s] ifep=[%s] type=[%s]"),ex.ice_name().c_str(),ex.message.c_str(), opDesc.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating streamer " + it->netId;
                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Ice exception name[%s] when %s streamer with netId=[%s] ifep=[%s] type=[%s]"),ex.ice_name().c_str(), opDesc.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating streamer " + it->netId;
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Unknown exception when %s streamer with netId=[%s] ifep=[%s] type=[%s]"), opDesc.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = "Unknown exception during updating streamer " + it->netId;
                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() import streamer data is successful. %u items"), td.streamerList.size());
    }
    { // service group
        TransportData::ServiceGroupDataList::iterator it = td.servicegroupList.begin();
        for(; it != td.servicegroupList.end(); ++it)
        {
            if(it->id.empty())
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() invalid service group id: id=[%s]"), it->id.c_str());
                err = "Must supply digital id for service group";
                return false;
            }
            if(!it->type.empty())
            {
                envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() service group type is reserved, ignore the current value. id=[%s] type=[%s]"), it->id.c_str(), it->type.c_str());
                it->type.clear();
            }
            Ice::Int sgId = atoi(it->id.c_str());
            try
            {
                if(Delete == op)
                {
                    if(!removeServiceGroup(sgId, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to remove servicegroup: id=[%s]"), it->id.c_str());
                        err = "Failed to remove service group " + it->id;
                        return false;
                    }
                }
                else
                {
                    if(!updateServiceGroup(sgId, it->desc, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to update servicegroup: id=[%s]"), it->id.c_str());
                        err = "Failed to update service group " + it->id;
                        return false;
                    }
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch TianShan exception name[%s] message[%s] when %s servicegroup: id=[%s]"),ex.ice_name().c_str(),ex.message.c_str(), opDesc.c_str(), it->id.c_str());
                err = ex.ice_name() + " during updating service group " + it->id;
                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Ice exception name[%s] when %s servicegroup: id=[%s]"),ex.ice_name().c_str(), opDesc.c_str(), it->id.c_str());
                err = ex.ice_name() + " during updating service group " + it->id;
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Unknown exception when %s servicegroup: id=[%s]"), opDesc.c_str(), it->id.c_str());
                err = "Unknown exception during updating service group " + it->id;
                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() import servicegroup data is successful. %u items"), td.servicegroupList.size());
    }
    { // storage link
        TransportData::StorageLinkDataList::iterator it = td.storagelinkList.begin();
        for(; it != td.storagelinkList.end(); ++it)
        {
            if(it->storageId.empty() || it->streamerId.empty() || it->type.empty())
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() bad attribute of storage link: linkId=[%s] storage=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = "Must supply storage, streamer, and type for storage link";
                return false;
            }
            try
            {
                if(Append == op)
                {
                    if(!it->ident.empty())
                    {
                        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() ignore the storage link identity in the Append mode. linkId=[%s]"), it->ident.c_str());
                    }
                    linkStorage(it->storageId, it->streamerId, it->type, it->privateData, c);
                }
                else
                { // need verify the link object
                    if(it->ident.empty())
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() need storage link identity in the %s mode"), opDesc.c_str());
                        err = "Must supply link id for storage link updating";
                        return false;
                    }
                    Ice::Identity ident = _env._communicator->stringToIdentity(it->ident);
                    TianShanIce::Transport::StorageLinkExPrx lnk;
                    try{
                        lnk = IdentityToObj(StorageLinkEx, ident);
                        lnk->ice_ping();
                    }
                    catch(...){
                        lnk = NULL;
                    }

                    // verify the existence
                    if(!lnk)
                    {
                        if(Delete == op)
                        { // may the streamer or storage be deleted
                            envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() can't delete the storage link [%s] because the link not exist"), it->ident.c_str());
                            continue;
                        }
                        else
                        {
                            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() no storage link object found with identity [%s]"), it->ident.c_str());
                            err = "Can't find link: " + it->ident;
                            return false;
                        }
                    }

                    // verify the attribute
                    std::string storageId = lnk->getStorageId();
                    std::string streamerId = lnk->getStreamerId();
                    std::string type = lnk->getType();
                    if(storageId != it->storageId || streamerId != it->streamerId || type != it->type)
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() imported storage link data don't match the object attribute. linkId=[%s] storageId=[%s]/[%s] streamerId=[%s]/[%s] type=[%s]/[%s]"), it->ident.c_str(), it->storageId.c_str(), storageId.c_str(), it->streamerId.c_str(), streamerId.c_str(), it->type.c_str(), type.c_str());
                        err = "Attributes mismatch for link: " + it->ident;
                        return false;
                    }
                    // apply the operation
                    if(Modify == op)
                    {
                        if(!lnk->updatePrivateData(it->privateData))
                        {
                            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to update the private data of storage link: linkId=[%s]"), it->ident.c_str());
                            err = "Failed to update private data of link: " + it->ident;
                            return false;
                        }
                    }
                    else // Delete
                    {
                        lnk->destroy();
                    }
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch TianShan exception name[%s] message[%s] when %s storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(),ex.message.c_str(), opDesc.c_str(), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = ex.ice_name() + " during";
                if(Append == op)
                {
                    err += " linking storage(" + it->storageId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                }
                else
                {
                    err += " updating link " + it->ident;
                }
                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Ice exception name[%s] when %s storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), opDesc.c_str(), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());

                err = ex.ice_name() + " during";
                if(Append == op)
                {
                    err += " linking storage(" + it->storageId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                }
                else
                {
                    err += " updating link " + it->ident;
                }

                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Unknown exception when %s storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"), opDesc.c_str(), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = "Unknown exception during";
                if(Append == op)
                {
                    err += " linking storage(" + it->storageId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                }
                else
                {
                    err += " updating link " + it->ident;
                }

                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() import storagelink data is successful. %u items"), td.storagelinkList.size());
    }

    { // stream link
        TransportData::StreamLinkDataList::iterator it = td.streamlinkList.begin();
        for(; it != td.streamlinkList.end(); ++it)
        {
            if(it->servicegroupId.empty() || it->streamerId.empty() || it->type.empty())
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() bad attribute of stream link: linkId=[%s] servicegroup=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = "Must supply service group, streamer and type for stream link";
                return false;

            }
            Ice::Int sgId = atoi(it->servicegroupId.c_str());
            try
            {
                if(Append == op)
                {
                    if(!it->ident.empty())
                    {
                        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() ignore the stream link identity in the Append mode. linkId=[%s]"), it->ident.c_str());
                    }
                    linkStreamer(sgId, it->streamerId, it->type, it->privateData, c);
                }
                else
                { // need verify the link object
                    if(it->ident.empty())
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() need stream link identity in the %s mode"), opDesc.c_str());
                        err = "Must supply link id for stream link updating";
                        return false;
                    }
                    Ice::Identity ident = _env._communicator->stringToIdentity(it->ident);
                    TianShanIce::Transport::StreamLinkExPrx lnk;
                    try{
                        lnk = IdentityToObj(StreamLinkEx, ident);
                        lnk->ice_ping();
                    }
                    catch(...){
                        lnk = NULL;
                    }

                    // verify the existence
                    if(!lnk)
                    {
                        if(Delete == op)
                        { // may the servicegroup or streamer be deleted
                            envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() can't delete the stream link [%s] because the link didn't exist"), it->ident.c_str());
                            continue;
                        }
                        else
                        {
                            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() no stream link object found with identity [%s]"), it->ident.c_str());
                            err = "Can't find link: " + it->ident;
                            return false;
                        }
                    }

                    // verify the attribute
                    Ice::Int servicegroupId = lnk->getServiceGroupId();
                    std::string streamerId = lnk->getStreamerId();
                    std::string type = lnk->getType();
                    if(servicegroupId != sgId || streamerId != it->streamerId || type != it->type)
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() imported stream link data don't match the object attribute. linkId=[%s] servicegroup=[%s]/[%d] streamerId=[%s]/[%s] type=[%s]/[%s]"), it->ident.c_str(), it->servicegroupId.c_str(), servicegroupId, it->streamerId.c_str(), streamerId.c_str(), it->type.c_str(), type.c_str());
                        err = "Attributes mismatch for link: " + it->ident;
                        return false;
                    }
                    // apply the operation
                    if(Modify == op)
                    {
                        if(!lnk->updatePrivateData(it->privateData))
                        {
                            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() failed to update the private data of stream link: linkId=[%s]"), it->ident.c_str());
                            err = "Failed to update private data of link: " + it->ident;
                            return false;
                        }
                    }
                    else // Delete
                    {
                        lnk->destroy();
                    }
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch TianShan exception name[%s] message[%s] when %s streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(),ex.message.c_str(), opDesc.c_str(), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = ex.ice_name() + " during";
                if(Append == op)
                {
                    err += " linking servicegroup(" + it->servicegroupId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                }
                else
                {
                    err += " updating link " + it->ident;
                }

                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Ice exception name[%s] when %s streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), opDesc.c_str(), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = ex.ice_name() + " during";
                if(Append == op)
                {
                    err += " linking servicegroup(" + it->servicegroupId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                }
                else
                {
                    err += " updating link " + it->ident;
                }

                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml() catch Unknown exception when %s streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"), opDesc.c_str(), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
               err = "Unknown exception during";
                if(Append == op)
                {
                    err += " linking servicegroup(" + it->servicegroupId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                }
                else
                {
                    err += " updating link " + it->ident;
                }

                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml() import streamlink data is successful. %u items"), td.streamlinkList.size());
    }

    envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPathsImpl, "doImportXml() import transport data OK. operation:%s."), opDesc.c_str());
    return true;
}

static bool isValidInteger(const std::string& s)
{
    if(!s.empty())
    {
        size_t nDigit = std::count_if(s.begin(), s.end(), isdigit);
        if(s[0] == '-')
        {
            return (nDigit == s.size() - 1);
        }
        else
        {
            return (nDigit == s.size());
        }
    }
    else
    {
        return false;
    }
}
// convert 32-bit integer to string
static std::string str(Ice::Int i)
{
    char buf[20];
    sprintf(buf, "%d", i);
    return buf;
}

// the implemention of importXml()
bool AccreditedPathsImpl::doImportXml2(const std::string& xmlData, bool validateOnly, std::string& err, const ::Ice::Current& c)
{
    err.clear();

	//parse xml file
    TransportData td;
    if(!parseTransportXml(envlog, xmlData, td, err))
    {
        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() Failed to parse the xml data. reason:%s"), err.c_str());
        return false;
    }

    // do validate

    typedef std::set<std::string> StringSet;
    // validate servicegroup
    StringSet validServiceGroups;
    for(TransportData::ServiceGroupDataList::iterator it = td.servicegroupList.begin(); it != td.servicegroupList.end(); ++it)
    {
        if(!isValidInteger(it->id))
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() invalid service group id: id=[%s]"), it->id.c_str());
            err = "Must supply digital id for service group " + it->id;
            return false;
        }

        if(!it->type.empty())
        {
            envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() service group type is reserved, ignore the current value. id=[%s] type=[%s]"), it->id.c_str(), it->type.c_str());
            it->type.clear();
        }
        if(!validServiceGroups.insert(it->id).second)
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() duplicate service group id=[%s]"), it->id.c_str());
            err = "duplicate service group " + it->id;
            return false;
        }
    }

    // validate storage
    StringSet validStorages;
    for(TransportData::StorageDataList::iterator it = td.storageList.begin(); it != td.storageList.end(); ++it)
    {
        if(it->netId.empty() || it->ifep.empty() || it->type.empty())
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() bad attribute of storage: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
            err = "Must supply netid(" + it->netId + ") ifep(" + it->ifep + ") and type(" + it->type + ") for storage";
            return false;
        }
        if(!validStorages.insert(it->netId).second)
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() duplicate storage netId=[%s]"), it->netId.c_str());
            err = "duplicate storage " + it->netId;
            return false;
        }
    }

    // validate streamer
    StringSet validStreamers;
    for(TransportData::StreamerDataList::iterator it = td.streamerList.begin(); it != td.streamerList.end(); ++it)
    {
        if(it->netId.empty() || it->ifep.empty() || it->type.empty())
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() bad attribute of streamer: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
            err = "Must supply netid(" + it->netId + ") ifep(" + it->ifep + ") and type(" + it->type + ") for streamer";
            return false;
        }
        if(!validStreamers.insert(it->netId).second)
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() duplicate streamer netId=[%s]"), it->netId.c_str());
            err = "duplicate streamer " + it->netId;
            return false;
        }
    }

    // validate storage link
    StringSet validStorageLinkTypes;
    {
        TianShanIce::StrValues  types = listSupportedStorageLinkTypes(c);
        std::copy(types.begin(), types.end(), std::inserter(validStorageLinkTypes, validStorageLinkTypes.end()));
    }

#define isValInSet(V, S) (S.end() != S.find(V))
    ::TianShanIce::Transport::StorageLinks keptStorageLinks;
    for(TransportData::StorageLinkDataList::iterator it = td.storagelinkList.begin(); it != td.storagelinkList.end(); ++it)
    {
        if(!isValInSet(it->storageId, validStorages)
           || !isValInSet(it->streamerId, validStreamers)
           || !isValInSet(it->type, validStorageLinkTypes))
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() bad attribute of storage link: linkId=[%s] storage=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
            err = "Must supply valid storage(" + it->storageId + ") streamer(" + it->streamerId + ") and type(" + it->type + ") for storage link " + it->ident;
            return false;
        }
#pragma message(__MSGLOC__"TODO: validate the private data")
        if(!it->ident.empty())
        { // check the attributes
            Ice::Identity ident = _env._communicator->stringToIdentity(it->ident);
            TianShanIce::Transport::StorageLinkExPrx lnk;
            try{
                lnk = IdentityToObj(StorageLinkEx, ident);
                lnk->ice_ping();
            }
            catch(...){
                lnk = NULL;
            }

            if(lnk)
            {
                try
                {
                    std::string storageId = lnk->getStorageId();
                    std::string streamerId = lnk->getStreamerId();
                    std::string type = lnk->getType();

					//bool isEnable = (it->status == "0") ? true : false;
					//lnk->enableLink(isEnable);
					int status = atoi(it->status.c_str());
					lnk->importStatus(status);

                    if(storageId != it->storageId || streamerId != it->streamerId || type != it->type)
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() imported storage link data don't match the object attribute. linkId=[%s] storageId=[%s]/[%s] streamerId=[%s]/[%s] type=[%s]/[%s]"), it->ident.c_str(), it->storageId.c_str(), storageId.c_str(), it->streamerId.c_str(), streamerId.c_str(), it->type.c_str(), type.c_str());
                        err = "Attributes mismatch for link: " + it->ident;
                        return false;
                    }
                    keptStorageLinks.push_back(lnk);
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when verify storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = ex.ice_name() + " when verify storagelink " + it->ident;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch unknown exception when verify storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = "Unknown exception when verify storagelink " + it->ident;
                    return false;
                }
            }
            else
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() storagelink not exist. linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = "Link " + it->ident + " not exist";
                return false;
            }
        }
    }

    // validate stream link
    StringSet validStreamLinkTypes;
    {
        TianShanIce::StrValues  types = listSupportedStreamLinkTypes(c);
        std::copy(types.begin(), types.end(), std::inserter(validStreamLinkTypes, validStreamLinkTypes.end()));
    }

    ::TianShanIce::Transport::StreamLinks keptStreamLinks;
    for(TransportData::StreamLinkDataList::iterator it = td.streamlinkList.begin(); it != td.streamlinkList.end(); ++it)
    {
        if(!isValInSet(it->servicegroupId, validServiceGroups)
           || !isValInSet(it->streamerId, validStreamers)
           || !isValInSet(it->type, validStreamLinkTypes))
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() bad attribute of stream link: linkId=[%s] servicegroup=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
            err = "Must supply valid servicegroup(" + it->servicegroupId + ") streamer(" + it->streamerId + ") and type(" + it->type + ") for stream link " + it->ident;
            return false;
        }
#pragma message(__MSGLOC__"TODO: validate the private data")
        if(!it->ident.empty())
        { // check the attributes
            Ice::Identity ident = _env._communicator->stringToIdentity(it->ident);
            TianShanIce::Transport::StreamLinkExPrx lnk;
            try{
                lnk = IdentityToObj(StreamLinkEx, ident);
                lnk->ice_ping();
            }
            catch(...){
                lnk = NULL;
            }

            if(lnk)
            {
                try
                {
                    Ice::Int servicegroupId = lnk->getServiceGroupId();
                    std::string streamerId = lnk->getStreamerId();
                    std::string type = lnk->getType();

					//bool isEnable = (it->status == "0") ? true : false;
					//lnk->enableLink(isEnable);
					int status = atoi(it->status.c_str());
					lnk->importStatus(status);

                    if(servicegroupId != atoi(it->servicegroupId.c_str()) || streamerId != it->streamerId || type != it->type)
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() imported stream link data don't match the object attribute. linkId=[%s] servicegroup=[%s]/[%d] streamerId=[%s]/[%s] type=[%s]/[%s]"), it->ident.c_str(), it->servicegroupId.c_str(), servicegroupId, it->streamerId.c_str(), streamerId.c_str(), it->type.c_str(), type.c_str());

                        err = "Attributes mismatch for link: " + it->ident;
                        return false;
                    }
                    keptStreamLinks.push_back(lnk);
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when verify streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = ex.ice_name() + " when verify streamlink " + it->ident;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch unknown exception when verify streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = "Unknown exception when verify streamlink " + it->ident;
                    return false;
                }
            }
            else
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() streamlink not exist. linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
                err = "Link " + it->ident + " not exist";
                return false;
            }
        }
    }

    // all validated
    if(validateOnly)
    {
        envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPathsImpl, "doImportXml2() validate xml data OK."));
        return true;
    }

    // do import
    // compare and delete
    { // service group
        int nRemoved = 0;
        ::TianShanIce::Transport::ServiceGroups sgs = listServiceGroups(c);
        ::TianShanIce::Transport::ServiceGroups::iterator it;
        for(it = sgs.begin(); it != sgs.end(); ++it)
        {
            std::string sgIdStr = str(it->id);
            if(!isValInSet(sgIdStr, validServiceGroups))
            {
                try
                {
                    if(!removeServiceGroup(it->id, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to remove servicegroup: id=[%d]"), it->id);
                        err = "Failed to remove service group " + sgIdStr;
                        return false;
                    }
                    ++nRemoved;
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught %s when remove servicegroup: id=[%d]"), ex.ice_name().c_str(), it->id);
                    err = "Failed to remove service group " + sgIdStr;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() unknown exception when remove servicegroup: id=[%d]"), it->id);
                    err = "Failed to remove service group " + sgIdStr;
                    return false;
                }
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() removed %d servicegroups"), nRemoved);
    }
    { // storage
        int nRemoved = 0;
        ::TianShanIce::Transport::Storages storages = listStorages(c);
        ::TianShanIce::Transport::Storages::iterator it;
        for(it = storages.begin(); it != storages.end(); ++it)
        {
            if(!isValInSet(it->netId, validStorages))
            {
                try
                {
                    if(!removeStorage(it->netId, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to remove storage: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                        err = "Failed to remove storage " + it->netId;
                        return false;
                    }
                    ++nRemoved;
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught %s when remove storage: netId=[%s]"), ex.ice_name().c_str(), it->netId.c_str());
                    err = "Failed to remove storage " + it->netId;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() unknown exception when remove storage: netId=[%s]"), it->netId.c_str());
                    err = "Failed to remove storage " + it->netId;
                    return false;
                }
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() removed %d storages"), nRemoved);
    }

    { // streamer
        int nRemoved = 0;
        ::TianShanIce::Transport::Streamers streamers = listStreamers(c);
        ::TianShanIce::Transport::Streamers::iterator it;
        for(it = streamers.begin(); it != streamers.end(); ++it)
        {
            if(!isValInSet(it->netId, validStreamers))
            {
                try
                {
                    if(!removeStreamer(it->netId, c))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to remove streamer: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                        err = "Failed to remove streamer " + it->netId;
                        return false;
                    }
                    ++nRemoved;
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught %s when remove streamer: netId=[%s]"), ex.ice_name().c_str(), it->netId.c_str());
                    err = "Failed to remove streamer " + it->netId;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() unknown exception when remove streamer: netId=[%s]"), it->netId.c_str());
                    err = "Failed to remove streamer " + it->netId;
                    return false;
                }
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() removed %d streamers"), nRemoved);
    }
    { // storage link
        int nRemoved = 0;
        // list all the storage links
        ::TianShanIce::Transport::StorageLinks allStorageLinks;
        try
        {
            ::TianShanIce::Transport::Streamers streamers = listStreamers(c);
            ::TianShanIce::Transport::Streamers::iterator it;
            for(it = streamers.begin(); it != streamers.end(); ++it)
            {
                ::TianShanIce::Transport::StorageLinks lnks = listStorageLinksByStreamer(it->netId, c);
                allStorageLinks.insert(allStorageLinks.end(), lnks.begin(), lnks.end());
            }
        }
        catch(const Ice::Exception& ex)
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught %s when list storagelinks"), ex.ice_name().c_str());
            err = "Failed to list storagelinks";
            return false;
        }
        catch(...)
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught unknown exception when list storagelinks"));
            err = "Failed to list storagelinks";
            return false;
        }
        std::sort(allStorageLinks.begin(), allStorageLinks.end(), Ice::proxyIdentityLess);
        std::sort(keptStorageLinks.begin(), keptStorageLinks.end(), Ice::proxyIdentityLess);
        ::TianShanIce::Transport::StorageLinks oldLinks;
        std::set_difference(allStorageLinks.begin(), allStorageLinks.end(), keptStorageLinks.begin(), keptStorageLinks.end(), std::back_inserter(oldLinks), Ice::proxyIdentityLess);
        // do the removing
        for(::TianShanIce::Transport::StorageLinks::iterator it = oldLinks.begin(); it != oldLinks.end(); ++it)
        {
            try
            {
                (*it)->destroy();
                ++nRemoved;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught %s when remove storagelink:%s"), ex.ice_name().c_str(), _env._communicator->proxyToString(*it).c_str());
                err = "Failed to remove old storagelinks";
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught unknown exception when remove storagelink:%s"), _env._communicator->proxyToString(*it).c_str());
                err = "Failed to remove old storagelinks";
                return false;
            }
        }
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() removed %d storagelinks"), nRemoved);
    }

    { // stream link
        int nRemoved = 0;
        // list all the stream links
        ::TianShanIce::Transport::StreamLinks allStreamLinks;
        try
        {
            ::TianShanIce::Transport::Streamers streamers = listStreamers(c);
            ::TianShanIce::Transport::Streamers::iterator it;
            for(it = streamers.begin(); it != streamers.end(); ++it)
            {
                ::TianShanIce::Transport::StreamLinks lnks = listStreamLinksByStreamer(it->netId, c);
                allStreamLinks.insert(allStreamLinks.end(), lnks.begin(), lnks.end());
            }
        }
        catch(const Ice::Exception& ex)
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught %s when list streamlinks"), ex.ice_name().c_str());
            err = "Failed to list streamlinks";
            return false;
        }
        catch(...)
        {
            envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught unknown exception when list streamlinks"));
            err = "Failed to list streamlinks";
            return false;
        }
        std::sort(allStreamLinks.begin(), allStreamLinks.end(), Ice::proxyIdentityLess);
        std::sort(keptStreamLinks.begin(), keptStreamLinks.end(), Ice::proxyIdentityLess);
        ::TianShanIce::Transport::StreamLinks oldLinks;
        std::set_difference(allStreamLinks.begin(), allStreamLinks.end(), keptStreamLinks.begin(), keptStreamLinks.end(), std::back_inserter(oldLinks), Ice::proxyIdentityLess);
        // do the removing
        for(::TianShanIce::Transport::StreamLinks::iterator it = oldLinks.begin(); it != oldLinks.end(); ++it)
        {
            try
            {
                (*it)->destroy();
                ++nRemoved;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught %s when remove streamlink:%s"), ex.ice_name().c_str(), _env._communicator->proxyToString(*it).c_str());
                err = "Failed to remove old streamlinks";
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() caught unknown exception when remove streamlink:%s"), _env._communicator->proxyToString(*it).c_str());
                err = "Failed to remove old streamlinks";
                return false;
            }
        }
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() removed %d streamlinks"), nRemoved);
    }
    // update

    { // service group
        TransportData::ServiceGroupDataList::iterator it = td.servicegroupList.begin();
        for(; it != td.servicegroupList.end(); ++it)
        {
            Ice::Int sgId = atoi(it->id.c_str());
            try
            {
                if(!updateServiceGroup(sgId, it->desc, c))
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to update servicegroup: id=[%s]"), it->id.c_str());
                    err = "Failed to update servicegroup " + it->id;
                    return false;
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch TianShan exception name[%s] message[%s] when update servicegroup: id=[%s]"), ex.ice_name().c_str(), ex.message.c_str(), it->id.c_str());
                err = ex.ice_name() + " during updating service group " + it->id;
                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when update servicegroup: id=[%s]"), ex.ice_name().c_str(), it->id.c_str());
                err = ex.ice_name() + " during updating service group " + it->id;
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Unknown exception when update servicegroup: id=[%s]"), it->id.c_str());
                err = "Unknown exception during updating service group " + it->id;
                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() updated %u servicegroups"), td.servicegroupList.size());
    }


    { // storage
        TransportData::StorageDataList::iterator it = td.storageList.begin();
        for(; it != td.storageList.end(); ++it)
        {
            try
            {
                if(!updateStorage(it->netId, it->type, it->ifep, it->desc, c))
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to update storage: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                    err = "Failed to update storage " + it->netId;
                    return false;
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch TianShan exception name[%s] message[%s] when update storage with netId=[%s] ifep=[%s] type=[%s]"), ex.ice_name().c_str(), ex.message.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating storage " + it->netId;
                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when update storage with netId=[%s] ifep=[%s] type=[%s]"), ex.ice_name().c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating storage " + it->netId;
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Unknown exception when update storage with netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = "Unknown exception during updating storage " + it->netId;
                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() updated %u storages"), td.storageList.size());
    }

    { // streamer
        TransportData::StreamerDataList::iterator it = td.streamerList.begin();
        for(; it != td.streamerList.end(); ++it)
        {
            try
            {
                if(!updateStreamer(it->netId, it->type, it->ifep, it->desc, c))
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to update streamer: netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                    err = "Failed to update streamer " + it->netId;
                    return false;
                }
            }
            catch(const TianShanIce::BaseException& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch TianShan exception name[%s] message[%s] when update streamer with netId=[%s] ifep=[%s] type=[%s]"), ex.ice_name().c_str(), ex.message.c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating streamer " + it->netId;
                return false;
            }
            catch(const Ice::Exception& ex)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when update streamer with netId=[%s] ifep=[%s] type=[%s]"),ex.ice_name().c_str(), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = ex.ice_name() + " during updating streamer " + it->netId;
                return false;
            }
            catch(...)
            {
                envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Unknown exception when update streamer with netId=[%s] ifep=[%s] type=[%s]"), it->netId.c_str(), it->ifep.c_str(), it->type.c_str());
                err = "Unknown exception during updating streamer " + it->netId;
                return false;
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() updated %u streamers"), td.streamerList.size());
    }

    { // storage link
        int nAdded = 0;
        int nUpdated = 0;
        TransportData::StorageLinkDataList::iterator it = td.storagelinkList.begin();
        for(; it != td.storagelinkList.end(); ++it)
        {
            if(it->ident.empty())
            { // new record
                try
                {
                    linkStorage(it->storageId, it->streamerId, it->type, it->privateData, c);
                    ++nAdded;
                }
                catch(const TianShanIce::BaseException& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch TianShan exception name[%s] message[%s] when link storage with storage=[%s] streamer=[%s] type=[%s]"), ex.ice_name().c_str(), ex.message.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
                    err = ex.ice_name() + " during link storage(" + it->storageId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                    return false;
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when link storage with storage=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = ex.ice_name() + " during link storage(" + it->storageId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Unknown exception when link storage with storage=[%s] streamer=[%s] type=[%s]"), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = "Unknown exception during link storage(" + it->storageId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                    return false;
                }
            }
            else
            { // update

                try
                {
                    Ice::Identity ident = _env._communicator->stringToIdentity(it->ident);
                    TianShanIce::Transport::StorageLinkExPrx lnk = IdentityToObj(StorageLinkEx, ident);
                    if(!lnk->updatePrivateData(it->privateData))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to update the private data of storage link: linkId=[%s]"), it->ident.c_str());
                        err = "Failed to update private data of link: " + it->ident;
                        return false;
                    }
                    ++nUpdated;
                }
                catch(const TianShanIce::BaseException& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch TianShan exception name[%s] message[%s] when update storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(),ex.message.c_str(), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
                    err = ex.ice_name() + " during update link " + it->ident;
                    return false;
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when update storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = ex.ice_name() + " during update link " + it->ident;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Unknown exception when update storagelink with linkId=[%s], storage=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->storageId.c_str(), it->streamerId.c_str(), it->type.c_str());
                    err = "Unknown exception during update link " + it->ident;
                    return false;
                }
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() added %u new storagelinks and updated %u storagelinks"), nAdded, nUpdated);
    }

    { // stream link
        int nAdded = 0;
        int nUpdated = 0;
        TransportData::StreamLinkDataList::iterator it = td.streamlinkList.begin();
        for(; it != td.streamlinkList.end(); ++it)
        {
            if(it->ident.empty())
            { // new record
                try
                {
                    linkStreamer(atoi(it->servicegroupId.c_str()), it->streamerId, it->type, it->privateData, c);
                    ++nAdded;
                }
                catch(const TianShanIce::BaseException& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch TianShan exception name[%s] message[%s] when link streamer with servicegroup=[%s] streamer=[%s] type=[%s]"), ex.ice_name().c_str(), ex.message.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
                    err = ex.ice_name() + " during link servicegroup(" + it->servicegroupId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                    return false;
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when link streamer with servicegroup=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = ex.ice_name() + " during link servicegroup(" + it->servicegroupId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Unknown exception when link streamer with servicegroup=[%s] streamer=[%s] type=[%s]"), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = "Unknown exception during link servicegorup(" + it->servicegroupId + ")";
                    err += " streamer(" + it->streamerId + ")";
                    err += " with type " + it->type;
                    return false;
                }
            }
            else
            { // update

                try
                {
                    Ice::Identity ident = _env._communicator->stringToIdentity(it->ident);
                    TianShanIce::Transport::StreamLinkExPrx lnk = IdentityToObj(StreamLinkEx, ident);
                    if(!lnk->updatePrivateData(it->privateData))
                    {
                        envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() failed to update the private data of stream link: linkId=[%s]"), it->ident.c_str());
                        err = "Failed to update private data of link: " + it->ident;
                        return false;
                    }
                    ++nUpdated;
                }
                catch(const TianShanIce::BaseException& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch TianShan exception name[%s] message[%s] when update streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(),ex.message.c_str(), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
                    err = ex.ice_name() + " during update link " + it->ident;
                    return false;
                }
                catch(const Ice::Exception& ex)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Ice exception name[%s] when update streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"),ex.ice_name().c_str(), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());

                    err = ex.ice_name() + " during update link " + it->ident;
                    return false;
                }
                catch(...)
                {
                    envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AccreditedPathsImpl, "doImportXml2() catch Unknown exception when update streamlink with linkId=[%s], servicegroup=[%s] streamer=[%s] type=[%s]"), it->ident.c_str(), it->servicegroupId.c_str(), it->streamerId.c_str(), it->type.c_str());
                    err = "Unknown exception during update link " + it->ident;
                    return false;
                }
            }
        } // end for
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "doImportXml2() added %u new streamlinks and updated %u streamlinks"), nAdded, nUpdated);
    }

    envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPathsImpl, "doImportXml2() import transport data OK."));
    return true;
}

void AccreditedPathsImpl::importXml(const ::std::string& pathsDef, bool cleanup, const ::Ice::Current& c)
{
	if(pathsDef.length() == 0)
	{
        ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths", 1201, "importXml() need a valid file name."));
	}
    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "importXml() start importing transport data from file %s."), pathsDef.c_str());

    std::string xmlData;
    // load xml file into the memery
    std::ifstream xmlFile(pathsDef.c_str());
    if(xmlFile.good())
    {
        std::ostringstream buf;
        char ch;
        while(xmlFile.get(ch))
        {
            buf << ch;
        }

        // check the state
        if(xmlFile.eof())
        {
			std::string tmp = buf.str();	
            xmlData.swap(tmp);
        }
        else
        {
            ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths", 1202, "importXml() failed to load file %s."), pathsDef.c_str());
        }
    }
    else
    { // file not exist
        ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT("AccreditedPaths", 1203, "importXml() cannot read file %s."), pathsDef.c_str());
    }

#pragma message(__MSGLOC__"Not roll back from the fail importing")

    // do the import
    std::string err;
    bool importOk = false;
    try
    {
        importOk = doImportXml2(xmlData, false, err, c);
        //            ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT("AccreditedPaths", 1205, "importXml() failed to import file %s and can't restore the data."), pathsDef.c_str());
    }
    catch(...)
    {
        importOk = false;
        err = "Unexpected exception";
    }

    if(importOk)
    { // everything OK, log the success
        envlog(ZQ::common::Log::L_INFO, CLOGFMT(AccreditedPathsImpl, "importXml() import file %s successfully"), pathsDef.c_str());
    }
    else
    { // fail to do the import, rollback and report error
        ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT("AccreditedPaths", 1204, "importXml() failed to import file %s. Reason:%s"), pathsDef.c_str(), err.c_str());
    }
}

::std::string AccreditedPathsImpl::privDataToXml(::TianShanIce::ValueMap& valMap)
{
	std::string strXml = "";	

	for(::TianShanIce::ValueMap::iterator mapit = valMap.begin(); mapit != valMap.end(); mapit++)
	{
		std::string strVec = "";
		strXml += "<Data key=\"" + mapit->first + "\" type=\"";
		if(mapit->second.type == ::TianShanIce::vtBin)//bype type
		{
			char buf[20] = {0};
			for(::TianShanIce::BValues::iterator it = mapit->second.bin.begin(); it < mapit->second.bin.end(); it++)
			{
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%c",*it);
				strVec += "<Item value=\"";
				strVec += buf;
				strVec += "\" />";
			}
			strXml += "Byte"; 
		}
		else if(mapit->second.type == ::TianShanIce::vtInts)//int type
		{
			char buf[20] = {0};
			for(::TianShanIce::IValues::iterator it = mapit->second.ints.begin(); it < mapit->second.ints.end(); it++)
			{
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%d",*it);
				strVec += "<Item value=\"";
				strVec += buf;
				strVec += "\" />";
			}
			strXml += "Int";
		}
		else if(mapit->second.type == ::TianShanIce::vtLongs)//long type
		{
			char buf[50] = {0};
			for(::TianShanIce::LValues::iterator it = mapit->second.lints.begin(); it < mapit->second.lints.end(); it++)
			{
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%lld",*it);
				strVec += "<Item value=\"";
				strVec += buf;
				strVec += "\" />";
			}
			strXml += "Long";
		}
		else if(mapit->second.type == ::TianShanIce::vtStrings)//string type
		{
			for(::TianShanIce::StrValues::iterator it = mapit->second.strs.begin(); it < mapit->second.strs.end(); it++)
			{
				strVec += "<Item value=\"";
				strVec += *it;
				strVec += "\" />";
			}
			strXml += "String";
		}
		else if(mapit->second.type == ::TianShanIce::vtFloats)//float type
		{
			char buf[20] = {0};
			for(::TianShanIce::FValues::iterator it = mapit->second.floats.begin(); it < mapit->second.floats.end(); it++)
			{
				memset(buf,0,sizeof(buf));
				sprintf(buf,"%f",*it);
				strVec += "<Item value=\"";
				strVec += buf;
				strVec += "\" />";
			}
			strXml += "Float";
		}
		else//do not know the type
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AccreditedPathsImpl, "privDataToXml() not know the private data type"));

		strXml += "\" range=\"";
		if(mapit->second.bRange)
			strXml += "1\" >";
		else
			strXml += "0\" >";

		strXml += strVec + "</Data>";
	}

	return strXml;
}

static ::TianShanIce::ValueMap convertXmlToPrivData(ZQ::common::Log& log, ZQ::common::XMLPreferenceEx* pPre)
{
	::TianShanIce::ValueMap valMap;
	char value[1024] = {0};
	//parse private data,and push to the right struct
	for(ZQ::common::XMLPreferenceEx* pNode=(ZQ::common::XMLPreferenceEx*)pPre->firstChild("Data"); pNode; pNode=(ZQ::common::XMLPreferenceEx*)pPre->nextChild())
	{
		std::string strKey = "";
		std::string strType = "";
		::TianShanIce::Variant variant;
		memset(value, 0, sizeof(value));
		pNode->get("key",value,"failure",sizeof(value));
		strKey = value;
		memset(value, 0, sizeof(value));
		pNode->get("type",value,"failure",sizeof(value));
		strType = value;
		memset(value, 0, sizeof(value));
		pNode->get("range",value,"failure",sizeof(value));
		variant.bRange = (atoi(value) ==1) ? true : false;

		if(stricmp(strType.c_str(),"byte") == 0)
		{
			variant.type = ::TianShanIce::vtBin;
			for(ZQ::common::XMLPreferenceEx* pvec=(ZQ::common::XMLPreferenceEx*)pNode->firstChild("Item"); pvec; pvec=(ZQ::common::XMLPreferenceEx*)pNode->nextChild())
			{
				memset(value, 0, sizeof(value));
				pvec->get("value",value,"failure",sizeof(value));
				variant.bin.push_back(value[0]);
				pvec->free();
			}
		}
		else if(stricmp(strType.c_str(),"int") == 0)
		{
			variant.type = ::TianShanIce::vtInts;
			for(ZQ::common::XMLPreferenceEx* pvec=(ZQ::common::XMLPreferenceEx*)pNode->firstChild("Item"); pvec; pvec=(ZQ::common::XMLPreferenceEx*)pNode->nextChild())
			{
				memset(value, 0, sizeof(value));
				pvec->get("value",value,"failure",sizeof(value));
				variant.ints.push_back(atoi(value));	
				pvec->free();
			}
		}
		else if(stricmp(strType.c_str(),"long") == 0)
		{
			variant.type = ::TianShanIce::vtLongs;
			for(ZQ::common::XMLPreferenceEx* pvec=(ZQ::common::XMLPreferenceEx*)pNode->firstChild("Item"); pvec; pvec=(ZQ::common::XMLPreferenceEx*)pNode->nextChild())
			{
				memset(value, 0, sizeof(value));
				pvec->get("value",value,"failure",sizeof(value));
				variant.lints.push_back(_atoi64(value));
				pvec->free();
			}
		}
		else if(stricmp(strType.c_str(),"float") == 0)
		{
			variant.type = ::TianShanIce::vtFloats;
			for(ZQ::common::XMLPreferenceEx* pvec=(ZQ::common::XMLPreferenceEx*)pNode->firstChild("Item"); pvec; pvec=(ZQ::common::XMLPreferenceEx*)pNode->nextChild())
			{
				memset(value, 0, sizeof(value));
				pvec->get("value",value,"failure",sizeof(value));
				variant.floats.push_back((float)atof(value));
				pvec->free();
			}
		}
		else if(stricmp(strType.c_str(),"string") == 0)
		{
			variant.type = ::TianShanIce::vtStrings;
			for(ZQ::common::XMLPreferenceEx* pvec=(ZQ::common::XMLPreferenceEx*)pNode->firstChild("Item"); pvec; pvec=(ZQ::common::XMLPreferenceEx*)pNode->nextChild())
			{
				memset(value, 0, sizeof(value));
				pvec->get("value",value,"failure",sizeof(value));
				variant.strs.push_back(value);		
				pvec->free();
			}
		}
		else
		{
			log(ZQ::common::Log::L_WARNING, CLOGFMT(AccreditedPathsImpl, "xmlToPrivData() not know private date type the key is[%s]"),strKey.c_str());
			pNode->free();
			continue;
		}
		if(variant.bin.size() || variant.ints.size() || variant.lints.size() || variant.floats.size() || variant.strs.size())
		{
			std::pair<std::string,::TianShanIce::Variant> pair;
			pair = std::make_pair(strKey,variant);

			valMap.insert(pair);
		}
		else
			log(ZQ::common::Log::L_WARNING, CLOGFMT(AccreditedPathsImpl, "xmlToPrivData() not have value data the key is[%s]"),strKey.c_str());

		pNode->free();
	}

	return valMap;
}
::TianShanIce::ValueMap AccreditedPathsImpl::xmlToPrivData(ZQ::common::XMLPreferenceEx* pPre)
{
    return convertXmlToPrivData(envlog, pPre);
}


}} // namespace

// vim: ts=4 sw=4 bg=dark
