#include "RtspSession.h"
#include "TimeUtil.h"
#include "RtspSessionManager.h"

extern SessCSeq g_SessCseq(1);

namespace ZQHammer
{

RtspSessionMap::RtspSessionMap(ZQ::common::Log* log)
:_log(log), _successSessions(0)
{
}

RtspSessionMap::~RtspSessionMap()
{
	_log = NULL;
}

int64 RtspSessionMap::getSuccessSessions()
{
	ZQ::common::MutexGuard mutexGuard(_successSessionsLock);
	return _successSessions;
}

void RtspSessionMap::increaseSuccessSessions()
{
	ZQ::common::MutexGuard mutexGuard(_successSessionsLock);
	_successSessions++;
}

void RtspSessionMap::decreaseSuccessSessions()
{
	ZQ::common::MutexGuard mutexGuard(_successSessionsLock);
	_successSessions--;
}

RtspSession* RtspSessionMap::getSession(const std::string sessionID)
{
	if (sessionID.empty())
	{
		return NULL;
	}
	//ZQ::common::AutoReadLock readLock(_IdLock);
	ZQ::common::MutexGuard mutexGuard(_IdLock);
	std::map<std::string, RtspSession*>::iterator iter = _IdMap.find(sessionID);
	if (iter != _IdMap.end())
	{
		return iter->second;
	}
	return NULL;
}

RtspSession* RtspSessionMap::getSession(DWORD sequenceID)
{
	if (sequenceID <= 0)
	{
		return NULL;
	}
	//ZQ::common::AutoReadLock readLock(_sequenceLock);
	ZQ::common::MutexGuard mutexGuard(_sequenceLock);
	std::map<DWORD, RtspSession*>::iterator iter = _sequenceMap.find(sequenceID);
	if (iter != _sequenceMap.end())
	{
		return iter->second;
	}
	return NULL;
}

RtspSession* RtspSessionMap::getSession(ZQRtspEngine::RtspClientPtr communicator)
{
	if (communicator == NULL)
	{
		return NULL;
	}

	{
		//ZQ::common::AutoReadLock readLock(_sequenceLock);
		ZQ::common::MutexGuard mutexGuard(_sequenceLock);
		std::map<DWORD, RtspSession*>::iterator iter = _sequenceMap.begin();
		for(; iter != _sequenceMap.end(); iter++)
		{
			if( (iter->second)->getCommunicator() == communicator )
			{
				return iter->second;
			}
		}
	}

	{
		//ZQ::common::AutoReadLock readLock(_IdLock);
		ZQ::common::MutexGuard mutexGuard(_IdLock);
		std::map<std::string, RtspSession*>::iterator iter = _IdMap.begin();
		for (; iter != _IdMap.end(); iter++)
		{
			if( (iter->second)->getCommunicator() == communicator )
			{
				return iter->second;
			}
		}
	}
	return NULL;
}

bool RtspSessionMap::addSession(DWORD sequenceID, RtspSession* session)
{
	if (sequenceID <= 0 || NULL == session)
	{
		return false;
	}
	//ZQ::common::AutoWriteLock writeLock(_sequenceLock);
	ZQ::common::MutexGuard mutexGuard(_sequenceLock);
	std::map<DWORD, RtspSession*>::iterator iter = _sequenceMap.find(sequenceID);
	if (iter != _sequenceMap.end())
	{
		XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RtspSessionMap, "addSession() failed to add session as sequence id [%d] is already in rtsp session map"), sequenceID);
		return false;
	}
	else
	{
		_sequenceMap.insert(std::make_pair(sequenceID, session));
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionMap, "addSession() successed to add session [%p] with sequence id [%d] "), session, sequenceID);
		return true;
	}
}

bool RtspSessionMap::addSession(const std::string sessionID, ZQHammer::RtspSession *session)
{
	if (sessionID.empty() || NULL == session)
	{
		return false;
	}
	//ZQ::common::AutoWriteLock writeLock(_IdLock);
	ZQ::common::MutexGuard mutexGuard(_IdLock);
	std::map<std::string, RtspSession*>::iterator iter = _IdMap.find(sessionID);
	if (iter != _IdMap.end())
	{
		XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RtspSessionMap, "addSession() failed to add session as id [%s] already in rtsp session map"), sessionID.c_str());
		return false;
	}
	else
	{
		_IdMap.insert(std::make_pair(sessionID, session));
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionMap, "addSession() success to add Session[%p] with id[%s]"), session, sessionID.c_str());
		return true;
	}
}

