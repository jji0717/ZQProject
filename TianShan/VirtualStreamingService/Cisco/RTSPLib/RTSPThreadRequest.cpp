#include <WinSock2.h>
#include "CVSSImpl.h"
#include "RTSPThreadRequest.h"
#include "CVSSRtspParser/ClientSocket.h"
#include "CVSSRtspParser/RTSPMessage/RTSPMessage.h"
#include "CVSSRtspParser/RTSPMessage/RTSPMessageParser.h"
#include "rtsp_action.h"

//RTSP request sender thread request
RTSPRequestSender::RTSPRequestSender(::ZQ::common::FileLog *logFile, ::ZQ::common::NativeThreadPool &pool, CVSSRtspSession *sess)
:ThreadRequest(pool)
,_pLog(logFile)
,_pSess(sess)
{

}

RTSPRequestSender::~RTSPRequestSender()
{
	_pSess = NULL;
}

int RTSPRequestSender::run(void)
{
	if (_pSess == NULL)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSPResponseRecver,"session is null"));
		return -1;
	}
	//_pSess->_lock.ReadLock();
	if (_pSess->_rtspSocket._status == false)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSPResponseRecver,"session socket(%d) status is abnormal"), _pSess->_rtspSocket._socket);
		//_pSess->_lock.ReadUnlock();
		return -1;
	}

	char pMessageBuf[SENDBUFMAXSIZE];
	unsigned short sendLen = 0;

	//check the client status and decide the message type
	switch(_pSess->iRTSPClientState) {
	case SETUP:
		RTSPMessage::SetupMessage(*_pSess, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case PLAY:
		RTSPMessage::PlayMessage(*_pSess, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case PAUSE:
		RTSPMessage::PauseMessage(*_pSess, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case TEARDOWN:
		RTSPMessage::TeardownMessage(*_pSess, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case ANNOUNCE:
		RTSPMessage::AnnounceMessage(*_pSess, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case GETPARAMETER:
		RTSPMessage::GetParameterMessage(*_pSess, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case SETPARAMETER:
		RTSPMessage::SetParameterMessage(*_pSess, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	default:
		sendLen = 0;
		return 0;
	}
	pMessageBuf[sendLen] = 0;

	if (sendLen > 0)
	{
		//cout << "send one message\r\n" << pMessageBuf << endl;
		//write log
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSPRequestSender,"socket(%d) send one message"), _pSess->_rtspSocket._socket);
		sprintf(hint, "SOCKET: %08x\0", _pSess->_rtspSocket._socket);
		if (_pLog)
			_pLog->hexDump(ZQ::common::Log::L_DEBUG, pMessageBuf, sendLen, hint);

		_pSess->_lastSendTime = GetTickCount();
		//send message
		int ret = sTCPSend(pMessageBuf, sendLen, _pSess->_rtspSocket._socket);
		//_pSess->_lock.ReadUnlock();
		if (ret == sendLen)
			return 1;
		else
			return 0;
	}
	else
	{
		//_pSess->_lock.ReadUnlock();
		return 0;
	}
}


//RTSP receiver thread request
RTSPResponseRecver::RTSPResponseRecver(::ZQ::common::FileLog *logFile, ::ZQ::common::NativeThreadPool &pool, CVSSRtspSession *sess, RtspCSeqSignal &rtspCSeqSignal, ::Ice::CommunicatorPtr communicator, ZQADAPTER_DECLTYPE adapter, bool IsRelease)
:ThreadRequest(pool)
,_pLog(logFile)
,_pSess(sess)
,_rtspCSeqSignal(rtspCSeqSignal)
,_isReleas(IsRelease)
,_communicator(communicator)
,_adapter(adapter)
{

}

RTSPResponseRecver::~RTSPResponseRecver()
{
	_pSess->_rtspSocket.setRecvStatus(false);
	if (_isReleas)
	{
		closesocket(_pSess->_rtspSocket._socket);
		delete _pSess;
		_pSess = NULL;
	}
}

int RTSPResponseRecver::run(void)
{
	if (_pSess == NULL)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSPResponseRecver,"session is null"));
		return -1;
	}
#ifdef USERWLOCK
	_pSess->_lock.ReadLock();
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_pSess->_mutex);
#endif	
	if (_pSess->_rtspSocket._status == false)
	{
		MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSPResponseRecver,"session socket(%d) status is abnormal"), _pSess->_rtspSocket._socket);
#ifdef USERWLOCK
		_pSess->_lock.ReadUnlock();
#endif
		return -1;
	}

	//set variables for easy use
	SmartBuffer &buf = _pSess->_rtspSocket._smartBuffer;
	int32 &iWritePos = buf._iWritePos;
	int32 &iReadPos = buf._iReadPos;
	SOCKET &sock = _pSess->_rtspSocket._socket;

	strlist msgList;
	::std::string strMsg;

	while (1)
	{
		int ret = sRecv(buf._pBuffer + iWritePos, buf._iBufferMaxSize - iWritePos, sock, TCPSOCKET, NONBLOCK);

		//check the recv return value
		if (ret < 0)
		{
			int err = WSAGetLastError();
			_pSess->_rtspSocket._status = false;
			MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSPResponseRecver,"session socket(%d) recv encounter error"), sock);
