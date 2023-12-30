#ifndef __SessionWatchDog_H__
#define __SessionWatchDog_H__

#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "ZQ_common_conf.h"
#include "FileLog.h"
#include "ConfigHelper.h"

#include "XML_SessCtxHandler.h"
#include "XML_RequestHandler.h"
#include "XML_ResponseHandler.h"
#include "XML_RTSPServerHandler.h"
#include "XML_SleepHandler.h"
#include "RTSP_common_structure.h"

#define REQ_TIMEOUT 5000
#define DEFAULT_TIMEOUT 100

class SessionMap;
class SessionWatchDog;

//session handler class
//response the OnTimer function call and parse the RTSP response message
class SessionHandler
{
public:
	SessionHandler(SessionWatchDog &sessionWatchDog, ::ZQ::common::FileLog &fileLog, ::ZQ::common::NativeThreadPool &pool, XML_RtspServerHandler &xml_RtspServerHandler, ::ZQ::common::XMLPreferenceDocumentEx &root, XML_SessCtxHandler &gxml_SessCtxHandler, SessionMap &sessIdMap, uint32 loopNum);
	~SessionHandler();

	void	PostRequest(::std::string &strMessage);
	void	OnTimer();
	void	OnResponse(const char* msg);

	::std::string					_sessId;
	uint16							_uLastCSeq;
	SessionSocket					*_sessionSocket;

	inline void lock()
	{
		//EnterCriticalSection(&_CS);
		//_rwLock.WriteLock();
		_mutex.enter();
	}
	inline void unlock()
	{
		//LeaveCriticalSection(&_CS);
		//_rwLock.WriteUnlock();
		_mutex.leave();
	}

	inline bool getExit()
	{
		//::ZQ::common::AutoReadLock rlock(_rwLock);
		::ZQ::common::MutexGuard guard(_mutex);
		return _bExit;
	}
	inline void setExit(bool bExit)
	{
		//::ZQ::common::AutoWriteLock wlock(_rwLock);
		::ZQ::common::MutexGuard guard(_mutex);
		_bExit = bExit;
	}
	uint16							_cseqIdx;

private:
	bool							_bConnected;//if connect to server
	int								_responseTimeout;
	//CRITICAL_SECTION				_CS;
	//::ZQ::common::RWLock			_rwLock;
	::ZQ::common::Mutex				_mutex;

	int								_timeout;
	bool							bWaitingForResponse;
	SessionMap						&_sessIdMap;
	XML_RtspServerHandler			_xml_RtspServerHandler;	
	SessionWatchDog					&_sessionWatchDog;
	XML_SessCtxHandler				_xml_SessCtxHandler;//local sessionCtx
	XML_SessCtxHandler				&_gxml_SessCtxHandler;//global sessionCtx
	XML_RequestHandler				_xml_RequestHandler;
	XML_ResponseHandler				_xml_ResponseHandler;
	XML_SleepHandler				_xml_SleepHandler;
	::ZQ::common::FileLog			*_log;
	::ZQ::common::NativeThreadPool	&_pool;
	::ZQ::common::XMLUtil::XmlNode	_XMLNodeRoot;
	::ZQ::common::XMLUtil::XmlNode	_XMLRootNodeBase;
	::ZQ::common::XMLPreferenceEx	*_XMLChildNode;
	bool							bXMLEnd;

	ZQ::common::Mutex				_lockExpirations;
	uint32							_loopNum;
	bool							bTimeout;

	bool							_bExit;

	void	nextXMLNode()
	{
		//delete _XMLChildNode;
		//_XMLChildNode = NULL;
		_XMLChildNode->free();
		_XMLChildNode = _XMLNodeRoot->nextChild();
	}
};

typedef ::std::map<::std::string, SessionHandler*> SessionHandlerMap; 

class SessionMap//session map class, restore the session list
{
public:
	SessionMap(::ZQ::common::FileLog &fileLog, XML_RtspServerHandler &xml_RtspServerHandler);
	~SessionMap();

	SessionHandler	*getSessionHandler(::std::string &sessId);
	SessionHandler	*getSessionHandler(uint16 uCSeq);
	SessionHandler	*getSessionHandler(SOCKET sock);

	bool			addSessionHandler(::std::string &sessId, SessionHandler *sessionHandler);
	bool			addSessionHandler(uint16 uCSeq, SessionHandler *sessionHandler);

	bool			removeSessionHandler(SessionHandler *sessionHandler);
	bool			removeSessionHandlerByCSeq(SessionHandler *sessionHandler);

	bool			removeSessionHandlerByCSeq(uint16 CSeq);

	uint16			getSessionHandlerCSeqKey(SessionHandler *sessionHandler);

	//inline void		getLock()
	//{
	//	//EnterCriticalSection(&_CS);
	//	_lock.WriteLock();
	//}
	//inline void		releaseLock()
	//{
	//	//LeaveCriticalSection(&_CS);
	//	_lock.WriteUnlock();
	//}

	inline size_t	getSessionNum()
	{
		return _seqIdMap.size();
	}

private:
	::ZQ::common::FileLog *_log;
	XML_RtspServerHandler &_xml_RtspServerHandler;

	SessionHandlerMap _sessionHandlerMap;
	::std::map<uint16, SessionHandler*> _seqIdMap;
	//CRITICAL_SECTION _CS;
#define USEMUTEX
#ifdef USELOCK
	::ZQ::common::RWLock _lock;
	::ZQ::common::RWLock _idmapLock;
#endif
#ifdef USEMUTEX
	::ZQ::common::Mutex _mutex;
	::ZQ::common::Mutex _idmapMutex;
#endif

	inline void	printSessionNumber()
	{
		//cout << "SessionMap size: " << _sessionHandlerMap.size() << endl;
	}
};

class SessionWatchDog : public ZQ::common::NativeThread
{
public:
	/// constructor
	SessionWatchDog(::ZQ::common::NativeThreadPool &pool, ::ZQ::common::FileLog &fileLog, SessionMap &sessionMap);
	virtual ~SessionWatchDog();

	///@param[in] sessIdent identity of session
	///@param[in] timeout the timeout to wake up timer to check the specified session
	void watchSession(::std::string sessId, uint64 timeout);

	void watchSession(uint16 uCSeq, uint64 timeout);

	void removeSession(::std::string sessId);
	void removeSession(uint16 uCSeq);
	
	//quit watching
	void quit();

protected: // impls of ThreadRequest

	virtual bool init(void);
	virtual int run(void);
	

	void wakeup();

protected:

	typedef ::std::map<::std::string, uint64> ExpirationMap; // sessId to expiration map
	typedef ::std::map<uint16, uint64> ExpirationMap_CSeq;
	ZQ::common::Mutex   _lockExpirations;
	ExpirationMap		_expirations;
	ExpirationMap_CSeq	_expirations_CSeq;
	uint64			_nextWakeup;
	::ZQ::common::FileLog *_log;
	::ZQ::common::NativeThreadPool &_pool;

	bool		  _bQuit;
	HANDLE		  _hWakeupEvent;

	SessionMap	&_sessionMap;
};

#endif // __SessionWatchDog_H__