void RtspSessionMap::removeSession(const std::string sessionID)
{
	if (sessionID.empty())
	{
		return ;
	}
	//ZQ::common::AutoWriteLock writeLock(_IdLock);
	ZQ::common::MutexGuard mutexGuard(_IdLock);
	std::map<std::string, RtspSession*>::iterator iter = _IdMap.find(sessionID);
	if (iter != _IdMap.end())
	{
		_IdMap.erase(sessionID);
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionMap, "removeSession() successed to remove session with  session id [%s]"), sessionID.c_str());
	}
	else
	{
		XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RtspSessionMap, "removeSession() failed to remove session with  session id [%s]"), sessionID.c_str());
	}
}

void RtspSessionMap::removeSession(DWORD uCSeq)
{
	if (uCSeq <= 0)
	{
		return ;
	}
	//ZQ::common::AutoWriteLock writeLock(_sequenceLock);
	ZQ::common::MutexGuard mutexGuard(_sequenceLock);
	std::map<DWORD, RtspSession*>::iterator iter = _sequenceMap.find(uCSeq);
	if (iter != _sequenceMap.end())
	{
		_sequenceMap.erase(iter);
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionMap, "removeSession() successed to remove session with sequence id [%d]"), uCSeq);
	}
	else
	{
		XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RtspSessionMap, "removeSession() failed to remove session with sequence id [%d]"), uCSeq);
	}
}

size_t RtspSessionMap::getSessionNumbers()
{
	//ZQ::common::AutoReadLock readLock(_sequenceLock);
	ZQ::common::MutexGuard mutexGuard(_sequenceLock);
	return _sequenceMap.size();
}

// ---------------------------- Implement of session watch dog
SessionWatchDog::SessionWatchDog(::ZQ::common::NativeThreadPool& pool, ::ZQ::common::Log* log, RtspSessionMap &sessionMap)
:_log(log), _pool(pool), _nextWakeup(0), _sessionMap(sessionMap), _bQuit(false), _hWakeupEvent(NULL)
{

}

SessionWatchDog::~SessionWatchDog()
{
	quit();

	if(_hWakeupEvent)
	{
		CloseHandle(_hWakeupEvent);
		_hWakeupEvent = NULL;
	}
}

void SessionWatchDog::wakeup()
{
	::SetEvent(_hWakeupEvent);
}

bool SessionWatchDog::init(void)
{
	_hWakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (_hWakeupEvent == NULL || _bQuit)
	{
		return false;
	}
	return true;
}

#define MIN_YIELD	(100)  // 100 msec

int SessionWatchDog::run()
{	
	while(!_bQuit)
	{
		uint64 timeOfNow = GetTickCount();
		_nextWakeup = timeOfNow;

		::std::vector<::std::string> sessIdCollection;
		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			for(ExpirationMap::iterator it = _expirations.begin(); it != _expirations.end(); it ++)
			{
				if (it->second <= timeOfNow)
				{
					sessIdCollection.push_back(it->first);
				}
				else
				{
					_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
				}
			}

			//erase time out session
			::std::vector<::std::string>::iterator iter = sessIdCollection.begin();
			for (; iter != sessIdCollection.end(); iter ++)
			{
				_expirations.erase(*iter);
			}
		}
		
		for (::std::vector<::std::string>::iterator it = sessIdCollection.begin(); it != sessIdCollection.end(); it++)
		{
			RtspSession* session = _sessionMap.getSession(*it);
			if (session != NULL)
			{
				OnTimerRequest* request = new (std::nothrow) OnTimerRequest(_pool, _log, session);
				if (request)
				{
					request->start();
				}
			}
		}

		if(_bQuit)
		{
			break;	// should quit polling
		}

		::std::vector<DWORD> cseqCollection;
		{
			ZQ::common::MutexGuard gd(_lockExpirations_CSeq);
			ExpirationMap_CSeq::iterator it = _expirations_CSeq.begin();
			for(; it != _expirations_CSeq.end(); it ++)
			{
				if (it->second <= timeOfNow)
				{
					cseqCollection.push_back(it->first);
				}
				else
				{
					_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
				}
			}
			for (::std::vector<DWORD>::iterator iter = cseqCollection.begin(); iter != cseqCollection.end(); iter ++)
			{
				_expirations_CSeq.erase(*iter);
			}
		}
		
		for (::std::vector<DWORD>::iterator it = cseqCollection.begin(); it != cseqCollection.end(); it++)
		{
			RtspSession* session = _sessionMap.getSession(*it);
			if (session != NULL)
			{
				OnTimerRequest* request = new (std::nothrow) OnTimerRequest(_pool, _log, session);
				if (request)
				{
					request->start();
				}
			}
		}

		if(_bQuit)
		{
			break;	// should quit polling
		}

		long sleepTime = (long) (_nextWakeup - GetTickCount());
		if (sleepTime < MIN_YIELD)
		{
			sleepTime = MIN_YIELD;
		}

		::WaitForSingleObject(_hWakeupEvent, sleepTime);
	} // while

	return 0;
}
void SessionWatchDog::quit()
{
	if (!_bQuit)
	{
		_bQuit=true;
		wakeup();
		waitHandle(INFINITE);
	}
}

