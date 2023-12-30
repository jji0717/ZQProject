#ifndef __RTSP_COMMON_STRUCTURE_H__
#define __RTSP_COMMON_STRUCTURE_H__

#include "ZQ_common_conf.h"
#include "FileLog.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "ClientSocket.h"
#include <list>
#include <deque>
#include <iostream>
#include <map>

typedef ::std::list<::std::string> strlist;
typedef ::std::deque<::std::string> strdeque;
//RTSP message shared buffer
//the message will be combined, we should chop these messages to single one
//#define RTSPMessageBufferSize (16*1024)
static int32 RTSPMessageBufferSize = 16*1024;//default size

typedef struct MessageBufferIterater
{
	int32	iWritePos;
	int32	iReadPos;
	//char	strBuffer[RTSPMessageBufferSize];
	char	*strBuffer;
	//::std::vector<char> strBuffer;
}MessageBufferIterater;

typedef ::std::map<SOCKET, MessageBufferIterater *> MessageBufferMap;

typedef struct RTSPMessageBuffer
{	
	RTSPMessageBuffer()
	{
		//InitializeCriticalSection(&_CS);
	}
	~RTSPMessageBuffer()
	{
		::ZQ::common::MutexGuard guard(_mutex);
		//writeLock();
		for(MessageBufferMap::iterator iter = _messageBufferMap.begin(); iter != _messageBufferMap.end(); iter++)
		{
			if ((*iter).second->strBuffer != NULL)
			{
				delete (*iter).second->strBuffer;
				(*iter).second->strBuffer = NULL;
			}
			delete (*iter).second;
			(*iter).second = NULL;
		}
		//writeUnLock();
		//DeleteCriticalSection(&_CS);
	}

	inline size_t size()
	{
		::ZQ::common::MutexGuard guard(_mutex);
		//readLock();
		size_t size = _messageBufferMap.size();
		//readUnLock();
		return size;
	}

	MessageBufferMap _messageBufferMap;

	//inline void readLock()
	//{
	//	_lock.ReadLock();
	//}
	//inline void readUnLock()
	//{
	//	_lock.ReadUnlock();
	//}
	//inline void writeLock()
	//{
	//	_lock.WriteLock();
	//}
	//inline void writeUnLock()
	//{
	//	_lock.WriteUnlock();
	//}
	//CRITICAL_SECTION	m_CS;
	::ZQ::common::Mutex	_mutex;
	//::ZQ::common::RWLock	_lock;

	bool add(SOCKET &sockIdx, MessageBufferIterater *messageBuffer)
	{
		if (NULL == messageBuffer)
			return false;

		::ZQ::common::MutexGuard guard(_mutex);
		//writeLock();
		MessageBufferMap::iterator iter = _messageBufferMap.find(sockIdx);
		if (iter != _messageBufferMap.end())
		{
			//writeUnLock();
			return false;
		}

		messageBuffer->iReadPos = -1;
		messageBuffer->iWritePos = 0;
		//memset(messageBuffer->strBuffer, 0, RTSPMessageBufferSize);
		//_messageBufferMap[sockIdx] = messageBuffer;
		_messageBufferMap.insert(MessageBufferMap::value_type(sockIdx,  messageBuffer));
		//writeUnLock();
		return true;
	}

	MessageBufferIterater *getMessageBuffer(SOCKET &sockIdx)
	{
		::ZQ::common::MutexGuard guard(_mutex);
		//readLock();
		MessageBufferMap::iterator iter = _messageBufferMap.find(sockIdx);		
		if (iter == _messageBufferMap.end())
		{
			//readUnLock();
			return NULL;
		}

		MessageBufferIterater *ret = (*iter).second;
		//readUnLock();
		return ret;
	}
	
}RTSPMessageBuffer;

//single RTSP message shared list, chopped NSSRTSPMessageBuffer
typedef struct MessageNode
{
	void			*idx;
	::std::string	strMsg;
}MessageNode;
//typedef ::std::map<void *, ::std::string> MessageMap;
typedef ::std::list<MessageNode *> MessageMap;

