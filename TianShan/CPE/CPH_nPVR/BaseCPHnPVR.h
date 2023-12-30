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
// ===========================================================================

#ifndef __ZQTianShan_BaseNPVR_CPH_H__
#define __ZQTianShan_BaseNPVR_CPH_H__

#include "TianShanDefines.h"
#include "ICPHelper.h"
//#include "IMemAlloc.h"
#include "BufferPool.h"
#include "IPushTrigger.h"
#include "CPE.h"

#include "NativeThreadPool.h"

#include "TimerWatcher.h"

namespace ZQTianShan {
namespace ContentProvision {

#define CPM_FILENAME_PREFIX		"CPH_"

extern void initHelpers();
extern void unintHelpers();

class BaseCPHelper;
class BaseNPVRSession;
// -----------------------------
// class BaseNPVRSession
// -----------------------------
/// A per plugin interface exported from a MethodHelper object. The upper layer engine will invoke this
/// interface to drive the plugin to perform content provisioning
class BaseNPVRSession : public ICPHSession, public TimerObject
{
public:
	typedef IceUtil::Handle<BaseNPVRSession> Ptr;

	BaseNPVRSession(BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess);
	virtual ~BaseNPVRSession();

public: // impl of ICPHSession

	void destroy();

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
    void updateProgress(const ::Ice::Long processed, const ::Ice::Long total);

	virtual bool preLoad()
	{
		_bPreloaded = true;
		return true;		
	}
	
	virtual bool prime();	

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
	friend class BaseNPVRSession;
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

	ZQ::common::BufferPool*	getMemoryAllocator()
	{
		return _mgr->getMemoryAllocator();
	}

	virtual void cleanupSessions();
		
	ICPHSession* find(const char* provisionId);
	void  increaseLoad(const std::string& methodtype,int nbandwidth);
	void  decreaseLoad(const std::string& methodtype,int nbandwidth);
	void  getCurrentLoad(const std::string& methodtype, uint32& allocatedKbps,uint& sessions);
	
	void createTimer(TimerObject* pTimerObj, int nTimeOutInMs);
	void removeTimer(TimerObject* pTimerObj);

protected:
	bool reg(const ::std::string& provisionId, BaseNPVRSession::Ptr pSess);
	void unreg(const std::string& provisionId);
	

protected:

	typedef std::map <std::string, BaseNPVRSession::Ptr> SessMap; // map of provsionSessId to CPHSess within a CPHO
	SessMap _sessMap;
	ZQ::common::Mutex   _lockSessMap;
	ICPHManager*	_mgr;

	static std::string _confDir, _logDir;
	std::map<std::string,int> _allocSess;
	std::map<std::string,int> _countSess;

	TimerWatcher				_timerWatcher;
private:
	bool _bInCleanup;
};

#define EngHelperLog ((NULL ==_pCPELogger) ? ZQTianShan::ContentProvision::BaseCPHelper::NullLog : *_pCPELogger)
#define EngSessLog ((NULL ==_helper._pCPELogger) ? ZQTianShan::ContentProvision::BaseCPHelper::NullLog : *_helper._pCPELogger)

}} // namespace

#endif // __ZQTianShan_ICPHelper_H__

