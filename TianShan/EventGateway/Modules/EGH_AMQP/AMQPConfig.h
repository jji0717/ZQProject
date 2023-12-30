#ifndef __EventGw_JMS_Config_H__
#define __EventGw_JMS_Config_H__
#include <ConfigHelper.h>

namespace EventGateway{
    namespace AMQP{
    struct StringProperty
    {
        std::string name;
        std::string value;

        static void structure(ZQ::common::Config::Holder<StringProperty> &holder)
        {
            using namespace ZQ::common::Config;
            holder.addDetail("", "name", &StringProperty::name, NULL, optReadOnly);
            holder.addDetail("", "value", &StringProperty::value, NULL, optReadOnly);
        }
    };
    struct IntProperty
    {
        std::string name;
        int32 value;

        static void structure(ZQ::common::Config::Holder<IntProperty> &holder)
        {
            using namespace ZQ::common::Config;
            holder.addDetail("", "name", &IntProperty::name, NULL, optReadOnly);
            holder.addDetail("", "value", &IntProperty::value, NULL, optReadOnly);
        }
    };
    typedef std::map< std::string, std::string > StringProperties;
    typedef std::map< std::string, int > IntProperties;
    struct ChannelConfig
    {
        std::string name;
		int32       id;
        int32       msgType; // destination type

		int32       mandatory;
		int32		immediate;

		int32 TTL;
		int32 optionEnabled;

        StringProperties msgPropertiesString;
        IntProperties msgPropertiesInt;

        static void structure(ZQ::common::Config::Holder<ChannelConfig> &holder)
        {
            using namespace ZQ::common::Config;
            holder.addDetail("", "name", &ChannelConfig::name, NULL, optReadOnly);
		    holder.addDetail("", "id", &ChannelConfig::id, NULL, optReadOnly);
            holder.addDetail("", "dstType", &ChannelConfig::msgType, "0");
			holder.addDetail("", "mandatory", &ChannelConfig::mandatory, "0");
			holder.addDetail("", "immediate", &ChannelConfig::immediate, "0");
			holder.addDetail("Option", "TTL", &ChannelConfig::TTL, "0", optReadOnly);
			holder.addDetail("Option", "enabled", &ChannelConfig::optionEnabled, "1", optReadOnly);
            holder.addDetail("MessageProperties/StringProperty", &ChannelConfig::readStringProperty, &ChannelConfig::registerNothing);
            holder.addDetail("MessageProperties/IntProperty", &ChannelConfig::readIntProperty, &ChannelConfig::registerNothing);
        }
        void readStringProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<StringProperty> strProp;
            strProp.read(node, hPP);
            msgPropertiesString[strProp.name] = strProp.value;
        }

        void readIntProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<IntProperty> intProp;
            intProp.read(node, hPP);
            msgPropertiesInt[intProp.name] = intProp.value;
        }

