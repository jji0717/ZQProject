#include "JMSMessageTransporter.h"

namespace EventGateway{
namespace JMS{
using namespace ZQ::common;
using namespace ZQ::JndiClient;
typedef std::map<std::string, std::string> SimpleSetting;
// format of config text:
//      key1=value1; key2=value2;
// keys are case-sensitive
static void parseSetting(const std::string& txt, SimpleSetting &cfg)
{
    if(txt.empty())
        return;
    cfg.clear();

    using namespace ZQ::common;
    stringHelper::STRINGVECTOR vec;
    stringHelper::SplitString(txt, vec, ";");
    for(size_t i = 0; i < vec.size(); ++i)
    {
        stringHelper::STRINGVECTOR item;
        stringHelper::SplitString(vec[i], item, "=", "= ");
        if(item.size() != 2)
            continue; // discard bad configuration

        cfg[item[0]] = item[1];
    }
}
static Log::loglevel_t toLogLevel(int lvl, Log::loglevel_t defVal) {
    switch(lvl) {
    case 0: return Log::L_EMERG;
    case 1: return Log::L_ALERT;
    case 2: return Log::L_CRIT;
    case 3: return Log::L_ERROR;
    case 4: return Log::L_WARNING;
    case 5: return Log::L_NOTICE;
    case 6: return Log::L_INFO;
    case 7: return Log::L_DEBUG;
    default: return defVal;
    }
}
#define MessageTransporter_ExitTimeoutMSec         10000
#define MessageTransporter_ReconnectIntevalMSec    2000
MessageTransporter::MessageTransporter(ZQ::common::Log& log, const ServerConfig &serverConfig, const std::string& safestore, Ice::CommunicatorPtr ic)
    :_log(log), context_(NULL), _serverConfig(serverConfig), _ic(ic)
{
    _name = _serverConfig.name;

    // create the jms context
    ClientContext::Properties ctxProps;
    parseSetting(_serverConfig.env, ctxProps);
    ctxProps[JNDICTXPROP_INITIAL_CONTEXT_FACTORY] = _serverConfig.namingContextFactory;

    context_ = new ClientContext(_serverConfig.URL, toLogLevel(_serverConfig.traceLevel, Log::L_WARNING), ctxProps);

    try{
        _freezeConn = Freeze::createConnection(_ic, safestore);
    }catch(const Ice::Exception &e)
    {
        _log(Log::L_ERROR, JMSLOGFMT(MessageTransporter, "MessageTransporter() Caught [%s] during initializing Freeze."), e.ice_name().c_str());
        throw;
    }
}
MessageTransporter::~MessageTransporter()
{
    try {
        clear();
    } catch (...){}

    try
    {
        _freezeConn->close();
    }
    catch(const Ice::Exception &e)
    {
        _log(Log::L_ERROR, JMSLOGFMT(MessageTransporter, "~MessageTransporter() Caught [%s] during close Freeze connection."), e.ice_name().c_str());
    }
    catch(...)
    {
        _log(Log::L_ERROR, JMSLOGFMT(MessageTransporter, "~MessageTransporter() Caught kunkown exception during close Freeze connection."));
    }
    // release the context
    if(context_) {
        delete context_;
        context_ = NULL;
    }
}

// query the channel with name
MessageChannel* MessageTransporter::getChannel(const std::string &name)
{
    ZQ::common::MutexGuard sync(_lock);
    ChannelMap::iterator it = _channels.find(name);
    if(it != _channels.end())
    {
        return it->second;
    }
    else
    {
        return NULL;
    }
}

// create a channel base on the config
MessageChannel* MessageTransporter::createChannel(const ChannelConfig& config)
{
    // step 1: check the config value
    // step 2: create channel object
    ZQ::common::MutexGuard sync(_lock);
    ChannelMap::iterator it = _channels.find(config.name);
    if(it == _channels.end()) // prevent creating duplicate channel
    {
        MessageChannel *channel = NULL;
        try{
            channel = new MessageChannel(*context_, _log, config, _freezeConn);
            _channels[config.name] = channel;
            _log(Log::L_INFO, JMSLOGFMT(MessageTransporter, "createChannel() Created channel [%s] at [%p] successfully."), config.name.c_str(), channel);

            return channel;

        }catch(const Ice::Exception &e){
            _log(Log::L_ERROR, JMSLOGFMT(MessageTransporter, "createChannel() Caught Ice exception [%s] during creating channel [%s]."), e.ice_name().c_str(), config.name.c_str());
        } catch (const ZQ::common::Exception& e) {
            _log(Log::L_ERROR, JMSLOGFMT(MessageTransporter, "createChannel() Caught ZQ exception [%s] during creating channel [%s]."), e.getString(), config.name.c_str());
        } catch (...) {
            _log(Log::L_ERROR, JMSLOGFMT(MessageTransporter, "createChannel() Caught unknown exception during creating channel [%s]."), config.name.c_str());
        }
    }
    else
    {
        _log(Log::L_WARNING, JMSLOGFMT(MessageTransporter,
            "createChannel() Attempted to create channel [%s] that already exist."), config.name.c_str());
    }
    return NULL;
}
// clear all resource
void MessageTransporter::clear()
{
    {
        ZQ::common::MutexGuard sync(_lock);
        for(ChannelMap::iterator it = _channels.begin(); it != _channels.end(); ++it)
        {
            try {
                delete it->second;
            }catch(...){}
        }
        _channels.clear();
    }
}

// transport manager

TransportManager::TransportManager(ZQ::common::Log& log)
    :_log(log)
{
    int i = 0;
    _communicator = Ice::initialize(i, NULL);
}
TransportManager::~TransportManager()
{
    clear();
    try{ _communicator->destroy(); }catch(...){}
}
MessageTransporter* TransportManager::getTransporter(const std::string& name)
{
    ZQ::common::MutexGuard gd(_lock);
    TransporterMap::iterator it = _transporters.find(name);
    if(it != _transporters.end())
    {
        return it->second;
    }
    else
    {
        return NULL;
    }
}
MessageTransporter* TransportManager::createTransporter(const ServerConfig &serverConf, const std::string& safestore)
{
    ZQ::common::MutexGuard gd(_lock);
    TransporterMap::iterator it = _transporters.find(serverConf.name);
    if(it == _transporters.end())
    {
        try
        {
            MessageTransporter* transporter = new MessageTransporter(_log, serverConf, safestore, _communicator);
            _transporters[serverConf.name] = transporter;
            _log(Log::L_INFO, CLOGFMT(TransportManager, "createTransporter() Created transporter [%s] at [%p]."), serverConf.name.c_str(), transporter);

            return transporter;
        } catch(const Ice::Exception &e) {
            _log(Log::L_ERROR, CLOGFMT(TransportManager, "createTransporter() Caught Ice exception [%s] during creating transporter [%s]."), e.ice_name().c_str(), serverConf.name.c_str());
        } catch ( const ZQ::common::Exception& e) {
            _log(Log::L_ERROR, CLOGFMT(TransportManager, "createTransporter() Caught ZQ exception [%s] during creating transporter [%s]."), e.getString(), serverConf.name.c_str());
        }
        catch(...)
        {
            _log(Log::L_ERROR, CLOGFMT(TransportManager, "createTransporter() Unknown exception during creationg transporter [%s]."), serverConf.name.c_str());
        }
    }
    else
    {
        _log(Log::L_WARNING, CLOGFMT(TransportManager, "createTransporter() Try to create transporter [%s] that already exist."), serverConf.name.c_str());
    }
    return NULL;
}

void TransportManager::clear()
{
    ZQ::common::MutexGuard gd(_lock);
    for(TransporterMap::iterator it = _transporters.begin(); it != _transporters.end(); ++it)
    {
        try{ it->second->clear(); }catch(...){}
        try{ delete it->second; }catch(...){}
    }
    _transporters.clear();
}
} // namespace JMS
} // namespace EventGateway
