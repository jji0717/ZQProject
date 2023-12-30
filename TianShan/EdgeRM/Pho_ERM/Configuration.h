#ifndef _TIANSHAN_EDGERM_PHO_CONFIGURATION_HEADER_FILE_H__
#define _TIANSHAN_EDGERM_PHO_CONFIGURATION_HEADER_FILE_H__

#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include <map>
#include <string>
#include <list>
using namespace ZQ::common;

struct S6ServerInfo
{
	std::string ipPort;
	int         maxSessionGroups;

	static void structure(ZQ::common::Config::Holder<S6ServerInfo> &holder)
	{
		using namespace ZQ::common::Config;
		//database folder
		holder.addDetail("", "ipport", &S6ServerInfo::ipPort, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessionGroups", &S6ServerInfo::maxSessionGroups, "10", ZQ::common::Config::optReadOnly);
	}
	S6ServerInfo()
	{
		ipPort = "";
		maxSessionGroups = 10;
	}
};

typedef std::map<std::string, S6ServerInfo> S6ServerInfos;

struct SessionGroup
{


	std::string netId;	
	int defaultSG;
	int32 groupSyncInterval;
	int32 maxSessionPerGroup;
	std::string bindAddress;
	int SessionInterfaceDisconnectAtTimeout;

	S6ServerInfos s6ServerInfos;
	SessionGroup()
	{
		defaultSG = 1;
		groupSyncInterval = 600000;
		maxSessionPerGroup = 300;
		SessionInterfaceDisconnectAtTimeout  = 10;
	}

	static void structure(ZQ::common::Config::Holder<SessionGroup> &holder)
	{
		using namespace ZQ::common::Config;
		//database folder
		holder.addDetail("", "netId", &SessionGroup::netId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "defaultMaxSessionGroups", &SessionGroup::defaultSG, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "groupSyncInterval", &SessionGroup::groupSyncInterval, "600000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessionsPerGroup", &SessionGroup::maxSessionPerGroup, "300", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "bindAddress", &SessionGroup::bindAddress, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "disconnectAtTimeout", &SessionGroup::SessionInterfaceDisconnectAtTimeout, "10", ZQ::common::Config::optReadOnly);
		holder.addDetail("S6Server", &SessionGroup::readS6ServerParam, &SessionGroup::registerNothing);
	}
	void readS6ServerParam(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<S6ServerInfo> s6ServerInfo("");
		s6ServerInfo.read(node, hPP);
		s6ServerInfos[s6ServerInfo.ipPort] = s6ServerInfo;
	}
    void registerNothing(const std::string&){}
};

struct PHOConfig
{
	char szPHOLogFileName[512];

	std::string databasePath;
	std::string databaseRuntimepath;

	// stores the ice properties
	std::map<std::string, std::string> icePropMap;

	int32 lPHOLogLevel;
	int32 lPHOLogFileSize;
	int32 lPHOlogFileCount;
	int32 lPHOLogBufferSize;
	int32 lPHOLogWriteTimteout;
	
	int32 maxCandidates;
	int32 allocationLease;
	int32 evictorSize;
	int32 interval;

	std::string endpoint;

	int32 threadPoolsize;

	int32 sessionTimeOut;
    int32 requestTimeOut;
	int32 connectTimeout;

	SessionGroup sessionGroup;

	PHOConfig()
	{
		strcpy(szPHOLogFileName,"Pho_ERM.log");
		lPHOLogLevel			= 7;
		lPHOLogFileSize			= 1024 * 1000 * 10;
		lPHOLogBufferSize		= 10240;
		lPHOLogWriteTimteout	= 2;

		maxCandidates = 5;
		allocationLease = 30000;
		evictorSize = 1000;
		threadPoolsize = 50;
		interval = 7200;

		sessionTimeOut = 600000;
		requestTimeOut = 5000;
		connectTimeout = 3000;
	}

	static void structure(ZQ::common::Config::Holder<PHOConfig> &holder)
	{
		using namespace ZQ::common::Config;
		//database folder
		holder.addDetail("default/Database", "path", &PHOConfig::databasePath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("default/Database", "runtimePath", &PHOConfig::databaseRuntimepath, NULL, ZQ::common::Config::optReadOnly);

		//ice property
		holder.addDetail("default/IceProperties/prop",&PHOConfig::readIceProperties, &PHOConfig::registerNothing);

		//log config
		holder.addDetail("PHOERM/Log", "level", &PHOConfig::lPHOLogLevel, "7", optReadOnly);
		holder.addDetail("PHOERM/Log", "size", &PHOConfig::lPHOLogFileSize, "40960000", optReadOnly);
		holder.addDetail("PHOERM/Log", "count", &PHOConfig::lPHOlogFileCount, "5", optReadOnly);
		holder.addDetail("PHOERM/Log", "buffer", &PHOConfig::lPHOLogBufferSize, "10240", optReadOnly);
		holder.addDetail("PHOERM/Log", "flushtimeout", &PHOConfig::lPHOLogWriteTimteout, "2", optReadOnly);
		
		//PHO allocation attribute
		holder.addDetail("PHOERM/Allocation", "maxCandidates", &PHOConfig::maxCandidates, "5", optReadOnly);
		holder.addDetail("PHOERM/Allocation", "allocationLease", &PHOConfig::allocationLease, "30000", optReadOnly);
		holder.addDetail("PHOERM/Allocation", "evictorSize", &PHOConfig::evictorSize, "1000", optReadOnly);
		holder.addDetail("PHOERM/Allocation", "interval", &PHOConfig::interval, "7200", optReadOnly);

		//PHO bind information
		holder.addDetail("PHOERM/Bind", "endpoint", &PHOConfig::endpoint, NULL, ZQ::common::Config::optReadOnly);

        //ThreadPoolSize
		holder.addDetail("PHOERM/ThreadPool", "size", &PHOConfig::threadPoolsize, "50", ZQ::common::Config::optReadOnly);

		//s6 session
		holder.addDetail("PHOERM/S6Session", "sessionTimeOut", &PHOConfig::sessionTimeOut, "600000", ZQ::common::Config::optReadOnly);
		holder.addDetail("PHOERM/S6Session", "requestTimeout", &PHOConfig::requestTimeOut, "5000", ZQ::common::Config::optReadOnly);
		holder.addDetail("PHOERM/S6Session", "connectTimeout", &PHOConfig::connectTimeout, "3000", ZQ::common::Config::optReadOnly);

		//sessionGroup
		holder.addDetail("PHOERM/SessionGroup", &PHOConfig::readSessionGroup, &PHOConfig::registerNothing); //, Config::Range(0,1));

	}
	void readSessionGroup(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<SessionGroup> sessGP("");
		sessGP.read(node, hPP);
		sessionGroup  = sessGP;
	}
	void readIceProperties(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		icePropMap[nvholder.name] = nvholder.value;
	}
	void registerNothing(const std::string&){}
};

#endif //_TIANSHAN_EDGERM_PHO_CONFIGURATION_HEADER_FILE_H__
