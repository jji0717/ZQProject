#if !defined __EVENTSENDERCFG_H__
#define __EVENTSENDERCFG_H__

#include <ConfigHelper.h>
#include <string>
#include <vector>
using namespace ZQ::common;

struct JmsMsgProp
{
	std::string		type;
	std::string		key;
	std::string		value;

	static void structure(Config::Holder< JmsMsgProp > &holder)
    {
        holder.addDetail("", "type", &JmsMsgProp::type, NULL);
        holder.addDetail("", "key", &JmsMsgProp::key, NULL);
        holder.addDetail("", "value", &JmsMsgProp::value, NULL);
    }
};

struct EventSender
{
	//log item
	std::string		logPath;
	int32			logSize;
	int32			logLevel;

	//jmssender item
	std::string		context;
	std::string		ipPort;
	std::string		destinationName;
	std::string		connectionFactory;
	typedef std::vector< Config::Holder< JmsMsgProp > > JmsMsgProps;
    JmsMsgProps		jmsmsgprops;
	int32			timeToLive;
	int32			jmsDequeSize;
	std::string		jmsSavePath;

	//icesender item
	std::string		endPoint;
	int32			timeout;
	int32			iceDequeSize;
	std::string		iceSavePath;

	//textwrite item
	std::string		recvFilePath;

	static void structure(Config::Holder< EventSender > &holder)
    {
		//log detail
        holder.addDetail("EventSender/Log", "logPath", &EventSender::logPath, NULL, Config::optReadOnly);
        holder.addDetail("EventSender/Log", "logFileSize", &EventSender::logSize, NULL, Config::optReadOnly);
		holder.addDetail("EventSender/Log", "logLevel", &EventSender::logLevel, NULL, Config::optReadOnly);
		
		//jms detail
		holder.addDetail("EventSender/JmsSender/Basic", "context", &EventSender::context, NULL,Config::optReadOnly);
		holder.addDetail("EventSender/JmsSender/Basic", "ipPort", &EventSender::ipPort, NULL, Config::optReadOnly);
        holder.addDetail("EventSender/JmsSender/Basic", "destinationName", &EventSender::destinationName, NULL, Config::optReadOnly);
		holder.addDetail("EventSender/JmsSender/Basic", "connectionFactory", &EventSender::connectionFactory, NULL, Config::optReadOnly);

        holder.addDetail("EventSender/JmsSender/JmsMessageProperty/MsgProperty", &EventSender::readJmsMsgProp, &EventSender::registerJmsMsgProp);
		
		holder.addDetail("EventSender/JmsSender/ProducerOpt", "timeToLive", &EventSender::timeToLive, NULL, Config::optReadOnly);
		holder.addDetail("EventSender/JmsSender/Other", "dequeSize", &EventSender::jmsDequeSize, NULL);
		holder.addDetail("EventSender/JmsSender/Other", "savePath", &EventSender::jmsSavePath, NULL);
		
		//ice detail
		holder.addDetail("EventSender/IceSender/Basic", "endPoint", &EventSender::endPoint, NULL, Config::optReadOnly);
		holder.addDetail("EventSender/IceSender/Basic", "timeout", &EventSender::timeout, NULL, Config::optReadOnly);
		holder.addDetail("EventSender/IceSender/Other", "dequeSize", &EventSender::iceDequeSize, NULL);
		holder.addDetail("EventSender/IceSender/Other", "savePath", &EventSender::iceSavePath, NULL);
		
        //textwriter detail
		holder.addDetail("EventSender/TextWriter/Basic", "receiveFile", &EventSender::recvFilePath, NULL,Config::optReadOnly);		

    }	

	void readJmsMsgProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
    {
        Config::Holder<JmsMsgProp> jmsmsgpropholder("");
        jmsmsgpropholder.read(node, hPP);
        jmsmsgprops.push_back(jmsmsgpropholder);
    }

    void registerJmsMsgProp(const std::string &full_path)
    {
        for (JmsMsgProps::iterator it = jmsmsgprops.begin(); it != jmsmsgprops.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
};


#endif//__EVENTSENDERCFG_H__
