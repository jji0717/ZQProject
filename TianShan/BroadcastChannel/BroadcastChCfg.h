#ifndef __BroadcastChCfgLoader_H__
#define __BroadcastChCfgLoader_H__
#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include "MRTDef.h"
#include <map>
#include <string>
using namespace ZQ::common;
/*
struct SoapMRTCfg
{
	int32 enable;
	int32 connectTimeout;
	int32 sendTimeout;
	int32 receiverTimeout;
	std::string protocol;
	int32 port;
	std::string uri;
	std::string ip;
	SoapMRTCfg()
	{
		enable = 0;
		connectTimeout = 5000;
        sendTimeout = 5000;
		receiverTimeout = 5000;
		protocol= "http";
		port = 12345;
		uri = "/wsdl?";
		ip="";
	}

	static void structure(Config::Holder<SoapMRTCfg> &holder)
	{
		holder.addDetail("", "enable", &SoapMRTCfg::enable, "0", Config::optReadOnly);
		holder.addDetail("", "protocol", &SoapMRTCfg::protocol, "http", Config::optReadOnly);	
		holder.addDetail("", "port", &SoapMRTCfg::port, "12345", Config::optReadOnly);	
		holder.addDetail("", "uri", &SoapMRTCfg::uri, "/wsdl?", Config::optReadOnly);	
		holder.addDetail("", "ip", &SoapMRTCfg::ip, "", Config::optReadOnly);
		holder.addDetail("", "connectTimeout", &SoapMRTCfg::connectTimeout, "5000", Config::optReadOnly);	
		holder.addDetail("", "sendTimeOut", &SoapMRTCfg::sendTimeout, "5000",Config::optReadOnly);	
		holder.addDetail("", "receiveTimeout", &SoapMRTCfg::receiverTimeout, "5000", Config::optReadOnly);	
	}
};
*/
struct Streamer
{
	std::string streamerName;
	std::string mrtEndpoint;
	int32 connectTimeout;
	int32 sendTimeout;
	int32 receiverTimeout;

	static void structure(ZQ::common::Config::Holder<Streamer> &holder)
	{
		holder.addDetail("", "name", &Streamer::streamerName, NULL, Config::optReadOnly);
		holder.addDetail("", "mrtEndpoint", &Streamer::mrtEndpoint, NULL,Config::optReadOnly);
		holder.addDetail("", "connectTimeout", &Streamer::connectTimeout, "5000", Config::optReadOnly);	
		holder.addDetail("", "sendTimeOut", &Streamer::sendTimeout, "5000",Config::optReadOnly);	
		holder.addDetail("", "receiveTimeout", &Streamer::receiverTimeout, "5000", Config::optReadOnly);	
	}
	Streamer()
	{
		connectTimeout = 5000;
		sendTimeout = 5000;
		receiverTimeout = 5000;
	}
};

struct MRTStreamerCfg
{
	int enable;
	int32 pauseMaxCfg ;
	int32 pauseMinCfg ;
	int32 targetTime;

	std::string  bindEndPoint ;
	int          maxPenalty;
	int          penalty;

	std::string  replicaSubscriberEndpoint, eventChannel ;

	std::string nodeId;
	std::vector<std::string> spigotIds;
	StreamNetIDToMRTEndpoints streamToMRTEndpointInfos;

	MRTStreamerCfg()
	{
		enable = 0;
		pauseMaxCfg=2000;
		pauseMinCfg=500;
	}

