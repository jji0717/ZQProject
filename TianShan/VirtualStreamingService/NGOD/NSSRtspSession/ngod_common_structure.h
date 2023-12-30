#ifndef _NGOD_COMMON_STRUCTURE_H_
#define _NGOD_COMMON_STRUCTURE_H_

#include "ZQ_common_conf.h"
#include "FileLog.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "ngod_rtsp_parser/RTSPHeader/RTSPHeader.h"

//NSS Session Key, used in NSSSessionMap
typedef struct SessionMapKey
{
	string strOnDemandSessionId;
	string strSessionId;
}SessionMapKey;

//NSS Session Key Map
//support use either OnDemandSessionId(for weiwoo request) or SessionId(NGOD MediaClust Response) to search
//typedef map<SessionMapKey, RTSPClientSession *> NSSSessionMap;
typedef map<string, RTSPClientSession *> NSSSessionMap;

#define NSSSessionMapMaxSize 5

//RTSP message shared buffer
//the message will be combined, we should chop these messages to single one
#define NSSRTSPMessageBufferSize (1024*1024*10)
typedef struct NSSRTSPMessageBuffer
{
	NSSRTSPMessageBuffer():iReadPos(-1),iWritePos(0)
	{
		memset(strBuffer, 0, NSSRTSPMessageBufferSize);
	}
	~NSSRTSPMessageBuffer(){}

	int32	iWritePos;
	int32	iReadPos;
	char	strBuffer[NSSRTSPMessageBufferSize];
	//char	*strBuffer;
}NSSRTSPMessageBuffer;

//single RTSP message shared list, chopped NSSRTSPMessageBuffer
typedef struct NSSRTSPMessageList 
{
	NSSRTSPMessageList()
	{
		InitializeCriticalSection(&m_CS);
		m_MessageList.clear();
	}
	~NSSRTSPMessageList()
	{
		DeleteCriticalSection(&m_CS);
		m_MessageList.clear();
	}

	//strdeque			m_MessageList;
	strlist				m_MessageList;
	CRITICAL_SECTION	m_CS;

	void PushBack(string &str)
	{
		//get lock
		EnterCriticalSection(&m_CS);

		m_MessageList.push_back(str);

		//release lock
		LeaveCriticalSection(&m_CS);
		
	}

	void PopFront()
	{
		//get lock
		EnterCriticalSection(&m_CS);

		if (!m_MessageList.empty())
			m_MessageList.pop_front();

		//release lock
		LeaveCriticalSection(&m_CS);
	}
	
	string First()
	{
		string tmp = "";
		//get lock
		EnterCriticalSection(&m_CS);

		if (!m_MessageList.empty())
			tmp = m_MessageList.front();

		//release lock
		LeaveCriticalSection(&m_CS);
		return tmp;
	}
}NSSRTSPMessageList;

typedef struct SessionSocket
{
	SOCKET	m_Socket;//socket number
	bool	m_Status;//socket status
}SessionSocket;

//add by lxm for CCUR
typedef struct 
{
	SessionSocket			_sessSock;
	std::string				_strServerIp;
	int						_iServerPort;
	NSSRTSPMessageBuffer	_messageBuffer;
}SessionGroupSocket;

//use the _strC1ServerIp:_iC1ServerPort as map key
typedef std::map<std::string, SessionGroupSocket *> SessionGroupSocketMap;

typedef struct 
{
	SessionSocket *_pSessionSocket;
	NSSRTSPMessageBuffer	*_pRTSPMessageBuffer;
	NSSRTSPMessageList		*_pRTSPMessageList;
}SessionSocketBuffer;
typedef std::list<SessionSocketBuffer *> SessionSocketBufferVec;
/////////////////////

typedef enum SessionGroupStatus
{
	Sync = 1,
	Idle = 2
}SessionGroupStatus;

class CSessGroupStatus
{
public:
	CSessGroupStatus()
	{
		InitializeCriticalSection(&m_CS);
	}
	~CSessGroupStatus()
	{
		DeleteCriticalSection(&m_CS);
	}
	CRITICAL_SECTION		m_CS;
	void setStatus(SessionGroupStatus status)
	{
		EnterCriticalSection(&m_CS);
		m_SessionGroupStatus = status;
		LeaveCriticalSection(&m_CS);
	}
	SessionGroupStatus getStatus()
	{
		return m_SessionGroupStatus;
	}
private:
	SessionGroupStatus m_SessionGroupStatus;
};

//NSS SessionGroup
typedef struct NSSSessionGroup
{
	uint16			usClientSeq;
	uint16			usServerSeq;
	string			strSessionGroup;
	string			strServerPath;
	uint16			uServerPort;
	uint16			uMaxSession;
	SessionSocket	m_SessionSocket;
	NSSSessionMap	m_NSSSessionMap;
	NSSSessionMap	m_NSSOnDemandSessionMap;
	CRITICAL_SECTION		m_CS_SessionMap;//guarantee session map shared by multi-thread
	CRITICAL_SECTION		m_CS;//critical section to guarantee many thread share one socket to send
	CRITICAL_SECTION		m_CS_ClientSeq;	//pointer to group client sequence critical section
	CRITICAL_SECTION		m_CS_ServerSeq;	//pointer to group server sequence critical section
	NSSRTSPMessageList		m_NSSRTSPMessageList;
	//NSSRTSPMessageBuffer	m_NSSRTSPMessageBuffer;
	CSessGroupStatus		m_SessionGroupStatus;

	::ZQ::common::URLStr _groupUrlStr;

	//add by lxm for CCUR
	SessionGroupSocketMap	m_SessionGroupSocketMap;//C1 connection map
	::ZQ::common::Mutex		m_SessionGroupSocketMapMutex;
}NSSSessionGroup; 

//use list to support more session group
typedef list<NSSSessionGroup *> NSSSessionGroupList;

class FindBySessionGroup
{
public:
	FindBySessionGroup(string &strSessionGroup):m_strSessionGroup(strSessionGroup){}

	bool operator() (NSSSessionGroup *pNSSSessionGroup)
	{
		if (m_strSessionGroup.compare(pNSSSessionGroup->strSessionGroup) == 0)
			return true;
		else
			return false;
	}
private:
	string m_strSessionGroup;
};

#endif