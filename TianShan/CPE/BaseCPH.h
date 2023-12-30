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
// Ident : $Id: BaseCPH.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/BaseCPH.h $
// 
// 3     7/15/13 11:25a Li.huang
// 
// 2     10-12-15 14:13 Li.huang
// use new bufferpool
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 12    10-04-07 15:24 Jie.zhang
// add parameter on updateProgress
// 
// 11    09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 11    09-04-16 13:57 Jie.zhang
// changed the memory allocator interface
// 
// 10    09-02-12 13:58 Yixin.tian
// 
// 9     09-01-20 17:59 Jie.zhang
// 
// 8     08-11-18 10:59 Jie.zhang
// merge from TianShan1.8
// 
// 9     08-10-24 14:49 Jie.zhang
// removed unnecesary items
// 
// 8     08-08-27 14:52 Xia.chen
// 
// 7     08-06-11 22:17 Jie.zhang
// 
// 6     08-03-27 16:15 Jie.zhang
// 
// 6     08-03-17 19:56 Jie.zhang
// 
// 5     08-02-28 16:17 Jie.zhang
// 
// 4     08-02-20 16:16 Jie.zhang
// 
// 3     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_BaseCPH_H__
#define __ZQTianShan_BaseCPH_H__

#include "../common/TianShanDefines.h"
#include "ICPHelper.h"
//#include "IMemAlloc.h"
#include "BufferPool.h"
#include "IPushTrigger.h"
#include "CPE.h"

#include "NativeThreadPool.h"

namespace ZQTianShan {
namespace ContentProvision {

#define CPM_FILENAME_PREFIX		"CPH_"

extern void initHelpers();
extern void unintHelpers();

class BaseCPHelper;
class BaseCPHSession;
// -----------------------------
// class BaseCPHSession
// -----------------------------
/// A per plugin interface exported from a MethodHelper object. The upper layer engine will invoke this
/// interface to drive the plugin to perform content provisioning
class BaseCPHSession : public ICPHSession, public ::Ice::LocalObject, protected ZQ::common::ThreadRequest
{
public:

	typedef IceUtil::Handle<BaseCPHSession> Ptr;

	BaseCPHSession(BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess);
	virtual ~BaseCPHSession();

public: // impl of ICPHSession
	
	void execute() { ThreadRequest::start(); };

	virtual void destroy() { delete this; }

	virtual bool isStreamable() const { return false; };

	virtual void setErrorInfo(int nCode, const char* szErrMsg){_nErrCode=nCode;if(szErrMsg)_strErrMsg=szErrMsg;}
	virtual const char* getErrorMsg() {return _strErrMsg.c_str();}
	virtual int getErrorCode() {return _nErrCode;}
	virtual void updateScheduledTime(const ::std::string& startTimeUTC, const ::std::string& endTimeUTC){};

protected:
	// access to the ProvisionSession
	void notifyStarted(const ::TianShanIce::Properties& params);
	void notifyStopped(bool errorOccurred, const ::TianShanIce::Properties& params);
	void notifyStreamable(bool streamable=true);
    void updateProgress(const ::Ice::Long processed, const ::Ice::Long total, const ::TianShanIce::Properties& params);

	// entries must be customized in the child classes
// 	virtual bool doInit()
// 	{
// 		_bInited = true;
// 		return _bInited;
// 	}
	virtual bool preLoad()
	{
		_bPreloaded = true;
		return true;		
	}
	
	virtual bool prime();	
	virtual void doCleanup() {}
	virtual int run(void) { return 0; }

private:
	// non-overwriteable impl of ThreadRequest
	bool init(void);
	void final(int retcode =0, bool bCancelled =false);

protected:
	
	BaseCPHelper&	_helper;
	::TianShanIce::ContentProvision::ProvisionSessionExPtr _sess;
	std::string _provisionId;
	bool _bInited;
	bool _bPreloaded;

	int	_nErrCode;
	std::string _strErrMsg;

	ZQTianShan::ContentProvision::IPushSource* _pPushSource;
};
#define CPHSESSLOGFMT(_C, _X) CLOGFMT(_C, "helperSess[%p|%s] " _X), this, _provisionId.c_str()

// -----------------------------
// class BaseCPHelper
// -----------------------------
/// A per plugin interface exported from a MethodHelper object. The upper layer engine will invoke this
/// interface to drive the plugin to perform content provisioning
class BaseCPHelper : public ICPHelper, public ::Ice::LocalObject
{
	friend class BaseCPHSession;
public:

	BaseCPHelper(ZQ::common::NativeThreadPool& pool, ICPHManager* mgr);
	virtual ~BaseCPHelper();

	ZQ::common::Log* _pCPELogger;
	static ZQ::common::Log NullLog;
	ZQ::common::NativeThreadPool& _pool;

public: // impl of ICPHelper

	ZQ::common::Log* getLog()
	{
		return _pCPELogger;
	}

	ZQ::common::NativeThreadPool& getThreadPool()
	{
		return _pool;
	}

//	IMemAlloc*	getMemoryAllocator()
	ZQ::common::BufferPool*	getMemoryAllocator()
	{
		return _mgr->getMemoryAllocator();
	}

	virtual void cleanupSessions();

    ///validate a potential ProvisionSession about to setup
	///@param[in] sess access to the ProvisionSession about to setup
	///@param[out] schema the collection of schema definition
	///@return true if succeeded
    virtual bool validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
		throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource) =0;
	
    virtual bool getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost) =0;
	
	virtual ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess) =0;
	ICPHSession* find(const char* provisionId);
	void  increaseLoad(const std::string& methodtype,int nbandwidth);
	void  decreaseLoad(const std::string& methodtype,int nbandwidth);
	void  getCurrentLoad(const std::string& methodtype, uint32& allocatedKbps,uint& sessions);
	
protected:
	bool reg(const ::std::string& provisionId, BaseCPHSession* pSess);
	void unreg(const std::string& provisionId, const BaseCPHSession* pSess);
	
protected:

	typedef std::map <std::string, BaseCPHSession *> SessMap; // map of provsionSessId to CPHSess within a CPHO
	SessMap _sessMap;
	ZQ::common::Mutex   _lockSessMap;
	ICPHManager*	_mgr;

	static std::string _confDir, _logDir;
	std::map<std::string,int> _allocSess;
	std::map<std::string,int> _countSess;

private:
	bool _bInCleanup;
};

#define EngHelperLog ((NULL ==_pCPELogger) ? ZQTianShan::ContentProvision::BaseCPHelper::NullLog : *_pCPELogger)
#define EngSessLog ((NULL ==_helper._pCPELogger) ? ZQTianShan::ContentProvision::BaseCPHelper::NullLog : *_helper._pCPELogger)

}} // namespace

#endif // __ZQTianShan_ICPHelper_H__

