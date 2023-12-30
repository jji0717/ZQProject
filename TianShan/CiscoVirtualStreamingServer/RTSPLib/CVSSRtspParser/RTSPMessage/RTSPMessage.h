/*
* =====================================================================================
* 
*       Filename:  RTSPMessage.h
* 
*    Description:  定义创建NGOD RTSP消息的名字空间以及对应的函数
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

#include "../CVSSRtspSession.h"

#define FLOATTHRESWHOLD 0.00001

const int g_iSDPMaxLen = 10240;

namespace RTSPMessage  
{	
	//RTSP message creator
	unsigned short SetupMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short TeardownMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short AnnounceMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short GetParameterMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short SetParameterMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short PingMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short PauseMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);

	unsigned short PlayMessage(CVSSRtspSession &rtspClient, char *fSendBuffer, const unsigned short sBufferMaxSize, unsigned short &fSendBufferSize);
};

#endif

