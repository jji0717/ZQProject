#include <vector>
#include "strHelper.h"
#include "TelnetParser.h"

namespace ZQ{

namespace common{

TelnetParser::TelnetParser(ZQ::common::Log *ZQLog)
:_log(ZQLog)
{
}
TelnetParser::~TelnetParser()
{
	_log = NULL;
}

bool TelnetParser::getContentByHeader(const std::string &msg, const char *header, std::string &res)
{
	if (header == NULL)
	{
		TELNETLOG(ZQ::common::Log::L_WARNING, CLOGFMT(TelnetParser, "input header is NULL"));
		return false;
	}

	std::string strRet;
	size_t sPos = msg.find(header);
	if (sPos == std::string::npos)
	{
		TELNETLOG(ZQ::common::Log::L_WARNING, CLOGFMT(TelnetParser, "header(%s): can't find specified header"), header);
		return false;
	}

	size_t ePos = msg.find(PLEOF, sPos);
	if (sPos == std::string::npos)
	{
		TELNETLOG(ZQ::common::Log::L_WARNING, CLOGFMT(TelnetParser, "header(%s): can't find EOF"), header);
		return false;
	}

	res = msg.substr(sPos + strlen(header), ePos - sPos - strlen(header));
	res.erase(0, res.find_first_not_of(" "));
	res.erase(res.find_last_not_of(" ") + 1);
	TELNETLOG(ZQ::common::Log::L_INFO, CLOGFMT(TelnetParser, "header(%s): find header content(%s)"), header, res.c_str());
	return true;
}

bool TelnetParser::checkError(const std::string &msg)
{
	if (msg.find(PLUnknownCMD) != std::string::npos)
	{
		TELNETLOG(ZQ::common::Log::L_ERROR, CLOGFMT(TelnetParser, "receive unknow command error, message:%s"), msg.c_str());
		return false;
	}
	else if (msg.find(PLUnknown) != std::string::npos)
	{
		TELNETLOG(ZQ::common::Log::L_ERROR, CLOGFMT(TelnetParser, "receive unknow parameter error, message:%s"), msg.c_str());
		return false;
	}
	else if (msg.find(PLWrongCMD) != std::string::npos)
	{
		TELNETLOG(ZQ::common::Log::L_ERROR, CLOGFMT(TelnetParser, "receive wrong command syntax error, message:%s"), msg.c_str());
		return false;
	}
	else
		return true;
}

bool TelnetParser::checkPLName(const std::string &msg, const char *plName)
{
	if (msg.find(plName) == std::string::npos)
	{
		TELNETLOG(ZQ::common::Log::L_ERROR, CLOGFMT(TelnetParser, "faile to get the playlist(%s) name in the response msg"), plName);
		return false;
	}
	else
		return true;
}

bool TelnetParser::getList(const std::string &msg, std::list<std :: string> &lst, const std::string strListName)
{
	// clear list first;
	lst.clear();

	// check if valid list name 
	if (strListName != "media" && strListName != "schedule")
	{
		TELNETLOG(ZQ::common::Log::L_WARNING, CLOGFMT(TelnetParser, "Invalid list name [%s]"), strListName.c_str());
		return false;
	}

	// check if right message format
	size_t nStart = msg.find("show"); 
	if (std::string::npos == nStart || std::string::npos == msg.find(strListName))
	{
		TELNETLOG(ZQ::common::Log::L_WARNING, CLOGFMT(TelnetParser, "It can't find show command or list name"));
		return false;
	}

	// parse message into lines from "show" line
	std::vector<std::string> vecLines;
	std::string strLine;
	size_t nPos = 0;
	while(nStart < msg.size())
	{
		nPos = msg.find(PLEOF, nStart);
		if (nPos == std::string::npos)
		{
			TELNETLOG(ZQ::common::Log::L_WARNING, CLOGFMT(TelnetParser, "It can't find EOF"));
			break;
			//return false;
		}
		strLine = msg.substr(nStart, nPos - nStart);
		vecLines.push_back(strLine);
		nStart = nPos + strlen(PLEOF);
	}
	size_t nNumOfLines = vecLines.size();

	// list is empty
	if (nNumOfLines < 3)
	{
		TELNETLOG(ZQ::common::Log::L_INFO, CLOGFMT(TelnetParser, "%s list are empty"), strListName.c_str());
		return true;
	}
	// list standard
	size_t nListStandard = vecLines[1].find_first_not_of("\t "); 

	// find list item standard
	size_t i = 2;
	size_t nLine = vecLines[i].find_first_not_of("\t ");
	while (nLine <= nListStandard && ++i < nNumOfLines)
	{
		nLine = vecLines[i].find_first_not_of("\t ");
	}
	if (i >= nNumOfLines)
	{
		TELNETLOG(ZQ::common::Log::L_INFO, CLOGFMT(TelnetParser, "Play list and schedule list are empty"));
		return true;
	}
	size_t nListItemStandard = nLine;

	if ("media" == strListName)
	{
		if (i != 2)
		{
			TELNETLOG(ZQ::common::Log::L_INFO, CLOGFMT(TelnetParser, "Play list is empty"));
			return true;
		}
		while (nLine != nListStandard && i < nNumOfLines) // i == 2 
		{
			nLine = vecLines[i].find_first_not_of("\t ");
			if (nLine == nListItemStandard)
			{
				strLine = vecLines[i];
				stringHelper::TrimExtra(strLine);
				lst.push_back(strLine);
			}
			i++;
		}
	}
	else
	{
		if (std::string::npos != msg.find("media"))
		{
			i = 2;
			while (nLine != nListStandard && i < nNumOfLines)
			{
				nLine = vecLines[i].find_first_not_of("\t ");
				i++;
			}

		}
		while (i < nNumOfLines)
		{
			nLine = vecLines[i].find_first_not_of("\t ");
			if (nLine == nListItemStandard)
			{
				strLine = vecLines[i];
				stringHelper::TrimExtra(strLine);
				lst.push_back(strLine);
			}
			i++;
		}
	}
	return true;
}

}//namespace common

}//namespace ZQ

