/*
* =====================================================================================
* 
*       Filename:  RTSPMessageLine.h
* 
*    Description:  定义创建NGOD RTSP消息某些特定header的方法
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

#ifndef __RTSPMESSAGELINE_H__
#define __RTSPMESSAGELINE_H__

#include "ZQ_common_conf.h"
#include "../RTSPHeader/RTSPHeader.h"

namespace RTSPMessageLine
{
	uint16 FirstLine(const char *pMethode,  string &pServerPath, uint16 uServerPort,  string &pSessionID, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 LastLine(char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 SequenceLine(uint16 uSeq, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 RequireLine(RTSPRequireHeader &pRequire, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 OnDemandSessionIdLine(string &pOnDemandSessionID, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 SessionGroupLine(RTSPSessionGroupHeader &pRTSPSessionGroupHeader, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 TransportLine(RTSPTransportHeader &pRTSPTransportHeader, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 SessionLine(string &pSessionID, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 UserAgentLine(string &pUserAgentID, char *fSendBuffer, const uint16 sBufferMaxSize);
};
#endif