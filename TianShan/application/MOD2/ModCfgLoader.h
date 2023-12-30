#ifndef __ZQMODApplication_ModCfgLoader_H__
#define __ZQMODApplication_ModCfgLoader_H__
#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include <list>
#include <map>
#include <string>
#include <vector>
#include "TianShanDefines.h"
#include "MRTDef.h"
using namespace ZQ::common;

#define DEFAULTAPPPATH "DefaultAppPath"
typedef std::map<std::string,  Config::Holder<Config::NVPair> > PARAMMAP;
typedef std::vector<std::string> VOLUMESSTRS;
struct TestItemInfo
{
	std::string name;
	std::string bandWidth;
	int32 cueIn;
	int32 cueOut;
	std::string nasurls;
	PARAMMAP userpropmap;
	VOLUMESSTRS volumes;
	static void structure(Config::Holder<TestItemInfo>& holder)
    {
        holder.addDetail("", "name", &TestItemInfo::name, NULL,Config::optReadOnly);
        holder.addDetail("", "bandWidth", &TestItemInfo::bandWidth, "3750000",Config::optReadOnly);
		holder.addDetail("", "cueIn", &TestItemInfo::cueIn, NULL,Config::optReadOnly);
        holder.addDetail("", "cueOut", &TestItemInfo::cueOut, NULL,Config::optReadOnly);
		holder.addDetail("", "nasurls", &TestItemInfo::nasurls, "",Config::optReadOnly);
		holder.addDetail("UserProp", &TestItemInfo::readUserProp, &TestItemInfo::registerUserProp, Config::Range(0, -1));
    }
	void readUserProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
		Config::Holder<Config::NVPair> nvholder("name");
		nvholder.read(node, hPP);
		if(nvholder.name != "Volumes")
			userpropmap[nvholder.name] = nvholder;
		else
			volumes.push_back(nvholder.value);
    }	
	void registerUserProp(const std::string &full_path)
    {
		for(PARAMMAP::iterator it = userpropmap.begin(); it != userpropmap.end(); ++it)
            (it->second).snmpRegister(full_path);
    }
};
struct  TestPlaylist
{

	int32 enable;
	typedef std::vector< Config::Holder<TestItemInfo> > TestItemVEC;
	TestItemVEC testitems;
	static void structure(Config::Holder<TestPlaylist>& holder)
    {
        holder.addDetail("", "enable", &TestPlaylist::enable, "0",Config::optReadWrite);
		holder.addDetail("Element", &TestPlaylist::readTestItem, &TestPlaylist::registerNothing);
    }

	void readTestItem(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<TestItemInfo> itemholder("");
        itemholder.read(node, hPP);
        testitems.push_back(itemholder);
    }
	void registerNothing(const std::string&){}
};
struct TestAuthorizeParam
{
	int32 enable;
	PARAMMAP authorParam;
	static void structure(Config::Holder<TestAuthorizeParam>& holder)
    {
        holder.addDetail("", "enable", &TestAuthorizeParam::enable, "0",Config::optReadOnly);
		holder.addDetail("param", &TestAuthorizeParam::readTestAuthor, &TestAuthorizeParam::registerNothing);
	}
	void readTestAuthor(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<Config::NVPair> nvholder("");
        nvholder.read(node, hPP);
        authorParam[nvholder.name] = nvholder;
    }
	void registerNothing(const std::string&){}
};
struct AppDataPattern
{
	std::string param;
	std::string pattern;
	PARAMMAP appDataParammap;
	static void structure(Config::Holder<AppDataPattern>& holder)
	{
		holder.addDetail("", "param", &AppDataPattern::param, "0",Config::optReadOnly);
		holder.addDetail("", "pattern", &AppDataPattern::pattern, "0",Config::optReadOnly);
		holder.addDetail("param", &AppDataPattern::readAppDataParam, &AppDataPattern::registerAppData);
	}
	void readAppDataParam(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		appDataParammap[nvholder.name] = nvholder;
	}
	void registerAppData(const std::string &full_path)
	{
		for(PARAMMAP::iterator it = appDataParammap.begin(); it != appDataParammap.end(); ++it)
			(it->second).snmpRegister(full_path);
	}
	void registerNothing(const std::string&){}

};

typedef std::map< std::string, Config::Holder<AppDataPattern> > AppDataPatternMAP;

struct AdsProps
{
	std::string adsType;
	PARAMMAP    adsprops;
	int32 bitwiseOr;
	static void structure(Config::Holder<AdsProps>& holder)
	{
		holder.addDetail("", "adsType", &AdsProps::adsType,NULL, Config::optReadWrite);
		holder.addDetail("", "bitwiseOr", &AdsProps::bitwiseOr,NULL, Config::optReadWrite);
		holder.addDetail("prop", &AdsProps::readadsProps, &AdsProps::registerNothing, Config::Range(0, -1));
	}
	AdsProps()
	{
		adsType = "";
		adsprops.clear();
	}
	void readadsProps(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("name");
		nvholder.read(node, hPP);
		adsprops[nvholder.name] = nvholder;
	}
	void registerNothing(const std::string&){}
};

