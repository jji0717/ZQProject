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
// Ident : $Id: TimerWatcher.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPH_nPVR/TimerWatcher.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 1     09-02-20 18:34 Jie.zhang
// 
// 4     08-11-13 14:18 Yixin.tian
// modify to compile in Linux OS
// 
// 3     08-10-28 15:09 Hui.shao
// 
// 2     08-10-28 11:32 Hui.shao
// 
// 1     08-10-16 19:43 Hui.shao
// ===========================================================================
#include "TianShanDefines.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/timeb.h>
#include <time.h>
}
#endif


namespace ZQTianShan {

	namespace ContentProvision {


class TimerWatcher;


class TimerObject: public ::Ice::LocalObject
{
public:
	typedef IceUtil::Handle<TimerObject> Ptr;

	
	virtual void OnTimer() = 0;


	virtual Ice::Identity getIdentity()
	{
		return _ident;
	}

	virtual void setIdentity(const Ice::Identity& ident)
	{
		_ident = ident;
	}
protected:
	Ice::Identity		_ident;
};


// -----------------------------
// class TimeoutCmd
// -----------------------------
///
class TimeoutCmd : protected ZQ::common::ThreadRequest
{
	friend class TimerWatcher;
protected:
	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
	TimeoutCmd(const TimerObject::Ptr& ptrObj, ZQ::common::NativeThreadPool& pool, ZQ::common::Log& log);
	virtual ~TimeoutCmd() {}

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	ZQ::common::Log&				_log;
	TimerObject::Ptr	_ptrObj;

	std::string			_strLogHint;
};



// -----------------------------
// class TimerWatcher
// -----------------------------
/// classes to be under watching by this watchdog must be defined "class XXXX implements TianShanUtils::TimeoutObj" in ICE
class TimerWatcher : protected ZQ::common::NativeThread
{
	friend class TimeoutCmd;
public:
	/// constructor
	TimerWatcher(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const char* name=NULL, const int idleInterval =WatchDog_MAX_IDLE_INTERVAL);
	virtual ~TimerWatcher();

	bool start();

	///@param[in] ptrObject auto pointer of the object
	///@param[in] timeout the timeout to wake up timer to check the specified object
	void watch(const TimerObject::Ptr& ptrObject, long timeout =0);

	void remove(const TimerObject::Ptr& ptrObject);

	//quit watching
	void quit();

protected: // impls of ThreadRequest

	virtual bool init(void);
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false);

	void wakeup();

protected:

	struct Item
	{
		TimerObject::Ptr	ptrObj;
		Ice::Long			expireTime;
	};

	typedef std::map <Ice::Identity, Item> ExpirationMap; // sessId to expiration map
	typedef std::vector<TimerObject::Ptr>  TimeoutObjects;

	ZQ::common::Mutex   _lockExpirations;
	ExpirationMap		_expirations;

	ZQ::common::Log& _log;
	ZQ::common::NativeThreadPool& _thrdPool;

	bool		  _bQuit;
#ifdef ZQ_OS_MSWIN
	HANDLE		  _hWakeupEvent;
#else
	sem_t		  _hWakeupSem;
#endif
	::Ice::Long   _nextWakeup;

	int			  _idleInterval;

private:
	::std::string _name;
};


}} // namespace
