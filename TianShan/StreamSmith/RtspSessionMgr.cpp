// RtspSessionMgr.cpp: implementation of the RtspSessionMgr class.
//
//////////////////////////////////////////////////////////////////////

#include <assert.h>

#pragma warning(disable:4786)
// #define _NO_TRACK_MEMORY
#include "global.h"
#include "DataPostHouseService.h"

#include "RtspSessionMgr.h"
#include "RtspSession.h"
#include "RtspDialog.h"
#include "DialogCreator.h"
#include <strHelper.h>
#include <TimeUtil.h>

#include "NativeThreadPool.h"

#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include <ZQDaemon.h>
#endif

using ZQ::common::BaseZQServiceApplication;
extern ZQ::common::BaseZQServiceApplication* Application;

#if defined _RTSP_PROXY		
	#include "rtspProxyConfig.h"
    extern ZQ::StreamSmith::DataPostHouseService* serviceFrm;
#endif

#ifdef _SUPPORT_LSC_PROTOCOL_
	#include "LscDialogImpl.h"
#endif

extern ZQ::StreamSmith::RtspDialogCreatorPtr dialogCreator;

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

//////////////////////////////////////////////////////////////////////////
// SessionClearThread

//不得已啊，要用这种方法的来取得当前的连接数


class statusChecker : public NativeThread
{
public:
	statusChecker( RtspSessionMgr& sessionMgr) 
		:mgr(sessionMgr)
	{
		bQuit = true;
	}
	~statusChecker( )
	{
		stop( );
	}
public:
	void stop( )
	{
		if(!bQuit)
		{
			bQuit = true;
			mSem.post();
			waitHandle(10*1000);	
		}
	}
	bool init(void)
	{		
		bQuit = false;
		return true;
	}
	int run()
	{

#if defined _RTSP_PROXY		

		long sessionCount = 0;
		long listenPort = 0;
		long maxSessionCount = 0;		
		long maxConnCount = dialogCreator->getMaxConnection();

		listenPort = GAPPLICATIONCONFIGURATION.lListenPort;
		int64 waitTime	= GAPPLICATIONCONFIGURATION.statusCheckInterval; 
		maxSessionCount = GAPPLICATIONCONFIGURATION.lMaxSessionCount;
		if (maxSessionCount <= 0)
			maxSessionCount = 500;

		int64 lastLoadInfoTime = 0;
		while(!bQuit)
		{
			{
				ZQ::common::MutexGuard gd(mgr._sessionMapLock);
				sessionCount = (long)mgr._sessionMap.size();
			}
			long connLoad = 0;
			if ( maxConnCount <= 0 )
			{
				connLoad = 100;
			}
			else
			{
				connLoad = dialogCreator->getMainConnCount() * 100 / maxConnCount;
			}
			
			long sessLoad = sessionCount * 100 / maxSessionCount;
			if( sessLoad>100 )
				sessLoad = 100;

			int64 timenow = ZQ::common::now();
			if( ( timenow  - lastLoadInfoTime ) >= waitTime )
			{
				lastLoadInfoTime = timenow;
				glog(ZQ::common::Log::L_NOTICE,CLOGFMT(RtspProxy,"loadInfo() serverIP[%s] serverPort[%d] "
									"session[%d/%d] connection[%d/%d] load[%d/%d]"),
									"0.0.0.0", listenPort ,									
									sessionCount, maxSessionCount ,
									dialogCreator->getMainConnCount(), 
									dialogCreator->getMaxConnection(),
									max(connLoad,sessLoad),100);
			}

			//WaitForSingleObject( hEvent , 30 * 1000 );
			mSem.timedWait(30*1000);

			if( bQuit)
				break;

			_GlobalObject::responseCounters.addCount( std::string("SET_PARAMETER"), "200 OK" );			
		}
#endif
		return 1;
	}
private:
	RtspSessionMgr&			mgr;
	ZQ::common::Semaphore	mSem;
	bool					bQuit;
};

