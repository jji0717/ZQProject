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
// Ident : $Id: BaseCSCmds.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentCmds.cpp $
// 
// 13    1/11/16 5:41p Dejian.fei
// 
// 12    12/12/13 1:43p Hui.shao
// %lld
// 
// 11    8/10/12 2:07p Hui.shao
// 
// 10    8/10/12 2:04p Hui.shao
// ticket#11973
// 
// 9     4/25/12 4:36p Hui.shao
// drafted most unpopular list of TopFolder
// 
// 8     3/23/11 3:43p Hui.shao
// 
// 7     3/23/11 1:33p Hui.shao
// 
// 6     3/23/11 12:45p Hui.shao
// filter out the duplicated file events
// 
// 5     3/23/11 11:27a Hui.shao
// 
// 4     3/17/11 4:42p Hui.shao
// 
// 3     3/09/11 5:10p Hui.shao
// added populate subfolder per file event
// 
// 2     10-11-17 18:31 Hui.shao
// flush some metadata into content per fileevent
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 45    10-11-03 16:21 Hongquan.zhang
// fill master replica net id in content's metadata
// 
// 44    10-10-26 13:16 Hui.shao
// confirmed the merge from V1.10
// 
// 43    10-04-28 13:15 Haoyuan.lu
// 
// 41    10-04-26 15:17 Hui.shao
// added cached content list
// 
// 40    10-02-22 10:42 Yixin.tian
// 
// 39    09-12-22 17:46 Hui.shao
// abstracted folder from volume
// 
// 38    09-12-22 14:06 Jie.zhang
// merge from TianShan1.10
// 
// 37    09-08-03 13:53 Jie.zhang
// 
// 36    09-08-03 12:57 Jie.zhang
// 
// 35    09-07-30 18:41 Fei.huang
// * fix compiling error on linux
// 
// 34    09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 33    09-06-26 16:47 Fei.huang
// 
// 32    09-06-11 18:55 Hui.shao
// merged back _checkResidentialInFileDeleteEvent from 1.10
// 
// 30    09-02-05 13:53 Hui.shao
// added the event entires from the ContentStoreLib to the impls derived
// from the base ContentStoreImpl
// 
// 29    09-01-23 15:44 Hongquan.zhang
// 
// 28    09-01-20 16:49 Yixin.tian
// 
// 27    08-12-26 18:08 Fei.huang
// 
// 26    08-12-18 19:36 Hui.shao
// 
// 25    08-12-18 14:24 Hui.shao
// skip the file event on master replic on forwarding if known as a state
// change event
// 
// 24    08-12-17 15:19 Jie.zhang
// 
// 23    08-12-15 19:27 Hui.shao
// moved the sync fs to a separate thread request
// 
// 22    08-12-11 19:28 Hui.shao
// impl the event forwader
// 
// 21    08-12-11 15:44 Hui.shao
// 
// 20    08-12-11 12:23 Hui.shao
// report the replica to the master
// 
// 19    08-12-09 12:39 Fei.huang
// * sort algorithm for Identity in list contents
// 
// 18    08-11-15 13:57 Yixin.tian
// modify can compile for Linux OS
// 
// 17    08-11-13 11:08 Jie.zhang
// add notifyReplicasChanged
// 
// 16    08-11-12 10:08 Jie.zhang
// 
// 15    08-11-11 18:43 Jie.zhang
// 
// 14    08-11-11 18:30 Jie.zhang
// fixupPathnme on listcontents input
// 
// 13    08-11-11 18:15 Jie.zhang
// 
// 12    08-11-11 17:40 Yixin.tian
// 
// 11    08-11-11 15:15 Jie.zhang
// updateReplica return value in seconds
// 
// 10    08-11-10 11:51 Hui.shao
// rely on the pre-filled getMetaData()
// 
// 9     08-11-07 11:00 Jie.zhang
// add common log define to unify log using style
// 
// 8     08-11-06 13:22 Hui.shao
// 
// 7     08-11-03 11:36 Hui.shao
// fixed per-vol listContents()
// 
// 6     08-10-30 16:17 Hui.shao
// forward ContentStoreImpl::listContents_async & impl QueryReplicaCmd
// 
// 5     08-10-28 17:57 Hui.shao
// test the last write if it is worthy to populate attributes
// 
// 4     08-10-23 18:31 Hui.shao
// moved watchdog out to common
// 
// 3     08-10-14 11:33 Hui.shao
// 
// 2     08-10-07 19:55 Hui.shao
// added volume layer
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// 
// 6     08-08-13 18:51 Hui.shao
// 
// 5     08-08-11 18:42 Hui.shao
// added store replica handling
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

#include "ContentCmds.h"
#include "ContentImpl.h"
#include "ContentState.h"
#include "Guid.h"
#include "ContentUser.h"
#include <algorithm>


#define MOLOG	(_store._log)
#define MOEVENT	(_store._eventlog)


using namespace ZQ::common;

