/*
* =====================================================================================
* 
*       Filename:  RTSPMessageParser.cpp
* 
*    Description:  实现解析RTSP消息对应的函数
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

#include "RTSPMessageParser.h"

//try to change the string to lowercase 
::std::string &str2Lower(string &str)
{
	//copy the string content to char
	char *tmpChar = new char[str.length() + 1];
	memcpy(tmpChar, str.data(), str.length());
	tmpChar[str.length()] = 0;

	//try to convert to lowercase
	//errno_t err = _strlwr_s(tmpChar, str.length() + 1);
	char *pLowerCase = strlwr(tmpChar);

	if (pLowerCase == NULL)
	{
		cerr << "convert string to lowercase error" << endl;
		delete[] tmpChar;
		return str;
	}

	str = string(pLowerCase);

	//release memory
	delete[] tmpChar;
	return str;
}

bool skipBlank(char *str, uint16 usMaxSize)
{
	for (int32 i = 0; i < usMaxSize; i++)
		if (str[i] != ' ')
			return true;

	//reach the end of string
	return false;
}

uint16 RTSPMessageParser::ParseSessionStatus(const char *pMessageBuf, uint16 usBufSize)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	string strHeader("RTSP/1.0 ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
		return atoi(strMessage.c_str() + index + strHeader.length());
}


uint16 RTSPMessageParser::GetContentLength(const char *pMessageBuf, uint16 usBufSize)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	string strHeader("Content-Length:");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
	{
		char *pHead =  &(strMessage[index + strHeader.length()]);
		if (skipBlank(pHead, strMessage.length() - index - strHeader.length()))
			return atoi(strMessage.c_str() + index + strHeader.length());
		else
			return 0;
	}
}

uint16 RTSPMessageParser::GetNptRange(const char *pMessageBuf, uint16 usBufSize, ::std::string &pRangeStart, ::std::string &pRangeEnd)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	static string strHeader("Range: npt=");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		char *pTail = strstr(pHead, "-");
		pRangeStart = strMessage.substr(index + strHeader.length(), pTail - pHead);
		pHead = pTail + 1;
		pTail = strstr(pHead, "\r\n");
		if (pTail - pHead > 0)
			pRangeEnd = strMessage.substr(index + strHeader.length() + pRangeStart.length() + 1, pTail - pHead);
		return 1;
	}
}

uint16 RTSPMessageParser::GetSessionID(const char *pMessageBuf, uint16 usBufSize, ::std::string &pSessionID,int32 iTimeOut)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	static string strHeader("Session: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		char *pTail = strstr(pHead, "\r\n");
		pSessionID = strMessage.substr(index + strHeader.length(), pTail - pHead);
		const char *colon = strstr(pSessionID.c_str(), ";");
		if (colon != NULL)
		{
			//get time out
			iTimeOut = atoi(colon + 1);
			pSessionID = strMessage.substr(index + strHeader.length(), colon - pHead);
		}
		return 1;
	}
}


uint16 RTSPMessageParser::GetScale(const char *pMessageBuf, uint16 usBufSize, ::std::string &pScale)
{
	pScale.clear();

	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	static ::std::string strHeader("Scale: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == ::std::string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		char *pTail = strstr(pHead, "\r\n");
		if (pTail == NULL)
			return 0;
		else
			pScale = ::std::string(pHead, pTail - pHead);
		return 1;
	}
}

uint16 RTSPMessageParser::GetSequence(const char *pMessageBuf, uint16 usBufSize, uint32 &pCseq)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	static string strHeader("CSeq: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		pCseq = atoi(pHead);
		return 1;
	}
}


uint16 RTSPMessageParser::MessageParser(const char *pMessageBuf, uint16 usBufSize, CVSSRtspSession &pRTSPClientSession)
{
	RTSPClientState state = CheckMessageType(pMessageBuf, usBufSize);
	uint16 usSessionState = ParseSessionStatus(pMessageBuf, usBufSize);

	//if (usSessionState != 200)
	//	return usSessionState;

	uint16 usContentLength = GetContentLength(pMessageBuf, usBufSize);

	//split the message body and sdp content
	string strMessageBody = string(pMessageBuf).substr(0, usBufSize - usContentLength);
	//strMessageBody.resize(usBufSize - usContentLength);
	string strSDPContent;
	if (usContentLength > 0)
	{
		strSDPContent = string(pMessageBuf).substr(strMessageBody.length(), usContentLength);
		RTSPMessageParser::SDPParser(strSDPContent.c_str(), (uint16)strSDPContent.length(), pRTSPClientSession, state);
	}
	else
		strSDPContent.resize(0);

	RTSPMessageParser::GetSessionID(strMessageBody.c_str(), (uint16)strMessageBody.length(), pRTSPClientSession._commonResHeader._strSessionId, pRTSPClientSession._commonResHeader._iTimeOut);

	RTSPMessageParser::GetScale(strMessageBody.c_str(), (uint16)strMessageBody.length(), pRTSPClientSession._commonResHeader._strScale);

	RTSPMessageParser::GetNptRange(strMessageBody.c_str(), (uint16)strMessageBody.length(), pRTSPClientSession._commonResHeader._strRangeStart, pRTSPClientSession._commonResHeader._strRangeEnd);

	return usSessionState;
}

uint16 RTSPMessageParser::SDPParser(const char *pMessageBuf, uint16 usBufSize, CVSSRtspSession &pRTSPClientSession, RTSPClientState messageType)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	//try to parser a=control header
	static string strHeader("a=control:");
	static string strProtocol("://");
	static string strHost(":");
	static string strPort("/");
	static string strStreamHandle("\r\n");

	strHeader = str2Lower(strHeader);

	return 0;
}

RTSPClientState RTSPMessageParser::CheckMessageType(const char *pMessageBuf, uint16 usBufSize)
{
	static ::std::string strSetup("setup");
	static ::std::string strTeardown("teardown");
	static ::std::string strPlay("play");
	static ::std::string strPause("pause");
	static ::std::string strGetParameter("get_parameter");
	static ::std::string strAnnounce("anounce");


	::std::string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	if (strMessage.find(strSetup) != ::std::string::npos)
		return SETUP;
	else if (strMessage.find(strTeardown) != ::std::string::npos)
		return TEARDOWN;
	else if (strMessage.find(strPlay) != ::std::string::npos)
		return PLAY;
	else if (strMessage.find(strPause) != ::std::string::npos)
		return PAUSE;
	else if (strMessage.find(strGetParameter) != ::std::string::npos)
		return GETPARAMETER;
	else if (strMessage.find(strAnnounce) != ::std::string::npos)
		return ANNOUNCE;
	else
		return UNKNOWNSTATE;
}

uint16 RTSPMessageParser::AnnouceNoticeParser(const char *pMessageBuf, uint16 usBufSize,AnnounceReqHeader &mAnnounceReqHeader)
{
	static string strNotice("x-notice: ");

	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	size_t index = strMessage.find(strNotice);
	if (index != string::npos)
	{
		char *pHead = &(strMessage[index + strNotice.length()]);
		char *pTail = strstr(pHead, " ");
		if (pTail == NULL)
			return 0;
		else
		{
			mAnnounceReqHeader._strEventCode = string(pHead).substr(0, pTail - pHead);
			pHead = pTail + 1;
			pTail = strstr(pHead, " ");
			if (pTail == NULL)
				return 0;
			else
			{
				mAnnounceReqHeader._strEventPhrase = string(pHead).substr(0, pTail - pHead);
				pHead = pTail + 1;
				pHead = strstr(pHead, "event_date=");
				pHead += ::std::string("event_data=").length();
				pTail = strstr(pHead, "\r\n");
				if (pTail == NULL)
					return 0;
				else
				{
					mAnnounceReqHeader._strEventDate = string(pHead).substr(0, pTail - pHead);
				}
			}
		}
	}
	else
		return 0;

	return 1;
}

void RTSPMessageParser::MessageSplit(const char *pMessageBuf, uint16 usBufferSize, strlist &msgList)
{
	static ::std::string strLine("\r\n\r\n");
	::std::string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);
	
	::std::string::size_type begin_of_msg = 0;
	::std::string::size_type end_of_msg = ::std::string::npos;
	const char *head = NULL;

	while ((head = strstr(strMessage.c_str() + begin_of_msg, strLine.c_str())) != NULL)
	{
		end_of_msg = head - strMessage.c_str();
		::std::string msg = strMessage.substr(begin_of_msg, end_of_msg + strLine.length() - begin_of_msg);
		begin_of_msg = end_of_msg + strLine.length();

		//get content length, if len>0, get content
		uint16 contentLen = GetContentLength(msg.c_str(), msg.length());
		if (contentLen > 0)
		{
			msg += strMessage.substr(begin_of_msg, contentLen);
			begin_of_msg += contentLen;
		}

		msgList.push_back(msg);
	}
}