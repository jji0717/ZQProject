
#include "RTSPMessage.h"
#include "RTSPMessageLine.h"
using namespace RTSPMessage;
using namespace RTSPMessageLine;

//generate the SETUP rtsp request
unsigned short RTSPMessage::SetupMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);
	char pSDP[g_iSDPMaxLen];
	//memset(pSDP, 0, g_iSDPMaxLen);

	//local variables for convenient use
	SDPRequestContentVec *pSDPRequestContent = &(rtspClient.m_RTSPR2Header.m_SDPRequestContent);
	string strOndemandSessionID = rtspClient.m_RTSPR2Header.strOnDemandSessionID;


	//generate sdp content
	//memset(pSDP, 0, g_iSDPMaxLen);

	int iSDPLen = 0;
	iSDPLen += sprintf(pSDP + iSDPLen, "v=0\r\n");

	iSDPLen += sprintf(pSDP + iSDPLen, "o=- %s IN IP4 %s", strOndemandSessionID.data(), rtspClient.strLocalIP.data());
	if (rtspClient.uLocalPort > 0)
		iSDPLen += sprintf(pSDP + iSDPLen, ":%d",rtspClient.uLocalPort);
	iSDPLen += sprintf(pSDP + iSDPLen, "\r\n");

	iSDPLen += sprintf(pSDP + iSDPLen, "s=\r\n");
	iSDPLen += sprintf(pSDP + iSDPLen, "t=0 0\r\n");
	
	//generate "a=X-playlist-item: "
	for (SDPRequestContentVec::iterator iter = pSDPRequestContent->begin(); 
		iter != pSDPRequestContent->end(); iter++)
		{
			iSDPLen += sprintf(pSDP + iSDPLen, "a=X-playlist-item: %s %s", (*iter).strProvider_id.data(), (*iter).strAsset_id.data());
			if (!(*iter).strRange.empty())
				iSDPLen += sprintf(pSDP + iSDPLen, " %s", (*iter).strRange.data());
		}
		iSDPLen += sprintf(pSDP + iSDPLen, "\r\n");

	iSDPLen += sprintf(pSDP + iSDPLen, "c=IN IP4 0.0.0.0\r\n");

	iSDPLen += sprintf(pSDP + iSDPLen, "m=video 0 udp MP2T\r\n");
	pSDP[iSDPLen] = 0;


	//generate setup message body
	// first line
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("SETUP", rtspClient.strServerPath, rtspClient.uServerPort, rtspClient.m_RTSPR2Header.strSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);	

	// request headers
	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Type: application/sdp\r\n");

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient.uClientCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::OnDemandSessionIdLine(strOndemandSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::RequireLine(rtspClient.m_RTSPR2Header.Require, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionGroupLine(rtspClient.m_RTSPR2Header.SessionGroup, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	if (!rtspClient.m_RTSPR2Header.StreamControlProto.strType.empty())
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "StreamControlProto: %s\r\n",rtspClient.m_RTSPR2Header.StreamControlProto.strType.data());
	
	fSendBufferSize += RTSPMessageLine::TransportLine(rtspClient.m_RTSPR2Header.m_RTSPTransportHeader, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);
	
	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Volume: %s\r\n", rtspClient.m_RTSPR2Header.Volume.strName.data());

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Length: %d\r\n", iSDPLen);

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "\r\n");

	//copy the sdp-content to the message tail
	if (strlen(pSDP) != 0)
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "%s", pSDP);

	pSDPRequestContent = NULL;
	fSendBuffer[fSendBufferSize] = 0;
	return 1;
}

