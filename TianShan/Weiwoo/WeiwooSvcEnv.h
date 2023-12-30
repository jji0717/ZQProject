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
// Ident : $Id: WeiwooSvcEnv.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/WeiwooSvcEnv.h $
// 
// 13    1/12/16 8:53a Dejian.fei
// 
// 12    10/29/15 4:00p Li.huang
// 
// 11    10/29/15 11:56a Li.huang
// 
// 10    4/01/15 2:07p Build
// cleaned old snmp
// 
// 9     3/18/15 7:26p Build
// removed the snmp subagent of .5
// 
// 8     7/07/14 4:54p Hui.shao
// TIMEOUT_YIELD_AFTER_ERROR_OCCURED
// 
// 7     9/20/12 6:55p Hui.shao
// 
// 7     9/20/12 6:51p Build
// 
// 6     9/20/12 5:48p Hui.shao
// 
// 6     9/12/12 6:11p Zonghuan.xiao
// implement snmp   HeadEnd Sessions (Weiwoo)
// 
// 5     3/10/11 11:51a Hongquan.zhang
// 
// 4     3/09/11 4:55p Hongquan.zhang
// 
// 3     3/09/11 4:42p Hongquan.zhang
// 
// 2     3/07/11 5:00p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:08 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 29    10-06-18 16:19 Hui.shao
// removed index IdToSess
// 
// 28    08-04-23 11:11 Hongquan.zhang
// migrate configuration to ConfigHelper
// Add Ice Performance Tunning configuration when OpenDB
// 
// 27    07-12-14 18:03 Hongquan.zhang
// Use ZQTianShan::Adapter
// 
// 26    07-10-25 14:07 Hongquan.zhang
// 
// 25    07-10-17 12:29 Hongquan.zhang
// 
// 24    07-09-18 12:56 Hongquan.zhang
// 
// 23    07-07-02 12:12 Hongquan.zhang
// 
// 22    07-06-26 13:29 Hongquan.zhang
// 
// 21    07-04-12 14:02 Hongquan.zhang
// 
// 20    07-03-28 16:42 Hui.shao
// moved business router to namespace Site
// 
// 19    07-03-15 15:54 Hui.shao
// 
// 18    07-03-13 17:12 Hongquan.zhang
// 
// 17    07-01-11 17:09 Hongquan.zhang
// 
// 16    07-01-11 16:09 Hongquan.zhang
// 
// 15    07-01-05 14:43 Hui.shao
// moved the static proxy pathmgr into WeiwooEnv
// 
// 14    07-01-05 10:59 Hongquan.zhang
// 
// 13    06-12-25 19:33 Hui.shao
// support embedded pathsvc mode
// 
// 12    06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 11    06-12-13 18:48 Hongquan.zhang
// 
// 10    9/21/06 4:36p Hui.shao
// batch checkin 20060921
// ===========================================================================

#ifndef __ZQTianShan_WeiwooSvcConf_H__
#define __ZQTianShan_WeiwooSvcConf_H__

#include "../common/TianShanDefines.h"

#include "WeiwooFactory.h"
#include "WeiwooAdmin.h"
#include "Log.h"

#include "NativeThreadPool.h"
#include "SessionWatchDog.h"
// #include "snmp/SubAgent.hpp"

#include "TsTransport.h"
#include "TsSite.h"
#include "TsApplication.h"
#include "TsStreamer.h"

#define  DEFAULT_SESS_CONTAINER_SIZE			 20

#define  DEFAULT_TTL_IDLE_SESSION				(10 *60 *1000)	// 10 min
//#define  MAX_TTL_IDLE_SESSION					(60 *60 *1000)	// 60 min
#ifndef _DEBUG
#  define  MAX_TTL_IDLE_SESSION					(60 *60 *1000)	// 60 min
#  define  MIN_TTL_IDLE_SESSION					(30 *1000)		// 30 sec
#else // _DEBUG
#  define  MAX_TTL_IDLE_SESSION					(60 *1000)		// 60 sec
#  define  MIN_TTL_IDLE_SESSION					(10 *1000)		// 10  sec
#endif // _DEBUG

#define  DEFAULT_TTL_OUTSERVICE_SESSION			(10 *1000)		// 10 sec
#define  MAX_TTL_OUTSERVICE_SESSION				(60 *1000)		// 60 sec
#define  TIMEOUT_YIELD_AFTER_ERROR_OCCURED      (2*1000)          // 1sec


