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
// Ident : $Id: CacheFolder.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheFolder.cpp $
// 
// 45    7/10/13 3:16p Hui.shao
// 
// 44    12/19/12 4:20p Hui.shao
// 
// 43    9/19/12 3:20p Hui.shao
// 1. exclude those content that has METADATA_PersistentTill specified AND
// not yet met
// 2. issue a SyncCmd on leaf folder if its last sync has been older than
// 2days
// 
// 42    9/05/12 2:55p Hui.shao
// refreshing ContentDB that may be out of date
// 
// 41    9/05/12 12:06p Hui.shao
// flag logging
// 
// 40    8/09/12 5:15p Hui.shao
// NULL pointer test for linux
// 
// 39    8/09/12 12:21p Hui.shao
// corrected logging
// 
// 37    8/06/12 8:36p Hui.shao
// 
// 36    8/06/12 12:50p Hui.shao
// store::OnTimer to call ensureSpace()
// 
// 35    8/03/12 2:43p Hui.shao
// 
// 34    8/02/12 3:45p Hui.shao
// 
// 33    7/27/12 10:27a Hui.shao
// changed maxSpace from GB to MB
// 
// 32    7/26/12 7:20p Hui.shao
// enh#16577 to limit max diskspace via configuration
// 
// 31    7/25/12 1:49p Hui.shao
// 
// 30    7/25/12 12:21p Hui.shao
// 
// 29    7/19/12 5:47p Hui.shao
// 
// 28    7/19/12 5:35p Hui.shao
// added and refer to store-wide rootVolName
// 
// 27    6/28/12 2:38p Hui.shao
// 
// 26    6/28/12 1:16p Hui.shao
// 
// 25    6/27/12 6:40p Li.huang
// 
// 24    6/26/12 5:26p Hui.shao
// 
// 23    6/26/12 5:03p Hui.shao
// FWU to faster find other folders for free space
// 
// 22    6/26/12 2:55p Hui.shao
// 
// 21    6/26/12 1:22p Hui.shao
// 
// 20    6/25/12 3:54p Li.huang
// 
// 19    6/25/12 3:41p Hui.shao
// more logs at refreshing unpopulars
// 
// 18    6/25/12 2:56p Li.huang
// 
// 17    6/20/12 6:40p Hui.shao
// 
// 16    6/20/12 4:56p Li.huang
// 
// 15    6/20/12 3:12p Hui.shao
// 
// 14    6/15/12 3:05p Hui.shao
// folder space
// 
// 13    6/13/12 6:20p Hui.shao
// 
// 12    5/10/12 6:50p Hui.shao
// 
// 11    5/08/12 11:14a Build
// 
// 10    4/27/12 3:37p Build
// 
// 9     4/27/12 3:24p Hui.shao
// 
// 8     4/26/12 6:31p Hui.shao
// 
// 7     4/25/12 4:36p Hui.shao
// drafted most unpopular list of TopFolder
// 
// 6     4/11/12 10:37a Li.huang
// 
// 5     1/18/12 1:50p Hui.shao
// linkable
// 
// 4     1/17/12 11:35a Hui.shao
// linkable
// 
// 3     1/16/12 10:35p Hui.shao
// defined the counter and topfolder at ice level
// 
// 2     1/16/12 1:22p Hui.shao
// 
// 1     1/13/12 2:24p Hui.shao
// splitted from CacheStoreImpl.cpp
// ===========================================================================
#include "CacheStoreImpl.h"

#include "MD5CheckSumUtil.h"
#include "CacheCmds.h"

#include "ContentState.h"
#include "urlstr.h"
#include "ContentCmds.h"

#include <set> // for multiset

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#else
	#include <sys/stat.h>
#endif
	#include <math.h>
};

#ifndef min
#  define min(_X, _Y) ((_X>_Y)?_Y:_X)
#endif

