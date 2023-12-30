#include "SessionWatchDog.h"
#include "OnTimerRequest.h"

SessionWatchDog::SessionWatchDog(::ZQ::common::NativeThreadPool &pool, ::ZQ::common::FileLog &fileLog, SessionMap &sessionMap)
:_bQuit(false) 
,_hWakeupEvent(NULL)
,_nextWakeup(GetTickCount())
,_pool(pool)
,_log(&fileLog)
,_sessionMap(sessionMap)
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
	while(!_bQuit)
	{
		uint64 timeOfNow = GetTickCount();

		//_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "%d Session under watch"), _expirations.size() + _expirations_CSeq.size());

		::std::vector<::std::string> sessIdCollection;
		::std::vector<uint16> sessIdCollect_CSeq;
		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			

			_nextWakeup = timeOfNow;
			
			for(ExpirationMap::iterator it = _expirations.begin(); it != _expirations.end(); it ++)
			{
				if (it->second <= timeOfNow)
					sessIdCollection.push_back(it->first);
				else
					_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
			}

			for(ExpirationMap_CSeq::iterator it = _expirations_CSeq.begin(); it != _expirations_CSeq.end(); it ++)
			{
				if (it->second <= timeOfNow)
					sessIdCollect_CSeq.push_back(it->first);
				else
					_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
			}

			//erase time out session
			for (::std::vector<::std::string>::iterator iter = sessIdCollection.begin(); iter != sessIdCollection.end(); iter ++)
				_expirations.erase(*iter);

			for (::std::vector<uint16>::iterator iter = sessIdCollect_CSeq.begin(); iter != sessIdCollect_CSeq.end(); iter ++)
				_expirations_CSeq.erase(*iter);

		}

		if(_bQuit)
			break;	// should quit polling

		
		for (::std::vector<::std::string>::iterator it = sessIdCollection.begin(); it < sessIdCollection.end(); it++)
			(new OnTimerRequest(_pool, *_log, _sessionMap, *it))->start();

		for (::std::vector<uint16>::iterator it = sessIdCollect_CSeq.begin(); it < sessIdCollect_CSeq.end(); it++)
			(new OnTimerRequest(_pool, *_log, _sessionMap, *it))->start();


		if(_bQuit)
			break;	// should quit polling

		long sleepTime = (long) (_nextWakeup - GetTickCount());

		if (sleepTime < MIN_YIELD)
			sleepTime = MIN_YIELD;

		::WaitForSingleObject(_hWakeupEvent, sleepTime);
	} // while

	return 0;
}
void SessionWatchDog::quit()
{
	_bQuit=true;
	wakeup();
}

void SessionWatchDog::watchSession(::std::string sessId, uint64 timeout)
{
	ZQ::common::MutexGuard gd(_lockExpirations);

	XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "add Watch Session[id:%s]"), sessId.c_str());

	if (timeout <0)
		timeout = 0;
	uint64 newExp = GetTickCount() + timeout;
	
	_expirations[sessId] = newExp;
	
	if (newExp < _nextWakeup)
		wakeup();
}

void SessionWatchDog::watchSession(uint16 uCSeq, uint64 timeout)
{
	ZQ::common::MutexGuard gd(_lockExpirations);

	XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "add Watch Session[CSeq:%d]"), uCSeq);

	if (timeout <0)
		timeout = 0;
	uint64 newExp = GetTickCount() + timeout;

	_expirations_CSeq[uCSeq] = newExp;

	if (newExp < _nextWakeup)
		wakeup();
}

void SessionWatchDog::removeSession(::std::string sessId)
{
	ZQ::common::MutexGuard gd(_lockExpirations);

	ExpirationMap::iterator it = _expirations.find(sessId);
	if (it!=_expirations.end())
	{
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "remove Watch Session[id:%s]"), sessId.c_str());
		_expirations.erase(it);
	}
}

void SessionWatchDog::removeSession(uint16 uCSeq)
{
	ZQ::common::MutexGuard gd(_lockExpirations);

	ExpirationMap_CSeq::iterator it = _expirations_CSeq.find(uCSeq);
	if (it!=_expirations_CSeq.end())
	{
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "remove Watch Session[CSeq:%d]"), uCSeq);

		_expirations_CSeq.erase(it);
	}
}