class FindMessageByIdx
{
public:
	FindMessageByIdx(void *idx):_idx(idx){}
	bool operator() (MessageNode *messageNode)
	{
		if (_idx == messageNode->idx)
			return true;
		else
			return false;
	}
private:
	void *_idx;
};
typedef struct RTSPMessageList 
{
	RTSPMessageList()
	{
		//InitializeCriticalSection(&m_CS);
		m_MessageList.clear();
	}
	~RTSPMessageList()
	{
		//DeleteCriticalSection(&m_CS);
		m_MessageList.clear();
	}

	//strdeque			m_MessageList;
	//strlist			m_MessageList;
	MessageMap			m_MessageList;
	//inline void readLock()
	//{
	//	_lock.ReadLock();
	//}
	//inline void readUnLock()
	//{
	//	_lock.ReadUnlock();
	//}
	//inline void writeLock()
	//{
	//	_lock.WriteLock();
	//}
	//inline void writeUnLock()
	//{
	//	_lock.WriteUnlock();
	//}
	//CRITICAL_SECTION	m_CS;
	::ZQ::common::Mutex	_mutex;

	void PushBack(void *idx, ::std::string &str)
	{
		//get lock
		//EnterCriticalSection(&m_CS);
		::ZQ::common::MutexGuard guard(_mutex);
		if (idx == NULL)
		{
			//m_MessageList.push_back(str);
			MessageNode *node = new MessageNode();
			node->idx = idx;
			node->strMsg = str;
			m_MessageList.push_back(node);
		}
		else
		{
			//MessageMap::iterator iter = m_MessageList.find(idx);
			MessageMap::iterator iter = find_if(m_MessageList.begin(), m_MessageList.end(), FindMessageByIdx(idx));
			if (iter == m_MessageList.end())
			{
				//m_MessageList.push_back(str);
				MessageNode *node = new MessageNode();
				node->idx = idx;
				node->strMsg = str;
				//m_MessageList[idx] = str;
				m_MessageList.push_back(node);
			}
		}
	}

	void PopFront()
	{
		//get lock
		::ZQ::common::MutexGuard guard(_mutex);

		//if (!m_MessageList.empty())
		m_MessageList.pop_front();
	}
	
	void First(void **idx, ::std::string &msg)
	{
		//string tmp = "";
		MessageMap::iterator iter;

		//get lock
		::ZQ::common::MutexGuard guard(_mutex);

		if (!m_MessageList.empty())
			iter = m_MessageList.begin();
		
		*idx = (*iter)->idx;
		msg = (*iter)->strMsg;
	}
}RTSPMessageList;

typedef struct SessionSocket
{
	SessionSocket()
	{
		//InitializeCriticalSection(&m_CS);
		m_Socket = 0;
	}
	~SessionSocket()
	{
		//DeleteCriticalSection(&m_CS);
		if (m_Socket > 0)
			CloseSocket(m_Socket);
	}

	//inline void readLock()
	//{
	//	_lock.ReadLock();
	//}
	//inline void readUnLock()
	//{
	//	_lock.ReadUnlock();
	//}
	//inline void writeLock()
	//{
	//	_lock.WriteLock();
	//}
	//inline void writeUnLock()
	//{
	//	_lock.WriteUnlock();
	//}
	//CRITICAL_SECTION	m_CS;
	::ZQ::common::Mutex	_mutex;
	SOCKET	m_Socket;//socket number
	bool	m_Status;//socket status
	DWORD	_postMessageTime;
}SessionSocket;

typedef ::std::list<SessionSocket *> SessionSocketList;

class FindBySocket
{
public:
	FindBySocket(SOCKET &sock):_sock(sock){}
	bool operator() (SessionSocket *sessSocket)
	{
		if (sessSocket->m_Socket == _sock)
			return true;
		else
			return false;
	}
private:
	SOCKET _sock;
};

#define SleepContinue {Sleep(1);continue;}

#define MYLOG if (_fileLog) (*_fileLog)

#endif __RTSP_COMMON_STRUCTURE_H__