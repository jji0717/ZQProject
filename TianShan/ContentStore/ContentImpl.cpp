// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentImpl.cpp $
// 
// 45    1/11/16 5:42p Dejian.fei
// 
// 44    8/13/15 4:02p Li.huang
// 
// 43    2/02/15 3:53p Hui.shao
// merged from V1.15
// 
// 42    9/04/14 5:35p Hui.shao
// 
// 41    3/05/13 4:27p Hui.shao
// 
// 40    12/19/12 5:27p Hui.shao
// 
// 39    9/20/12 4:47p Hui.shao
// ticket#12077, round up the invalid startTime to sec in provision()
// 
// 38    8/09/12 12:21p Hui.shao
// 
// 37    8/09/12 12:17p Hui.shao
// 
// 36    7/09/12 3:12p Hui.shao
// 
// 35    6/13/12 11:43a Hui.shao
// 
// 34    4/27/12 5:57p Hui.shao
// added Content::getProvisionSession()
// 
// 33    4/09/12 6:51p Hui.shao
// 
// 32    11/30/11 9:28p Hui.shao
// merged playtime interval limitation from v1.15
// 
// 33    11/29/11 4:36p Hui.shao
// ticket#9981, JIRA ACE-9345, do not call Vstrm
// VstrmClassLoadFullAssetInfoEx() too frequently
// 
// 32    9/15/11 4:30p Hui.shao
// 
// 31    9/06/11 3:50p Hui.shao
// added an event Content::ProvisionScheduleAdjusted
// 
// 30    8/22/11 10:13a Hongquan.zhang
// 
// 29    7/06/11 4:10p Hui.shao
// 
// 28    7/06/11 3:49p Hui.shao
// reviewed createContent()
// 
// 27    6/23/11 10:26a Hongquan.zhang
// 
// 26    6/22/11 7:33p Build
// 
// 25    6/22/11 5:46p Hongquan.zhang
// add a wlock to getsubtype
// 
// 24    6/16/11 6:12p Hongquan.zhang
// 
// 23    5/26/11 5:51p Hongquan.zhang
// 
// 22    5/26/11 4:03p Hongquan.zhang
// 
// 21    4/11/11 5:58p Jie.zhang
// add metadata METADATA_MonoProvision
// 
// 20    4/11/11 4:43p Hui.shao
// added mono-provision test
// 
// 19    3/18/11 3:52p Jie.zhang
// 
// 18    3/18/11 2:52p Jie.zhang
// 
// 17    3/18/11 12:59p Hui.shao
// 
// 16    3/18/11 11:31a Hui.shao
// 
// 15    3/17/11 5:44p Jie.zhang
// 
// 14    3/17/11 5:08p Hui.shao
// 
// 13    3/17/11 4:32p Hui.shao
// make the cluster-ed CS to force to create slave replicas in
// createContent()
// 
// 12    3/04/11 11:42a Jie.zhang
// 
// 8     2/23/11 7:36p Jie.zhang
// support subfolder for mediaclustercs
// 
// 7     1/24/11 2:23p Hui.shao
// 
// 6     1/14/11 2:03p Hui.shao
// updated log format for objects
// 
// 5     10-12-08 14:04 Hui.shao
// 
// 4     10-12-08 12:23 Hui.shao
// enable timer() persistently for volume, and added volume::setMetaData()
// 
// 3     10-11-25 18:56 Hui.shao
// scheduled periodical checking free space into volume::OnTimer
// 
// 2     10-11-17 18:08 Hui.shao
// include Volume under the watchdog to yield fs-sync at the begining of
// service start
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 72    10-10-29 10:10 Hongquan.zhang
// fix bug which lead to failed to open content event it exist
// 
// 71    10-10-27 14:58 Hui.shao
// double checked the mergie the changes in V1.10 since 3/19/2009
// 
// 69    10-10-26 13:23 Hui.shao
// confirmed the merge from V1.10
// 
// 68    10-08-11 17:17 Hui.shao
// disabled the duplicated invocation to _cancelProvision()
// 
// 67    10-04-28 13:21 Haoyuan.lu
// 
// 66    10-04-26 15:17 Hui.shao
// added cached content list
// 
// 65    10-03-05 15:40 Fei.huang
// * added NoResource exception to exception specifications of provision
// 
// 64    10-03-03 16:58 Xia.chen
// adjust opensubfolder implement
// 
// 63    10-02-22 10:42 Yixin.tian
// change VolumeImpl::destroy() lock
// 
// 62    10-02-05 15:58 Yixin.tian
// support subfolder
// 
// 61    10-01-29 16:00 Jie.zhang
// fix the openSubFolder volume ident issue
// 
// 60    10-01-25 17:26 Jie.zhang
// add listSubFolder to folder
// 
// 59    09-12-22 17:46 Hui.shao
// abstracted folder from volume
// 
// 58    09-12-22 14:06 Jie.zhang
// merge from TianShan1.10
// 
// 57    09-09-14 16:17 Hui.shao
// 
// 56    09-09-14 14:43 Hui.shao
// 
// 55    09-09-11 11:17 Hui.shao
// made the auto sync with filesystem optional
// 
// 54    09-09-07 14:04 Hui.shao
// added invoke signature and update content delete strategy
// 
// 53    09-09-03 13:24 Hui.shao
// separate the handling on interested file events
// 
// 52    09-07-30 14:10 Jie.zhang
// 
// 51    09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 50    09-07-24 15:00 Xia.chen
// change the third parameter from contentName to contentKey for
// getExportUrl of contentstore
// 
// 49    09-06-11 15:09 Hui.shao
// exported some timeout as configurable, enlarged the interval of
// populating if know populating fail
// 
// 48    09-06-05 10:54 Jie.zhang
// merge from 1.10
// 
// 49    09-05-15 11:08 Jie.zhang
// the schedulestart stop can be empty in adjustschedule
// 
// 48    09-05-13 21:19 Jie.zhang
// adjustProvisionSchedule() implementation added
// 
// 47    09-04-28 14:33 Jie.zhang
// disable the file check on content open
// 
// 46    09-02-20 18:39 Hongquan.zhang
// 
// 45    09-02-05 13:53 Hui.shao
// added the event entires from the ContentStoreLib to the impls derived
// from the base ContentStoreImpl
// 
// 44    09-01-23 15:44 Hongquan.zhang
// 
// 43    08-12-25 18:26 Hui.shao
// 
// 42    08-12-25 18:02 Hui.shao
// 
// 41    08-12-18 16:38 Jie.zhang
// add volume info cache
// 
// 40    08-12-18 12:03 Jie.zhang
// 
// 39    08-12-17 21:23 Jie.zhang
// 
// 38    08-12-17 18:39 Hui.shao
// 
// 37    08-12-17 16:48 Hui.shao
// quit the program more quickly from sync procedure
// 
// 36    08-12-16 11:57 Hui.shao
// 
// 35    08-12-16 11:24 Hui.shao
// restrict names of content/volume to open
// 
// 34    08-12-11 12:17 Hui.shao
// 
// 33    08-11-28 15:52 Yixin.tian
// modify provision timestamps
// 
// 32    08-11-27 16:35 Yixin.tian
// 
// 31    08-11-26 17:29 Hui.shao
// enter Provisioning if the Content::provision() is submitted
// 
// 30    08-11-24 12:45 Hui.shao
// force to populate attr if streamable and in provisioning
// 
// 29    08-11-24 12:29 Jie.zhang
// add a parameter on checkResidencialStatus
// 
// 28    08-11-19 17:48 Yixin.tian
// 
// 27    08-11-18 18:30 Yixin.tian
// 
// 26    08-11-18 17:38 Yixin.tian
// modify can compile for Linux OS
// 
// 25    08-11-18 14:00 Hui.shao
// fix the setMetaData()s
// 
// 24    08-11-15 15:20 Hui.shao
// 
// 23    08-11-15 14:01 Hui.shao
// added check file residential status checking
// 
// 22    08-11-13 16:10 Jie.zhang
// fix volume::destroy() bug
// 
// 21    08-11-13 11:54 Jie.zhang
// fixed the openContent logic
// 
// 20    08-11-11 18:43 Jie.zhang
// 
// 19    08-11-11 18:04 Fei.huang
// + wildcard match in VolumeImpl->listContent
// 
// 18    08-11-10 11:48 Hui.shao
// pre-fill the some attributes when query for metadata, added OnIdle()
// entry
// 
// 17    08-11-07 13:52 Hui.shao
// added _volume() for portal to access without locking the content obj
// 
// 16    08-11-07 11:00 Jie.zhang
// add common log define to unify log using style
// 
// 15    08-11-06 18:19 Hui.shao
// added a Content::isInUse() query for clusterCS to query node
// 
// 14    08-11-06 13:23 Hui.shao
// added provision event handling
// 
// 13    08-11-03 11:35 Hui.shao
// splitted CS impl to ContentStoreImpl.cpp
// 
// 12    08-11-03 11:17 Jie.zhang
// add contentName to some interface
// 
// 11    08-10-30 16:17 Hui.shao
// forward ContentStoreImpl::listContents_async
// 
// 10    08-10-29 18:50 Yixin.tian
// 
// 9     08-10-28 17:57 Hui.shao
// test the last write if it is worthy to populate attributes
// 
// 8     08-10-28 13:15 Yixin.tian
// change opensubvolume function ident to identChild
// 
// 7     08-10-27 10:43 Jie.zhang
// 
// 6     08-10-23 18:31 Hui.shao
// moved watchdog out to common
// 
// 5     08-10-14 11:33 Hui.shao
// 
// 4     08-10-07 19:54 Hui.shao
// added volume layer
// 
// 2     08-08-14 18:01 Hui.shao
// 
// 14    08-08-13 15:56 Hui.shao
// 
// 13    08-08-13 12:42 Hui.shao
// 
// 12    08-08-11 18:42 Hui.shao
// added store replica handling
// 
// 10    08-07-31 18:43 Hui.shao
// restrict on state for provision-related operation
// 
// 9     08-07-31 17:20 Hui.shao
// added the portail enties for provisioning
// 
// 8     08-07-29 12:16 Hui.shao
// added event log as sentry's input
// 
// 7     08-07-21 11:51 Hui.shao
// check in the works of last weekend
// 
// 6     08-07-18 18:55 Hui.shao
// 
// 3     08-07-15 14:19 Hui.shao
// 
// 2     08-07-14 10:57 Hui.shao
// 
// 1     08-07-10 19:29 Hui.shao
// ===========================================================================
#include "ContentImpl.h"
#include "ContentState.h"
#include "ContentCmds.h"
#include "Guid.h"
#include "Log.h"
#include "ContentUser.h"
#include "ContentSysMD.h"


#define ContentEventFMT(_EVENT, _EVENTCODE, _X) EventFMT(_store._netId.c_str(), Content, _EVENT, _EVENTCODE, "Content[%s] vol[%s] name[%s] " _X), ident.name.c_str(), identVolume.name.c_str(), _name().c_str()

#ifdef ZQ_OS_MSWIN
	#include <shlwapi.h>
	extern "C"
	{
		#include <io.h>
	};

#else 
	extern "C"
	{
		#include "fnmatch.h"
	}
#endif


#pragma comment(lib, "shlwapi.lib")

#define MOLOG	(_store._log)
#define MOEVENT	(_store._eventlog)