namespace ZQTianShan {
namespace ContentStore {


// -----------------------------
// class BaseCmd
// -----------------------------
BaseCmd::BaseCmd(ContentStoreImpl& store)
: ThreadRequest(store._thpool), _store(store)
{
	int pendsize = store._thpool.pendingRequestSize();
	int poolsize = store._thpool.size();
	if (pendsize > poolsize*4)
	{
		int actsize = store._thpool.activeCount();
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(BaseCmd, "threadpool[%d/%d] has long pending requests[%d]"), actsize, poolsize, pendsize);
	}
}

BaseCmd::~BaseCmd()
{
}

Freeze::EvictorPtr& BaseCmd::theContainer()
{
	return _store._eContent;
}


// -----------------------------
// class FileEventCmd
// -----------------------------
FileEventCmd::PendingHashTable FileEventCmd::_htPendingEvents; // a hash table of "_event::_fullname";
::ZQ::common::Mutex FileEventCmd::_lkPendingEvents;

const char* FileEventCmd::eventStr(const ::TianShanIce::Storage::FileEvent event)
{
#define SWITCH_CASE_EVENT(_EVENT)	case ::TianShanIce::Storage::fse##_EVENT: return #_EVENT
	switch(event)
	{
		SWITCH_CASE_EVENT(FileCreated);
		SWITCH_CASE_EVENT(FileDeleted);
		SWITCH_CASE_EVENT(FileRenamed);
		SWITCH_CASE_EVENT(FileModified);
		SWITCH_CASE_EVENT(Security);
	default:
		return "<Unknown>";
	}
#undef SWITCH_CASE_EVENT
}

bool FileEventCmd::findPendingEvent(::TianShanIce::Storage::FileEvent event, const std::string& filename)
{
	char key[512];
	snprintf(key, sizeof(key)-2, "%d::%s", event, filename.c_str());
	::ZQ::common::MutexGuard g(_lkPendingEvents);
	return (_htPendingEvents.end() != _htPendingEvents.find(key));
}

//FileEventCmd::FileEventCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_ContentStoreEx_OnFileEventPtr& amdCB, ::TianShanIce::Storage::FileEvent event, const ::std::string& name, const ::TianShanIce::Properties& params)
//: BaseCmd(store), _amdCB(amdCB), _event(event), _params(params), _name(name)
FileEventCmd::FileEventCmd(ContentStoreImpl& store, ::TianShanIce::Storage::FileEvent event, const ::std::string& fullname, const ::TianShanIce::Properties& params)
: BaseCmd(store), _event(event), _params(params)
{
	char key[512];
	snprintf(key, sizeof(key)-2, "%d::%s", event, fullname.c_str());
	_hashkey = key;
	{
		::ZQ::common::MutexGuard g(_lkPendingEvents);
		MAPSET(PendingHashTable, _htPendingEvents, _hashkey, 1);
	}

	_fullname = _store.fixupPathname(_store, fullname);
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "FileEventCmd generated, file[%s] event[%s(%d)]"), _fullname.c_str(), eventStr(_event), _event);

}

bool FileEventCmd::findContentWithFilename(::Ice::Identity& identVolume, ::Ice::Identity& identContent, std::string& contentName)
{
	::TianShanIce::Storage::VolumeExPrx volume;
	bool bVolumeFound = false;

	try {

		std::string volumeToSearch = LOGIC_FNSEPS;
		bool bContinue = false;

		do {
			bContinue = false;
			IdentCollection linkIds = _store._idxChildVolume->find(volumeToSearch);

			for (IdentCollection::iterator it = linkIds.begin(); it < linkIds.end(); it++)
			{
				try	{
#ifdef _DEBUG
					MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] try to test vol[%s]"), _fullname.c_str(), eventStr(_event), _event, it->name.c_str());
#endif // _DEBUG
					volume = IdentityToObjEnv(_store, VolumeEx, *it);
					if (!volume)
						continue;

					std::string mountPath = _store._storeAggregate ? volume->getIdent().name : volume->getMountPath();

					if (STOREFLAG_populateSubfolders & _store._storeFlags && FNSEPC != mountPath[mountPath.length()-1])
						mountPath+= FNSEPC;

					if (_store._storeAggregate && LOGIC_FNSEPC != mountPath[mountPath.length()-1])
						mountPath+= LOGIC_FNSEPC;

					if (0 == _fullname.compare(0, mountPath.length(), mountPath))
					{
						identVolume = *it;
						volumeToSearch = identVolume.name;
						contentName = _fullname.substr(mountPath.length());
						bContinue = true;
						bVolumeFound = true;

						if (FNSEPC != LOGIC_FNSEPC)
							std::replace(contentName.begin(), contentName.end(), FNSEPC, LOGIC_FNSEPC);

						try {
							// try to check if the quick guessed folder record already exist to speed up this scan
							size_t pos = contentName.find_last_of(LOGIC_FNSEPS);

							if (std::string::npos != pos)
							{
								identVolume.name = volumeToSearch + LOGIC_FNSEPS + contentName.substr(0, pos);
								if (IdentityToObjEnv(_store, VolumeEx, *it))
								{
									volumeToSearch = identVolume.name;
									contentName = contentName.substr(pos+1);
									bContinue = false;
								}
							}

						} catch(...) {}

						break;
					}

				} catch(...) {}
			}
		} while (bContinue);

		if (bVolumeFound)
			volume = IdentityToObjEnv(_store, VolumeEx, identVolume);
	}
	catch(const ::Ice::Exception&)
	{
		// log nothing
	}

	if (!bVolumeFound || !volume)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] can not recognize the file in any volume, ignore"), _fullname.c_str(), eventStr(_event), _event);
		return false;
	}

	contentName = ContentStoreImpl::memberFileNameToContentName(_store, contentName);
	if (contentName.empty())
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] can not recognize the file, empty content name, ignore"), _fullname.c_str(), eventStr(_event), _event);
		return false;
	}

	for (size_t pos = contentName.find(LOGIC_FNSEPC); STOREFLAG_populateSubfolders & _store._storeFlags && std::string::npos != pos; pos = contentName.find(LOGIC_FNSEPC))
	{
		// the contentName still consists of subdir and the system is configured to populate subfolders
		std::string subfolderName = contentName.substr(0, pos);
		contentName = contentName.substr(pos+1);

		if (0 == subfolderName.compare("."))
			continue;
		else if (0 == subfolderName.compare(".."))
		{
			volume = ::TianShanIce::Storage::VolumeExPrx::checkedCast(volume->parent());
		}
		else if (!subfolderName.empty())
		{
			volume = ::TianShanIce::Storage::VolumeExPrx::checkedCast(volume->openSubFolder(subfolderName, true, 0));
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] pupulated record for detected subfolder[%s] under folder[%s]"), _fullname.c_str(), eventStr(_event), _event, subfolderName.c_str(), identVolume.name.c_str());
		}

		identVolume = volume->getIdent();
	}

//	identVolume = volume->getIdent();
	identContent = _store.toContentIdent(identVolume.name + LOGIC_FNSEPS + contentName);
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] associated as content[%s] of f[%s]"), _fullname.c_str(), eventStr(_event), _event, contentName.c_str(), identVolume.name.c_str());

	return true;
}

