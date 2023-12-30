/*
* =====================================================================================
* 
*       Filename:  RTSPMessageParser.h
* 
*    Description:  �������NGOD RTSP��Ϣ�����ֿռ��Լ���Ӧ�ĺ���
*				   ��������Ϣ��Ľ����Ͷ�SDP���ֵĽ���

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
#include "../RTSPHeader/RTSPHeader.h"

/*namespace MessageHeader
{
	static string strOnDemandSessionID("OnDemandSessionId: ");
	static string strSession("Session: ");
	static string strControl("a=control:");
	static string strContentLen("Content-Length: ");
	static string strFirstLine("RTSP/1.0 ");

};*/

namespace RTSPMessageParser
{
	//parse the RTSP response state
	uint16 ParseSessionStatus(const char *pMessageBuf, uint16 usBufSize);

	//get the Content-Length:
	uint16 GetContentLength(const char *pMessageBuf, uint16 usBufSize);

	//get the Response OnDemandSessionId: in order to find the specify client session
	uint16 GetOnDemandSessionID(const char *pMessageBuf, uint16 usBufSize, string &pOnDemandSessionID);

	//get the Range: npt=***-
	uint16 GetNptRange(const char *pMessageBuf, uint16 usBufSize, string &pRange);

	//get the Response Session: in order to find the specify client session
	uint16 GetSessionID(const char *pMessageBuf, uint16 usBufSize, string &pSessionID);

	//get the Scale: *.**
	uint16 GetScale(const char *pMessageBuf, uint16 usBufSize, float &pScale);

	//get the Response CSeq: header
	uint16 GetSequence(const char *pMessageBuf, uint16 usBufSize, uint16 &pCseq);

	//RTSP response message body parser
	uint16 MessageParser(const char *pMessageBuf, uint16 usBufSize, RTSPClientSession &pRTSPClientSession);

	//sdp response content parser
	uint16 SDPParser(const char *pMessageBuf, uint16 usBufSize, SDPResponseContent &pSDPResponseContent);

	uint16 GetParameterContentParser(const char *pMessageBuf, uint16 usBufSize, GetPramameterRes_ExtHeader &pGetPramameterRes_ExtHeader);
	
	bool CheckAnnounceMessage(const char *pMessageBuf, uint16 usBufSize);

	bool CheckGetParameterMessage(const char *pMessageBuf, uint16 usBufSize);

	uint16 AnnouceNoticeParser(const char *pMessageBuf, uint16 usBufSize, RTSPNoticeHeader &mNoticeHeader); 
};

#endif