#ifndef __DODContentStoreCfgLoader_H__
#define __DODContentStoreCfgLoader_H__
#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include <list>
#include <map>
#include <string>
#include <vector>
using namespace ZQ::common;

struct DODContentStoreCfg
{
	std::string      dbPath;
	std::string      logPath;
	std::string      endpoint;
	std::string      netId;

	int32			 maxSessionCount;          // Graph count in GraphPool
	int32			 buffPoolSize;             // Buffer Pool size in each Graph
	int32			 buffSize;                 // Buffer size of Graph's buff
	int32			 traceProvDetails;         // boolean, trace provision details
	int32			 progressReportInterval;   // progress report interval in seconds
	std::string      homeDirectory;			   // the Home directory for output of wrapped ts files
	std::string      desURLPrefix;			   // the prefix of destination URL
	int32			 yieldTime;                // yield time for datawrapping

	std::string      logFile;
	int32			 logLevel;
	int32			 logBuffer;
	int32			 logSize;
	int32			 logTimeout;

	// stores the ice properties
	std::map<std::string, std::string> icePropMap;

	static void structure(Config::Holder<DODContentStoreCfg> &holder)
	{
		holder.addDetail("Default/DatabaseFolder", "path", &DODContentStoreCfg::dbPath, "", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Bind", "endpoint", &DODContentStoreCfg::endpoint, "", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Host", "netId", &DODContentStoreCfg::netId, "ADMIN2_SS_CL", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "maxSessionCount", &DODContentStoreCfg::maxSessionCount, "5", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "buffPoolSize", &DODContentStoreCfg::buffPoolSize, "52", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "buffSizeInPool", &DODContentStoreCfg::buffSize, "65536", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "traceProvDetails", &DODContentStoreCfg::traceProvDetails, "1", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "progressReportInterval", &DODContentStoreCfg::progressReportInterval, "5", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "homeDirectory", &DODContentStoreCfg::homeDirectory,NULL, Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "desURLPrefix", &DODContentStoreCfg::desURLPrefix, "", Config::optReadOnly);	
		holder.addDetail("DataContentStore/Provision", "yieldTime", &DODContentStoreCfg::yieldTime, "0", Config::optReadOnly);	

		holder.addDetail("DataContentStore/IceProperties/prop", &DODContentStoreCfg::readProp, &DODContentStoreCfg::registerNothing);


	};
	void readProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		icePropMap[nvholder.name] = nvholder.value;
	}

	void registerNothing(const std::string&){}
};

static void showCfg(const DODContentStoreCfg& cfg)
{
	using namespace std;
	cout << "DODContentStore CfgLoader: \n";

}
#endif //__DODContentStoreCfgLoader_H__