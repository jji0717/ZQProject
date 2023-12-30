#include "RTSP_MessageSendThrd.h"
#include "ClientSocket.h"

RTSP_MessageSendThrd::RTSP_MessageSendThrd(::ZQ::common::FileLog &fileLog, ::ZQ::common::NativeThreadPool &pool, SessionSocket &sessionSocket, ::std::string &rtspMessage, XML_SessCtxHandler &xml_SessCtxHandler/*, XML_SessCtxHandlerVec &xml_SessCtxHandlerVec, int32 &interval, CRITICAL_SECTION &CS*/)
:ThreadRequest(pool)
,_log(&fileLog)
,_socket(sessionSocket)
,_rtspMessage(rtspMessage)
,_xml_SessCtxHandler(xml_SessCtxHandler)
{

}

RTSP_MessageSendThrd::~RTSP_MessageSendThrd()
{

}

int RTSP_MessageSendThrd::run()
{
	bool b = _xml_SessCtxHandler.fixupMacro(_rtspMessage);
	if (!b)
	{
		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageSendThrd,"socket(%d) fixup one message Macro error"), _socket.m_Socket);
		return 0;
	}

	if (_socket.m_Status)
	{
		//send message
		uint16 sendLen = (uint16)_rtspMessage.length();
		int ret = 0;

		{
			::ZQ::common::MutexGuard guard(_socket._mutex);
			_socket._postMessageTime = GetTickCount();
			ret = sTCPSend(_rtspMessage.c_str(), sendLen, _socket.m_Socket);
		}

		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageSendThrd,"socket(%d) send one message\r\n%s"), _socket.m_Socket, _rtspMessage.c_str());

		//(*_xml_SessCtxHandlerVec.begin())->modifyGlobalMacro();
		if (ret == sendLen)
			return 1;
		else
			return 0;
	}
	else
	{
		XMLLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSP_MessageSendThrd,"send socket(%d) status error\r\n"), _socket.m_Socket);
		return -1;
	}
}