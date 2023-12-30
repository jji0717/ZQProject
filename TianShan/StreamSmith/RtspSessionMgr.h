// RtspSessionMgr.h: interface for the RtspSessionMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RTSPSESSIONMGR_H__61E51E85_AA1B_432A_8194_A6E672F0D9DB__INCLUDED_)
#define AFX_RTSPSESSIONMGR_H__61E51E85_AA1B_432A_8194_A6E672F0D9DB__INCLUDED_

#define NO_LICENSE_SESSIONS_MAX (500)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StreamSmithModule.h"
#include "proxydefinition.h"

//#include "rwlock.h"
#include <map>
#include <string>

#include <Locks.h>

// #include "snmp/SubAgent.hpp"

//////////////////////////////////////////////////////////////////////////

#define	MAX_SESSIONID			60
#define SESSION_MAP_TIMEOUT		5000

//////////////////////////////////////////////////////////////////////////

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

class RtspSession;
class RtspConnection;
class SessionClearThread;
class statusChecker;

class SessionEvent {
public:
	enum 
	{
		SessionRemoved = 1, 
	};
	virtual ~SessionEvent(){}

	virtual void onSessionRemoved(const std::string& sessionId) = 0;
};

class RtspSessionMgr
{
	friend class _GlobalObject;
	friend class RtspSession;
	friend class SessionClearThread;	
	friend class statusChecker;
public:
	virtual ~RtspSessionMgr();
	RtspSessionMgr();
	
	bool enable(bool enabled);
	bool isEnabled();

	// ����һ�� session ����
	// ����:
	//		sessId, ����ָ�����ַ�������session
	//		type	session ������, �ο� IClientSession ���Ͷ���
	//		uri		���session�������URI
	IClientSession* createSession(const char* sessId,IClientSession::SessType type,const char* uri);

	// ɾ�� ָ���� session ����
	// ����:
	//		sessionID		Ҫɾ���� session �� ID
	//		closeConnection	�Ƿ�ͬʱɾ������ص� connection
	bool deleteSession(const char* sessionID, bool closeConnection = false);

	// ͨ�� id ����һ�� session ����
	// ����:
	//		sessionID		Ҫ���ҵ� session �� ID
	IClientSession* findSession(const char* sessionID);

	// �õ��� session ��ص� server request ����. ��Ҫ����
	// ��������ͻ���������.
	// ����:
	//		sessionID		Ҫ���� server request ���� ��session �� ID
	//		ConnectionIDentity connection idenetity which identify a connection associated with the session specified by sessionID
	IServerRequest* getServerRequest(const char* sessionID,const std::string& ConnectionIDentity);

	// ��һ�����ӹر�ʱ, ���ⲿ֪ͨ sessionMgr
	// ����:
	//		conn			�ѹرյ�����
	void onConnectDestroyed(IConnectionInternalPtr conn);

	
	//��һ�����ӱ�������ʱ��
	void onconnectionSetup(IConnectionInternalPtr conn);
	/*
	// ��һ�����ӷ���һ�� session ʱ, ֪ͨ sessionMgr
	// ����:
	//		sessionID		������ session �� ID
	//		conn			���ʸ� session �� connection
	void onAccessingSession(const char* sessionID, RtspConnection* conn);
	*/
	void onAccessingSession(IClientRequest* req);
	

	// ���� sessionTimeout ֵ. ��ֵ���ڼ�� session �Ƿ�ʱ
	// ����:
	//		timeo			timeout ֵ
	bool setSessionTimeout(uint32 timeo)
	{
		_sessionTimeout = timeo;
		return true;
	}
	void setMaxSession(uint32 maxSession)
	{
		_maxSession = maxSession;
	}
	uint32 getMaxSession()
	{
		return _maxSession;
	}
	void setSessionExpireTime(int64 time)
	{
		_sessionExpireTime = time;
	}
	int64 getSessionExpireTime()
	{
		return _sessionExpireTime;
	}
	bool					sinkSessionEvent(uint32 mask, SessionEvent* event);
	bool					unsinkSessionEvent(SessionEvent* event);

	IConnectionInternalPtr	getConnection(uint64 connID);

	size_t					getSessionCount() const;
	// int                     registerSnmp(void);

protected:
	// ��ʼ��
	bool					init();

	// ����ʼ��
	void					uninit();

	// ���� session ID
	void					generateSessionID(std::string& sessionID);

	// �ڲ����� session ����
	IClientSessionInternal* _findSession(const char* sessionID);
	// bool _eraseSession(const std::string& );

	// ��ʼ dead session ������
	bool					startClearThread();
	// ֹͣ�����߳�
	bool					stopClearThread();

	void					raiseSessionRemoved(const std::string& sessionId);

public:
	typedef std::map<std::string, IClientSessionInternal* > SessionMap;

protected:
	typedef std::map<uint64, IConnectionInternalPtr> ConnMap;
	ConnMap								_connMap;
	ZQ::common::Mutex					_connMapLock;

	typedef std::pair<std::string, IClientSessionInternal* > SessionPair;	

	ZQ::common::Mutex					_sessionMapLock;
	ZQ::common::Mutex					_genIdCritSec;
	SessionMap							_sessionMap;

	uint32									_maxSession;
	int64									_sessionExpireTime;
//	SessionClearThread*					_clearThread;
	uint32								_sessionTimeout;

	bool								_enabled;

	typedef std::vector<SessionEvent* > SessionReceivers;
	SessionReceivers					_eventReceivers;
	ZQ::common::Mutex					_eventRecvLock;

	statusChecker*						_statusChecker;
//	ZQ::Snmp::Subagent*                 _rtspSnmpAgent;
};

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE

#endif // !defined(AFX_RTSPSESSIONMGR_H__61E51E85_AA1B_432A_8194_A6E672F0D9DB__INCLUDED_)

