#include "RTSP_MessageParser.h"

//try to change the string to lowercase 
::std::string &str2Lower(::std::string &str)
{
	//copy the string content to char
	char *tmpChar = new char[str.length() + 1];
	memcpy(tmpChar, str.data(), str.length());
	tmpChar[str.length()] = 0;

	//try to convert to lowercase
	//errno_t err = _strlwr_s(tmpChar, str.length() + 1);
	char *pLowerCase = strlwr(tmpChar);

	if (pLowerCase == NULL)
		::std::cerr << "convert string to lowercase error" << endl;

	str = ::std::string(pLowerCase);

	//release memory
	delete[] tmpChar;
	return str;
}

uint16 RTSPMessageParser::ParseSessionStatus(const char *pMessageBuf, uint16 usBufSize)
{
	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	::std::string strHeader("RTSP/1.0 ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == ::std::string::npos)
		return 0;
	else
		return atoi(strMessage.c_str() + index + strHeader.length());
}


uint16 RTSPMessageParser::GetContentLength(const char *pMessageBuf, uint16 usBufSize)
{
	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	::std::string strHeader("Content-Length: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == ::std::string::npos)
		return 0;
	else
		return atoi(strMessage.c_str() + index + strHeader.length());
}

uint16 RTSPMessageParser::GetOnDemandSessionID(const char *pMessageBuf, uint16 usBufSize, ::std::string &pOnDemandSessionID)
{
	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	::std::string strHeader("OnDemandSessionId: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == ::std::string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		char *pTail = strstr(pHead, "\r\n");
		pOnDemandSessionID = strMessage.substr(index + strHeader.length(), pTail - pHead);
		return 1;
	}
}

uint16 RTSPMessageParser::GetNptRange(const char *pMessageBuf, uint16 usBufSize, ::std::string &pRange, float &fTimeLen)
{
	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	static ::std::string strHeader("Range: npt=");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == ::std::string::npos)
		return 0;
	else
	{
		char *pHead = &(strMessage[index + strHeader.length()]);
		char *pTail = strstr(pHead, "-");
		if (pTail)
		{
			pRange = strMessage.substr(index + strHeader.length(), pTail - pHead);
			pHead = pTail + 1;
			index++;
			if (pHead[0] <= '9' && pHead[0] >= '0')//digital after - symbol
			{
				//TODO:get film length
				fTimeLen = atof(pHead);
			}
		}
		return 1;
	}
}

uint16 RTSPMessageParser::GetSessionID(const char *pMessageBuf, uint16 usBufSize, ::std::string &pSessionID)
{
	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	static ::std::string strHeader("Session: ");
	strHeader = str2Lower(strHeader);

	pSessionID.clear();

	size_t index = strMessage.find(strHeader);
	if (index == ::std::string::npos)
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
	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	static ::std::string strHeader("Scale: ");
	strHeader = str2Lower(strHeader);

	size_t index = strMessage.find(strHeader);
	if (index == ::std::string::npos)
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
	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	static ::std::string strHeader("CSeq: ");
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

bool RTSPMessageParser::CheckAnnounceMessage(const char *pMessageBuf, uint16 usBufSize)
{
	static ::std::string strNotice("announce ");

	::std::string strMessage(pMessageBuf, usBufSize);
	strMessage = str2Lower(strMessage);

	size_t index = strMessage.find(strNotice);
	if (index != ::std::string::npos)
		return true;
	else
		return false;
}

void RTSPMessageParser::splitMsg2Line(const char *pMessageBuf, uint16 usBufSize, ::std::vector<::std::string> &msgLine)
{
	::std::string strMessage(pMessageBuf, usBufSize);
	::std::string::size_type pos_begin = 0;
	::std::string::size_type pos_end = 0;
	
	while(1)
	{
		pos_end = strMessage.find_first_of("\r\n", pos_begin);

		if(::std::string::npos == pos_end) // not a valid macro reference
			break;

		::std::string line = strMessage.substr(pos_begin, pos_end - pos_begin);

		//get line
		msgLine.push_back(line);

		//move pos_begin to next line
		pos_begin = pos_end + 2;
	}
	
}