#ifndef __TianShan_EventGw_JMS_MessageChannel_H__
#define __TianShan_EventGw_JMS_MessageChannel_H__
#include <ZQ_common_conf.h>
#include <JndiClient.h>
#include "JMSConfig.h"
#include "SafeStore.h"
#include "MessageRecord.h"
#include <NativeThread.h>
#include <Locks.h>
#include <Log.h>
#include "SystemUtils.h"

#define JMSLOGFMT(_C, _X) "%-18s [%s/%p] " _X, #_C, _name.c_str(), this

#define MsgReceivedTimeStamp  "MsgReceivedTimeStamp"

//#define MessageChannel_ExitTimeoutMSec 5000
namespace EventGateway{
namespace JMS{

class MessageTransporter;
class MessageChannel:virtual public ZQ::common::NativeThread, virtual public ZQ::JndiClient::JmsSession
{
    friend class MessageTransporter;
public:
    MessageChannel(::ZQ::JndiClient::ClientContext& context, ZQ::common::Log& log, const ChannelConfig& config, const Freeze::ConnectionPtr& dbConn);
    virtual ~MessageChannel();

    // push a new message to the channel
    void push(const Message& msg);
protected:
    // the message sending thread
    virtual int run();

    // connection status callback
	virtual void OnConnected(const std::string& notice);
    virtual void OnConnectionLost(const std::string& notice);
private:
    // send a message
    bool send(const Message &msg);
private:
    // copying forbidden
    MessageChannel& operator=(const MessageChannel&);
    MessageChannel(const MessageChannel&);
private:
    // notify the new message's arrival
    void notifyNewMessage();

    // build the message header
    void buildMessageProperties(ZQ::JndiClient::ClientContext::Properties& props);
private:
    ZQ::common::Log& _log; // logger instance

    ChannelConfig _config;
    std::string _name;
    ZQ::JndiClient::ClientContext::Properties msgProperties_;

    bool _quit;
    SYS::SingleObject _hNotifyNewMessage;

    // the internal message queue
    SafeQueue<MessageRecord>* _queue;
    ZQ::common::Mutex _lockMessage;

    bool connOk_; // connection status
};

} // namespace JMS
} // namespace EventGateway
#endif