namespace ZQTianShan {
namespace ContentStore {

#define ContentStateLOGFMT(_C, _X) CLOGFMT(_C, "content[%s:%s(%d)] " _X), ident.name.c_str(), ContentStateBase::stateStr(state), state
#define ContentStateEXPFMT(_C, _ERRCODE, _X) EXPFMT(_C, _ERRCODE, "content[%s:%s(%d)] " _X), ident.name.c_str(), ContentStateBase::stateStr(state), state

std::string invokeSignature(const ::Ice::Current& c)
{
	::Ice::Context::const_iterator itCtx = c.ctx.find("signature");
	if (c.ctx.end() != itCtx)
		return itCtx->second;

	if(!c.con || c.requestId <=0)
		return ""; // make sure return empty string if it is not a remote call from the Content interface

	char buf[64];
	return ::ZQTianShan::IceCurrentToStr(c) + " @"+ ZQTianShan::TimeToUTC(ZQTianShan::now(), buf, sizeof(buf) -2, true);
}

// -----------------------------
// class ContentImpl
// -----------------------------
ContentImpl::ContentImpl(ContentStoreImpl& store)
:_store(store)
{
}

ContentImpl::ContentImpl(ContentImpl& old, const std::string& newName)
:_store(old._store)
{
	ident = _store.toContentIdent(old.identVolume.name + LOGIC_FNSEPS + newName);

	stampCreated          = old.stampCreated;
	stampProvisioned      = old.stampProvisioned;
	stampLastUpdated      = old.stampLastUpdated;
	state                 = old.state;
	provisionPrxStr       = old.provisionPrxStr;
	trickSpeeds           = old.trickSpeeds;
//	metaData              = old.metaData;

	identVolume           =  old.identVolume;

	fromFsOrphan          = old.fromFsOrphan;
	expiration            = old.expiration;
	bDirtyAttrs           = old.bDirtyAttrs;
}
	
::Ice::Identity ContentImpl::getIdent(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident;
}

bool ContentImpl::isDirty(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return bDirtyAttrs;
}

bool ContentImpl::isCorrupt(const ::Ice::Current& c) const
{
	RLock sync(*this);

	try 
	{
		ContentImpl::Ptr pContent = const_cast<ContentImpl*>(this);
		uint64 flags = ContentStoreImpl::checkResidentialStatus(_store, RSDFLAG(frfAbsence), pContent, ident.name, _mainFilePathname());
		return (0 != flags);
	}
	catch (...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(Content, "doPopulateFromFs() portal::checkResidentialStatus() throw exception, quit state changing"));
		return false;
	}

	return false;
}

::TianShanIce::Storage::ContentStorePrx ContentImpl::getStore(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return _store.proxy(true);
}

::std::string ContentImpl::getName(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return _name();
}

::std::string ContentImpl::_name() const
{
	size_t nOffset = identVolume.name.length()+1;
	if (nOffset>=ident.name.length())
		return "";

	return ident.name.substr(nOffset);
}

::TianShanIce::Storage::ContentState ContentImpl::getState(const ::Ice::Current& c) const
{
#ifdef ZQ_OS_MSWIN	
	RLock sync(*this);
#endif//
	return state;
}

::TianShanIce::Properties ContentImpl::getMetaData(const ::Ice::Current& c) const
{
	::TianShanIce::Properties ret;
	std::string strCreated, strProvisioned, strLastUpdated;

	try {
		RLock sync(*this);
		ret = metaData;

		char buf[64];
		strCreated = ZQTianShan::TimeToUTC(stampCreated, buf, sizeof(buf));
		strProvisioned = ZQTianShan::TimeToUTC(stampProvisioned, buf, sizeof(buf));
		strLastUpdated = ZQTianShan::TimeToUTC(stampLastUpdated, buf, sizeof(buf));
	}
	catch(...) {}

	MAPSET(::TianShanIce::Properties, ret, SYS_PROP(Name),				_name());
	MAPSET(::TianShanIce::Properties, ret, SYS_PROP(Volume),			identVolume.name);
	MAPSET(::TianShanIce::Properties, ret, SYS_PROP(StampCreated),		strCreated);
	MAPSET(::TianShanIce::Properties, ret, SYS_PROP(StampProvisioned),	strProvisioned);
	MAPSET(::TianShanIce::Properties, ret, SYS_PROP(StampLastUpdated),	strLastUpdated);
	MAPSET(::TianShanIce::Properties, ret, SYS_PROP(State),				ContentStateBase::stateStr(state));
	return ret;
}

void ContentImpl::setUserMetaData(const ::std::string& key, const ::std::string& value, const ::Ice::Current& c)
{
	if (key.empty() || 0 == key.compare(0, sizeof(SYS_PROP_PREFIX) -1, SYS_PROP_PREFIX))
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Content, 1001, "setUserMetaData() invalid metadata[%s] to update"), key.c_str());

	WLock sync(*this);
	std::string userKey = (0 != key.compare(0, sizeof(USER_PROP_PREFIX) -1, USER_PROP_PREFIX))
		? std::string(USER_PROP_PREFIX) + key : key;

	MAPSET(::TianShanIce::Properties, metaData, userKey, value);
}

void ContentImpl::setUserMetaData2(const ::TianShanIce::Properties& metadata, const ::Ice::Current& c)
{
	WLock sync(*this);
	for(::TianShanIce::Properties::const_iterator itMD = metadata.begin(); itMD != metadata.end(); itMD ++)
	{
		if (itMD->first.empty() || 0 == itMD->first.compare(0, sizeof(SYS_PROP_PREFIX) -1, SYS_PROP_PREFIX))
			continue; // ignore any tries to set a system metadata

		std::string userKey = (0 != itMD->first.compare(0, sizeof(USER_PROP_PREFIX) -1, USER_PROP_PREFIX))
							? std::string(USER_PROP_PREFIX) + itMD->first : itMD->first;

		MAPSET(::TianShanIce::Properties, metaData, userKey, itMD->second);
	}
}

bool ContentImpl::isProvisioned(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return (TianShanIce::Storage::csInService == state || TianShanIce::Storage::csProvisioningStreamable == state  || TianShanIce::Storage::csProvisioning == state);
}

::std::string ContentImpl::getProvisionTime(const ::Ice::Current& c) const
{
	RLock sync(*this);
	if (0 == stampProvisioned)
		return "";

	char buf[32];
	return ZQTianShan::TimeToUTC(stampProvisioned, buf, sizeof(buf));
}

/*
bool ContentImpl::isInUse(const ::Ice::Current& c)
{
	return ContentStoreImpl::isContentInUse(_store, *this, _mainFilePathname());
}
*/

::Ice::Long ContentImpl::checkResidentialStatus(::Ice::Long flagsToTest, const ::Ice::Current& c) const
{
	return ContentStoreImpl::checkResidentialStatus(_store, flagsToTest, const_cast<ContentImpl*> (this), ident.name,  _mainFilePathname());
}

void ContentImpl::destroy(const ::Ice::Current& c)
//		throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "destroy()"));
	destroy2(false, c);
}
    
void ContentImpl::destroy2(bool mandatory, const ::Ice::Current& c)
//		throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt)
{
	std::string signature = invokeSignature(c);
	WLock sync(*this);
	MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "destroy2() mandatory[%s] sig[%s]"), mandatory ? "true" : "false", signature.c_str());

	if (!mandatory && state > ::TianShanIce::Storage::csNotProvisioned && state < ::TianShanIce::Storage::csOutService)
	{
		std::string strMainFile = _mainFilePathname();
		
		uint64 flags = ContentStoreImpl::checkResidentialStatus(_store, RSDFLAG(frfResidential) | RSDFLAG(frfReading) | RSDFLAG(frfWriting), const_cast<ContentImpl*> (this), ident.name, strMainFile);
		if ((flags & RSDFLAG(frfResidential)) && (flags & (RSDFLAG(frfReading) | RSDFLAG(frfWriting))))		
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(ContentStore, ::TianShanIce::Storage::csexpContentInUse, "Content[%s] is in use, refuse to non-mandatory destroy"), ident.name.c_str());
	}

	if (!signature.empty())
		MAPSET(::TianShanIce::Properties, metaData, SYS_PROP(DestroySignature), signature);

	ContentStateOutService(_store, *this).enter();
}

::std::string ContentImpl::getLocaltype(const ::Ice::Current& c) const
{
	RLock sync(*this);
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_LocalType);
	return (metaData.end() == it) ? "" : it->second;
}

::std::string ContentImpl::getSubtype(const ::Ice::Current& c) 
{
	std::string subType;

	{
		RLock sync(*this);
		::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_SubType);
		subType = (metaData.end() == it) ? "" : it->second;
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "getSubtype() [%s]"), subType.c_str());

	if (subType.empty() && (::TianShanIce::Storage::csProvisioning == state || ::TianShanIce::Storage::csProvisioningStreamable == state))
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "getSubtype() populate"));
		{
			WLock sync(*this);
			(const_cast<ContentImpl*>(this))->populateAttrDirect();
		}

		{
			RLock sync(*this);
			::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_SubType);
			subType = (metaData.end() == it) ? "" : it->second;
		}
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "getSubtype() [%s]"), subType.c_str());
	return subType;
}


::Ice::Float ContentImpl::getFramerate(const ::Ice::Current& c) const
//		throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt)
{
	if (!isProvisioned(c))
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStore, 1001, "getFramerate() not supported in this state"));

	RLock sync(*this);
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_FrameRate);
	if (metaData.end() == it)
		return 0;
	
	::Ice::Float fps = (::Ice::Float) ::atof(it->second.c_str());
	return fps;
}

void ContentImpl::getResolution(::Ice::Int& pixelHorizontal, ::Ice::Int& pixelVertical, const ::Ice::Current& c) const
//	throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt)
{
	if (!isProvisioned(c))
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStore, 1001, "getResolution() not supported in this state"));

	RLock sync(*this);
	pixelHorizontal = pixelVertical = 0;
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_PixelHorizontal);
	if (metaData.end() != it)
		pixelHorizontal = ::atoi(it->second.c_str());

	it = metaData.find(METADATA_PixelVertical);
	if (metaData.end() != it)
		pixelVertical = ::atoi(it->second.c_str());
}

::Ice::Long ContentImpl::getFilesize(const ::Ice::Current& c) const
{
	RLock sync(*this);
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_FileSize);
	if (metaData.end() == it)
		return 0;
#ifdef ZQ_OS_MSWIN	
	return ::_atoi64(it->second.c_str());
#else
	uint64 fs = 0;
	fs = strtoull(it->second.c_str(), (char* *)NULL ,10);
	return fs;
#endif
}

::Ice::Long ContentImpl::getSupportFileSize(const ::Ice::Current& c) const
{
	RLock sync(*this);
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_SupportFileSize);
	if (metaData.end() == it)
		return 0;
#ifdef ZQ_OS_MSWIN	
	return ::_atoi64(it->second.c_str());
#else
	uint64 fs = 0;
	fs = strtoull(it->second.c_str(), (char* *)NULL ,10);
	return fs;
#endif
}

::Ice::Long ContentImpl::getPlayTime(const ::Ice::Current& c) const
{
	return getPlayTimeEx(c) /1000; // convert msec to sec
}

bool ContentImpl::populateAttrDirect()
{
	try
	{
		::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, identVolume);
		std::string pathOfVolume = volume->getMountPath();

		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "populateAttrDirect() found path-of-volume[%s]"), pathOfVolume.c_str());

		WLock sync(*this);
		if (ContentStoreImpl::populateAttrsFromFile(_store, *(const_cast<ContentImpl*>(this)), pathOfVolume + _name()))
		{
			stampLastUpdated = ZQTianShan::now();
			return true;
		}
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "populateAttrDirect() looking up the path-of-volume caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "populateAttrDirect() looking up the path-of-volume caught unknown exception"));
	}

	return false;
}