	static void structure(Config::Holder<MRTStreamerCfg> &holder)
	{
		holder.addDetail("","enable",&MRTStreamerCfg::enable, "0",Config::optReadOnly);

		holder.addDetail("Bind","endpoint",&MRTStreamerCfg::bindEndPoint,NULL,Config::optReadOnly);
		holder.addDetail("Bind","maxPenalty",&MRTStreamerCfg::maxPenalty,"100",Config::optReadOnly);
		holder.addDetail("Bind","penalty",&MRTStreamerCfg::penalty,"20",Config::optReadOnly);
		holder.addDetail("RandomTime","pausemax",&MRTStreamerCfg::pauseMaxCfg,"50",Config::optReadOnly);
		holder.addDetail("RandomTime","pausemin",&MRTStreamerCfg::pauseMinCfg,"10",Config::optReadOnly);
		holder.addDetail("Service","replicaSubscriberEndpoint",&MRTStreamerCfg::replicaSubscriberEndpoint,NULL,Config::optReadOnly);
		holder.addDetail("Service","eventChannel",&MRTStreamerCfg::eventChannel,NULL,Config::optReadOnly);
		holder.addDetail("Streamers/streamer", &MRTStreamerCfg::readStreamers, &MRTStreamerCfg::registerNothing);
		holder.addDetail("TimerWatch","targettime",&MRTStreamerCfg::targetTime,"1000",Config::optReadOnly);
	}
	void readStreamers(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
	{
		using namespace ZQ::common::Config;
		Holder<Streamer> propHolder;
		propHolder.read(node, hPP);
		std::string strKey = nodeId + "/" + propHolder.streamerName; 

		MRTEndpointInfo mrtEndpointInfo;
		mrtEndpointInfo.mrtEndpoint = propHolder.mrtEndpoint;
		mrtEndpointInfo.connectTimeout = propHolder.connectTimeout;
		mrtEndpointInfo.receiverTimeout =propHolder.receiverTimeout;
		mrtEndpointInfo.sendTimeout = propHolder.sendTimeout;
		streamToMRTEndpointInfos[strKey] = mrtEndpointInfo;

		spigotIds.push_back(propHolder.streamerName);
	}
	void setNetId(const std::string& netId)
	{
		nodeId = netId;
	}
	void registerNothing(const std::string&){}
};

struct BroadcastChCfg
{	
	// defines the BcastChannelPublisher properties 
	std::string  broadcastPPendpoint;
	int32        BroadcastPPEvitSize;
	int32        ChannelItemEvitSize;
	int32        InputLocalTime;
	int32		 DefaultChannelMaxBitrate;
	int32		 IsRepeat;
	int32		 renewtime;
	int32        windowsize;
	int32        miniPLcount;
	int32        expirationTime;
    //AppInfo
	std::string  BcastRtspURL;
	int32        groupId;

	// defines the ice log properties
	std::string iceLogPath;
	int32 iceLogLevel;
	int32 iceLogSize;
	int32 iceLogCount;

    //NVOD
	int32        portIncreaseBase;

	// defines the PurchaseManagement properties 
	int32 PurchaseEvitSize;
	int32 PurchaseItemEvitSize;
	int32 purchaseTimeout;

	// size of ThreadPool
	int32 ThreadPoolSize;

	// the directory of database
	int32		 dumpFullMemory;
	std::string  crushDumpPath;

	std::string  dbpath;
	std::string dbRuntimeDataPath;

	std::string  weiwooendpoint;

	// defines the TianshanEvent properties
	std::string TopicMgrEndPoint;
	std::string ListenEventEndPoint;
	std::string TSEventRuntimeDataPath;

	// stores the ice properties
	std::map<std::string, std::string> icePropMap;


//	SoapMRTCfg soapMRTCfg;

	MRTStreamerCfg mrtStreamServiceCfg;
	std::string netId;