        void registerNothing(const std::string&){}
    };

    struct ServerConfig
    {
        std::string name;
        std::string ip;
        int32       port ;
        std::string user;
		std::string passwd;
		std::string vHost;
		int32       frame_max;
        int32 enable;
		

        typedef std::map<std::string, ZQ::common::Config::Holder<ChannelConfig> > Channels;
        Channels channels;

        static void structure(ZQ::common::Config::Holder<ServerConfig> &holder)
        {
            using namespace ZQ::common::Config;
            holder.addDetail("", "name", &ServerConfig::name, NULL, optReadOnly);
            holder.addDetail("", "ip", &ServerConfig::ip, NULL, optReadOnly);
			holder.addDetail("", "port", &ServerConfig::port, NULL, optReadOnly);
            holder.addDetail("", "user", &ServerConfig::user, NULL, optReadOnly);
            holder.addDetail("", "passwd", &ServerConfig::passwd, NULL);
			holder.addDetail("", "vhost", &ServerConfig::vHost, "/");
			holder.addDetail("", "frameMax", &ServerConfig::frame_max, "131072");
            holder.addDetail("", "enabled", &ServerConfig::enable, "1");
            holder.addDetail("Channel", &ServerConfig::readChannel, &ServerConfig::registerChannels);
        }

        void readChannel(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<ChannelConfig> channel("name");
            channel.read(node, hPP);
            channels[channel.name] = channel;
        }
        void registerChannels(const std::string& full_path)
        {
            Channels::iterator it;
            for(it = channels.begin(); it != channels.end(); ++it)
            {
                it->second.snmpRegister(full_path);
            }
        }

    };

    struct TextMessage
    {
        std::string content;
        static void structure(ZQ::common::Config::Holder<TextMessage> &holder)
        {
            holder.addDetail("", "content", &TextMessage::content);
        }
    };
    struct MapProperty
    {
        std::string key;
        std::string value;
        static void structure(ZQ::common::Config::Holder<MapProperty> &holder)
        {
            holder.addDetail("", "key", &MapProperty::key);
            holder.addDetail("", "value", &MapProperty::value);
        }
    };
    struct MapMessage
    {
        std::map<std::string, std::string> content;
        static void structure(ZQ::common::Config::Holder<MapMessage> &holder)
        {
            holder.addDetail("property", &MapMessage::readProperty, &MapMessage::registerNothing);
        }
        void readProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<MapProperty> reader;
            reader.read(node, hPP);
            content[reader.key] = reader.value;
        }
        void registerNothing(const std::string&){}
    };

    struct MessageTemplate
    {
        std::string type; // text or map
        std::string vars; // optional predefined variables
        std::string txtContent;
        std::map<std::string, std::string> mapContent;
        static void structure(ZQ::common::Config::Holder<MessageTemplate>& holder)
        {
            holder.addDetail("", "type", &MessageTemplate::type);
            holder.addDetail("", "vars", &MessageTemplate::vars, "");
            holder.addDetail("text", &MessageTemplate::readTextContent, &MessageTemplate::registerNothing, ZQ::common::Config::Range(0, 1));
            holder.addDetail("map", &MessageTemplate::readMapContent, &MessageTemplate::registerNothing, ZQ::common::Config::Range(0, 1));
        }

        void readTextContent(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<TextMessage> reader;
            reader.read(node, hPP);
            txtContent = reader.content;
        }

        void readMapContent(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<MapMessage> reader;
            reader.read(node, hPP);
            mapContent = reader.content;
        }

        void registerNothing(const std::string&){}
    };

    struct EventFilterConfig
    {
        std::string eventName;
        std::string category;
        std::string sourceNetId;
        std::map<std::string, std::string> props;
        static void structure(ZQ::common::Config::Holder<EventFilterConfig> &holder)
        {
            holder.addDetail("", "eventName", &EventFilterConfig::eventName);
            holder.addDetail("", "category", &EventFilterConfig::category);
            holder.addDetail("", "sourceNetId", &EventFilterConfig::sourceNetId);
            holder.addDetail("parameter", &EventFilterConfig::readProperty, &EventFilterConfig::registerNothing);
        }

        void readProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<MapProperty> reader;
            reader.read(node, hPP);
            props[reader.key] = reader.value;
        }
        void registerNothing(const std::string&){}
    };

    struct HandlerConfig
    {
        int32 enabled;
        std::string name;
        std::string target;
        std::string source;
        EventFilterConfig filter;
        MessageTemplate msgTemplate;

        static void structure(ZQ::common::Config::Holder<HandlerConfig> &holder)
        {
            using namespace ZQ::common::Config;
            holder.addDetail("", "enabled", &HandlerConfig::enabled);
            holder.addDetail("", "name", &HandlerConfig::name);
            holder.addDetail("", "target", &HandlerConfig::target);
            holder.addDetail("", "source", &HandlerConfig::source, "TianShan/Event/Generic");

            holder.addDetail("filter", &HandlerConfig::readFilter, &HandlerConfig::registerNothing, ZQ::common::Config::Range(1, 1));

            holder.addDetail("message", &HandlerConfig::readMessageTemplate, &HandlerConfig::registerNothing, ZQ::common::Config::Range(1, 1));
        }
        void readFilter(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<EventFilterConfig> reader;
            reader.read(node, hPP);
            filter = reader;
        }
        void readMessageTemplate(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<MessageTemplate> holder;
            holder.read(node, hPP);
            msgTemplate = holder;
        }
        void registerNothing(const std::string&){}
    };

    struct AMQPConfig
    {
        int32 logLevel;
        int32 logFileSize;
        int32 logCount;
        std::string safeStorePath;

        typedef std::map<std::string, ZQ::common::Config::Holder<ServerConfig> > Servers;
        Servers servers;

        typedef std::vector< ZQ::common::Config::Holder<HandlerConfig> > HandlersConfig;
        HandlersConfig handlers;

        static void structure(ZQ::common::Config::Holder<AMQPConfig> &holder)
        {
            using namespace ZQ::common::Config;
            holder.addDetail("EGH_AMQP/Log", "level", &AMQPConfig::logLevel, NULL, optReadOnly);
            holder.addDetail("EGH_AMQP/Log", "size", &AMQPConfig::logFileSize, NULL, optReadOnly);
            holder.addDetail("EGH_AMQP/Log", "count", &AMQPConfig::logCount, "5", optReadOnly);
            holder.addDetail("EGH_AMQP/SafeStore", "path", &AMQPConfig::safeStorePath, NULL, optReadOnly);

            holder.addDetail("EGH_AMQP/RabbitMQServer", &AMQPConfig::readRabbitMQServer, &AMQPConfig::registerRabbitMQServers, Range(1, -1));
            holder.addDetail("EGH_AMQP/EventHandlers/Handler", &AMQPConfig::readHandler, &AMQPConfig::registerHandlers);
        }
		void readRabbitMQServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			ZQ::common::Config::Holder<ServerConfig> server("name");
			server.read(node, hPP);
			if(server.enable)
				servers[server.name] = server;
		}
        void registerRabbitMQServers(const std::string& full_path)
        {
            for(Servers::iterator it = servers.begin(); it != servers.end(); ++it)
                it->second.snmpRegister(full_path);
        }
        void readHandler(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
        {
            ZQ::common::Config::Holder<HandlerConfig> reader;
            reader.read(node, hPP);
            handlers.push_back(reader);
        }
        void registerHandlers(const std::string& full_path)
        { // not register to snmp
        }
    };
}
}
#endif