int FileEventCmd::run(void)
{
	{
		::ZQ::common::MutexGuard g(_lkPendingEvents);
		_htPendingEvents.erase(_hashkey);
	}

	::std::string paramstr;
	for (::TianShanIce::Properties::const_iterator itParam = _params.begin(); itParam != _params.end(); itParam ++)
		paramstr += itParam->first + "[" + itParam->second + "], ";

	// step 1. associate the volume by comparing the mount path in the hiriachy
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] associating the file to content; params: %s"), _fullname.c_str(), eventStr(_event), _event, paramstr.c_str());
	::Ice::Identity identVolume;
	::Ice::Identity identContent;
	std::string contentName;

	//	if (!_store._storeAggregate)
	//	{
	if (!findContentWithFilename(identVolume, identContent, contentName))
		return 0;
	/*
	}
	else
	{
		//cluster, the file name is the content full name
		identContent = _store.toContentIdent(_fullname);

		// get the volume & content name from the full name
		//identVolume = "";
		//contentName = "";
		std::string::size_type nPos = _fullname.find_last_of(LOGIC_FNSEPC);
		if (nPos == std::string::npos || nPos + 1 >= _fullname.length() || nPos <= 0)
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] can not recognize the content name and volume, ignore"), _fullname.c_str(), eventStr(_event), _event);
			return 0;
		}

		contentName = _fullname.substr(nPos+1);
		std::string volumeName = _fullname.substr(0, nPos);
		identVolume = _store.toVolumeIdent(volumeName);
		
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] recognize the content name[%s] and volume[%s]"), 
			_fullname.c_str(), eventStr(_event), _event, contentName.c_str(), volumeName.c_str());
	}
*/
	
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file[%s] event[%s(%d)] is associated as Content[%s]"), _fullname.c_str(), eventStr(_event), _event, identContent.name.c_str());
	MAPSET(::TianShanIce::Properties, _params, SYS_PROP(Volume), identVolume.name);
	MAPSET(::TianShanIce::Properties, _params, SYS_PROP(Content), identContent.name);

	::TianShanIce::Storage::UnivContentPrx contentPrx;

	try {
		// must uncheckedCast here
		contentPrx = IdentityToObjEnv(_store, UnivContent, identContent);
	}
	catch(const ::Ice::Exception&)
	{
		// log nothing
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileEventCmd,"open content[%s] caught unknown exception"), identContent.name.c_str());
	}
	
	::TianShanIce::Properties newMetaData;
	bool fromMastReplic =false;

	if (_params.end() != _params.find(SYS_PROP(SourceReplicaId)))
		MAPSET(::TianShanIce::Properties, newMetaData, METADATA_RecentUpdateNetId, _params[SYS_PROP(SourceReplicaId)]);

	if (_params.end() != _params.find(METADATA_MasterReplicaNetId))
		MAPSET(::TianShanIce::Properties, newMetaData, METADATA_MasterReplicaNetId, _params[METADATA_MasterReplicaNetId]);
	
	if (_params.end() != _params.find(METADATA_MasterReplicaEndpoint))
	{
		MAPSET(::TianShanIce::Properties, newMetaData, METADATA_MasterReplicaEndpoint, _params[METADATA_MasterReplicaEndpoint]);
		fromMastReplic = true;
	}

	try {
		switch(_event)
		{
		case ::TianShanIce::Storage::fseFileCreated:
			{
				if (!contentPrx && _store.validateMainFileName(_store, _fullname, ""))
				{
					::TianShanIce::Storage::VolumeExPrx volume;
					volume = IdentityToObjEnv(_store, VolumeEx, identVolume);

					contentPrx = volume->createContent(identContent, true, "");
				}

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
				MOEVENT(EventFMT(_store._netId.c_str(), Content, FileCreated, 10, "Content[%s] vol[%s] name[%s] member-file[%s]"), identContent.name.c_str(), identVolume.name.c_str(), contentName.c_str(), _fullname.c_str());

				if (newMetaData.size() >0)
					contentPrx->setMetaData(newMetaData);
				contentPrx->OnFileCreated(_fullname);
			}
			break;

		case ::TianShanIce::Storage::fseFileDeleted:
			{
				if(!contentPrx)
					break;

				uint64 flags = RSDFLAG(frfAbsence);				
				try
				{
					if ( _store._checkResidentialInFileDeleteEvent)
					{		
						flags = contentPrx->isCorrupt() ? RSDFLAG(frfAbsence) :0;
					}
					else if (::TianShanIce::Storage::csNotProvisioned != contentPrx->getState() && contentPrx->isCorrupt())
					{
						flags = RSDFLAG(frfAbsence);//file set is corrupt
						// 							uint64 flagsToTest = RSDFLAG(frfAbsence);
						// 							flags = ContentStoreImpl::checkResidentialStatus(_store, flagsToTest, NULL,
						// 								identContent.name, contentPrx->getMainFilePathname() );
						// 							MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentState, "doPopulateFromFs() portal::checkResidentialStatus(%04I64x) returns %04I64x"), flagsToTest, flags);

					}
					else flags =0; //flags &= ~RSDFLAG(frfAbsence);
				}
				catch (...)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(FileEventCmd, "Content[%s] FileDeleted member-file[%s] caught exception on check resident"), identContent.name.c_str(), _fullname.c_str());
					return 1;
				}

				if (newMetaData.size() >0)
					contentPrx->setMetaData(newMetaData);

				if (flags & (uint64) RSDFLAG(frfAbsence))
					contentPrx->destroy2( true );

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
				MOEVENT(EventFMT(_store._netId.c_str(), Content, FileDeleted, 11, "Content[%s] vol[%s] name[%s] member-file[%s]"), identContent.name.c_str(), identVolume.name.c_str(), contentName.c_str(), _fullname.c_str());
				
			}
			break;

		case ::TianShanIce::Storage::fseFileModified:
			{
				try {
					// double check if the content can be populated from the filesystem
					if (!contentPrx)
					{
						MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "file event[%d] fromMastReplic[%s] refers to a non-exist content[%s], trying to check FS"), _event, (fromMastReplic? "true": "false"), identContent.name.c_str());
						::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, identVolume);
						contentPrx = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(volume->openContent(contentName, "", fromMastReplic));
					}

					if (contentPrx && fromMastReplic)
						contentPrx->setMetaData(newMetaData);
				}
				catch (...) {}

				if (contentPrx)
				{
					contentPrx->OnFileModified();

					::TianShanIce::Storage::ContentState state = contentPrx->getState();
					if (::TianShanIce::Storage::csProvisioning != state && ::TianShanIce::Storage::csProvisioningStreamable != state)
					{
#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
						MOEVENT(EventFMT(_store._netId.c_str(), Content, FileModified, 12, "Content[%s:%s(%d)] vol[%s] name[%s] member-file[%s]"), identContent.name.c_str(), ContentStateBase::stateStr(state), state, identVolume.name.c_str(), contentName.c_str(), _fullname.c_str());

						ZQ::common::MutexGuard gd(_store._lockReplicaSubscriberMap);
						if (_store._replicaSubscriberMap.end() != _store._replicaSubscriberMap.find(_store._prxstrMasterReplica))
						{
							MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FileEventCmd, "need to forward the event[%d] to the master replica[%s]"), _event, _store._prxstrMasterReplica.c_str());
							(new ForwardEventToMasterCmd(_store, _store._replicaSubscriberMap[_store._prxstrMasterReplica], identContent.name, _event, _params))->execute();
						}
					}
				}
			}
			break;

		case ::TianShanIce::Storage::fseFileRenamed:
			{
				if (contentPrx)
				{
					std::string newName;
					if (_params.end() != _params.find("newFilename"))
						newName = _params["newFilename"];

					::TianShanIce::Storage::ContentState state = contentPrx->getState();

					if (newMetaData.size() >0)
						contentPrx->setMetaData(newMetaData);

					contentPrx->OnFileRenamed(newName);

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
					if (::TianShanIce::Storage::csProvisioning != state && ::TianShanIce::Storage::csProvisioningStreamable != state)
						MOEVENT(EventFMT(_store._netId.c_str(), Content, FileRenamed, 13, "Content[%s:%s(%d)] vol[%s] name[%s] member-file[%s] renamedTo[%s]"), identContent.name.c_str(), ContentStateBase::stateStr(state), state, identVolume.name.c_str(), contentName.c_str(), _fullname.c_str(), newName.c_str());
				}
			}
			break;

		default:
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(FileEventCmd, "unknown file event %d"), _event);
		}

		return 0;
	}
	catch(const ::Ice::ObjectNotExistException& )
	{
		if (::TianShanIce::Storage::fseFileDeleted == _event)
			return 0;

		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(FileEventCmd, "[%s:%d] failed to find the Content record"), identContent.name.c_str(), _event);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(FileEventCmd, "[%s:%d] caught exception[%s]: %s"), identContent.name.c_str(), _event, ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(FileEventCmd, "[%s:%d] caught exception[%s]"), identContent.name.c_str(), _event, ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(FileEventCmd, "[%s:%d] caught unknown exception"), identContent.name.c_str(), _event);
	}
	
	if(contentPrx)
	{
		//fill master replica net id into content's metadata if METADATA_MasterReplicaNetId is available
		TianShanIce::Properties extraProps;
		std::string masterReplicaNetId;
		TianShanIce::Properties::const_iterator itMasterReplicaNetId = _params.find(METADATA_MasterReplicaNetId);
		
		if( itMasterReplicaNetId!= _params.end() && (!itMasterReplicaNetId->second.empty() ) )
		{
			extraProps[METADATA_MasterReplicaNetId] = itMasterReplicaNetId->second;
			try
			{
				contentPrx->setMetaData(extraProps);
			}
			catch(...){}
		}
	}

	
	return 1;
}

