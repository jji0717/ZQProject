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
// Ident : $Id: CacheCmds.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheCmds.cpp $
// 
// 16    12/19/12 4:49p Hui.shao
// 
// 15    8/03/12 2:48p Hui.shao
// 
// 14    8/02/12 3:46p Hui.shao
// to avoid a big amount of RefreshUnpopularListCmd start at a same time
// 
// 13    7/31/12 12:17p Hui.shao
// 
// 12    7/25/12 4:12p Hui.shao
// added api to list hot locals
// 
// 11    7/24/12 2:47p Hui.shao
// stat counters on hitrate
// 
// 10    7/19/12 7:36p Hui.shao
// adjusted counter list compress logic
// 
// 9     7/19/12 5:35p Hui.shao
// added and refer to store-wide rootVolName
// 
// 8     6/26/12 1:22p Hui.shao
// 
// 7     6/12/12 4:35p Li.huang
// 
// 6     5/10/12 6:50p Hui.shao
// kick off cache missed content per popularity
// 
// 5     4/11/12 10:37a Li.huang
// 
// 4     1/18/12 1:50p Hui.shao
// linkable
// 
// 3     1/17/12 11:35a Hui.shao
// linkable
// 
// 2     1/16/12 10:35p Hui.shao
// defined the counter and topfolder at ice level
// 
// 1     12/22/11 1:38p Hui.shao
// created
// ===========================================================================

#include "CacheCmds.h"
#include <algorithm>

namespace ZQTianShan {
namespace ContentStore {

#define storelog (_store._log)

// -----------------------------
// class CacheCmd
// -----------------------------
CacheCmd::CacheCmd(CacheStoreImpl& store)
: ThreadRequest(store._thpool), _store(store)
{
	int pendsize = store._thpool.pendingRequestSize();
	int poolsize = store._thpool.size();
	if (pendsize > poolsize*4)
	{
		int actsize = store._thpool.activeCount();
		storelog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheCmd, "threadpool[%d/%d] has long pending requests[%d]"), actsize, poolsize, pendsize);
	}
}

CacheCmd::~CacheCmd()
{
}

// -----------------------------
// class RefreshUnpopularListCmd
// -----------------------------
CacheCmd::Map RefreshUnpopularListCmd::_pendingMap; // a hash table to avoid duplicate
::ZQ::common::Mutex RefreshUnpopularListCmd::_lkPending;

RefreshUnpopularListCmd* RefreshUnpopularListCmd::newCmd(CacheStoreImpl& store, TopFolderImpl& folder)
{
	::ZQ::common::MutexGuard g(_lkPending);
	Map::iterator it = _pendingMap.find(folder.ident.name);
	if (_pendingMap.end() != it)
		return NULL;

	RefreshUnpopularListCmd* pCmd = new RefreshUnpopularListCmd(store, folder);
	if (NULL == pCmd)
		return NULL;

	MAPSET(Map, _pendingMap, folder.ident.name, pCmd);

	return pCmd;
}

	
RefreshUnpopularListCmd::RefreshUnpopularListCmd(CacheStoreImpl& store, TopFolderImpl& folder)
: CacheCmd(store), _topFolder(folder)
{
}