::Ice::Long ContentImpl::getPlayTimeEx(const ::Ice::Current& c) const
{
	if (::TianShanIce::Storage::csProvisioning == state || ::TianShanIce::Storage::csProvisioningStreamable == state)
	{
		// ticket#9981, JIRA ACE-9345, do not call Vstrm VstrmClassLoadFullAssetInfoEx() too frequently
		int64 esp = ZQTianShan::now() - stampLastUpdated;
		if (_store._timeoutOfPlaytimeAtProvisioning >0 && esp > _store._timeoutOfPlaytimeAtProvisioning) 
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "getPlayTimeEx() attribute is out-of-date per diff[%lld] > %dmsec, try populate attribute from file system"), esp, _store._timeoutOfPlaytimeAtProvisioning);
			(const_cast<ContentImpl*>(this))->populateAttrDirect();
		}
	}

	RLock sync(*this);
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_PlayTime);
	if (metaData.end() == it)
		return 0;
	
	return ::_atoi64(it->second.c_str());
}

::Ice::Int ContentImpl::getBitRate(const ::Ice::Current& c) const
{
	RLock sync(*this);
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_BitRate);
	if (metaData.end() == it)
		return 0;

	return ::atol(it->second.c_str());
}

::std::string ContentImpl::getMD5Checksum(const ::Ice::Current& c) const
{
	RLock sync(*this);
	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_MD5CheckSum);
	if (metaData.end() == it)
		return "";
	
	return it->second;
}

::TianShanIce::Storage::TrickSpeedCollection ContentImpl::getTrickSpeedCollection(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return trickSpeeds;
}

::std::string ContentImpl::getSourceUrl(const ::Ice::Current& c) const
{
	RLock sync(*this);

	::TianShanIce::Properties::const_iterator it = metaData.find(METADATA_SourceUrl);
	if (metaData.end() == it)
		return "";
	
	return it->second;
}

::std::string ContentImpl::getExportURL(const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
//?	WLock sync(*this);

	if (::TianShanIce::Storage::csInService != state && ::TianShanIce::Storage::csProvisioningStreamable != state)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 301, "getExportURL() content[%s] state[%s(%d)] invalid state"), ident.name.c_str(), ContentStateBase::stateStr(state), state);

	if (transferProtocol.empty()) 
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Content, TianShanIce::Storage::csexpUnsupportProto, "getExportURL() no protocol specified"));

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = getName(c);
	contentKey.contentStoreNetId = _store._netId;
	contentKey.volume =  (*this).identVolume.name;

	std::string url = _store.getExportURL(_store, *this,contentKey, transferProtocol, transferBitrate, ttl, params);

	MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "getExportURL() transferProtocol[%s] transferBitrate[%d] by sig[%s], returns url[%s] ttl[%d]"), 
		transferProtocol.c_str(), transferBitrate, invokeSignature(c).c_str(), url.c_str(), ttl);

	return url;
}

void ContentImpl::provision(const ::std::string& sourceUrl,
							const ::std::string& sourceType,
							bool overwrite,
							const ::std::string& startTimeUTC,
							const ::std::string& stopTimeUTC,
							::Ice::Int maxTransferBitrate,
							const ::Ice::Current& c)
//							throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
//                                   ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	std::string signature = invokeSignature(c);

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "provision() source[%s] type[%s] overwrite[%d] bitrate[%d] timeWindow[%s ~ %s] sig[%s]"), 
		sourceUrl.c_str(), sourceType.c_str(), overwrite?1:0, maxTransferBitrate, startTimeUTC.c_str(), stopTimeUTC.c_str(), signature.c_str());

	if (::TianShanIce::Storage::csNotProvisioned != state)
	{
		// call the ontimer
		bDirtyAttrs = true;
		_store._watchDog.watch(ident, 0);
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 201, "provision() content[%s], current state[%s(%d)] not the [NotProvisioned]"), ident.name.c_str(), ContentStateBase::stateStr(state), state);
	}

	char buf[128];
	if (!overwrite && stampProvisionSetup >0)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 202, "provision() content[%s] provision has been submitted at %s"), ident.name.c_str(), ZQTianShan::TimeToUTC(stampProvisionSetup, buf, sizeof(buf)));

	if (startTimeUTC.empty() || stopTimeUTC.empty())
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Content, TianShanIce::Storage::csexpInvalidTime, "provision time is not specified"));

	::Ice::Long stopTime = ZQTianShan::ISO8601ToTime(stopTimeUTC.c_str()),
	startTime = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str()),
	stampNow = ZQTianShan::now();

	if (stopTime <=0)
		stopTime = ZQTianShan::ISO8601ToTime("2999-12-31T23:59:59Z");
	if (startTime < stampNow)
		startTime = ((::Ice::Long) ((stampNow +999)/1000)) *1000; // ticket#12077, round up to sec

	if (stopTime <= startTime)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Content, TianShanIce::Storage::csexpInvalidTime, "provision stopTime is no later than startTime or now"));
	
	std::string strStartUTC,strStopUTC;
	memset(buf,0,sizeof(buf));
	strStartUTC =  ZQTianShan::TimeToUTC(startTime,buf,sizeof(buf));
	memset(buf,0,sizeof(buf));
	strStopUTC = ZQTianShan::TimeToUTC(stopTime,buf,sizeof(buf));
	::TianShanIce::Properties md2slave;

	{		
		WLock sync(*this);

#define SET_METADATA(_KEY, _VAL)	MAPSET(::TianShanIce::Properties, metaData, _KEY, _VAL); MAPSET(::TianShanIce::Properties, md2slave, _KEY, _VAL)

		::TianShanIce::Properties::iterator itMetaData;
		SET_METADATA(METADATA_SourceUrl, sourceUrl);
		SET_METADATA(METADATA_SourceType, sourceType);
		SET_METADATA(METADATA_ScheduledProvisonStart, strStartUTC);
		SET_METADATA(METADATA_ScheduledProvisonEnd, strStopUTC);
		SET_METADATA(SYS_PROP(ProvisionSignature), signature);

		memset(buf,0,sizeof(buf));
		std::stringstream os;
		os<<maxTransferBitrate;
		os>>buf;
		SET_METADATA(METADATA_MaxProvisonBitRate, buf);
	}

	TianShanIce::ContentProvision::ProvisionSessionPrx provisionSess = _store.submitProvision(_store, *this, getName(c),
		sourceUrl, sourceType, strStartUTC, strStopUTC, maxTransferBitrate);

	do
	{
		if (!_store._storeAggregate)
			break;

		std::string strVolume, strFolder, strContent;
		_store.chopPathname(ident.name, strVolume, strFolder, strContent);			

		if (strFolder.empty())
			break;

		::TianShanIce::Storage::ContentStoreExPrx nodeStore;
		std::string replicaId, strCPENetId, strMonoProvision;
		strCPENetId = metaData["sys.CPE.NetId"];
		strMonoProvision = metaData[METADATA_MonoProvision];
		if (!strMonoProvision.empty())
			MAPSET(::TianShanIce::Properties, md2slave, METADATA_MonoProvision, strMonoProvision);
		SET_METADATA(METADATA_MasterReplicaNetId, strCPENetId);

		ContentStoreImpl::NodeReplicaQueue nrqueue;
		_store.buildNodeReplicaQueue(nrqueue);

		if (nrqueue.empty())
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "no node replica when try to opencontent on slave"));
			break;
		}

		for(; !nrqueue.empty(); nrqueue.pop())
		{
			const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;
			nodeStore = ::TianShanIce::Storage::ContentStoreExPrx::uncheckedCast(replica.obj);	
			replicaId = replica.replicaId;

			if (!nodeStore)
				continue;

			std::string tmpstr = ident.name;

			size_t pos = tmpstr.find(LOGIC_FNSEPC, 1);
			tmpstr = tmpstr.substr(pos+1);

			try 
			{
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "openVolume(%s) on replica[%s]"), strVolume.c_str(), replicaId.c_str());
				::TianShanIce::Storage::FolderPrx folder = ::TianShanIce::Storage::FolderPrx::uncheckedCast(nodeStore->openVolume(strVolume));
				for (size_t pos = tmpstr.find(LOGIC_FNSEPC); std::string::npos != pos; pos = tmpstr.find(LOGIC_FNSEPC))
				{
					strFolder = tmpstr.substr(0, pos);
					MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "openSubFolder(%s) on replica[%s]"), strFolder.c_str(), replicaId.c_str());
					folder = folder->openSubFolder(strFolder, true, 0);
					tmpstr = tmpstr.substr(pos+1);
				}
				strContent = tmpstr;

				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s]"), strContent.c_str(), replicaId.c_str());
				::TianShanIce::Storage::ContentPrx contentPrx = folder->openContent(strContent, "", true);
				if (contentPrx)
				{
					MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] successful"), 
						strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str());

					::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(contentPrx);

					uniContent->setMetaData(md2slave);
					uniContent->enterState(::TianShanIce::Storage::csProvisioning);
					MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "[%s] set content metadata %s=%s on replica[%s]"), 
						strContent.c_str(), METADATA_MasterReplicaNetId, strCPENetId.c_str(), replicaId.c_str());
				}
			}
			catch(const ::Ice::ObjectNotExistException&)
			{
				MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught ObjectNotExistException"), 
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str());
			}
			catch(const ::TianShanIce::BaseException& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught exception[%s]: %s"), 
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught exception[%s]"),
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught unknown exception"),
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str());
			}
		}
	}while(0);

	WLock sync(*this);
	if (provisionSess)
		provisionPrxStr = _store._adapter->getCommunicator()->proxyToString(provisionSess);

	stampProvisionSetup = now();
	ContentStateProvisioning(_store, *this).enter();
}

::std::string ContentImpl::provisionPassive(const ::std::string& sourceType,
						bool overwrite,
						const ::std::string& startTimeUTC,
						const ::std::string& stopTimeUTC,
						::Ice::Int maxTransferBitrate,
						const ::Ice::Current& c)