// -----------------------------
// class PopulateFileAttrsCmd
// -----------------------------
ZQ::common::Mutex PopulateFileAttrsCmd::_lkMap;
PopulateFileAttrsCmd::Map PopulateFileAttrsCmd::_map;

PopulateFileAttrsCmd::PopulateFileAttrsCmd(ContentStoreImpl& store, const ::Ice::Identity& identContent)
: BaseCmd(store), _identContent(identContent)
{
	// lower this priority a bit than normal
	uint8 priority = ZQ::common::ThreadRequest::getPriority() +10;
	ZQ::common::ThreadRequest::setPriority(priority);

	ZQ::common::MutexGuard g(_lkMap);
	Map::iterator it = _map.find(_identContent.name);
	if (_map.end() != it && this != it->second)
		throw ZQ::common::Exception("a PopulateFileAttrsCmd of the same content is queued, ignore this post");

	_map.insert(Map::value_type(_identContent.name, this));
}

int PopulateFileAttrsCmd::run(void)
{
	{
		ZQ::common::MutexGuard g(_lkMap);
		Map::iterator it = _map.find(_identContent.name);
		if (_map.end() != it)
			_map.erase(it);
	}

	try {
		::TianShanIce::Storage::UnivContentPrx contentPrx = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_store._adapter->createProxy(_identContent));
		contentPrx->populateAttrsFromFilesystem();
		return 0;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		// ignore because this cmd has lower piriority
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(PopulateFileAttrsCmd, "[%s] caught exception[%s]: %s"), _identContent.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(PopulateFileAttrsCmd, "[%s] caught exception[%s]"), _identContent.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(PopulateFileAttrsCmd, "[%s] caught unknown exception"), _identContent.name.c_str());
	}
	
	return 1;
}


// -----------------------------
// class ListContentsCmd
// -----------------------------
ListContentsCmd::ListContentsCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::Ice::Identity& identVolume, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount)
: BaseCmd(store), _amdCB(amdCB), _identVolume(identVolume), _metaDataNames(metaDataNames), _maxCount(maxCount)
{
	_startName = store.fixupPathname(store, startName);
}

