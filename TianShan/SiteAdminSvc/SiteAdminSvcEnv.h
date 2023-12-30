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
// Ident : $Id: SiteAdminSvcEnv.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SiteAdminSvcEnv.h $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 14    08-11-24 11:48 Hongquan.zhang
// 
// 13    08-08-19 12:43 Hongquan.zhang
// 
// 12    08-04-30 16:25 Xiaohui.chai
// 
// 11    08-04-21 15:50 Guan.han
// 
// 10    07-12-14 16:36 Xiaohui.chai
// 
// 9     07-12-13 18:27 Hui.shao
// 
// 8     07-12-10 18:47 Hui.shao
// moved event out of txn
// 
// 7     07-10-25 14:09 Hongquan.zhang
// 
// 6     07-07-25 16:00 Hongquan.zhang
// 
// 5     07-06-06 18:29 Hui.shao
// modified ZQ adpater
// 
// 4     07-05-23 13:32 Hui.shao
// use wrappered adapter
// 
// 3     07-04-12 13:46 Hongquan.zhang
// 
// 2     07-03-23 15:07 Hui.shao
// 
// 1     07-03-15 19:02 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_ADPDatabase_H__
#define __ZQTianShan_ADPDatabase_H__

#include "../common/TianShanDefines.h"
#include "NativeThreadPool.h"
#include <NativeThread.h>


#include "SiteAdminSvc.h"
#include "SASFactory.h"

#include "AppDict.h"
#include "SiteDict.h"
#include "SiteToMount.h"
#include "AppToMount.h"

#include "MountToTxn.h"
#include "SiteToTxn.h"
#include "TxnToEvent.h"

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>

#include "MdbLog.h"


#include <list>

#ifdef _DEBUG
#  pragma comment(lib, "Iced")
#  pragma comment(lib, "IceUtild")
#  pragma comment(lib, "freezed")
#else
#  pragma comment(lib, "Ice")
#  pragma comment(lib, "IceUtil")
#  pragma comment(lib, "freeze")
#endif //_DEBUG

// #define YTD_EVICTOR

namespace ZQTianShan {
namespace Site {

class SiteAdminSvcEnv;

class LiveTxnTransfer : public ZQ::common::NativeThread
{
public:
	LiveTxnTransfer(SiteAdminSvcEnv& env);
	~LiveTxnTransfer();
public:
	///add expired liveTxn session id,this liveTxn will be transfered to YTD database
	void		AddSess(const std::string& sessID);

	int			run(void);

private:
	typedef std::list<std::string>		TxnSessList;
	HANDLE								_hEvent;
	TxnSessList							_sessList;
	ZQ::common::Mutex					_listMutex;
	bool								_bQuit;
	SiteAdminSvcEnv&					_env;
};

class LivetxnWatchDog : public ZQ::common::NativeThread
{
public:
	LivetxnWatchDog(SiteAdminSvcEnv& env);
	~LivetxnWatchDog();
	void	WatchMe(const std::string& txnId,Ice::Long lMilliSec);
	void	UnWatchMe(const std::string& txnId);
	int		run();
protected:
private:
	typedef std::map< std::string, Ice::Long >	WATCHMAP;
	WATCHMAP									_watchMap;
	ZQ::common::Mutex							_mapMutex;
	
	Ice::Long									_nextWakeup;

	HANDLE										_hEvent;
	bool										_bQuit;
	SiteAdminSvcEnv&							_env;
};

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
#define DECLARE_INDEX(_IDX)	TianShanIce::Site::##_IDX##Ptr _idx##_IDX;

// -----------------------------
// class SiteAdminSvcEnv
// -----------------------------
class SiteAdminSvcEnv : public ::Ice::LocalObject
{
	friend class PathHelperMgr;

public:
	
	SiteAdminSvcEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, 
		Ice::CommunicatorPtr& communicator, const char* endpoint = DEFAULT_ENDPOINT_SiteAdminSvc,
		const char* databasePath = NULL ,
		const char* runtimeDbPath = NULL );

	virtual ~SiteAdminSvcEnv();

protected:

	virtual void initWithConfig(void)
	{
		// do nothing here
	}


public:

	Ice::CommunicatorPtr			_communicator;
    ZQADAPTER_DECLTYPE              _adapter;
	Freeze::ConnectionPtr			_conn;
	ZQ::common::NativeThreadPool&	_thpool;

	DECLARE_INDEX(AppToMount);
	DECLARE_INDEX(SiteToMount);
	DECLARE_CONTAINER(AppMount);

	DECLARE_INDEX(SiteToTxn);
	DECLARE_INDEX(MountToTxn);
	DECLARE_CONTAINER(LiveTxn);
	
	DECLARE_INDEX(TxnToEvent);
	DECLARE_CONTAINER(TxnEvent);

#ifdef YTD_EVICTOR
	TianShanIce::Site::SessToTxnPtr _idxSessToYTDTxn;
	TianShanIce::Site::TxnToEventPtr _idxYTDTxnToEvent;

	DECLARE_CONTAINER(YTDTxn);
	DECLARE_CONTAINER(YTDEvent);
#endif // YTD_EVICTOR

	typedef TianShanIce::Site::AppDict*		AppDictPtr;
	typedef TianShanIce::Site::SiteDict*	SiteDictPtr;
	
	DECLARE_DICT(AppDict);
	DECLARE_DICT(SiteDict);
	
	SASFactory::Ptr			_factory;

	std::string				_programRootPath;
	ZQ::common::Log&		_log;
	
	LiveTxnTransfer*		_liveTxnTransfer;
	LivetxnWatchDog*		_txnWatchDog;

	ZQ::common::MdbLog		_mdbLog;

	std::string				_endpoint;

//	ZQTianShan::Site::EventSenderManager*	_evnetSenderManager;
	
protected: // service configurations
	std::string				_dbPath;
	
	std::string				_phoPath;

protected:
	
	bool openDB(const char* databasePath = NULL,  const char* runtimeDbPath = NULL );
	void closeDB(void);
    bool initTxnMDB();
};

#define DBFILENAME_AppDict			"Apps"
#define DBFILENAME_SiteDict			"Sites"
#define DBFILENAME_AppMount			"AppMount"
#define DBFILENAME_Txn				"Txn"
#define	DBFILENAME_TxnEvent			"Event"

#define INDEXFILENAME(_IDX)	#_IDX "Idx"

#define IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Site::##_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))
#define envlog			(_env._log)

}} // namespace

#endif // __ZQTianShan_ADPDatabase_H__
