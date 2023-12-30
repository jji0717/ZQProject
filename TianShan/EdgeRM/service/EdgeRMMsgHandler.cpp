#include "EdgeRMMsgHandler.h"
#include <algorithm>

namespace ZQTianShan {
namespace EdgeRM {

std::string StringUtil::trimString(const std::string &s, const std::string &chs)
{
	if (0 == s.length()) 
	{
		return s;
	}
	std::string::size_type pos_beg = std::string::npos;
	std::string::size_type pos_end = std::string::npos;
	pos_beg = s.find_first_not_of(chs);
	if(std::string::npos == pos_beg)
		return "";
	pos_end = s.find_last_not_of(chs);
	return s.substr(pos_beg, pos_end - pos_beg + 1);
}

std::string StringUtil::trimWS(const std::string &s)
{
	return trimString(s, " \f\n\r\t\v");
}

void StringUtil::splitString(std::vector<std::string> &result, const std::string &str, 
	const std::string &delimiter)
{
	using namespace std;
	result.clear();
	string::size_type pos_from = 0;
	while((pos_from = str.find_first_not_of(delimiter, pos_from)) != string::npos)
	{
		string::size_type pos_to = str.find_first_of(delimiter, pos_from);
		if(pos_to != string::npos)
		{
			result.push_back(str.substr(pos_from, pos_to - pos_from));
		}
		else
		{
			result.push_back(str.substr(pos_from));
			break;
		}
		pos_from = pos_to;
	}
}

//-------------------------------------------------------------------------------------------------------
bool RtspMsgHandler::HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg)
{
	return false;
}

void RtspMsgHandler::getTransportElements(const std::string& strTransport, std::vector<std::string> &elements)
{
	size_t nStart = 0;
	size_t nPos = strTransport.find(",", nStart);
	while (nPos != std::string::npos)
	{
		elements.push_back(strTransport.substr(nStart, nPos - nStart));
		nStart = nPos + 1;
		nPos = strTransport.find(",", nStart);
	}
	elements.push_back(strTransport.substr(nStart));
}

void RtspMsgHandler::getTransportElementProperties(const std::string &element, 
												   std::map<std::string,std::string> &properties)
{
	std::vector<std::string> params;
	StringUtil::splitString(params, element, ";");
	for (int i = 0; i < params.size(); i++)
	{
		std::vector<std::string> param;
		StringUtil::splitString(param, params[i], "=");
		std::transform(param.begin(), param.end(), param.begin(), StringUtil::trimWS); // erase empty space
		switch(param.size())
		{
		case 1:
			properties.insert(std::make_pair(param[0], param[0]));
			break;
		case 2:
			properties.insert(std::make_pair(param[0], param[1]));
			break;
		default:
			break;
		}
	}
}

} // end for EdgeRM
} // end for ZQTianShan
