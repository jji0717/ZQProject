#include "C2LocatorConf.h"
#include <strHelper.h>

namespace ZQTianShan{
namespace CDN{
struct ContentInfoConf
{
    std::string name;
    int32 bandwidth;
    std::string volumeList; // with SPACE as delimiter

    static void structure(ZQ::common::Config::Holder<ContentInfoConf>& holder)
    {
        holder.addDetail("", "name", &ContentInfoConf::name);
        holder.addDetail("", "bandwidth", &ContentInfoConf::bandwidth);
        holder.addDetail("", "volumeList", &ContentInfoConf::volumeList);
    }
};

struct LocateForwardConf
{
    int32 enabled;
    std::string excludeStates;
    int32 onOutOfResource;
    int32 maxHop;
    std::string url;
    static void structure(ZQ::common::Config::Holder<LocateForwardConf>& holder)
    {
        holder.addDetail("", "enable", &LocateForwardConf::enabled);
        holder.addDetail("", "excludeStates", &LocateForwardConf::excludeStates, "InService,ProvisioningStreamable");
        holder.addDetail("", "onOutOfResource", &LocateForwardConf::onOutOfResource, "0");
        holder.addDetail("", "maxHop", &LocateForwardConf::maxHop, "2");
        holder.addDetail("Server", "url", &LocateForwardConf::url);
    }
};

struct ObjectResolutionConf:public ObjectResolution
{
    int32 enabled;
    static void structure(ZQ::common::Config::Holder<ObjectResolutionConf>& holder)
    {
        holder.addDetail("", "enabled", &ObjectResolutionConf::enabled);
        holder.addDetail("", "type", &ObjectResolutionConf::type);
        holder.addDetail("", "identifier", &ObjectResolutionConf::identifier);
        holder.addDetail("", "providerId", &ObjectResolutionConf::providerId);
        holder.addDetail("", "assetId", &ObjectResolutionConf::assetId);
        holder.addDetail("", "extension", &ObjectResolutionConf::extension);
    }
};

struct StorageConf
{
    std::string netid;
    std::string endpoint;
    static void structure(ZQ::common::Config::Holder<StorageConf>& holder)
    {
        holder.addDetail("", "netid", &StorageConf::netid);
        holder.addDetail("", "endpoint", &StorageConf::endpoint);
    }
};

void C2LocatorConf::readObjectResolution(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ZQ::common::Config::Holder<ObjectResolutionConf> reader;
    reader.read(node, hPP);
    if(reader.enabled)
        objectResolutions.push_back(reader);
}

void C2LocatorConf::readTestContent(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ZQ::common::Config::Holder<ContentInfoConf> reader;
    reader.read(node, hPP);
    testContent.name = reader.name;
    testContent.bandwidth = reader.bandwidth;
    ZQ::common::stringHelper::SplitString(reader.volumeList, testContent.volumeList, " ");
}


struct VSISFixupConf:public VSISFixup
{
	std::string subtypes;
	static void structure(ZQ::common::Config::Holder<VSISFixupConf>& holder)
	{
		holder.addDetail("", "subTypeToPID", &VSISFixupConf::subtypes);		
	}
};

void C2LocatorConf::readFixupVSIS(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<VSISFixupConf> reader;
	reader.read(node, hPP);	
	//ZQ::common::stringHelper::SplitString(reader.volumeList, testContent.volumeList, " ");
	std::vector<std::string> subtypenames;
	ZQ::common::stringHelper::SplitString( reader.subtypes , subtypenames, ";" );
	std::vector<std::string>::const_iterator it = subtypenames.begin();
	for( ; it != subtypenames.end() ; it ++ )
	{
		vsisFixup.illegalSubtypes.insert( *it );
	}
}

void C2LocatorConf::structure(ZQ::common::Config::Holder<C2LocatorConf>& holder)
{
    using namespace ZQ::common;
    holder.addDetail("default/EventChannel", "endpoint", &C2LocatorConf::icestormEP, NULL, Config::optReadOnly, "C2Locator/EventChannel/endpoint");
    holder.addDetail("default/Database", "path", &C2LocatorConf::dbPath, NULL, Config::optReadOnly, "C2Locator/Database/path");
    holder.addDetail("default/Database", "runtimePath", &C2LocatorConf::dbRuntimePath, NULL, Config::optReadOnly, "C2Locator/Database/runtimePath");

    holder.addDetail("C2Locator", "uri", &C2LocatorConf::alternateLocateUriExp, "", Config::optReadOnly);
    holder.addDetail("C2Locator", "notice", &C2LocatorConf::tianshanNoticeEnabled, "1", Config::optReadOnly);
    holder.addDetail("C2Locator/Log", "level", &C2LocatorConf::loglevel, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/Log", "size", &C2LocatorConf::logsize, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/Log", "buffer", &C2LocatorConf::logbuffersize, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/Log", "count", &C2LocatorConf::logCount, "5", Config::optReadOnly);

    holder.addDetail("C2Locator/Bind", "endpoint", &C2LocatorConf::endpoint, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/ThreadPool", "size", &C2LocatorConf::watchThreadPoolSize, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort", "updateIntervalSec", &C2LocatorConf::replicaReportIntervalSec, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort", "selectionRetryMax", &C2LocatorConf::selectionRetryMax, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort", "mask4", &C2LocatorConf::subnetMask4, "0.0.0.0", Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort", "ignoreExclusion", &C2LocatorConf::ignoreExclusion, "0", Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort", "ignoreIngressCapacity", &C2LocatorConf::ignoreIngressCapacity, "0", Config::optReadOnly);
    // penalty setting
    holder.addDetail("C2Locator/TransferPort/Penalty", "punishmentUnit", &C2LocatorConf::penaltyPunishmentUnit, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort/Penalty", "reducingIntervalMsec", &C2LocatorConf::penaltyReducingIntervalMsec, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort/Penalty", "retryLimit", &C2LocatorConf::penaltyRetryLimit, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferPort/Penalty", "max", &C2LocatorConf::penaltyMax, NULL, Config::optReadOnly);

    holder.addDetail("C2Locator/TransferOption", "indexFileRate", &C2LocatorConf::indexFileTransferRate, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferOption", "transferAheadPercent", &C2LocatorConf::transferAheadRatePercent, NULL, Config::optReadOnly);
	holder.addDetail("C2Locator/TransferOption", "exposeIndex", &C2LocatorConf::exposeAssetIndexData, "0", Config::optReadOnly);
	holder.addDetail("C2Locator/TransferOption", "authEnable", &C2LocatorConf::authEnable, "0", Config::optReadOnly);
	holder.addDetail("C2Locator/TransferOption", "checkExpirationInAuth", &C2LocatorConf::checkExpirationInAuth, "0", Config::optReadOnly);	
	holder.addDetail("C2Locator/TransferOption", "authKeyfile", &C2LocatorConf::authKeyfile, "", Config::optReadOnly);

    holder.addDetail("C2Locator/Content", "fullName", &C2LocatorConf::contentFullNameFmt, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/Content", "lamEndpoint", &C2LocatorConf::lamEndpoint, "", Config::optReadOnly);
    holder.addDetail("C2Locator/Content", "contentLibraryEndpoint", &C2LocatorConf::contentLibEndpoint, "", Config::optReadOnly);
    holder.addDetail("C2Locator/Content", "overwriteAvailableRange", &C2LocatorConf::overwriteAvailableRange, "1", Config::optReadOnly);
    holder.addDetail("C2Locator/Content/ObjectResolution", &C2LocatorConf::readObjectResolution, &C2LocatorConf::registerNothing);
	holder.addDetail("C2Locator/Content/FixupVSIS", &C2LocatorConf::readFixupVSIS, &C2LocatorConf::registerNothing);
	holder.addDetail("C2Locator/Content/Storage", &C2LocatorConf::readStorage, &C2LocatorConf::registerNothing);

	holder.addDetail("C2Locator/TransferSession", "timeOut", &C2LocatorConf::transferSessionTimeOut, NULL, Config::optReadOnly);
    holder.addDetail("C2Locator/TransferSession", "cacheSize", &C2LocatorConf::transferSessionEvictorSize, "100", Config::optReadOnly);
	
	//cacheserver setting
	holder.addDetail("C2Locator/CacheServer", "fullname", &C2LocatorConf::cacheserverfullname, "CacheEndpoint", Config::optReadOnly);
	holder.addDetail("C2Locator/CacheServer", "cachesEndpoint", &C2LocatorConf::cacheserverendpoint, "CacheFacade:tcp -h 127.0.0.1 -p 60006", Config::optReadOnly);
	holder.addDetail("C2Locator/CacheServer", "uri", &C2LocatorConf::cacheserveruri, "/?CacheServer", Config::optReadOnly);
	holder.addDetail("C2Locator/CacheServer", "forwardURL", &C2LocatorConf::cacheforwardurl, "", Config::optReadOnly);
	
	holder.addDetail("C2Locator/TransferSession", "freezeCheckPointPeriod", &C2LocatorConf::freezeCheckPointPeriod, "240", Config::optReadOnly);
	holder.addDetail("C2Locator/TransferSession", "freezeSavePeriod", &C2LocatorConf::freezeSavePeriod, "60000", Config::optReadOnly);
	holder.addDetail("C2Locator/TransferSession", "freezeSaveSizeTrigger", &C2LocatorConf::freezeSaveSizeTrigger, "50", Config::optReadOnly);
	

    holder.addDetail("C2Locator/IgnoreLAMWithContent", &C2LocatorConf::readTestContent, &C2LocatorConf::registerNothing, ZQ::common::Config::Range(0, 1));

    holder.addDetail("C2Locator/LocateForward", &C2LocatorConf::readLocateForward, &C2LocatorConf::registerNothing, ZQ::common::Config::Range(0, 1));

    holder.addDetail("default/PublishedLogs/Log", &C2LocatorConf::readPublishedLog, &C2LocatorConf::registerNothing);
    holder.addDetail("default/IceProperties/prop", &C2LocatorConf::readIceProp, &C2LocatorConf::registerNothing);
    holder.addDetail("C2Locator/CheckRemoteHealth", &C2LocatorConf::readCheckRH, &C2LocatorConf::registerNothing, ZQ::common::Config::Range(0, 1));
	holder.addDetail("C2Locator/AssetStack", &C2LocatorConf::readAssetStack, &C2LocatorConf::registerNothing, ZQ::common::Config::Range(0, 1));

}

void C2LocatorConf::readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    using namespace ZQ::common::Config;
    Holder<NVPair> nvHolder;
    nvHolder.read(node, hPP);
    iceProps[nvHolder.name] = nvHolder.value;
}

void C2LocatorConf::readLocateForward(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ZQ::common::Config::Holder<LocateForwardConf> reader;
    reader.read(node, hPP);
    forwardExcludeStates = reader.excludeStates;
    if(reader.enabled) {
        forwardUrl = reader.url;
    } else {
        forwardUrl.clear();
    }
    forwardOnOutOfResource = reader.onOutOfResource;
    maxHop = reader.maxHop;
}

void C2LocatorConf::readPublishedLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
    ZQ::common::Config::Holder<PublishedLog> reader;
    reader.read(node, hPP);
    pubLogs.push_back(reader);
}

void PublishedLog::structure(ZQ::common::Config::Holder<PublishedLog>& holder) {
    holder.addDetail("", "path", &PublishedLog::path);
    holder.addDetail("", "syntax", &PublishedLog::syntax);
    holder.addDetail("", "key", &PublishedLog::key);
    holder.addDetail("", "type", &PublishedLog::type);

    holder.addDetail("property", &PublishedLog::readProperty, &PublishedLog::registerNothing);
}

void  PublishedLog::readProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
    using namespace ZQ::common;
    Config::Holder<Config::NVPair> reader;
    reader.read(node, hPP);
    properties[reader.name] = reader.value;
}
void C2LocatorConf::readStorage( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP ) {
    using namespace ZQ::common;
    Config::Holder<StorageConf> reader;
    reader.read(node, hPP);
    storageMap[reader.netid] = reader.endpoint;
}
void C2LocatorConf::readCheckRH( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP ) {
    using namespace ZQ::common;
    Config::Holder<CheckRHConf> reader;
    reader.read(node, hPP);
    checkRH = reader;
}
void C2LocatorConf::readAssetStack(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	using namespace ZQ::common;
	Config::Holder<AssetStackConf> reader;
	reader.read(node, hPP);
	assetStack = reader;
}

}} // namespace ZQTianShan::CDN
