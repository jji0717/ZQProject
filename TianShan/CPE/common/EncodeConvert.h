

#ifndef _CPE_ENCODE_CONVERT_MGR
#define _CPE_ENCODE_CONVERT_MGR

#include "ZQ_common_conf.h"
#include <string>
#include <memory>

class EncodeConvert
{
public:
	static bool unicode_to_utf8(const std::wstring& unicode, std::string& utf8);
	static bool utf8_to_unicode(const std::string& utf8, std::wstring& unicode);
	static bool unicode_to_ansi(const std::wstring& unicode, std::string& ansi);
	static bool utf8_to_ansi(const std::string& utf8, std::string& ansi);
	static bool ansi_to_utf8(const std::string& ansi, std::string& utf8);
	static bool ansi_to_unicode(const std::string& ansi, std::wstring& unicode);
};


#endif

