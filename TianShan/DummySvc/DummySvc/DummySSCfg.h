#ifndef  _DUMMYSS_CFG_H
#define _DUMMYSS_CFG_H

#include "ConfigHelper.h"
#include "DummyStreamSmith.h"

#include "stdafx.h"


using namespace  ZQ::common;
struct Streamer
{
	std::string streamerName;
	static void structure(ZQ::common::Config::Holder<Streamer> &holder)
	{
		holder.addDetail("", "name", &Streamer::streamerName);
	}
};

class DummySSCfg
{
public:
	DummySSCfg() ;
	~DummySSCfg() ;
public:
	int32 pauseMaxCfg ;
	int32 pauseMinCfg ;
	int32 logLevel;
	int32 targetTime;

	std::string  bindEndPoint ;
	std::string  replicaSubscriberEndpoint, eventChannel ;
	std::string  dataPath, runTimePath ;
	std::string	IceLogPath;

	std::string nodeId;
	std::vector<std::string > spigotIds;

public:
	static void structure(Config::Holder<DummySSCfg> &holder);
	 void registerNothing(const std::string&){}
	void readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readNetIdStreamers(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
public:
	std::map<std::string, std::string> iceProperties;
};


#endif