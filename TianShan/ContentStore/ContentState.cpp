// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Poscontention, use,
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
// Ident : $Id: ContentState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentState.cpp $
// 
// 25    10/19/16 2:57p Hui.shao
// 
// 24    5/17/16 5:22p Hui.shao
// 
// 23    4/18/16 5:51p Hui.shao
// 
// 22    12/03/14 5:15p Li.huang
// 
// 21    9/04/14 5:32p Hui.shao
// 
// 20    1/21/14 1:33p Build
// 
// 19    1/21/14 1:27p Hui.shao
// 
// 18    1/16/14 12:47p Hui.shao
// 
// 17    1/15/14 10:41a Hui.shao
// per bug#18785, for cluster mode, it is needed to ensure the deletion
// gets completed as the destroy2(true) fired by this instance to slave is
// async call
// 
// 16    1/02/14 2:43p Hui.shao
// bug#18785 kick off slave deleting replicas when enter OutService
// 
// 15    12/12/13 1:43p Hui.shao
// %lld
// 
// 14    3/05/13 4:33p Hui.shao
// 
// 13    12/19/12 4:48p Hui.shao
// 
// 12    9/18/12 2:07p Hui.shao
// enh#16995 - Serve the cached copy that is catching up the PWE copy on
// source storage
// 
// 11    9/12/12 2:36p Hui.shao
// 
// 10    9/12/12 11:29a Hui.shao
// 
// 9     4/11/11 4:43p Hui.shao
// added mono-provision test
// 
// 8     10-11-23 14:53 Hongquan.zhang
// update bDirtyAttribute to true after successfully checkResidential
// 
// 6     10-11-22 17:35 Hui.shao
// 
// 5     10-11-22 11:20 Hui.shao
// 
// 2     10-11-17 18:17 Hui.shao
// no need to increase attr check intervals if the service just started
// 
// 41    10-11-02 12:12 Hui.shao
// fix of bug#12018
// 
// 40    10-10-27 14:58 Hui.shao
// double checked the mergie the changes in V1.10 since 3/19/2009
// 
// 38    10-10-26 13:30 Hui.shao
// confirmed the merge from V1.10
// 
// 37    10-08-11 17:50 Build
// 
// 36    10-08-11 14:47 Hui.shao
// per NGOD-512, increase interval if attr-populating has error occurs 
// 
// 35    10-07-29 18:54 Hui.shao
// 
// 34    09-12-22 14:06 Jie.zhang
// merge from TianShan1.10
// 
// 33    09-09-07 14:04 Hui.shao
// added invoke signature and update content delete strategy
// 
// 32    09-09-03 13:24 Hui.shao
// separate the handling on interested file events
// 
// 31    09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 30    09-06-12 11:18 Hui.shao
// 
// 29    09-06-12 10:39 Hui.shao
// adjusted some default timeout values
// 
// 27    09-06-11 15:50 Hui.shao
// moved the enlarged wait to _populateAttrFromFs()
// 
// 25    09-06-05 10:52 Hui.shao
// fix the provision session checking at the state of provioning
// 
// 24    09-05-07 16:55 Fei.huang
// * fix print specifier for 64bit value %lld -> %lld
// 
// 23    09-02-20 18:40 Hongquan.zhang
// 
// 22    09-02-05 13:53 Hui.shao
// added the event entires from the ContentStoreLib to the impls derived
// from the base ContentStoreImpl
// 
// 21    08-12-29 19:32 Hui.shao
// fixed the missed case that (file doesn't exist but provisioning is well
// out of window)
// 
// 20    08-12-11 12:31 Hui.shao
// 
// 19    08-12-10 18:32 Jie.zhang
// 
// 18    08-11-27 16:13 Hui.shao
// fixed the message reference from zq::common::exception
// 
// 17    08-11-24 12:44 Hui.shao
// Provisioning::OnTimer() test scheduleEnd and provision session
// 
// 16    08-11-24 12:29 Jie.zhang
// add a parameter on checkResidencialStatus
// 
// 15    08-11-18 14:46 Yixin.tian
// modify can compile Linux OS
// 
// 14    08-11-15 17:59 Hui.shao
// 
// 13    08-11-15 17:39 Hui.shao
// 
// 12    08-11-15 15:20 Hui.shao
// 
// 11    08-11-15 14:01 Hui.shao
// switching state by checking residential status
// 
// 10    08-11-13 16:29 Jie.zhang
// 
// 9     08-11-13 11:09 Jie.zhang
// remove log
// 
// 8     08-11-12 12:06 Jie.zhang
// add volumePath to populateAttrsFromFile
// 
// 7     08-11-11 12:25 Hui.shao
// retry in MIN_UPDATE_INTERVAL if portal throw exception from
// populateAttributes()
// 
// 6     08-11-10 11:52 Hui.shao
// fixed a mispick for timeout
// 
// 5     08-11-07 11:00 Jie.zhang
// add common log define to unify log using style
// 
// 4     08-11-03 11:36 Hui.shao
// 
// 3     08-10-28 17:57 Hui.shao
// test the last write if it is worthy to populate attributes
// 
// 2     08-10-07 19:56 Hui.shao
// added volume layer
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// 
// 8     08-08-13 12:41 Hui.shao
// 
// 7     08-07-31 18:43 Hui.shao
// restrict on state for provision-related operation
// 
// 6     08-07-31 17:20 Hui.shao
// added the portail enties for provisioning
// 
// 5     08-07-29 12:16 Hui.shao
// added event log as sentry's input
// 
// 4     08-07-21 11:51 Hui.shao
// check in the works of last weekend
// 
// 3     08-07-18 18:52 Hui.shao
// 
// 2     08-07-18 15:12 Hui.shao
// 
// 1     08-07-15 14:19 Hui.shao
// initial check in
// ===========================================================================

#include "ContentState.h"
#include "ContentImpl.h"
#include "ContentCmds.h"

extern "C"
{
#include <stdlib.h>
#include <time.h>


}
#include <stdarg.h>
#include <stdio.h>

using namespace ZQ::common;


#define MOLOG	(_store._log)
#define MOEVENT	(_store._eventlog)


namespace ZQTianShan {
namespace ContentStore {

#define ContentStateLOGFMT(_C, _X) CLOGFMT(_C, "content[%s:%s(%d)] " _X), _content.ident.name.c_str(), ContentStateBase::stateStr(_content.state), _content.state
#define ContentStateEXPFMT(_C, _ERRCODE, _X) EXPFMT(_C, _ERRCODE, "content[%s:%s(%d)] " _X), _content.ident.name.c_str(), ContentStateBase::stateStr(_content.state), _content.state
#define ContentEventFMT(_EVENT, _EVENTCODE, _X) EventFMT(_store._netId.c_str(), Content, _EVENT, _EVENTCODE, "Content[%s] vol[%s] name[%s] " _X), _content.ident.name.c_str(), _content.identVolume.name.c_str(), _content._name().c_str()

// -----------------------------
// class ContentStateBase
// -----------------------------
const char* ContentStateBase::stateStr(const ::TianShanIce::Storage::ContentState state)
{
#define SWITCH_CASE_STATE(_ST)	case ::TianShanIce::Storage::cs##_ST: return #_ST
	switch(state)
	{
		SWITCH_CASE_STATE(NotProvisioned);
		SWITCH_CASE_STATE(Provisioning);
		SWITCH_CASE_STATE(ProvisioningStreamable);
		SWITCH_CASE_STATE(InService);
		SWITCH_CASE_STATE(OutService);
		SWITCH_CASE_STATE(Cleaning);
	default:
		return "<Unknown>";
	}
#undef SWITCH_CASE_STATE
}

::TianShanIce::Storage::ContentState ContentStateBase::stateId(const char* stateStr)
{
#ifdef ZQ_OS_MSWIN
	if (NULL == stateStr)                                   return ::TianShanIce::Storage::csNotProvisioned;
	else if (0 == stricmp("NotProvisioned", stateStr))      return ::TianShanIce::Storage::csNotProvisioned;
	else if (0 == stricmp("Provisioning", stateStr))        return ::TianShanIce::Storage::csProvisioning;
	else if (0 == stricmp("ProvisioningStreamable", stateStr)) return ::TianShanIce::Storage::csProvisioningStreamable;
	else if (0 == stricmp("InService", stateStr))           return ::TianShanIce::Storage::csInService;
	else if (0 == stricmp("OutService", stateStr))          return ::TianShanIce::Storage::csOutService;
	else if (0 == stricmp("Cleaning", stateStr))            return ::TianShanIce::Storage::csCleaning;
#else
	if (NULL == stateStr)                                   return ::TianShanIce::Storage::csNotProvisioned;
	else if (0 == strcasecmp("NotProvisioned", stateStr))      return ::TianShanIce::Storage::csNotProvisioned;
	else if (0 == strcasecmp("Provisioning", stateStr))        return ::TianShanIce::Storage::csProvisioning;
	else if (0 == strcasecmp("ProvisioningStreamable", stateStr)) return ::TianShanIce::Storage::csProvisioningStreamable;
	else if (0 == strcasecmp("InService", stateStr))           return ::TianShanIce::Storage::csInService;
	else if (0 == strcasecmp("OutService", stateStr))          return ::TianShanIce::Storage::csOutService;
	else if (0 == strcasecmp("Cleaning", stateStr))            return ::TianShanIce::Storage::csCleaning;
#endif

	return ::TianShanIce::Storage::csNotProvisioned;
}

ContentStateBase::ContentStateBase(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::Storage::ContentState state)
: _store(store), _content(content), _oldState(content.state), _theState(state)
{
}

void ContentStateBase::_commitState(bool fireEvent, const ::std::string& msg)
{
#if ICE_INT_VERSION / 100 >= 306
	ContentImpl::Lock lock(_content);
#else
	ContentImpl::WLock lock(_content);
#endif
	_content.state = _theState;
	if (_theState == _oldState)
	{
#ifdef _DEBUG
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_commitState() same state, ignore"));
#endif // _DEBUG
		return;
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "committing state change: %s(%d) -> %s(%d)"),
			ContentStateBase::stateStr(_oldState), _oldState, ContentStateBase::stateStr(_theState), _theState);	

	if (!fireEvent)
		return;

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	MOEVENT(ContentEventFMT(StateChanged, 2, "oldState[%s(%d)] newState[%s(%d)] %s"),
		ContentStateBase::stateStr(_oldState), _oldState, ContentStateBase::stateStr(_theState), _theState, msg.c_str());	

	_content.bDirtyAttrs =  true;

	_store.OnContentStateChanged(_content.ident, _oldState, _theState);

	MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(ContentState, "commit state change: %s(%d) -> %s(%d) OK"),
			ContentStateBase::stateStr(_oldState), _oldState, ContentStateBase::stateStr(_theState), _theState);	

	if (!_store._prxstrMasterReplica.empty())
	{
		::TianShanIce::Properties params;
		char tempbuf[64];
		MAPSET(::TianShanIce::Properties, params, SYS_PROP(Volume),          _content.identVolume.name);
		MAPSET(::TianShanIce::Properties, params, SYS_PROP(ContentFullName), _content.ident.name);
		MAPSET(::TianShanIce::Properties, params, SYS_PROP(StateId),         itoa(_content.state, tempbuf, 10));

		ZQ::common::MutexGuard gd(_store._lockReplicaSubscriberMap);
		if (_store._replicaSubscriberMap.end() != _store._replicaSubscriberMap.find(_store._prxstrMasterReplica))
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_commitState() need to forward the modified event to the master replica[%s]"), _store._prxstrMasterReplica.c_str());
			(new ForwardEventToMasterCmd(_store, _store._replicaSubscriberMap[_store._prxstrMasterReplica], _content.ident.name, ::TianShanIce::Storage::fseFileModified, params))->execute();
		}
	}
}

/*
void ContentStateBase::doPopulateFromFs(const ::Ice::Current& c)
{
	::TianShanIce::Storage::ContentState targetState = _populateAttrFromFs();

	if (targetState == _content.state)
		return;

	switch (targetState)
	{
	case ::TianShanIce::Storage::csNotProvisioned:
		// program should not hit here: ContentStateNotProvisioned(_store, _content).enter();
		break; // do nothing, and continue with the initialization steps

	case ::TianShanIce::Storage::csProvisioning:
		ContentStateProvisioning(_store, _content).enter();
		break;
	case ::TianShanIce::Storage::csProvisioningStreamable:
		ContentStateProvisioningStreamable(_store, _content).enter();
		break;
	case ::TianShanIce::Storage::csInService:
		ContentStateInService(_store, _content).enter();
		break;
	case ::TianShanIce::Storage::csOutService:
		ContentStateOutService(_store, _content).enter();
		break;

	case ::TianShanIce::Storage::csCleaning:
		ContentStateCleaning(_store, _content).enter();
		break;

	default:
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (MOLOG, ContentStateEXPFMT(ContentStateBase, 901, "unknown targetState[%d] from the portal, ignore"), targetState);
	}
}
*/
#define IS_EXISTED(FLAGS) (FLAGS & RSDFLAG(frfResidential))
#define IS_WRITTING(FLAGS) (FLAGS & RSDFLAG(frfWriting))
#define IS_READING(FLAGS) (FLAGS & RSDFLAG(frfReading))
#define MISSING_MEM_FILE(FLAGS) (FLAGS & RSDFLAG(frfAbsence))

#define MINIMIZE_FILESIZE		1000

bool ContentStateBase::checkInServiceCondition(uint64 residentialStatusFlags)
{
	if (MISSING_MEM_FILE(residentialStatusFlags) || IS_WRITTING(residentialStatusFlags))
		return false;

	::TianShanIce::Properties::const_iterator it = _content.metaData.find(METADATA_FileSize);
	if (_content.metaData.end() == it)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "checkInServiceCondition(), no metadata %s"), METADATA_FileSize);
		return false;			// no file size information
	}

	uint64 llFileSize = 0;

#ifdef ZQ_OS_MSWIN	
	llFileSize = ::_atoi64(it->second.c_str());
#else
	llFileSize = strtoull(it->second.c_str(), (char* *)NULL ,10);
#endif

	if (llFileSize < MINIMIZE_FILESIZE)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "checkInServiceCondition(), the metadata [%s=%d] is smaller than minimize filesize %d"), 
			METADATA_FileSize, int(llFileSize), MINIMIZE_FILESIZE);
		return false;
	}

	return true;
}

static void countPopulateError(ContentImpl& theContent, bool errorOccured=true) // NGOD-512
{
	if (!errorOccured)
	{
		MAPSET(::TianShanIce::Properties, theContent.metaData, SYS_PROP(PopulateAttrsFailures), "0");
		return;
	}

	// NGOD-512
	char buf[32] = "1";
	::TianShanIce::Properties::const_iterator it = theContent.metaData.find(SYS_PROP(PopulateAttrsFailures));
	if (theContent.metaData.end() != it)
	{
		int failCount = atoi(it->second.c_str());
		if (++failCount >100)
			failCount = 100;
		snprintf(buf, sizeof(buf) -2, "%d", failCount);
	}

	MAPSET(::TianShanIce::Properties, theContent.metaData, SYS_PROP(PopulateAttrsFailures), buf);
}

void ContentStateBase::doPopulateFromFs(const ::Ice::Current& c)
{
	bool populateError = false;
	if (_content.state < ::TianShanIce::Storage::csOutService) // skip to calling portal if it is in OutService or Cleaning
		populateError = !_populateAttrFromFs();
		
	countPopulateError(_content, populateError);

	if (populateError)
	{
		// NGOD-512
		char buf[32] = "1";
		::TianShanIce::Properties::const_iterator it = _content.metaData.find(SYS_PROP(PopulateAttrsFailures));
		if (_content.metaData.end() != it)
		{
			int failCount = atoi(it->second.c_str());
			if (++failCount >100)
				failCount = 100;
			snprintf(buf, sizeof(buf) -2, "%d", failCount);
		}
		
		MAPSET(::TianShanIce::Properties, _content.metaData, SYS_PROP(PopulateAttrsFailures), buf);
	}
	else	MAPSET(::TianShanIce::Properties, _content.metaData, SYS_PROP(PopulateAttrsFailures), "0");

	if ( populateError &&  _content.state > ::TianShanIce::Storage::csProvisioning && !_store._edgeMode )
	{
		// if current state > provision and not in edgeMode, return directly
		MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentState, "doPopulateFromFs() quit due to portal populateErr and non-edge mode"));
		return;
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "doPopulateFromFs() checking the file residential status after populating the attributes"));

	uint64 flags = 0;
	try {
		uint64 flagsToTest = RSDFLAG(frfAbsence) | RSDFLAG(frfWriting) | RSDFLAG(frfResidential);
		flags = ContentStoreImpl::checkResidentialStatus(_store, flagsToTest, &_content, _content.ident.name, _content._mainFilePathname());
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() portal::checkResidentialStatus(%04llx) returns %04llx"), flagsToTest, flags);
	}
	catch (...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "doPopulateFromFs() portal::checkResidentialStatus() throw exception, quit state changing"));
		return;
	}

	// determin dirty
	_content.bDirtyAttrs = false;

	if (populateError)
		_content.bDirtyAttrs = true;
	else if (flags & (RSDFLAG(frfAbsence) | RSDFLAG(frfWriting)))
		_content.bDirtyAttrs = true;

	switch (_content.state)
	{
	case ::TianShanIce::Storage::csNotProvisioned:

		if (IS_EXISTED(flags) || IS_WRITTING(flags))
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() member file exists or is under writing, goes to state Provisioning"));
			ContentStateProvisioning(_store, _content).enter();
		}

		if (populateError)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentState, "doPopulateFromFs() quits due to portal populateErr"));
			return;
		}

		if (checkInServiceCondition(flags))
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() member file are all residented and no more writing, goes to state InService"));
			ContentStateInService(_store, _content).enter();
		}

		break;

	case ::TianShanIce::Storage::csProvisioning:
		// skip to stay in csProvisioning if no member file exists, trust as provision sess hasn't started yet, unless
		// the session has been out of provisioning window
		if (!IS_EXISTED(flags) )
		{
			::Ice::Long scheduledEnd = 0;
			if (_content.metaData.end() != _content.metaData.find(METADATA_ScheduledProvisonEnd))
				scheduledEnd = ISO8601ToTime(_content.metaData[METADATA_ScheduledProvisonEnd].c_str());

			::Ice::Long stampNow = ZQTianShan::now();

			if (scheduledEnd <=0 || stampNow <= scheduledEnd + UNATTENDED_TIMEOUT)
				return;

			// Bug#12018, the files have gone although the proivision has ever been started. could be cleaned by CPE
#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			MOEVENT(ContentEventFMT(ProvisionGivenUp, 24, "scheduleEnd reached but no member file residents"));
			ContentStateOutService(_store, _content).enter();
		}

		if (populateError)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentState, "doPopulateFromFs() quits due to portal populateEr"));
			return;
		}

		// if any member file is under writing, check if it is ready to csProvisioningStreamable
		if (IS_WRITTING(flags))
		{
			int monoProvision = 0;
			if (_content.metaData.end() != _content.metaData.find(METADATA_MonoProvision))
				monoProvision = atoi(_content.metaData[METADATA_MonoProvision].c_str());

			if (!monoProvision && !MISSING_MEM_FILE(flags))
			{

				::Ice::Long playtime = _atoi64(_content.metaData[METADATA_PlayTime].c_str());

				// for enh#16995 Serve the cached copy that is catching up the PWE copy on source storage
				::Ice::Long streamablePlaytime = 0;
				if (_content.metaData.end() != _content.metaData.find(METADATA_EstimatedStreamable))
					streamablePlaytime = _atoi64(_content.metaData[METADATA_EstimatedStreamable].c_str());

				streamablePlaytime = max(_store._streamableLength, streamablePlaytime);

				if (streamablePlaytime >0 && playtime < streamablePlaytime)
					return;

				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() playtime[%lld] is longer than streamable[%lld]ms, goes to state ProvisioningStreamable"), playtime, streamablePlaytime);
				ContentStateProvisioningStreamable(_store, _content).enter();
			}

			return; // no more checking for this state now
		}

		// no writing if the program reach here, go to InService if all the member files are ready
		if (checkInServiceCondition(flags))
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() no writting and all member files are residented and no more writing, goes to state InService"));
			ContentStateInService(_store, _content).enter();
		}

		break;

	case ::TianShanIce::Storage::csProvisioningStreamable:
		// no writing if the program reach here, go to InService if all the member files are ready
		if (populateError)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentState, "doPopulateFromFs() quits due to portal populateErr"));
			return;
		}

		if (checkInServiceCondition(flags))
		{
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() no writting and all member files are residented and no more writing, goes to state InService"));
			ContentStateInService(_store, _content).enter();
		}

		if (!IS_EXISTED(flags) )
		{
			// Bug#12018, the files have gone although it ever been streamable. could be cleaned by CPE
			::Ice::Long scheduledEnd = 0;
			if (_content.metaData.end() != _content.metaData.find(METADATA_ScheduledProvisonEnd))
				scheduledEnd = ISO8601ToTime(_content.metaData[METADATA_ScheduledProvisonEnd].c_str());

			::Ice::Long stampNow = ZQTianShan::now();

			if (scheduledEnd <=0 || stampNow <= scheduledEnd + UNATTENDED_TIMEOUT)
				return;

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			MOEVENT(ContentEventFMT(ProvisionGivenUp, 24, "scheduleEnd reached but no member file residents"));
			ContentStateOutService(_store, _content).enter();
		}


		break;

	case ::TianShanIce::Storage::csInService:
#pragma message ( __MSGLOC__ "TODO: more to check for fileset healthy")

		if (_store._edgeMode)
		{
			// if it is edge mode, cache of a persistent storage, the member file gone should always be monitored during InService
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() edgeMode checking member file missed[%c]"), MISSING_MEM_FILE(flags)?'T':'F');
		}
		else
		{
			// persistent storage logic here
			// if the store doesn't care file deleted from the filesystem, leave the content as it is
			if (0 != ((FLAG(::TianShanIce::Storage::fseFileDeleted) | FLAG(::TianShanIce::Storage::fseFileRenamed)) & _store._fileEventFlags))
				return;

			if (!_store._enableInServiceCheck)
				return;
		}

		// if all the member file are still there, stay in the state
		if (!MISSING_MEM_FILE(flags))
			return;

		// any file is missed, goes to OutOfService
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "doPopulateFromFs() member file missed, goes to state OutService"));
		ContentStateOutService(_store, _content).enter();
		break;

	case ::TianShanIce::Storage::csOutService:
	case ::TianShanIce::Storage::csCleaning:
		// nothing to do with these states except follows the timer of OutService
		break; 

	default:
//		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (MOLOG, ContentStateEXPFMT(ContentState, 901, "unknown targetState[%d] from the portal, ignore"), targetState);
		break;
	}
}


bool ContentStateBase::_populateAttrFromFs()
{
	::TianShanIce::Storage::ContentState targetState = _content.state;
	if (!_content.bDirtyAttrs)
		return true; // some other thread already did the job;

	std::string pathOfVolume;

	try
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() look up the path-of-volume"));

		::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, _content.identVolume);
		pathOfVolume = volume->getMountPath();

		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() found path-of-volume[%s]"), pathOfVolume.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() looking up the path-of-volume caught exception: %s"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() looking up the path-of-volume caught unknown exception"));
		return false;
	}

	bool populated = false;

	try {
		// mark it as ever triggered populating but not certain if it would succeeded
		if (_content.stampLastUpdated <=0)
			_content.stampLastUpdated =1;
		
		populated = ContentStoreImpl::populateAttrsFromFile(_store, _content, pathOfVolume + _content._name());
		if (populated)
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() portal indicate the target state should be %s(%d) size=%s"), ContentStateBase::stateStr(targetState), targetState, _content.metaData[METADATA_FileSize].c_str());	
		else
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() portal returns failed"));	
	}
	catch (const ZQ::common::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() populate attribute caught exception: %s"), ex.getString());
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() populate attribute caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() populate attribute caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() populate attribute caught unknown exception"));
	}

	::Ice::Long stampNow = ZQTianShan::now();
	if (!populated)
	{
		long nextWakeup = MIN_UPDATE_INTERVAL;
		if (_content.fromFsOrphan && 1 == _content.stampLastUpdated) // means content created by the filesystem, and ever called populating but never get succeeded
			nextWakeup = UNATTENDED_TIMEOUT*5;

		if ((::TianShanIce::Storage::csProvisioning == _content.state || ::TianShanIce::Storage::csProvisioningStreamable == _content.state)
			&& _content.metaData.end() != _content.metaData.find(METADATA_ScheduledProvisonEnd))
		{
			std::string scheduledEndStr = _content.metaData[METADATA_ScheduledProvisonEnd];
			::Ice::Long timeToscheduledEnd = ISO8601ToTime(scheduledEndStr.c_str()) - stampNow;
			if (timeToscheduledEnd < nextWakeup)
				nextWakeup = (long) MAX(5000, timeToscheduledEnd); // no less than 5sec
		}

		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_populateAttrFromFs() schedule the retry in %dmsec"), nextWakeup);
		_updateExpiration(stampNow + nextWakeup); // try again in MIN_UPDATE_INTERVAL
		return populated;
	}

	_content.stampLastUpdated = stampNow;
	//_content.bDirtyAttrs =false;
	return populated;
}

#define MINI_TIMMER_MS		50
void ContentStateBase::_updateExpiration(const ::Ice::Long newExpiration)
{
#if ICE_INT_VERSION / 100 >= 306
	ContentImpl::Lock lock(_content);
#else
	ContentImpl::WLock lock(_content);
#endif
	_content.expiration = newExpiration;
	if (_content.expiration >0)
	{
		int nTime = int(newExpiration-now());
		if (nTime < 0)
			nTime = 0;
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "update expiration with %d(ms)"), nTime);
		_store._watchDog.watch(_content.ident, (long) nTime);
	}
	else MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentState, "_updateExpiration() skip updating an invalid expiration value"));
}

void ContentStateBase::_destroy()
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_destroy() enter"));
	try
	{
		_store._eContent->remove(_content.ident);

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
		MOEVENT(ContentEventFMT(Destroyed, 1, ""));
		_store.OnContentDestroyed(_content.ident);

		MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(ContentState, "_destroy() content removed from DB"));
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentState, "_destroycontent() object already gone from the container, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_destroycontent() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_destroycontent() caught unknown exception"));
	}

}

void ContentStateBase::_cancelProvision()
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "_cancelProvision() enter"));
	try
	{
		_content._cancelProvision();
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentState, "_cancelProvision() object already gone from the container, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_cancelProvision() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "_cancelProvision() caught unknown exception"));
	}
}

// avoid the attrbiute populating from file won't happened too frequently
static int64 determinDirtyExpiration(ContentImpl& theContent) // NGOD-512
{
	int failCount =0; //TODO: this field should goes to a member of Content record instead of a metadata[SYS_PROP(PopulateAttrsFailures)]
	long interval = MIN_UPDATE_INTERVAL;
	::TianShanIce::Properties::const_iterator it = theContent.metaData.find(SYS_PROP(PopulateAttrsFailures));
	if (theContent.metaData.end() != it)
		failCount = atoi(it->second.c_str());
				
	for (int i=0; i < failCount && interval < (MAX_NOT_PROVISIONED_TIMEOUT >>2); i++)
		interval <<=1; //double the interval
				
	return theContent.stampLastUpdated + interval;
}

void ContentStateBase::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = now();

	try {
		if (_content.bDirtyAttrs)
		{
			// avoid the attrbiute populating from file won't happened too frequently
			int failCount =0; //TODO: NGOD-512, this field should goes to a member of Content record instead of a metadata[SYS_PROP(PopulateAttrsFailures)]
			long interval = MIN_UPDATE_INTERVAL;
			::TianShanIce::Properties::const_iterator it = _content.metaData.find(SYS_PROP(PopulateAttrsFailures));
			if (_content.metaData.end() != it)
				failCount = atoi(it->second.c_str());

			for (int i=0; i < failCount && interval < (MAX_NOT_PROVISIONED_TIMEOUT *2); i++)
				interval <<=1; //double the interval

			int64 exp = _content.stampLastUpdated + interval;
			char buf[64];

			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateBase, "OnTimer() dirty content has minimal [%d] failure(s) at previous attribute populating, test against popu-exp[%s]"), failCount, ZQTianShan::TimeToUTC(exp, buf, sizeof(buf) -2));
			if (stampNow > exp)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateBase, "OnTimer() the attributes are out-of-date, force to popluate attributes from the file"));
				(new PopulateFileAttrsCmd(_store, _content.ident))->execute();
			}
			else _updateExpiration(exp);

			return;
		}
	}
	catch(const ZQ::common::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStateBase,"OnTimer() caugh exception: %s"), ex.getString());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateBase, "OnTimer() caught unknown exception when add PopulateFileAttrsCmd"));
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateBase, "OnTimer() renew the timer for next wakup in %d msec"), UNATTENDED_TIMEOUT);
	_updateExpiration(stampNow + UNATTENDED_TIMEOUT); // do not allow 0 expiration by default
}


void ContentStateBase::OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c)
{
 	::Ice::Long stampWakeup = _content.expiration >0 ? _content.expiration : (ZQTianShan::now() + UNATTENDED_TIMEOUT);
 	char buf[64];
 	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "OnRestore() reset timer at %s"), ZQTianShan::TimeToUTC(stampWakeup, buf, sizeof(buf) -2));
 	_updateExpiration(stampWakeup);
}

// -----------------------------
// class ContentStateNotProvisioned
// -----------------------------
void ContentStateNotProvisioned::enter(void)
{
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateNotProvisioned, "enter()"));

		switch(_content.state)
		{
		case ::TianShanIce::Storage::csNotProvisioned:
			return; // do nothing, and continue with the initialization steps

		case ::TianShanIce::Storage::csProvisioning:
		case ::TianShanIce::Storage::csProvisioningStreamable:
		case ::TianShanIce::Storage::csInService:
		case ::TianShanIce::Storage::csOutService:
		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStateNotProvisioned, 0101, "not allowed to enter"));
		}
	}

	if (_store._timeoutNotProvisioned < 0 || _store._timeoutNotProvisioned > MAX_NOT_PROVISIONED_TIMEOUT)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ContentStateNotProvisioned::enter() store configuration on not-provisioned timeout is out of range, adjust it to %dmsec"), MAX_NOT_PROVISIONED_TIMEOUT);
		_store._timeoutNotProvisioned = MAX_NOT_PROVISIONED_TIMEOUT;
	}

	_commitState(false); // do not send the state change for this state
	_updateExpiration(_store._timeoutNotProvisioned +now());
}

void ContentStateNotProvisioned::OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c)
{
	if (stampLastFileWrite.empty())
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateNotProvisioned, "OnRestore() file doesn't not exist, force to OnTimer()"));
		OnTimer(c);
		return;
	}

	// if fileExists here
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateNotProvisioned, "OnRestore() file exists, populate the attributes from file"));

	if (ZQTianShan::ISO8601ToTime(stampLastFileWrite.c_str()) > _content.stampLastUpdated)
		_content.bDirtyAttrs = true;

	_updateExpiration(now() + 2000);
}

void ContentStateNotProvisioned::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = now();
	if (_store._timeoutNotProvisioned <0 || _store._timeoutNotProvisioned > MAX_NOT_PROVISIONED_TIMEOUT)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ContentStateNotProvisioned::OnTimer() store configuration on not-provisioned timeout is out of range, adjust it to %dmsec"), MAX_NOT_PROVISIONED_TIMEOUT);
		_store._timeoutNotProvisioned = MAX_NOT_PROVISIONED_TIMEOUT;
	}

	if (_content.expiration <= 0)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateNotProvisioned, "OnTimer() uninitialized expiration, set to expire in %d msec"), _store._timeoutNotProvisioned);
		_updateExpiration(stampNow + _store._timeoutNotProvisioned); // do not allow 0 expiration at this state
		return;
	}

	long  msUnattended = (long)(stampNow - _content.stampCreated);
	int32 timeout = _store._timeoutNotProvisioned;
	if (_content.bDirtyAttrs)
		timeout *= 3;

	if (msUnattended > timeout)
	{
		MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateNotProvisioned, "OnTimer() long unattended provision record, force to go out-of-service"));
		ContentStateOutService(_store, _content).enter();
		return;
	}

	ContentStateBase::OnTimer(c);
}

/*
void ContentStateNotProvisioned::doPopulateFromFs(const ::Ice::Current& c)
{
	::TianShanIce::Storage::ContentState targetState = _populateAttrFromFs();

	switch (targetState)
	{
	case ::TianShanIce::Storage::csNotProvisioned:

	case ::TianShanIce::Storage::csOutService:
		// file may not resident yet, keep staying in csNotProvisioned
		break; 

	case ::TianShanIce::Storage::csProvisioning:
		ContentStateProvisioning(_store, _content).enter();
		break;
	case ::TianShanIce::Storage::csProvisioningStreamable:
		ContentStateProvisioningStreamable(_store, _content).enter();
		break;
	case ::TianShanIce::Storage::csInService:
		ContentStateInService(_store, _content).enter();
		break;
	case ::TianShanIce::Storage::csCleaning:
		ContentStateCleaning(_store, _content).enter();
		break;

	default:
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (MOLOG, ContentStateEXPFMT(ContentStateNotProvisioned, 901, "unknown targetState[%d] from the portal, ignore"), targetState);
	}
}
*/
// -----------------------------
// class ContentStateProvisioning
// -----------------------------
void ContentStateProvisioning::enter(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "enter()"));

	// only allowed to be entered from the state of stNotProvisioned
	switch(_content.state)
	{

	case ::TianShanIce::Storage::csProvisioning:
		return; // do nothing, and continue with the initialization steps

	case ::TianShanIce::Storage::csNotProvisioned:
		ContentStateNotProvisioned(_store, _content).leave();
		break;

	case ::TianShanIce::Storage::csProvisioningStreamable:
		// allow to switch between the state csProvisioning & csProvisioningStreamable
		ContentStateProvisioningStreamable(_store, _content).leave();
		break;

	case ::TianShanIce::Storage::csInService:
	case ::TianShanIce::Storage::csOutService:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStateProvisioning, 101, "enter() not allowed from this state"));
	}

	_commitState();
	_updateExpiration(ZQTianShan::now() + UNATTENDED_TIMEOUT);
}

void ContentStateProvisioning::leave(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "leave()"));
}

void ContentStateProvisioning::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = now();

	::Ice::Long scheduledEnd = 0;
	::std::string scheduledEndStr;
	if (_content.metaData.end() != _content.metaData.find(METADATA_ScheduledProvisonEnd))
	{
		scheduledEndStr = _content.metaData[METADATA_ScheduledProvisonEnd];
		scheduledEnd = ISO8601ToTime(scheduledEndStr.c_str());
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() metaData[" METADATA_ScheduledProvisonEnd "] =[%s]"), scheduledEndStr.c_str());
	}
	
	if (scheduledEnd <=0)
	{
		if (_store._timeoutIdleProvisioning < _store._timeoutNotProvisioned && _store._timeoutNotProvisioned >0)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ContentStateProvisioning::OnTimer() store configuration on idle privisioning timeout is less than not-provisioned timeout, adjust it to %dmsec"), _store._timeoutNotProvisioned);
			_store._timeoutIdleProvisioning = _store._timeoutNotProvisioned;
		}

		if (_store._timeoutIdleProvisioning <=0 || _store._timeoutIdleProvisioning > MAX_IDLE_PROVISIONING_TIMEOUT)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ContentStateProvisioning::OnTimer() store configuration on idle privisioning timeout is out of range, adjust it to %dmsec"), MAX_IDLE_PROVISIONING_TIMEOUT);
			_store._timeoutIdleProvisioning = MAX_IDLE_PROVISIONING_TIMEOUT;
		}

		scheduledEnd = _content.stampCreated + _store._timeoutIdleProvisioning;
		char buf[64];
		scheduledEndStr = TimeToUTC(scheduledEnd, buf, sizeof(buf) -2);
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() metaData[" METADATA_ScheduledProvisonEnd "] is not specified, using expiration[%s]"), scheduledEndStr.c_str());
		MAPSET(::TianShanIce::Properties, _content.metaData, METADATA_ScheduledProvisonEnd, scheduledEndStr);
	}

	if (stampNow < scheduledEnd + UNATTENDED_TIMEOUT)
	{
		try {
			if (!_content.bDirtyAttrs && stampNow > _content.stampLastUpdated + (MIN_UPDATE_INTERVAL>>2))
				_content.bDirtyAttrs = true;

			if (_content.bDirtyAttrs)
			{
				// avoid the attrbiute populating from file won't happened too frequently
				int failCount =0; //TODO: NGOD-512, this field should goes to a member of Content record instead of a metadata[SYS_PROP(PopulateAttrsFailures)]
				long interval = MIN_UPDATE_INTERVAL;

				if (stampNow > _store._stampStarted +MIN_UPDATE_INTERVAL*2) // the service start period has no need to double interval per failure count
				{
					::TianShanIce::Properties::const_iterator it = _content.metaData.find(SYS_PROP(PopulateAttrsFailures));
					if (_content.metaData.end() != it)
						failCount = atoi(it->second.c_str());

					for (int i=0; i < failCount && interval < (MAX_NOT_PROVISIONED_TIMEOUT >>2); i++)
						interval <<=1; //double the interval
				}

				int64 exp = _content.stampLastUpdated + interval;
				char buf[64];
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateBase, "OnTimer() dirty content has minimal [%d] failure(s) at previous attribute populating, test against expiration [%s]"), failCount, ZQTianShan::TimeToUTC(exp, buf, sizeof(buf) -2));

				if (stampNow > exp)
				{
					MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() the attributes are out-of-date, force to popluate attributes from the file"));
					(new PopulateFileAttrsCmd(_store, _content.ident))->execute();
					exp = stampNow + UNATTENDED_TIMEOUT;
				}
				
				_updateExpiration(exp);

				return;
			}
		}
		catch(const ZQ::common::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStateProvisioning,"OnTimer() caugh exception: %s"), ex.getString());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() caught unknown exception when add PopulateFileAttrsCmd"));
		}

		long nextWakeup = UNATTENDED_TIMEOUT;
		if (_content.fromFsOrphan && 1 == _content.stampLastUpdated) // means triggered by the filesystem, and ever called populating but never get succeeded
			nextWakeup = MIN(UNATTENDED_TIMEOUT*5, (long)(scheduledEnd + UNATTENDED_TIMEOUT - stampNow));

		_updateExpiration(stampNow + nextWakeup); // do not allow 0 expiration by default
		return;
	}

	// stampNow > scheduledEnd + UNATTENDED_TIMEOUT	if the program reached here,

	// check the provision session if there is any bound
	try {
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() metaData[" METADATA_ScheduledProvisonEnd "]=[%s] checking the Privisioning session[%s]"), scheduledEndStr.c_str(), _content.provisionPrxStr.c_str());
		::TianShanIce::ContentProvision::ProvisionSessionPrx provisionSess;
		provisionSess = ::TianShanIce::ContentProvision::ProvisionSessionPrx::uncheckedCast(_store._adapter->getCommunicator()->stringToProxy(_content.provisionPrxStr));
		provisionSess->ice_ping();
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() keep staying at the state because of the alive ProvisionSession[%s]"), _content.provisionPrxStr.c_str());
		_content.bDirtyAttrs = true;
	}
	catch(const IceUtil::NullHandleException&)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStateProvisioning,"OnTimer() null ProvisionSession"));
		_content.bDirtyAttrs = false;
	}
	catch(const Ice::ObjectNotExistException&)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStateProvisioning,"OnTimer() ProvisionSession has gone"));
		_content.bDirtyAttrs = false;
	}
	catch(const Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStateProvisioning,"OnTimer() force to populate due to cast ProvisionSession caugh exception: %s"), ex.ice_name().c_str());
		_content.bDirtyAttrs = (stampNow < scheduledEnd + UNATTENDED_TIMEOUT*3);
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() force to populate due to cast ProvisionSession caugh unknown exception"));
		_content.bDirtyAttrs = (stampNow < scheduledEnd + UNATTENDED_TIMEOUT*3);
	}

	try {
		if (!_content.bDirtyAttrs)
		{
			uint64 uRet = ContentStoreImpl::checkResidentialStatus(_store, RSDFLAG(frfAbsence), NULL, _content.ident.name, _content._mainFilePathname());
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "checkResidentialStatus(%s) result[%I64x]"), _content.ident.name.c_str(), uRet);
			
			bool bAbsence = RSDFLAG(frfAbsence) & uRet;

			if (bAbsence)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() expired at the state, force to be OutService"));
				ContentStateOutService(_store, _content).enter();
				return;
			}
		}

		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() attributes needs populated due to ProvisionSession status"));
		(new PopulateFileAttrsCmd(_store, _content.ident))->execute();
	}
	catch(const ZQ::common::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStateProvisioning,"OnTimer() populate attrs per provision session, caugh exception: %s"), ex.getString());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStateProvisioning,"OnTimer() populate attrs per provision session, caugh unknown exception"));
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnTimer() renew the timer for next wakup in %d msec"), UNATTENDED_TIMEOUT);
	_updateExpiration(stampNow + UNATTENDED_TIMEOUT); // do not allow 0 expiration by default
}

void ContentStateProvisioning::OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnRestore() follows with OnTimer()"));
	if (ZQTianShan::ISO8601ToTime(stampLastFileWrite.c_str()) > _content.stampLastUpdated)
		_content.bDirtyAttrs = true;

#if 0
	OnTimer(c); // force to put self under watching
#else
	int nWakeUp = 1000;	//delay 1000 seconds to do
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioning, "OnRestore() renew the timer for next wakup in %d msec"), nWakeUp);
	::Ice::Long stampNow = now();
	_updateExpiration(stampNow + nWakeUp); // do not allow 0 expiration by default
#endif
}


// -----------------------------
// class ContentStateProvisioningStreamable
// -----------------------------
void ContentStateProvisioningStreamable::enter(void)
{
	// only allowed to be entered from the state of stNotProvisioned
	switch(_content.state)
	{

	case ::TianShanIce::Storage::csProvisioningStreamable:
		return; // do nothing, and continue with the initialization steps

	case ::TianShanIce::Storage::csNotProvisioned:
		ContentStateNotProvisioned(_store, _content).leave();
		break;

	case ::TianShanIce::Storage::csProvisioning:
		// allow to switch between the state csProvisioning & csProvisioningStreamable
		ContentStateProvisioning(_store, _content).leave();
		break;

	case ::TianShanIce::Storage::csInService:
	case ::TianShanIce::Storage::csOutService:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStateProvisioningStreamable, 101, "enter() not allowed from this state"));
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//  add a streamable event log
	MOEVENT(Log::L_INFO, "EVENT(STREAMABLE) content(%s)", _content._name().c_str());

	_commitState();
	_updateExpiration(ZQTianShan::now() + UNATTENDED_TIMEOUT);
}

void ContentStateProvisioningStreamable::leave(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioningStreamable, "leave()"));
}

void ContentStateProvisioningStreamable::OnTimer(const ::Ice::Current& c)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioningStreamable, "OnTimer() borrow ContentStateProvisioning::OnTimer()"));
	ContentStateProvisioning(_store, _content).OnTimer(c);
}

void ContentStateProvisioningStreamable::OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateProvisioningStreamable, "OnRestore() follows with OnTimer()"));
	if (ZQTianShan::ISO8601ToTime(stampLastFileWrite.c_str()) > _content.stampLastUpdated)
		_content.bDirtyAttrs = true;

	OnTimer(c);
}

// -----------------------------
// class ContentStateInService
// -----------------------------
void ContentStateInService::enter(void)
{	
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateInService, "enter()"));

	switch(_content.state)
	{

	case ::TianShanIce::Storage::csInService:
		return; // do nothing, and continue with the initialization steps

	case ::TianShanIce::Storage::csNotProvisioned:
		ContentStateNotProvisioned(_store, _content).leave();
		break;

	case ::TianShanIce::Storage::csProvisioning:
		ContentStateProvisioning(_store, _content).leave();
		break;

	case ::TianShanIce::Storage::csProvisioningStreamable:
		ContentStateProvisioningStreamable(_store, _content).leave();
		break;

	case ::TianShanIce::Storage::csOutService:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStateProvisioningStreamable, 101, "enter() not allowed from this state"));
	}

	_content.stampProvisioned = now();
	_commitState();
	
	_content.bDirtyAttrs = true;
	_updateExpiration(ZQTianShan::now());

	//	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateInService, "enter() skip to be under watching"));	
}

void ContentStateInService::leave(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateInService, "leave()"));
}


void ContentStateInService::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = now();

	try {
		if (_content.bDirtyAttrs)
		{
			if (stampNow - _content.stampLastUpdated > MIN_UPDATE_INTERVAL)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateInService, "OnTimer() the attributes are out-of-date, force to popluate attributes from the file"));
				(new PopulateFileAttrsCmd(_store, _content.ident))->execute();
			}
			else _updateExpiration(MAX(stampNow + 5000, _content.stampLastUpdated + MIN_UPDATE_INTERVAL)); // at least after 5 sec for next check

			return;
		}
	}
	catch(const ZQ::common::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStateInService,"OnTimer() caugh exception: %s"), ex.getString());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateInService, "OnTimer() caught unknown exception when add PopulateFileAttrsCmd"));
	}

	//For this state, NO NEED to updateExpiration and directly drop self from watching
}

void ContentStateInService::OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c)
{
	if (stampLastFileWrite.empty())
	{
		MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(ContentStateInService, "OnRestore() force to out-service since the file doesn't exist on filesystem anymore"));
		ContentStateOutService(_store, _content).enter();
		return;
	}

	::Ice::Long lastWrite = ZQTianShan::ISO8601ToTime(stampLastFileWrite.c_str());
	if (lastWrite > _content.stampLastUpdated)
		_content.bDirtyAttrs = true;

	OnTimer(c);
}

// -----------------------------
// class ContentStateOutService
// -----------------------------
void ContentStateOutService::enter(void)
{	
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateOutService, "enter()"));
	
	try {
		switch(_content.state)
		{
		case ::TianShanIce::Storage::csOutService:
			return; // do nothing, and continue with the initialization steps

		case ::TianShanIce::Storage::csNotProvisioned:
			ContentStateNotProvisioned(_store, _content).leave();
			break;

		case ::TianShanIce::Storage::csProvisioning:
			_cancelProvision();
			ContentStateProvisioning(_store, _content).leave();
			break;

		case ::TianShanIce::Storage::csProvisioningStreamable:
			_cancelProvision();
			ContentStateProvisioningStreamable(_store, _content).leave();
			break;

		case ::TianShanIce::Storage::csInService:
			ContentStateInService(_store, _content).leave();
			break;

		default:
			//ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStateOutService, 101, "enter() not allowed from this state"));
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateOutService, "enter() ignore invalid new state ContentStateOutService"));
			return;
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateOutService, "enter() ignore the exContentStoretion[%s] when leaving prev state: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateOutService, "enter() ignore the exContentStoretion[%s] when leaving prev state"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateOutService, "enter() ignore the unknown exContentStoretion when leaving prev state"));
	}

	if (_store._storeAggregate)
	{
		// per bug#18785, we wish to ask the slaves to enter OutService too at such a moment instead of waiting for OUTSERVICE_TIMEOUT by this clusterCS
		try {
			std::string mainFilePathname = _content._mainFilePathname();
			MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(ContentStateOutService, "enter() destroying slave replicas"));
			_store.deleteFileByContent(_store, _content, mainFilePathname);
		}
		catch (const ZQ::common::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "leave() failed to clean content/file, exception: %s"), ex.getString());
		}
		catch (...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "leave() failed to clean content/file, unknown exception"));
		}
	}

	_commitState();
	_updateExpiration(ZQTianShan::now() + OUTSERVICE_TIMEOUT);
}