void SessionWatchDog::watchSession(::std::string sessId, uint64 timeout)
{
	if (sessId.empty())
	{
		XMLLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SessionWatchDog, "watchSession() failed to add Watch Session[id:%s]"), sessId.c_str());
		return;
	}
	if (timeout < 0)
	{
		timeout = 0;
	}
	uint64 newExp = GetTickCount() + timeout;
	ZQ::common::MutexGuard gd(_lockExpirations);
	_expirations[sessId] = newExp;
	if (newExp < _nextWakeup)
	{
		wakeup();
	}
	XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "watchSession() successed to add Watch Session[id:%s]"), sessId.c_str());
}

void SessionWatchDog::watchSession(DWORD uCSeq, uint64 timeout)
{
	if (uCSeq <= 0)
	{
		XMLLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SessionWatchDog, "watchSession() failed to add Watch Session[CSeq:%d]"), uCSeq);
	}
	if (timeout <0)
	{
		timeout = 0;
	}
	uint64 newExp = GetTickCount() + timeout;

	ZQ::common::MutexGuard gd(_lockExpirations_CSeq);
	_expirations_CSeq[uCSeq] = newExp;
	if (newExp < _nextWakeup)
	{
		wakeup();
	}
	XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "add Watch Session[CSeq:%d]"), uCSeq);
}

void SessionWatchDog::removeSession(::std::string sessId)
{
	if(sessId.empty())
	{
		return;
	}
	ZQ::common::MutexGuard gd(_lockExpirations);
	ExpirationMap::iterator it = _expirations.find(sessId);
	if (it != _expirations.end())
	{
		_expirations.erase(it);
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "remove Watch Session[id:%s]"), sessId.c_str());
	}
}

void SessionWatchDog::removeSession(DWORD uCSeq)
{
	if (uCSeq <= 0)
	{
		return;
	}
	ZQ::common::MutexGuard gd(_lockExpirations_CSeq);
	ExpirationMap_CSeq::iterator it = _expirations_CSeq.find(uCSeq);
	if (it != _expirations_CSeq.end())
	{
		_expirations_CSeq.erase(it);
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionWatchDog, "remove Watch Session[CSeq:%d]"), uCSeq);
	}
}

// --------------------------- Implement of --------------------------------------------
OnTimerRequest::OnTimerRequest(ZQ::common::NativeThreadPool &pool, ZQ::common::Log* log, ZQHammer::RtspSession *session)
:ThreadRequest(pool), _log(log), _session(session), _usedMilli(0)
{

}

OnTimerRequest::~OnTimerRequest()
{

}
bool OnTimerRequest::init()
{
	_usedMilli = GetTickCount();
	//XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "start() GUID: [%s](CSeq[%d])"), _sessId.c_str(), _uCSeq);
	return true;
}

int OnTimerRequest::run(void)
{
	if (_session != NULL)
	{
		_session->onTimer();
	}
	return 1;
}

void OnTimerRequest::final(int retcode, bool bCancelled)
{
	_usedMilli = GetTickCount() - _usedMilli;
	//XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "Session timeout leave, GUID: [%s] or CSeq[%d], used time: [%lld]"), _sessId.c_str(), _uCSeq, _usedMilli);
	delete this;
}

// --------------------------- Implement of RtspSession ----------------------------------

RtspSession::RtspSession(ZQ::common::Log* log, 
						 ZQRtspEngine::RtspClientPtr rtspCommunicator, 
						 RtspSessionManager& rtspSessionManager, 
						 int loopNum,
						 size_t requestNums,
						 XML_SessCtxHandler& local_xml_SessCtxHandler)
:_log(log), _rtspCommunicator(rtspCommunicator), _rtspSessionManager(rtspSessionManager), 
_loopNum(loopNum),_requestNums(0), _requestNumsBase(requestNums), 
_cseqIdx(0), _uLastCSeq(0), _sessId(""), 
_bRequest(true), _bWaitingForResponse(false), _bTimeout(false), _bQuit(false) 
{
	_xml_SessCtxHandler = local_xml_SessCtxHandler;
	_xml_SessCtxHandler.setTailorProp(rtspSessionManager._xml_ClientHandler.strTailorType, rtspSessionManager._xml_ClientHandler.tailorRange);
}

