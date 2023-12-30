/*
* =====================================================================================
* 
*       Filename:  RTSPMessageSession.cpp
* 
*    Description:  实现NGOD RTSP交互过程中供调用的各类方法
*        Version:  1.0
*        Created:  March 20th, 2008
*       Revision:  
*       Compiler:  vs.net 2005
* 
*         Author:  Xiaoming Li
*        Company:  
* 
* =====================================================================================
*/

#include "RTSPMessageSession.h"
#include "RTSPMessageParser.h"
#include "../ClientSocket.h"
using namespace RTSPMessageSession;

unsigned short RTSPMessageSession::SendSetup(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;

	//generate setup message
	RTSPMessage::SetupMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);

	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);

	if (ret != usMessageLen)
		return 0;
	else
		return 1;
}

unsigned short RTSPMessageSession::SendPlay(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;

	//generate setup message
	RTSPMessage::PlayMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);

	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);

	if (ret != usMessageLen)
		return 0;

	else
		return 1;
}

unsigned short RTSPMessageSession::SendPause(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;

	//generate setup message
	RTSPMessage::PauseMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);

	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);

	if (ret != usMessageLen)
		return 0;
	else
		return 1;
}

unsigned short RTSPMessageSession::SendTeardown(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;

	//generate setup message
	RTSPMessage::TeardownMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);

	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);

	if (ret != usMessageLen)
		return 0;
	else
		return 1;
}

unsigned short RTSPMessageSession::SendAnnouce(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;
	
	//generate setup message
	RTSPMessage::AnnounceMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);
	
	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);
	
	if (ret != usMessageLen)
		return 0;
	else
		return 1;
}

unsigned short RTSPMessageSession::SendPing(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;

	//generate setup message
	RTSPMessage::PauseMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);

	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);

	if (ret != usMessageLen)
		return 0;
	else
		return 1;
}

unsigned short RTSPMessageSession::SendGetParameter(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;

	//generate setup message
	RTSPMessage::GetParameterMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);

	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);

	if (ret != usMessageLen)
		return 0;
	else
		return 1;
}

unsigned short RTSPMessageSession::SendSetParameter(RTSPClientSession &pRTSPClientSession)
{
	char pRTSPSendBuf[g_usRTSPSendBufMax];
	//memset(pRTSPSendBuf, 0, g_usRTSPSendBufMax);
	unsigned short usMessageLen = 0;

	//generate setup message
	RTSPMessage::SetParameterMessage(pRTSPClientSession, pRTSPSendBuf, g_usRTSPSendBufMax, usMessageLen);

	//try to send to server
	int16 ret = sTCPSend(pRTSPSendBuf, usMessageLen, *pRTSPClientSession.RTSPSocket);

	if (ret != usMessageLen)
		return 0;
	else
		return 1;
}