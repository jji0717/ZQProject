#ifndef __C2Locator_Conf_H__
#define __C2Locator_Conf_H__
#include <set>
#include <ConfigHelper.h>


namespace ZQTianShan{
namespace CDN{

struct ContentInfo
{
    std::string name;
    int32 bandwidth;
    std::vector<std::string> volumeList;
};

struct ObjectResolution
{
    std::string type;
    std::string identifier;
    std::string providerId;
    std::string assetId;
    std::string extension;
};

struct PublishedLog
{
    std::string path;
    std::string syntax;
    std::string key;
    std::string type;

    std::map<std::string, std::string> properties;
    static void structure(ZQ::common::Config::Holder<PublishedLog>& holder);
    void readProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerNothing(const std::string&){}
};
typedef std::vector<PublishedLog> PublishedLogs;

struct VSISFixup
{
	std::set<std::string>	illegalSubtypes;
};

struct CheckRHConf
{
    std::string script;
    int32 timeout;
    int32 interval;
    static void structure(ZQ::common::Config::Holder<CheckRHConf>& holder) {
        holder.addDetail("", "script", &CheckRHConf::script);
        holder.addDetail("", "timeout", &CheckRHConf::timeout);
        holder.addDetail("", "interval", &CheckRHConf::interval);
    }
};
struct ProviderConf
{
	std::string name;
	static void structure(ZQ::common::Config::Holder<ProviderConf>& holder) {
		holder.addDetail("", "name", &ProviderConf::name);
	}
};
struct AssetStackConf
{
	int32 enable;
	std::vector<std::string> paids;
	static void structure(ZQ::common::Config::Holder<AssetStackConf>& holder) {
		holder.addDetail("", "enable", &AssetStackConf::enable, "0");
		holder.addDetail("Provider", &AssetStackConf::readPaids, &AssetStackConf::registerNothing, ZQ::common::Config::Range(0, -1));
	}

	AssetStackConf()
	{
		enable = 0;
	}

	void readPaids(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		using namespace ZQ::common;
		Config::Holder<ProviderConf> reader;
		reader.read(node, hPP);
		paids.push_back(reader.name);
	}

	 void registerNothing(const std::string&){}
};
struct C2LocatorConf
{
    std::string icestormEP;
    std::string dbPath;
    std::string dbRuntimePath;

    int32 loglevel;
    int32 logsize;
    int32 logbuffersize;
    int32 logCount;

    std::string endpoint;
    std::string lamEndpoint;
    std::string contentLibEndpoint;
    int32 overwriteAvailableRange;

    int32 watchThreadPoolSize;
    int32 replicaReportIntervalSec;
    int32 selectionRetryMax;
    std::string subnetMask4;
    int32 ignoreExclusion;
    int32 ignoreIngressCapacity;

    // the penalty setting
    int32 penaltyPunishmentUnit;
    int32 penaltyReducingIntervalMsec;
    int32 penaltyRetryLimit;
    int32 penaltyMax;

    int32 indexFileTransferRate;
    int32 transferAheadRatePercent;
	int32 exposeAssetIndexData;
	int32 authEnable;
	int32 checkExpirationInAuth;
	std::string authKeyfile;

	int32 transferSessionTimeOut;
    int32 transferSessionEvictorSize;

	//cacheserve setting
	std::string cacheserverfullname;
	std::string cacheserverendpoint;
	std::string cacheserveruri;
	std::string cacheforwardurl;

	std::string freezeCheckPointPeriod;
	std::string freezeSavePeriod;
	std::string freezeSaveSizeTrigger;

	std::string contentFullNameFmt;

    std::vector<ObjectResolution> objectResolutions;

    // test content info
    ContentInfo testContent;

    std::string alternateLocateUriExp;
    std::string forwardUrl;
    std::string forwardExcludeStates;
    int32 forwardOnOutOfResource;
    int32 maxHop;

    int32 tianshanNoticeEnabled;

    CheckRHConf checkRH;

    // published logs
    PublishedLogs pubLogs;

	VSISFixup	vsisFixup;

    std::map<std::string, std::string> storageMap;

    std::map<std::string, std::string> iceProps;
    void readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	AssetStackConf assetStack;
    void readObjectResolution(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void readTestContent(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void readLocateForward(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void readPublishedLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readFixupVSIS( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );
    void readStorage( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );
    void readCheckRH(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readAssetStack(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerNothing(const std::string&){}

    static void structure(ZQ::common::Config::Holder<C2LocatorConf>& holder);
};

}} // namespace ZQTianShan::CDN
#endif
