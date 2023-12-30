#ifndef __TianShan_EventChannel_Config_H__
#define __TianShan_EventChannel_Config_H__
#include <ConfigHelper.h>

struct TopicConf {
    typedef std::vector<std::string> Topics;
    Topics topics;
    struct TopicLink {
        std::string from;
        std::string to;
        int32 cost;
        static void structure(ZQ::common::Config::Holder<TopicLink>& holder) {
            holder.addDetail("", "from", &TopicLink::from);
            holder.addDetail("", "to", &TopicLink::to);
            holder.addDetail("", "cost", &TopicLink::cost, "0");
        }
    };
    typedef std::vector<TopicLink> TopicLinks;
    TopicLinks links;

    static void structure(ZQ::common::Config::Holder<TopicConf>& holder) {
        holder.addDetail("Topic", &TopicConf::readTopic, &TopicConf::registerNothing);
        holder.addDetail("Link", &TopicConf::readLink, &TopicConf::registerNothing);
    }
    struct TopicReader {
        std::string name;
        static void structure(ZQ::common::Config::Holder<TopicReader>& holder) {
            holder.addDetail("", "name", &TopicReader::name);
        }
    };
    void readTopic(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        using namespace ZQ::common;
        Config::Holder<TopicReader> reader;
        reader.read(node, hPP);
        topics.push_back(reader.name);
    }
    void readLink(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        using namespace ZQ::common;
        Config::Holder<TopicLink> reader;
        reader.read(node, hPP);
        links.push_back(reader);
    }
    void registerNothing(const std::string&){}
};

struct EventChannelConfig
{
    std::string crashDumpPath;
    int32 crashDumpEnabled;

    int32 iceTraceEnabled;
    int32 iceTraceLevel;
    int32 iceTraceSize;
    int32 iceTraceVerbose;
    int32 iceTraceCount;

    std::string bindHost;
    int32 bindPort;
    std::string dataDir;

    typedef std::map<std::string, std::string> Properties;

    Properties properties;
    Properties eventProp;
    int32 selfcheckInterval;
    int32 selfcheckThreshold;

    TopicConf preConf; 
    static void structure(ZQ::common::Config::Holder<EventChannelConfig>& holder)
    {
        using namespace ZQ::common;
        holder.addDetail("default/CrashDump", "path", &EventChannelConfig::crashDumpPath, NULL, Config::optReadOnly);
        holder.addDetail("default/CrashDump", "enabled", &EventChannelConfig::crashDumpEnabled, "1", Config::optReadOnly);

        holder.addDetail("default/IceTrace", "enabled", &EventChannelConfig::iceTraceEnabled, "1", Config::optReadOnly);
        holder.addDetail("default/IceTrace", "level", &EventChannelConfig::iceTraceLevel, "7", Config::optReadWrite);
        holder.addDetail("default/IceTrace", "size", &EventChannelConfig::iceTraceSize, "2048000", Config::optReadOnly);
        holder.addDetail("default/IceTrace", "verbose", &EventChannelConfig::iceTraceVerbose, "0", Config::optReadOnly);
        holder.addDetail("default/IceTrace", "count", &EventChannelConfig::iceTraceCount, "5", Config::optReadOnly);

        holder.addDetail("EventChannel/Bind", "host", &EventChannelConfig::bindHost, NULL, Config::optReadOnly);
        holder.addDetail("EventChannel/Bind", "port", &EventChannelConfig::bindPort, NULL, Config::optReadOnly);

        holder.addDetail("EventChannel/Database", "path", &EventChannelConfig::dataDir, NULL, Config::optReadOnly);
        holder.addDetail("EventChannel/SelfCheck", "interval", &EventChannelConfig::selfcheckInterval, "3000");
        holder.addDetail("EventChannel/SelfCheck", "threshold", &EventChannelConfig::selfcheckThreshold, "30000");

        holder.addDetail("default/IceProperties/prop", &EventChannelConfig::readProperty, &EventChannelConfig::registerNothing);

        holder.addDetail("EventChannel/PreConf", &EventChannelConfig::readPreConf, &EventChannelConfig::registerNothing, ZQ::common::Config::Range(0, 1));
        
        holder.addDetail("EventLockProperties/prop", &EventChannelConfig::readEventProperty, &EventChannelConfig::registerNothing);
	}

    void readProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<Config::NVPair> prop;
		prop.read(node, hPP);
		properties[prop.name] = prop.value;
	}
    void readEventProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<Config::NVPair> prop;
        prop.read(node, hPP);
        eventProp[prop.name] = prop.value;
    }
    void readPreConf(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<TopicConf> reader;
        reader.read(node, hPP);
        preConf = reader;
    }

    void registerNothing(const std::string&){}
};

extern ZQ::common::Config::Loader<EventChannelConfig> gConfig;
#endif