///////////////// class SessionClearThread ////////////////////////////
class SessionClearThread: public NativeThread {
public:
	SessionClearThread(RtspSessionMgr& sessionMgr): 
	  _sessionMgr(sessionMgr)
	{
		_stoped = true;
	}

	bool stop()
	{
		glog(Log::L_DEBUG, "SessionClearThread::stop() called.");
		_quit = true;
		mSem.post();
		waitHandle(10*1000);
		return true;
	}
	
protected:
	virtual bool init()
	{
		_quit = false;
		_stoped = false;		
		glog(Log::L_DEBUG, "SessionClearThread started.");
		return true;
	}

	virtual int run()
	{
		uint32 sessionTimeout = _sessionMgr._sessionTimeout;
		ZQ::common::Mutex& sessionMapLock = _sessionMgr._sessionMapLock;
		RtspSessionMgr::SessionMap& sessionMap = _sessionMgr._sessionMap;
		RtspSessionMgr::SessionMap::iterator itor;
		IClientSessionInternal* session;
		RtspSessionMgr::SessionPair pair;
		
		do 
		{
			//r = WaitForSingleObject(_quitEvent, sessionTimeout);
			mSem.timedWait(sessionTimeout);
			if (_quit||_stoped) 
			{
				DEBUG_DETAIL(DEBUG_DETAIL_LEVEL1)
				{
					glog(Log::L_DEBUG, "SessionClearThread::run()\tQuiting...");
				}
				if (_quit)
					break;
			}
			
			DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) 
			{
				glog(Log::L_DEBUG, "SessionClearThread::run()\t"
					"scanning session map(sessionMap size = %d).", 
					sessionMap.size());
			}
			ZQ::common::MutexGuard guard(sessionMapLock);

			try 
			{
				itor = sessionMap.begin();
				while (itor != sessionMap.end())
				{
					session = itor->second;
					if (session) 
					{
						if (!session->isActive() || session->checkTimeout(sessionTimeout)) 
						{

							DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) 
							{
								glog(Log::L_DEBUG, "SessionClearThread::run()\t"
									"session timeout(session ID = %s).", 
									session->getSessionID());
							}
							
							session->close();
							session->release();
							 /*itor = */sessionMap.erase(itor);
							itor = sessionMap.begin();

						} 
						else 
						{
							itor ++;
						}
					} 
					else 
					{
						 /*itor = */sessionMap.erase(itor);
						itor = sessionMap.begin();
					}
				}
			}
			catch(...) 
			{
				glog(Log::L_DEBUG, "SessionClearThread::run()\t"
					"exception occurred.");
				// assert(false);
			}

//			rwlock_unlock_write(sessionMapLock);
			DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) {
				glog(Log::L_DEBUG, "SessionClearThread::run()\t"
					"scanning finished(sessionMap size = %d).", 
					sessionMap.size());
			}
		}while(!_quit);
		
		return 0;
	}

	virtual void final()
	{
		_stoped = true;		
		glog(Log::L_DEBUG, "SessionClearThread stopped.");
	}

protected:
	RtspSessionMgr&	_sessionMgr;
	bool			_quit;
	bool			_stoped;
	ZQ::common::Semaphore mSem;
};



//////////////////////////////////////////////////////////////////////
// RtspSessionMgr

RtspSessionMgr::RtspSessionMgr()	
{
//	_LastSID = 0;
	_maxSession = NO_LICENSE_SESSIONS_MAX;
	_sessionTimeout = 300000;
//	_clearThread = NULL;
	_enabled = false;
	_statusChecker = NULL;
//	_rtspSnmpAgent = NULL;
}

RtspSessionMgr::~RtspSessionMgr()
{
	//if (NULL != _rtspSnmpAgent)
	//{
	//	ZQ::Snmp::Subagent* tmpAgent = _rtspSnmpAgent;
	//	_rtspSnmpAgent = NULL;
	//	delete tmpAgent;
	//}
}