namespace ZQTianShan {
namespace ContentStore {

#define storelog     (_store._log)

// -----------------------------
// class TopFolderImpl
// -----------------------------
TopFolderImpl::TopFolderImpl(CacheStoreImpl& store)
	:_store(store), _freeSpaceMB(0), _totalSpaceMB(0)
{
}

TopFolderImpl::~TopFolderImpl()
{
}

::Ice::Int TopFolderImpl::unpopularSize(const ::Ice::Current& c)
{
	RLock sync(*this);
	if (unpopulars)
		return unpopulars->size(c);
	else return 0;
}

bool TopFolderImpl::eraseFromUnpopular(const ::std::string& fullContentName, const ::Ice::Current& c)
{
	if (0 != fullContentName.compare(0, ident.name.length(), ident.name) || !unpopulars)
		return false; // not belongs to this folder

	std::string contentSearchFor = fullContentName.substr(ident.name.length());

	WLock sync(*this);
	TianShanIce::Storage::AccessCounter ac;
	if (!unpopulars->get(contentSearchFor, ac))
		return false;

	unpopulars->erase(contentSearchFor);

	// evictor this from the unpopular list
	storelog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheFolder, "tf[%s] OnContentHit() evicting content[%s] from unpopular list"), ident.name.c_str(), fullContentName.c_str());
	for (::TianShanIce::StrValues::iterator itQ= unpopularQueue.begin(); itQ < unpopularQueue.end(); itQ++)
	{
		if (contentSearchFor == *itQ)
		{
			unpopularQueue.erase(itQ);
			break;
		}
	}

	try {
		FlushAccessCounterCmd* pCmd = FlushAccessCounterCmd::newCmd(_store, ac);
		if (pCmd)
			pCmd->exec();
	}
	catch (...)
	{}

	_store._watchDog.watch(ident, 1000);
	return true;
}
	
/// returns the content names that have been deleted
::TianShanIce::StrValues TopFolderImpl::freeForSpace(::Ice::Long wishedSpaceMB, ::Ice::Long& freedMB, const ::Ice::Current& c)
{
	freedMB =0;
	::TianShanIce::StrValues contentsDeleted;

	storelog(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, freeForSpace, "tf[%s] wishedSpaceMB[%lld] unpopularSize[%d]"), ident.name.c_str(), wishedSpaceMB, unpopularQueue.size());

	WLock sync(*this);
	int cDeleted =0;

	for (cDeleted =0;  unpopularQueue.size() >0 && freedMB < wishedSpaceMB; cDeleted++)
	{
		::TianShanIce::StrValues::iterator itQ= unpopularQueue.begin();
		std::string contentName = *itQ;
		unpopularQueue.erase(itQ);

		TianShanIce::Storage::AccessCounter ac;
		if (!unpopulars || !unpopulars->get(contentName, ac))
			continue;
		
		unpopulars->erase(contentName);

		try {
			storelog(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, freeForSpace, "tf[%s] destorying content[%s]"), ident.name.c_str(), contentName.c_str());
			std::string fullContentName = _store._rootVolName + LOGIC_FNSEPS + ac.folderName + LOGIC_FNSEPS + contentName;

			::TianShanIce::Properties metaData;
			MAPSET(::TianShanIce::Properties, metaData, SYS_PROP(DestroySignature), "CacheStore::TopFolder::freeForSpace()");

			::TianShanIce::Storage::UnivContentPrx content = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(_store._contentStore.openContentByFullname(fullContentName, c));
			content->setMetaData(metaData);
			content->destroy2(true);

			freedMB += ac.fileSizeMB;
			contentsDeleted.push_back(contentName);
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			storelog(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, freeForSpace, "tf[%s] openContentByFullname(%s) caught ObjectNotExistException, continue"), 
				ident.name.c_str(), contentName.c_str());
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			storelog(ZQ::common::Log::L_WARNING,FLOGFMT(CacheFolder, freeForSpace, "tf[%s] openContentByFullname(%s) caught %s: %s"), 
				ident.name.c_str(), contentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			storelog(ZQ::common::Log::L_WARNING,FLOGFMT(CacheFolder, freeForSpace, "tf[%s] openContentByFullname(%s) caught %s"), 
				ident.name.c_str(), contentName.c_str(), ex.ice_name().c_str());
		}
		catch (...) 
		{
			storelog(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, freeForSpace, "tf[%s] openContentByFullname(%s) caught exception"),
				ident.name.c_str(), contentName.c_str());
		}
	}

	std::string cnList;
	for (::TianShanIce::StrValues::iterator itCn = contentsDeleted.begin(); itCn < contentsDeleted.end(); itCn++)
		cnList += std::string("[") + *itCn + "],";

	storelog(ZQ::common::Log::L_INFO, FLOGFMT(CacheFolder, freeForSpace, "tf[%s] tried %d contents, deleted %d, freed %lldMB for %lldMB: %s"),
		ident.name.c_str(), cDeleted, contentsDeleted.size(), freedMB, wishedSpaceMB, cnList.c_str());

