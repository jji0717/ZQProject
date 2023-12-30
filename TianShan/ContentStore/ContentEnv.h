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
// Ident : $Id: ContentStoreImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentEnv.h $
// 
// 2     5/25/15 1:54p Zhiqiang.niu
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 2     08-11-05 14:37 Jie.zhang
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// ===========================================================================

#ifndef __ZQTianShan_ContentEnv_H__
#define __ZQTianShan_ContentEnv_H__

#include "../common/TianShanDefines.h"
#include "ZQ_common_conf.h"
#include "Locks.h"
#include "Log.h"
#include "NativeThreadPool.h"

#include "ContentStore.h"
#include "ContentFactory.h"
#include "ContentCmds.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#ifndef _DEBUG
#  define MAX_NOT_PROVISIONED_TIMEOUT		(7*24*60*60*1000) // 1 week
#  define UNATTENDED_TIMEOUT				(10*60*1000) // 10 min
#  define MIN_UPDATE_INTERVAL				(10*1000) // 10 sec
#else
#  define MAX_NOT_PROVISIONED_TIMEOUT		(10*60*1000) // 10 min
#  define UNATTENDED_TIMEOUT				(30*1000) // 30 sec
#  define MIN_UPDATE_INTERVAL				(30*1000) // 30 sec
#endif //! _DEBUG

namespace ZQTianShan {
namespace ContentStore {

class ContentStoreImpl;

#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ
#define DECLARE_INDEX(_IDX)	TianShanIce::Storage::##_IDX##Ptr _idx##_IDX

// the following are in the metaData map of the content object
#define METADATA_SourceUrl                SYS_PROP(SourceUrl)          ///< where the content was provisioned from
#define METADATA_ParentName               SYS_PROP(ParentName)         ///< asset if necessary 
#define METADATA_Comment                  SYS_PROP(Comment)            ///< string comment of the content
#define METADATA_FileSize                 SYS_PROP(FileSize)           ///< file size in bytes
#define METADATA_SupportFileSize          SYS_PROP(SupportFileSize)    ///< file size subtotal, in bytes, of supplemental files excluding the main file
#define METADATA_PixelHorizontal          SYS_PROP(PixelHorizontal)    ///< picture resoultion
#define METADATA_PixelVertical            SYS_PROP(PixelVertical)      ///< picture resoultion 
#define METADATA_BitRate                  SYS_PROP(BitRate)            ///< the encoded bitrate in bps
#define METADATA_PlayTime                 SYS_PROP(PlayTime)           ///< play time in msec
#define METADATA_FrameRate                SYS_PROP(FrameRate)          ///< framerate in fps
#define METADATA_SourceType               SYS_PROP(SourceType)         ///< source format type when the content is provisioned from
#define METADATA_LocalType                SYS_PROP(LocalType)          ///< local format type after save the content to local storage
#define METADATA_SubType                  SYS_PROP(SubType)            ///< sub type in addition to the local format type
#define METADATA_MD5CheckSum              SYS_PROP(MD5CheckSum)        ///< the MD5 checksum of the main file

// -----------------------------
// class ContentEnv
// -----------------------------

class ContentImpl;
class ContentFactory;
class ContentStateBase;
class BaseCmd;
class FileEventCmd;
class ContentWatchDog;

class ContentEnv
{
public:
    ContentEnv(ZQ::common::Log& log, ZQ::common::Log& eventlog,
			   ZQ::common::NativeThreadPool& threadPool, 
			   Ice::CommunicatorPtr& communicator, 
			   const char* endpoint = DEFAULT_ENDPOINT_Weiwoo,
			   const char* databasePath = NULL,
			   const char* runtimeDBFolder = NULL);

	virtual ~ContentEnv();

	/// check the configurations
	bool	validateConfig();

	virtual void initWithConfig(void)
	{
		// do nothing here
	}

public:
	ZQ::common::Log&		_log;
	ZQ::common::Log&		_eventlog;
	ZQADAPTER_DECLTYPE&		_adapter;

	ZQ::common::NativeThreadPool& _thpool;

	Ice::Identity			_localId;
	::TianShanIce::Storage::ContentStorePrx _thisPrx;
	ContentFactory::Ptr		_factory;
	ContentWatchDog			_watchDog;

	std::string				_programRootPath;
	std::string				_dbPath;
	std::string				_endpoint;
	::TianShanIce::State	_serviceState;

	DECLARE_CONTAINER(Content);

	// configuration, should be moved to store later
	std::string				_netId;
	std::string				_storeType;
	std::string				_homeDir; // the logical home directory of the ContentStore


protected:

	void openDB(const char* databasePath=NULL);
	void closeDB();
	void syncDBWithFS();

};

#define DBFILENAME_Content "Content"
#define IdentityToObj(_CLASS, _ID) ::TianShanIce::Storage::##_CLASS##Prx::uncheckedCast(_adapter->createProxy(_ID))
#define IdentityToObj2(_CLASS, _ID) ::TianShanIce::Storage::##_CLASS##Prx::checkedCast(_adapter->createProxy(_ID))
#define IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Storage::##_CLASS##Prx::checkedCast(_ENV._adapter->createProxy(_ID))

}} // namespace

#endif // __ZQTianShan_ContentEnv_H__
