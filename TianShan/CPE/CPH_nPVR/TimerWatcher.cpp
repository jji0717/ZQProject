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
// Ident : $Id: TimerWatcher.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPH_nPVR/TimerWatcher.cpp $
// 
// 2     7/26/13 1:13p Ketao.zhang
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
#include "TimerWatcher.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/timeb.h>
#include <time.h>
}
#endif

#define WatchDog_MIN_IDLE_INTERVAL (1000)


using namespace ZQ::common;

namespace ZQTianShan {

	namespace ContentProvision {




TimeoutCmd::TimeoutCmd(const TimerObject::Ptr& ptrObj, NativeThreadPool& pool, Log& log)
	: ThreadRequest(pool), _ptrObj(ptrObj), _log(log)
{
	_strLogHint = ptrObj->getIdentity().name;
}

int TimeoutCmd::run(void)
{
	try
	{	
		_ptrObj->OnTimer();
	}
	catch(...)
	{
		_log(Log::L_ERROR, CLOGFMT(TimeoutCmd, "[%s] OnTimer() unknown exception occurs"), _strLogHint.c_str());
	}

	return 0;
}

// -----------------------------
// class TimerWatcher
// -----------------------------
TimerWatcher::TimerWatcher(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const char* name, const int idleInterval)
: _log(log), _thrdPool(thrdPool), _bQuit(false), _idleInterval(idleInterval)
{
	_name = name ? name : "";
#ifdef ZQ_OS_MSWIN
	_hWakeupEvent = NULL;
	_hWakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	int nre = sem_init(&_hWakeupSem,0,0);
	if(nre == -1)
		perror("TimerWatcher() sem_init");
#endif

	if (_idleInterval < WatchDog_MIN_IDLE_INTERVAL)
		_idleInterval = WatchDog_MIN_IDLE_INTERVAL;

	if (_idleInterval > WatchDog_MAX_IDLE_INTERVAL)
		_idleInterval = WatchDog_MAX_IDLE_INTERVAL;
}

TimerWatcher::~TimerWatcher()
{
	quit();
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
}

void TimerWatcher::wakeup()
{
#ifdef ZQ_OS_MSWIN
	::SetEvent(_hWakeupEvent);
#else
	sem_post(&_hWakeupSem);
#endif
}

bool TimerWatcher::init(void)
{
#ifdef ZQ_OS_MSWIN
	return (NULL != _hWakeupEvent && !_bQuit);
#else
	int nV = 0;
	return (sem_getvalue(&_hWakeupSem,&nV) != -1 && !_bQuit);
#endif
}

#define MIN_YIELD	(100)  // 100 msec

int TimerWatcher::run()
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerWatcher, "watchdog[%s] start running"), _name.c_str());

	while(!_bQuit)
	{
		::Ice::Long timeOfNow = now();

		TimeoutObjects objsToExec;

		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			{
				static size_t _slastWatchSize =0;
				size_t watchSize=_expirations.size();

				if (watchSize != _slastWatchSize)
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerWatcher, "watchdog[%s] %d object(s) under watching"), _name.c_str(), watchSize);

				_slastWatchSize = watchSize;
			}

			_nextWakeup = timeOfNow + _idleInterval;
			
			for(ExpirationMap::iterator it = _expirations.begin(); it != _expirations.end(); it ++)
			{
				if (it->second.expireTime<= timeOfNow)
					objsToExec.push_back(it->second.ptrObj);
				else
					_nextWakeup = (_nextWakeup > it->second.expireTime) ? it->second.expireTime : _nextWakeup;
			}

			for (TimeoutObjects::iterator it2 = objsToExec.begin(); it2 < objsToExec.end(); it2++)
			{
				ExpirationMap::iterator it = _expirations.find((*it2)->getIdentity());
				if (it!=_expirations.end())
				{
					_expirations.erase(it);
				}
			}
		}

		if(_bQuit)
			break;	// should quit polling

		for (TimeoutObjects::iterator it = objsToExec.begin(); it < objsToExec.end(); it++)
			(new TimeoutCmd(*it, this->_thrdPool, this->_log))->start();
		
		objsToExec.clear();

		if(_bQuit)
			break;	// should quit polling

		long sleepTime = (long) (_nextWakeup - now());

		if (sleepTime < MIN_YIELD)
			sleepTime = MIN_YIELD;
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

	} // while

	_log(ZQ::common::Log::L_WARNING, CLOGFMT(TimerWatcher, "watchdog[%s] ends watching"), _name.c_str());

	return 0;
}

void TimerWatcher::quit()
{
	_bQuit=true;
	{
		ZQ::common::MutexGuard gd(_lockExpirations);
		_expirations.clear();
	}

	wakeup();
#ifdef ZQ_OS_MSWIN
	waitHandle(INFINITE);
#else
	waitHandle(-1);
#endif
}

void TimerWatcher::watch(const TimerObject::Ptr& ptrObject, long timeout)
{
	::Ice::Identity ident = ptrObject->getIdentity(); 

	ZQ::common::MutexGuard gd(_lockExpirations);

	if (timeout <0)
		timeout = 0;
	::Ice::Long newExp = now() + timeout;
	
	Item itm;
	itm.expireTime = newExp;
	itm.ptrObj = ptrObject;
	ExpirationMap::iterator it = _expirations.find(ident);
	if (_expirations.end() == it)
		_expirations.insert(ExpirationMap::value_type(ident, itm));
	else _expirations[ident] = itm;

	if (newExp < _nextWakeup)
		wakeup();
}

void TimerWatcher::final(int retcode, bool bCancelled)
{
	_log(ZQ::common::Log::L_WARNING, CLOGFMT(TimerWatcher, "watchdog[%s] stopped"), _name.c_str());
}

void TimerWatcher::remove( const TimerObject::Ptr& ptrObject )
{
	ZQ::common::MutexGuard gd(_lockExpirations);

	ExpirationMap::iterator it = _expirations.find(ptrObject->getIdentity());
	if (_expirations.end() != it)
	{
		_expirations.erase(it);
	}
}

bool TimerWatcher::start()
{
	return NativeThread::start();
}

}} // namespace