bool RtspSessionMgr::init()
{
//	_sessionMapLock = create_rwlock();
//	if (_sessionMapLock == NULL)
//		return false;

//	_clearThread = new SessionClearThread(*this);
#ifdef _RTSP_PROXY
	_maxSession = gRtspProxyConfig.lMaxSessionCount;
	_sessionExpireTime = gRtspProxyConfig.licenseTime;
#else
	_maxSession = 500;
	_sessionExpireTime = ZQ::common::TimeUtil::now();
#endif
	_statusChecker = new statusChecker(*this);
	//if (NULL == _rtspSnmpAgent)
	//	_rtspSnmpAgent = new ZQ::Snmp::Subagent(1000, 5, Application->getInstanceId());

	//if (NULL != _rtspSnmpAgent)
	//	_rtspSnmpAgent->start();

	// registerSnmp();

	return true;
}

bool RtspSessionMgr::isEnabled()
{
	return _enabled;
}

bool RtspSessionMgr::enable(bool enabled)
{
	if (_enabled == enabled)
		return false;
	if (enabled) {
		/*
		if (!_clearThread->start())
			return false;
		*/
		_enabled = true;
	} else {
// 		if (!_clearThread->stop())
// 			return false;
		_enabled = false;
	}

	if(_statusChecker)
	{
		_statusChecker->start();
	}

	return true;
}

void RtspSessionMgr::uninit()
{
	if (_enabled)
		enable(false);

	if(_statusChecker)
	{
		_statusChecker->stop();
	}
	delete _statusChecker;
	_statusChecker = NULL;

//	delete _clearThread;

	{
		ZQ::common::MutexGuard gd(_sessionMapLock);
		_sessionMap.clear();
	}

	// 此时已经无其它线程访问 session map
//	if (_sessionMapLock)
//		destroy_rwlock(_sessionMapLock);
}

size_t RtspSessionMgr::getSessionCount() const
{
	ZQ::common::MutexGuard guard(_sessionMapLock);
	return _sessionMap.size();
}

IClientSession* RtspSessionMgr::createSession(const char* sessId, 
											  IClientSession::SessType type, 
											  const char* uri)
{
#ifdef _RTSP_PROXY
	if (ZQ::common::TimeUtil::now() > _sessionExpireTime && _maxSession > NO_LICENSE_SESSIONS_MAX)
	{
		char timeBuffer[64];
		memset(timeBuffer, '\0', sizeof(timeBuffer));
		ZQ::common::TimeUtil::TimeToUTC(gRtspProxyConfig.licenseTime, timeBuffer, sizeof(timeBuffer) - 1, true);
		_maxSession = NO_LICENSE_SESSIONS_MAX;
		glog(Log::L_WARNING, CLOGFMT(RtspSessionMgr, "createSession() license expiration[%s] exceeded, reduced to maxSession[%d]"),timeBuffer, _maxSession);
	}
#endif
	if (getSessionCount() >= _maxSession)
	{
		glog(Log::L_WARNING, CLOGFMT(RtspSessionMgr,"createSession()  Exceed the max session[%d] count"), _maxSession);
		return NULL;
	}
	std::string sessionID;
	if(sessId != NULL)
	{
		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) 
		{
			glog(Log::L_DEBUG, "RtspSessionMgr::createSession()\t"
				"enter. with sessionID = %s", sessId);
		}
		sessionID = sessId;
	}
	else
	{
		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) 
		{
			glog(Log::L_DEBUG, "RtspSessionMgr::createSession()\t"
				"enter. with sessionID = NULL,generate a sessionID");
		}
		generateSessionID(sessionID);
	}
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) {
		glog(Log::L_DEBUG, "RtspSessionMgr::createSession()\t"
			"entering with. sessionID = %s", sessionID.c_str());
	}
		
