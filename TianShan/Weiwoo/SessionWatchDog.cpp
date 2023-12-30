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
// Ident : $Id: SessionWatchDog.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionWatchDog.cpp $
// 
// 2     3/07/11 4:59p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 12    07-08-30 15:46 Hongquan.zhang
// 
// 11    07-05-09 18:28 Hongquan.zhang
// 
// 10    07-05-09 18:16 Hongquan.zhang
// 
// 9     07-05-09 17:54 Hongquan.zhang
// 
// 8     07-01-05 14:53 Hui.shao
// 
// 7     07-01-05 10:59 Hongquan.zhang
// 
// 6     06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 5     06-10-16 12:59 Hongquan.zhang
// 
// 4     10/09/06 4:18p Hui.shao
// more log messages
// 
// 3     9/21/06 4:36p Hui.shao
// batch checkin 20060921
// 
// 2     06-09-12 20:19 Hui.shao
// added SessionWatchDog
// ===========================================================================


#include "SessionWatchDog.h"
#include "SessionState.h"
#include "SessionCommand.h"

namespace ZQTianShan {
namespace Weiwoo {

// -----------------------------
// class SessionWatchDog
// -----------------------------
SessionWatchDog::SessionWatchDog(WeiwooSvcEnv& env)
:ThreadRequest(env._thpool), _bQuit(false), _env(env), _nextWakeup(now() + MAX_TTL_IDLE_SESSION)
{
}

SessionWatchDog::~SessionWatchDog()
{
	_bQuit = true;
	wakeup();

	SYS::sleep(1);
}

void SessionWatchDog::wakeup()
{
	_hWakeupEvent.signal();
}

bool SessionWatchDog::init(void)
{
	return (!_bQuit);
}

#define MIN_YIELD	(100)  // 100 msec

int SessionWatchDog::run()
{
	_env._log(ZQ::common::Log::L_DEBUG,"SessionWatchDog() Session WatchDog is running");
	while(!_bQuit)
	{
		::Ice::Long timeOfNow = now();

		IdentCollection sessIdentsToExecute;

		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "%d session(s) under watching"), _expirations.size());
//#ifdef _DEBUG
//			_nextWakeup = timeOfNow + _env._ttlIdleSess;
//#else // _DEBUG
			_nextWakeup = timeOfNow + MAX_TTL_IDLE_SESSION;
//#endif// _DEBUG
			
			for(ExpirationMap::iterator it = _expirations.begin(); it != _expirations.end(); it ++)
			{
				if (it->second <= timeOfNow)
					sessIdentsToExecute.push_back(it->first);
				else
					_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
			}

			for (IdentCollection::iterator it2 = sessIdentsToExecute.begin(); it2 < sessIdentsToExecute.end(); it2++)
				_expirations.erase(*it2);
		}

		if(_bQuit)
			break;	// should quit polling

		for (IdentCollection::iterator it = sessIdentsToExecute.begin(); it < sessIdentsToExecute.end(); it++)
			(new SessionTimerCommand(_env, *it))->start();

		if(_bQuit)
			break;	// should quit polling

		long sleepTime = (long) (_nextWakeup - now());

		if (sleepTime < MIN_YIELD)
			sleepTime = MIN_YIELD;

		_hWakeupEvent.wait(sleepTime);

	} // while

	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionWatchDog, "end of Session WatchDog"));

	return 0;
}
void SessionWatchDog::quit()
{
	_bQuit=true;
	wakeup();
}
void SessionWatchDog::watchSession(const ::Ice::Identity& sessIdent, long timeout)
{
	ZQ::common::MutexGuard gd(_lockExpirations);

//	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "watchSession() sess[%s]"), sessIdent.name.c_str());

	if (timeout <0)
		timeout = 0;
	::Ice::Long newExp = now() + timeout;
			
	ExpirationMap::iterator it = _expirations.find(sessIdent);

	//if (_expirations.end() == it /*|| it->second < newExp*/)
	{
		_expirations[sessIdent] = newExp;

		if (newExp < _nextWakeup)
			wakeup();
	}
}

void SessionWatchDog::final(int retcode, bool bCancelled)
{
	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionWatchDog, "session watch dog stopped"));
}

}} // namespace
