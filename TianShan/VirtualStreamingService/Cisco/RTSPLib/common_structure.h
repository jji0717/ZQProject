#ifndef _COMMON_STRUCTURE_H_
#define _COMMON_STRUCTURE_H_

#include "ZQ_common_conf.h"
#include "FileLog.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "CVSSRtspParser/CVSSRtspSession.h"

#define MYLOG if (_pLog) (*_pLog)

typedef ::std::list<SOCKET> SOCKETList;
typedef ::std::map<SOCKET, SessionSocket *> SessionSocketMap;
typedef ::std::map<::std::string, SessionSocket *> SessionIdMap;
typedef ::std::map<SOCKET, CVSSRtspSession *> CVSSRtspSessionSocketMap;

class BaseSessionSet
{
public:
	BaseSessionSet(){}
	virtual ~BaseSessionSet(){}
	virtual void writeLock(){_lock.WriteLock();}
	virtual void writeUnlock(){_lock.WriteUnlock();}
	virtual void readLock(){_lock.ReadLock();}
	virtual void readUnlock(){_lock.ReadUnlock();}

	virtual size_t size(){return 0;}
	virtual bool empty(){return true;}

private:
	::ZQ::common::RWLock _lock;
};

//set for socket, used in every thread to record current live socket as an index
class SocketSet : public BaseSessionSet
{
public:
	SOCKETList _socketList;

	virtual size_t size()
	{
		return _socketList.size();
	}

	virtual bool empty()
	{
		return _socketList.empty();
	}

	//if this index already in socket map
	//true: in map; false: not in
	bool inSet(SOCKET &sock)
	{
		//readLock();
		SOCKETList::iterator iter = find(_socketList.begin(), _socketList.end(), sock);
		if (iter == _socketList.end())
		{
			//readUnlock();
			return false;
		}
		else
		{
			//readUnlock();
			return true;;
		}
	}

	void push(SOCKET &sock)
	{
		//writeLock();
		_socketList.push_back(sock);
		//writeUnlock();
	}

	bool remove(SOCKET &sock)
	{
		//writeLock();
		SOCKETList::iterator iter = find(_socketList.begin(), _socketList.end(), sock);
		if (iter == _socketList.end())
		{
			//writeUnlock();
			return false;
		}
		else
		{
			_socketList.erase(iter);
			//writeUnlock();
			return true;
		}
	}
};


//session socket set
class FindBySessionId
{
public:
	FindBySessionId(::std::string &sessId):_sessId(sessId){}

	bool operator() (::std::string &sessId)
	{
		if (_sessId.compare(sessId) == 0)
			return true;
		else
			return false;
	}
private:
	::std::string &_sessId;
};

class FindBySocket
{
public:
	FindBySocket(SOCKET &sock):_sock(sock){}

	bool operator() (SOCKET &sock)
	{
		if (_sock == sock)
			return true;
		else
			return false;
	}
private:
	SOCKET &_sock;
};

//SessionSocket set, use SOCKET number as index, used in recv&chop thread
class SessionSocketSet : public BaseSessionSet
{
public:
	SessionSocketMap _sessionSocketMap;

	size_t size()
	{
		return _sessionSocketMap.size();
	}

	bool empty()
	{
		return  _sessionSocketMap.empty();
	}

	//if this index already in socket map
	//true: in map; false: not in
	SessionSocket* inSet(SOCKET &sock)
	{
		readLock();
		SessionSocketMap::iterator iter = _sessionSocketMap.find(sock);
		if (iter == _sessionSocketMap.end())
		{
			readUnlock();
			return NULL;
		}
		else
		{
			SessionSocket *sessSocket = iter->second;
			readUnlock();
			return sessSocket;;
		}
	}

	void push(SOCKET &sock, SessionSocket *sessSocket)
	{
		writeLock();
		_sessionSocketMap[sock] = sessSocket;
		writeUnlock();
	}

	SessionSocket *remove(SOCKET &sock)
	{
		writeLock();
		SessionSocketMap::iterator iter = _sessionSocketMap.find(sock);
		if (iter == _sessionSocketMap.end())
		{
			writeUnlock();
			//not in map
			return NULL;
		}
		else
		{
			SessionSocket *sessSocket = iter->second;
			_sessionSocketMap.erase(iter);
			writeUnlock();
			return sessSocket;
		}
	}
};

class SessionIdSet  : public BaseSessionSet
{
public:
	SessionIdMap _sessionIdMap;