	static void structure(Config::Holder<BroadcastChCfg> &holder)
	{
		holder.addDetail("BroadcastChannel", "netId", &BroadcastChCfg::netId, "LiveCh", Config::optReadOnly);	

		holder.addDetail("BroadcastChannel/BcastPublishPoint", "endpoint", &BroadcastChCfg::broadcastPPendpoint, NULL, Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "cacheSize", &BroadcastChCfg::BroadcastPPEvitSize, "100", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "itemCacheSize", &BroadcastChCfg::ChannelItemEvitSize, "1000", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "inputAsLocalTime", &BroadcastChCfg::InputLocalTime, "1", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "defaultChannelBitrate", &BroadcastChCfg::DefaultChannelMaxBitrate, "4000000", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "repeat", &BroadcastChCfg::IsRepeat, "0", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "renewtime", &BroadcastChCfg::renewtime, "3600", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "windowsize", &BroadcastChCfg::windowsize, "20", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "minimumPLItemCount", &BroadcastChCfg::miniPLcount, "5", Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/BcastPublishPoint", "expirationTime", &BroadcastChCfg::expirationTime, "600000", Config::optReadOnly);	

		holder.addDetail("BroadcastChannel/AppInfo", "url", &BroadcastChCfg::BcastRtspURL, NULL, Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/AppInfo", "groupid", &BroadcastChCfg::groupId, NULL, Config::optReadOnly);	

		holder.addDetail("BroadcastChannel/IceLog", "path", &BroadcastChCfg::iceLogPath, "C:\\TianShan\\Logs\\", Config::optReadOnly);
		holder.addDetail("BroadcastChannel/IceLog", "level", &BroadcastChCfg::iceLogLevel, "7", Config::optReadWrite);
		holder.addDetail("BroadcastChannel/IceLog", "size", &BroadcastChCfg::iceLogSize, "10240000", Config::optReadWrite);
		holder.addDetail("BroadcastChannel/IceLog", "count", &BroadcastChCfg::iceLogCount, "5", Config::optReadWrite);

		holder.addDetail("BroadcastChannel/NVODPublishPoint", "portIncreaseBase", &BroadcastChCfg::portIncreaseBase, "1", Config::optReadOnly);	

		holder.addDetail("BroadcastChannel/ThreadPool", "size", &BroadcastChCfg::ThreadPoolSize, "20", ZQ::common::Config::optReadOnly);

		holder.addDetail("BroadcastChannel/CrashDump", "path", &BroadcastChCfg::crushDumpPath, NULL, Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/CrashDump", "fullDump", &BroadcastChCfg::dumpFullMemory, NULL, Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/Database", "path", &BroadcastChCfg::dbpath, NULL, Config::optReadOnly);	
		holder.addDetail("BroadcastChannel/Database", "runtimeData", &BroadcastChCfg::dbRuntimeDataPath, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("BroadcastChannel/Weiwoo", "endpoint", &BroadcastChCfg::weiwooendpoint, NULL, Config::optReadOnly);	

		holder.addDetail("BroadcastChannel/TianShanEvents", "EventChannelEndPoint", &BroadcastChCfg::TopicMgrEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("BroadcastChannel/TianShanEvents", "listenEndpoint", &BroadcastChCfg::ListenEventEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("BroadcastChannel/TianShanEvents", "runtimeData", &BroadcastChCfg::TSEventRuntimeDataPath, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("BroadcastChannel/IceProperties/prop", &BroadcastChCfg::readProp, &BroadcastChCfg::registerNothing);
//		holder.addDetail("BroadcastChannel/SoapMRT", &BroadcastChCfg::readSoapMRT, &BroadcastChCfg::registerNothing, ZQ::common::Config::Range(0, 1));

		holder.addDetail("BroadcastChannel/MRTStreamer", &BroadcastChCfg::readMRTStreamer, &BroadcastChCfg::registerNothing, ZQ::common::Config::Range(0, 1));

	};
	BroadcastChCfg()
	{
	}
	void readProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		icePropMap[nvholder.name] = nvholder.value;
	}

	/*void readSoapMRT(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<SoapMRTCfg> soapMRTholder("");
		soapMRTholder.read(node, hPP);
		soapMRTCfg = soapMRTholder;
	}*/ 
	 
	void readMRTStreamer(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<MRTStreamerCfg> mrtCfgholder("");
		mrtCfgholder.setNetId(netId);
		mrtCfgholder.read(node, hPP);
		mrtStreamServiceCfg = mrtCfgholder;
	}
	void registerNothing(const std::string&){}
};

#endif //__BroadcastChCfgLoader_H__