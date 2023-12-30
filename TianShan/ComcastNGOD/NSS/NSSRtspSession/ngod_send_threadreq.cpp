#include "ngod_send_threadreq.h"
#include "ngod_rtsp_parser/RTSPMessage/RTSPMessage.h"
#include "ngod_rtsp_parser/ClientSocket.h"

#define MYLOG (*m_pLogFile)

ngod_send_threadreq::ngod_send_threadreq(ZQ::common::FileLog *logfile, 
										 ZQ::common::NativeThreadPool& Pool, 
										 RTSPClientSession *ss,
										 bool IsRelease)
:ThreadRequest(Pool),
m_pRTSPClientSession(ss),
m_pLogFile(logfile),
_IsRelease(IsRelease)
{

}

ngod_send_threadreq::~ngod_send_threadreq()
{
	//if (m_pRTSPClientSession->iRTSPClientState == SETPARAMETER || m_pRTSPClientSession->iRTSPClientState ==GETPARAMETER)
	if (_IsRelease)
		delete m_pRTSPClientSession;
	m_pRTSPClientSession = NULL;
	m_pLogFile = NULL;
}

int ngod_send_threadreq::run(void)
{
	char pMessageBuf[SENDBUFMAXSIZE];
	unsigned short sendLen = 0;

	//check the client status and decide the message type
	switch(m_pRTSPClientSession->iRTSPClientState) {
	case SETUP:
		RTSPMessage::SetupMessage(*m_pRTSPClientSession, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case PLAY:
		RTSPMessage::PlayMessage(*m_pRTSPClientSession, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case PAUSE:
		RTSPMessage::PauseMessage(*m_pRTSPClientSession, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case TEARDOWN:
		RTSPMessage::TeardownMessage(*m_pRTSPClientSession, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case ANNOUNCE:
		RTSPMessage::AnnounceMessage(*m_pRTSPClientSession, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case GETPARAMETER:
		RTSPMessage::GetParameterMessage(*m_pRTSPClientSession, pMessageBuf, SENDBUFMAXSIZE, sendLen);
		break;
	case SETPARAMETER:
		RTSPMessage::SetParameterMessage(*m_pRTSPClientSession, pMessageBuf, SENDBUFMAXSIZE, sendLen);
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
		MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_send_threadreq,"send one message"));
		sprintf(hint, "SOCKET: %08x", *m_pRTSPClientSession->RTSPSocket);
		if (m_pLogFile)
			m_pLogFile->hexDump(ZQ::common::Log::L_DEBUG, pMessageBuf, sendLen, hint);
					
		//send message
		EnterCriticalSection(m_pRTSPClientSession->m_pCS);
		int ret = sTCPSend(pMessageBuf, sendLen, *m_pRTSPClientSession->RTSPSocket);
		LeaveCriticalSection(m_pRTSPClientSession->m_pCS);
		if (ret == sendLen)
			return 1;
		else
			return 0;
	}
	else
		return 0;	
}