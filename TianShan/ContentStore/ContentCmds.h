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
// Ident : $Id: ContentCmds.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentCmds.h $
// 
// 4     1/11/16 5:42p Dejian.fei
// 
// 3     3/23/11 12:56p Hui.shao
// 
// 2     3/23/11 12:44p Hui.shao
// filter out the duplicated file events
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 15    10-04-26 15:17 Hui.shao
// added cached content list
// 
// 14    09-12-22 17:46 Hui.shao
// abstracted folder from volume
// 
// 13    09-08-03 12:57 Jie.zhang
// 
// 12    09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 11    08-12-18 14:20 Hui.shao
// 
// 10    08-12-15 19:27 Hui.shao
// moved the sync fs to a separate thread request
// 
// 9     08-12-11 12:08 Hui.shao
// added replica report cmd
// 
// 8     08-11-10 11:51 Hui.shao
// 
// 7     08-11-03 11:36 Hui.shao
// fixed per-vol listContents()
// 
// 6     08-10-30 16:16 Hui.shao
// forward ContentStoreImpl::listContents_async
// 
// 5     08-10-28 17:57 Hui.shao
// test the last write if it is worthy to populate attributes
// 
// 4     08-10-23 18:31 Hui.shao
// moved watchdog out to common
// 
// 3     08-10-14 11:33 Hui.shao
// 
// 2     08-10-07 19:56 Hui.shao
// added volume layer
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
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

#ifndef __ZQTianShan_ContentCmds_H__
#define __ZQTianShan_ContentCmds_H__

#include "../common/TianShanDefines.h"

#include "ContentImpl.h"

namespace ZQTianShan {
namespace ContentStore {

// -----------------------------
// class BaseCmd
// -----------------------------
///
class BaseCmd : protected ZQ::common::ThreadRequest
{
protected:
	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
    BaseCmd(ContentStoreImpl& store);
	virtual ~BaseCmd();

public:

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void) { return 0; }
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	Freeze::EvictorPtr& theContainer();

protected:

	ContentStoreImpl&     _store;
};


// -----------------------------
// class ListContentsCmd
// -----------------------------
///
class ListContentsCmd : public BaseCmd
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	ListContentsCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::Ice::Identity& identVolume, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount);
	virtual ~ListContentsCmd() {}

	static int readContentInfos(::TianShanIce::Storage::ContentInfos& results, ContentStoreImpl& store, IdentCollection& sortedContentIdents, IdentCollection::iterator& itContent, const ::TianShanIce::StrValues& metaDataNames, int maxCount, const std::string& startName, int nSkipLen);
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::Storage::AMD_Folder_listContentsPtr _amdCB;
	::Ice::Identity _identVolume;
	::TianShanIce::StrValues _metaDataNames;
	::std::string _startName;
	::Ice::Int _maxCount;
};

// -----------------------------
// class CSLstContsFwdr
// -----------------------------
///
class CSLstContsFwdr : public BaseCmd
{
public:
    CSLstContsFwdr(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, const ::TianShanIce::Storage::VolumeExPrx& volume);
	virtual ~CSLstContsFwdr() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::Storage::AMD_Folder_listContentsPtr _amdCB;
	::TianShanIce::Storage::VolumeExPrx _localVolume;
	::TianShanIce::StrValues _metaDataNames;
	::std::string _startName;
	::Ice::Int _maxCount;
};

// -----------------------------
// class ListVolumesCmd
// -----------------------------
///
class ListVolumesCmd : public BaseCmd
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
    ListVolumesCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_ContentStore_listVolumesPtr& amdCB, const ::std::string& listFrom, bool includingVirtual);
	virtual ~ListVolumesCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::Storage::AMD_ContentStore_listVolumesPtr _amdCB;
	::std::string _listFrom;
	bool _includingVirtual;
};

// -----------------------------
// class QueryReplicaCmd
// -----------------------------
///
class QueryReplicaCmd : public BaseCmd
{
public:
	/// constructor
    QueryReplicaCmd(ContentStoreImpl& store, const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly);
	virtual ~QueryReplicaCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr _amdCB;
	::std::string _category;
	::std::string _groupId;
	bool _localOnly;
};

// -----------------------------
// class UpdateReplicaCmd
// -----------------------------
///
class UpdateReplicaCmd : public BaseCmd
{
public:
	/// constructor
    UpdateReplicaCmd(ContentStoreImpl& store, const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const ::TianShanIce::Replicas& stores);
	virtual ~UpdateReplicaCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr _amdCB;
	::TianShanIce::Replicas _stores;
};

// -----------------------------
// class FileEventCmd
// -----------------------------
///
class FileEventCmd : public BaseCmd
{
public:

	/// constructor
//	FileEventCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_ContentStoreEx_OnFileEventPtr& amdCB, ::TianShanIce::Storage::FileEvent event, const ::std::string& name, const ::TianShanIce::Properties& params);
	FileEventCmd(ContentStoreImpl& store, ::TianShanIce::Storage::FileEvent event, const ::std::string& name, const ::TianShanIce::Properties& params);
	static const char* eventStr(const ::TianShanIce::Storage::FileEvent event);

	bool findContentWithFilename(::Ice::Identity& identVolume, ::Ice::Identity& identContent, std::string& contentName);

protected: // overwrite of BaseCmd

	virtual int run(void);

public:
	typedef ::std::map<std::string, int> PendingHashTable;
	static bool findPendingEvent(::TianShanIce::Storage::FileEvent event, const std::string& filename);

protected:

//	::TianShanIce::Storage::AMD_ContentStoreEx_OnFileEventPtr _amdCB;
	::TianShanIce::Storage::FileEvent _event;
	::std::string _fullname, _hashkey;
	::TianShanIce::Properties _params;

	static PendingHashTable _htPendingEvents; // a hash table of "_event::_fullname";
	static ::ZQ::common::Mutex _lkPendingEvents;
};

// -----------------------------
// class PopulateFileAttrsCmd
// -----------------------------
///
class PopulateFileAttrsCmd : public BaseCmd
{
public:

	PopulateFileAttrsCmd(ContentStoreImpl& store, const ::Ice::Identity& identContent);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::Ice::Identity _identContent;

	typedef std::map<std::string, PopulateFileAttrsCmd* > Map;
	static ZQ::common::Mutex _lkMap;
	static Map _map;
};

// -----------------------------
// class ReportReplicaCmd
// -----------------------------
///
class ReportReplicaCmd : public BaseCmd
{
public:

	ReportReplicaCmd(ContentStoreImpl& store, const ContentStoreImpl::ReplicaSubscriberInfo& subscriberInfo, const ::TianShanIce::Replicas& replicas);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	ContentStoreImpl::ReplicaSubscriberInfo _subscriberInfo;
	::TianShanIce::Replicas _replicas;
};

// -----------------------------
// class ForwardEventToMasterCmd
// -----------------------------
///
class ForwardEventToMasterCmd : public BaseCmd
{
public:

	ForwardEventToMasterCmd(ContentStoreImpl& store, const ContentStoreImpl::ReplicaSubscriberInfo& subscriberInfo, std::string contentFullName, ::TianShanIce::Storage::FileEvent event, const ::TianShanIce::Properties& params);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	ContentStoreImpl::ReplicaSubscriberInfo _subscriberInfo;
	::TianShanIce::Storage::FileEvent _event;
	::TianShanIce::Properties _params;
	::std::string _contentFullName, _forwardDesc;
};

// -----------------------------
// class SyncFSCmd
// -----------------------------
///
class SyncFSCmd : public BaseCmd
{
public:

	SyncFSCmd(ContentStoreImpl& store, const ::Ice::Identity& identVolume);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::Ice::Identity _identVolume;
};


// -----------------------------
// class UpdateContentOnSlaveCmd
// -----------------------------
///
class UpdateContentOnSlaveCmd : public BaseCmd
{
public:

	UpdateContentOnSlaveCmd(ContentStoreImpl& store, const std::string& contentFullName);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::Replicas _replicas;
	std::string			_contentFullName;
};

// -----------------------------
// class CachedListContentsCmd
// -----------------------------
///
class CachedListContentsCmd : public BaseCmd
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	CachedListContentsCmd(ContentStoreImpl& store, const ::TianShanIce::Storage::AMD_VolumeEx_cachedListContentsPtr& amdCB, const ::Ice::Identity& identVolume, ::Ice::Int timeout);
	virtual ~CachedListContentsCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::Storage::AMD_VolumeEx_cachedListContentsPtr _amdCB;
	::Ice::Int _timeout;
	::Ice::Identity _identVolume;
};

// -----------------------------
// class CachedContentListReadResultCmd
// -----------------------------
///
#if  ICE_INT_VERSION / 100 >= 306
class CachedContentListReadResultCmd : public BaseCmd, WLock
#else
class CachedContentListReadResultCmd : public BaseCmd, ::IceUtil::RWRecMutex::WLock
#endif
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	CachedContentListReadResultCmd(ContentStoreImpl& store, CachedContentListImpl& list, const ::TianShanIce::Storage::AMD_CachedContentList_nextPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount);
	virtual ~CachedContentListReadResultCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	CachedContentListImpl& _list;
	::TianShanIce::Storage::AMD_CachedContentList_nextPtr _amdCB;
	::TianShanIce::StrValues _metaDataNames;
	std::string _startName;
	int _maxCount;

};

}} // namespace

#endif // __ZQTianShan_ContentCmds_H__