typedef std::map<std::string, Config::Holder<AdsProps> > ADSPROPS;
struct AdsReplacement
{
	std::string adsEntry;
	PARAMMAP    adsParams;
	ADSPROPS    adsProps;
	static void structure(Config::Holder<AdsReplacement>& holder)
	{
		holder.addDetail("", "entry", &AdsReplacement::adsEntry,NULL, Config::optReadWrite);
		holder.addDetail("param", &AdsReplacement::readadsParams, &AdsReplacement::registerNothing, Config::Range(1, -1));
		holder.addDetail("AdsProp", &AdsReplacement::readAdsProps, &AdsReplacement::registerNothing, Config::Range(0,-1));
	}
	AdsReplacement()
	{
		adsEntry = "";
		adsParams.clear();
	}
	void readadsParams(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("name");
		nvholder.read(node, hPP);
		adsParams[nvholder.name] = nvholder;
	}
	void readAdsProps(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<AdsProps> nvholder("adsType");
		nvholder.read(node, hPP);
		adsProps[nvholder.adsType] = nvholder;
	}
	 void registerNothing(const std::string&){}
};


struct AssetLocationCfg
{
	std::string alEntry;
	PARAMMAP    alParams;
	static void structure(Config::Holder<AssetLocationCfg>& holder)
	{
		holder.addDetail("", "entry", &AssetLocationCfg::alEntry,NULL, Config::optReadWrite);
		holder.addDetail("param", &AssetLocationCfg::readalParams, &AssetLocationCfg::registerNothing, Config::Range(1, -1));
	}
	AssetLocationCfg()
	{
		alEntry = "";
		alParams.clear();
	}
	void readalParams(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("name");
		nvholder.read(node, hPP);
		alParams[nvholder.name] = nvholder;
	}
	void registerNothing(const std::string&){}
};
/*
<AssociateProvider enable="1" module="${TianShanHomeDir}\modules\MHO_MODPlugIn.dll" entry="LookupPid">

<param name="endpoint" value="LAM:tcp -h *.*.*.* -p 2222"/>

</AssociateProvider>
*/

struct AssociateProvider 
{
    int32       enable;
	std::string apidEntry;
	PARAMMAP    apidParams;
	static void structure(Config::Holder<AssociateProvider >& holder)
	{
		holder.addDetail("", "enable", &AssociateProvider ::enable, "0",Config::optReadWrite);
		holder.addDetail("", "entry", &AssociateProvider ::apidEntry,NULL, Config::optReadWrite);
		holder.addDetail("param", &AssociateProvider::readadsParams, &AssociateProvider ::registerNothing, Config::Range(1, -1));
	}
	AssociateProvider ()
	{
		enable = 0;
		apidEntry = "";
		apidParams.clear();
	}
	void readadsParams(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("name");
		nvholder.read(node, hPP);
		apidParams[nvholder.name] = nvholder;
	}
	void registerNothing(const std::string&){}
};

struct PlaylistRender
{
	int playAdOnce;
	static void structure(Config::Holder<PlaylistRender>& holder)
	{
		holder.addDetail("", "playAdOnce", &PlaylistRender::playAdOnce, NULL,Config::optReadWrite);
	}
	PlaylistRender()
	{
		playAdOnce = 0;
	}
	void registerNothing(const std::string&){}
};

struct StorageNetId
{
	std::string netId;
	static void structure(Config::Holder<StorageNetId>& holder)
	{
		holder.addDetail("", "netId", &StorageNetId::netId, NULL,Config::optReadOnly);
	}
	StorageNetId()
	{
		netId = "";
	}
};
struct ResourceRestriction
{
	std::vector<std::string> storageNetIds;
	std::string name;
	static void structure(Config::Holder<ResourceRestriction>& holder)
	{
		holder.addDetail("Storages/Storage", &ResourceRestriction::readStorage, &ResourceRestriction ::registerNothing, Config::Range(0,-1));
	}
	ResourceRestriction()
	{
		storageNetIds.clear();
	}
	void registerNothing(const std::string&){}

	void readStorage(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<StorageNetId> nvholder("");
		nvholder.read(node, hPP);
		if(storageNetIds.end() == std::find(storageNetIds.begin(), storageNetIds.end(), nvholder.netId))
			storageNetIds.push_back(nvholder.netId);
	}
};

struct Urlpattern
{
    int32 enable;
    std::string pattern;
	int32 priority;
    
