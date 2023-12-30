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

	// 创建一个 session 对象
	// 参数:
	//		sessId, 根据指定的字符串创建session
	//		type	session 的类型, 参看 IClientSession 类型定义
	//		uri		与该session相关联的URI
	IClientSession* createSession(const char* sessId,IClientSession::SessType type,const char* uri);

	// 删除 指定的 session 对象
	// 参数:
	//		sessionID		要删除的 session 的 ID
	//		closeConnection	是否同时删除其相关的 connection
	bool deleteSession(const char* sessionID, bool closeConnection = false);

	// 通过 id 查找一个 session 对象
	// 参数:
	//		sessionID		要查找的 session 的 ID
	IClientSession* findSession(const char* sessionID);

	// 得到与 session 相关的 server request 对象. 主要用于
	// 服务器向客户发送请求.
	// 参数:
	//		sessionID		要返回 server request 对象 的session 的 ID
	//		ConnectionIDentity connection idenetity which identify a connection associated with the session specified by sessionID
	IServerRequest* getServerRequest(const char* sessionID,const std::string& ConnectionIDentity);

	// 当一个连接关闭时, 由外部通知 sessionMgr
	// 参数:
	//		conn			已关闭的连接
	void onConnectDestroyed(IConnectionInternalPtr conn);

	
	//当一个连接被建立的时候
	void onconnectionSetup(IConnectionInternalPtr conn);
	/*
	// 当一个连接访问一个 session 时, 通知 sessionMgr
	// 参数:
	//		sessionID		被访问 session 的 ID
	//		conn			访问该 session 的 connection
	void onAccessingSession(const char* sessionID, RtspConnection* conn);
	*/
	void onAccessingSession(IClientRequest* req);
	

	// 设置 sessionTimeout 值. 该值用于检查 session 是否超时
	// 参数:
	//		timeo			timeout 值
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
	// 初始化
	bool					init();

	// 反初始化
	void					uninit();

	// 生成 session ID
	void					generateSessionID(std::string& sessionID);

	// 内部查找 session 函数
	IClientSessionInternal* _findSession(const char* sessionID);
	// bool _eraseSession(const std::string& );

	// 开始 dead session 的清理
	bool					startClearThread();
	// 停止清理线程
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

