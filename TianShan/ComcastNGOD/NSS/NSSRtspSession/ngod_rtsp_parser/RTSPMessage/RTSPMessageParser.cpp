/*
* =====================================================================================
* 
*       Filename:  RTSPMessageParser.cpp
* 
*    Description:  实现解析NGOD RTSP消息对应的函数
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
string &str2Lower(string &str)
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

	string strHeader("Content-Length: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
		return atoi(strMessage.c_str() + index + strHeader.length());
}

uint16 RTSPMessageParser::GetOnDemandSessionID(const char *pMessageBuf, uint16 usBufSize, string &pOnDemandSessionID)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	string strHeader("OnDemandSessionId: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		char *pTail = strstr(pHead, "\r\n");
		pOnDemandSessionID = strMessage.substr(index + strHeader.length(), pTail - pHead);
		return 1;
	}
}

uint16 RTSPMessageParser::GetNptRange(const char *pMessageBuf, uint16 usBufSize, string &pRange)
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
		pRange = strMessage.substr(index + strHeader.length(), pTail - pHead);
		return 1;
	}
}

uint16 RTSPMessageParser::GetSessionID(const char *pMessageBuf, uint16 usBufSize, string &pSessionID)
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
		return 1;
	}
}


uint16 RTSPMessageParser::GetScale(const char *pMessageBuf, uint16 usBufSize, float &pScale)
{
	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	static string strHeader("Scale: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		pScale = atof(pHead);
		return 1;
	}
}

uint16 RTSPMessageParser::GetSequence(const char *pMessageBuf, uint16 usBufSize, uint16 &pCseq)
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


uint16 RTSPMessageParser::MessageParser(const char *pMessageBuf, uint16 usBufSize, RTSPClientSession &pRTSPClientSession)
{
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
		RTSPMessageParser::SDPParser(strSDPContent.c_str(), (uint16)strSDPContent.length(), pRTSPClientSession.m_RTSPR2Header.m_SDPResponseContent);
		RTSPMessageParser::GetParameterContentParser(strSDPContent.c_str(), (uint16)strSDPContent.length(), pRTSPClientSession.m_RTSPR2Header.m_GetPramameterRes_ExtHeader);
	}
	else
		strSDPContent.resize(0);

	RTSPMessageParser::GetOnDemandSessionID(strMessageBody.c_str(), (uint16)strMessageBody.length(), pRTSPClientSession.m_RTSPR2Header.strOnDemandSessionID);

	RTSPMessageParser::GetSessionID(strMessageBody.c_str(), (uint16)strMessageBody.length(), pRTSPClientSession.m_RTSPR2Header.strSessionID);

	float tmpScale = 0.0;
	if (RTSPMessageParser::GetScale(strMessageBody.c_str(), (uint16)strMessageBody.length(), tmpScale))
		pRTSPClientSession.fScale = tmpScale;

	if (RTSPMessageParser::GetNptRange(strMessageBody.c_str(), (uint16)strMessageBody.length(), pRTSPClientSession.strCurrentTimePoint) == 0)
		pRTSPClientSession.strCurrentTimePoint.clear();	

	return usSessionState;
}

uint16 RTSPMessageParser::SDPParser(const char *pMessageBuf, uint16 usBufSize, SDPResponseContent &pSDPResponseContent)
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

	size_t index = strMessage.find(strHeader);
	if (index != string::npos)
	{
		//get <protocol>
		char *pHead = &(strMessage[index + strHeader.length()]);
		char *pTail = strstr(pHead, strProtocol.c_str());
		if (pTail != NULL)
		{
			pSDPResponseContent.strProtocol = string(pHead).substr(0, pTail-pHead);
			pHead = (pTail + strProtocol.length());

			//get <host>
			pTail = strstr(pHead, strHost.c_str());
			if (pTail != NULL)
			{
				pSDPResponseContent.strHost = string(pHead).substr(0, pTail-pHead);
				pHead = (pTail + strHost.length());

				//get <port>
				if (pHead != NULL)
					pSDPResponseContent.uPort = atoi(pHead);

				//get streamhandle
				pHead = strstr(pHead, strPort.c_str());
				pHead += 1;
				pTail = strstr(pHead, strStreamHandle.c_str());
				if (pTail != NULL)
				{
					pSDPResponseContent.strStreamhandle = string(pHead).substr(0, pTail-pHead);
					pHead = (pTail + strStreamHandle.length());
				}
			}
			return 1;
		}
		else
			return 0;
	}
	else
		return 0;
}

uint16 RTSPMessageParser::GetParameterContentParser(const char *pMessageBuf, uint16 usBufSize, GetPramameterRes_ExtHeader &pGetPramameterRes_ExtHeader)
{
	//try to parser GET_PARAMETER response SDP Content
	static string strPresentation_state("presentation_state: ");
	static string strPosition("position: ");
	static string strScale("scale: ");
	static string strConnection_timeout("connection_timeout: ");
	static string strSession_list("session_list: ");
	static string strEndLine(" ");

	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	size_t index = 0;
	//try to get extension header except <session_list>
	for (int i = 0; i < 4; i++)
	{
		index = strMessage.find(string(pGetPramameter_ExtHeader[i]));
		if (index != string::npos)
		{
			int iHeaderIndex = int(index + strPresentation_state.length());
			char *pHead = &(strMessage[iHeaderIndex]);
			char *pTail = strstr(pHead, strEndLine.c_str());
			switch (i)
			{
				case presentation_state:
					pGetPramameterRes_ExtHeader.strPresentation_state = strMessage.substr(iHeaderIndex, pTail - pHead);
					break;
				case position:
					pGetPramameterRes_ExtHeader.strPosition = strMessage.substr(iHeaderIndex, pTail - pHead);
					break;
				case scale:
					pGetPramameterRes_ExtHeader.strScale = strMessage.substr(iHeaderIndex, pTail - pHead);
					break;
				case connection_timeout:
					pGetPramameterRes_ExtHeader.strConnection_timeout = strMessage.substr(iHeaderIndex, pTail - pHead);
					break;
				default:
					break;
			}
		}
	}

	index = strMessage.find(strSession_list);
	if (index != string::npos)
	{
			int iHeaderIndex = int(index + strSession_list.length());
			char *pHead = &(strMessage[iHeaderIndex]);
			char *pTail = NULL;
			while ((pTail = strstr(pHead, strEndLine.c_str())) != NULL)
			{
				string strSession = string(pHead).substr(0, pTail - pHead);
				string strColon(":");
				char *pColon = strstr(pHead, strColon.c_str());
				Session_list tmpstrSession_list;
				tmpstrSession_list.strRTSPSessionID = strSession.substr(0, strSession.find(strColon));
				tmpstrSession_list.strOnDemandSessionID = strSession.substr(strSession.find(strColon) + 1, strSession.length() - strSession.find(strColon) - 1);
				
				pHead = (pTail + strEndLine.length());
				pGetPramameterRes_ExtHeader.vstrSession_list.push_back(tmpstrSession_list);
			}
	}

	return 1;
}

bool RTSPMessageParser::CheckAnnounceMessage(const char *pMessageBuf, uint16 usBufSize)
{
	static string strNotice("announce");

	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	size_t index = strMessage.find(strNotice);
	if (index != string::npos)
		return true;
	else
		return false;
}

bool RTSPMessageParser::CheckGetParameterMessage(const char *pMessageBuf, uint16 usBufSize)
{
	static string strNotice("get_parameter");

	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	size_t index = strMessage.find(strNotice);
	if (index != string::npos)
		return true;
	else
		return false;
}

uint16 RTSPMessageParser::AnnouceNoticeParser(const char *pMessageBuf, uint16 usBufSize, RTSPNoticeHeader &mNoticeHeader)
{
	static string strNotice("notice: ");

	string strMessage = pMessageBuf;
	strMessage = str2Lower(strMessage);

	size_t index = strMessage.find(strNotice);
	if (index != string::npos)
	{
		char *pHead = &(strMessage[index + strNotice.length()]);
		char *pTail = strstr(pHead, " \"");
		if (pTail == NULL)
			return 0;
		else
		{
			mNoticeHeader.strNotice_code = string(pHead).substr(0, pTail - pHead);
			pHead = pTail + 2;
			pTail = strstr(pHead, "\" ");
			if (pTail == NULL)
				return 0;
			else
			{
				mNoticeHeader.strText_description = string(pHead).substr(0, pTail - pHead);
				pHead = pTail + 2;
				pTail = strstr(pHead, "event_date=");
				if (pTail == NULL)
					return 0;
				else
				{
					mNoticeHeader.strDatetime = string(pHead).substr(0, pTail - pHead);
					pHead = pTail + strlen("event_date=");
					pTail = strstr(pHead, "npt=");
					if (pTail == NULL)
						return 0;
					else
						mNoticeHeader.strNpt_value = string(pHead).substr(0, pTail - pHead);
				}
			}
		}
	}
	else
		return 0;

	return 1;
}