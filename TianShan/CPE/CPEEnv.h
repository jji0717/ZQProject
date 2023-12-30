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
// Ident : $Id: CPEEnv.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPEEnv.h $
// 
// 8     4/01/15 10:01a Build
// cleaned old snmp
// 
// 6     8/10/12 1:45p Hongquan.zhang
// merge from maintree for snmp table view
// 
// 5     8/17/11 2:19p Li.huang
// add DirectIO for linux
// 
// 4     3/21/11 2:17p Li.huang
// fix bug 13454 1#
// 
// 3     10-12-15 14:13 Li.huang
// use new bufferpool
// 
// 2     10-12-03 15:35 Li.huang
// 13454
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 22    09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 24    09-05-08 14:14 Xia.chen
// change provisionstore pointer to object
// 
// 23    09-05-05 17:25 Xia.chen
// add sessions to provisionstore map
// 
// 22    09-05-05 13:20 Jie.zhang
// 
// 21    09-03-09 14:53 Yixin.tian
// modify LONGLONG to int64
// 
// 20    09-03-05 16:21 Jie.zhang
// 
// 19    09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 18    09-01-20 17:59 Jie.zhang
// 
// 17    08-12-19 15:58 Yixin.tian
// modify for Linux OS
// 
// 16    08-11-28 12:04 Jie.zhang
// 
// 15    08-11-18 10:59 Jie.zhang
// merge from TianShan1.8
// 
// 14    08-08-12 17:06 Xia.chen
// 
// 13    08-06-26 12:15 Jie.zhang
// add init() to cpeenv
// 
// 12    08-05-17 19:09 Jie.zhang
// 
// 11    08-05-13 11:31 Jie.zhang
// 
// 10    08-04-25 16:08 Jie.zhang
// 
// 16    08-04-22 18:28 Jie.zhang
// 
// 15    08-04-16 22:35 Build
// 
// 14    08-04-14 15:03 Build
// 
// 13    08-04-13 22:53 Build
// 
// 12    08-03-25 14:03 Jie.zhang
// 
// 11    08-03-24 19:41 Jie.zhang
// 
// 10    08-03-17 19:56 Jie.zhang
// 
// 9     08-03-07 21:29 Jie.zhang
// 
// 8     08-02-28 16:17 Jie.zhang
// 
// 7     08-02-21 18:27 Jie.zhang
// logs change and bug fixs
// 
// 6     08-02-20 16:16 Jie.zhang
// 
// 5     08-02-18 18:46 Jie.zhang
// changes check in
// 
// 4     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 3     08-02-14 16:28 Hongquan.zhang
// ProvisionWatchDog must destroyed freed before ProvisionFactory
// 
// 2     08-02-14 16:26 Hui.shao
// added ami callbacks
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_CPEEnv_H__
#define __ZQTianShan_CPEEnv_H__

#include "../common/TianShanDefines.h"

#include "CPE.h"
#include "CPEFactory.h"
#include "ProvisionFactory.h"

#include "Log.h"
#include "NativeThreadPool.h"

#include "ContentToProvision.h"
#include "FtpServer.h"
#include "CPCClient.h"
#include "ErrorProcess.h"
#include "IMethodCost.h"

#include "ProvisionStore.h"

//#include "SubAgent.hpp"
//#include "smival.hpp"

using namespace ZQ::common;
#ifdef _DEBUG
#  define UNATTEND_TIMEOUT			(60*60*1000) // 1 hours
#  define DEFAULT_SCH_ERROR_WIN		(60000) // 1 min
#  define MAX_START_DELAY			(60*1000) //1 min
#  define STOP_REMAIN_TIMEOUT		(5*1000) // 5 sec
#  define MIN_PROGRESS_INTERVAL		(10*1000) // 10 sec
#else
#  define UNATTEND_TIMEOUT			(120*1000) // 120 sec
#  define DEFAULT_SCH_ERROR_WIN		(5000) // 5 sec
#  define MAX_START_DELAY			(5*60*1000) // 5 min
#  define STOP_REMAIN_TIMEOUT		(60*1000) // 1 min
#  define MIN_PROGRESS_INTERVAL		(60*1000) // 60 sec
#endif // _DEBUG

#define MAX_IDLE (60*60*1000) // 1hour
#define DEFAULT_IDLE (5* 60*1000) // 5sec


namespace ZQTianShan {
namespace CPE {

#define DECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define DECLARE_INDEX(_IDX)	TianShanIce::ContentProvision::_IDX##Ptr _idx##_IDX
#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ

class ProvisionFactory;

// -----------------------------
// class ProvisionWatchDog
// -----------------------------
class ProvisionWatchDog : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    ProvisionWatchDog(CPEEnv& env);
	virtual ~ProvisionWatchDog();
	