    int32 Authenable;
    std::string authEntry;
    PARAMMAP authParams;
	AppDataPatternMAP authAppDataMap;
	
    std::string playlistEntry;
    PARAMMAP playlistParams;
	AppDataPatternMAP playListAppDataMap;

	typedef std::vector< Config::Holder<TestItemInfo> > TestItemVEC;
	TestItemVEC playlistItems;

	AdsReplacement adsReplacement;
	AssetLocationCfg assetLocation;
	PlaylistRender plRender;
	AssociateProvider plAPID;
	ResourceRestriction resRestriction;


    static void structure(Config::Holder<Urlpattern>& holder)
    {
        holder.addDetail("", "enable", &Urlpattern::enable, "0",Config::optReadWrite);
        holder.addDetail("", "pattern", &Urlpattern::pattern,"",Config::optReadOnly);
		holder.addDetail("", "priority", &Urlpattern::priority,"",Config::optReadOnly);
        holder.addDetail("Authorization", "enable", &Urlpattern::Authenable, "0", Config::optReadWrite);
        holder.addDetail("Authorization", "entry", &Urlpattern::authEntry,NULL, Config::optReadWrite);
        holder.addDetail("Authorization/param", &Urlpattern::readAuthParam, &Urlpattern::registerAuthor, Config::Range(0, -1));
		holder.addDetail("Authorization/AppDataPattern", &Urlpattern::readauthAppData, &Urlpattern::registerauthAppData, Config::Range(0, -1));
		
        holder.addDetail("PlayList", "entry", &Urlpattern::playlistEntry,NULL, Config::optReadWrite);
		holder.addDetail("PlayList/param", &Urlpattern::readPlaylistParam, &Urlpattern::registerPlaylist, Config::Range(0, -1));
		holder.addDetail("PlayList/Element", &Urlpattern::readPlayListItems, &Urlpattern::registerNothing, Config::Range(0, -1));
		holder.addDetail("PlayList/AppDataPattern", &Urlpattern::readplayListAppData, &Urlpattern::registerplayListAppData, Config::Range(0, -1));
		holder.addDetail("ResourceRestriction", &Urlpattern::readResourceRestriction, &Urlpattern::registerNothing, Config::Range(0,1));

		holder.addDetail("AdsReplacement", &Urlpattern::readAdsReplacement, &Urlpattern::registerNothing, Config::Range(0,1));

		holder.addDetail("AssetLocation", &Urlpattern::readAssetLocation, &Urlpattern::registerNothing, Config::Range(0,1));

		holder.addDetail("PlaylistRender", &Urlpattern::readplRender, &Urlpattern::registerNothing, Config::Range(0,1));
		holder.addDetail("AssociateProvider", &Urlpattern::readAssociateProvider, &Urlpattern::registerNothing, Config::Range(0,1));
    }

    void readAuthParam(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<Config::NVPair> nvholder("name");
        nvholder.read(node, hPP);
        authParams[nvholder.name] = nvholder;
    }

    void readPlaylistParam(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<Config::NVPair> nvholder("name");
        nvholder.read(node, hPP);
        playlistParams[nvholder.name] = nvholder;
    }
	void readPlayListItems(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<TestItemInfo> nvholder("");
		nvholder.read(node, hPP);
		playlistItems.push_back(nvholder);
	}

	void readauthAppData(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<AppDataPattern> authAppholder("param");
		authAppholder.read(node, hPP);
		authAppDataMap[authAppholder.pattern]  = authAppholder;
	}

	void readplayListAppData(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<AppDataPattern> playlistAppholder("param");
		playlistAppholder.read(node, hPP);
		playListAppDataMap[playlistAppholder.pattern]  = playlistAppholder;
	}

	void registerAuthor(const std::string &full_path)
    {
		for(PARAMMAP::iterator it = authParams.begin(); it != authParams.end(); ++it)
            (it->second).snmpRegister(full_path);
    }

	void registerPlaylist(const std::string &full_path)
    {
		for(PARAMMAP::iterator it = playlistParams.begin(); it != playlistParams.end(); ++it)
            (it->second).snmpRegister(full_path);
    }

	void registerauthAppData(const std::string &full_path)
	{
		for(AppDataPatternMAP::iterator it = authAppDataMap.begin(); it != authAppDataMap.end(); ++it)
			(it->second).snmpRegister(full_path);
	}

	void registerplayListAppData(const std::string &full_path)
	{
		for(AppDataPatternMAP::iterator it = playListAppDataMap.begin(); it != playListAppDataMap.end(); ++it)
			(it->second).snmpRegister(full_path);
	}

	void readAdsReplacement(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<AdsReplacement> adsReplacementholder("");
		adsReplacementholder.read(node, hPP);
		adsReplacement = adsReplacementholder;
	}

