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
// Ident : $Id: CVSSEnv.h $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CiscoVirtualStreamingServer/CVSSEnv.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 2     09-01-20 9:50 Xiaoming.li
// modify for cisco
// 
// 1     08-12-15 9:07 Xiaoming.li
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_CVSSEnv_H__
#define __ZQTianShan_CVSSEnv_H__

#include <TianShanDefines.h>

//Ice include header
#include <SessionIdx.h>
#include "CVSS.h"
#include "CVSSFactory.h"
#include "CVSSEventSinkI.h"

//tianshan common include header
#include "locks.h"
#include "FileLog.h"
#include "ConfigHelper.h"
#include "Guid.h"
#include "NativeThreadPool.h"

//rtsp session include header
#include "daemon_thread.h"

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

#define ADAPTER_NAME_CVSS				"CVSS"
#define DEFAULT_ENDPOINT_CVSS			"default -p 10800"
#define SERVICE_NAME_CiscoVirtualStreamService	"CiscoVirtualStream"

namespace ZQTianShan{
namespace CVSS{

#define CVSSDECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define CVSSDECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ
#define DECLARE_CVSS_INDEX(_IDX)	TianShanIce::Streamer::CiscoVirtualStreamServer::##_IDX##Ptr _idx##_IDX
#define CVSSDECLARE_AMICB(_CL, _API)	TianShanIce::Streamer::CiscoVirtualStreamServer::AMI_##_CL##_##_API##Ptr _amiCB##_API

class CVSSEnv
{
public:
	//constructor
	CVSSEnv(::ZQ::common::FileLog& filelog, 
		   ::ZQ::common::NativeThreadPool& threadPool,
		   ::Ice::CommunicatorPtr& communicator,
		   const char* iceStormEndpoint,
		   const char* endpoint = DEFAULT_ENDPOINT_CVSS,
		   const char* databasePath = NULL,
		   const char* runtimeDBFolder = NULL);
	virtual ~CVSSEnv();

	::ZQTianShan::ContentStore::ContentStoreImpl *_contentStore;
	inline void setContentStore(::ZQTianShan::ContentStore::ContentStoreImpl &contentStoreImpl){_contentStore = &contentStoreImpl;}

	inline void SetDaemonThrd(daemon_thread *daemonThrd)
	{
		_daemonThrd = daemonThrd;
	}

	// configurations
	::std::string					_dbPath;	
	::std::string					_endpoint;
	::std::string					_dbRuntimePath;
	::std::string					_programRootPath;

	CVSSFactory::Ptr				_factory;
	Freeze::EvictorPtr				_eCvssStream;
	::ZQ::common::FileLog&			_logFile;
	daemon_thread					*_daemonThrd;

	::Ice::CommunicatorPtr			_communicator;
	ZQADAPTER_DECLTYPE				_adapter;

	::ZQ::common::NativeThreadPool& _thpool;
	
	//DECLARE_INDEX(SessionIdx);
	::TianShanIce::Streamer::CiscoVirtualStreamServer::SessionIdxPtr	_idxSessionIdx;
	CVSSEventSinkI					_cvssEventSinkI;

	RtspCSeqSignal					_rtspCSeqSignal; //signal for RTSP response
	
protected:
	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);
};

#define DBFILENAME_CVSSSession			"CVSS"

#define INDEXFILENAME(_IDX)			#_IDX"Idx"

#define CVSSIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::CiscoVirtualStreamServer::##_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))

#define CVSSFindIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::CiscoVirtualStreamServer::##_CLASS##Prx::uncheckedCast(&(_ENV)._adapter->find(_ID))

#define envlog			(_env._logFile)

}//namespace CVSS

}//namespace ZQTianShan

#endif __ZQTianShan_CVSSEnv_H__