//	throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
//           ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	std::string signature = invokeSignature(c);
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "provisionPassive() type[%s] overwrite[%d] bitrate[%d] timeWindow[%s ~ %s] sig[%s]"),
		sourceType.c_str(), overwrite?1:0, maxTransferBitrate, startTimeUTC.c_str(), stopTimeUTC.c_str(), signature.c_str());

	if (::TianShanIce::Storage::csNotProvisioned != state)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 201, "provisionPassive() content[%s] state[%s(%d)]"), ident.name.c_str(), ContentStateBase::stateStr(state), state);

	char buf[128];
	if (!overwrite && stampProvisionSetup >0)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 202, "provisionPassive() content[%s] provision has been submitted at %s"), ident.name.c_str(), ZQTianShan::TimeToUTC(stampProvisionSetup, buf, sizeof(buf)));

	if (startTimeUTC.empty() || stopTimeUTC.empty())
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Content, TianShanIce::Storage::csexpInvalidTime, "provision time is not specified"));

	::Ice::Long stopTime = ZQTianShan::ISO8601ToTime(stopTimeUTC.c_str()),
	startTime = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str()),
	stampNow = ZQTianShan::now();

	if (stopTime <=0)
		stopTime = ZQTianShan::ISO8601ToTime("2999-12-31T23:59:59.999Z");
	if (startTime < stampNow)
		startTime = stampNow;


	if (stopTime <= startTime)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Content, TianShanIce::Storage::csexpInvalidTime, "provision stopTime is no later than startTime or now"));

	std::string strStartUTC,strStopUTC;
	memset(buf,0,sizeof(buf));
	strStartUTC =  ZQTianShan::TimeToUTC(startTime,buf,sizeof(buf));
	memset(buf,0,sizeof(buf));
	strStopUTC = ZQTianShan::TimeToUTC(stopTime,buf,sizeof(buf));

	::TianShanIce::Properties md2slave;
	{
		WLock sync(*this);

		::TianShanIce::Properties::iterator itMetaData;
		SET_METADATA(METADATA_SourceType, sourceType);
		SET_METADATA(METADATA_ScheduledProvisonStart, strStartUTC);
		SET_METADATA(METADATA_ScheduledProvisonEnd, strStopUTC);
		SET_METADATA(SYS_PROP(ProvisionSignature), signature);

		memset(buf,0,sizeof(buf));
		std::stringstream os;
		os<<maxTransferBitrate;
		os>>buf;
		SET_METADATA(METADATA_MaxProvisonBitRate, buf);
	}

	std::string pushUrl;

	TianShanIce::ContentProvision::ProvisionSessionPrx provisionSess = _store.bookPassiveProvision(_store, *this, getName(c),pushUrl, sourceType, strStartUTC, strStopUTC, maxTransferBitrate);

	do
	{
		if (!_store._storeAggregate)
			break;

		std::string strVolume, strFolder, strContent;
		_store.chopPathname(ident.name, strVolume, strFolder, strContent);			

		if (strFolder.empty())
			break;

		::TianShanIce::Storage::ContentStoreExPrx nodeStore;
		std::string replicaId, strCPENetId, strMonoProvision;
		strCPENetId = metaData["sys.CPE.NetId"];
		strMonoProvision = metaData[METADATA_MonoProvision];
		if (!strMonoProvision.empty())
			MAPSET(::TianShanIce::Properties, md2slave, METADATA_MonoProvision, strMonoProvision);
		SET_METADATA(METADATA_MasterReplicaNetId, strCPENetId);

		ContentStoreImpl::NodeReplicaQueue nrqueue;
		_store.buildNodeReplicaQueue(nrqueue);

		if (nrqueue.empty())
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "no node replica when try to opencontent on slave"));
			break;
		}

		for(; !nrqueue.empty(); nrqueue.pop())
		{
			const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;
			nodeStore = ::TianShanIce::Storage::ContentStoreExPrx::uncheckedCast(replica.obj);	
			replicaId = replica.replicaId;

			if (!nodeStore)
				continue;

			std::string tmpstr = ident.name;

			size_t pos = tmpstr.find(LOGIC_FNSEPC, 1);
			tmpstr = tmpstr.substr(pos+1);

			try 
			{
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "openVolume(%s) on replica[%s]"), strVolume.c_str(), replicaId.c_str());
				::TianShanIce::Storage::FolderPrx folder = ::TianShanIce::Storage::FolderPrx::uncheckedCast(nodeStore->openVolume(strVolume));
				for (size_t pos = tmpstr.find(LOGIC_FNSEPC); std::string::npos != pos; pos = tmpstr.find(LOGIC_FNSEPC))
				{
					strFolder = tmpstr.substr(0, pos);
					MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "openSubFolder(%s) on replica[%s]"), strFolder.c_str(), replicaId.c_str());
					folder = folder->openSubFolder(strFolder, true, 0);
					tmpstr = tmpstr.substr(pos+1);
				}
				strContent = tmpstr;

				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s]"), strContent.c_str(), replicaId.c_str());
				::TianShanIce::Storage::ContentPrx contentPrx = folder->openContent(strContent, "", true);
				if (contentPrx)
				{
					MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] successful"), 
						strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str());

					::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(contentPrx);

					uniContent->setMetaData(md2slave);
					uniContent->enterState(::TianShanIce::Storage::csProvisioning);
					MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ClusterCS, "[%s] set content metadata %s=%s on replica[%s]"), 
						strContent.c_str(), METADATA_MasterReplicaNetId, strCPENetId.c_str(), replicaId.c_str());
				}
			}
			catch(const ::Ice::ObjectNotExistException&)
			{
				MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught ObjectNotExistException"), 
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str());
			}
			catch(const ::TianShanIce::BaseException& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught exception[%s]: %s"), 
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught exception[%s]"),
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(Content, "openContent(%s) on replica[%s] volume[%s] folder[%s] caught unknown exception"),
					strContent.c_str(), replicaId.c_str(), strVolume.c_str(), strFolder.c_str());
			}
		}
	}while(0);


	{
		WLock sync(*this);
		if (provisionSess)
			provisionPrxStr = _store._adapter->getCommunicator()->proxyToString(provisionSess);

		stampProvisionSetup = now();
		ContentStateProvisioning(_store, *this).enter();
	}

	return pushUrl;
}

void ContentImpl::cancelProvision(const ::Ice::Current& c)
//	throw (::TianShanIce::InvalidStateOfArt, ::TianShanIce::NotImplemented)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "cancelProvision() sig[%s]"), invokeSignature(c).c_str());

	if(::TianShanIce::Storage::csProvisioning != state && ::TianShanIce::Storage::csProvisioningStreamable != state)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("Content", ::TianShanIce::Storage::csexpContentIsReady, "unable to cancel provisioned content[%s]", ident.name.c_str());

	_cancelProvision();

#pragma message ( __MSGLOC__ "TBD: cancelProvision() should we keep the content in NotProvisioned for next provision()??")
	ContentStateCleaning(_store, *this).enter();
}

void ContentImpl::_cancelProvision()
{
	if (!stampProvisionSetup)
		return;

	_store.cancelProvision(_store, *this, provisionPrxStr);

	WLock sync(*this);
	stampProvisionSetup = 0;
	provisionPrxStr = "";
}

::TianShanIce::ContentProvision::ProvisionSessionPrx ContentImpl::getProvisionSession(const ::Ice::Current& c) const
{
	RLock sync(*this);

	if (!stampProvisionSetup || provisionPrxStr.empty())
		return NULL;

	if (::TianShanIce::Storage::csProvisioningStreamable != state && ::TianShanIce::Storage::csProvisioning != state)
		return NULL;

	return ::TianShanIce::ContentProvision::ProvisionSessionPrx::uncheckedCast(_store._adapter->getCommunicator()->stringToProxy(provisionPrxStr));
}

void ContentImpl::populateAttrsFromFilesystem(const ::Ice::Current& c)
{
	WLock sync(*this);

	switch(state)
	{
	case ::TianShanIce::Storage::csNotProvisioned:
		ContentStateNotProvisioned(_store, *this).doPopulateFromFs(c);
		break;

	case ::TianShanIce::Storage::csProvisioning:
		ContentStateProvisioning(_store, *this).doPopulateFromFs(c);
		break;

	case ::TianShanIce::Storage::csProvisioningStreamable:
		ContentStateProvisioningStreamable(_store, *this).doPopulateFromFs(c);
		break;

	case ::TianShanIce::Storage::csInService:
		ContentStateInService(_store, *this).doPopulateFromFs(c);
		break;

	case ::TianShanIce::Storage::csOutService:
		ContentStateOutService(_store, *this).doPopulateFromFs(c);
		break;

	case ::TianShanIce::Storage::csCleaning:
		ContentStateCleaning(_store, *this).doPopulateFromFs(c);
		break;
	}

#ifdef _DEBUG
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "populateAttrsFromFilesystem() executed"));
#endif // _DEBUG
}

void ContentImpl::setMetaData(const ::TianShanIce::Properties& newMetaData, const ::Ice::Current& c)
{
	WLock sync(*this);
	std::string setStr;
	for (::TianShanIce::Properties::const_iterator it = newMetaData.begin(); it != newMetaData.end(); it++)
	{
		if (it->first.empty())
			continue;

		if (metaData.end() == metaData.find(it->first))
			metaData.insert(*it);
		else metaData[it->first] = it->second;

		setStr += it->first + "[" + it->second + "]; ";
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "setMetaData() %s"), setStr.c_str());
}

::TianShanIce::Storage::VolumePrx ContentImpl::theVolume(const ::Ice::Current& c) const
{
    RLock sync(*this);
    return _volume();
}

::TianShanIce::Storage::VolumeExPrx ContentImpl::_volume() const
{
    ::TianShanIce::Storage::VolumeExPrx volume;

	try {
       volume = IdentityToObjEnv(_store, VolumeEx, identVolume);
    }
    catch (const ::Ice::Exception& ex)
    {
       MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(Content, "_volume() vol[%s] caught exception[%s]"), identVolume.name.c_str(), ex.ice_name().c_str());
    }
    catch(...)
    {
       MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(Content, "_volume() vol[%s] caught unknown exception"), identVolume.name.c_str());
    }

    return volume;
}

void ContentImpl::OnTimer(const ::Ice::Current& c)
{
	WLock sync(*this);
	switch(state)
	{
	case ::TianShanIce::Storage::csNotProvisioned:
		ContentStateNotProvisioned(_store, *this).OnTimer(c);
		break;

	case ::TianShanIce::Storage::csProvisioning:
		ContentStateProvisioning(_store, *this).OnTimer(c);
		break;

	case ::TianShanIce::Storage::csProvisioningStreamable:
		ContentStateProvisioningStreamable(_store, *this).OnTimer(c);
		break;

	case ::TianShanIce::Storage::csInService:
		ContentStateInService(_store, *this).OnTimer(c);
		break;

	case ::TianShanIce::Storage::csOutService:
		ContentStateOutService(_store, *this).OnTimer(c);
		break;

	case ::TianShanIce::Storage::csCleaning:
		ContentStateCleaning(_store, *this).OnTimer(c);
		break;
	}
}

void ContentImpl::OnRestore(const ::std::string& stampLastFileWrite, const ::Ice::Current& c)
{
#ifdef _DEBUG
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnRestore() stampLastFileWrite[%s]"), stampLastFileWrite.c_str());
#endif // _DEBUG

	WLock sync(*this);
	switch(state)
	{
	case ::TianShanIce::Storage::csNotProvisioned:
		ContentStateNotProvisioned(_store, *this).OnRestore(stampLastFileWrite, c);
		break;

	case ::TianShanIce::Storage::csProvisioning:
		ContentStateProvisioning(_store, *this).OnRestore(stampLastFileWrite, c);
		break;

	case ::TianShanIce::Storage::csProvisioningStreamable:
		ContentStateProvisioningStreamable(_store, *this).OnRestore(stampLastFileWrite, c);
		break;

	case ::TianShanIce::Storage::csInService:
		ContentStateInService(_store, *this).OnRestore(stampLastFileWrite, c);
		break;

	case ::TianShanIce::Storage::csOutService:
		ContentStateOutService(_store, *this).OnRestore(stampLastFileWrite, c);
		break;

	case ::TianShanIce::Storage::csCleaning:
		ContentStateCleaning(_store, *this).OnRestore(stampLastFileWrite, c);
		break;

	default: 
		return;
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnRestore() stampLastFileWrite[%s] DB record activated"), stampLastFileWrite.c_str());
}

void ContentImpl::OnFileModified(const ::Ice::Current& c)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileModified() enter"));

	WLock sync(*this);
	bDirtyAttrs = true;
	_store._watchDog.watch(ident, 500); // per ticket#12777, yield 500msec in order to check fs as of the latest fileevent of a punch of member files are closed shortly by ingestion
}