//	int r;
	RtspSession* session = new RtspSession(this, sessionID.c_str(), type);
	if (session) 
	{

		session->set(RESERVED_ATTR(RtspURI), ZQVariant( std::string(uri!=NULL ? uri : "" )));

		ZQ::common::MutexGuard guard(_sessionMapLock);
//		r = rwlock_lock_write(_sessionMapLock, SESSION_MAP_TIMEOUT);
//		if (!r) 
//		{
//			delete session;
//			session = NULL;
//			glog(Log::L_ERROR, "RtspSessionMgr::createSession()\t"
//				"lock sessionMap failed. sessionID = %s(error code = %x).", 
//				sessionID.c_str(), GetLastError());
//#ifdef _DEBUG
//			RaiseException(0xcf000001,EXCEPTION_NONCONTINUABLE,0,NULL);
//#endif
//			// assert(false);
//			goto leaveLable;
//		}

		pair<SessionMap::iterator,bool> insret;
		try 
		{
			insret = _sessionMap.insert(SessionPair(sessionID, session));
		} catch(...) 
		{
			// assert(false);
			glog(Log::L_ERROR, "RtspSessionMgr::createSession()\t"
				"exception occurred.");
		}

//		r = rwlock_unlock_write(_sessionMapLock);
//		if (!r) 
//		{
//			glog(Log::L_ERROR, "RtspSessionMgr::createSession()\t"
//				"unlock sessionMap failed. sessionID = %s(error code = %x).", 
//				sessionID.c_str(), GetLastError());
//#ifdef _DEBUG
//			RaiseException(0xcf000001,EXCEPTION_NONCONTINUABLE,0,NULL);
//#endif
//			// assert(false);
//		}

		if (insret.second) 
		{
			DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) 
			{
				long nowC = 0;
				static volatile long sessionCount = 0;
				{
					ZQ::common::MutexGuard gd(_genIdCritSec);
					nowC = ++sessionCount;
				} 
				glog(Log::L_DEBUG, "total session count %d",nowC);
				glog(Log::L_DEBUG, "new session id = %s, session count = %d", 
					sessionID.c_str(), _sessionMap.size());
			}			
		}
		else 
		{
			delete session;
			session = NULL;
			glog(Log::L_ERROR, "RtspSessionMgr::createSession()\t"
				"sessionMap.insert() failed(sessionID = %s).", 
				sessionID.c_str());
			goto leaveLable;
		}
			
	}
	else
	{
		glog(Log::L_ERROR, "createSession failed. out of memory.");
	}

	session->reference();

leaveLable:

	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) {
		glog(Log::L_DEBUG, "RtspSessionMgr::createSession()\t"
			"leave. sessionID = %s", sessionID.c_str());
	}

	return session;
}

bool RtspSessionMgr::deleteSession(const char* sessionID, 
								   bool closeConnection)
{
	assert(_enabled);
	// TEARDOWN may try to _delete a session that was NOT created.
	// // assert(session);

	SessionMap::size_type r;
	ZQ::common::MutexGuard guard(_sessionMapLock);
//	uint32 ret = rwlock_lock_write(_sessionMapLock, SESSION_MAP_TIMEOUT);
//	if (!ret) {
//		glog(Log::L_ERROR, "RtspSessionMgr::deleteSession()\t"
//			"lock sessionMap failed. sessionID = %s(error code = %x).", 
//			sessionID, GetLastError());
//		// assert(false);
//		return false;
//	}
	IClientSessionInternal* session =NULL;
	try 
	{
		session = dynamic_cast<IClientSessionInternal* >(_findSession(sessionID));
		if (session == NULL) 
		{
//			rwlock_unlock_write(_sessionMapLock);
			return false;
		}
		//session->sessionDown();
		raiseSessionRemoved(sessionID);
		r = _sessionMap.erase(sessionID);
		glog(ZQ::common::Log::L_INFO,"RtspSessionMgr::deleteSession() session [%s] deleted",sessionID);
	} 
	catch (...) 
	{
		// assert(false);
		glog(Log::L_ERROR, "RtspSessionMgr::deleteSession()\t"
			"exception occurred.");
	}
	
//	ret = rwlock_unlock_write(_sessionMapLock);
//	// assert(r > 0);
//	if (!ret) {
//		glog(Log::L_ERROR, "RtspSessionMgr::deleteSession()\t"
//			"unlock sessionMap failed. sessionID = %s(error code = %x).", 
//			sessionID, GetLastError());
//		// assert(false);
//	}
	// RWLockGuard guard(*dynamic_cast<RtspSession* >(session));
	
	if (closeConnection)
	{
		IConnectionInternalPtr conn = session->getActiveConnection();
		if (conn)
			conn->close();
	}

	session->close();
	session->release();

	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) 
	{
		glog(Log::L_DEBUG, "RtspSessionMgr::deleteSession()\t"
			"deleted sessionID = %s, session count = %d", sessionID, 
			_sessionMap.size());
	}
	
	return  r > 0;
}

