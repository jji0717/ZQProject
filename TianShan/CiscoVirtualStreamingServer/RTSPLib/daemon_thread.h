#ifndef __DAEMON_THREAD_H__
#define __DAEMON_THREAD_H__

#include "common_structure.h"
#include "CVSSRtspParser/ClientSocket.h"
#include "recv_thread.h"
#include "../CVSSEventSinkI.h"

//debug define
//#define DAEMONTHRDDEBUG
//#define THRDLOG (*_pLogFile)

class daemon_thread : public BaseThread
{
public:
	daemon_thread(::ZQ::common::FileLog *logfile,
				  ::ZQ::common::NativeThreadPool &recvPool,
				  ::ZQ::common::NativeThreadPool &sendPool,
				  RtspCSeqSignal &rtspCSeqSignal,
				  ::ZQTianShan::CVSS::CVSSEventList &eventList,
				  ::Ice::CommunicatorPtr communicator,
				  ZQADAPTER_DECLTYPE adapter);
	~daemon_thread();

	bool	init(void);

	int		run(void);
	
	void stopThrd()
	{
		_recvThrd.stopThrd();
		BaseThread::terminate(0);
		_socketSet._socketList.clear();
	}

	//member function for manage CVSSSessionMap
	//return false if reach the max concurrent number
	bool	addRTSPSession(CVSSRtspSession *sess, SOCKET &sock);
	bool	addRTSPSession(CVSSRtspSession *sess, ::std::string &strSessionId);

	//return false if could not find sess in any session set
	bool	removeRTSPSession(SOCKET &sock);
	bool	removeRTSPSession(::std::string &strSessionId);

	CVSSRtspSession *findRTSPSession(SOCKET &sock);
	CVSSRtspSession *findRTSPSession(::std::string &strSessionId);

	::ZQTianShan::CVSS::CVSSEventList &_eventList;

	::Ice::CommunicatorPtr	_communicator;
	ZQADAPTER_DECLTYPE		_adapter;

private:
	void	initRTSPClientSession(CVSSRtspSession &sess);
	void	doHeartBeat(CVSSRtspSession &sess);

	SocketSet				_socketSet;			//self socket list to iteration all sessions
	//::ZQ::common::RWLock	_lock;				//SocketSet lock
	::ZQ::common::Mutex		_mutex;				//SocketSet lock

	SessionSocketSet	_sessionSocketSet;		//SessionSocket class set, shared with recv thread
	CVSSRtspSessionSet	_cvssRtspSessionSet;	//RTSP session set, shared with parse thread

	ZQ::common::NativeThreadPool &_pool;		//thread pool object

	int		m_iMaxSessionNum;
	int32	_iDefaultHeartBeatTime;

	recv_thread	_recvThrd;
};

#endif __DAEMON_THREAD_H__