//generate the PLAY rtsp request
unsigned short RTSPMessage::PlayMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);

	SDPResponseContent *pSDPResponseContent = &(rtspClient.m_RTSPR2Header.m_SDPResponseContent);
	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("PLAY", pSDPResponseContent->strHost, pSDPResponseContent->uPort, pSDPResponseContent->strStreamhandle, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient.uClientCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::RequireLine(rtspClient.m_RTSPR2Header.Require, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient.m_RTSPR2Header.strSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);


	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Scale: %.3f\r\n", rtspClient.fScale);

	if (rtspClient.strCurrentTimePoint.length() > 0)
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Range: npt=%s-\r\n", rtspClient.strCurrentTimePoint.data());

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the PAUSE rtsp request
unsigned short RTSPMessage::PauseMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);

	SDPResponseContent *pSDPResponseContent = &(rtspClient.m_RTSPR2Header.m_SDPResponseContent);

	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("PAUSE", pSDPResponseContent->strHost, pSDPResponseContent->uPort, pSDPResponseContent->strStreamhandle, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient.uClientCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::RequireLine(rtspClient.m_RTSPR2Header.Require, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient.m_RTSPR2Header.strSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the TEARDOWN rtsp request
unsigned short RTSPMessage::TeardownMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);

	SDPResponseContent *pSDPResponseContent = &(rtspClient.m_RTSPR2Header.m_SDPResponseContent);

	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("TEARDOWN", pSDPResponseContent->strHost, pSDPResponseContent->uPort, pSDPResponseContent->strStreamhandle, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient.uClientCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient.m_RTSPR2Header.strSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "x-reason: User Requested Teardown\r\n");

	fSendBufferSize += RTSPMessageLine::OnDemandSessionIdLine(rtspClient.m_RTSPR2Header.strOnDemandSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the GET_PARAMETER rtsp request
unsigned short RTSPMessage::GetParameterMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);

	SDPResponseContent *pSDPResponseContent = &(rtspClient.m_RTSPR2Header.m_SDPResponseContent);
	GetPramameterReq_ExtHeader *pSDPRequestContent = &(rtspClient.m_RTSPR2Header.m_GetPramameterReq_ExtHeader);

	//generate GET_PARAMETER message content
	string strContent;
	for (vector<GETPARAMETER_EXT>::iterator iter = pSDPRequestContent->header.begin(); iter != pSDPRequestContent->header.end(); iter++)
	{
		if (iter != pSDPRequestContent->header.begin())
			strContent += string(" ");

		strContent += pGetPramameter_ExtHeader[(*iter)];
	}

	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("GET_PARAMETER", pSDPResponseContent->strHost, pSDPResponseContent->uPort, pSDPResponseContent->strStreamhandle, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SequenceLine(rtspClient.uClientCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::RequireLine(rtspClient.m_RTSPR2Header.Require, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Type: text/parameters\r\n");

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient.m_RTSPR2Header.strSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	if (strContent.find("session_list") != string::npos)
		fSendBufferSize += RTSPMessageLine::SessionGroupLine(rtspClient.m_RTSPR2Header.SessionGroup, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Length: %d\r\n", strContent.length());

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "\r\n");

	//copy the sdp-content to the message tail
	if (strContent.length() != 0)
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "%s", strContent.c_str());

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the SET_PARAMETER rtsp request
unsigned short RTSPMessage::SetParameterMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	string strContent;
	vector<string> *pGroups = &(rtspClient.m_RTSPR2Header.m_SessionGroups.strSessionGroup);
	if (!pGroups->empty())
	{
		strContent += string("session_groups: ");
		for(int i = 0; i < pGroups->size(); i++)
		{
			if (i != 0)
				strContent += " ";
			strContent += rtspClient.m_RTSPR2Header.m_SessionGroups.strSessionGroup[i];
		}
		strContent += string("\r\n");
	}
	else
		strContent.resize(0);
	
	//memset(fSendBuffer, 0, sBufferMaxSize);

	SDPResponseContent *pSDPResponseContent = &(rtspClient.m_RTSPR2Header.m_SDPResponseContent);

	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("SET_PARAMETER", pSDPResponseContent->strHost, pSDPResponseContent->uPort, pSDPResponseContent->strStreamhandle, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SequenceLine(rtspClient.uClientCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);
	
	fSendBufferSize += RTSPMessageLine::RequireLine(rtspClient.m_RTSPR2Header.Require, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);
	
	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Type: text/parameters\r\n");

	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "Content-Length: %d\r\n", strContent.length());

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	if (strContent.length() > 0)
		fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "%s", strContent.c_str());

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the ANNOUNCE rtsp response
unsigned short RTSPMessage::AnnounceMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);

	fSendBufferSize = 0;
	fSendBufferSize += sprintf(fSendBuffer + fSendBufferSize, "RTSP/1.0 %d OK\r\n", rtspClient.iRTSPSessionState);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient.uServerCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	//fSendBufferSize += RTSPMessageLine::RequireLine(rtspClient.m_RTSPR2Header.Require, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient.m_RTSPR2Header.strSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::OnDemandSessionIdLine(rtspClient.m_RTSPR2Header.strOnDemandSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}

//generate the PING rtsp request
unsigned short PingMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize)
{
	//memset(fSendBuffer, 0, sBufferMaxSize);

	SDPResponseContent *pSDPResponseContent = &(rtspClient.m_RTSPR2Header.m_SDPResponseContent);

	fSendBufferSize = 0;
	fSendBufferSize += RTSPMessageLine::FirstLine("PING", pSDPResponseContent->strHost, pSDPResponseContent->uPort, pSDPResponseContent->strStreamhandle, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize +=  RTSPMessageLine::SequenceLine(rtspClient.uClientCSeq, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::RequireLine(rtspClient.m_RTSPR2Header.Require, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::SessionLine(rtspClient.m_RTSPR2Header.strSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::OnDemandSessionIdLine(rtspClient.m_RTSPR2Header.strOnDemandSessionID, fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBufferSize += RTSPMessageLine::LastLine(fSendBuffer + fSendBufferSize, sBufferMaxSize - fSendBufferSize);

	fSendBuffer[fSendBufferSize] = 0;

	return 1;
}