//	checkUnpopular();
	_store._watchDog.watch(ident, 1000);
	return contentsDeleted;
}

struct LessAC
{
     bool operator()(const ::TianShanIce::Storage::AccessCounter& A, const ::TianShanIce::Storage::AccessCounter& B)
     {
		 return (A.base.accessCount < B.base.accessCount);
     }
};

typedef std::multiset<TianShanIce::Storage::AccessCounter, LessAC> SortedAC;

void TopFolderImpl::doRefreshUnpopular(const ::Ice::Current& c)
{
	::TianShanIce::Storage::AccessRegistrarPtr pNewUnpopulars = new AccessRegistrarImpl(_store, ident.name +"#unpopulars");
	// step 1. enumerate all the files under this top folder
	if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
		_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] scanning subfolers for unpopular files, sig[%s]"),	ident.name.c_str(), invokeSignature(c).c_str());

	::TianShanIce::StrValues fullFilenames;
	::TianShanIce::Storage::FolderPrx folder;

	try	{
		if (!_folder)
			_openFolder(c);

		folder = ::TianShanIce::Storage::FolderPrx::uncheckedCast(_folder->ice_timeout(60*1000));
		::TianShanIce::StrValues fns = folder->listContent("*");
		for (::TianShanIce::StrValues::iterator it = fns.begin(); it < fns.end(); it++)
			fullFilenames.push_back(ident.name + LOGIC_FNSEPS + *it);
	}
	catch (const Ice::Exception& ex)
	{
		if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
			_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] list content of topfolder caught exception[%s]"), ident.name.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
			_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] list content of topfolder caught exception"), ident.name.c_str());
	}

	int cSubFolder=0; size_t i=0;
	::TianShanIce::IValues leafConts;

	int maxLoad = max(_store._thisDescriptor.desc.loadStream, _store._thisDescriptor.desc.loadImport);
	maxLoad = max(maxLoad, _store._thisDescriptor.desc.loadCacheWrite);
	int64 stampNow = ZQ::common::now();

	::TianShanIce::Properties dummyLastSyncStampMD;
	{
		char tmpbuf[64];
		ZQTianShan::TimeToUTC(stampNow - 60*60*1000, tmpbuf, sizeof(tmpbuf) -2);
		MAPSET(::TianShanIce::Properties, dummyLastSyncStampMD, SYS_PROP(StampLastSyncWithFileSystem), tmpbuf);
	}

	::Ice::Identity identF;
	identF.category = DBFILENAME_Volume;

	for (i=0; i < CACHE_FOLDER_SIZE; i++)
	{
		leafConts.push_back(0); // insert the counter to ensure the vector size first
		char leafName[32];
		snprintf(leafName, sizeof(leafName)-2, LEAF_FOLDER_FMT, i);
		std::string folderName = ident.name + LOGIC_FNSEPS + leafName;
		try	{
			folder = ::TianShanIce::Storage::FolderPrx::uncheckedCast(_store._contentStore.openFolderEx(folderName, false, 0, c)->ice_timeout(60*1000));

			::TianShanIce::StrValues fns = folder->listContent("*");
			for (::TianShanIce::StrValues::iterator it = fns.begin(); it < fns.end(); it++)
				fullFilenames.push_back(folderName + LOGIC_FNSEPS + *it);

			leafConts[i] = fns.size();
			cSubFolder++;

			if (maxLoad < MAX_LOAD /20) // pretty idle
			{
				try {
					::TianShanIce::Storage::VolumeExPrx vol = ::TianShanIce::Storage::VolumeExPrx::uncheckedCast(folder);
					::TianShanIce::Storage::VolumeInfo vi = vol->getInfo();
					int64 stampLastSync =0;
					std::string strLastSync;
					::TianShanIce::Properties::iterator itMD = vi.metaData.find(SYS_PROP(StampLastSyncWithFileSystem));
					if (vi.metaData.end() != itMD)
					{
						strLastSync = itMD->second;
						stampLastSync = ZQ::common::TimeUtil::ISO8601ToTime(strLastSync.c_str());
					}

					if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
						_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "folder[%s]'s lastSyncFs[%s]"), folderName.c_str(), strLastSync.c_str());

					if (stampLastSync >0 && stampLastSync < stampNow - 2*24*60*60*1000) // older than 2days no sync
					{
						// issue a SyncFSCmd to refresh sync
						vol->setMetaData(dummyLastSyncStampMD);
						_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "folder[%s] long time no sync lastSyncFs[%s], issue a request to sync"), folderName.c_str(), strLastSync.c_str());
						(new ZQTianShan::ContentStore::SyncFSCmd(_store._contentStore, identF))->execute();
					}
				}
				catch(...) {}
			}
		}
		catch (const Ice::ObjectNotExistException&)
		{
		}
		catch (const IceUtil::NullHandleException&)
		{
		}
		catch (const Ice::Exception& ex)
		{
			_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] list contents of leaf[%s] caught exception[%s]"), ident.name.c_str(), leafName, ex.ice_name().c_str());
		}
		catch (...)
		{
			_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] list contents of leaf[%s] caught exception"), ident.name.c_str(), leafName);
		}
	}

	contentSubtotal = fullFilenames.size();
	if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
		_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] %d files of %d subfolers found, looking for most unpopulars"), ident.name.c_str(), contentSubtotal, cSubFolder);

	// step 2. read each files for the metaData unpopular and file sizes
	int maxACCollected = 999999999; // initialize with a big value
	SortedAC unpopularSet; // as a sorted container
	stampNow = ZQ::common::now();
	int64 usedMB =0;
	for (i =0; i < fullFilenames.size(); i++)
	{
		::TianShanIce::Storage::AccessCounter ac;
		// chop for the short contentName
		size_t pos = fullFilenames[i].find_last_of(LOGIC_FNSEPS);
		ac.base.contentName = fullFilenames[i].substr(pos+1);
		ac.folderName = fullFilenames[i].substr(0, pos);

		ac.base.accessCount =0;
		ac.base.stampSince  = ac.base.stampLatest =0;
		ac.knownSince  =0;
		ac.fileSizeMB  =0;

		try {
			// case 1 if the content is in the hot list, no need to read it from the ContentStore DB, skip
			if (_store._acHotLocals->get(ac.base.contentName, ac, c))
			{
				if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
					_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] ignored Content[%s] that is known as a hot local"), ident.name.c_str(), fullFilenames[i].c_str());
				continue;
			}

			::TianShanIce::Storage::UnivContentPrx content = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_store._contentStore.openContentByFullname(_store._rootVolName + LOGIC_FNSEPS + fullFilenames[i], c));
			if (!content)
			{
				if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
					_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] failed to open Content[%s]"), ident.name.c_str(), fullFilenames[i].c_str());
				continue;
			}

			// reading the content metadata
			::TianShanIce::Properties metaData = content->getMetaData();
			std::string knownSinceStr;

			// exclude those content that has METADATA_PersistentTill specified AND not yet met
			::TianShanIce::Properties::iterator itMD = metaData.find(METADATA_PersistentTill);
			if (metaData.end() != itMD)
			{
				int64 stampPersistentTill = ::ZQ::common::TimeUtil::ISO8601ToTime(itMD->second.c_str());

				if (stampPersistentTill >0 && stampNow < stampPersistentTill)
				{
					if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
					{
						_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] excludes content[%s] since its PersistentTill[%s] has not yet been met"), ident.name.c_str(),
						            fullFilenames[i].c_str(), itMD->second.c_str());
					}

					continue;
				}
			}

			// reading the metadatas about AccessCount
			itMD = metaData.find(METADATA_AccessCount);
			if (metaData.end() != itMD)
				ac.base.accessCount = atol(itMD->second.c_str());

			itMD = metaData.find(METADATA_AccessCountSince);
			if (metaData.end() != itMD && itMD->second.length() >0)
				ac.base.stampSince = ::ZQ::common::TimeUtil::ISO8601ToTime(itMD->second.c_str());

			itMD = metaData.find(METADATA_AccessCountLatest);
			if (metaData.end() != itMD && itMD->second.length() >0)
				ac.base.stampLatest = ::ZQ::common::TimeUtil::ISO8601ToTime(itMD->second.c_str());

			itMD = metaData.find(METADATA_FileSize);
			if (metaData.end() != itMD && itMD->second.length() >0)
				ac.fileSizeMB = (atol(itMD->second.c_str()) + 1024*1024/2) /1024/1024; // round to MegaBytes

			itMD = metaData.find(SYS_PROP(State));
			if (metaData.end() != itMD && itMD->second.length() >0)
				ac.localState = ContentStateBase::stateId(itMD->second.c_str()) ;

			itMD = metaData.find(SYS_PROP(StampCreated));
			if (metaData.end() != itMD && itMD->second.length() >0)
			{
				knownSinceStr = itMD->second;
				ac.knownSince = ::ZQ::common::TimeUtil::ISO8601ToTime(knownSinceStr.c_str());
			}

			if (ac.knownSince <=0)
				ac.knownSince = stampNow;

			char buf[200];
			if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
			{
				_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] read from DB: %s; since[%s]; size[%lld]MB; state[%s(%d)]"), ident.name.c_str(),
							AccessRegistrarImpl::ContentAccessStr(ac.base, buf, sizeof(buf)-2), knownSinceStr.c_str(), ac.fileSizeMB, ContentStateBase::stateStr(ac.localState), ac.localState);
			}

			///////////////////////////////////////////////////////////

			// inserting it into pNewUnpopulars->counters if necessary
			switch (ac.localState)
			{
			case ::TianShanIce::Storage::csNotProvisioned: 
			case ::TianShanIce::Storage::csProvisioning:
			case ::TianShanIce::Storage::csProvisioningStreamable:
				// if a content failed to complete caching within DOCACHE_TIMEOUT, make it eligible to be in the unpopular list
				if (ac.knownSince < stampNow - DOCACHE_TIMEOUT)
					break;
				else continue;

			case ::TianShanIce::Storage::csInService:
				// all are eligible to be in the unpopular list
				break;

			case ::TianShanIce::Storage::csOutService:
			case ::TianShanIce::Storage::csCleaning:
				// no need to check eligiblity to be in the unpopular list
				continue;
			}
			
			if ((int) pNewUnpopulars->counters.size() < maxUnpopular)
			{
				// case 2. counters size hasn't reached the limitation yet, insert directly
				MAPSET(::TianShanIce::Storage::ContentCounterMap, pNewUnpopulars->counters, ac.base.contentName, ac);
				if (maxACCollected >= ac.base.accessCount)
				{
					maxACCollected = ac.base.accessCount;
					unpopularSet.insert(ac);
					usedMB += ac.fileSizeMB;
				}

				if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
					_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] content[%s] inserted into unpopular list, ac[%d] maxCollected[%d]"), ident.name.c_str(), ac.base.contentName.c_str(), ac.base.accessCount, maxACCollected);

				continue;
			}
			
			// case 3. accessCount is no less than know max of collected, skip
			if (ac.base.accessCount >= maxACCollected || unpopularSet.empty())
			{
				if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
					_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] content[%s] skipped since its ac[%d] >= maxCollected[%d]"), ident.name.c_str(), ac.base.contentName.c_str(), ac.base.accessCount, maxACCollected);
				continue;
			}

			// case 4. accessCount is less than collected, exchange with the most biggest AC of the sorted unpopularSet
			SortedAC::iterator itBiggest = unpopularSet.end(); itBiggest--;
			if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
			{
				_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] content[%s][%d] exchanging with content[%s][%d] that is least unpopular in the list"), ident.name.c_str(),
				      ac.base.contentName.c_str(), ac.base.accessCount, itBiggest->base.contentName.c_str(), itBiggest->base.accessCount);
			}
			
			maxACCollected = ac.base.accessCount;

			// remove the old one
			usedMB -= itBiggest->fileSizeMB;
			pNewUnpopulars->counters.erase(itBiggest->base.contentName);
			unpopularSet.erase(itBiggest);

			// add the new one
			usedMB += ac.fileSizeMB;
			MAPSET(::TianShanIce::Storage::ContentCounterMap, pNewUnpopulars->counters, ac.base.contentName, ac);
			unpopularSet.insert(ac);
		}
		catch (const Ice::Exception& ex)
		{
			_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] checking content[%s] caught exception[%s]"), ident.name.c_str(), ac.base.contentName.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] checking content[%s] caught exception"), ident.name.c_str(), ac.base.contentName.c_str());
		}
	}

	if (pNewUnpopulars->counters.size() <=0)
		maxACCollected = 0;

	int minACCollected =0;
	if (unpopularSet.size() >0)
		minACCollected = unpopularSet.begin()->base.accessCount;

	_store._log(ZQ::common::Log::L_INFO, FLOGFMT(CacheFolder, doRefreshUnpopular, "tf[%s] %d out of %d contents w/%lldMB build up the unpopular list, accessCount range[%d~%d]"), ident.name.c_str(), pNewUnpopulars->counters.size(), contentSubtotal, usedMB, minACCollected, maxACCollected);

	CacheStoreImpl::FWU fwu;
	fwu.topFolderName  = ident.name;
	fwu.minAccessCount = minACCollected;
	fwu.usedMB = usedMB;
	fwu.unpopularSize = pNewUnpopulars->size();
	_store._updateFWU(fwu);

	WLock sync(*this);
	unpopulars = pNewUnpopulars;
	contentsOfleaves =leafConts;
	usedSpaceMB = usedMB;
	stampLastRefresh = ZQ::common::now();

	// copy the sorted unpopularSet to unpopularQueue from access count least to max
	unpopularQueue.clear();
	for (SortedAC::iterator itUnpopular = unpopularSet.begin(); itUnpopular != unpopularSet.end(); itUnpopular++)
		unpopularQueue.push_back(itUnpopular->base.contentName);
}

