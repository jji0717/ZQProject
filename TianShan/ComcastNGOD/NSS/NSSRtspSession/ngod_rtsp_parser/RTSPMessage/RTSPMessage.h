/*
* =====================================================================================
* 
*       Filename:  RTSPMessage.h
* 
*    Description:  ���崴��NGOD RTSP��Ϣ�����ֿռ��Լ���Ӧ�ĺ���
*        Version:  1.0
*        Created:  March 18th, 2008
*       Revision:  
*       Compiler:  vs.net 2005
* 
*         Author:  Xiaoming Li
*        Company:  
* 
* =====================================================================================
*/

#ifndef __RTSPMESSAGE_H__
#define __RTSPMESSAGE_H__

#include "../RTSPHeader/RTSPHeader.h"

const int g_iSDPMaxLen = 10240;

namespace RTSPMessage  
{	
	//RTSP message creator
	unsigned short SetupMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short TeardownMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short AnnounceMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short GetParameterMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short SetParameterMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short PingMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short PauseMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short PlayMessage(RTSPClientSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);
};

#endif

