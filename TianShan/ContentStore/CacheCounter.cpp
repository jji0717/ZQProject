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
// Ident : $Id: CacheCounter.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheCounter.cpp $
// 
// 13    7/24/12 2:47p Hui.shao
// stat counters on hitrate
// 
// 12    7/20/12 2:44p Hui.shao
// 
// 11    7/19/12 7:36p Hui.shao
// adjusted counter list compress logic
// 
// 10    6/12/12 2:12p Hui.shao
// 
// 9     6/11/12 5:22p Li.huang
// 
// 8     6/06/12 4:03p Hui.shao
// to print the count result in the log
// 
// 7     5/10/12 6:50p Hui.shao
// kick off cache missed content per popularity
// 
// 6     4/11/12 10:37a Li.huang
// 
// 5     3/16/12 4:52p Hui.shao
// 
// 4     1/19/12 12:09p Hui.shao
// 
// 3     1/18/12 1:50p Hui.shao
// linkable
// 
// 2     1/16/12 10:35p Hui.shao
// defined the counter and topfolder at ice level
// 
// 1     1/13/12 2:24p Hui.shao
// splitted from CacheStoreImpl.cpp
// ===========================================================================
#include "CacheStoreImpl.h"

#include "MD5CheckSumUtil.h"
#include "CacheCmds.h"

#include "ContentState.h"
#include "urlstr.h"

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#else
	#include <sys/stat.h>
#endif
	#include <math.h>
};

