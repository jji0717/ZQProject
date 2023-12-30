/*
* =====================================================================================
* 
*       Filename:  RTSPMessageSession.h
* 
*    Description:  定义NGOD RTSP交互过程中供调用的各类方法
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

#ifndef __RTSPMESSAGESESSION_H__
#define __RTSPMESSAGESESSION_H__

#include "RTSPMessage.h"

const unsigned short g_usRTSPSendBufMax = 10240;
namespace RTSPMessageSession
{
	unsigned short SendSetup(RTSPClientSession &pRTSPClientSession);

	unsigned short SendPlay(RTSPClientSession &pRTSPClientSession);

	unsigned short SendPause(RTSPClientSession &pRTSPClientSession);

	unsigned short SendTeardown(RTSPClientSession &pRTSPClientSession);

	unsigned short SendAnnouce(RTSPClientSession &pRTSPClientSession);

	unsigned short SendPing(RTSPClientSession &pRTSPClientSession);

	unsigned short SendGetParameter(RTSPClientSession &pRTSPClientSession);

	unsigned short SendSetParameter(RTSPClientSession &pRTSPClientSession);
};

#endif