IClientSessionInternal* RtspSessionMgr::_findSession(const char* sessionID)
{
	SessionMap::iterator itor;
	IClientSessionInternal* result = NULL;
	if(! ( sessionID&&strlen(sessionID)>0 ) )
	{
		glog(ZQ::common::Log::L_INFO,"RtspSessionMgr::_findSession()\t null sessionID passed in");
		return NULL;
	}
	std::string	strSessionID = sessionID;
	ZQ::common::stringHelper::TrimExtra(strSessionID);
	
	ZQ::common::MutexGuard guard(_sessionMapLock);
	itor = _sessionMap.find(strSessionID);
	if (itor != _sessionMap.end())
	{
		result = itor->second;
	}
	else 
	{
		glog(Log::L_DEBUG, "RtspSessionMgr::_findSession()\t"
			"session(%s) not found.", strSessionID.c_str());
		return NULL;
	}
	
	return result;
}

IClientSession* RtspSessionMgr::findSession(const char* sessionID)
{
	assert(_enabled);
	IClientSessionInternal* result = NULL;
	
	ZQ::common::MutexGuard guard(_sessionMapLock);

//	uint32 r = rwlock_lock_read(_sessionMapLock, SESSION_MAP_TIMEOUT);
//	if (!r)
//	{
//		glog(Log::L_ERROR, "RtspSessionMgr::findSession()\t"
//			"lock sessionMap failed. sessionID = %s(error code = %x).", 
//			sessionID, GetLastError());
//		// assert(false);
//		return NULL;
//	}
	try 
	{
		result = (IClientSessionInternal*)_findSession(sessionID);
		if (result)
		{
			// RWLockGuard guard(*result);
			result->reference();
		}
	} 
	catch(...) 
	{
		// assert(false);
		glog(Log::L_ERROR, "RtspSessionMgr::findSession()\t"
			"exception occurred.");
		return NULL;
	}

//	r = rwlock_unlock_read(_sessionMapLock);
//	if (!r) {
//		glog(Log::L_ERROR, "RtspSessionMgr::findSession()\t"
//			"unlock sessionMap failed. sessionID = %s(error code = %x).", 
//			sessionID, GetLastError());
//		// assert(false);
//	}

	return result;
}

void RtspSessionMgr::generateSessionID(std::string& sessionID)
{
	char buf[256];
	
	time_t timet;
	struct tm* pstm;
	uint32 id = 0;
	static uint32 sLastLWord = 0;
	ZQ::common::MutexGuard gd(_genIdCritSec);
	{
		timet = time(0);
		pstm = gmtime(&timet);
		if(pstm)
			id =(uint32)pstm->tm_hour << 27 | 
				(uint32)pstm->tm_min << 21 | 
				(uint32)pstm->tm_sec << 15;

		id = id | (pstm->tm_mday<<10 )| (sLastLWord ++);
		if(sLastLWord >= ( 1 << 10 ) )
			sLastLWord = 0;
	}

#ifdef _RTSP_PROXY
	if ( GAPPLICATIONCONFIGURATION.lUseLongSessionId >= 1 )
	{
		sprintf(buf, "9%011u",  id);
	}
	else
#endif
		sprintf(buf, "%u",  id);
	
	// // assert(findSession(buf) == NULL);
	sessionID = buf;
}