	void readAssetLocation(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<AssetLocationCfg> AssetLoactiontholder("");
		AssetLoactiontholder.read(node, hPP);
		assetLocation = AssetLoactiontholder;
	}

	void readplRender(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<PlaylistRender> plRenderholder("");
		plRenderholder.read(node, hPP);
		plRender = plRenderholder;
	}
	void readAssociateProvider(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<AssociateProvider> plAPIDholder("");
		plAPIDholder.read(node, hPP);
		plAPID = plAPIDholder;
	}

	void readResourceRestriction(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<ResourceRestriction> nvholder("");
		nvholder.read(node, hPP);
		resRestriction = nvholder;
	}
    void registerNothing(const std::string&){}
};

typedef std::list<Config::Holder<Urlpattern> > URLPATLIST;

struct  MODPlugIn
{
	// defines the ice log properties
	std::string logFileName;
	int32 LogLevel;
	int32 LogSize;
	int32 LogCount;
	MODPlugIn()
	{
		logFileName = "MODPlugIn.log";
		LogLevel = 7;
		LogSize = 10240000;
		LogCount =5;
	}
	static void structure(Config::Holder<MODPlugIn>& holder)
	{
		holder.addDetail("", "filename", &MODPlugIn::logFileName, "MODPlugIn.log", Config::optReadOnly);
		holder.addDetail("", "level", &MODPlugIn::LogLevel, "7", Config::optReadWrite);
		holder.addDetail("", "size", &MODPlugIn::LogSize, "10240000", Config::optReadWrite);
		holder.addDetail("", "count", &MODPlugIn::LogCount, "5", Config::optReadWrite);
	}
};
struct StorageLinkType
{
	std::string type;
	static void structure(Config::Holder<StorageLinkType>& holder)
	{
		holder.addDetail("", "type", &StorageLinkType::type, NULL, Config::optReadOnly);
	}
};
struct LibraryAsset
{
	int mandatory;
	std::vector<std::string>allowedStorageLinks;

	static void structure(Config::Holder<LibraryAsset>& holder)
	{
		holder.addDetail("", "mandatory", &LibraryAsset::mandatory, "0", Config::optReadOnly);
		holder.addDetail("AllowedStorageLink", &LibraryAsset::readStorageLinkType, &LibraryAsset::registerNothing, Config::Range(0, -1));
	}
	LibraryAsset()
	{
		mandatory = 0;
	}
	void readStorageLinkType(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<StorageLinkType> linkType("");
		linkType.read(node, hPP);
		allowedStorageLinks.push_back(linkType.type);
	}
	void registerNothing(const std::string&){}
};

struct Streams
{
	std::string name;
	static void structure(Config::Holder<Streams>& holder)
	{
		holder.addDetail("", "name", &Streams::name, NULL, Config::optReadOnly);
	}
};
struct AllowedStreamers 
{
	int mandatory;
	std::vector<std::string>streams ;

	static void structure(Config::Holder<AllowedStreamers>& holder)
	{
		holder.addDetail("", "mandatory", &AllowedStreamers::mandatory, "0", Config::optReadOnly);
		
		holder.addDetail("Streamer", &AllowedStreamers::readStreamers, &AllowedStreamers::registerNothing, Config::Range(0, -1));
	}
	AllowedStreamers()
	{
		mandatory = 0;
	}
	void readStreamers(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Streams> stream("");
		stream.read(node, hPP);
		streams.push_back(stream.name);
	}
	void registerNothing(const std::string&){}
};
struct PlugInFiles
{
	std::string path;
	int enable;
	static void structure(Config::Holder<PlugInFiles>& holder)
	{
		holder.addDetail("", "file", &PlugInFiles::path, NULL, Config::optReadOnly);
		holder.addDetail("", "enable", &PlugInFiles::enable, NULL, Config::optReadOnly);
	}
	void registerNothing(const std::string&){}
};
struct PlugIns 
{
	std::vector<std::string>files;

	static void structure(Config::Holder<PlugIns>& holder)
	{
		holder.addDetail("MHO", &PlugIns::readPlugIns, &PlugIns::registerNothing, Config::Range(0, -1));
	}
	PlugIns()
	{
	}
	void readPlugIns(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<PlugInFiles> plugInFiles("");
		plugInFiles.read(node, hPP);
		if(plugInFiles.enable)
			files.push_back(plugInFiles.path);
	}
	void registerNothing(const std::string&){}
};
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

struct AssetRulesCfg
{
	std::string rule;
	static void structure(Config::Holder<AssetRulesCfg> &holder)
	{
		holder.addDetail("","regex",&AssetRulesCfg::rule, NULL,Config::optReadOnly);
	}
};

struct ServiceGroupPumpCfg
{
	int enable;
	std::string ip;
	int port;
	int32 timeout;
	int32 interval;
	std::string sgPumpformat;
	std::vector<std::string> AssetRules;

