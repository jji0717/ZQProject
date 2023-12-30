//file to define the thread
//inherit NativeThread
//chop the RTSP message from NGOD server
//after chop, stored the message into a message list
#pragma once

#include "ngod_common_structure.h"

#include "ngod_send_threadreq.h"
#include "ngod_recv_thread.h"
#include "ngod_chop_thread.h"
#include "ngod_parse_thread.h"

//debug define
//#define DAEMONTHRDDEBUG

class ngod_daemon_thread : public ZQ::common::NativeThread
{
public:
	ngod_daemon_thread(ZQ::common::FileLog *logfile,
					  ::ZQTianShan::NSS::NssEventList &eventList,
					  ZQ::common::NativeThreadPool &pool,
					  ::Ice::CommunicatorPtr communicator,
					  ZQADAPTER_DECLTYPE adapter);

	ngod_daemon_thread(ZQ::common::FileLog *logfile,
					  ::ZQTianShan::NSS::NssEventList &eventList, 
					  NSSSessionGroupList &_NSSSessionGroupList, 
					  ZQ::common::NativeThreadPool &pool,
					  ::Ice::CommunicatorPtr communicator,
					  ZQADAPTER_DECLTYPE adapter);
	~ngod_daemon_thread();

	void setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList);

	bool	initialize(void);

	int		run(void);
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

	//member function for manage NSSSessionGroupList
	void	addGroup(NSSSessionGroup *SessionGroup);
	void	removeGroup(string &strSessionGroup);
	void	updateGroup(string &strSessionGroup);

	//member function for manage NSSSessionMap
	//return false if all session group are full
	bool	addSession(RTSPClientSession *sess);
	bool	addSession(RTSPClientSession *sess, string &strSessGroup);

	//bool	addSessionByOnDemandSessionId(RTSPClientSession *sess, string &strSessGroup);
	//bool	addSessionBySessionId(RTSPClientSession *sess, string &strSessGroup);

	//return false if could not find sess in any session group
	bool	removeSession(RTSPClientSession *sess);

	//always return true now
	bool	updateSession(RTSPClientSession *sess,
					      SDPRequestContent &pSDPReqContent,
					      RTSPTransportUdpHeader &pTransportHeader);

	RTSPClientSession *findSessionByOnDemandSessionId(const string	&strOnDemandSessionId);
	RTSPClientSession *findSessionBySessionId(const string	&strSessionId);

	::ZQTianShan::NSS::NssEventList &_eventList;

	::Ice::CommunicatorPtr	_communicator;
	ZQADAPTER_DECLTYPE		_adapter;
	StreamStrList			_streamerStrList;

private:
	void	syncSessionGroup(string &strSessionGroup);

	void	InitEvent();

	void	initRTSPClientSession(RTSPClientSession &sess, 
								  NSSSessionGroup &pNSSSessionGroup);

	bool	m_bRunToken;
	HANDLE	m_Event;
	NSSSessionGroupList	*m_NSSSessionGroupList;
	ZQ::common::NativeThreadPool *m_pPool;
	ZQ::common::FileLog *m_pLogFile;

	int		m_iMaxSessionNum;	

	ngod_chop_thread *m_pngod_chop_thread;
	ngod_parse_thread *m_pngod_parse_thread;
	ngod_recv_thread *m_pngod_recv_thread;

public:
	CRITICAL_SECTION _CS;
};