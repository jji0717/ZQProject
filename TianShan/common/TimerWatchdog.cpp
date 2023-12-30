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
// Ident : $Id: TimerWatchDog.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/TimerWatchdog.cpp $
// 
// 6     1/07/14 11:09a Hui.shao
// 
// 5     1/07/14 11:05a Hui.shao
// 
// 4     1/07/14 11:04a Hui.shao
// 
// 3     1/07/14 10:57a Hui.shao
// avoid to post TimerCmd() after threadpool is released
// 
// 2     1/11/11 12:17p Hongquan.zhang
// use ZQ::common::Semaphore in WatchDog
// 
// 5     10-03-04 14:05 Jie.zhang
// fixed the issue of OnTimer skipped when some exception happen, cause
// OnTimer would not be called again. (based on OnTimer call to continue
// next call)
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

#define WatchDog_MIN_IDLE_INTERVAL (1000)

namespace ZQTianShan {

// -----------------------------
// class TimeoutCmd
// -----------------------------
///
class TimeoutCmd : protected ZQ::common::ThreadRequest
{
	friend class TimerWatchDog;
protected:
	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
    TimeoutCmd(TimerWatchDog& watchdog, const ::Ice::Identity& identObj);
	virtual ~TimeoutCmd() {}

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	TimerWatchDog& _watchdog;
	::Ice::Identity  _identObj;
};

TimeoutCmd::TimeoutCmd(TimerWatchDog& watchdog, const ::Ice::Identity& identObj)
: ThreadRequest(watchdog._thrdPool), _watchdog(watchdog), _identObj(identObj)
{
}

#define RETRY_TIME_MS	2000
int TimeoutCmd::run(void)
{
	::TianShanUtils::TimeoutObjPrx obj;

	try
	{
		obj = ::TianShanUtils::TimeoutObjPrx::checkedCast(_watchdog._adapter->createProxy(_identObj));
		obj->OnTimer();
		return 0;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_watchdog._log(ZQ::common::Log::L_WARNING, CLOGFMT(TimeoutCmd, "%s[%s] exception occurs: %s:%s, retry in %dms"), 
			_identObj.category.c_str(), _identObj.name.c_str(), ex.ice_name().c_str(), ex.message.c_str(), RETRY_TIME_MS);
		_watchdog.watch(_identObj, RETRY_TIME_MS);
	}
	catch(const ::Ice::Exception& ex)
	{
		_watchdog._log(ZQ::common::Log::L_WARNING, CLOGFMT(TimeoutCmd, "%s[%s] exception occurs: %s, retry in %dms"), 
			_identObj.category.c_str(), _identObj.name.c_str(), ex.ice_name().c_str(), RETRY_TIME_MS);
		_watchdog.watch(_identObj, RETRY_TIME_MS);
	}
	catch(...)
	{
		_watchdog._log(ZQ::common::Log::L_ERROR, CLOGFMT(TimeoutCmd, "%s[%s] unknown exception occurs"), _identObj.category.c_str(), _identObj.name.c_str());
	}

	return -1;
}

// -----------------------------
// class TimerWatchDog
// -----------------------------
TimerWatchDog::TimerWatchDog(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, ZQADAPTER_DECLTYPE& adapter, const char* name, const int idleInterval)
: ThreadRequest(thrdPool), _log(log), _thrdPool(thrdPool), _adapter(adapter), _bQuit(false), _idleInterval(idleInterval)
{
	_name = name ? name : "";
/*
#ifdef ZQ_OS_MSWIN
	_hWakeupEvent = NULL;
	_hWakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	int nre = sem_init(&_hWakeupSem,0,0);
	if(nre == -1)
		perror("TimerWatchDog() sem_init");
#endif
*/
	if (_idleInterval < WatchDog_MIN_IDLE_INTERVAL)
		_idleInterval = WatchDog_MIN_IDLE_INTERVAL;

	if (_idleInterval > WatchDog_MAX_IDLE_INTERVAL)
		_idleInterval = WatchDog_MAX_IDLE_INTERVAL;
}

TimerWatchDog::~TimerWatchDog()
{
	quit();
/*
#ifdef ZQ_OS_MSWIN
	if(_hWakeupEvent)
		::CloseHandle(_hWakeupEvent);
	_hWakeupEvent = NULL;
#else
	try
	{
		sem_destroy(&_hWakeupSem);
	}
	catch(...){}
#endif
*/
}

void TimerWatchDog::wakeup()
{
	_semWakeup.post();
/*
#ifdef ZQ_OS_MSWIN
	::SetEvent(_hWakeupEvent);
#else
	sem_post(&_hWakeupSem);
#endif
*/

}

bool TimerWatchDog::init(void)
{
/*
#ifdef ZQ_OS_MSWIN
	return (NULL != _hWakeupEvent && !_bQuit);
#else
	int nV = 0;
	return (sem_getvalue(&_hWakeupSem,&nV) != -1 && !_bQuit);
#endif
*/
	return true;
}

#define MIN_YIELD	(100)  // 100 msec

int TimerWatchDog::run()
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerWatchDog, "watchdog[%s] start running"), _name.c_str());

	while(!_bQuit)
	{
		::Ice::Long timeOfNow = now();

		IdentCollection objIdentsToExec;

		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			{
				static size_t _slastWatchSize =0;
				size_t watchSize=_expirations.size();

				if (watchSize != _slastWatchSize)
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerWatchDog, "watchdog[%s] %d object(s) under watching"), _name.c_str(), watchSize);

				_slastWatchSize = watchSize;
			}

			_nextWakeup = timeOfNow + _idleInterval;
			
			for(ExpirationMap::iterator it = _expirations.begin(); it != _expirations.end(); it ++)
			{
				if (it->second <= timeOfNow)
					objIdentsToExec.push_back(it->first);
				else
					_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
			}

			for (IdentCollection::iterator it2 = objIdentsToExec.begin(); it2 < objIdentsToExec.end(); it2++)
				_expirations.erase(*it2);
		}

		if(_bQuit)
			break;	// should quit polling

		for (IdentCollection::iterator it = objIdentsToExec.begin(); !_bQuit && it < objIdentsToExec.end(); it++)
		{
			try {
				(new TimeoutCmd(*this, *it))->start();
			}
			catch(...) {}
		}
		
		objIdentsToExec.clear ();

		if(_bQuit)
			break;	// should quit polling

		long sleepTime = (long) (_nextWakeup - now());

		if (sleepTime < MIN_YIELD)
			sleepTime = MIN_YIELD;
		_semWakeup.timedWait(sleepTime);