	static void structure(Config::Holder<ServiceGroupPumpCfg> &holder)
	{
		holder.addDetail("","enable",&ServiceGroupPumpCfg::enable, "0",Config::optReadOnly);
		holder.addDetail("","ip",&ServiceGroupPumpCfg::ip, NULL,Config::optReadOnly);
		holder.addDetail("","port",&ServiceGroupPumpCfg::port, "0",Config::optReadOnly);
		holder.addDetail("","timeout",&ServiceGroupPumpCfg::timeout, "5000",Config::optReadOnly);
		holder.addDetail("","interval",&ServiceGroupPumpCfg::interval, "1000",Config::optReadOnly);
		holder.addDetail("","sgPumpformat",&ServiceGroupPumpCfg::sgPumpformat,"SmartCardID:${smartCartID};ServiceGroup:${serviceGroup}",Config::optReadOnly);
		holder.addDetail("AssetRules/Rule",&ServiceGroupPumpCfg::readAssetRules,&ServiceGroupPumpCfg::registerNothing);
	}
	void readAssetRules(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
	{
		using namespace ZQ::common::Config;
		Holder<AssetRulesCfg> propHolder;
		propHolder.read(node, hPP);
		AssetRules.push_back(propHolder.rule);
	}
	void registerNothing(const std::string&){}
};

struct ModCfg
{
	std::string  InstanceID;
	std::string   mrtSSnetId;
	int32 enableCrushDump;
	std::string crushDumpPath;

	//
	int32  defaultBandWidth;
	
	// the directory of safeStore
	std::string safeStorePath;
	std::string runtimePath;
	std::string  checkpointPeriod;
	std::string  dbRecoverFatal;
	
	// stores the ice properties
	std::map<std::string, std::string> icePropMap;
	
	// stores the parameters to authorization
	int32 useTestAuthParam;
	std::map<std::string, std::string> testAuthParamMap;
	
	// defines the ice storm endpoint
	std::string iceStormEndPoint;
	
	// defines the ice log properties
	int32 enableIceTrace;
	std::string iceLogPath;
	int32 iceLogLevel;
	int32 iceLogSize;
	int32 iceLogCount;
	
	// defines the local endpoint of adapter
	std::string adapEndPoint;
	
//	int mandatory;
//	std::string allowedstoragelink;
	
	// the endpoint of the lam system
	//std::string lamEndPoint;
	
	// defines whether or not to use the configured testing items
	// as the asset elements gained from the LAM system
//	int32 useTestItem;
//	TestItemInfo testItems;

	TestPlaylist testItems;

	// defines whether or not to use the configured testing Autorization
//	int useTestAuthor;
//	PARAMMAP authorProp;
	TestAuthorizeParam testAuthor;

	// defines the authorization properties
	//bool oteAuthEnable;
	//std::string oteEndPoint;
	
	// the timeout value of mod purchase
	int32 purchaseTimeout; // ms
	// defines the size of the active queue of purchase's evictor
	int32 evctSize;
	// defines the initial size when load from db
	int32 initRecordBufferSize;
	// defines the maximal time to live of the purchase in second
	int32 maxTTL;
	
	LibraryAsset libraryAsset;

	AllowedStreamers allowedStreams;
    URLPATLIST urlpattern;

	PlugIns plugins;
	MODPlugIn modPlugIn;
	MRTStreamerCfg mrtStreamServiceCfg;

	ServiceGroupPumpCfg serviceGroupPumpCfg;

    ModCfg()
	{
		testItems.enable = 0;
		testAuthor.enable = 0;
		defaultBandWidth =0;
	}
    static void structure(Config::Holder<ModCfg> &holder)
    {
		holder.addDetail("", "netid", &ModCfg::InstanceID, "1", Config::optReadOnly);
		holder.addDetail("", "MRTSSNetId", &ModCfg::mrtSSnetId, "", Config::optReadOnly);

		holder.addDetail("CrashDump", "enable", &ModCfg::enableCrushDump, "0", Config::optReadOnly);
		holder.addDetail("CrashDump", "path", &ModCfg::crushDumpPath, "C:\\TianShan\\CrashDump\\", Config::optReadOnly);
		
		holder.addDetail("Default", "bandWidth", &ModCfg::defaultBandWidth, "0", Config::optReadOnly);

		holder.addDetail("Database", "path", &ModCfg::safeStorePath, "C:\\TianShan\\Data\\", Config::optReadOnly);
		holder.addDetail("Database", "runtimePath", &ModCfg::runtimePath, "C:\\TianShan\\Data\\runtime", Config::optReadOnly);
		holder.addDetail("Database", "CheckpointPeriod", &ModCfg::checkpointPeriod, "240", Config::optReadOnly);
		holder.addDetail("Database", "DbRecoverFatal", &ModCfg::dbRecoverFatal, "1", Config::optReadOnly);

		holder.addDetail("IceProperties/prop", &ModCfg::readProp, &ModCfg::registerNothing);
		
        holder.addDetail("EventChannel", "endpoint", &ModCfg::iceStormEndPoint, NULL, Config::optReadOnly);
		
		holder.addDetail("IceLog", "enable", &ModCfg::enableIceTrace,"1",Config::optReadOnly);
		holder.addDetail("IceLog", "path", &ModCfg::iceLogPath, "C:\\TianShan\\Logs\\", Config::optReadOnly);
		holder.addDetail("IceLog", "level", &ModCfg::iceLogLevel, "7", Config::optReadWrite);
		holder.addDetail("IceLog", "size", &ModCfg::iceLogSize, "10240000", Config::optReadWrite);
		holder.addDetail("IceLog", "count", &ModCfg::iceLogCount, "5", Config::optReadWrite);

		holder.addDetail("TestPlaylist",&ModCfg::readtestItem, &ModCfg::registerNothing, Config::Range(0,1));
		holder.addDetail("TestAuthorizeParam", &ModCfg::readTestAuthor, &ModCfg::registerNothing,Config::Range(0,1));

        holder.addDetail("Bind", "endpoint", &ModCfg::adapEndPoint,DEFAULT_ENDPOINT_MODSvc, Config::optReadOnly);
		
		holder.addDetail("LibraryAsset", &ModCfg::readLibraryAsset, &ModCfg::registerNothing, Config::Range(0, 1));
		holder.addDetail("AllowedStreamers", &ModCfg::readAllowedStreams, &ModCfg::registerNothing, Config::Range(0, 1));

//		holder.addDetail("LibraryAsset", "mandatory", &ModCfg::mandatory,"0", Config::optReadOnly);
//		holder.addDetail("LibraryAsset/AllowedStorageLink", "type", &ModCfg::allowedstoragelink,"SeaChange.NSS.C2Transfer", Config::optReadOnly);
	
		holder.addDetail("PurchaseRecord", "cacheSize", &ModCfg::evctSize, "500", Config::optReadOnly);
		holder.addDetail("PurchaseRecord", "timeout", &ModCfg::purchaseTimeout, "1800000", Config::optReadWrite);
		holder.addDetail("PurchaseRecord", "initRecordBufferSize", &ModCfg::initRecordBufferSize, "20000", Config::optReadOnly);
		holder.addDetail("PurchaseRecord", "maxTTL", &ModCfg::maxTTL, "10800", Config::optReadWrite);

//		holder.addDetail("DefaultAppPath", &ModCfg::readDefaultPath, &ModCfg::registerDefaultAppPath,Config::Range(1,1));
//      holder.addDetail("AppPath", &ModCfg::readPath, &ModCfg::registerAppPath);
		holder.addDetail("UrlPattern", &ModCfg::readUrlPattern, &ModCfg::registerUrlPattern, Config::Range(1, -1));

		holder.addDetail("MODPlugIn", &ModCfg::readMODPlugIn, &ModCfg::registerNothing, Config::Range(0, 1));

		holder.addDetail("PlugIns", &ModCfg::readPlugIns, &ModCfg::registerNothing);

		holder.addDetail("MRTStreamer", &ModCfg::readMRTStreamer, &ModCfg::registerNothing, ZQ::common::Config::Range(0, 1));

		holder.addDetail("ServiceGroupPump", &ModCfg::readServiceGroupPump, &ModCfg::registerNothing, ZQ::common::Config::Range(0, 1));

    };
	void readProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<Config::NVPair> nvholder("");
        nvholder.read(node, hPP);
        icePropMap[nvholder.name] = nvholder.value;
    }
	void readtestItem(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {   
        Config::Holder<TestPlaylist> nvholder("");
        nvholder.read(node, hPP);
		testItems = nvholder;
    }
	void readTestAuthor(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<TestAuthorizeParam> nvholder("");
        nvholder.read(node, hPP);
        testAuthor = nvholder;
    }
	void readMODPlugIn(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<MODPlugIn> nvholder("");
		nvholder.read(node, hPP);
		modPlugIn = nvholder;
	}
	void readPlugIns(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<PlugIns> nvholder("");
		nvholder.read(node, hPP);
		plugins = nvholder;
	}
	void readLibraryAsset(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{   
		Config::Holder<LibraryAsset> nvholder("");
		nvholder.read(node, hPP);
		libraryAsset = nvholder;
	}
	void readAllowedStreams(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{   
		Config::Holder<AllowedStreamers> nvholder("");
		nvholder.read(node, hPP);
		allowedStreams = nvholder;
	}
/*	void readDefaultPath(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<AppPath> pathholder;
        pathholder.read(node, hPP);	
		
		pathholder.path = DEFAULTAPPPATH;
		pathholder.enable = 1;
		DefaultAppPath = pathholder;
    }
*/

    void readUrlPattern(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<Urlpattern> urlpatternholder("pattern");
        urlpatternholder.read(node, hPP);
        
		if(urlpatternholder.enable == 0 )
		{
/*			glog(ZQ::common::Log,
				"the UrlPattern is disenable, Urlpattern = %s",
				urlpatternholder.pattern.c_str());*/
			return;
		}

		/*check duplicate URLPattern*/
		URLPATLIST::iterator itor = urlpattern.begin();
		while(itor != urlpattern.end())
		{
			if(itor->pattern == urlpatternholder.pattern)
			{
				throwf<CfgException>(EXFMT(CfgException, "the same UrlPattern info, [UrlPattern = %s]"), urlpatternholder.pattern.c_str());
			}
			else
			{
				itor++;
			}
		}
		int ret = 0;
		/*sort for the URLPattern with priority, 0 is the highest priority */
		for(itor = urlpattern.begin(); itor != urlpattern.end(); itor++)
		{
			if(urlpatternholder.priority <= (*itor).priority)
			{
				urlpattern.insert(itor, urlpatternholder);
				ret = 1;
				break;
			}
		}
		if(!ret)
		{
			urlpattern.push_back(urlpatternholder);
		}       
    }

/*	void registerDefaultAppPath(const std::string &full_path)
    {
		DefaultAppPath.snmpRegister(full_path);
    }
*/
	void registerUrlPattern(const std::string &full_path)
    {
       for(URLPATLIST::iterator it = urlpattern.begin(); it != urlpattern.end(); ++it)
            (*it).snmpRegister(full_path);
    }

	void readMRTStreamer(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<MRTStreamerCfg> mrtCfgholder("");
		mrtCfgholder.setNetId(mrtSSnetId);
		mrtCfgholder.read(node, hPP);
		mrtStreamServiceCfg = mrtCfgholder;
	}

	void readServiceGroupPump(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<ServiceGroupPumpCfg> PumpCfgholder("");
		PumpCfgholder.read(node, hPP);
		serviceGroupPumpCfg = PumpCfgholder;
	}

    void registerNothing(const std::string&){}
};

typedef std::map< std::string, Config::Holder<ModCfg> > ModCfgMap;

struct MODCFGGROUP
{
	ModCfgMap modcfg;
	static void structure(Config::Holder<MODCFGGROUP> &holder)
    {		
		holder.addDetail("MODService", &MODCFGGROUP::readModfig, &MODCFGGROUP::registerModfig,Config::Range(1,-1));
    };
	void readModfig(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        //Config::Holder<ModCfg> MODCfgholder("netid");
		Config::Holder<ModCfg> MODCfgholder;
        MODCfgholder.read(node, hPP);
		
		ModCfgMap::iterator itor = modcfg.find(MODCfgholder.InstanceID);
		if(itor == modcfg.end())
			modcfg[MODCfgholder.InstanceID] = MODCfgholder;
		else
		{
			throwf<CfgException>(EXFMT(CfgException, "the same MODService info, [netID = %s ]"), MODCfgholder.InstanceID.c_str());
		}
    }

	void registerModfig(const std::string &full_path)
    {
		for(ModCfgMap::iterator it = modcfg.begin(); it != modcfg.end(); ++it)
            (it->second).snmpRegister(full_path);
    }
};
static void showAppDataPattern(const AppDataPattern& appdatapat)
{
	using namespace std;
	std::string param;
	std::string pattern;
	cout << "\t param: " << appdatapat.param << "\n";
	cout << "\t pattern: " << appdatapat.pattern << "\n";
	cout << "\t\t params: ";
	for(PARAMMAP::const_iterator it_appdatapat= appdatapat.appDataParammap.begin(); it_appdatapat != appdatapat.appDataParammap.end(); ++it_appdatapat)
	{
		cout << "(" << it_appdatapat->first << " , " << it_appdatapat->second.value << "), ";
	}
	cout<< "\n";
}
static void showUrlPattern(const Urlpattern& urlpattern)
{
    using namespace std;
    cout << "Urlpattern:\n";
    cout << "\t enable: " << urlpattern.enable << "\n";
    cout << "\t path: " << urlpattern.pattern << "\n";
	cout << "\t priorty: " << urlpattern.priority << "\n";
    cout << "\t Authorization: \n";
    cout << "\t\t enable: " << urlpattern.Authenable << "\n";
    cout << "\t\t entry: " << urlpattern.authEntry << "\n";
    cout << "\t\t params: ";
    for(PARAMMAP::const_iterator it_auth = urlpattern.authParams.begin(); it_auth != urlpattern.authParams.end(); ++it_auth)
    {
        cout << "(" << it_auth->first << " , " << it_auth->second.value << "), ";
    }
	cout << "\t\t AuthAppUrlPattern: \n";
	for(AppDataPatternMAP::const_iterator it_authappdata = urlpattern.authAppDataMap.begin(); it_authappdata != urlpattern.authAppDataMap.end(); ++it_authappdata)
	{
		showAppDataPattern((*it_authappdata).second);
	}
	
    cout << "\t PlayList: \n";
    cout << "\t\t entry: " << urlpattern.playlistEntry << "\n";
    cout << "\t\t params: ";
    for(PARAMMAP::const_iterator  it_pl = urlpattern.playlistParams.begin(); it_pl != urlpattern.playlistParams.end(); ++it_pl)
    {
        cout << "(" << it_pl->first << " , " << it_pl->second.value << "), ";
    }
	cout << "\t\t PlayListAppUrlPattern: \n";
	for(AppDataPatternMAP::const_iterator it_plappdata = urlpattern.playListAppDataMap.begin(); it_plappdata != urlpattern.playListAppDataMap.end(); ++it_plappdata)
	{
		showAppDataPattern((*it_plappdata).second);
	}
    cout<<"\n\n";
}
static void showModCfg(const ModCfg& cfg)
{
    using namespace std;
    cout << "MODConfig: \n";
	cout << "netid: " <<cfg.InstanceID << "\n";
	cout << "\t enableCrushDump \t" << cfg.enableCrushDump<< "\n";
	cout << "\t crushDumpPath \t" << cfg.crushDumpPath << "\n";
	cout << "\t safeStorePath \t" << cfg.safeStorePath << "\n";
	cout << "\t iceStormEndPoint \t" << cfg.iceStormEndPoint <<"\n";
	cout << "\t enableIceTrace \t" << cfg.enableIceTrace << "\n";
	cout << "\t iceLogPath \t" << cfg.iceLogPath << "\n";
	cout << "\t iceLogLevel \t" << cfg.iceLogLevel << "\n";

    cout << "\t MOD endpoint: \t " << cfg.adapEndPoint << "\n";

	cout << "\t purchaseTimeout \t" << cfg.purchaseTimeout << "\n";
	cout << "\t evctSize \t" << cfg.evctSize << "\n";
	cout << "\t initRecordBufferSize \t" << cfg.initRecordBufferSize << "\n";

   	cout << "\t IceProperties" <<"\n";
	for(std::map<std::string, std::string>::const_iterator it_pl = cfg.icePropMap.begin(); it_pl != cfg.icePropMap.end(); ++it_pl)
    {
        cout << "(" << it_pl->first << " , " << it_pl->second << "), "<<"\n";
    }
    cout << "\t testPlayList \t"<<"\n";
	cout << "\t TestPlaylist enable\t"<< cfg.testItems.enable << "\n";
	for(std::vector< Config::Holder<TestItemInfo> >::const_iterator it_TPL = cfg.testItems.testitems.begin(); it_TPL != cfg.testItems.testitems.end(); ++it_TPL)
    {
        cout << "(" << it_TPL->name << " , " << it_TPL->bandWidth << " , " << it_TPL->cueIn << " , " << it_TPL->cueOut << "), "<<"\n";
		for(PARAMMAP::const_iterator it_user = it_TPL->userpropmap.begin(); it_user != it_TPL->userpropmap.end(); it_user++)
		{
			cout << "UserProp Name = "<< it_user->first << " Value = " << it_user->second.value << "\n";
		}
    }

	cout << "\t testAuthorization  \t"<<"\n";
	cout << "\t testAuthorization enable \t"<< cfg.testAuthor.enable << "\n";
	for(PARAMMAP::const_iterator it_TAU = cfg.testAuthor.authorParam.begin(); it_TAU != cfg.testAuthor.authorParam.end(); ++it_TAU)
    {
        cout << "(" << it_TAU->first << " , " << it_TAU->second.value << "), "<<"\n";
    }
	
//  cout << " DefaultAppPaths: \n"; 
//	showPath(cfg.DefaultAppPath);
	cout << cfg.urlpattern.size() << " UrlPattern: \n";
	URLPATLIST::const_iterator itor = (cfg.urlpattern).begin();
	while(itor != cfg.urlpattern.end())
	{
		showUrlPattern(*itor);
		itor++;
	}
}
static void showMODCfgGroup(const MODCFGGROUP& cfggroup)
{
	for(std::map< std::string, Config::Holder<ModCfg> >::const_iterator it_TPL = cfggroup.modcfg.begin(); 
	    it_TPL != cfggroup.modcfg.end(); ++it_TPL)
    {
 //       cout << it_TPL->first.c_str()<< "\n";
		showModCfg(it_TPL->second);
    }
}
#endif // #define __ZQMODApplication_ModCfgLoader_H__


