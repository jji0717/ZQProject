#include "StringUtil.h"

using StorageWeb::StringUtil;

StringUtil::StringUtil(void)
{
}

StringUtil::~StringUtil(void)
{
}

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

void StringUtil::splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter)
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