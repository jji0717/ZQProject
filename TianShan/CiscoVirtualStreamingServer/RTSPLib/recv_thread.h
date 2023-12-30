#ifndef __RECV_THREAD_H__
#define __RECV_THREAD_H__

#include "common_structure.h"
//#include "chop_thread.h"
#include "RTSPThreadRequest.h"

class recv_thread : public BaseThread
{
public:
	recv_thread(::ZQ::common::FileLog *logfile,
				SessionSocketSet &sessionSocketSet,
				CVSSRtspSessionSet *rtspSessMap,
				::ZQ::common::NativeThreadPool &recvPool,
				::ZQ::common::NativeThreadPool &sendPool,
				RtspCSeqSignal &rtspCSeqSignal,
				::Ice::CommunicatorPtr communicator,
				ZQADAPTER_DECLTYPE adapter);
	~recv_thread();

	int	run(void);
	bool init(void);

	void stopThrd()
	{
		//_pChopThrd.stopThrd();
		BaseThread::terminate(0);
		_socketSet._socketList.clear();
	}

	bool addSessionSocket(SOCKET &sock);
	bool removeSessionSocket(SOCKET &sock);
	
private:
	//self socket list to iteration all sessions
	SocketSet			_socketSet;

	//SessionSocket class set, get from daemon thread, shared with chop thread
	SessionSocketSet	&_sessionSocketSet;

	//chop_thread _pChopThrd;

	CVSSRtspSessionSet *_pCVSSRtspSessinoSet;
	RtspCSeqSignal &_rtspCSeqSignal;

	::ZQ::common::NativeThreadPool &_recvPool;

	::Ice::CommunicatorPtr	_communicator;
	ZQADAPTER_DECLTYPE		_adapter;
};


class recvThrdRequest : public ::ZQ::common::ThreadRequest
{
public:
	recvThrdRequest(::ZQ::common::FileLog *logFile, ::ZQ::common::NativeThreadPool &pool, SessionSocket *sessSock);
	~recvThrdRequest();

	int run(void);
	void final(int retcode, bool bCancelled)
	{
		delete this;
	}
private:
	::ZQ::common::Log *_pLog;
	SessionSocket *pSessSocket;
	char hint[128];
};

#endif __RECV_THREAD_H__