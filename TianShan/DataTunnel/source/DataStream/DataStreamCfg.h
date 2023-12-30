#ifndef __DataStreamCfgLoader_H__
#define __DataStreamCfgLoader_H__
#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include <list>
#include <map>
#include <string>
#include <vector>
using namespace ZQ::common;

struct DataStreamCfg
{
	std::string             endpoint;
//	std::string				dbPath;
	std::string				netId;
	int32					checkStreamTimeout;
	int32					totalRate;
	int32					higherPriority;

    int32					readerThreadPoolMinSize;
	int32       			readerThreadPoolMaxSize;
	int32					senderThreadPoolMinSize;
	int32					senderThreadPoolMaxSize;
	int32					playThreadPoolSize;

	std::string				netWorkcardIP;

	int32					stdPeriod;
	int32					stdMaxQueue;
	int32					stdMinQueue;
	int32					profileFlag;
	char					catchDir[MAX_PATH];


	// stores the ice properties
	std::map<std::string, std::string> icePropMap;

	static void structure(Config::Holder<DataStreamCfg> &holder)
	{
		holder.addDetail("DataStream/Bind", "endpoint", &DataStreamCfg::endpoint, "default -p 10040", Config::optReadOnly);	
//		holder.addDetail("DataStream/DatabaseFolder", "path", &DataStreamCfg::dbPath, NULL, Config::optReadOnly);	
		holder.addDetail("DataStream/System", "netId", &DataStreamCfg::netId, "ADMIN2_SS_CL", Config::optReadOnly);	
		holder.addDetail("DataStream/System", "checkStreamTimeout", &DataStreamCfg::checkStreamTimeout, "172800", Config::optReadOnly);	
		holder.addDetail("DataStream/System", "totalRate", &DataStreamCfg::totalRate, "94371840", Config::optReadOnly);	
		holder.addDetail("DataStream/System", "higherPriority", &DataStreamCfg::higherPriority, "0", Config::optReadOnly);	

		holder.addDetail("DataStream/ThreadPool", "readerThreadPoolMaxSize", &DataStreamCfg::readerThreadPoolMaxSize, "150", Config::optReadOnly);	
		holder.addDetail("DataStream/ThreadPool", "senderThreadPoolMaxSize", &DataStreamCfg::senderThreadPoolMaxSize, "150", Config::optReadOnly);	
		holder.addDetail("DataStream/ThreadPool", "playThreadPoolSize", &DataStreamCfg::playThreadPoolSize, "20", Config::optReadOnly);
		holder.addDetail("DataStream/NetWork", "netWorkCard", &DataStreamCfg::netWorkcardIP, "", Config::optReadOnly);	

		holder.addDetail("DataStream/IceProperties/prop", &DataStreamCfg::readProp, &DataStreamCfg::registerNothing);

	};
	DataStreamCfg()
	{
		totalRate = 100 * 1024 * 1024; // 100MB
		higherPriority = false;
		profileFlag = 0;

		readerThreadPoolMinSize = 5;
		readerThreadPoolMaxSize = 150;
		senderThreadPoolMinSize = 10;
		senderThreadPoolMaxSize = 150;
		checkStreamTimeout = 70 * 60;

		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, _COUNTOF(path));
		char* c = path + lstrlen(path);
		while(c >= path) {
			if (*c == '\\') {
				*c = '\0';
				break;
			}

			c --;
		}

		lstrcatA(path, "\\ds_cache");
		strncpy(catchDir, path, MAX_PATH);

		stdPeriod = DEFAULT_STD_PERIOD;
		stdMaxQueue = DEFAULT_STD_MAXQUEUE;
		stdMinQueue = DEFAULT_STD_MINQUEUE;
	}
	void readProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		icePropMap[nvholder.name] = nvholder.value;
	}
	void registerNothing(const std::string&){}
};

static void showCfg(const DataStreamCfg& cfg)
{
	using namespace std;
	cout << "DataStream CfgLoader: \n";
// 	cout << "\t DBUserName \t" << cfg.szUsername << "\n";	
// 	cout << "\t PassWord: \t " << cfg.szPassword << "\n";	
// 	cout << "\t DBName \t" << cfg.szDBName << "\n";
}
#endif //__DataStreamCfgLoader_H__