/*
#ifdef ZQ_OS_MSWIN
		::WaitForSingleObject(_hWakeupEvent, sleepTime);
#else
		struct timespec tsp;
		struct timeb tb;

		ftime(&tb);
		long mt = tb.millitm + sleepTime%1000;
		if(mt > 1000)
		{
			tb.time += 1;
			mt = mt%1000;
		}
		tsp.tv_sec = tb.time + sleepTime/1000;
		tsp.tv_nsec = mt * 1000000;

		sem_timedwait(&_hWakeupSem,&tsp);
#endif
*/
	} // while

	_log(ZQ::common::Log::L_WARNING, CLOGFMT(TimerWatchDog, "watchdog[%s] ends watching"), _name.c_str());

	return 0;
}

void TimerWatchDog::quit()
{
	{
		ZQ::common::MutexGuard gd(_lockExpirations);
		_expirations.clear();
		_bQuit=true;
	}

	wakeup();
#ifdef ZQ_OS_MSWIN
	::Sleep(1);
#else
	usleep(1000);
#endif
}

void TimerWatchDog::watch(const ::Ice::Identity& contentIdent, long timeout)
{
	if (timeout <0)
		timeout = 0;
	::Ice::Long newExp = now() + timeout;
			
	ZQ::common::MutexGuard gd(_lockExpirations);
	if (_bQuit)
		return;

	ExpirationMap::iterator it = _expirations.find(contentIdent);
	if (_expirations.end() == it)
		_expirations.insert(ExpirationMap::value_type(contentIdent, newExp));
	else _expirations[contentIdent] = newExp;

	if (newExp < _nextWakeup)
		wakeup();
}

void TimerWatchDog::final(int retcode, bool bCancelled)
{
	_log(ZQ::common::Log::L_WARNING, CLOGFMT(TimerWatchDog, "watchdog[%s] stopped"), _name.c_str());
}


} // namespace
