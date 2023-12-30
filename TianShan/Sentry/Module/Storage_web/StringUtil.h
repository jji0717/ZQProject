#ifndef __STRINGUTIL_H__
#define __STRINGUTIL_H__

#include <string>
#include <vector>

namespace StorageWeb
{
class StringUtil
{
public:
	StringUtil(void);
public:
	~StringUtil(void);
public:
	static std::string trimString(const std::string &s, const std::string &chs = " ");
    static std::string trimWS(const std::string &s);
	static void splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter);
};
}

#endif 
