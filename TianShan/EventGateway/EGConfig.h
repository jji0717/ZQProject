#ifndef __TIANSHAN_EVENT_GATEWAY_CONFIG_H__
#define __TIANSHAN_EVENT_GATEWAY_CONFIG_H__
#include <ConfigHelper.h>
#include <set>
namespace EventGateway{

struct ModuleConfig
{
    std::string image;
    static void structure(ZQ::common::Config::Holder<ModuleConfig> &holder);
};

struct PluginsConfig
{
    std::string populatePath;
    std::string configDir;
    std::string logDir;
    std::set<std::string> modules;
    void populate(const std::string &filespec);
    static void structure(ZQ::common::Config::Holder<PluginsConfig> &holder);
    void readModule(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerModule(const std::string &full_path);
};

struct EGConfig
{
    std::string iceStormEndpoint;

    // crash dump properties
    std::string dumpPath;
    int32 dumpEnabled;
    int32 fullMemoryDumpEnabled;

    // ice trace properties
    int32 iceTraceEnabled;
    int32 iceTraceLevel;
    int32 iceTraceFileSize;
    int32 iceTraceCount;

    // ice properties
    std::map<std::string, std::string> iceProperties;

    // connection maintenance properties
    int32 connCheckIntervalMSec;

    // plug-in config
    ZQ::common::Config::Holder<PluginsConfig> plugins;
    static void structure(ZQ::common::Config::Holder<EGConfig> &holder);
    void readPlugins(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerPlugins(const std::string &full_path);

    void readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerNothing(const std::string &);
};
} // namespace EventGateway 
extern ZQ::common::Config::Loader<EventGateway::EGConfig> gConfig;
#endif