int RefreshUnpopularListCmd::run(void)
{
	{
		// remove self from the pending map
		ZQ::common::MutexGuard g(_lkPending);
		_pendingMap.erase(_topFolder.ident.name);
	}

	try {
		Ice::Current c;
		char buf[32];
		snprintf(buf, sizeof(buf), "RefreshUnpopularListCmd(%p)", this);
		MAPSET(::Ice::Context, c.ctx, "signature", buf);

		_topFolder.doRefreshUnpopular(c);
		
		// the adjustion by rand() is to avoid a batch of big amount of RefreshUnpopularListCmd at same time
		long nextSleep = 1000*5 + (rand() %1000); 
		_store._watchDog.watch(_topFolder.ident, nextSleep); // wake up the watching after refreshing
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_store._log(ZQ::common::Log::L_ERROR, FLOGFMT(RefreshUnpopularListCmd,run, "tf[%s] caught %s: %s"), _topFolder.ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		_store._log(ZQ::common::Log::L_ERROR, FLOGFMT(RefreshUnpopularListCmd, run,"tf[%s] caught %s"), _topFolder.ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_store._log(ZQ::common::Log::L_ERROR, FLOGFMT(RefreshUnpopularListCmd, run,"tf[%s] caught exception"), _topFolder.ident.name.c_str());
	}

	return 1;
}

// -----------------------------
// class FlushAccessCounterCmd
// -----------------------------
CacheCmd::Map FlushAccessCounterCmd::_pendingMap; // a hash table to avoid duplicate
::ZQ::common::Mutex FlushAccessCounterCmd::_lkPending;

FlushAccessCounterCmd* FlushAccessCounterCmd::newCmd(CacheStoreImpl& store, ::TianShanIce::Storage::AccessCounter& ac)
{
	if (ac.base.contentName.empty())
		return NULL;

	::ZQ::common::MutexGuard g(_lkPending);
	Map::iterator it = _pendingMap.find(ac.base.contentName);
	if (_pendingMap.end() != it)
		return NULL;

	FlushAccessCounterCmd* pCmd = new FlushAccessCounterCmd(store, ac);
	if (NULL == pCmd)
		return NULL;

	MAPSET(Map, _pendingMap, ac.base.contentName, pCmd);
	return pCmd;
}

#define COUNTVALFMT "%d[%s~%s]"
int FlushAccessCounterCmd::run(void)
{
	int64 stampNow = ZQ::common::now();
	std::string& contentName = _countToFlush.base.contentName;
	_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushAccessCounterCmd, "content[%s]"), contentName.c_str());
	{
		// remove self from the pending map
		ZQ::common::MutexGuard g(_lkPending);
		_pendingMap.erase(contentName);
	}

	std::string replicaName;
	::TianShanIce::Storage::UnivContentPrx content;
	TianShanIce::Storage::AccessCounter counterOfDB = _countToFlush; // initialize with some default values same as _countToFlush
	::TianShanIce::Properties metaData;
	std::string strSince, strLatest, dbOldCounterStr, count2addStr, newCountStr;

	try {
		replicaName = _store._rootVolName + _store.getFolderNameOfContent(contentName, Ice::Current()) + contentName;
		
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushAccessCounterCmd, "content[%s] opening local replica[%s] from DB"), contentName.c_str(), replicaName.c_str());
		content = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_store._contentStore.openContentByFullname(replicaName, Ice::Current()));
		if (content)
		{
			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushAccessCounterCmd, "content[%s] reading metaData of replica[%s]"), contentName.c_str(), replicaName.c_str());
			metaData = content->getMetaData();

			::TianShanIce::Properties::iterator itMD = metaData.find(METADATA_AccessCount);
			if (metaData.end() != itMD)
				counterOfDB.base.accessCount = atoi(itMD->second.c_str());
			else counterOfDB.base.accessCount = 0;

			itMD = metaData.find(METADATA_AccessCountSince);
			if (metaData.end() != itMD)
			{
				strSince = itMD->second;
				counterOfDB.base.stampSince = ZQ::common::TimeUtil::ISO8601ToTime(strSince.c_str());
			}
			else counterOfDB.base.stampSince = 0;

			itMD = metaData.find(METADATA_AccessCountLatest);
			if (metaData.end() != itMD)
			{
				strLatest = itMD->second;
				counterOfDB.base.stampLatest = ZQ::common::TimeUtil::ISO8601ToTime(strLatest.c_str());
			}
			else counterOfDB.base.stampLatest = 0;

			_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushAccessCounterCmd, "content[%s] read metaData of replica[%s]: " COUNTVALFMT), contentName.c_str(), replicaName.c_str(), counterOfDB.base.accessCount, strSince.c_str(), strLatest.c_str());
		}
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FlushAccessCounterCmd, "content[%s] accessing replica[%s] caught %s: %s"), _countToFlush.base.contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FlushAccessCounterCmd, "content[%s] accessing replica[%s] caught %s"), _countToFlush.base.contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FlushAccessCounterCmd, "content[%s] accessing replica[%s] caught exception"), _countToFlush.base.contentName.c_str(), replicaName.c_str());
	}

	if (!content)
	{
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushAccessCounterCmd, "content[%s] ignoring since replica[%s] doesn't exist"), contentName.c_str(), replicaName.c_str());
		return 1;
	}

	char buf[128];
	snprintf(buf, sizeof(buf)-2, COUNTVALFMT, counterOfDB.base.accessCount, strSince.c_str(), strLatest.c_str());
	dbOldCounterStr = buf;

	strSince = strLatest = "";
	if (ZQ::common::TimeUtil::TimeToUTC(_countToFlush.base.stampSince, buf, sizeof(buf)-2))
		strSince = buf;
	if (ZQ::common::TimeUtil::TimeToUTC(_countToFlush.base.stampLatest, buf, sizeof(buf)-2))
		strLatest = buf;
	snprintf(buf, sizeof(buf)-2, COUNTVALFMT, _countToFlush.base.accessCount, strSince.substr(5).c_str(), strLatest.substr(5).c_str());
	count2addStr = buf;

	_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushAccessCounterCmd, "content[%s] merging counter of replica[%s]"), contentName.c_str(), replicaName.c_str());
	counterOfDB = AccessRegistrarImpl::mergeAccessCount(counterOfDB, _countToFlush, stampNow - max(1000*60*30, _store._timeWinOfPopular*2));

	strSince = strLatest = "";
	if (ZQ::common::TimeUtil::TimeToUTC(counterOfDB.base.stampSince, buf, sizeof(buf)-2))
		strSince  = buf;
	if (ZQ::common::TimeUtil::TimeToUTC(counterOfDB.base.stampLatest, buf, sizeof(buf)-2))
		strLatest = buf;
	snprintf(buf, sizeof(buf)-2, COUNTVALFMT, counterOfDB.base.accessCount, strSince.substr(5).c_str(), strLatest.substr(5).c_str());
	newCountStr = buf;

	try {
		_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushAccessCounterCmd, "content[%s] flushing access count of replica[%s]: %s"), contentName.c_str(), replicaName.c_str(), newCountStr.c_str());
		snprintf(buf, sizeof(buf)-2, "%d", counterOfDB.base.accessCount);
		MAPSET(::TianShanIce::Properties, metaData, METADATA_AccessCount, buf);
		MAPSET(::TianShanIce::Properties, metaData, METADATA_AccessCountSince,  strSince);
		MAPSET(::TianShanIce::Properties, metaData, METADATA_AccessCountLatest, strLatest);

		content->setMetaData(metaData);
		_store._log(ZQ::common::Log::L_INFO, CLOGFMT(FlushAccessCounterCmd, "content[%s] updated access count of replica[%s] DB:%s + %s = %s"), contentName.c_str(), replicaName.c_str(), dbOldCounterStr.c_str(), count2addStr.c_str(), newCountStr.c_str());
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FlushAccessCounterCmd, "content[%s] accessing replica[%s] caught %s: %s"), contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FlushAccessCounterCmd, "content[%s] accessing replica[%s] caught %s"), contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_store._log(ZQ::common::Log::L_ERROR, CLOGFMT(FlushAccessCounterCmd, "content[%s] accessing replica[%s] caught exception"), contentName.c_str(), replicaName.c_str());
	}

	return 2;
}

