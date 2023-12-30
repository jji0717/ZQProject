#include "daemon_thread.h"
#include "CVSSRtspParser/RTSPMessage/RTSPMessageParser.h"
#include "CVSSImpl.h"

#define SessionSyncTime 30*1000

int iClientNumber = 0;
string strLocalIP="192.168.81.103";

//const variables for r2 interaction
const char* pComPath = "com.comcast.org";
const char* pInterfaceId = "r2";
const char* pTransportType = "MP2T/H2221/UDP";
const char* pSDPProviderId = "schange.com";
const char* pSDPRange = "0-";
const char* pStreamControlProtoType = "rtsp";

daemon_thread::daemon_thread(::ZQ::common::FileLog *logfile,
							 ::ZQ::common::NativeThreadPool &recvPool,
							 ::ZQ::common::NativeThreadPool &sendPool,
							 RtspCSeqSignal &rtspCSeqSignal,
							 ::ZQTianShan::CVSS::CVSSEventList &eventList,
							 ::Ice::CommunicatorPtr communicator,
							 ZQADAPTER_DECLTYPE adapter)
:BaseThread(logfile)
,_pool(sendPool)
,_iDefaultHeartBeatTime(SessionSyncTime)
,_recvThrd(logfile, _sessionSocketSet,  &_cvssRtspSessionSet, recvPool, sendPool, rtspCSeqSignal, communicator, adapter)
,_eventList(eventList)
,_communicator(communicator)
,_adapter(adapter)
{

}

daemon_thread::~daemon_thread()
{
	stopThrd();
}

bool daemon_thread::init(void)
{
	_recvThrd.start();
	return true;
}

int daemon_thread::run(void)
{
	_bLoop = true;

	int iStartTime = GetTickCount();
	int iEndTime = GetTickCount();
	while (_bLoop)
	{
		bool bProcess = false;

		//readLock();
		//{
		//::ZQ::common::MutexGuard guard(_mutex);
		_mutex.enter();
		if (_socketSet.empty())
		{
			//readUnlock();
			Sleep(100);
			continue;
		}

		//DWORD sTime = GetTickCount();
		SOCKETList tmpSockList(_socketSet._socketList);
		_mutex.leave();
		//MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(daemon_thread, "copy socket list cost %dms"), GetTickCount() - sTime);
			
		//}
		//readUnlock();

		for (SOCKETList::iterator iter = tmpSockList.begin(); iter != tmpSockList.end(); iter++)
		{
			CVSSRtspSession *sess = _cvssRtspSessionSet.inSet(*iter);

			//not find in socket set(an mismatch error should be process)
			if (sess == NULL)
				continue;

			//socket status error
			if (sess->_rtspSocket._status == false)
				continue;

			int timeInterval = GetTickCount() - sess->_lastHeartBeatTime;
			if (timeInterval >= sess->_commonResHeader._iTimeOut)
			{
				//TODO: use thread request to process heartbeat message
				doHeartBeat(*sess);
				sess->_lastHeartBeatTime = GetTickCount();
			}
		}//for loop _sockList

		//if no client has data to receive, sleep a while, else sleep 1 millisecond
		if (bProcess == false)
			Sleep(100);
		else
			Sleep(1);
	}//while (_bLoop)

	::SetEvent(_handle);
	MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(daemon_thread, "thread exit"));
	return 1;
}

bool daemon_thread::addRTSPSession(CVSSRtspSession *sess, SOCKET &sock)
{
	if (sess == NULL)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(daemon_thread, "addRTSPSession fail, session object is NULL"));
		return false;
	}
	if (sock < 0)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(daemon_thread, "addRTSPSession fail, socket index invalid"));
		return false;
	}

	//add to daemon thread session socket list
	bool b = _sessionSocketSet.inSet(sock);
	if (b == true)
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "addRTSPSession fail, socket index(%d) already in map"), sock);
		return false;
	}
	else
		_sessionSocketSet.push(sock, &sess->_rtspSocket);

	//add CVSS RTSP session to map
	b = _cvssRtspSessionSet.inSet(sock);
	if (b == true)
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "addRTSPSession fail, socket index(%d) already in RTSP session map"), sock);
		return false;
	}
	else
		_cvssRtspSessionSet.push(sock, sess);

	//add to recv thread session socket map
	b = _recvThrd.addSessionSocket(sock);
	if (b == false)
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "addRTSPSession fail, socket index(%d) already in recv thread socket session map"), sock);
		return false;
	}

	//writeLock();
	{
		::ZQ::common::MutexGuard guard(_mutex);
		_socketSet.push(sock);
	}
	//writeUnlock();

	MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(daemon_thread, "addRTSPSession RTSP session with socket index(%d) success"), sock);
	return true;
}

