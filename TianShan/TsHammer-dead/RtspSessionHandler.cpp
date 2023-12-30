#include "RtspSessionHandler.h"

namespace ZQHammer
{

RtspSessionHandler::RtspSessionHandler(ZQ::common::Log* log, ZQHammer::RtspSessionMap &sessionMap)
:_log(log), _sessionMap(sessionMap)
{

}

RtspSessionHandler::~RtspSessionHandler()
{

}

bool RtspSessionHandler::HandleMsg(ZQRtspCommon::IRtspReceiveMsg *receiveMsg, ZQRtspCommon::IRtspSendMsg *sendMsg)
{
	// ignore announce message 
	if (ZQRtspCommon::RTSP_MTHD_ANNOUNCE == receiveMsg->getVerb())
	{
		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(RtspSessionHandler, "HandleMsg() Ignore announce message"));
		return true;
	}

	// find the session 
	std::string strSequence = receiveMsg->getHeader("CSeq");
	DWORD sequence = atol(strSequence.c_str());
	RtspSession* session = _sessionMap.getSession(sequence);
	if (session != NULL)
	{
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionHandler, "HandleMsg() success to find session by sequence[%s]"), strSequence.c_str());
		session->onResponse(receiveMsg, sendMsg);
	}
	else
	{
		std::string strSessionID = receiveMsg->getHeader("Session");
		session = _sessionMap.getSession(strSessionID);
		if (session != NULL)
		{
			XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionHandler, "HandleMsg() success to find session by session ID[%s]"), strSessionID.c_str());
			session->onResponse(receiveMsg, sendMsg);
		}
		else
		{
			XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionHandler, "HandleMsg() failed to find session by sequence and session ID"));
		}
	}
	return true;
}

void RtspSessionHandler::onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{
	ZQRtspEngine::RtspClientPtr client = ZQRtspEngine::RtspClientPtr::dynamicCast(communicator);
	RtspSession* session = _sessionMap.getSession(client);
	if (session != NULL)
	{
		session->onCommunicatorError();
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionHandler, "onCommunicatorError() successed to find session by communicator[%lld]"), communicator->getCommunicatorId());
	}
	else
	{
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspSessionHandler, "onCommunicatorError() failed to find session by communicator[%lld]"), communicator->getCommunicatorId());
	}
	communicator = NULL;
}

}