// -----------------------------
// class ListContentsApt
// -----------------------------
int ListContentsApt::list(::TianShanIce::Storage::ContentAccessList& results, ::TianShanIce::Storage::AccessRegistrarPtr& acList, uint32 timeWin, std::string& lastError)
{
	results.clear();

	try	{
		::TianShanIce::Storage::ContentCounterList sortedList;
		if (acList)
			acList->sort(timeWin, false, 0, sortedList);

		for (int i=0; i < _maxNum && i < (int) sortedList.size(); i++)
			results.push_back(sortedList[i].base);

		return sortedList.size();
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListContentsApt() caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListContentsApt() caught  exception"); 
		lastError = buf;
	}

	return -1;
}

// -----------------------------
// class ListMissedContentsCmd
// -----------------------------
int ListMissedContentsCmd::run(void)
{
	std::string lastError;
	::TianShanIce::Storage::ContentAccessList results;
	storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListMissedContentsCmd, "list missed contents"));
	
	int allSz = list(results, _store._acMissed, _store._timeWinOfPopular, lastError);
	if (allSz >=0)
	{
		storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListMissedContentsCmd, "responding %d of %d missed contents"), results.size(), allSz);
		_amdCB->ice_response(results);
		return 0;
	}

	storelog(ZQ::common::Log::L_ERROR, CLOGFMT(ListMissedContentsCmd, "list contents caught error[%s]"), lastError.c_str());
	TianShanIce::ServerError ex("CacheStore", 500, lastError);
	_amdCB->ice_exception(ex);
	return 1;
}

// -----------------------------
// class ListHotLocalsCmd
// -----------------------------
int ListHotLocalsCmd::run(void)
{
	std::string lastError;
	::TianShanIce::Storage::ContentAccessList results;
	storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListHotLocalsCmd, "list missed contents"));
	
	int allSz = list(results, _store._acHotLocals, _store._timeWinOfPopular, lastError);
	if (allSz >=0)
	{
		storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListHotLocalsCmd, "responding %d of %d missed contents"), results.size(), allSz);
		_amdCB->ice_response(results);
		return 0;
	}

	storelog(ZQ::common::Log::L_ERROR, CLOGFMT(ListHotLocalsCmd, "list contents caught error[%s]"), lastError.c_str());
	TianShanIce::ServerError ex("CacheStore", 500, lastError);
	_amdCB->ice_exception(ex);
	return 1;
}

// -----------------------------
// class ImportContentCmd
// -----------------------------
ImportContentCmd::ImportContentCmd(CacheStoreImpl& store, const std::string& contentName)
    : CacheCmd(store), _contentName(contentName)
{
	ThreadRequest::setPriority(DEFAULT_REQUEST_PRIO +10); // lower a bit priority for this type of ThreadRequest
}

int ImportContentCmd::run(void)
{
	TianShanIce::Properties params;
	try {
		Ice::Current c;
		char buf[32];
		snprintf(buf, sizeof(buf), "ImportContentCmd(%p)", this);
		MAPSET(::Ice::Context, c.ctx, "signature", buf);
		_store.cacheContent(_contentName, NULL, params, c);
		return 0;
	}
	catch(...) {}

	return 1;
}

}} // namespace
