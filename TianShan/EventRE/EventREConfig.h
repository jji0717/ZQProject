#include <ConfigHelper.h>
#include <list>
#include <map>

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

struct Topic{
	std::string name;
	static void structure(ZQ::common::Config::Holder< Topic > &holder)
	{
		holder.addDetail("", "name", &Topic::name, NULL);		
	}
};

struct TianShanEvents{
	std::string iceStormEndPoint;
	std::string listenEndPoit;
	std::string runtimeData;
	static void structure(ZQ::common::Config::Holder< TianShanEvents > &holder)
	{
		holder.addDetail("", "EventChannelEndPoint", &TianShanEvents::iceStormEndPoint, NULL);
		holder.addDetail("", "listenEndpoint", &TianShanEvents::listenEndPoit, NULL);
		holder.addDetail("", "runtimeData", &TianShanEvents::runtimeData, NULL);
	}
};

struct EventREConfig{
	int32 dumpFullMemory;
	std::string crushDumpPath;

	// size of ThreadPool
	int32 ThreadPoolSize;

	// max depth of rule
	int32 RuleDepth;
	std::string actionPath;
	std::string configPath;

	// size of Time Ahead
	int32 ItemStartTimeAhead;

	// defines the TianshanEvent properties
	std::string TopicMgrEndPoint;
	std::string ListenEventEndPoint;
	std::string TSEventRuntimeDataPath;

	std::string    _szEventPubType;


	typedef std::vector< ZQ::common::Config::Holder< IceProperty > > IceProps;
	IceProps iceProps;

	typedef std::vector< ZQ::common::Config::Holder< Topic > > Topics;
	Topics topics;

	static void structure(ZQ::common::Config::Holder< EventREConfig > &holder)
	{
		holder.addDetail("EventRuleEngine/CrashDump", "fullDump", &EventREConfig::dumpFullMemory, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("EventRuleEngine/CrashDump", "path", &EventREConfig::crushDumpPath, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("EventRuleEngine/ThreadPool", "size", &EventREConfig::ThreadPoolSize, "20", ZQ::common::Config::optReadOnly);

		holder.addDetail("EventRuleEngine/Rule", "depth", &EventREConfig::RuleDepth, "5", ZQ::common::Config::optReadOnly);
		holder.addDetail("EventRuleEngine/Rule", "actionPath", &EventREConfig::actionPath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EventRuleEngine/Rule", "configPath", &EventREConfig::configPath, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("EventRuleEngine/TianShanEvents", "EventChannelEndPoint", &EventREConfig::TopicMgrEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EventRuleEngine/TianShanEvents", "listenEndpoint", &EventREConfig::ListenEventEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EventRuleEngine/TianShanEvents", "runtimeData", &EventREConfig::TSEventRuntimeDataPath, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("EventRuleEngine/Events/topic", &EventREConfig::readTopic, &EventREConfig::registerTopic);

		holder.addDetail("EventRuleEngine/IceProperties/prop", &EventREConfig::readIceProperty, &EventREConfig::registerIceProperty);
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

	void readTopic(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<Topic> TopicHolder("name");
		TopicHolder.read(node, hPP);
		topics.push_back(TopicHolder);
	}
	void registerTopic(const std::string &full_path)
	{
		for (Topics::iterator it = topics.begin(); it != topics.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

};

extern ZQ::common::Config::Loader< EventREConfig > _config;

