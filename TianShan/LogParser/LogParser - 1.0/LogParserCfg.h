#ifndef _LOG_PARSER_CONFIGURATION_HEADER_FILE_H__
#define _LOG_PARSER_CONFIGURATION_HEADER_FILE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ConfigHelper.h>
#include <string>
#include <vector>

struct Module
{
	std::string dll;
	std::string config;
	std::string type;
	int32 enable;
	static void structure(ZQ::common::Config::Holder< Module > &holder)
	{
		using namespace ZQ::common;
		holder.addDetail("", "Dll", &Module::dll, NULL,Config::optReadOnly);
		holder.addDetail("", "Config", &Module::config, NULL, Config::optReadOnly);
		holder.addDetail("", "Type", &Module::type, NULL, Config::optReadOnly);
		holder.addDetail("", "Enable", &Module::enable, NULL,Config::optReadOnly);
	}
};
struct MonitoringLog
{
	std::string path;
	std::string type;
	std::string syntax;
	std::string key;
	std::map<std::string, std::string> ctx;

	static void structure(ZQ::common::Config::Holder< MonitoringLog > &holder)
	{
		holder.addDetail("", "path", &MonitoringLog::path);
		holder.addDetail("", "type", &MonitoringLog::type);
		holder.addDetail("", "syntax", &MonitoringLog::syntax);
		holder.addDetail("", "key", &MonitoringLog::key);

		holder.addDetail("property", &MonitoringLog::readProperty, &MonitoringLog::registerNothing);
	}

	void readProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		using namespace ZQ::common;
		Config::Holder<Config::NVPair> prop("");
		prop.read(node, hPP);
		ctx[prop.name] = prop.value;
	}
	void registerNothing(const std::string &){};
};
struct LogParserCfg
{
	std::string logPath;
	int32	logSize;
	int32   logLevel;
	int32   logCount;
	std::string posDbPath;
	int32 posDbEvictorSize;
	typedef std::vector< ZQ::common::Config::Holder< Module > > Modules;
	Modules modules;

	std::map<std::string, std::string> contextProps;

	std::vector<MonitoringLog> initialMonitoringLogs;

	static void structure(ZQ::common::Config::Holder< LogParserCfg > &holder)
	{
		using namespace ZQ::common;
		holder.addDetail("EventSink/Log", "LogPath", &LogParserCfg::logPath, NULL, Config::optReadOnly);
		holder.addDetail("EventSink/Log", "LogFileSize", &LogParserCfg::logSize, NULL, Config::optReadOnly);
		holder.addDetail("EventSink/Log", "LogLevel", &LogParserCfg::logLevel, NULL, Config::optReadOnly);
		holder.addDetail("EventSink/Log", "count", &LogParserCfg::logCount, "5", Config::optReadOnly);
		holder.addDetail("EventSink/PosDataBase", "Path", &LogParserCfg::posDbPath, NULL);
		holder.addDetail("EventSink/PosDataBase", "evictorSize", &LogParserCfg::posDbEvictorSize, "5");

		holder.addDetail("EventSink/PlugIn/Module", &LogParserCfg::readModule, &LogParserCfg::registerModules);
		holder.addDetail("EventSink/Context/property", &LogParserCfg::readContextProperty, &LogParserCfg::registerNothing);

		holder.addDetail("EventSink/InitialMonitoring/Log", &LogParserCfg::readInitalMonitoringLog, &LogParserCfg::registerNothing);
	}
	void readModule(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		using namespace ZQ::common;
		Config::Holder<Module> moduleholder("");
		moduleholder.read(node, hPP);
		modules.push_back(moduleholder);
	}
	void registerModules(const std::string &full_path)
	{
		for (Modules::iterator it = modules.begin(); it != modules.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	void readContextProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		using namespace ZQ::common;
		Config::Holder<Config::NVPair> prop;
		prop.read(node, hPP);
		contextProps[prop.name] = prop.value;
	}

	void readInitalMonitoringLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		using namespace ZQ::common;
		Config::Holder<MonitoringLog> mLog;
		mLog.read(node, hPP);
		initialMonitoringLogs.push_back(mLog);
	}

	void registerNothing(const std::string &){};
};

typedef  ZQ::common::Config::Loader< LogParserCfg > LogParserConfig;

#endif // !defined(AFX_EVENTSINKCFG_H__503CF03E_5426_4172_8E22_BF0515CD23D9__INCLUDED_)