int ListContentsCmd::readContentInfos(::TianShanIce::Storage::ContentInfos& results, ContentStoreImpl& store, IdentCollection& sortedContentIdents, IdentCollection::iterator& itContent, const ::TianShanIce::StrValues& metaDataNames, int maxCount, const std::string& startName, int nSkipLen)
{
	if (itContent >= sortedContentIdents.end() || itContent < sortedContentIdents.begin())
		return 0;

	::Ice::Long stamp2 = ZQTianShan::now();

	// build up the content info collection based on the search result
	for (; itContent < sortedContentIdents.end() && (maxCount <=0 || results.size() < (size_t) maxCount); itContent++)
	{
		if (!startName.empty())
		{
			std::string contentName = itContent->name.substr(nSkipLen);
			if (contentName.empty())
				continue;

			contentName = store.fixupPathname(store, contentName);
			if (contentName.compare(startName) <0)
				continue;
		}

		try {
			::TianShanIce::Storage::UnivContentPrx content = IdentityToObjEnv(store, UnivContent, *itContent);
			::TianShanIce::Storage::ContentInfo contentInfo;
			contentInfo.state = content->getState();
			contentInfo.name = content->getName();
			contentInfo.fullname = itContent->name;

			::TianShanIce::Properties metaData = content->getMetaData();
			for (::TianShanIce::StrValues::const_iterator pit= metaDataNames.begin(); pit < metaDataNames.end(); pit++)
			{
				if (0 == pit->compare("*"))
				{
					contentInfo.metaData = metaData;
					break;
				}

				if (metaData.end() !=metaData.find(*pit))
					contentInfo.metaData.insert(::TianShanIce::Properties::value_type(*pit, metaData[*pit]));
			}

			results.push_back(contentInfo);
		}
		catch (...) {}
	}

	::Ice::Long stamp3 = ZQTianShan::now();
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ListContentsCmd, "readContentInfos() required %d, read %d content(s) as the result, preparation took %lldms"), maxCount, results.size(), stamp3-stamp2);
	return results.size();
}

int ListContentsCmd::run(void)
{
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListContentsCmd, "vol[%s] list contents"), _identVolume.name.c_str());

	IdentCollection Idents;
	::Ice::Long stamp0 = ZQTianShan::now();

	try	{
		Idents = _store._idxFileOfVol->find(_identVolume);

		::Ice::Long stamp1 = ZQTianShan::now();
		::std::sort(Idents.begin(), Idents.end());

		::Ice::Long stamp2 = ZQTianShan::now();
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListContentsCmd, "vol[%s] %d contents found, took [%lld]ms, sort [%lld]ms"), _identVolume.name.c_str(), Idents.size(), stamp1-stamp0, stamp2-stamp1);

		// build up the content info collection based on the search result
		::TianShanIce::Storage::ContentInfos results;

		IdentCollection::iterator it= Idents.begin();
		readContentInfos(results, _store, Idents, it, _metaDataNames, _maxCount, _startName, _identVolume.name.length()+1);

		_amdCB->ice_response(results);
		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListContentsCmd() vol[%s] caught exception[%s]", _identVolume.name.c_str(), ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListContentsCmd() vol[%s] caught unknown exception", _identVolume.name.c_str()); 
		lastError = buf;
	}

	MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ListContentsCmd, "vol[%s] list contents caught %s"), _identVolume.name.c_str(), lastError.c_str());
	TianShanIce::ServerError ex("ContentStore", 500, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}

// -----------------------------
// class CSLstContsFwdr
// -----------------------------
CSLstContsFwdr::CSLstContsFwdr(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, const ::TianShanIce::Storage::VolumeExPrx& volume)
: BaseCmd(store), _amdCB(amdCB), _metaDataNames(metaDataNames), _startName(startName), _maxCount(maxCount)
{
	_localVolume = ::TianShanIce::Storage::VolumeExPrx::checkedCast(volume->ice_collocationOptimized(false));
}

int CSLstContsFwdr::run(void)
{
	try {
		::TianShanIce::Storage::ContentInfos results = _localVolume->listContents(_metaDataNames, _startName, _maxCount);
		_amdCB->ice_response(results);
		return 0;
	}
	catch (const ::Ice::Exception& ex)
	{
		_amdCB->ice_exception(ex);
	}
	catch(...)
	{
		TianShanIce::ServerError ex("ContentStore", 3103, "listContents_async() caught unknown exception when redirect to the default volume");
		_amdCB->ice_exception(ex);
	}

	return 1;
}

// -----------------------------
// class ListContentsCmd
// -----------------------------
ListVolumesCmd::ListVolumesCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_ContentStore_listVolumesPtr& amdCB, const ::std::string& listFrom, bool includingVirtual)
: BaseCmd(store), _amdCB(amdCB), _listFrom(listFrom), _includingVirtual(includingVirtual)
{
}

int ListVolumesCmd::run(void)
{
	std::string lastError;

	if (_listFrom.empty() || 1 == _listFrom.length() && (LOGIC_FNSEPC == _listFrom[0] || '*' ==_listFrom[0]) )
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListVolumesCmd, "list volumes from root"));
		_listFrom = LOGIC_FNSEPS;
	}
	else if (LOGIC_FNSEPC != _listFrom[_listFrom.length() -1])
		_listFrom += LOGIC_FNSEPS;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListVolumesCmd, "list volumes from parent[%s] %s"), _listFrom.c_str(), (_includingVirtual ? " including virtuals" : ""));

	IdentCollection Idents;
	try	{
		::Freeze::EvictorIteratorPtr itptr = _store._eVolume->getIterator("", 10);
		while (itptr && itptr->hasNext())
			Idents.push_back(itptr->next());

		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListVolumesCmd, "%d volumes found"), Idents.size());

		// build up the content info collection based on the search result
		::TianShanIce::Storage::VolumeInfos results;

		for (IdentCollection::iterator it= Idents.begin(); it < Idents.end(); it++)
		{
			try {
				if (0 != it->name.compare(0, _listFrom.length(), _listFrom))
					continue; // skip unmatched

				::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, *it);
				::TianShanIce::Storage::VolumeInfo volInfo  = volume->getInfo();
				if (_includingVirtual || !volInfo.isVirtual)
					results.push_back(volInfo);
			}
			catch (...) {}
		}

		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListVolumesCmd, "get information of %d matched volume(s) as the result"), results.size());
		_amdCB->ice_response(results);
		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[100];
		snprintf(buf, sizeof(buf)-2, "ListVolumesCmd() caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[80];
		snprintf(buf, sizeof(buf)-2, "ListVolumesCmd() caught unknown exception"); 
		lastError = buf;
	}

	TianShanIce::ServerError ex("ListVolumesCmd", 500, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}


// -----------------------------
// class QueryReplicaCmd
// -----------------------------
///
QueryReplicaCmd::QueryReplicaCmd(ContentStoreImpl& store, const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly)
:BaseCmd(store), _amdCB(amdCB), _category(category), _groupId(groupId), _localOnly(localOnly)
{
}
	