#define  DEFAUL_TTL_PATH_ALLOCATION				(30 *1000)		// 30 sec
#define  MIN_TTL_PATH_ALLOCATION				(10 *1000)		// 10 sec
#define  DEFAUL_MAX_PATHTICKETNUM				(5)				// 5 tickets

namespace ZQTianShan {
namespace Weiwoo {

#define DECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ;
#define DECLARE_INDEX(_IDX)	TianShanIce::SRM::##_IDX##Ptr _idx##_IDX;



// -----------------------------
// class WeiwooSvcEnv
// -----------------------------
class setUserPropertyAmiImpl;
class TxnpostYTDAmiImpl;
class TxnTraceAmiImpl;
class TxnCommitStateChangeAmiImpl;

#if  ICE_INT_VERSION / 100 >= 306
class Purchase_detachCB : public IceUtil::Shared
{
public:
	Purchase_detachCB(){}
private:
	void handleException(const Ice::Exception&){}
public:
	void detach(const Ice::AsyncResultPtr& r);
};
typedef IceUtil::Handle<Purchase_detachCB> Purchase_detachCBPtr;
#else
class AMI_Purchase_detachImpl : public TianShanIce::Application::AMI_Purchase_detach
{
public:
  virtual void ice_response() 
  {

  }
  virtual void ice_exception(const ::Ice::Exception&)
  {

  }
};
#endif

#if  ICE_INT_VERSION / 100 >= 306
class TxnStateCB : public IceUtil::Shared
{
public:
	TxnStateCB(WeiwooSvcEnv& env);
private:
	void handleException(const Ice::Exception&) {}
public:
	void commitStateChange(const Ice::AsyncResultPtr&);	
	void trace(const Ice::AsyncResultPtr&);
	void postYTD(const Ice::AsyncResultPtr&);
	void setUserProperty(const Ice::AsyncResultPtr&);
protected:
	WeiwooSvcEnv& _env;
};
typedef IceUtil::Handle<TxnStateCB> TxnStateCBPtr;
#else
class TxnCommitStateChangeAmiImpl : public TianShanIce::Site::AMI_TxnService_commitStateChange
{
public:
	TxnCommitStateChangeAmiImpl(WeiwooSvcEnv& env):_env(env)
	{
	}
	void ice_response()
	{
	}
	void ice_exception(const ::Ice::Exception& ex)
	{
//		_env._log(ZQ::common::Log::L_ERROR,"Caught a exception when invoke TxnCommitStateChange:%s",ex.ice_name().c_str());
	}
private:
	WeiwooSvcEnv& _env;
};
class TxnTraceAmiImpl : public TianShanIce::Site::AMI_TxnService_trace
{
public:
	TxnTraceAmiImpl(WeiwooSvcEnv& env):_env(env)
	{
	}
	void ice_response()
	{
	}
	void ice_exception(const ::Ice::Exception& ex)
	{
//		_env._log(ZQ::common::Log::L_ERROR,"Caught a exception when invoke TxnTrace:%s",ex.ice_name().c_str());
	}
private:
	WeiwooSvcEnv& _env;
};
class TxnpostYTDAmiImpl : public TianShanIce::Site::AMI_TxnService_postYTD
{
public:
	TxnpostYTDAmiImpl(WeiwooSvcEnv& env):_env(env)
	{
	}
	void ice_response()
	{
	}
	void ice_exception(const ::Ice::Exception& ex)
	{
		//_env._log(ZQ::common::Log::L_ERROR,"Caught a exception when invoke postYDT:%s",ex.ice_name().c_str());
	}
private:
	WeiwooSvcEnv& _env;
};
class setUserPropertyAmiImpl : public TianShanIce::Site::AMI_TxnService_setUserProperty
{
public:
	setUserPropertyAmiImpl(WeiwooSvcEnv& env):_env(env)
	{
	}
	void ice_response()
	{
	}
	void ice_exception(const ::Ice::Exception& ex)
	{
		//_env._log(ZQ::common::Log::L_ERROR,"Caught a exception when invoke setUserProperty:%s",ex.ice_name().c_str());
	}
private:
	WeiwooSvcEnv& _env;
};

#endif

class WeiwooSvcEnv
{
public:
    WeiwooSvcEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, 
					Ice::CommunicatorPtr& communicator, 
					const char* endpoint = DEFAULT_ENDPOINT_Weiwoo
					);
	virtual ~WeiwooSvcEnv();

