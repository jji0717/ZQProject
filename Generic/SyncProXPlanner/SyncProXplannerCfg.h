#ifndef __SyncProXplannerCfg_H__
#define __SyncProXplannerCfg_H__
#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include <map>
#include <string>
using namespace ZQ::common;

struct people
{
	std::string  userEmail;
	std::string  userID;
	std::map<std::string, std::string> peopleParam;

	static void structure(Config::Holder<people>& holder)
	{
		holder.addDetail("", "userEmail", &people::userEmail, NULL,Config::optReadWrite);
		holder.addDetail("", "userID", &people::userID, NULL,Config::optReadWrite);
		holder.addDetail("param", &people::readParam, &people::registerNothing, Config::Range(0, -1));
	}
	void readParam(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		peopleParam[nvholder.name] = nvholder.value;
	}
	void registerNothing(const std::string&){}
};

typedef std::map< std::string, Config::Holder<people> > PeopleMap;

struct SyncProXplannerCfg
{
	std::string host;
	std::string port;
	std::string dbName;
	std::string username;
	std::string passwd;

	std::string xpusername;
	std::string xppasswd; 
	std::string endpoint;
	int32       timeout;

	int32 syncDays;
	std::string dayofweek;
	int32 hours;
	int32 interval;

    PeopleMap peoples;
	static void structure(Config::Holder<SyncProXplannerCfg>& holder)
	{
		holder.addDetail("SyncProXplanner/DataBase", "host", &SyncProXplannerCfg::host, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/DataBase", "port", &SyncProXplannerCfg::port, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/DataBase", "dbname", &SyncProXplannerCfg::dbName, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/DataBase", "username", &SyncProXplannerCfg::username, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/DataBase", "passwd", &SyncProXplannerCfg::passwd, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/SyncData", "username", &SyncProXplannerCfg::xpusername, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/SyncData", "passwd", &SyncProXplannerCfg::xppasswd, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/SyncData", "endpoint", &SyncProXplannerCfg::endpoint, NULL, Config::optReadOnly);
		holder.addDetail("SyncProXplanner/SyncData", "timeout", &SyncProXplannerCfg::timeout, "50", Config::optReadOnly);

		holder.addDetail("SyncProXplanner/AutoSync", "totalDays", &SyncProXplannerCfg::syncDays, "7", Config::optReadOnly);
		holder.addDetail("SyncProXplanner/AutoSync", "dayofweek", &SyncProXplannerCfg::dayofweek, "1", Config::optReadOnly);
		holder.addDetail("SyncProXplanner/AutoSync", "hours", &SyncProXplannerCfg::hours, "20", Config::optReadOnly);
		holder.addDetail("SyncProXplanner/AutoSync", "interval", &SyncProXplannerCfg::interval, "1800", Config::optReadOnly);

		holder.addDetail("SyncProXplanner/PeopleInfo/people", &SyncProXplannerCfg::readPeople, &SyncProXplannerCfg::registerNothing, Config::Range(0, -1));
	}

	void readPeople(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{   
		Config::Holder<people> nvholder("");
		nvholder.read(node, hPP);
		peoples[nvholder.userEmail] = nvholder;
	}
	void registerNothing(const std::string&){}
	SyncProXplannerCfg()
	{

	}
};

#endif // #define __SyncProXplannerCfg_H__