TianShanIce::Storage::FolderPrx TopFolderImpl::_openFolder(const ::Ice::Current& c)
{
	if (!_folder && _store._rootVol)
	{
		try {
			_folder = _store._contentStore.openFolderEx(ident.name, true, 0, c);
			storelog(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, _openFolder, "tf[%s] folder opened"), ident.name.c_str());
		}
		catch(...) {}
	}

	return _folder;
}

#define FOLDER_MAX_WATCH_INTERVAL (1000*60*60)

void TopFolderImpl::OnTimer(const ::Ice::Current& c)
{
	int64 stampNow = ZQ::common::now();

	_store._watchDog.watch(ident, FOLDER_MAX_WATCH_INTERVAL); // dummy watch to make sure the folder will not be missed from watching
	uint32 nextSleep = 1000*60*1; // start with 1min

	WLock sync(*this);
	_openFolder(c);

	// doRefreshUnpopular(c);

#ifndef INDIVIDUAL_FOLDER_QUOTA

	// assuming all the cache folders are under a same volume, and share the same space limitation
	_store._readStoreSpace(_freeSpaceMB, _totalSpaceMB);
#else
#error not supported yet
#endif // INDIVIDUAL_FOLDER_QUOTA

#pragma message (__TODO__ "step 1. if the ContentStore::Folder has long time no sync plus it is not the peak time beyond a day, drive to perform ContentStore::Folder::sync() in a separate thread")

	// step 2. kick off to refresh the unpopluar list
	try {
		int cWished = min(contentSubtotal/5, maxUnpopular/2); // 20% of total or 50% of _maxUnpopular

		// adjust the size threshold if there is few space
		if (_totalSpaceMB >0 && _freeSpaceMB < _totalSpaceMB/2 && stampLastRefresh < stampNow - FOLDER_MAX_WATCH_INTERVAL/4)
			cWished = maxUnpopular/2;

		if (cWished < CACHE_MIN_UnpopularSize)
			cWished = CACHE_MIN_UnpopularSize;

		if (cWished > contentSubtotal)
			cWished = contentSubtotal;

		// test the curret unpopularlist, see if it need a refresh
		int listSize = 0;
		if (unpopulars)
			listSize = unpopulars->size(c);

		if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
			_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheFolder, OnTimer, "tf[%s] unpopularSize[%d/%d] wished[%d/%d], space[%lld/%lld]MB"), ident.name.c_str(),
			       listSize, contentSubtotal, cWished, maxUnpopular, _freeSpaceMB, _totalSpaceMB);

		if (stampLastRefresh < stampNow - FOLDER_MAX_WATCH_INTERVAL/2 || listSize < cWished)
		{
			storelog(ZQ::common::Log::L_INFO, FLOGFMT(CacheFolder,OnTimer, "tf[%s] refreshing unpopularList[%d/%d] wished[%d/%d], space[%lld/%lld]MB"), ident.name.c_str(),
				listSize, contentSubtotal, cWished, maxUnpopular, _freeSpaceMB, _totalSpaceMB);
			
			RefreshUnpopularListCmd* pCmd = RefreshUnpopularListCmd::newCmd(_store, *this);
			if (0 == pCmd)
			{
				if (_store._flags & CacheStoreImpl::sfLoggingUnpopularScan)
					_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheFolder, OnTimer, "tf[%s] failed to new RefreshUnpopularListCmd"), ident.name.c_str());
			}
			else
			{
				pCmd->exec();
				stampLastRefresh = stampNow;

				// refreshing was supposed to take long time to finish, and the RefreshUnpopularListCmd would adjust to smaller watching after completion
				nextSleep = FOLDER_MAX_WATCH_INTERVAL; 
			}
		}
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, OnTimer, "tf[%s] failed to create RefreshUnpopularListCmd, caught %s: %s"), 
			ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, OnTimer,"tf[%s] failed to create RefreshUnpopularListCmd, caught %s"), 
			ident.name.c_str(), ex.ice_name().c_str());
	}
	catch( ...)
	{
		storelog(ZQ::common::Log::L_ERROR,  FLOGFMT(CacheFolder,  OnTimer,"tf[%s] failed to create RefreshUnpopularListCmd, caught exception"), 
			ident.name.c_str());
	}

	_store._watchDog.watch(ident, nextSleep);
}

bool TopFolderImpl::getSpaceUsage(::Ice::Long& totalMB, ::Ice::Long& freeMB, ::Ice::Long& usedMB, const ::Ice::Current& c)
{
	RLock sync(*this);
	totalMB= _totalSpaceMB;
	freeMB= _freeSpaceMB;
	usedMB= usedSpaceMB;

	return true;
}

}} // namespace