void ContentImpl::OnFileCreated( const ::std::string& memebrFilename, const ::Ice::Current& )
{
	if (!_store._storeAggregate)
	{
		WLock sync(*this);
		bDirtyAttrs = true;
		///get file set
		int iSubFileIndex = 0;
		std::vector<std::string> fileSet;
		char buf[1024];
		std::string		value;	
		while ( 1 )
		{
			sprintf( buf , "%s%d" , METADATA_SUBFILENAME , iSubFileIndex );
			TianShanIce::Properties::const_iterator itMetaData = metaData.find(buf);
			if( itMetaData != metaData.end() )
			{
				if (!stricmp(memebrFilename.c_str(), itMetaData->second.c_str()))
					return;

				fileSet.push_back( itMetaData->second );			
			}		
			else
			{
				break;
			}
			iSubFileIndex++;
		}	
		{
			sprintf( buf , "%s%d" , METADATA_SUBFILENAME , iSubFileIndex );
			metaData[buf] = memebrFilename;
			_store._watchDog.watch(ident, 0);
		}
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileCreated() with filename %s"), memebrFilename.c_str());
}

void ContentImpl::OnFileRenamed(const ::std::string& newName, const ::Ice::Current& c)
{
	WLock sync(*this);
	::Ice::Identity identNew;
	try {

		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileRenamed() look up the path-of-volume"));

		::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, identVolume);
		std::string pathOfVolume = volume->getMountPath();

		if (0 != newName.compare(0, pathOfVolume.length(), pathOfVolume))
			ZQTianShan::_IceThrow<TianShanIce::NotSupported> (MOLOG, EXPFMT(Content, 4007, "Not support renaming across volume from volpath[%s] to new[%s]"), pathOfVolume.c_str(), newName.c_str());

		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileRenamed() found path-of-volume[%s]"), pathOfVolume.c_str());

		if (! _store.completeRenaming(_store, pathOfVolume + _name(), newName))
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileRenamed() portal confirm renaming has not been complete or allowed"));
		}
		else
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileRenamed() processing renaming to \"%s\""), newName.c_str());
			ContentImpl::Ptr newContent = new ContentImpl(*this, newName.substr(pathOfVolume.length()));

			if (newContent && 0 != newContent->ident.name.compare(ident.name))
			{
				identNew = newContent->ident;
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileRenamed() add into the database"));
				_store._eContent->add(newContent, identNew);
				::TianShanIce::Storage::UnivContentPrx contentPrx = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_store._adapter->createProxy(identNew));

				if (contentPrx)
				{
					MOEVENT(EventFMT(_store._netId.c_str(), Content, Duplicated, 5, "Content[%s:%s(%d)] vol[%s] name[%s] duplicatedFrom[%s]"), newContent->ident.name.c_str(), ContentStateBase::stateStr(newContent->state), newContent->state, newContent->identVolume.name.c_str(), newContent->_name().c_str(), ident.name.c_str());
					char buf[64];
					contentPrx ->OnRestore(ZQTianShan::TimeToUTC(ZQTianShan::now(), buf, sizeof(buf) -2));

					::TianShanIce::Properties params;
					char tempbuf[64];
					MAPSET(::TianShanIce::Properties, params, SYS_PROP(Volume),           newContent->identVolume.name);
					MAPSET(::TianShanIce::Properties, params, SYS_PROP(ContentFullName),  newContent->ident.name);
					MAPSET(::TianShanIce::Properties, params, SYS_PROP(StateId),          itoa(newContent->state, tempbuf, 10));

					ZQ::common::MutexGuard gd(_store._lockReplicaSubscriberMap);
					if (_store._replicaSubscriberMap.end() != _store._replicaSubscriberMap.find(_store._prxstrMasterReplica))
					{
						MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "OnFileRenamed() needs to forward the create event to the master replica[%s]"), _store._prxstrMasterReplica.c_str());
						(new ForwardEventToMasterCmd(_store, _store._replicaSubscriberMap[_store._prxstrMasterReplica], newContent->ident.name, ::TianShanIce::Storage::fseFileCreated, params))->execute();
					}
				}
			}
		}	
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(Content, "OnFileRenamed() renaming to [%s] caught exception[%s]: %s"), identNew.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(Content, "OnFileRenamed() renaming to [%s] caught exception[%s]"), identNew.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(Content, "OnFileRenamed() renaming to [%s] caught unknown exception"), identNew.name.c_str());
	}

	MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "OnFileRenamed() renamed to [%s], old one should go to OutService"), identNew.name.c_str());
	ContentStateOutService(_store, *this).enter();
}

::std::string ContentImpl::getMainFilePathname(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return _mainFilePathname();
}

::TianShanIce::Storage::ContentState ContentImpl::enterState(::TianShanIce::Storage::ContentState targetState, const ::Ice::Current& c)
{
	std::string signature =invokeSignature(c);
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "sig[%s] request to enter state %s"), 
		signature.c_str(), ContentStateBase::stateStr(targetState));

	WLock sync(*this);
	switch (targetState)
	{
	case ::TianShanIce::Storage::csNotProvisioned:
		ContentStateNotProvisioned(_store, *this).enter();
		break;

	case ::TianShanIce::Storage::csProvisioning:
		ContentStateProvisioning(_store, *this).enter();
		break;

	case ::TianShanIce::Storage::csProvisioningStreamable:
		ContentStateProvisioningStreamable(_store, *this).enter();
		break;

	case ::TianShanIce::Storage::csInService:
		ContentStateInService(_store, *this).enter();
		break;

	case ::TianShanIce::Storage::csOutService:
		if (state == ::TianShanIce::Storage::csProvisioning || state == ::TianShanIce::Storage::csProvisioningStreamable)
		{
			MOEVENT(ContentEventFMT(ProvisionGivenUp, 24, "cancel provisioning because goes to OutService"));
		}
		ContentStateOutService(_store, *this).enter();
		break;

	case ::TianShanIce::Storage::csCleaning:
		ContentStateCleaning(_store, *this).enter();
		break;
	}

	return state;
}

::std::string ContentImpl::_mainFilePathname() const
{
	try {
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "_mainFilePathname() look up the path-of-volume"));

		::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, identVolume);
		std::string pathOfVolume = volume->getMountPath();
		return pathOfVolume + _name();
	}
	catch (const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (MOLOG, EXPFMT(Content, 301, "getMainFilePathname() content[%s] caught exception[%s]"), ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (MOLOG, EXPFMT(Content, 301, "getMainFilePathname() content[%s] caught unknown exception"), ident.name.c_str());
	}

	return ""; // program will never touch here
}

void ContentImpl::adjustProvisionSchedule( const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const ::Ice::Current& c)
//	throw (::TianShanIce::InvalidStateOfArt)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "adjustProvisionSchedule() timeWindow[%s ~ %s]"),
		startTimeUTC.c_str(), stopTimeUTC.c_str());

	if (::TianShanIce::Storage::csProvisioningStreamable != state && ::TianShanIce::Storage::csProvisioning != state)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 201, "adjustProvisionSchedule() content[%s] state[%s(%d)]"), ident.name.c_str(), ContentStateBase::stateStr(state), state);

	if (provisionPrxStr.empty())
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 201, "adjustProvisionSchedule() content[%s] no provision session session found"), ident.name.c_str());

	if (startTimeUTC.empty() && stopTimeUTC.empty())
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Content, TianShanIce::Storage::csexpInvalidTime, "provision time is not specified"));

	::Ice::Long stopTime = ZQTianShan::ISO8601ToTime(stopTimeUTC.c_str());
	::Ice::Long startTime = ZQTianShan::ISO8601ToTime(startTimeUTC.c_str());

	std::string strStartUTC,strStopUTC;
	char buf[128];	
	if (startTime>0)
	{
		memset(buf,0,sizeof(buf));
		if (ZQTianShan::TimeToUTC(startTime,buf,sizeof(buf)))
			strStartUTC =  buf;
	}
	
	if (stopTime>0)
	{
		memset(buf,0,sizeof(buf));
		if (ZQTianShan::TimeToUTC(stopTime,buf,sizeof(buf)))
			strStopUTC = buf;
	}

	try
	{
		TianShanIce::ContentProvision::ProvisionSessionPrx session;
		session = TianShanIce::ContentProvision::ProvisionSessionPrx::uncheckedCast(
			_store._adapter->getCommunicator()->stringToProxy(provisionPrxStr));

		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(Content, "updating schedule time to CPE: start[%s] stop[%s]"),
			startTimeUTC.c_str(), stopTimeUTC.c_str());

		session->updateScheduledTime(startTimeUTC, stopTimeUTC);
	} 
	catch (const Ice::Exception& ex) 
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Content, 201, "updateScheduledTime() content[%s] caught %s"),
			ident.name.c_str(),	ex.ice_name().c_str());
	}

	{		
		WLock sync(*this);

		if (!strStartUTC.empty())
		{
			metaData[METADATA_ScheduledProvisonStart] = strStartUTC;
		}

		if (!strStopUTC.empty())
		{
			metaData[METADATA_ScheduledProvisonEnd] = strStopUTC;
		}
	}

	MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(Content, "adjustProvisionSchedule() successful"));
	MOEVENT(ContentEventFMT(ProvisionScheduleAdjusted, 27, " new inputted provisionStart[%s] provisionEnd[%s]"), strStartUTC.c_str(), strStopUTC.c_str());
}


// -----------------------------
// module VolumeImpl
// -----------------------------
#define VolLOGFMT(_X) CLOGFMT(Volume, "vol[%s] " _X), ident.name.c_str()
#define VolEventFMT(_EVENT, _EVENTCODE, _X) EventFMT(_store._netId.c_str(), Volume, _EVENT, _EVENTCODE, "vol[%s] " _X), ident.name.c_str()

VolumeImpl::VolumeImpl(ContentStoreImpl& store)
: _store(store) //, _bSyncWithFSOnPending(false)
{
}

VolumeImpl::~VolumeImpl()
{
}

::std::string VolumeImpl::getVolumeName(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

::std::string VolumeImpl::getName(const ::Ice::Current& c) const
{
	return getVolumeName(c);
}

void VolumeImpl::getCapacity(::Ice::Long& freeMB, ::Ice::Long& totalMB, const ::Ice::Current& c)
//	throw (::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError)
{
	RLock sync(*this);

	uint32 diskFree, diskTotal;
	ContentStoreImpl::getStorageSpace(_store, diskFree, diskTotal, mountPath.c_str());

	if (!diskTotal)
	{
		MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("getCapacity() from storage free=%d, total=%d, use the cached volume info instead"), 
			diskFree, diskTotal);
		
		if (_store._volumeInfoCache.get(ident.name, diskFree, diskTotal))
			MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("getCapacity() from cache free=%d, total=%d"), diskFree, diskTotal);
		else
			MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("getCapacity() from cache, no cache saved yet"));
	}
	else
	{
		_store._volumeInfoCache.update(ident.name, diskFree, diskTotal);
	}

	if (quotaSpaceMB >0)
		totalMB = MIN(quotaSpaceMB, diskTotal);
	else
		totalMB = diskTotal;

	freeMB = diskFree;
}

::TianShanIce::StrValues VolumeImpl::listContent(const ::std::string& condition, const ::Ice::Current& c) const
//	throw (::TianShanIce::InvalidStateOfArt)
{
	::TianShanIce::StrValues contents;
	try	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("listContent() enumerates all the contents from database"));
		IdentCollection ContentIds = _store._idxFileOfVol->find(ident);

		for (IdentCollection::iterator it = ContentIds.begin(); it < ContentIds.end(); it++)
		{
			std::string contentName = it->name.substr(ident.name.length()+1);
			if (contentName.empty()) {
				continue;
			}
#ifdef ZQ_OS_MSWIN
			if(PathMatchSpec(contentName.c_str(), condition.c_str())) {
				contents.push_back(contentName);
			}
#else
			if(fnmatch(condition.c_str(), contentName.c_str(), FNM_PATHNAME | FNM_PERIOD | FNM_CASEFOLD) == 0)
				contents.push_back(contentName);
#endif
		}
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listContent() exception occurs: %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listContent() exception occurs: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listContent() unknown exception occurs"));
	}

	std::sort(contents.begin(), contents.end());

	MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("listContent() %d content(s) found"), contents.size());

	return contents;
}

