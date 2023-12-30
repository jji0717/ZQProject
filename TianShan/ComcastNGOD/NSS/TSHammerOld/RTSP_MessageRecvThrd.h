#ifndef __RTSP_MESSAGERECVTHRD_H__
#define __RTSP_MESSAGERECVTHRD_H__

#include "RTSP_common_structure.h"
#include "RTSP_MessageChopThrd.h"
#include "ClientSocket.h"

class RTSP_MessageRecvThrd : public ZQ::common::NativeThread
{
public:
	RTSP_MessageRecvThrd(::ZQ::common::FileLog &fileLog/*, RTSPMessageBuffer &rtspMessageBuffer*/, SessionMap &sessionMap);
	~RTSP_MessageRecvThrd();

	int		run(void);

	void	startThrd();

	bool	addSocket(SessionSocket *sessionSocket);
	::ZQ::common::Mutex		_mutex;

private:
	::ZQ::common::FileLog	*_fileLog;

	SessionSocketList		_socketList;
	//CRITICAL_SECTION		_socketListLock;
	//::ZQ::common::RWLock	_socketListLock;
	
	inline void lock()
	{
		//EnterCriticalSection(&_socketListLock);
		//_socketListLock.WriteLock();
		//_mutex.enter();
	}
	inline void unlock()
	{
		//LeaveCriticalSection(&_socketListLock);
		//_socketListLock.WriteUnlock();
		//_mutex.leave();
	}

	//RTSPMessageBuffer		&_rtspMessageBuffer;
	RTSPMessageBuffer		_rtspMessageBuffer;

	RTSP_MessageChopThrd	*_rtspChopThrd;
	//RTSP_MessageParseThrd	*_rtspParseThrd;
};

#define RECVTHRDCONTINUE {/*_rtspMessageBuffer.readUnLock();*/continue;}

#endif __RTSP_MESSAGERECVTHRD_H__