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
// Ident : $Id: NSSEnv.h $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ComcastNGOD/NSS/NSSEnv.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 8     08-12-05 17:09 Xiaoming.li
// 
// 7     08-11-04 10:34 Xiaoming.li
// 
// 6     08-08-22 11:32 Xiaoming.li
// 
// 5     08-08-19 10:56 Xiaoming.li
// 
// 4     08-08-19 10:34 Xiaoming.li
// 
// 3     08-07-21 15:32 Xiaoming.li
// modify nonmutating to ["cp:const", "freeze:read"]
// 
// 2     08-07-14 14:54 Xiaoming.li
// 
// 1     08-06-13 11:23 Xiaoming.li
// 
// 1     08-04-18 15:40 xiaoming.li
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_NSSEnv_H__
#define __ZQTianShan_NSSEnv_H__

#include <TianShanDefines.h>

//Ice include header
#include <SessionIdx.h>
#include "NSS.h"
#include "NSSFactory.h"
#include "NSSEventSinkI.h"

//tianshan common include header
#include "locks.h"
#include "FileLog.h"
#include "ConfigHelper.h"
#include "Guid.h"
#include "NativeThreadPool.h"

//rtsp session include header
#include "ngod_daemon_thread.h"

//content store include header
#include "ContentImpl.h"

#ifdef _DEBUG
#  define UNATTEND_TIMEOUT			(20*1000) // 20 sec
#  define DEFAULT_SCH_ERROR_WIN		(60000) // 1 min
#  define MAX_START_DELAY			(60*1000) //1 min
#  define STOP_REMAIN_TIMEOUT		(5*1000) // 5 sec
#  define MIN_PROGRESS_INTERVAL		(10*1000) // 10 sec
#else
#  define UNATTEND_TIMEOUT			(48*60*60*1000) // 48 hours
#  define DEFAULT_SCH_ERROR_WIN		(5000) // 5 sec
#  define MAX_START_DELAY			(5*60*1000) // 5 min
#  define STOP_REMAIN_TIMEOUT		(60*1000) // 1 min
#  define MIN_PROGRESS_INTERVAL		(30*1000) // 30 sec
#endif // _DEBUG

#define MAX_IDLE (60*60*1000) // 1hour
#define DEFAULT_IDLE (5* 60*1000) // 5sec

namespace ZQTianShan{
namespace NSS{

#define NSSDECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define NSSDECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ
#define DECLARE_NSS_INDEX(_IDX)	TianShanIce::Streamer::NGODStreamServer::##_IDX##Ptr _idx##_IDX
#define NSSDECLARE_AMICB(_CL, _API)	TianShanIce::Streamer::NGODStreamServer::AMI_##_CL##_##_API##Ptr _amiCB##_API

class NSSEnv
{
public:
	//constructor
	NSSEnv(::ZQ::common::FileLog& filelog, 
		   ::ZQ::common::NativeThreadPool& threadPool,
		   ::Ice::CommunicatorPtr& communicator,
		   const char* iceStormEndpoint,
		   const char* endpoint = DEFAULT_ENDPOINT_NSS,
		   const char* databasePath = NULL,
		   const char* runtimeDBFolder = NULL);
	virtual ~NSSEnv();

	::ZQTianShan::ContentStore::ContentStoreImpl *_contentStore;
	void setContentStore(::ZQTianShan::ContentStore::ContentStoreImpl &contentStoreImpl){_contentStore = &contentStoreImpl;}

	inline void SetDaemonThrd(ngod_daemon_thread *daemonThrd)
	{
		_daemonThrd = daemonThrd;
	}

	inline void SetSessionList(NSSSessionGroupList &sessList)
	{
		_sessionGroupList = &sessList;
	}

	// configurations
	::std::string					_dbPath;	
	::std::string					_endpoint;
	::std::string					_dbRuntimePath;
	::std::string					_programRootPath;

	NSSFactory::Ptr					_factory;
	Freeze::EvictorPtr				_eNssStream;
	::ZQ::common::FileLog&			_logFile;
	NSSSessionGroupList				*_sessionGroupList;
	ngod_daemon_thread				*_daemonThrd;

	::Ice::CommunicatorPtr			_communicator;
	ZQADAPTER_DECLTYPE				_adapter;

	::ZQ::common::NativeThreadPool& _thpool;
	
	//DECLARE_INDEX(SessionIdx);
	::TianShanIce::Streamer::NGODStreamServer::SessionIdxPtr	_idxSessionIdx;
	NSSEventSinkI					_nssEventSinkI;
	
protected:
	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);
};

#define DBFILENAME_NssSession			"NSS"

#define INDEXFILENAME(_IDX)			#_IDX"Idx"

#define NSSIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::NGODStreamServer::##_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))

#define NSSFindIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::NGODStreamServer::##_CLASS##Prx::uncheckedCast(&(_ENV)._adapter->find(_ID))

#define envlog			(_env._logFile)

}//namespace NSS

}//namespace ZQTianShan

#endif __ZQTianShan_NSSEnv_H__