IServerRequest* RtspSessionMgr::getServerRequest(const char* sessionID,const std::string& ConnectionIDentity)
{
	RtspSession* session = dynamic_cast<RtspSession *>(findSession(sessionID));
	if (session == NULL) 
	{
		glog(Log::L_ERROR, "RtspSessionMgr::getServerRequest()\t "
			"session with id=%s not found",sessionID);
		return NULL;
	}
	IConnectionInternalPtr conn=NULL;
	if( ConnectionIDentity.empty())
	{
		conn = session->getActiveConnection();
	}
	else
	{//here add a reference
		//conn = session->getConnection(ConnectionIDentity);
		if(!conn)
		{//如果没有找到
			//首先在
			/*extern RtspDialogCreator		dialogCreator;*/
			uint64 connID = 0;
			sscanf(ConnectionIDentity.c_str(),FMT64U,&connID);
			//conn = dialogCreator.getConnection(connID);
			conn = getConnection(connID);
			if(conn)
			{
				session->onAccess(conn);
//				conn->assocSession(session);
				conn->updateLastSession(sessionID);		
//				session->release();	
//				conn->release();
			}
		}
	}
	if (conn == NULL) 
	{
		glog(Log::L_ERROR, "RtspSessionMgr::getServerRequest()\t "
			"Can't get connection with ID[%s]",ConnectionIDentity.c_str());
		session->release();
		return NULL;
	}
	else
	{
		glog(ZQ::common::Log::L_DEBUG,"RtspSessionMgr::getServerRequest()\t "
				"get connection with ID[%s] ok and SessionID [%s]",ConnectionIDentity.c_str(),session->getSessionID());
	}

	IServerRequest* pReq = NULL;
#pragma message(__MSGLOC__"Judge the ClientSession ")
	switch( conn->getConnectionProtocol () ) 
	{
#ifdef _SUPPORT_LSC_PROTOCOL_
	case IConnectionInternal::CONNECTION_PROTCOL_TYPE_LSCP:
		{
			pReq = new LscServerRequest(conn,sessionID);			
		}
		break;
#endif//_SUPPORT_LSC_PROTOCOL_
	case IConnectionInternal::CONNECTION_PROTCOL_TYPE_RTSP:
		{
			pReq = new RtspServerRequest(conn,sessionID);			
		}
		break;
	default:
		{
			session->release ();
			conn = NULL;
			glog(ZQ::common::Log::L_DEBUG,CLOGFMT(RtspSessionMgr,"unsurpport Connection Protocol type [%d]"),conn->getConnectionProtocol ());
			return NULL;
		}
		break;
	}	
//	RtspServerRequest* req = new RtspServerRequest(conn,sessionID);
	session->release();
	return pReq;

}
IConnectionInternalPtr RtspSessionMgr::getConnection(uint64 connID)
{
	ZQ::common::MutexGuard gd(_connMapLock);
	ConnMap::const_iterator it = _connMap.find(connID);
	if(it!=_connMap.end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}
void RtspSessionMgr::onconnectionSetup(IConnectionInternalPtr conn)
{
	ZQ::common::MutexGuard gd(_connMapLock);
	_connMap.insert(ConnMap::value_type(conn->getConnectionIdentity(),conn));	
}
void RtspSessionMgr::onConnectDestroyed(IConnectionInternalPtr conn)
{
	std::string sessionID = conn->getLastSessionID();
	if (!sessionID.empty()) 
	{
		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL1) 
		{
			glog(Log::L_DEBUG, 
				"RtspSessionMgr::onConnectionDestroyed()\t sessionID: %s", 
				sessionID.c_str());
		}

		RtspSession* session = dynamic_cast<RtspSession*  >(findSession(sessionID.c_str()));
		if (session) 
		{
			session->onAccess(conn);			
			session->release();
		}
		else 
		{
			DEBUG_DETAIL(DEBUG_DETAIL_LEVEL1)
			{
				glog(Log::L_DEBUG,"RtspSessionMgr::onConnectionDestroyed()\t "
						"sessionID: %s not found",sessionID.c_str());
			}
		}
	
	}
	
	{
		ZQ::common::MutexGuard gd(_connMapLock);
		_connMap.erase(conn->getConnectionIdentity());
		conn = NULL;
	}
}

