#ifndef __RTSP_MESSAGECHOPTHRD_H__
#define __RTSP_MESSAGECHOPTHRD_H__

#include "RTSP_common_structure.h"
#include "RTSP_MessageParseThrd.h"

typedef ::std::list<SOCKET> SOCKETList;
class RTSP_MessageChopThrd : public ZQ::common::NativeThread
{
public:
	RTSP_MessageChopThrd(::ZQ::common::FileLog &fileLog, RTSPMessageBuffer *rtspMessageBuffer/*, RTSPMessageList &rtspMessageList*/, SessionMap &sessionMap);
	~RTSP_MessageChopThrd();

	int		run(void);

	void	startThrd();

	void addSocket(SOCKET &sock)
	{
		::ZQ::common::MutexGuard guard(_mutex);
		_socketList.push_back(sock);
	}

private:
	::ZQ::common::FileLog	*_fileLog;
	RTSPMessageList			_rtspMessageList;
	RTSPMessageBuffer		*_rtspMessageBuffer;
	RTSP_MessageParseThrd	*_rtspParseThrd;
	SessionMap				&_sessionMap;
	SOCKETList				_socketList;
	::ZQ::common::Mutex		_mutex;
};

#endif __RTSP_MESSAGECHOPTHRD_H__