	//init the env
	bool init(const char* databasePath = NULL,
		const char* runtimeDBFolder = NULL);

	/// check the configurations
	bool	validateConfig();

//	int     registerSnmp(void);

#ifdef WITH_ICESTORM
	void openPublisher(::IceStorm::TopicManagerPrx topicManager);
#endif // WITH_ICESTORM

	virtual void initWithConfig(void)
	{
		// do nothing here
	}

	//get business router proxy,if it is not open,open it through its endpoint
	::TianShanIce::Site::BusinessRouterPrx	getBusinessRouterPrx()
	{
		return _bizPrx;
	}

	::TianShanIce::Site::TxnServicePrx		getTxnServicePrx()
	{
		return _tnxPrx;
	}

	::TianShanIce::Transport::PathManagerPrx getPathManagerPrx()
	{
		return _pathmgr;
	}
	
public:	// exports all the members

//	DECLARE_INDEX(IdToSess);
	DECLARE_CONTAINER(Session);

	Ice::CommunicatorPtr	_communicator;
	
	//Ice::ObjectAdapterPtr		_adapter;
	ZQADAPTER_DECLTYPE          _adapter;

	WeiwooFactory::Ptr		_factory;
	
	TianShanIce::SRM::SessionEventSinkPrx _sessEventPublisher;
	ZQ::common::Mutex _lockSessEventSink;

	ZQ::common::NativeThreadPool& _thpool;
	SessionWatchDog			_watchDog;
	std::string				_programRootPath;
	// ZQ::Snmp::Subagent*     _weiwooSnmpAgnet;

	::TianShanIce::Transport::PathManagerPrx _pathmgr;

protected:
	
	::TianShanIce::Site::BusinessRouterPrx	 _bizPrx;
	::TianShanIce::Site::TxnServicePrx		_tnxPrx;

public:
#if  ICE_INT_VERSION / 100 >= 306

	TxnStateCBPtr commitCbPtr;
	TxnStateCBPtr traceCbPtr;
	TxnStateCBPtr postYTDCbPtr;
	TxnStateCBPtr setUserCbPtr;
	Purchase_detachCBPtr detachCBPtr;
	//Ice::CallbackPtr commitCB;
	//Ice::CallbackPtr traceCB;
	//Ice::CallbackPtr postYTDCB;
	//Ice::CallbackPtr setUserCB;
	//Ice::CallbackPtr detachCB;
#else
	TianShanIce::Site::AMI_TxnService_commitStateChangePtr		_commitStateChangePtr;
	TianShanIce::Site::AMI_TxnService_postYTDPtr				_postYTDPtr;
	TianShanIce::Site::AMI_TxnService_setUserPropertyPtr		_setUserPropertyPtr;
	TianShanIce::Site::AMI_TxnService_tracePtr					_txntracePtr;
	TianShanIce::Application::AMI_Purchase_detachPtr			_purchaseDetachPtr;
#endif

	// configurations
	std::string				_dbPath;
	std::string				_dbRuntimePath;
	std::string				_endpoint;
	std::string				_BussinessRouterEndpoint;


	long					_ttlIdleSess;		  ///< time-to-live for no operation on idle session at the state of "stNotProvisioned"
	long					_ttlOutOfServiceSess; ///< time-to-live for no operation on the session at the state of "stNotProvisioned"

	long					_ttlPathAllocation;   ///< time-to-live for the path tickets when they are initially allocated
	int						_maxTickets;		  ///< default maximal tickets during allocation

	std::string				_proxyPathMgr;		  ///< proxy string to PathManager

	ZQ::common::Log&		_log;

	TianShanIce::State		_serviceState;			

protected:
	
	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);
	

};

#define DBFILENAME_AppDict			"Apps"
#define DBFILENAME_SiteDict			"Sites"
#define DBFILENAME_AppMount			"AppMount"
#define DBFILENAME_Session			"Session"
#define INDEXFILENAME(_IDX)			#_IDX "Idx"

#define IdentityToObj(_CLASS, _ID) ::TianShanIce::SRM::_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#define envlog			(_env._log)



}} // namespace

#endif // __ZQTianShan_SessionImpl_H__