/*
void RtspSessionMgr::onAccessingSession(const char* sessionID, 
										RtspConnection* conn)
{
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) {
		glog(Log::L_DEBUG, "RtspSessionMgr::onAccessingSession():\t"
			"sessionID = %s, conn = %p", sessionID, conn);
	}

	RtspSession* session = dynamic_cast<RtspSession* >(
		findSession(sessionID));
	if (session) {
		session->onAccess(conn);
		conn->updateLastSession(sessionID);
		session->release();
	}
}
*/

void RtspSessionMgr::onAccessingSession(IClientRequest* req)
{
	IClientRequestWriterInternal* cliReq;
	try 
	{
		cliReq =(IClientRequestWriterInternal*)req;
	}
	catch(...)
	{
		glog(Log::L_ERROR, "RtspSessionMgr::onAccessingSession():\t"
			"convert RtspClientRequest failed");
		return;
	}

	if (cliReq == NULL) 
	{
		glog(Log::L_ERROR, "RtspSessionMgr::onAccessingSession():\t"
			"convert RtspClientRequest failed");
		return;
	}

	IConnectionInternalPtr conn = cliReq->getConnection();
	
	if (conn == NULL) 
	{
		glog(Log::L_ERROR, "RtspSessionMgr::onAccessingSession():\t"
			"active connection is NULL");
		//assert(false);
		return;
	}

	char sessionID[0x200];
	uint16 len = sizeof(sessionID);
	memset(sessionID, 0, len);	
	const char* pSessionID = cliReq->getClientSessionId();
	if(! (pSessionID&&strlen(pSessionID)>0) )
	{
		glog(Log::L_ERROR, "RtspSessionMgr::onAccessingSession():\t"
			"No SessionID is found");
		conn = NULL;
		return ;
	}

	const char* pToken = strstr(pSessionID,";");
	if(pToken)
	{
		strncpy( sessionID, pSessionID, pToken-pSessionID);
	}
	else
	{
		strncpy( sessionID, pSessionID, sizeof(sessionID)-1 );
	}
	len=strlen(sessionID);
	if (len <= 0) 
	{

		glog(Log::L_ERROR, "RtspSessionMgr::onAccessingSession():\t"
			"get session id failed");
		conn = NULL;
		return;
	}

	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) {
		glog(Log::L_DEBUG, "RtspSessionMgr::onAccessingSession():\t"
			"sessionID = %s, conn = %llu", sessionID, conn->getConnectionIdentity());
	}

	RtspSession* session = dynamic_cast<RtspSession* >(findSession(sessionID));

	if (session) 
	{
		session->onAccess(conn);
//		conn->assocSession(session);
		conn->updateLastSession(sessionID);		
		session->release();	
		conn = NULL;
	}
	else 
	{		
		conn = NULL;
		glog(Log::L_INFO, "RtspSessionMgr::onAccessingSession():\t"
			"session not found,it may be destroyed. sessionID = %s", sessionID);
	}
}

bool RtspSessionMgr::sinkSessionEvent(uint32 mask, SessionEvent* event)
{
	if (mask == 0 || event == NULL)
		return false;

	ZQ::common::MutexGuard guard(_eventRecvLock);
	SessionReceivers::iterator itor = _eventReceivers.begin();
	for (; itor != _eventReceivers.end(); itor ++) 
	{

		if (*itor == event) {
			return false;
		}
	}

	_eventReceivers.push_back(event);

	return true;
}