int QueryReplicaCmd::run(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(BaseContentStore, "QueryReplicaCmd() query for replica category[%s] and groupId[%s]"), _category.c_str(), _groupId.c_str());

	char buf[2048];
	if (0 != _category.compare(CATEGORY_ContentStore))
	{
		snprintf(buf, sizeof(buf)-2, "BaseContentStore::QueryReplicaCmd() only category[" CATEGORY_ContentStore "] is supported, query for category[%s] and groupId[%s]", _category.c_str(), _groupId.c_str());
		TianShanIce::ServerError ex(CATEGORY_ContentStore, 607, buf);
		_amdCB->ice_exception(ex);
		return 1;
	}

	std::string lastError;

	try {
		::TianShanIce::Replicas result = _store.exportStoreReplicas();

		if (!_localOnly)
		{
			_amdCB->ice_response(result);
			return 0;
		}

		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "QueryReplicaCmd() forwarding (%d) neighborhood replia info into the result"), result.size() -1);
		::TianShanIce::Replicas temp = result;
		result.clear();
		if (temp.size()>0)
			result.push_back(temp[0]);
		_amdCB->ice_response(result);
		return 0;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ContentStore::QueryReplicaCmd() caught unknown exception"); 
		lastError = buf;
	}

	TianShanIce::ServerError ex(CATEGORY_ContentStore, 604, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}
	
// -----------------------------
// class UpdateReplicaCmd
// -----------------------------
UpdateReplicaCmd::UpdateReplicaCmd(ContentStoreImpl& store, const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const ::TianShanIce::Replicas& stores)
:BaseCmd(store), _amdCB(amdCB), _stores(stores)
{
	MOLOG(Log::L_DEBUG, CLOGFMT(ContentStore, "UpdateReplicaCmd() created"));
}
	
int UpdateReplicaCmd::run(void)
{
	std::string lastError;
	MOLOG(Log::L_DEBUG, CLOGFMT(ContentStore, "UpdateReplicaCmd() enter"));

	try 
	{
		::TianShanIce::Replicas replicasOld, replicasNew;
		_store.getStoreReplicas(replicasOld);
		_store.updateStoreReplicas(_stores);
		_store.getStoreReplicas(replicasNew);

		int timeout = _store._replicaTimeout/1000;
		if (timeout <= 0)
			timeout = -1;
		_amdCB->ice_response(timeout);
#ifdef ZQ_OS_MSWIN
		Sleep(1);
#else
		usleep(1000);
#endif
		//
		// do sync with replica here
		//
		_store.notifyReplicasChanged(_store, replicasOld, replicasNew);

		return 0;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ContentStore::UpdateReplicaCmd() caught unknown exception"); 
		lastError = buf;

		MOLOG(Log::L_ERROR, CLOGFMT(ContentStore, "UpdateReplicaCmd() caught unknow exception"));
	}
	TianShanIce::ServerError ex(CATEGORY_ContentStore, 607, lastError);
	_amdCB->ice_exception(ex);
	return 1;
}
	
// -----------------------------
// class ReportReplicaCmd
// -----------------------------
///
ReportReplicaCmd::ReportReplicaCmd(ContentStoreImpl& store, const ContentStoreImpl::ReplicaSubscriberInfo& subscriberInfo, const ::TianShanIce::Replicas& replicas)
:BaseCmd(store), _subscriberInfo(subscriberInfo), _replicas(replicas)
{
	MOLOG(Log::L_DEBUG, CLOGFMT(ContentStore, "ReportReplicaCmd() created"));
}

int ReportReplicaCmd::run(void)
{
	std::string lastError;
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "ReportReplicaCmd() report replica to subscriber[%s]"), _subscriberInfo.proxyStr.c_str());

	bool bTimeoutChanged = false;

	try
	{
		long oldTimeout = _subscriberInfo.timeout;
		if (!_subscriberInfo.subscriber)
			_subscriberInfo.subscriber = ::TianShanIce::ReplicaSubscriberPrx::checkedCast(_store._adapter->getCommunicator()->stringToProxy(_subscriberInfo.proxyStr));

		_subscriberInfo.timeout = _subscriberInfo.subscriber->updateReplica( _replicas ) * 1000;
		_subscriberInfo.lastUpdated = ZQTianShan::now();
		bTimeoutChanged = (_subscriberInfo.timeout != oldTimeout);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ReportReplicaCmd() caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ReportReplicaCmd() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ReportReplicaCmd() caught unknown"));
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(ContentStore, "ReportReplicaCmd() return timeout = %d seconds"), _subscriberInfo.timeout);

	if ( _subscriberInfo.timeout <= 0 )
		_subscriberInfo.timeout = _store._replicaTimeout;

	// refresh the store map
	ZQ::common::MutexGuard gd(_store._lockReplicaSubscriberMap);
	MAPSET(ContentStoreImpl::ReplicaSubscriberMap, _store._replicaSubscriberMap, _subscriberInfo.proxyStr, _subscriberInfo);

	if (bTimeoutChanged)
		_store._watchDog.watch(_store._localId, 0); // necessary to wake up the watch dog if the timeout was changed by the subscriber

	return 0;
}
	
// -----------------------------
// class ForwardEventToMasterCmd
// -----------------------------
///
ForwardEventToMasterCmd::ForwardEventToMasterCmd(ContentStoreImpl& store, const ContentStoreImpl::ReplicaSubscriberInfo& subscriberInfo, std::string contentFullName, ::TianShanIce::Storage::FileEvent event, const ::TianShanIce::Properties& params)
:BaseCmd(store), _subscriberInfo(subscriberInfo), _event(event), _params(params), _contentFullName(contentFullName)
{
	// lower this priority a bit than normal
	uint8 priority = ZQ::common::ThreadRequest::getPriority() +10;
	ZQ::common::ThreadRequest::setPriority(priority);

	// preset some common parameters
	char buf[300];
	MAPSET(::TianShanIce::Properties, _params, SYS_PROP(SourceReplicaGroupId), _store._replicaGroupId);
	MAPSET(::TianShanIce::Properties, _params, SYS_PROP(SourceReplicaId),      _store._replicaId);
	MAPSET(::TianShanIce::Properties, _params, SYS_PROP(SourceStoreNetId),     _store._netId);
	MAPSET(::TianShanIce::Properties, _params, SYS_PROP(ForwardAsOf),          ::ZQTianShan::TimeToUTC(::ZQTianShan::now(), buf, sizeof(buf) -2));
	snprintf(buf, sizeof(buf) -2, "FileEvent[%s(%d)] of [%s] to master[%s]", FileEventCmd::eventStr(_event), _event, _contentFullName.c_str(), _subscriberInfo.proxyStr.c_str());
	_forwardDesc = buf;
}

int ForwardEventToMasterCmd::run(void)
{
	std::string lastError;
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "ForwardEventToMasterCmd() %s"), _forwardDesc.c_str());

	try
	{
//		long oldTimeout = _subscriberInfo.timeout;
		if (!_subscriberInfo.subscriber)
			_subscriberInfo.subscriber = ::TianShanIce::ReplicaSubscriberPrx::checkedCast(_store._adapter->getCommunicator()->stringToProxy(_subscriberInfo.proxyStr));

		::TianShanIce::Storage::ContentStoreExPrx store = ::TianShanIce::Storage::ContentStoreExPrx::checkedCast(_subscriberInfo.subscriber);
		::TianShanIce::Storage::ContentState targetState = ::TianShanIce::Storage::csNotProvisioned; 

		if (_params.end() != _params.find(SYS_PROP(StateId)))
			targetState = (::TianShanIce::Storage::ContentState) ::atoi(_params[SYS_PROP(StateId)].c_str());

		bool stateMode = (targetState != ::TianShanIce::Storage::csNotProvisioned);

		if (stateMode)
		{
			try {
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "ForwardEventToMasterCmd() %s forcing remote replica to state[%s(%d)] with FullName[%s]"),
					_forwardDesc.c_str(), ContentStateBase::stateStr(targetState), targetState , _contentFullName.c_str() );

				::TianShanIce::Storage::UnivContentPrx content = ::TianShanIce::Storage::UnivContentPrx::checkedCast(store->openContentByFullname(_contentFullName));
				if (!content && targetState < ::TianShanIce::Storage::csOutService)
					stateMode = false;
				else content->enterState(targetState);
			}
			catch (const ::Ice::ObjectNotExistException& ex)
			{
				if ( targetState < ::TianShanIce::Storage::csOutService)
					stateMode = false;

				// ignore the access failure if the state is csOutService or greater
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "ForwardEventToMasterCmd() %s remote replica not exist to force state[%s(%d)]"), _forwardDesc.c_str(), ContentStateBase::stateStr(targetState), targetState);
			}
			catch (const ::Ice::Exception& ex)
			{
				ex.ice_throw();
			}
			catch (...)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (_store._log, EXPFMT(ContentStore, 0, "ForwardEventToMasterCmd(%d) force state of remote replica cought unknown exception"), __LINE__);
			}
		}
		
		if (!stateMode)
			store->OnFileEvent(_event, _contentFullName, _params);

		return 0;
	}
	catch (const TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ForwardEventToMasterCmd() %s caught exception[%s]: %s"),
			 _forwardDesc.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ForwardEventToMasterCmd() %s caught exception[%s]"),
			_forwardDesc.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "ForwardEventToMasterCmd() %s caught unknown exception"), _forwardDesc.c_str());
	}

	return 1;
}

SyncFSCmd::SyncFSCmd(ContentStoreImpl& store, const ::Ice::Identity& identFolder)
:BaseCmd(store), _identVolume(identFolder)
{
}
/*
class GenericCB : public IceUtil::Shared {
public:
	typedef IceUtil::Handle<GenericCB> Ptr;
	void onComplete(  ) {
	}

	void onException( const Ice::Exception& ex) {
	}
};
*/
#if  ICE_INT_VERSION / 100 >= 306
    class WithFSCB : public IceUtil::Shared
    {   
    public:
        WithFSCB(){}   
    private:
        void handleException(const Ice::Exception& ex){}
    public:
        void syncWithFileSystem(const ::Ice::AsyncResultPtr& r)
		{ 
		::TianShanIce::Storage::VolumeExPrx VolumeEx = ::TianShanIce::Storage::VolumeExPrx::uncheckedCast(r->getProxy()); 
            try 
            {   
                VolumeEx->end_syncWithFileSystem(r);
            }   
            catch(const Ice::Exception& ex) 
            {   
                handleException(ex);
            } 
		}   
	};  
	typedef IceUtil::Handle<WithFSCB> WithFSCBPtr;
#endif

int SyncFSCmd::run(void)
{
	std::string lastError;
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "SyncFSCmd() sync vol[%s]"), _identVolume.name.c_str());

	try	{
		::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(_store, VolumeEx, _identVolume);

		if (volume)
		{
			::TianShanIce::Storage::VolumeExPrx localVol = ::TianShanIce::Storage::VolumeExPrx::checkedCast(volume->ice_collocationOptimized(false));
#if  ICE_INT_VERSION / 100 >= 306
		WithFSCBPtr cb = new WithFSCB();
		Ice::CallbackPtr genericCB = Ice::newCallback(cb, &WithFSCB::syncWithFileSystem);
		localVol->begin_syncWithFileSystem(genericCB);
#else
		localVol->syncWithFileSystem_async(new DummyAmiCb<TianShanIce::Storage::AMI_VolumeEx_syncWithFileSystem>(MOLOG));
#endif
		}
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "SyncFSCmd() vol[%s] open vol caught exception[%s]"), _identVolume.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "SyncFSCmd() vol[%s] open vol caught unknown exception"), _identVolume.name.c_str());
	}

	return 1;
}


UpdateContentOnSlaveCmd::UpdateContentOnSlaveCmd( ContentStoreImpl& store, const std::string& contentFullName )
	:BaseCmd(store), _contentFullName(contentFullName)
{
	
}

int UpdateContentOnSlaveCmd::run( void )
{
	ContentStoreImpl::NodeReplicaQueue nrqueue;
	_store.buildNodeReplicaQueue(nrqueue);

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "UpdateContentOnSlaveCmd() enter"));

	if (nrqueue.empty())
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "UpdateContentOnSlaveCmd() return because of no node replica"));
		return 0;
	}

	for(; !nrqueue.empty(); nrqueue.pop())
	{
		const ::TianShanIce::Replica& replica = nrqueue.top().replicaData;

		try 
		{
			std::string strNodePrxString = _store._adapter->getCommunicator()->proxyToString(replica.obj);
			::TianShanIce::Storage::ContentStoreExPrx nodeStore = ::TianShanIce::Storage::ContentStoreExPrx::uncheckedCast(replica.obj);

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "calling OnFileModified(%s) on replica[%s] "), _contentFullName.c_str(), strNodePrxString.c_str());			

			::TianShanIce::Storage::UnivContentPrx content = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(nodeStore->openContentByFullname(_contentFullName));
			content->OnFileModified();

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "calling OnFileModified(%s) on replica[%s] successful"), _contentFullName.c_str(), strNodePrxString.c_str());			
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "wakeupContent(%s) on replica[%s] caught exception[%s]: %s"), 
				_contentFullName.c_str(), replica.replicaId.c_str(), ex.ice_name().c_str(), ex.message.c_str());			
		}
		catch(const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "wakeupContent(%s) on replica[%s] caught exception[%s]"),
				_contentFullName.c_str(), replica.replicaId.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "wakeupContent(%s) on replica[%s] caught unknown exception"),
				_contentFullName.c_str(), replica.replicaId.c_str());
		}
	}

	return 0;
}

// -----------------------------
// class CachedListContentsCmd
// -----------------------------
CachedListContentsCmd::CachedListContentsCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_VolumeEx_cachedListContentsPtr& amdCB, const ::Ice::Identity& identVolume, ::Ice::Int timeout)
	: BaseCmd(store), _amdCB(amdCB), _identVolume(identVolume), _timeout(timeout)
{
	if (_timeout < 500)
		_timeout = 500;

	if (_timeout > UNATTENDED_TIMEOUT)
		_timeout = UNATTENDED_TIMEOUT;
}

int CachedListContentsCmd::run(void)
{
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CachedListContentsCmd, "vol[%s] list contents"), _identVolume.name.c_str());
	::Ice::Long stamp0 = ZQTianShan::now();

	try	{
		CachedContentListImpl::Ptr cachedList = new CachedContentListImpl(_store, _timeout);
		
		if (!cachedList)
			return -1;

		{
			ZQ::common::Guid guid;
			guid.create();
			char buf[64];
			cachedList->_ident.name = "CachedList/" + guid.toCompactIdstr(buf, sizeof(buf)-2);
		}

		cachedList->_identResults = _store._idxFileOfVol->find(_identVolume);
		cachedList->_identVolume = _identVolume;

		::Ice::Long stamp1 = ZQTianShan::now();
		::std::sort(cachedList->_identResults.begin(), cachedList->_identResults.end());
		cachedList->_itResults = cachedList->_identResults.begin();
		::Ice::Long stamp2 = ZQTianShan::now();
		cachedList->_expiration = stamp2 + _timeout;

		_store._adapter->add(cachedList, cachedList->_ident.name);
		cachedList->_ident = _store._adapter->getCommunicator()->stringToIdentity(cachedList->_ident.name);
		::TianShanIce::Storage::CachedContentListPrx prx = ::TianShanIce::Storage::CachedContentListPrx::uncheckedCast( _store._adapter->createProxy(cachedList->_ident));
		_store._watchDog.watch(cachedList->_ident, 500);
		::Ice::Long stamp3 = ZQTianShan::now();

		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CachedListContentsCmd, "vol[%s] %d contents found, took %lldms, sort took %lldms, publish took %lldms"), _identVolume.name.c_str(), cachedList->_identResults.size(), stamp1-stamp0, stamp2-stamp1, stamp3-stamp2);
		_amdCB->ice_response(prx);

		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "CachedListContentsCmd() vol[%s] caught exception[%s]", _identVolume.name.c_str(), ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "CachedListContentsCmd() vol[%s] caught unknown exception", _identVolume.name.c_str()); 
		lastError = buf;
	}

	TianShanIce::ServerError ex("ContentStore", 500, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}

// -----------------------------
// class CachedContentListReadResultCmd
// -----------------------------
///
#if  ICE_INT_VERSION / 100 >= 306
CachedContentListReadResultCmd::CachedContentListReadResultCmd(ContentStoreImpl& store, CachedContentListImpl& list, const ::TianShanIce::Storage::AMD_CachedContentList_nextPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount)
:BaseCmd(store), _list(list), _amdCB(amdCB), _metaDataNames(metaDataNames), _startName(startName), _maxCount(maxCount),WLock(list)
{
}
#else
CachedContentListReadResultCmd::CachedContentListReadResultCmd(ContentStoreImpl& store, CachedContentListImpl& list, const ::TianShanIce::Storage::AMD_CachedContentList_nextPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount) 
: BaseCmd(store), _list(list), _amdCB(amdCB), _metaDataNames(metaDataNames), _startName(startName), _maxCount(maxCount), ::IceUtil::RWRecMutex::WLock(list)
{
}
#endif

int CachedContentListReadResultCmd::run(void)
{
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CachedContentListReadResultCmd, "reading first %d contents from cachedlist[%s] start from name[%s]"), _maxCount, _list._ident.name.c_str(), _startName.c_str());

	IdentCollection Idents;
	::Ice::Long stamp0 = ZQTianShan::now();

	try	{
		// build up the content info collection based on the search result
		::TianShanIce::Storage::ContentInfos results;
		int nSkipLen = _list._identVolume.name.length() +1;

		ListContentsCmd::readContentInfos(results, _store, _list._identResults, _list._itResults, _metaDataNames, _maxCount, _startName, _list._identVolume.name.length() +1);
		::Ice::Long stamp3 = ZQTianShan::now();
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CachedContentListReadResultCmd, "CachedContentList() list[%s] of vol[%s] required %d, sending %d content(s) as the result, preparation took %lldms"), _list._ident.name.c_str(), _list._identVolume.name.c_str(), _maxCount, results.size(), stamp3-stamp0);
		_amdCB->ice_response(results);
		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "CachedContentList() list[%s] of vol[%s] caught exception[%s]", _list._ident.name.c_str(), _list._identVolume.name.c_str(), ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "CachedContentList() list[%s] of vol[%s] caught unknown exception", _list._ident.name.c_str(), _list._identVolume.name.c_str()); 
		lastError = buf;
	}

	MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CachedContentListReadResultCmd, "%s"), lastError.c_str());
	TianShanIce::ServerError ex("ContentStore", 500, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}


}} // namespace