	virtual size_t size()
	{
		return _sessionIdMap.size();
	}

	virtual bool empty()
	{
		return  _sessionIdMap.empty();
	}

	//if this index already in socket map
	//true: in map; false: not in
	SessionSocket* inSet(::std::string &sessId)
	{
		readLock();
		SessionIdMap::iterator iter =_sessionIdMap.find(sessId);
		if (iter == _sessionIdMap.end())
		{
			readUnlock();
			return NULL;
		}
		else
		{
			SessionSocket *sessSocket = iter->second;
			readUnlock();
			return sessSocket;;
		}
	}

	void push(::std::string &sessId, SessionSocket *sessSocket)
	{
		writeLock();
		_sessionIdMap[sessId] = sessSocket;
		writeUnlock();
	}

	SessionSocket *remove(::std::string &sessId)
	{
		writeLock();
		SessionIdMap::iterator iter = _sessionIdMap.find(sessId);
		if (iter == _sessionIdMap.end())
		{
			writeUnlock();
			//not in map
			return NULL;
		}
		else
		{
			SessionSocket *sessSocket = iter->second;
			_sessionIdMap.erase(iter);
			writeUnlock();
			return sessSocket;
		}
	}
};

//CVSS RTSP Session Set, to record all the live session information, used in daemon&parse thread
class CVSSRtspSessionSet  : public BaseSessionSet
{
public:
	CVSSRtspSessionSocketMap _cvssRtspSessionSocketMap;

	virtual size_t size()
	{
		return _cvssRtspSessionSocketMap.size();
	}

	virtual bool empty()
	{
		return  _cvssRtspSessionSocketMap.empty();
	}

	//if this index already in socket map
	//true: in map; false: not in
	CVSSRtspSession* inSet(SOCKET &sock)
	{
		readLock();
		CVSSRtspSessionSocketMap::iterator iter =_cvssRtspSessionSocketMap.find(sock);
		if (iter == _cvssRtspSessionSocketMap.end())
		{
			readUnlock();
			return NULL;
		}
		else
		{
			CVSSRtspSession *sessSocket = iter->second;
			readUnlock();
			return sessSocket;;
		}
	}

	CVSSRtspSession* inSet(::std::string &sessId)
	{
		readLock();
		for (CVSSRtspSessionSocketMap::iterator iter =_cvssRtspSessionSocketMap.begin(); iter != _cvssRtspSessionSocketMap.end(); iter++)
		{
			if (iter->second->_commonReqHeader._strSessionId == sessId)
			{
				readUnlock();
				return iter->second;
			}
		}
		readUnlock();
		return NULL;
	}

	void push(SOCKET &sock, CVSSRtspSession *sess)
	{
		writeLock();
		_cvssRtspSessionSocketMap[sock] = sess;
		writeUnlock();
	}

	CVSSRtspSession *remove(SOCKET &sock)
	{
		writeLock();
		CVSSRtspSessionSocketMap::iterator iter = _cvssRtspSessionSocketMap.find(sock);
		if (iter == _cvssRtspSessionSocketMap.end())
		{
			writeUnlock();
			//not in map
			return NULL;
		}
		else
		{
			CVSSRtspSession *sessSocket = iter->second;
			_cvssRtspSessionSocketMap.erase(iter);
			writeUnlock();
			return sessSocket;
		}
	}
};

class BaseThread : public ::ZQ::common::NativeThread
{
public:
	BaseThread(::ZQ::common::Log *log):_pLog(log)
	{
		_bLoop = false;
		InitEvent();
	}
	~BaseThread()
	{
		CloseHandle(_handle);
		_pLog = NULL;
	};

	int terminate(int code)
	{
		if (_bLoop == true)
		{
			_bLoop = false;
			::WaitForSingleObject(_handle, INFINITE);
		}
		return 1;
	}

	void InitEvent()
	{
		_handle = ::CreateEvent(NULL, true, false, NULL);
		::ResetEvent(_handle);
	}


	inline void writeLock(){_lock.WriteLock();}
	inline void writeUnlock(){_lock.WriteUnlock();}
	inline void readLock(){_lock.ReadLock();}
	inline void readUnlock(){_lock.ReadUnlock();}

protected:
	::ZQ::common::Log *_pLog;
	HANDLE	_handle;
	bool	_bLoop;

private:
	::ZQ::common::RWLock	_lock;
	
};

#endif