void ContentStateOutService::leave()
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateOutService, "leave()"));
}

void ContentStateOutService::OnTimer(const ::Ice::Current& c)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateOutService, "OnTimer()"));
	::Ice::Long stampNow = now();

	if (_content.expiration <= 0)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateOutService, "OnTimer() uninitialized expiration, set to expire in %d msec"), OUTSERVICE_TIMEOUT);
		_updateExpiration(stampNow + OUTSERVICE_TIMEOUT); // do not allow 0 expiration at this state
		return;
	}
	
#if 0 //this code is not needed anymore, because the cleanning state would check this destroy signature

	if (_content.metaData.end() == _content.metaData.find(SYS_PROP(DestroySignature)) || _content.metaData[SYS_PROP(DestroySignature)].empty())
	{
		try {
			uint64 flags = ContentStoreImpl::checkResidentialStatus(_store, (uint64)RSDFLAG(frfResidential), &_content, _content.ident.name, _content._mainFilePathname());
			MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentState, "OnTimer() portal::checkResidentialStatus() returns %04llx"), flags);
			// program reaches here because no explicit Content::Destory() API has been ever called
			if (IS_EXISTED(flags))
			{
				MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateOutService, "OnTimer() content is not destroyed explicit and member still on the filesystem, quit watching"));
				return;
			}
		}
		catch (...)
		{
			MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentState, "OnTimer() portal::checkResidentialStatus() throw exception, quit watching"));
			return;
		}
	}

#endif

	MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(ContentStateOutService, "OnTimer() entering ContentStateCleaning"));
	ContentStateCleaning(_store, _content).enter();
}

void ContentStateOutService::OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c)
{
	OnTimer(c); // force to put self under watching
}

// -----------------------------
// class ContentStateCleaning
// -----------------------------
void ContentStateCleaning::enter(void)
{	
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateCleaning, "enter()"));
	
	try {
		switch(_content.state)
		{
		case ::TianShanIce::Storage::csCleaning:
			return; // do nothing, and continue with the initialization steps

		case ::TianShanIce::Storage::csNotProvisioned:
			ContentStateNotProvisioned(_store, _content).leave();
			break;

		case ::TianShanIce::Storage::csProvisioning:
			_cancelProvision();
			ContentStateProvisioning(_store, _content).leave();
			break;

		case ::TianShanIce::Storage::csProvisioningStreamable:
			_cancelProvision();
			ContentStateProvisioningStreamable(_store, _content).leave();
			break;

		case ::TianShanIce::Storage::csInService:
			ContentStateInService(_store, _content).leave();
			break;

		case ::TianShanIce::Storage::csOutService:
			ContentStateOutService(_store, _content).leave();
			break;

		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, ContentStateEXPFMT(ContentStateProvisioningStreamable, 101, "enter() not allowed from this state"));

		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateCleaning, "enter() ignore the exContentStoretion[%s] when leaving prev state: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateCleaning, "enter() ignore the exContentStoretion[%s] when leaving prev state"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, ContentStateLOGFMT(ContentStateCleaning, "enter() ignore the unknown exContentStoretion when leaving prev state"));
	}

	_commitState();
	_updateExpiration(ZQTianShan::now());
}

void ContentStateCleaning::leave()
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateCleaning, "leave() cleaning the content"));
	
	bool bCleanDone = false;

	if (_content.metaData.end() == _content.metaData.find(SYS_PROP(DestroySignature))  || _content.metaData[SYS_PROP(DestroySignature)].empty())
	{
		MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(ContentStateCleaning, "leave() destroy with no signature, skipping files"));
		bCleanDone = true;
	}
	else
	{
		::std::string& destroySig = _content.metaData[SYS_PROP(DestroySignature)];
		try {
			std::string mainFilePathname = _content._mainFilePathname();
			MOLOG(ZQ::common::Log::L_INFO, ContentStateLOGFMT(ContentStateCleaning, "leave() cleaning all supplemental file(s) of [%s], destorySig[%s]"), mainFilePathname.c_str(), destroySig.c_str());
			bCleanDone = _store.deleteFileByContent(_store, _content, mainFilePathname);
		}
		catch (const ZQ::common::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "leave() failed to clean content/file, exception: %s"), ex.getString());
		}
		catch (...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "leave() failed to clean content/file, unknown exception"));
		}
	}

	if (!bCleanDone)
		throw ZQ::common::Exception("failed to clean all supplemental file(s) of this content");
}

void ContentStateCleaning::OnTimer(const ::Ice::Current& c)
{
	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateCleaning, "OnTimer()"));
//	::Ice::Long stampNow = now();

	if (_store._storeAggregate)
	{
		// per bug#18785, for cluster mode, it is needed to ensure the deletion gets completed as the destroy2(true) fired by this instance to slave is async call
		try {
			std::string mainFilePathname = _content._mainFilePathname();
			uint64 flags = _store.checkResidentialStatus(_store, RSDFLAG(frfResidential) | RSDFLAG(frfAbsence), &_content, _content.ident.name, mainFilePathname);
			if (0 != (RSDFLAG(frfResidential) & flags))
			{
				MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "OnTimer() slave replica still exists rdflags[0x%llx], delete and check later"), flags);
				try {
					_store.deleteFileByContent(_store, _content, mainFilePathname);
				}
				catch (...)	{}

				_updateExpiration(ZQTianShan::now() +OUTSERVICE_TIMEOUT);
				return;
			}
		}
		catch (const ZQ::common::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "leave() failed to clean content/file, exception: %s"), ex.getString());
		}
		catch (...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "leave() failed to clean content/file, unknown exception"));
		}
	}

	try {
		leave();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_WARNING, ContentStateLOGFMT(ContentStateCleaning, "OnTimer() failed to clean content/file, retry in %d msec"), UNATTENDED_TIMEOUT);
		_updateExpiration(ZQTianShan::now() + UNATTENDED_TIMEOUT);
		return;
	}

	MOLOG(ZQ::common::Log::L_DEBUG, ContentStateLOGFMT(ContentStateCleaning, "OnTimer() deleting content record from DB"));
	_destroy();
}

void ContentStateCleaning::OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c)
{
	OnTimer(c); // force to put self under watching
	_updateExpiration(now());
}


}} // namespace