bool daemon_thread::addRTSPSession(CVSSRtspSession *sess, ::std::string &strSessionId)
{
	if (sess == NULL)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(daemon_thread, "addSessionBySocketIdx fail, session object is NULL"));
		return false;
	}
	if (strSessionId.empty())
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(daemon_thread, "addSessionBySocketIdx fail, session index is NULL"));
		return false;
	}
	return true;
}

bool daemon_thread::removeRTSPSession(SOCKET &sock)
{
	bool b = true;
	if (sock < 0)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(daemon_thread, "removeRTSPSession fail, socket index is  invalid"));
		return false;
	}

	SessionSocket *sockSess = NULL;
	CVSSRtspSession *rtspSess = NULL;

	//remove session socket from map
	sockSess = _sessionSocketSet.remove(sock);
	if (sockSess != NULL)
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "removeRTSPSession:remove from Session Socket map succcess with socket index(%d)"), sock);
	else
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "removeRTSPSession:remove from Session Socket map fail with socket index(%d)"), sock);
		b = false;
	}

	rtspSess = _cvssRtspSessionSet.remove(sock);
	if (rtspSess != NULL)
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "remove RTSP Session from map success"));
	else
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "remove RTSP Session from map fail, not in map"));
		b = false;
	}

	//remove session socket from recv thread's map
	if (_recvThrd.removeSessionSocket(sock))
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(daemon_thread, "remove Session Socket(%d) from recv thread success"), sock);
	else
	{
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(daemon_thread, "remove Session Socket(%d) from recv thread fail"), sock);
		b = false;
	}

	//writeLock();
	{
		::ZQ::common::MutexGuard guard(_mutex);
		_socketSet.push(sock);
	}
	//writeUnlock();
	return b;
}

bool daemon_thread::removeRTSPSession(::std::string &strSessionId)
{
	bool b = true;

	//TODO: use session id to clean information

	return b;
}

CVSSRtspSession *daemon_thread::findRTSPSession(SOCKET &sock)
{
	CVSSRtspSession *sess = _cvssRtspSessionSet.inSet(sock);
	if (sess == NULL)
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "findSessionBySocketIdx find rtsp session with socket index(%d) fail"), sock);
		return NULL;
	}
	else
	{
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(daemon_thread, "findSessionBySocketIdx find rtsp session with socket index(%d) success"), sock);
		return sess;
	}
}

CVSSRtspSession *daemon_thread::findRTSPSession(::std::string &strSessionId)
{
	CVSSRtspSession *sess = _cvssRtspSessionSet.inSet(strSessionId);
	if (sess == NULL)
	{
		MYLOG(ZQ::common::Log::L_WARNING, CLOGFMT(daemon_thread, "findSessionBySocketIdx find rtsp session with sessionid index(%s) fail"), strSessionId.c_str());
		return NULL;
	}
	else
	{
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(daemon_thread, "findSessionBySocketIdx find rtsp session with sessionid index(%s) success"), strSessionId.c_str());
		return sess;
	}
}

void daemon_thread::initRTSPClientSession(CVSSRtspSession &sess)
{
}

void daemon_thread::doHeartBeat(CVSSRtspSession &sess)
{
	::Ice::Identity tmpident = _communicator->stringToIdentity(sess._strStreamName);
	::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx& sessPrx = ::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx::uncheckedCast(_adapter->createProxy(tmpident));

	sessPrx->doHeartBeat();

	if (sessPrx)
		sessPrx->renewPathTicket();
}