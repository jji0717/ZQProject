#ifndef __TianShan_EventGw_MQTT_MessageTransporter_H__
#define __TianShan_EventGw_MQTT_MessageTransporter_H__
#include "MQTTMessageChannel.h"

namespace EventGateway{
namespace MQTT{
class TransportManager;
class MessageTransporter
{
    friend class TransportManager;
public:
    MessageTransporter(ZQ::common::Log& log, const ServerConfig &serverConfig, const std::string& safestore, Ice::CommunicatorPtr ic);
    virtual ~MessageTransporter();

    // query the channel with name
    MessageChannel* getChannel(const std::string &name);

    // create a channel base on the config
    MessageChannel* createChannel(const ChannelConfig& config);

	ServerConfig& getServerConfig() {return _serverConfig;};

private:
    // clear all resource
    void clear();
private:
    MessageTransporter(const MessageTransporter&);
    MessageTransporter& operator=(const MessageTransporter&);

private:
    ZQ::common::Log& _log;

    ServerConfig _serverConfig;
    std::string _name;

    Ice::CommunicatorPtr	_ic;
    Freeze::ConnectionPtr	_freezeConn;

    typedef std::map<std::string, MessageChannel*> ChannelMap;
    ChannelMap _channels;
    ZQ::common::Mutex _lock;

};


// transport manager
class TransportManager
{
public:
    TransportManager(ZQ::common::Log& log);
    virtual ~TransportManager();
    MessageTransporter* getTransporter(const std::string& name);
    MessageTransporter* createTransporter(const ServerConfig &serverConf, const std::string& safestore);
private:
    void clear();
private:
    ZQ::common::Log& _log;

    typedef std::map< std::string, MessageTransporter* > TransporterMap;
    TransporterMap _transporters;
    ZQ::common::Mutex _lock;

    Ice::CommunicatorPtr _communicator;
};
} // namespace MQTT
} // namespace EventGateway
#endif
