#ifndef __RTSP_MESSAGEPARSER_H__
#define __RTSP_MESSAGEPARSER_H__

#include "RTSP_common_structure.h"

namespace RTSPMessageParser
{
	//parse the RTSP response state
	uint16 ParseSessionStatus(const char *pMessageBuf, uint16 usBufSize);

	//get the Content-Length:
	uint16 GetContentLength(const char *pMessageBuf, uint16 usBufSize);

	//get the Response OnDemandSessionId: in order to find the specify client session
	uint16 GetOnDemandSessionID(const char *pMessageBuf, uint16 usBufSize, string &pOnDemandSessionID);

	//get the Range: npt=***-
	uint16 GetNptRange(const char *pMessageBuf, uint16 usBufSize, string &pRange, float &fTimeLen);

	//get the Response Session: in order to find the specify client session
	uint16 GetSessionID(const char *pMessageBuf, uint16 usBufSize, string &pSessionID);

	//get the Scale: *.**
	uint16 GetScale(const char *pMessageBuf, uint16 usBufSize, float &pScale);

	//get the Response CSeq: header
	uint16 GetSequence(const char *pMessageBuf, uint16 usBufSize, uint16 &pCseq);

	//RTSP response message body parser
	//uint16 MessageParser(const char *pMessageBuf, uint16 usBufSize, RTSPClientSession &pRTSPClientSession);

	//sdp response content parser
	//uint16 SDPParser(const char *pMessageBuf, uint16 usBufSize, SDPResponseContent &pSDPResponseContent);

	//uint16 GetParameterContentParser(const char *pMessageBuf, uint16 usBufSize, GetPramameterRes_ExtHeader &pGetPramameterRes_ExtHeader);

	bool   CheckAnnounceMessage(const char *pMessageBuf, uint16 usBufSize);

	//uint16 AnnouceNoticeParser(const char *pMessageBuf, uint16 usBufSize, RTSPNoticeHeader &mNoticeHeader);

	void	splitMsg2Line(const char *pMessageBuf, uint16 usBufSize, ::std::vector<::std::string> &msgLine);
};
#endif __RTSP_MESSAGEPARSER_H__