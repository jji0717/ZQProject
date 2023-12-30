/*
* =====================================================================================
* 
*       Filename:  RTSPMessageLine.cpp
* 
*    Description:  RTSPMessageLine.h中定义方法的具体实现
*        Version:  1.0
*        Created:  March 19th, 2008
*       Revision:  
*       Compiler:  vs.net 2005
* 
*         Author:  Xiaoming Li
*        Company:  
* 
* =====================================================================================
*/

#include "RTSPMessageLine.h"

//generate the first line for every rtsp request
uint16 RTSPMessageLine::FirstLine(const char *pMethode,  string &pServerPath, uint16 uServerPort, string &pSessionID, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	int iMessageLen = 0;
	iMessageLen += sprintf(fSendBuffer + iMessageLen, "%s rtsp://%s:%d", pMethode, pServerPath.data(), uServerPort);

	if (!pSessionID.empty())
		iMessageLen += sprintf(fSendBuffer + iMessageLen, "/%s", pSessionID.data());

	iMessageLen += sprintf(fSendBuffer + iMessageLen, " RTSP/1.0\r\n");
	return iMessageLen;
}

uint16  RTSPMessageLine::LastLine(char *fSendBuffer, const uint16 sBufferMaxSize)
{
	return sprintf(fSendBuffer, "\r\n");
}

//generate the "Transport: " section for setup request
uint16 RTSPMessageLine::TransportLine(RTSPTransportHeader &pRTSPTransportHeader, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	//generate Transport header content
	int iMessageLen = 0;

	iMessageLen += sprintf(fSendBuffer + iMessageLen, "Transport: ");

	//generate transport information
	for (RTSPTransportUdpHeaderVec::iterator iter = pRTSPTransportHeader.TransportUdpHeader.begin(); iter != pRTSPTransportHeader.TransportUdpHeader.end(); iter++)
	{
		if (iter != pRTSPTransportHeader.TransportUdpHeader.begin())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ",");

		iMessageLen += sprintf(fSendBuffer + iMessageLen, "%s;", (*iter).strType.data());

		iMessageLen += sprintf(fSendBuffer + iMessageLen, "unicast;");

		iMessageLen += sprintf(fSendBuffer + iMessageLen, "client=%s", (*iter).strClient_id.data());

		if (!(*iter).strQam_destination.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";qam_destination=%s", (*iter).strQam_destination.data());

		if (!(*iter).strDestination.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";destination=%s", (*iter).strDestination.data());

		if (!(*iter).strClient_port.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";client_port=%s", (*iter).strClient_port.data());

		if (!(*iter).strSource.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";source=%s", (*iter).strSource.data());

		if (!(*iter).strBandwidth.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";bandwidth=%s", (*iter).strBandwidth.data());

		if (!(*iter).strSop_name.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";sop_name=%s", (*iter).strSop_name.data());

		if (!(*iter).strSop_group.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";sop_group=%s", (*iter).strSop_group.data());

		if (!(*iter).strQam_name.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";qam_name=%s", (*iter).strQam_name.data());

		if (!(*iter).strQam_group.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";qam_group=%s", (*iter).strQam_group.data());

		if (!(*iter).strEdge_input_group.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";edge_input_group=%s", (*iter).strEdge_input_group.data());

		if (!(*iter).strAnnex.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";annex=%s", (*iter).strAnnex.data());

		if (!(*iter).strChannel_width.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";channel_width=%s", (*iter).strChannel_width.data());

		if (!(*iter).strModulation.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";modulation=%s", (*iter).strModulation.data());

		if (!(*iter).strInterleaver.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";interleaver=%s", (*iter).strInterleaver.data());

		if (!(*iter).strEncryptor_group.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";encryptor_group=%s", (*iter).strEncryptor_group.data());

		if (!(*iter).strEncryptor_name.empty())
			iMessageLen += sprintf(fSendBuffer + iMessageLen, ";encryptor_name=%s", (*iter).strEncryptor_name.data());
	}

	iMessageLen += sprintf(fSendBuffer + iMessageLen, "\r\n");
	return iMessageLen;
}

//generate the "CSeq: " section for every rtsp request
uint16 RTSPMessageLine::SequenceLine(uint16 uSeq, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	return sprintf(fSendBuffer, "CSeq: %u\r\n", uSeq);
}

//generate the "Require: " section for every rtsp request except TEARDOWN
uint16 RTSPMessageLine::RequireLine(RTSPRequireHeader &pRTSPRequireHeader, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	return sprintf(fSendBuffer, "Require: %s.%s\r\n", pRTSPRequireHeader.strComPath.data(), pRTSPRequireHeader.strInterface_id.data());
}

//generate the "OnDemandSession: " section for SETUP/TEARDOWN/ANNOUNCE rtsp request
uint16 RTSPMessageLine::OnDemandSessionIdLine(string &pOnDemandSessionID, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	return sprintf(fSendBuffer, "OnDemandSessionId: %s\r\n", pOnDemandSessionID.data());
}

//generate the "SessionGroup: " section for SETUP/GET_PARAMETER/SET_PARAMETER rtsp request
uint16 RTSPMessageLine::SessionGroupLine(RTSPSessionGroupHeader &pRTSPSessionGroupHeader, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	if (pRTSPSessionGroupHeader.strToken.empty())
		return 0;
	else
		return sprintf(fSendBuffer, "SessionGroup: %s\r\n", pRTSPSessionGroupHeader.strToken.data());
}

//generate the "Session: " section fro every rtsp request except SETUP
uint16 RTSPMessageLine::SessionLine(string &pSessionID, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	if (pSessionID.length() == 0)
		return 0;
	else
		return sprintf(fSendBuffer, "Session: %s\r\n", pSessionID.data());
}

//generate the "User-Agent:  " section fro every rtsp request except SETUP
uint16 RTSPMessageLine::UserAgentLine(string &pUserAgentID, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	return sprintf(fSendBuffer, "User-Agent: %s\r\n", pUserAgentID.data());
}
