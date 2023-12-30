// FileName : RtspSession.h
// Author   : Zheng Junming
// Date     : 2009-11
// Desc     : 

#ifndef __TS_HAMMER_RTSP_SESSION_H__
#define __TS_HAMMER_RTSP_SESSION_H__

// ZQ Common
#include "ZQ_common_conf.h"
#include "log.h"
#include "Locks.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"

// 
#include "RtspClient.h"
#include "XML_Handler/XML_SessCtxHandler.h"

namespace ZQHammer
{

class RtspSession;

// ------------------------------ RtspSessionMap ------------------------------------------------------
class RtspSessionMap
{
public:
	RtspSessionMap(ZQ::common::Log* log);
	~RtspSessionMap();

public:
	RtspSession* getSession(const std::string sessionID);
	RtspSession* getSession(DWORD sequenceID);
	RtspSession* getSession(ZQRtspEngine::RtspClientPtr communicator);

	bool addSession(const std::string sessionID, RtspSession* session);
	bool addSession(DWORD sequenceID, RtspSession* session);

	void removeSession(const std::string sessionID);
	void removeSession(DWORD sequenceID);

	int64 getSuccessSessions();
	void increaseSuccessSessions();
	void decreaseSuccessSessions(); 

	size_t getSessionNumbers();

private:
	std::map<std::string, RtspSession*> _IdMap;
	std::map<DWORD, RtspSession*> _sequenceMap;
	int64 _successSessions;

private:
	ZQ::common::Mutex _IdLock;
	ZQ::common::Mutex _sequenceLock;
	ZQ::common::Mutex _successSessionsLock;
	ZQ::common::Log* _log;
};

// ------------------------------- Session Watch Dog -------------------------------------------
class SessionWatchDog : public ZQ::common::NativeThread
{
public:
	/// constructor
	SessionWatchDog(::ZQ::common::NativeThreadPool& pool, ::ZQ::common::Log* log, RtspSessionMap& sessionMap);
	~SessionWatchDog();

public:
	///@param[in] sessIdent identity of session
	///@param[in] timeout the timeout to wake up timer to check the specified session
	void watchSession(::std::string sessId, uint64 timeout);
	void watchSession(DWORD uCSeq, uint64 timeout);

	void removeSession(::std::string sessId);
	void removeSession(DWORD uCSeq);

	//quit watching
	void quit();

protected:
	virtual bool init(void);
	virtual int run(void);
	void wakeup();

private:
	::ZQ::common::Log *_log;
	::ZQ::common::NativeThreadPool& _pool;

private:
	typedef ::std::map<::std::string, uint64> ExpirationMap; // sessId to expiration map
	typedef ::std::map<DWORD, uint64> ExpirationMap_CSeq;
	ExpirationMap		_expirations;
	ExpirationMap_CSeq	_expirations_CSeq;
	ZQ::common::Mutex   _lockExpirations;
	ZQ::common::Mutex   _lockExpirations_CSeq;

private:
	uint64 _nextWakeup;
	RtspSessionMap& _sessionMap;

private:
	bool _bQuit;
	HANDLE _hWakeupEvent;
};



// -------------------------------- OnTimerRequest --------------------------------------------
class OnTimerRequest : public ZQ::common::ThreadRequest
{
public:
	OnTimerRequest(::ZQ::common::NativeThreadPool &pool, ::ZQ::common::Log* log, 
		ZQHammer::RtspSession* session);
	~OnTimerRequest();

protected: // impls of ScheduleTask
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected:
	::ZQ::common::Log* _log;
	ZQHammer::RtspSession* _session;
	uint64 _usedMilli;
};

// ---------------------------------RtspSession -------------------------------------------
class RtspSessionManager;

class RtspSession
{
public:
	RtspSession(ZQ::common::Log* log, ZQRtspEngine::RtspClientPtr rtspCommunicator, 
		RtspSessionManager& rtspSessionManager, int loopNum, size_t requestNums, XML_SessCtxHandler& local_xml_SessCtxHandler);
	~RtspSession();

public:
	void onTimer();
	void onResponse(ZQRtspCommon::IRtspReceiveMsg *receiveMsg, ZQRtspCommon::IRtspSendMsg *sendMsg);
	void onCommunicatorError();
	ZQRtspEngine::RtspClientPtr getCommunicator();

private:
	void monitor(int timeout);
	void unmonitor();

	void addToMap();
	void removeFromMap();

private:
	ZQ::common::Log* _log;
	ZQRtspEngine::RtspClientPtr _rtspCommunicator;
	RtspSessionManager& _rtspSessionManager;
	int _loopNum;

private:
	size_t _requestNums;
	size_t _requestNumsBase;

private:
	DWORD _cseqIdx;
	DWORD _uLastCSeq;
	std::string _sessId;
	XML_SessCtxHandler _xml_SessCtxHandler;

// function need
private:
	bool _bRequest;
	bool _bWaitingForResponse;
	bool _bTimeout;
	bool _bQuit;
	
private:
	::ZQ::common::Mutex _mutex;
};

} // end for ZQHammer

#endif // end for __TS_HAMMER_RTSP_SESSION_H__