::TianShanIce::Storage::ContentPrx VolumeImpl::openContent(const ::std::string& name, const ::std::string& destinationContentType, bool createIfNotExist, const ::Ice::Current& c)
//	throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError)
{
	::Ice::Identity identContent;
	
	::TianShanIce::Storage::UnivContentPrx contentPrx;
	std::string filename;

	{
		RLock sync(*this);

		if (name.empty() || std::string::npos != name.find_first_of(ILLEGAL_NAME_CHARS "/"))
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Volume, 2001, "vol[%s] openContent() illegal content name[%s] to open"), ident.name.c_str(), name.c_str());

		//::Ice::Identity identContent;
		identContent.name = ident.name + LOGIC_FNSEPS + name;
		identContent.category = DBFILENAME_Content;

		filename = mountPath + name;

		if (!ContentStoreImpl::validateMainFileName(_store, filename, destinationContentType))
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Volume, 2002, "vol[%s] openContent() failed at content type validation: filename[%s] type[%s]"), ident.name.c_str(), filename.c_str(), destinationContentType.c_str());

		// step 1. check the db if the content exists
		//::TianShanIce::Storage::UnivContentPrx contentPrx;
		try
		{
			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("openContent() content[%s] openning the db record, createIfNotExist[%c]"), identContent.name.c_str(), createIfNotExist?'T':'F');
			// must use checkedCast here to confirm record exists
			contentPrx = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_store._adapter->createProxy(identContent));
		}
		catch (...)
		{
			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("openContent() content[%s] does not pre-exist in the database"), identContent.name.c_str());
		}
	}

	// step 2. test and create the content record
	if (!contentPrx)
	{
		// step 2.1. test if the file has already been on the disk
		bool bInFS = false;
		if (!createIfNotExist)
		{
			try
			{
				uint64 uRet =  ContentStoreImpl::checkResidentialStatus(_store, RSDFLAG(frfResidential), NULL, identContent.name, filename);
				if (uRet & RSDFLAG(frfResidential))
					bInFS = true;
			}
			catch (...)
			{
				MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("openContent() content[%s] check residential caught exception"), identContent.name.c_str());
			}
		}
		
		// step 2.2 create the content object
		if (bInFS || createIfNotExist)
		{
			MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("openContent() creating content[%s] per bInFS[%s] or createIfNotExist[%s]"), identContent.name.c_str(), bInFS?"true":"false", createIfNotExist?"true":"false");
			contentPrx = createContent(identContent, bInFS && !createIfNotExist, "", c);
		}
	}

	// step 3. populate the attributes if database is out of date
	if (contentPrx)
	{
		try {
			if (contentPrx->isDirty())
			{
				MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("openContent() content[%s] populate the attributes since database is out-of-date than filesystem"), name.c_str());
				contentPrx->populateAttrsFromFilesystem();
			}
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("openContent() content[%s] exception[%s], %s"), name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("openContent() content[%s] exception[%s]"), name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("openContent() content[%s] unknown exception"), name.c_str());
		}
	}

	return ::TianShanIce::Storage::ContentPrx::uncheckedCast(contentPrx);
}

::Ice::Identity VolumeImpl::getIdent(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident;
}

::std::string VolumeImpl::getMountPath(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return mountPath;
}

::TianShanIce::Storage::VolumeInfo VolumeImpl::getInfo(const ::Ice::Current& c) const
{
	RLock sync(*this);
	::TianShanIce::Storage::VolumeInfo info;
	info.name = ident.name;
	info.isVirtual = isVirtual;
	info.quotaSpaceMB = quotaSpaceMB;
	info.metaData = metaData;

	char buf[32];
	info.metaData.insert(::TianShanIce::Properties::value_type(SYS_PROP(MountPath), mountPath));
	info.metaData.insert(::TianShanIce::Properties::value_type(SYS_PROP(StampCreated), ::ZQTianShan::TimeToUTC(stampCreated, buf, sizeof(buf)-2)));

	return info;
}

class reverseLock
{
public:
#if ICE_INT_VERSION / 100 >= 306
	reverseLock( IceUtil::LockT<IceUtil::RecMutex>& locker) :mLocker(locker)
	{
		mLocker.release();
	}
#else
	reverseLock( IceUtil::WLockT<IceUtil::RWRecMutex>& locker) :mLocker(locker)
	{
		mLocker.release();
	}
#endif
	~reverseLock()
	{
		mLocker.acquire();
	}
private:
#if ICE_INT_VERSION / 100 >= 306
	IceUtil::LockT<IceUtil::RecMutex>& mLocker;
#else
	IceUtil::WLockT<IceUtil::RWRecMutex>& mLocker;
#endif
};

::TianShanIce::Storage::UnivContentPrx VolumeImpl::createContent(const ::Ice::Identity& identContent, bool fromFsOrphan, const ::std::string& stampLastFileWriteI, const ::Ice::Current& c)
{
	WLock sync(*this);
	::TianShanIce::Storage::UnivContentPrx contentPrx;
	std::string	stampLastFileWrite =  stampLastFileWriteI;
	std::string createSignature = invokeSignature(c);

	if (fromFsOrphan && stampLastFileWrite.empty())
	{
		char buf[64];
		const char* stamp = ZQTianShan::TimeToUTC(ZQTianShan::now(), buf, sizeof(buf) -2);
		stampLastFileWrite = stamp ? stamp : "";
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() content[%s] fileOrphan, force stampLastFileWrite to [%s]"), identContent.name.c_str(), stampLastFileWrite.c_str());
	}

	try
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() content[%s] sig[%s]"), identContent.name.c_str(), createSignature.c_str());
		// must not use uncheckedCast here
		contentPrx = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_store._adapter->createProxy(identContent));

		if (contentPrx && fromFsOrphan)
		{
			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() content[%s] force to restore attributes from file system"), identContent.name.c_str());

			reverseLock reverseLocker(sync);
			contentPrx->OnRestore(stampLastFileWrite);
		}
	}
	catch (const ::Ice::ObjectNotExistException&)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() content[%s] does not pre-exist in the database"), identContent.name.c_str());
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("createContent() content[%s] touch DB record caught exception[%s]: %s"), identContent.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("createContent() content[%s] touch DB record caught exception[%s]"), identContent.name.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() content[%s] touch DB record caught exception"), identContent.name.c_str());
	}

	if (contentPrx)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() content[%s] created by other thread, return instantly"), identContent.name.c_str());
		return contentPrx;
	}

	try {
		ContentImpl::Ptr content  = new ContentImpl(_store);
		content->ident            = identContent;
		content->identVolume      = ident;
		content->stampCreated     = now();
		content->stampProvisioned = content->stampLastUpdated = 0;
		content->fromFsOrphan     = fromFsOrphan;
		content->state            = ::TianShanIce::Storage::csNotProvisioned;
		content->expiration       = content->stampProvisionSetup = 0;
		content->bDirtyAttrs      = false;

		MAPSET(::TianShanIce::Properties, content->metaData, SYS_PROP(VolumeMountPath), mountPath);
		MAPSET(::TianShanIce::Properties, content->metaData, SYS_PROP(CreateSignature), createSignature);

		if (fromFsOrphan)
			content->bDirtyAttrs  = true;

		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() content[%s] add into the database"), identContent.name.c_str());
		_store._eContent->add(content, content->ident);
		contentPrx = IdentityToObjEnv(_store, UnivContent, content->ident);
		if (contentPrx)
		{
#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			MOEVENT(EventFMT(_store._netId.c_str(), Content, Created, 0, "Content[%s] vol[%s] name[%s] is created"), identContent.name.c_str(), ident.name.c_str(), content->_name().c_str());
			_store.OnContentCreated(identContent);

			{
				reverseLock reverseLocker(sync);			
				contentPrx->OnRestore(stampLastFileWrite);
			}

			ZQ::common::MutexGuard gd(_store._lockReplicaSubscriberMap);
			if (_store._replicaSubscriberMap.end() != _store._replicaSubscriberMap.find(_store._prxstrMasterReplica))
			{
				::TianShanIce::Properties params;
				MAPSET(::TianShanIce::Properties, params, SYS_PROP(Volume), ident.name);
				MAPSET(::TianShanIce::Properties, params, SYS_PROP(Content), identContent.name);
				MAPSET(::TianShanIce::Properties, params, SYS_PROP(CreateSignature), createSignature);
				MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("createContent() need to forward the create event to the master replica[%s]"), _store._prxstrMasterReplica.c_str());
				(new ForwardEventToMasterCmd(_store, _store._replicaSubscriberMap[_store._prxstrMasterReplica], identContent.name, ::TianShanIce::Storage::fseFileCreated, params))->execute();
			}
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("createContent() access content[%s] exception[%s]: %s"), identContent.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("createContent() access content[%s] exception[%s]"), identContent.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("createContent() access content[%s] unknown exception"), identContent.name.c_str());
	}

	// if there is no cluster-ed ContentStores or self is NOT the aggregated ContentStore instance, return directly
	if (!_store._storeAggregate)
		return contentPrx; 

	ContentStoreImpl::NodeReplicaQueue nrqueue;
	_store.buildNodeReplicaQueue(nrqueue);
	
	if (nrqueue.empty())
		return contentPrx; 

	// self appears as a master aggregator of a number of slaves, make sure to create the content records on all the slaves
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "master calling (%d) slaves to create content replica"), nrqueue.size());			
	for(; !nrqueue.empty(); nrqueue.pop())
	{
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{
			std::string storereplicstr = _store._adapter->getCommunicator()->proxyToString(replica.obj);
			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("creating content replica[%s] on slave[%s]"), identContent.name.c_str(), storereplicstr.c_str());			
			::TianShanIce::Storage::ContentStoreExPrx nodeStore = ::TianShanIce::Storage::ContentStoreExPrx::uncheckedCast(replica.obj);
			std::string tmpstr = identContent.name;
			size_t pos = tmpstr.find(LOGIC_FNSEPC, 1);
			::TianShanIce::Storage::FolderPrx folder = ::TianShanIce::Storage::FolderPrx::uncheckedCast(nodeStore->openVolume(tmpstr.substr(1, pos-1)));
			tmpstr = tmpstr.substr(pos+1);

			for (size_t pos = tmpstr.find(LOGIC_FNSEPC); std::string::npos != pos; pos = tmpstr.find(LOGIC_FNSEPC))
			{
				folder = folder->openSubFolder(tmpstr.substr(0, pos), true, 0);
				tmpstr = tmpstr.substr(pos+1);
			}

			folder->openContent(tmpstr, "", true);
			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("creating content replica[%s] on slave[%s]"), identContent.name.c_str(), storereplicstr.c_str());
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("createContent(%s) on replica[%s] caught exception[%s]: %s"), 
				identContent.name.c_str(), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());			
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("createContent(%s) on replica[%s] caught exception[%s]"),
				identContent.name.c_str(), replica.replicaId.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("createContent(%s) on replica[%s] caught unknown exception"),
				identContent.name.c_str(), replica.replicaId.c_str());
		}
	}

	return contentPrx;
}

