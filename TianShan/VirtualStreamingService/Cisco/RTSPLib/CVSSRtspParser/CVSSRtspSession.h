#ifndef __CVSSRTSPSESSION_H__ 
#define __CVSSRTSPSESSION_H__

#include "RTSPHeader/ResponseCode.h"
#include "RTSPHeader/RequestState.h"
#include "RTSPHeader/CommonHeader.h"
#include "RTSPHeader/DescribeHeader.h"
#include "RTSPHeader/SetupHeader.h"
#include "RTSPHeader/TeardownHeader.h"
#include "RTSPHeader/PlayHeader.h"
#include "RTSPHeader/PauseHeader.h"
#include "RTSPHeader/GetParameterHeader.h"
#include "RTSPHeader/AnnounceHeader.h"

#include "ClientSocket.h"

#define USEMUTEX

//use RTSP sequence as index to get signal
typedef struct RtspCSeqSignal
{
	RtspCSeqSignal()
	{
	}
	~RtspCSeqSignal()
	{
		::ZQ::common::AutoWriteLock lock(_lock);
		for (::std::map<uint32, HANDLE>::iterator iter = _pHandle.begin(); iter != _pHandle.end(); iter++)
		{
			bool b = CloseHandle(iter->second);
		}
		_pHandle.clear();
	}
	bool m_Init(uint32 CSeq)
	{
		::ZQ::common::AutoWriteLock lock(_lock);
		if (_pHandle.find(CSeq) != _pHandle.end())
			return false;

		_pHandle[CSeq] = CreateEvent(NULL, true, false, NULL);
		return true;
	}
	bool m_SetEvent(uint32 CSeq)
	{
		::ZQ::common::AutoWriteLock lock(_lock);

		if (_pHandle.find(CSeq) == _pHandle.end())
			return false;

		return SetEvent(_pHandle[CSeq]);
	}
	bool m_ResetEvent(uint32 CSeq)
	{
		::ZQ::common::AutoWriteLock lock(_lock);

		if (_pHandle.find(CSeq) == _pHandle.end())
			return false;

		return ResetEvent(_pHandle[CSeq]);
	}
	bool m_CloseEvent(uint32 CSeq)
	{
		::ZQ::common::AutoWriteLock lock(_lock);

		::std::map<uint32, HANDLE>::iterator iter = _pHandle.find(CSeq);
		if (iter == _pHandle.end())
			return false;

		bool b = CloseHandle(iter->second);
		_pHandle.erase(iter);
		return b;
	}

	::ZQ::common::RWLock _lock;
	::std::map<uint32, HANDLE>	_pHandle;
}RtspCSeqSignal;

//RTSP message shared buffer
//the message will be combined, we should chop these messages to single one
typedef struct SmartBuffer
{
	SmartBuffer():_iReadPos(-1),_iWritePos(0),_pBuffer(NULL)
	{
	}
	~SmartBuffer(){}

	int32	_iWritePos;
	int32	_iReadPos;
	int32	_iBufferMaxSize;
	char*	_pBuffer;
}SmartBuffer;

