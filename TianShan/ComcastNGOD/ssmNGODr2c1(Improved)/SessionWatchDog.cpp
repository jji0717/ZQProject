#include "SessionWatchDog.h"
#include "SessionTimerCommand.h"
#include ".\ssmNGODr2c1.h"

SessionWatchDog::SessionWatchDog(ssmNGODr2c1& env)
:ThreadRequest(*(env._pThreadPool)), _bQuit(false), _hWakeupEvent(NULL), _env(env), _nextWakeup(ZQTianShan::now() + MAX_TTL_IDLE_SESSION)
{
	_hWakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

SessionWatchDog::~SessionWatchDog()
{
	_bQuit = true;
	wakeup();

	::Sleep(1);

	if(_hWakeupEvent)
		::CloseHandle(_hWakeupEvent);
}

void SessionWatchDog::wakeup()
{
	::SetEvent(_hWakeupEvent);
}

bool SessionWatchDog::init(void)
{
	return (NULL != _hWakeupEvent && !_bQuit);
}

#define MIN_YIELD	(100)  // 100 msec

int SessionWatchDog::run()
{
	_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "Session WatchDog is running"));

	while(!_bQuit)
	{
		::Ice::Long timeOfNow = ZQTianShan::now();

		IdentCollection sessIdentsToExecute;

		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "%d session(s) under watching"), _expirations.size());

			_nextWakeup = timeOfNow + MAX_TTL_IDLE_SESSION;
			
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

		_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "%d session(s) timeout"), sessIdentsToExecute.size());
		for (IdentCollection::iterator it = sessIdentsToExecute.begin(); it < sessIdentsToExecute.end(); it++)
			(new SessionTimerCommand(_env, *it))->start();

		if(_bQuit)
			break;	// should quit polling

		long sleepTime = (long) (_nextWakeup - ZQTianShan::now());

		if (sleepTime < MIN_YIELD)
			sleepTime = MIN_YIELD;

		::WaitForSingleObject(_hWakeupEvent, sleepTime);

	} // while

	_env._fileLog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionWatchDog, "end of Session WatchDog"));

	return 0;
}
void SessionWatchDog::quit()
{
	_bQuit=true;
	wakeup();
}

void SessionWatchDog::watchSession(const ::Ice::Identity& sessIdent, long timeout)
{
	_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "require watch, GUID: [%s], timeout: [%d]"), sessIdent.name.c_str(), timeout);

	ZQ::common::MutexGuard gd(_lockExpirations);

	if (timeout <0)
		timeout = 0;
	::Ice::Long newExp = ZQTianShan::now() + timeout;
	
	_expirations[sessIdent] = newExp;
	
	if (newExp < _nextWakeup)
		wakeup();
}

void SessionWatchDog::final(int retcode, bool bCancelled)
{
	_env._fileLog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionWatchDog, "session watch dog stopped"));

#pragma message(__MSGLOC__"delete this, because I use it with a pointer.")
	delete this; // newly add
}