RtspSession::~RtspSession()
{
	_rtspCommunicator = NULL;
}

void RtspSession::onTimer()
{
	::ZQ::common::MutexGuard guard(_mutex);
	if (_bQuit)
	{
		return;
	}
	if (_bWaitingForResponse)
	{
		_bTimeout = true;
		_bWaitingForResponse = false;
		XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RtspSession, "onTimer() repsonse [Cseq: %lld] is time out on communicator[%lld]"), _uLastCSeq, _rtspCommunicator->getCommunicatorId());
	}
	while (_loopNum > 0)
	{
		while (_requestNums < _requestNumsBase)
		{
			if (_bRequest)
			{
				if (_rtspSessionManager.getRequestSkip(_requestNums) && _bTimeout)
				{
					_requestNums++;
					continue;
				}
				else
				{
					_bTimeout = false;
				}
				std::string strCSeq = g_SessCseq.getCSeq();
				_uLastCSeq = atol(strCSeq.c_str());
				_xml_SessCtxHandler.updateMacro(SESSCSEQ, strCSeq);
				addToMap();
				std::string strRequest;
				if (_rtspSessionManager.composeRequest(this, _xml_SessCtxHandler, _requestNums, strRequest))
				{
					_rtspCommunicator->sendRawMsg(strRequest);
				}
				_bWaitingForResponse = true;
				_bRequest = false;
				monitor(_rtspSessionManager._adjustTimeout);
				return ;
			}
			_bRequest = true;
			if (!_bTimeout)
			{
				int sleepTime = _rtspSessionManager.getSleepTime(_requestNums);
				monitor(sleepTime);
				_requestNums++;
				return; 
			}
			_requestNums++;
		}
		_loopNum--;
		_requestNums = 0;
	}
	unmonitor();
	removeFromMap();
	_bQuit = true;
}

void RtspSession::addToMap()
{
	if (_sessId.empty())
	{
		if (_cseqIdx > 0)
		{
			_rtspSessionManager._sessionMap->removeSession(_cseqIdx);
		}
		_cseqIdx = _uLastCSeq;
		_rtspSessionManager._sessionMap->addSession(_cseqIdx, this);
	}
}

void RtspSession::removeFromMap()
{
	_rtspSessionManager._sessionMap->removeSession(_cseqIdx);
	_rtspSessionManager._sessionMap->removeSession(_sessId);
}

void RtspSession::monitor(int timeout)
{
	// add into session map and session watch dog
	if (_sessId.empty())
	{
		_rtspSessionManager._sessionWatchDog->watchSession(_cseqIdx, timeout);
	}
	else
	{
		_rtspSessionManager._sessionWatchDog->watchSession(_sessId, timeout);
	}
}

void RtspSession::unmonitor()
{
	_rtspSessionManager._sessionWatchDog->removeSession(_cseqIdx);
	_rtspSessionManager._sessionWatchDog->removeSession(_sessId);
}

void RtspSession::onResponse(ZQRtspCommon::IRtspReceiveMsg *receiveMsg, ZQRtspCommon::IRtspSendMsg *sendMsg)
{
	{
		::ZQ::common::MutexGuard guard(_mutex);
		unmonitor();
		std::string strMethod = receiveMsg->getHeader("Method-Code");
		if ("SETUP" == strMethod)
		{
			_sessId = receiveMsg->getHeader("Session");
			_rtspSessionManager._sessionMap->addSession(_sessId, this);
			if (200 == receiveMsg->getStatus())
			{
				_rtspSessionManager._sessionMap->increaseSuccessSessions();
			}
		}

		if ("TEARDOWN" == strMethod)
		{
			if (200 == receiveMsg->getStatus())
			{
				_rtspSessionManager._sessionMap->decreaseSuccessSessions();
			}
		}



		_bWaitingForResponse = false;
		_rtspSessionManager.parseResponse(_requestNums, receiveMsg->getRawMsg(), _xml_SessCtxHandler);
	}
	OnTimerRequest* onTimeRequest = new (std::nothrow) OnTimerRequest(_rtspSessionManager._sessionPool, _log, this);
	if (onTimeRequest)
	{
		onTimeRequest->start();
	}
}

void RtspSession::onCommunicatorError()
{
	::ZQ::common::MutexGuard guard(_mutex);
	unmonitor();
	removeFromMap();
	_bQuit = true;
}

ZQRtspEngine::RtspClientPtr RtspSession::getCommunicator()
{
	return _rtspCommunicator;
}

} // end for ZQHammer