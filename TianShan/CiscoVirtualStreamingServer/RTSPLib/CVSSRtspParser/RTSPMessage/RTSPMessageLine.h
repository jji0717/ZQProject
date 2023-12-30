/*
* =====================================================================================
* 
*       Filename:  RTSPMessageLine.h
* 
*    Description:  定义创建RTSP消息某些特定header的方法
*        Version:  1.0
*        Created:  Dec. 9th, 2008
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
#include "../CVSSRtspSession.h"

namespace RTSPMessageLine
{
	uint16 FirstLine(const char *pMethode,  ::std::string &pServerPath, uint16 uServerPort,  ::std::string &pContentID, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 LastLine(char *fSendBuffer);

	uint16 SequenceLine(uint32 uSeq, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 TransportLine(SetupReqHeader &pSetupReqHeader, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 SessionLine(::std::string &pSessionID, char *fSendBuffer, const uint16 sBufferMaxSize);

	uint16 UserAgentLine(::std::string &pUserAgentID, char *fSendBuffer, const uint16 sBufferMaxSize);
};
#endif