bool RtspSessionMgr::unsinkSessionEvent(SessionEvent* event)
{
	ZQ::common::MutexGuard guard(_eventRecvLock);
	SessionReceivers::iterator itor = _eventReceivers.begin();
	for (; itor != _eventReceivers.end(); itor ++) 
	{

		if (*itor == event)
		{
			_eventReceivers.erase(itor);
			return true;
		}
	}

	return false;
}

void RtspSessionMgr::raiseSessionRemoved(const std::string& sessionId)
{
	ZQ::common::MutexGuard guard(_eventRecvLock/*, RWLockGuard::LOCK_READ*/);
	SessionReceivers::iterator itor = _eventReceivers.begin();
	SessionEvent* event;
	for (; itor != _eventReceivers.end(); itor ++) 
	{

		event = *itor;
		event->onSessionRemoved(sessionId);
	}
}

extern ZQ::common::NativeThreadPool&  _gThreadPool;
/*
int RtspSessionMgr::registerSnmp(void)
{
	using namespace ZQ::Snmp;
	int nRev = false;
	int registerCount = 0;

	if (NULL == _rtspSnmpAgent)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "RtspSessionMgr registerSnmp failed, rtspSnmpAgent[NULL]"));
		return nRev;
	}

	try
	{
		using namespace ZQ::common;
		typedef RtspSessionMgr::SessionMap::size_type  map_size_type;
		typedef DECLARE_SNMP_RO_TYPE(RtspSessionMgr::SessionMap&, map_size_type (RtspSessionMgr::SessionMap::*)() const, int)   rtspSessionCount;
		typedef DECLARE_SNMP_RO_TYPE(NativeThreadPool&, const int (NativeThreadPool::*)(void), int)   rtspPendingSize;
		typedef DECLARE_SNMP_RO_TYPE(NativeThreadPool&, int (NativeThreadPool::*)(void) const, int)   rtspBusyThreads;
		typedef DECLARE_SNMP_RO_TYPE(NativeThreadPool&, int (NativeThreadPool::*)(void) const, int)   rtspThreadPoolSize;

		NativeThreadPool& thpool = _gThreadPool;
		_rtspSnmpAgent->addObject( Oid("1.2"), ManagedPtr(new SimpleObject(VariablePtr(new rtspSessionCount(_sessionMap, &SessionMap::size)),  AsnType_Integer, aReadOnly)));  ++registerCount;
		_rtspSnmpAgent->addObject( Oid("1.3"), ManagedPtr(new SimpleObject(VariablePtr(new rtspPendingSize(thpool, &NativeThreadPool::pendingRequestSize)), AsnType_Integer, aReadOnly)));  ++registerCount;
		_rtspSnmpAgent->addObject( Oid("1.4"), ManagedPtr(new SimpleObject(VariablePtr(new rtspBusyThreads(thpool, &NativeThreadPool::activeCount)), AsnType_Integer, aReadOnly)));  ++registerCount;
		_rtspSnmpAgent->addObject( Oid("1.5"), ManagedPtr(new SimpleObject(VariablePtr(new rtspThreadPoolSize(thpool, &NativeThreadPool::size)),     AsnType_Integer, aReadOnly)));  ++registerCount;

		nRev = true;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(RtspSessionMgr, "initialize RtspSessionMgr registerSnmp succeed registerCount[%d]"), registerCount);
	}
	catch (...)//not  allowed  to failed
	{
		nRev = false;
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(RtspSessionMgr, "initialize weiwoo registerSnmp failed registerCount[%d]"), registerCount);
	}

	return nRev;
}

bool RtspSessionMgr::_eraseSession(const std::string& sessionID)
{
	rwlock_lock_read(_sessionMapLock);
	SessionMap::size_type result = _sessionMap.erase(sessionID);
	rwlock_unlock_read(_sessionMapLock);
	return result < 0;
}
*/

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE
