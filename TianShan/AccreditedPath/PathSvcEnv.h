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
// Ident : $Id: PathSvcEnv.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathSvcEnv.h $
// 
// 7     6/18/15 11:11a Hui.shao
// ticket#17865 to export serviceGroup usage via csv
// 
// 6     5/06/15 6:00p Hui.shao
// removed dict-index, reimpl AliveStreamer Collector
// 
// 5     2/20/14 5:25p Hui.shao
// merged from V1.16
// 
// 5     2/14/14 11:52a Hui.shao
// added config  _streamLinksByMaxTicket
// 
// 4     3/26/13 10:09a Hui.shao
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
// 28    10-06-24 14:00 Build
// 
// 27    09-02-25 11:30 Hongquan.zhang
// receive and update replicas information
// 
// 26    08-12-29 12:21 Hui.shao
// 
// 26    08-12-24 21:03 Hui.shao
// 
// 25    07-12-14 18:04 Hongquan.zhang
// Use ZQTianShan::Adapter
// 
// 24    07-09-18 12:55 Hongquan.zhang
// 
// 23    07-03-14 12:33 Hongquan.zhang
// 
// 22    07-01-12 12:08 Hongquan.zhang
// 
// 21    06-12-28 16:45 Hongquan.zhang
// 
// 20    06-12-25 19:33 Hui.shao
// support embedded pathsvc mode
// 
// 19    06-12-25 16:58 Hui.shao
// fixed glog to envlog; _throw with envlog
// 
// 18    9/21/06 4:34p Hui.shao
// batch checkin on 20060921
// 
// 17    06-09-19 11:45 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_ADPDatabase_H__
#define __ZQTianShan_ADPDatabase_H__

#include "../common/TianShanDefines.h"
#include "NativeThreadPool.h"
#include "Pointer.h" // for AtomicInt

#include "PathHelperMgr.h"
#include "PathFactory.h"
#include "TsPathAdmin.h"
#include "ServiceGroupDict.h"
#include "StorageDict.h"
#include "StreamerDict.h"

#include "StorageToStorageLink.h"
#include "StreamerToStorageLink.h"
#include "StreamerToStreamLink.h"
#include "ServiceGroupToStreamLink.h"

#include "StreamLinkToTicket.h"
#include "StorageLinkToTicket.h"
#include "StreamerReplicaUpdater.h"

#ifdef EMBED_PATHSVC
#  include "WeiwooSvcEnv.h"
#  undef IdentityToObj
#  undef DECLARE_DICT
#  undef DECLARE_CONTAINER
#  undef DECLARE_INDEX
#endif // EMBED_PATHSVC

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>

#ifdef _DEBUG
#  pragma comment(lib, "Iced")
#  pragma comment(lib, "IceUtild")
#  pragma comment(lib, "freezed")
#else
#  pragma comment(lib, "Ice")
#  pragma comment(lib, "IceUtil")
#  pragma comment(lib, "freeze")
#endif //_DEBUG

namespace ZQTianShan {
namespace AccreditedPath {

// -----------------------------
// class DatabaseException
// -----------------------------
/// A sub-hierarchy for content store access
class DatabaseException : public ZQ::common::IOException
{
public:
	DatabaseException(const std::string &what_arg) : IOException(what_arg) {}
};


#define DECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ;
#define DECLARE_INDEX(_IDX)	TianShanIce::Transport::_IDX##Ptr _idx##_IDX;

// -----------------------------
// class PathSvcEnv
// -----------------------------
class PathSvcEnv : public ::Ice::LocalObject
{
	friend class PathHelperMgr;
	friend class SumServiceGroupBwCommand;

public:
	
#ifdef EMBED_PATHSVC
	PathSvcEnv(::ZQTianShan::Weiwoo::WeiwooSvcEnv& masterEnv, 
				Ice::CommunicatorPtr ic,
				ZQ::common::Log& log,
				ZQ::common::NativeThreadPool& threadPool, 
				const char* phoPath=NULL,
				const char* configFile=NULL,
				const char* logFolder=NULL);
#endif // EMBED_PATHSVC

	PathSvcEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, 
		Ice::CommunicatorPtr& communicator, const char* endpoint = DEFAULT_ENDPOINT_PathManager, 
		const char* databasePath = NULL, const char* phoPath=NULL,
		const char* configFile=NULL,const char* logFolder=NULL);

	virtual ~PathSvcEnv();

	/// get the reference to the path helper manager
	PathHelperMgr& pathHelperMgr() { return _pathHelperMgr; };
	
	/// do the validation during service initialization
	///@throw TianShanIce::InvalidStateOfArt if the data in database doesn't match schema
	///@throw TianShanIce::InvalidParameter if any configuration is not as expected
	void doValidation();

public:

	Ice::CommunicatorPtr	_communicator;

	//Ice::ObjectAdapterPtr	_adapter;
	ZQADAPTER_DECLTYPE		_adapter;
	
	Freeze::ConnectionPtr	_conn;
	ZQ::common::NativeThreadPool& _thpool;

	DECLARE_INDEX(StreamLinkToTicket);
	DECLARE_INDEX(StorageLinkToTicket);

	DECLARE_INDEX(StorageToStorageLink);
	DECLARE_INDEX(StreamerToStorageLink);

	DECLARE_INDEX(StreamerToStreamLink);
	DECLARE_INDEX(ServiceGroupToStreamLink);

	typedef TianShanIce::Transport::StreamerDict*					StreamerDictPtr;
	typedef TianShanIce::Transport::StorageDict*					StorageDictPtr;
	typedef TianShanIce::Transport::ServiceGroupDict*				ServiceGroupDictPtr;
	
	DECLARE_CONTAINER(PathTicket);
	DECLARE_CONTAINER(StorageLink);
	DECLARE_CONTAINER(StreamLink);

	ZQTianShan::MemoryIndex<Ice::Int>    _cachedIdxSvcGrpToStreamLink;
	ZQTianShan::MemoryIndex<std::string> _cachedIdxStreamerToStorageLink;

	DECLARE_DICT(StreamerDict);
	DECLARE_DICT(StorageDict);
	DECLARE_DICT(ServiceGroupDict);
	
	std::string				_programRootPath;
	ZQ::common::Log&		_log;

public: // service configurations
	std::string				_dbPath;
	std::string				_dbRuntimePath;
	std::string				_endpoint;
	std::string				_phoPath;

	AliveStreamerCollector::Ptr	_aliveStreamerCol;

	int				_streamLinksByMaxTicket; // try how many stream links if the links are too many

	TianShanIce::State		_serviceState;
protected:
	
	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);

	PathFactory::Ptr		_factory;
	PathHelperMgr			_pathHelperMgr;
	std::string             _logFolder;

	typedef struct _ServiceGrpStat
	{
		Ice::Int serviceGroupId;
		Ice::Long kbpsCommitted;
		Ice::Long kbpsAssigned;
		Ice::Int  cSessions;
		int64     stampAsOfAssigned;
	} ServiceGrpStat;

	typedef std::map <Ice::Int, ServiceGrpStat> ServiceGrpStatMap;
	ServiceGrpStatMap _svcGrpStatMap;
	ZQ::common::Mutex _lkSvcGrpStat;
	int64             _stampLastSum;

public:
	bool commitUsage(Ice::Int serviceGroupId, Ice::Int bpsAllocated);
	bool withdrawUsage(Ice::Int serviceGroupId, Ice::Int bpsAllocated);
	void sumAssigned(Ice::Int serviceGroupId);
	void setDirtySourceGroup(Ice::Int serviceGroupId);
	bool dumpUsageStat();
};

#define DBFILENAME_StreamerDict			"Streamers"
#define DBFILENAME_StorageDict			"Storages"
#define DBFILENAME_ServiceGroupDict		"ServiceGroups"

#define DBFILENAME_StorageLink		"StorageLink"
#define DBFILENAME_StreamLink		"StreamLink"
#define DBFILENAME_PathTicket		"PathTicket"

#define INDEXFILENAME(_IDX)	#_IDX "Idx"

#define IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Transport::_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))
#define envlog			(_env._log)

#define	REPLICA_STATUS()		SYS_PROP(ReplicaStatus)

}} // namespace

#endif // __ZQTianShan_ADPDatabase_H__
