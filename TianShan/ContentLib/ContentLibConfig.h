#include <ConfigHelper.h>
#include <list>
#include <map>
using namespace ZQ::common;

typedef std::map<std::string,  Config::Holder<Config::NVPair> > PARAMMAP;

struct IceProperty{
	std::string name;
	std::string value;
	static void structure(ZQ::common::Config::Holder< IceProperty > &holder)
    {
        holder.addDetail("", "name", &IceProperty::name, NULL);
        holder.addDetail("", "value", &IceProperty::value, "", ZQ::common::Config::optReadOnly);		
    }
};

struct IceProperties{
	std::string name;
	typedef std::vector< ZQ::common::Config::Holder< IceProperty > > IceProps;
	IceProps iceProps;
	static void structure(ZQ::common::Config::Holder< IceProperties > &holder)
	{
		holder.addDetail("", "name", &IceProperties::name, NULL);
		holder.addDetail("prop", &IceProperties::readIceProperty, &IceProperties::registerIceProperty);
	}
    void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<IceProperty> IcePropHolder("name");
        IcePropHolder.read(node, hPP);
        iceProps.push_back(IcePropHolder);
    }
    void registerIceProperty(const std::string &full_path)
    {
        for (IceProps::iterator it = iceProps.begin(); it != iceProps.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
};

struct TianShanEvents{
	std::string iceStormEndPoint;
	std::string listenEndPoit;
	static void structure(ZQ::common::Config::Holder< TianShanEvents > &holder)
	{
		holder.addDetail("", "EventChannelEndPoint", &TianShanEvents::iceStormEndPoint, NULL);
		holder.addDetail("", "listenEndpoint", &TianShanEvents::listenEndPoit, NULL);
	}
};

struct CSParam{
	std::string endpoint;
	static void structure(ZQ::common::Config::Holder< CSParam > &holder)
	{
		holder.addDetail("", "endpoint", &CSParam::endpoint, NULL);
	}
};

struct MetadataCategory{
	std::string category;
	int32 indexFlag;
	static void structure(ZQ::common::Config::Holder< MetadataCategory > &holder)
	{
		holder.addDetail("", "category", &MetadataCategory::category, NULL);
		holder.addDetail("", "indexFlag", &MetadataCategory::indexFlag, "1");
	}
};

struct ContentLibConfig{
	int32 dumpFullMemory;
	std::string crushDumpPath;

	// the directory of database
	std::string safeStorePath;
//	int32 maxContent;

	std::string ContentLibEndPoint;
	int32 timeToSync;

	//ice trace
	int32 iceTraceEnable;
	int32 iceLogLevel;
	int32 iceLogSize;
	int32 iceLogCount;

	// size of ThreadPool
	int32 ThreadPoolSize;


	// defines the TianshanEvent properties
	std::string TopicMgrEndPoint;
	std::string ListenEventEndPoint;

	typedef std::vector< ZQ::common::Config::Holder< IceProperty > > IceProps;
	IceProps iceProps;

	typedef std::vector< ZQ::common::Config::Holder< CSParam > > CSParams;
	CSParams csParams;

	typedef std::vector< ZQ::common::Config::Holder< MetadataCategory > > MetadataCategories;
	MetadataCategories categories;

	static void structure(ZQ::common::Config::Holder< ContentLibConfig > &holder)
	{
		holder.addDetail("ContentLib/CrashDump", "fullDump", &ContentLibConfig::dumpFullMemory, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/CrashDump", "path", &ContentLibConfig::crushDumpPath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/Database", "path", &ContentLibConfig::safeStorePath, NULL, ZQ::common::Config::optReadOnly);
//		holder.addDetail("ContentLib/Database", "maxContent", &ContentLibConfig::maxContent, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/ContentLibApp", "endpoint", &ContentLibConfig::ContentLibEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/ContentLibApp", "timeToSync", &ContentLibConfig::timeToSync, "3600000", ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/ThreadPool", "size", &ContentLibConfig::ThreadPoolSize, "20", ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/TianShanEvents", "EventChannelEndPoint", &ContentLibConfig::TopicMgrEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/TianShanEvents", "listenEndpoint", &ContentLibConfig::ListenEventEndPoint, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("ContentLib/IceTrace", "enable", &ContentLibConfig::iceTraceEnable, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/IceTrace", "level", &ContentLibConfig::iceLogLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/IceTrace", "size", &ContentLibConfig::iceLogSize, "10240000", ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentLib/IceTrace", "count", &ContentLibConfig::iceLogCount, "5", ZQ::common::Config::optReadOnly);

		holder.addDetail("ContentLib/IceProperties/prop", &ContentLibConfig::readIceProperty, &ContentLibConfig::registerIceProperty);
		holder.addDetail("ContentLib/ContentStore/param", &ContentLibConfig::readCSParam, &ContentLibConfig::registerCSParam);
		holder.addDetail("ContentLib/MetaData/param", &ContentLibConfig::readMetadataCategory, &ContentLibConfig::registerMetadataCategory);
	}

	void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<IceProperty> IcePropHolder("name");
        IcePropHolder.read(node, hPP);
        iceProps.push_back(IcePropHolder);
    }
    void registerIceProperty(const std::string &full_path)
    {
        for (IceProps::iterator it = iceProps.begin(); it != iceProps.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }

	void readCSParam(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<CSParam> CSParamHolder("endpoint");
		CSParamHolder.read(node, hPP);
		csParams.push_back(CSParamHolder);
	}
	void registerCSParam(const std::string &full_path)
	{
		for (CSParams::iterator it = csParams.begin(); it != csParams.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	void readMetadataCategory(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<MetadataCategory> MetadataCategoryHolder("category");
		MetadataCategoryHolder.read(node, hPP);
		categories.push_back(MetadataCategoryHolder);
	}
	void registerMetadataCategory(const std::string &full_path)
	{
		for (MetadataCategories::iterator it = categories.begin(); it != categories.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}
};

//extern ChodConfig					_config;
extern ZQ::common::Config::Loader< ContentLibConfig > _config;

