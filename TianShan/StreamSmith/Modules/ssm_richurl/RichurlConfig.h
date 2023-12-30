#ifndef _RICH_URL_CONFIG_H
#define _RICH_URL_CONFIG_H

#include <ZQ_common_conf.h>
#include <string>
#include "ConfigHelper.h"
#include "FileLog.h"

struct LogFile 
{
	std::string   _fileName;
	int  _fileSize;
	int  _fileLogLevel;
	int  _fileNumber;

	static void structure(::ZQ::common::Config::Holder<LogFile> &holder)
	{
		holder.addDetail("", "name",   &LogFile::_fileName,     NULL,       ZQ::common::Config::optReadOnly);
		holder.addDetail("", "size",   &LogFile::_fileSize,     "50000000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "level",  &LogFile::_fileLogLevel, "6",        ZQ::common::Config::optReadOnly);
		holder.addDetail("", "number", &LogFile::_fileNumber,   "10",       ZQ::common::Config::optReadOnly);
	}
};

struct UserAgent 
{
	std::string _keyword;
	static void structure(::ZQ::common::Config::Holder<UserAgent> &holder)
	{
		holder.addDetail("", "keyword", &UserAgent::_keyword, NULL, ZQ::common::Config::optReadOnly);
	}
};

// config loader structure
struct RichurlCfg
{
    struct LogFile    _logFile;
	struct UserAgent  _userAgent;

	static void structure(ZQ::common::Config::Holder<RichurlCfg> &holder)
	{
		holder.addDetail("ssm_richurl/LogFile",   &RichurlCfg::readLogFile,   &RichurlCfg::registerNothing);
		holder.addDetail("ssm_richurl/UserAgent", &RichurlCfg::readUserAgent, &RichurlCfg::registerNothing);
	}
	void readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<struct LogFile> logFileholder("");
		logFileholder.read(node, hPP);
		_logFile = logFileholder;
	}

	void readUserAgent(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<struct UserAgent> UAholder("");
		UAholder.read(node, hPP);
		_userAgent = UAholder;
	}

	void registerNothing(const std::string&){}
};

#endif//_RICH_URL_CONFIG_H