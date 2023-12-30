/*
* =====================================================================================
* 
*       Filename:  RTSPMessageParser.h
* 
*    Description:  定义解析NGOD RTSP消息的名字空间以及对应的函数
*				   包括对消息体的解析和对SDP部分的解析

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

#ifndef _RTSPMESSAGEPARSER_H_
#define _RTSPMESSAGEPARSER_H_

#include "ZQ_common_conf.h"
#include "../CVSSRtspSession.h"

namespace RTSPMessageParser
{
	//parse the RTSP response state
	uint16 ParseSessionStatus(const char *pMessageBuf, uint16 usBufSize);

	//get the Content-Length:
	uint16 GetContentLength(const char *pMessageBuf, uint16 usBufSize);

	//get the Range: npt=***-***
	uint16 GetNptRange(const char *pMessageBuf, uint16 usBufSize, ::std::string &pRangeStart, ::std::string &pRangeEnd);

	//get the Response Session: in order to find the specify client session
	uint16 GetSessionID(const char *pMessageBuf, uint16 usBufSize, ::std::string &pSessionID, int32 iTimeOut);

	//get the Scale: *.**
	uint16 GetScale(const char *pMessageBuf, uint16 usBufSize, ::std::string &pScale);

	//get the Response CSeq: header
	uint16 GetSequence(const char *pMessageBuf, uint16 usBufSize, uint32 &pCseq);

	//RTSP response message body parser
	uint16 MessageParser(const char *pMessageBuf, uint16 usBufSize, CVSSRtspSession &pRTSPClientSession);

	//sdp response content parser
	uint16 SDPParser(const char *pMessageBuf, uint16 usBufSize, CVSSRtspSession &pRTSPClientSession, RTSPClientState messageType);
	
	RTSPClientState CheckMessageType(const char *pMessageBuf, uint16 usBufSize);

	uint16 AnnouceNoticeParser(const char *pMessageBuf, uint16 usBufSize, AnnounceReqHeader &mAnnounceReqHeader); 

	void MessageSplit(const char *pMessageBuf, uint16 usBufferSize, strlist &msgList);
};

#endif