#ifdef USERWLOCK
			_pSess->_lock.ReadUnlock();
#endif
			return -1;
		}
		else if (ret > 0)
		{
			//move the write position
			iWritePos = 0;
			::std::string tmpStr(buf._pBuffer + iReadPos + 1, ret);
			strMsg += tmpStr;
		}
		else if (ret == 0)
		{
			break;
		}
	}

	if (strMsg.empty())
	{
#ifdef USERWLOCK
		_pSess->_lock.ReadUnlock();
#endif
		return 0;
	}
	
	chopMsg(strMsg, msgList);

	for (strlist::iterator iter = msgList.begin(); iter != msgList.end(); iter++)
	{
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSPResponseRecver,"socket(%d) begin to parse one message"), sock);

		bool b = parseMsg(*iter);
		if (!b)
			MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSPResponseRecver,"socket(%d) parse message encounter error"), sock);
		else
			MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSPResponseRecver,"socket(%d) parse message ok"), sock);
	}
	
#ifdef USERWLOCK
	_pSess->_lock.ReadUnlock();
#endif
	return 1;
}

void RTSPResponseRecver::chopMsg(::std::string &strMsg, strlist &msgList)
{
	RTSPMessageParser::MessageSplit(strMsg.c_str(), strMsg.length(), msgList);
}

bool RTSPResponseRecver::parseMsg(::std::string &msg)
{
	SOCKET &sock = _pSess->_rtspSocket._socket;

	MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSPResponseRecver, "parseMsg: message of socket(%d)"), sock);
	sprintf(hint, "SOCKET: %08x\0", sock);
	if (_pLog)
		_pLog->hexDump(ZQ::common::Log::L_DEBUG, msg.c_str(), msg.length(), hint);

	DWORD sTime = GetTickCount();
	::std::string strSessionId;
	int32 iTimeOut = 0;

	//get sequence number
	uint32 iCSeq = 0;
	RTSPMessageParser::GetSequence(msg.c_str(), msg.length(), iCSeq);

	//get session id
	if (strSessionId.empty())
		RTSPMessageParser::GetSessionID(msg.c_str(), msg.length(), strSessionId, iTimeOut);

	//check if announce message
	RTSPClientState state = RTSPMessageParser::CheckMessageType(msg.c_str(), msg.length());
	if (state == ANNOUNCE)
	{
		//TODO: response OK to server and check session
		//TODO: response OK to server and check session
		sendAnnounceResponse(_pSess, iCSeq);

		::Ice::Identity tmpident;
		try
		{
			tmpident = _communicator->stringToIdentity(_pSess->_strStreamName);
			::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx& sessPrx = ::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx::uncheckedCast(_adapter->createProxy(tmpident));
			sessPrx->playNextItem();
		}
		catch (::Ice::Exception &ex)
		{
			MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(parse_thread,"Ident(%s)Socket(%d)session(%s)CSeq(%d) play next item get ice exception(%s)"), tmpident.name.c_str(), _pSess->_rtspSocket._socket, _pSess->_commonReqHeader._strSessionId.c_str(), _pSess->_commonReqHeader._iCSeq, ex.what());
		}
		catch (...)
		{
			MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(parse_thread,"Ident(%s)Socket(%d)session(%s)CSeq(%d) play next item get unkonw exception"), tmpident.name.c_str(), _pSess->_rtspSocket._socket, _pSess->_commonReqHeader._strSessionId.c_str(), _pSess->_commonReqHeader._iCSeq);
		}
	}
	else//
	{
		//TODO: parse message and process
		RTSPSessionState state = (RTSPSessionState)RTSPMessageParser::MessageParser(msg.c_str(), msg.length(), *_pSess);	
		_rtspCSeqSignal.m_SetEvent(sock);
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSPResponseRecver,"parseMsg(): parsed socket(%d) msg cost %dms"), sock, GetTickCount() - _pSess->_lastSendTime);
	}
	MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTSPResponseRecver,"parseMsg(): parse one message of socket(%d) cost %dms"), sock, GetTickCount() - sTime);
	return true;
}

void RTSPResponseRecver::sendAnnounceResponse(CVSSRtspSession *cvssRtspSession, uint32 iCSeq)
{
	cvssRtspSession->_announceResHeader._iCSeq = iCSeq;
	bool b = rtsp_action::RTSPAction(ANNOUNCE, _rtspCSeqSignal, cvssRtspSession, _pool, *dynamic_cast<::ZQ::common::FileLog *>(_pLog));
}