bool VolumeImpl::deleteContent(const ::Ice::Identity& identContent, const ::Ice::Current& c)
{
	try {
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("deleteContent() content[%s]"), identContent.name.c_str());
		::TianShanIce::Storage::UnivContentPrx content = IdentityToObjEnv(_store, UnivContent, identContent);
		content->destroy();
		return true;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("deleteContent() access content[%s] exception[%s]: %s"), identContent.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("deleteContent() access content[%s] exception[%s]"), identContent.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("deleteContent() access content[%s] unknown exception"), identContent.name.c_str());
	}

	return false;
}

void VolumeImpl::destroy(const ::Ice::Current& c)
//	throw (::TianShanIce::NotSupported, ::TianShanIce::InvalidStateOfArt)
{
	{
	RLock sync(*this);

	if (!isVirtual)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(Volume, 3000, "destroy() vol[%s] is not a mounted volume, can not be destroyed"), ident.name.c_str());

	std::string signature = invokeSignature(c);
	try	
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("destroy() sig[%s] checking if the volume is empty"), signature.c_str());

		IdentCollection ChildrenIds = _store._idxFileOfVol->find(ident);
		if (ChildrenIds.size() >0)
			ZQTianShan::_IceThrow <TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Volume, 3001, "destroy() vol[%s] has %d contents, cannot be destroyed"), ident.name.c_str(), ChildrenIds.size());

		ChildrenIds = _store._idxChildVolume->find(ident.name);
		if (ChildrenIds.size() >0)
			ZQTianShan::_IceThrow <TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Volume, 3001, "destroy() vol[%s] has %d sub-volumes, cannot be destroyed"), ident.name.c_str(),  ChildrenIds.size());

		MAPSET(::TianShanIce::Properties, metaData, SYS_PROP(DestroySignature), signature);
	}
	catch(::TianShanIce::InvalidStateOfArt& ex)
	{
		ex.ice_throw();
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("destroy() look for children caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("destroy() look for children caught unknown exception"));
	}
	}
	
	WLock sync(*this);
	try
	{
		MAPSET(::TianShanIce::Properties, metaData, SYS_PROP(DestroySignature), invokeSignature(c));
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("destroy() ask portal to cleanup mountPath[%s]"), mountPath.c_str());

		if (!ContentStoreImpl::deletePathOfVolume(_store, mountPath))
			ZQTianShan::_IceThrow <TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(Volume, 3002, "destroy() vol[%s] failed to cleanup mountPath[%s]"), ident.name.c_str(), mountPath.c_str());

		_store._eVolume->remove(ident);

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
		MOEVENT(VolEventFMT(Destroyed, 1, ""));
		_store.OnContentDestroyed(ident);

		MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("destroy() volume removed from DB"));
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT( "destroy() object already gone from the container, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("destroy() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("destroy() caught unknown exception"));
	}
}

void VolumeImpl::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow   = now();
	long        nextInterv = MIN_UPDATE_INTERVAL*4;
	bool        bSyncWithFSOnPending = false;

	do {

		// read from the metadata
		::Ice::Long stampLastSync = 0;
		{
			RLock sync(*this);
			::TianShanIce::Properties::iterator it = metaData.find(SYS_PROP(PendingFileSystemSync));
			if (metaData.end() !=it && 0 != atoi(it->second.c_str()))
				bSyncWithFSOnPending = true;

			it = metaData.find(SYS_PROP(StampLastSyncWithFileSystem));
			if (metaData.end() !=it)
				stampLastSync = ISO8601ToTime(it->second.c_str());
		}

		if (bSyncWithFSOnPending)
		{
			if (stampNow < _store._stampStarted + MIN_UPDATE_INTERVAL*4)
			{
				// yield the sync if the service hasn't been started for enough time
				nextInterv = (long) (_store._stampStarted + MIN_UPDATE_INTERVAL*4 - stampNow);
				break;
			}

			if (stampLastSync > _store._stampStarted)
			{
				// temprarily ensure only once on the auto-sync: no need to sync again
				WLock sync(*this);
				metaData.erase(SYS_PROP(PendingFileSystemSync));
				break;
			}

			// kick off the sync command
			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("OnTimer() necessary to kick off a sync with the file system"));
			(new SyncFSCmd(_store, ident))->execute();

			WLock sync(*this);
			metaData.erase(SYS_PROP(PendingFileSystemSync));

			break;
		} // end of _bSyncWithFSOnPending

		if (_store._warningFreeSpacePercent >0)
		{
			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("OnTimer() checking the free space of the volume"));
			::Ice::Long freeMB, totalMB, tmpMB=0;
			getCapacity(freeMB, totalMB, c);

			if (totalMB <=0 || freeMB >=totalMB)
			{
				MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("OnTimer() ingnore invalid return of getCapacity(): freeMB[%lld] totalMB[%lld]"), freeMB, totalMB);
				break;
			}

			if (freeMB > totalMB * _store._warningFreeSpacePercent /100)
				break; // no need to warning

			// speed up the checking if free space is at a warning state
			nextInterv = MIN_UPDATE_INTERVAL;

			if (_store._stepFreeSpacePercent >= _store._warningFreeSpacePercent || _store._stepFreeSpacePercent <=0)
				_store._stepFreeSpacePercent = (_store._warningFreeSpacePercent +9) / 10;

			{
				RLock sync(*this);
				::TianShanIce::Properties::iterator it = metaData.find(SYS_PROP(LastWarnFreeSpaceMB));
				if (metaData.end() !=it)
					tmpMB = _atoi64(it->second.c_str());
			}

			tmpMB -=freeMB;
			if (tmpMB <0)
				tmpMB = -tmpMB;

			if (tmpMB < (totalMB * _store._stepFreeSpacePercent /100))
				break; // no need to warning if there is no significent changes

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			MOEVENT(VolEventFMT(FreeSpace, 5, " free space below %d%%: freeMB[%lld] totalMB[%lld]"), _store._warningFreeSpacePercent, freeMB, totalMB);
			
			// update the metadata
			char buf[100];
			snprintf(buf, sizeof(buf)-2, "%lld", freeMB);
			WLock sync(*this);
			MAPSET(TianShanIce::Properties, metaData, SYS_PROP(LastWarnFreeSpaceMB), buf);
		} // end of _warningFreeSpacePercent

	} while (0);

	_store._watchDog.watch(ident, nextInterv);
}
		

typedef struct _LocalContentInfo
{
	::TianShanIce::Storage::ContentState state;
	::Ice::Identity ident;
	::Ice::Identity identVol;
	::std::string name;
	//static bool less(const ContentInfo& A, const ContentInfo& B) { return (strcmp(A.name(), B.name()) <0); }
} LocalContentInfo;

typedef std::vector< LocalContentInfo > LocalContentInfos;

struct LessContentInfo
{
     bool operator()(const LocalContentInfo& A, const LocalContentInfo& B)
     {
          return (strcmp(A.ident.name.c_str(), B.ident.name.c_str()) <0);
     }
};

struct LessFileInfo
{
     bool operator()(const ContentStoreImpl::FileInfo& A, const ContentStoreImpl::FileInfo& B)
     {
          return (strcmp(A.filename.c_str(), B.filename.c_str()) <0);
     }
};

void VolumeImpl::syncWithFileSystem(const ::Ice::Current& c)
{
	//synchronize subfolders
	{
		TianShanIce::Storage::FolderInfos folderV;
        try
        {
            folderV = listSubFolders();
            for (TianShanIce::Storage::FolderInfos::iterator itV = folderV.begin(); itV < folderV.end(); itV ++)
            {
                ::Ice::Identity identF;
                identF.name = itV->name;
                identF.category = DBFILENAME_Volume;

                (new SyncFSCmd(_store, identF))->execute();
            }
        }
        catch(const ::Ice::Exception& ex)
        {
            MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() failed to listSubFolder dir[%s], caught exception[%s]"), mountPath.c_str(), ex.ice_name().c_str());
        }
        catch(...)
        {
            MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() failed to listSubFolder dir[%s], caught unknown exception"), mountPath.c_str());
        }
	
	}

	MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("syncWithFileSystem() listing all the files on the filesystem: dir[%s]"), mountPath.c_str());
	::Ice::Long stamp0 = ZQTianShan::now();
	ContentStoreImpl::FileInfos filesOnFs = _store.listMainFiles(_store, _store._storeAggregate?ident.name.c_str():mountPath.c_str());
	::Ice::Long stamp1 = ZQTianShan::now();
	std::sort(filesOnFs.begin(), filesOnFs.end(), LessFileInfo());
	::Ice::Long stamp2 = ZQTianShan::now();
	MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("syncWithFileSystem() %d file(s) found on the filesystem: dir[%s] portal took [%lld]ms, preparation took [%lld]ms"), filesOnFs.size(), mountPath.c_str(), stamp1- stamp0, stamp2-stamp1);

	if (::TianShanIce::stOutOfService == _store._serviceState)
		return;

	LocalContentInfos contents;
	try	
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("syncWithFileSystem() enumerating all the contents of the volume from database"));

		IdentCollection ContentIds = _store._idxFileOfVol->find(ident);
		::Ice::Long stamp3 = ZQTianShan::now();

		for (IdentCollection::iterator it = ContentIds.begin(); it < ContentIds.end(); it++)
		{
			if (::TianShanIce::stOutOfService == _store._serviceState)
				return;

			LocalContentInfo contentinfo;
			contentinfo.identVol = ident;
			try {
				contentinfo.ident = *it;
				::TianShanIce::Storage::UnivContentPrx content = IdentityToObjEnv(_store, UnivContent, contentinfo.ident);
				contentinfo.state = content->getState();
				contentinfo.name = content->getName();
				contents.push_back(contentinfo);
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("syncWithFileSystem() access content[%s] exception[%s], %s"), contentinfo.ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("syncWithFileSystem() access content[%s] exception[%s]"), contentinfo.ident.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("syncWithFileSystem() access content[%s] unknown exception"), contentinfo.ident.name.c_str());
			}
		}

		::Ice::Long stamp4 = ZQTianShan::now();
		std::sort(contents.begin(), contents.end(), LessContentInfo());
		MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("syncWithFileSystem() %d content(s) found in database, listing took [%lld/%lld/%lld]ms,"), contents.size(), stamp3-stamp2, stamp4-stamp3, ZQTianShan::now()-stamp4);
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("syncWithFileSystem() enumerates all the contents exception[%s], %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("syncWithFileSystem() enumerates all the contents exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("syncWithFileSystem() enumerates all the contents unknown exception"));
	}

	if (0 == filesOnFs.size() && contents.size() >0)
	{
		MOLOG(ZQ::common::Log::L_WARNING, VolLOGFMT("syncWithFileSystem() 0 file found but DB is not empty, give up sync because this status is very questioning. You can enable sync by ingesting a new content to ensure file system is not empty"));
		_store._watchDog.watch(_store._localId, UNATTENDED_TIMEOUT);
		return;
	}

	::TianShanIce::Storage::UnivContentPrx contentPrx;

	MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("syncWithFileSystem() comparing files and DB records"));
	ContentStoreImpl::FileInfos::iterator itFile = filesOnFs.begin();
	LocalContentInfos::iterator itContent = contents.begin();
	
	std::string stampNow;
	{
		char buf[64];
		ZQTianShan::TimeToUTC(ZQTianShan::now(), buf, sizeof(buf) -2);
		stampNow = buf;
		MAPSET(::TianShanIce::Properties, metaData, SYS_PROP(StampLastSyncWithFileSystem), stampNow);
	}

	while (itFile < filesOnFs.end())
	{
		if (::TianShanIce::stOutOfService == _store._serviceState)
			return;

		if (itFile->stampLastWrite.empty())
			itFile->stampLastWrite = stampNow;

		if (itContent == contents.end() ||  itFile->filename.compare(itContent->name) <0)
		{
			// only exists on filesystem
			try {
				if (0 != ((FLAG(::TianShanIce::Storage::fseFileCreated) | FLAG(::TianShanIce::Storage::fseFileRenamed)) & _store._fileEventFlags))
				{
					// the store is interested with the file created on filesystem
					MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("syncWithFileSystem() no content associated for file[%s], create new content record for it"), itFile->filename.c_str());
					contentPrx = createContent(ContentStoreImpl::toContentIdent(ident.name + LOGIC_FNSEPS + itFile->filename), true, itFile->stampLastWrite, ::Ice::Current());
				}
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() add content[%s] exception[%s], %s"), itFile->filename.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() add content[%s] exception[%s]"), itFile->filename.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() add content[%s] unknown exception"), itFile->filename.c_str());
			}

			itFile++;
			continue;
		}

		if (::TianShanIce::stOutOfService == _store._serviceState)
			return;

		if (itContent < contents.end())
		{
			if (0 == itFile->filename.compare(itContent->name))
			{
				// exists on both filesystem and database
				try {
					contentPrx = IdentityToObjEnv(_store, UnivContent, itContent->ident);
					contentPrx->OnRestore(itFile->stampLastWrite);
				}
				catch (const TianShanIce::BaseException& ex) 
				{
					MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] exception[%s], %s"), itContent->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				}
				catch (const ::Ice::Exception& ex)
				{
					MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] exception[%s]"), itContent->ident.name.c_str(), ex.ice_name().c_str());
				}
				catch(...)
				{
					MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] unknown exception"), itContent->ident.name.c_str());
				}

				itFile++;
			}
			else
			{
				// only exists in Database
				try {
					MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("syncWithFileSystem() restoring a meta-content [%s] to under watching"), itContent->ident.name.c_str());
					contentPrx = IdentityToObjEnv(_store, UnivContent, itContent->ident);
					contentPrx->OnRestore(""); // call with stampLastWrite="" to indicate the inexistence of content on the filesystem
				}
				catch (const TianShanIce::BaseException& ex) 
				{
					MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] exception[%s], %s"), itContent->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				}
				catch (const ::Ice::Exception& ex)
				{
					MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] exception[%s]"), itContent->ident.name.c_str(), ex.ice_name().c_str());
				}
				catch(...)
				{
					MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] unknown exception"), itContent->ident.name.c_str());
				}
			}

			itContent++;
			continue;
		}
	}

	for (; itContent < contents.end(); itContent++)
	{
		// only exists in Database
		if (::TianShanIce::stOutOfService == _store._serviceState)
			return;

		try {
			MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("syncWithFileSystem() restoring a meta-content [%s] to under watching"), itContent->ident.name.c_str());
			contentPrx = IdentityToObjEnv(_store, UnivContent, itContent->ident);
			contentPrx->OnRestore(""); // call with stampLastWrite="" to indicate the inexistence of content on the filesystem
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] exception[%s], %s"), itContent->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] exception[%s]"), itContent->ident.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("syncWithFileSystem() restore content[%s] unknown exception"), itContent->ident.name.c_str());
		}
	}

	_store._watchDog.watch(_store._localId, 0);
	
	MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("syncWithFileSystem() finished scanning"));
}

