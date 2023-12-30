
#include "RTSPMessage.h"
#include "RTSPMessageLine.h"
//using namespace RTSPMessage;
//using namespace RTSPMessageLine;

//generate the SETUP rtsp request
unsigned short RTSPMessage::SetupMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//generate setup message body
	// first line
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("SETUP", rtspClient._commonReqHeader._strServerIp, rtspClient._commonReqHeader._iServerPort, rtspClient._commonReqHeader._strContentId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	// request headers
	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Type: application/sdp\r\n");

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient._commonReqHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);
	
	fSendBufferSize += RTSPMessageLine::TransportLine(rtspClient._setupReqHeader, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "%s\r\n", g_strNotify.c_str());

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "%s\r\n", g_strNotify.c_str());

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;
	return 1;
}

//generate the PLAY rtsp request
unsigned short RTSPMessage::PlayMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("PLAY", rtspClient._commonReqHeader._strServerIp, rtspClient._commonReqHeader._iServerPort, rtspClient._commonReqHeader._strContentId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient._commonReqHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient._commonReqHeader._strSessionId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	if (!rtspClient._commonReqHeader._strRang.empty())
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Range: npt=%s-\r\n", rtspClient._commonReqHeader._strRang.c_str());

	if (!rtspClient._commonReqHeader._strScale.empty())
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Scale: %s\r\n", rtspClient._commonReqHeader._strScale.c_str());

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "%s\r\n", g_strPlayNow.c_str());
	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the PAUSE rtsp request
unsigned short RTSPMessage::PauseMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("PAUSE", rtspClient._commonReqHeader._strServerIp, rtspClient._commonReqHeader._iServerPort, rtspClient._commonReqHeader._strContentId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient._commonReqHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient._commonReqHeader._strSessionId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	if (!rtspClient._commonReqHeader._strRang.empty())
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Range: npt=%s-\r\n", rtspClient._commonReqHeader._strRang.c_str());

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the TEARDOWN rtsp request
unsigned short RTSPMessage::TeardownMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("TEARDOWN", rtspClient._commonReqHeader._strServerIp, rtspClient._commonReqHeader._iServerPort, rtspClient._commonReqHeader._strContentId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient._commonReqHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient._commonReqHeader._strSessionId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the GET_PARAMETER rtsp request
unsigned short RTSPMessage::GetParameterMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);
	GetParameterReqHeader *pSDPRequestContent = &(rtspClient._getParameterReqHeader);

	//generate GET_PARAMETER message content
	string strContent;
	if (!pSDPRequestContent->_strSDPField.empty())
		strContent += pSDPRequestContent->_strSDPField;

	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("GET_PARAMETER", rtspClient._commonReqHeader._strServerIp, rtspClient._commonReqHeader._iServerPort, rtspClient._commonReqHeader._strContentId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient._commonReqHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient._commonReqHeader._strSessionId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Type: text/parameters\r\n");

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Length: %d\r\n", strContent.length());

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	//copy the sdp-content to the message tail
	if (strContent.length() != 0)
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "%s", strContent.c_str());

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the SET_PARAMETER rtsp request
unsigned short RTSPMessage::SetParameterMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("SET_PARAMETER", rtspClient._commonReqHeader._strServerIp, rtspClient._commonReqHeader._iServerPort, rtspClient._commonReqHeader._strContentId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient._commonReqHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the ANNOUNCE rtsp response
unsigned short RTSPMessage::AnnounceMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);

	fSendBufferSize = 0;
	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "RTSP/1.0 200 OK\r\n");

	fSendBufferSize += RTSPMessageLine::SequenceLine(rtspClient._announceResHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient._commonReqHeader._strSessionId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the PING rtsp request
unsigned short PingMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("PING", rtspClient._commonReqHeader._strServerIp, rtspClient._commonReqHeader._iServerPort, rtspClient._commonReqHeader._strContentId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);
	
	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient._commonReqHeader._iCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient._commonReqHeader._strSessionId, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}