typedef struct SessionSocket
{
	SessionSocket()
	{
		_socket = CreateSocket(TCPSOCKET);
		_smartBuffer._iReadPos = -1;
		_smartBuffer._iWritePos = 0;
		_inRecv = false;
		_inChop = false;
	}
	~SessionSocket()
	{
		if (_socket >= 0)
			closesocket(_socket);

		if (_smartBuffer._pBuffer)
		{
			delete _smartBuffer._pBuffer;
			_smartBuffer._pBuffer = NULL;
		}
	}
	SOCKET _socket;
	bool	_status;			//socket status
	SmartBuffer	_smartBuffer;	//this socket receive buffer, should be initialized
#ifdef USERWLOCK
	::ZQ::common::RWLock _lock;
#endif
#ifdef USEMUTEX
	::ZQ::common::Mutex _mutex;
#endif
	bool	_inRecv;			//if this socket already in recv thread request
	bool	_inChop;			//if this socket already in chop thread request
	bool getStatus()
	{
#ifdef USERWLOCK
		::ZQ::common::AutoReadLock rlock(_lock);
#endif
#ifdef USEMUTEX
		::ZQ::common::MutexGuard guard(_mutex);
#endif
		return _status;
	}
	void setStatus(bool status)
	{
#ifdef USERWLOCK
		::ZQ::common::AutoWriteLock wlock(_lock);
#endif
#ifdef USEMUTEX
		::ZQ::common::MutexGuard guard(_mutex);
#endif
		_status = status;
	}

	bool getRecvStatus()
	{
#ifdef USERWLOCK
		::ZQ::common::AutoReadLock rlock(_lock);
#endif
#ifdef USEMUTEX
		::ZQ::common::MutexGuard guard(_mutex);
#endif
		return _inRecv;
	}
	void setRecvStatus(bool status)
	{
#ifdef USERWLOCK
		::ZQ::common::AutoWriteLock wlock(_lock);
#endif
#ifdef USEMUTEX
		::ZQ::common::MutexGuard guard(_mutex);
#endif
		_inRecv = status;
	}

	bool getChopStatus()
	{
#ifdef USERWLOCK
		::ZQ::common::AutoReadLock rlock(_lock);
#endif
#ifdef USEMUTEX
		::ZQ::common::MutexGuard guard(_mutex);
#endif
		return _inChop;
	}
	void setChopStatus(bool status)
	{
#ifdef USERWLOCK
		::ZQ::common::AutoWriteLock wlock(_lock);
#endif
#ifdef USEMUTEX
		::ZQ::common::MutexGuard guard(_mutex);
#endif
		_inChop = status;
	}

	inline void writeLock()
	{
#ifdef USERWLOCK
		_lock.WriteLock();
#endif
#ifdef USEMUTEX
		_mutex.enter();
#endif
	}
	inline void writeUnlock()
	{
#ifdef USERWLOCK
		_lock.WriteUnlock();
#endif
#ifdef USEMUTEX
		_mutex.leave();
#endif
	}
	inline void readLock()
	{
#ifdef USERWLOCK
		_lock.ReadLock();
#endif
#ifdef USEMUTEX
		_mutex.enter();
#endif
	}
	inline void readUnlock()
	{
#ifdef USERWLOCK
		_lock.ReadUnlock();
#endif
#ifdef USEMUTEX
		_mutex.leave();
#endif
	}
}SessionSocket;

typedef struct CVSSRtspSession
{
	DWORD	_lastSendTime;//for test
	::std::string			_strStreamName;			//CVSS session stream proxy string
	HANDLE					_evenHandle;			//event handler, to inform the RTSP operation success
#ifdef USERWLOCK
	::ZQ::common::RWLock _lock;						//self lock for multi-thread share
#endif
#ifdef USEMUTEX
	::ZQ::common::Mutex _mutex;						//self lock for multi-thread share
#endif					
	SessionSocket			_rtspSocket;			//the TCP connect socket to RTSP server
	//SOCKET		_rtspSocket;					//the TCP connect socket to RTSP server

	RTSPSessionState		iRTSPSessionState;		//the RTSP session status(200 OK, 404 Not Found...)
	RTSPClientState			iRTSPClientState;		//the RTSP method type(SETUP/PLAY)

	//RTSP header structure
	CommonReqHeader			_commonReqHeader;		//the field will be use in all the RTSP message
	CommonResHeader			_commonResHeader;		//the field should be parsed in all the RTSP message

	DescribeResHeader		_describeResHeader;		//the field should be parsed in DESCRIBE response
	
	SetupReqHeader			_setupReqHeader;		//the field should be included in SETUP request
	SetupResHeader			_setupResHeader;		//the field should be parsed in SETUP response

	GetParameterReqHeader	_getParameterReqHeader;	//the field should be included in GET_PARAMETER request
	GetParameterResHeader	_getParameterResHeader;	//the field should be parsed in GET_PARAMETER response

	AnnounceReqHeader		_announceReqHeader;		//the field should be included in ANNOUNCE request
	AnnounceResHeader		_announceResHeader;	//the field should be parsed in ANNOUNCE response

	int32	_lastHeartBeatTime;		//the latest heartbeat time
	strlist	_msgList;				//current RTSP message list(should be process)
	
}CVSSRtspSession;

//define session map
typedef ::std::map<SOCKET, CVSSRtspSession *> CVSSRtspSessionSocketMap;
typedef ::std::map<::std::string, CVSSRtspSession *> CVSSRtspSessionIdMap;

#endif