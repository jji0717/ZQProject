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
uint16 RTSPMessageLine::FirstLine(const char *pMethode,  ::std::string &pServerPath, uint16 uServerPort, ::std::string &pContentID, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	int iMessageLen = 0;
	iMessageLen += sprintf(fSendBuffer + iMessageLen, "%s rtsp://%s:%d", pMethode, pServerPath.data(), uServerPort);

	if (!pContentID.empty())
		iMessageLen += sprintf(fSendBuffer + iMessageLen, "/%s", pContentID.data());

	iMessageLen += sprintf(fSendBuffer + iMessageLen, " RTSP/1.0");
	iMessageLen += LastLine(fSendBuffer + iMessageLen);
	return iMessageLen;
}

uint16  RTSPMessageLine::LastLine(char *fSendBuffer)
{
	return sprintf(fSendBuffer, "\r\n");
}

//generate the "Transport: " section for setup request
uint16 RTSPMessageLine::TransportLine(SetupReqHeader &pSetupReqHeader, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	//generate Transport header content
	int iMessageLen = 0;

	iMessageLen += sprintf(fSendBuffer + iMessageLen, "Transport: ");
	iMessageLen += sprintf(fSendBuffer + iMessageLen, "MP2T/H2221/UDP;");
	iMessageLen += sprintf(fSendBuffer + iMessageLen, "unicast;");
	iMessageLen += sprintf(fSendBuffer + iMessageLen, "destination=%s;", pSetupReqHeader._strDestination.c_str());
	iMessageLen += sprintf(fSendBuffer + iMessageLen, "port=%d", pSetupReqHeader._iClientPort);

	iMessageLen += LastLine(fSendBuffer + iMessageLen);
	return iMessageLen;
}

//generate the "CSeq: " section for every rtsp request
uint16 RTSPMessageLine::SequenceLine(uint32 uSeq, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	return sprintf(fSendBuffer, "CSeq: %u\r\n", uSeq);
}

//generate the "Session: " section fro every rtsp request except SETUP
uint16 RTSPMessageLine::SessionLine(::std::string &pSessionID, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	if (pSessionID.length() == 0)
		return 0;
	else
		return sprintf(fSendBuffer, "Session: %s\r\n", pSessionID.data());
}

//generate the "User-Agent:  " section fro every rtsp request except SETUP
uint16 RTSPMessageLine::UserAgentLine(::std::string &pUserAgentID, char *fSendBuffer, const uint16 sBufferMaxSize)
{
	return sprintf(fSendBuffer, "User-Agent: %s\r\n", pUserAgentID.data());
}