void VolumeImpl::listContents_async(const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, const ::Ice::Current& c) const
//	throw (::TianShanIce::ServerError)
{
	try {
		(new ListContentsCmd(_store, amdCB, ident, metaDataNames, startName, maxCount))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listContents_async() failed to generate ListContentsCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("Volume", 501, "failed to generate ListContentsCmd"));
	}
}

::TianShanIce::Storage::FolderPrx VolumeImpl::openSubFolder(const ::std::string& subname, bool createIfNotExist, ::Ice::Long quotaSpaceMB, const ::Ice::Current& c)
{
	if (subname.empty() || std::string::npos != subname.find_first_of(ILLEGAL_NAME_CHARS))
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Volume, 7005, "openSubFolder() illegal subname[%s] to open volume"), subname.c_str());

	::Ice::Identity identChild;
	identChild.name = ident.name + LOGIC_FNSEPS + subname;
	identChild.category = DBFILENAME_Volume;

	::TianShanIce::Storage::VolumePrx volume;

	try	{

		volume = IdentityToObjEnv(_store, VolumeEx, identChild);
	}
	catch(const Ice::ObjectNotExistException)
	{
		MOLOG(ZQ::common::Log::L_INFO, VolLOGFMT("openSubFolder() name[%s] not exist"), identChild.name.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("openSubFolder() name[%s] caught exception[%s]"), identChild.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("openSubFolder() name[%s] caught unknown exception"), identChild.name.c_str());
	}

	if (createIfNotExist && !volume)
	{
		std::string pathOfVolume = ContentStoreImpl::fixupPathname(_store, mountPath + subname);
		if (!ContentStoreImpl::createPathOfVolume(_store, pathOfVolume, ident.name))
			ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Volume, 7004, "openSubFolder() failed to create the path-of-volume[%s] on the contentstore portal"), pathOfVolume.c_str());

		if(pathOfVolume[pathOfVolume.length()-1] != FNSEPC)
			pathOfVolume += FNSEPS;
		try {
			VolumeImpl::Ptr vol   = new VolumeImpl(_store);
			vol->ident            = identChild;
			vol->parentVolName    = ident.name;

			vol->isVirtual		  = true;
			vol->mountPath        = pathOfVolume;
			vol->quotaSpaceMB     = quotaSpaceMB >0 ? quotaSpaceMB : -1;
			vol->stampCreated     = now();

			MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("openSubFolder() mountpath[%s] adding into the database"), pathOfVolume.c_str());
			_store._eVolume->add(vol, vol->ident);
			volume = IdentityToObjEnv(_store, VolumeEx, identChild);

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			MOEVENT(VolEventFMT(Created, 0, "sub-volume[%s] is created, mountPath[%s], quotaSpaceMB[%lld]"), vol->ident.name.c_str(), vol->mountPath.c_str(), vol->quotaSpaceMB);
			_store.OnSubVolumeCreated(vol->ident);
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("openSubFolder() add new record caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("openSubFolder() add new record caught exception[%s]"), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("openSubFolder() add new record caught unknown exception"));
		}
	}

	return volume;
}

::TianShanIce::Storage::FolderPrx VolumeImpl::parent(const ::Ice::Current& c) const
{
	::Ice::Identity identParent;
	identParent.name = parentVolName;
	identParent.category = DBFILENAME_Volume;
	::TianShanIce::Storage::VolumePrx parentVolume;

	try	{
		parentVolume = IdentityToObjEnv(_store, VolumeEx, identParent);
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("parent() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("parent() caught unknown exception"));
	}

	return parentVolume;
}

TianShanIce::Storage::FolderInfos VolumeImpl::listSubFolders( const ::Ice::Current& c /*= ::Ice::Current()*/ ) const
{
	TianShanIce::Storage::FolderInfos folders;

    std::string strFrom = ident.name + LOGIC_FNSEPS;
    MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("listSubFolders() from parent[%s]"), strFrom.c_str());

    IdentCollection Idents;
    try {
        ::Freeze::EvictorIteratorPtr itptr = _store._eVolume->getIterator("", 10);
        while (itptr && itptr->hasNext())
            Idents.push_back(itptr->next());

//        MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("listSubFolders() [%d] volumes found"), Idents.size());

        // build up the content info collection based on the search result
        for (IdentCollection::iterator it= Idents.begin(); it < Idents.end(); it++)
        {
            try
            {
                if (0 != it->name.compare(0, strFrom.length(), strFrom) || it->name.rfind(LOGIC_FNSEPS) > strFrom.length())
                    continue; // skip unmatched and only deep 1
				
				struct TianShanIce::Storage::FolderInfo fInfo;
                ::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, *it);
                ::TianShanIce::Storage::VolumeInfo volInfo  = volume->getInfo();
				fInfo.name = volInfo.name;
				fInfo.metaData = volInfo.metaData;
                if (volInfo.isVirtual)
                    folders.push_back(fInfo);
            }
			catch(const Ice::Exception& ex)
			{
        		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listSubFolders() caught exception[%s]"), ex.ice_name().c_str());
			}
            catch (...) 
			{
        		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listSubFolders() caught unknown exception"));
			}
        }

        MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("listSubFolders() get information of [%d] matched volume(s) as the result"), folders.size());
    }
    catch(const ::Ice::Exception& ex)
    {
        MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listSubFolders() caught exception[%s]"), ex.ice_name().c_str());
    }
    catch(...)
    {
        MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("listSubFolders() caught unknown exception"));
    }

	return folders;
}

void VolumeImpl::cachedListContents_async(const ::TianShanIce::Storage::AMD_VolumeEx_cachedListContentsPtr& amdCB, ::Ice::Int timeout, const ::Ice::Current& c) const
{
	try {
		(new CachedListContentsCmd(_store, amdCB, ident, timeout))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, VolLOGFMT("cachedListContents_async() failed to generate CachedListContentsCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("Volume", 501, "failed to generate CachedListContentsCmd"));
	}
}

void VolumeImpl::setMetaData(const ::TianShanIce::Properties& newMetaData, const ::Ice::Current& c)
{
	WLock sync(*this);
	std::string setStr;
	for (::TianShanIce::Properties::const_iterator it = newMetaData.begin(); it != newMetaData.end(); it++)
	{
		if (it->first.empty())
			continue;

		if (metaData.end() == metaData.find(it->first))
			metaData.insert(*it);
		else metaData[it->first] = it->second;

		setStr += it->first + "[" + it->second + "]; ";
	}

	MOLOG(ZQ::common::Log::L_DEBUG, VolLOGFMT("setMetaData() %s"), setStr.c_str());
}


// -----------------------------
// class CachedContentListImpl
// -----------------------------
CachedContentListImpl::CachedContentListImpl(ContentStoreImpl& store, ::Ice::Int timeout)
: _store(store), _expiration(0)
{
}

void CachedContentListImpl::next_async(const ::TianShanIce::Storage::AMD_CachedContentList_nextPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, ::Ice::Int renewMs, const ::Ice::Current& c)
{
	if (renewMs >0)
	{
		if (renewMs < 1000)
			renewMs = renewMs;

		WLock sync(*this);
		_expiration = ZQTianShan::now() + renewMs;
		_store._watchDog.watch(_ident, renewMs);
	}

	try {
		(new CachedContentListReadResultCmd(_store, *this, amdCB, metaDataNames, startName, maxCount))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CachedContentList, "listContents_async() failed to generate CachedContentListReadResultCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("Volume", 501, "failed to generate CachedContentListReadResultCmd"));
	}
}

void CachedContentListImpl::destory(const ::Ice::Current& c)
{
	try {
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CachedContentList, "destory() cachedList[%s] of vol[%s] expired, removing"), _ident.name.c_str(), _identVolume.name.c_str());
		_store._adapter->remove(_ident);
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CachedContentList, "destory() cachedList[%s] of vol[%s] caught exception"), _ident.name.c_str(), _identVolume.name.c_str());
	}
}

void CachedContentListImpl::OnTimer(const ::Ice::Current& c)
{	
	RLock sync(*this);
	::Ice::Long lifeleft = _expiration - ZQTianShan::now();
	if (lifeleft >0)
	{
		_store._watchDog.watch(_ident, (::Ice::Int)lifeleft);
		return;
	}

	destory(c);
}

::Ice::Int CachedContentListImpl::size(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return _identResults.size();
}

::Ice::Int CachedContentListImpl::left(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return _identResults.end() - _itResults;
}

}} // namespace