namespace ZQTianShan {
namespace ContentStore {

// -----------------------------
// class AccessRegistrarImpl
// -----------------------------
const char* AccessRegistrarImpl::ContentAccessStr(const TianShanIce::Storage::ContentAccess& ac, char* buf, int maxLen)
{
	if (NULL == buf || maxLen<=4)
		return "";

	char *p=buf;
	snprintf(p, buf +maxLen-4 -p, "ContentAccess[%s;", ac.contentName.c_str()); p +=strlen(p);
	snprintf(p, buf +maxLen-4 -p, "%d;", ac.accessCount); p +=strlen(p);
	ZQ::common::TimeUtil::TimeToUTC(ac.stampSince, p, buf +maxLen-4 -p, true);  p +=strlen(p);
	*p++='~'; *p='\0';
	ZQ::common::TimeUtil::TimeToUTC(ac.stampLatest, p, buf +maxLen-4 -p, true); p +=strlen(p);
	*p++=']'; *p='\0';
	return buf;
}

::TianShanIce::Storage::ContentAccess AccessRegistrarImpl::count(const ::std::string& contentName, const ::std::string& subfile, ::Ice::Int countToAdd, const ::Ice::Current& c)
{
	static ::TianShanIce::Storage::ContentAccess acNil = {"", 0,0,0};
	WLock sync(*this);
	int64 stampNow = ZQ::common::now();
	::TianShanIce::Storage::ContentCounterMap::iterator it = counters.find(contentName);
	if (counters.end() == it)
	{
		::TianShanIce::Storage::AccessCounter ac;
		ac.base.contentName = contentName;
		ac.base.accessCount = 0;
		ac.base.stampSince = stampNow;
		// ac.base.stampLatest = stampNow;

		std::string top, leaf;
		ac.folderName = CacheStoreImpl::_content2FolderName(contentName, top, leaf);
		ac.knownSince = stampNow;
		ac.fileSizeMB = 0;
		ac.localState = TianShanIce::Storage::csNotProvisioned;

		MAPSET(::TianShanIce::Storage::ContentCounterMap, counters, contentName, ac);
		it = counters.find(contentName);
	}

	if (counters.end() == it)
	{
		// better to log this exception case here
		return acNil;
	}

	it->second.base.stampLatest = stampNow;
	if (countToAdd >0)
		it->second.base.accessCount += countToAdd;

	return it->second.base;
}

void AccessRegistrarImpl::set(const ::TianShanIce::Storage::AccessCounter& counterToOverwrite, const ::Ice::Current& c)
{
	if (counterToOverwrite.base.contentName.empty())
		return;

	WLock sync(*this);
	// counterToOverwrite.base.stampLatest = ZQ::common::now();
	MAPSET(::TianShanIce::Storage::ContentCounterMap, counters, counterToOverwrite.base.contentName, counterToOverwrite);
}

void AccessRegistrarImpl::erase(const ::std::string& contentName, const ::Ice::Current& c)
{
	WLock sync(*this);
	counters.erase(contentName);
}

bool AccessRegistrarImpl::get(const ::std::string& contentName, ::TianShanIce::Storage::AccessCounter& counter, const ::Ice::Current& c)
{
	RLock sync(*this);
	::TianShanIce::Storage::ContentCounterMap::iterator it = counters.find(contentName);
	if (counters.end() == it)
		return false;

	counter = it->second;
	return true;
}

::TianShanIce::Storage::AccessCounter AccessRegistrarImpl::mergeAccessCount(const ::TianShanIce::Storage::AccessCounter& acA, const ::TianShanIce::Storage::AccessCounter& acB, int64 newWindowFrom)
{
	const ::TianShanIce::Storage::AccessCounter* acOld = &acA, *acNew = &acB;
	
	//swap to keep acOld, acNew true per stampLatest
	if (acOld->base.stampLatest > acNew->base.stampLatest)
	{
		const ::TianShanIce::Storage::AccessCounter* acTmp = acOld;
		acOld = acNew;
		acNew = acTmp;
	}

	::TianShanIce::Storage::AccessCounter ac = *acOld;

	do {
		// non-intersect case 1
		if (newWindowFrom >= acNew->base.stampLatest)
		{
			ac.base.accessCount = 0;
			ac.base.stampLatest = ac.base.stampSince  = newWindowFrom;
			break;
		}

		// non-intersect case 2
		if (newWindowFrom >= acOld->base.stampLatest)
		{
			ac = *acNew;
			break;
		}

		// non-intersect case 3
		if (acOld->base.stampSince >= acOld->base.stampLatest)
		{
			ac = *acNew;
			ac.base.accessCount += acOld->base.accessCount /2; // round down for old count
			if (acOld->base.stampLatest >0 && acOld->base.stampLatest < acNew->base.stampSince)
				ac.base.stampSince = acOld->base.stampLatest;
			break;
		}

		// non-intersect case 4
		if (acOld->base.stampLatest <= acNew->base.stampSince)
		{
			if (newWindowFrom <= acOld->base.stampSince)
				ac.base.accessCount = acOld->base.accessCount + acNew->base.accessCount;
			else
			{
				ac.base.accessCount = acOld->base.accessCount * (acOld->base.stampLatest - newWindowFrom) / (acOld->base.stampLatest - acOld->base.stampSince);
				ac.base.accessCount += acNew->base.accessCount;
			}

			ac.base.stampLatest = acNew->base.stampLatest;
			ac.base.stampSince  = newWindowFrom;
			break;
		}

		// intersect case
		int64 overlapWin = acOld->base.stampLatest - acNew->base.stampSince;

		Ice::Int overLapCountOfOld = acOld->base.accessCount * overlapWin / (acOld->base.stampLatest - acOld->base.stampSince);
		Ice::Int overLapCountOfNew = acNew->base.accessCount * overlapWin / (acNew->base.stampLatest - acNew->base.stampSince);

		ac.base.accessCount = (Ice::Int) (acOld->base.accessCount + acNew->base.accessCount - (overLapCountOfOld + overLapCountOfNew) /2);
		ac.base.stampLatest = acNew->base.stampLatest;
		ac.base.stampSince  = acOld->base.stampSince;

	} while (0);


	// adjust by the specified time window
	if (newWindowFrom > ac.base.stampSince && ac.base.stampLatest > ac.base.stampSince)
	{
		ac.base.accessCount -= ac.base.accessCount * (newWindowFrom - ac.base.stampSince) / (ac.base.stampLatest - ac.base.stampSince);
		ac.base.stampSince  = newWindowFrom;
	}

	return ac;
}

::Ice::Int AccessRegistrarImpl::compress(::Ice::Int windowSize, ::Ice::Int flushWinSize, ::Ice::Int& reqsInWindow, ::TianShanIce::Storage::ContentCounterList& listToFlush, ::TianShanIce::Storage::ContentCounterList& listEvicted, const ::Ice::Current& c)
{
	WLock sync(*this);

	if (windowSize <=0) // no thing to do if no time window is specified
		return counters.size();

	int64 stampNow = ZQ::common::now();
	int64 stampExpired = stampNow - windowSize *4;
	int64 stampSince   = stampNow - windowSize;
	int64 stampToFlush = stampNow - flushWinSize;
	reqsInWindow = 0;

	for (::TianShanIce::Storage::ContentCounterMap::iterator it = counters.begin(); it != counters.end(); it++)
	{
		if (it->second.base.stampLatest < stampExpired || it->second.base.accessCount <=0)
		{
			// no more hits in the recent time frame [stampExpired, now], evict this content and stop attention on it
			listEvicted.push_back(it->second);
			continue;
		}

		int64 dur = it->second.base.stampLatest - it->second.base.stampSince;
		if (dur > windowSize)
			reqsInWindow += it->second.base.accessCount * windowSize / dur;
		else reqsInWindow += it->second.base.accessCount;

		if (flushWinSize >0 && it->second.base.stampLatest > stampToFlush)
		{
			// compress those hits of [-INFINIT, stampSince], then flush into ContentDB
			::TianShanIce::Storage::AccessCounter old   = it->second;
			::TianShanIce::Storage::AccessCounter dummy = old;
			dummy.base.accessCount = 0;
			dummy.base.stampSince  = dummy.base.stampLatest;
			it->second = mergeAccessCount(old, dummy, stampSince);

			listToFlush.push_back(old);
			continue;
		}
	}

	for (::TianShanIce::Storage::ContentCounterList::iterator itL = listEvicted.begin(); itL < listEvicted.end(); itL++)
		counters.erase(itL->base.contentName);

	return counters.size();
}

static bool lessPopular(::TianShanIce::Storage::AccessCounter i, ::TianShanIce::Storage::AccessCounter j)
{
	int diff = i.base.accessCount - j.base.accessCount;
	if (0 != diff)
		return (diff <0);
	
	return (i.base.stampLatest < j.base.stampLatest);
}

void AccessRegistrarImpl::sort(::Ice::Int windowSize, bool unpopular1st, int minCount, ::TianShanIce::Storage::ContentCounterList& result, const ::Ice::Current& c)
{
	int64 stampNow   = ZQ::common::now();
	int64 stampSince   = stampNow - windowSize;

	::TianShanIce::Storage::ContentCounterList tmplist;
	{
		// make a copy from the map for sorting
		::TianShanIce::Storage::AccessCounter dummyAc;
		dummyAc.base.accessCount =0;
		dummyAc.base.stampSince  =stampNow - windowSize;
		dummyAc.base.stampLatest =stampSince;

		RLock sync(*this);
		for (::TianShanIce::Storage::ContentCounterMap::iterator it = counters.begin(); it != counters.end(); it++)
		{
			dummyAc.base.contentName = it->second.base.contentName;
			::TianShanIce::Storage::AccessCounter compressedAC = mergeAccessCount(it->second, dummyAc, stampSince);

			if (minCount >0 && compressedAC.base.accessCount < minCount)
				continue; // ignore those counter that hasn't reached minCount yet

			tmplist.push_back(compressedAC);
		}
	}

	::std::sort(tmplist.begin(), tmplist.end(), lessPopular);
	if (!unpopular1st)
		::std::reverse(tmplist.begin(), tmplist.end());

	// export with the original counter data
	result.clear();
	RLock sync(*this);
	for (::TianShanIce::Storage::ContentCounterList::iterator itL = tmplist.begin(); itL < tmplist.end(); itL++)
	{
		::TianShanIce::Storage::ContentCounterMap::iterator it = counters.find(itL->base.contentName);
		if (counters.end() == it)
			continue;

		result.push_back(it->second);
	}
}

::Ice::Int AccessRegistrarImpl::size(const ::Ice::Current& c)
{
	RLock sync(*this);
	return counters.size();
}

}} // namespace