	///@param[in] sessIdent identity of session
	///@param[in] timeout the timeout to wake up timer to check the specified session
	void watchSession(const ::Ice::Identity& sessIdent, long timeout);
	
	//quit watching
	void quit();
	
protected: // impls of ThreadRequest
	
	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false);
	
	void wakeup();
	
protected:
	
	typedef std::map <Ice::Identity, ::Ice::Long > ExpirationMap; // sessId to expiration map
	ZQ::common::Mutex   _lockExpirations;
	ExpirationMap		_expirations;
	::Ice::Long			_nextWakeup;
	
	CPEEnv& _env;
	bool		  _bQuit;
#ifdef ZQ_OS_MSWIN
	HANDLE		  _hWakeupEvent;
#else
	sem_t		  _wakeupSem;
#endif
};


// -----------------------------
// class CPEEnv
// -----------------------------
class CPEEnv : public ErrorProcFunc, ZQTianShan::ContentProvision::MethodCostCol
{
public:
    CPEEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, 
		Ice::CommunicatorPtr& communicator,ZQ::common::NativeThreadPool& timethreadPool);
	virtual ~CPEEnv();

	bool init(const char* endpoint, const char* databasePath, const char* runtimeDBFolder);
	bool start();

	/// check the configurations
	bool	validateConfig();
#ifdef ZQ_OS_MSWIN
	bool checkRegEdit();
	bool SetRegEdit(LPCTSTR dataset,char* module);
#else
	bool checkXmlCfg();
	bool setXmlCfg(std::string& dataset, std::string& module);
#endif

#ifdef WITH_ICESTORM
	void openPublisher(::IceStorm::TopicManagerPrx topicManager);
#endif // WITH_ICESTORM

	virtual void initWithConfig(void)
	{
		// do nothing here
	}

	void logProvisionSessionBindAmiCBException(const char* APIname, const ::Ice::Exception& ex);

//	::TianShanIce::ContentProvision::ProvisionSessionPrx createByRDSSession(const char* szContent, int nStartTime, int nEndTime, int nBitrate);
//	bool removeByRDSSession(const char* szContent, int nCode, const char* szReason);

	// return the provisioning session count
	int getProvisioningSessCount();

    void setMediaSampleBuffer(int mediasamplebuffer, int maxBufferPoolSize, int minBufferPoolSize);
public:

	virtual bool stopReceiveRequest();
	virtual bool restartApplication();
	virtual bool canRestartApplication();
	
	void processProvisionError(bool success, const std::string& strError, const std::string& strCode);

	bool provisionCost(const Ice::Identity& provisionIdent, const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKBps);
	
public:

	virtual MethodCost* getMethodCost(const std::string& methodType);


public:	// exports all the members

	DECLARE_INDEX(ContentToProvision);
	DECLARE_CONTAINER(ProvisionSession);

	Ice::CommunicatorPtr	_communicator;
	
	ZQADAPTER_DECLTYPE          _adapter;
	ZQ::common::NativeThreadPool& _thpool;
     ZQ::common::NativeThreadPool& _timerthpool;
	ProvisionWatchDog		_watchDog;

 	CPEFactory::Ptr		_factory;
	IceUtil::Handle<ProvisionFactory>	_provisionFactory;	
	
	std::string				_programRootPath;

	// configurations
	std::string				_dbPath;
	std::string				_dbRuntimePath;
	std::string				_endpoint;

	ZQ::common::Log&		_log;

	CPCClient				_cpcClient;
	FtpServer				_ftpServer;

	// configuraitons:
	unsigned int _scheduleErrorWindow; // error window in msec

	ErrorProcess			_errorProc;
    ProvisionStore          _pProvStore;

	int                     _mediasamplebuffer;
	int                     _maxBufferPoolSize;
	int                     _minBufferPoolSize;
	int                     _alignTo;

	SysLog*                 _pEventLog;
protected:
	ZQ::common::Mutex		_resourceBookLock;

	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);

public:
//	ZQ::Snmp::Subagent*  _cpsvcSnmpTableAgent;
};


#define DBFILENAME_ProvisionSession			"ProvSess"
#define INDEXFILENAME(_IDX)			#_IDX "Idx"
#define envlog			(_env._log)
#define IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::ContentProvision::_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))

}}; // namespace

#endif // __ZQTianShan_CPEEnv_H__
