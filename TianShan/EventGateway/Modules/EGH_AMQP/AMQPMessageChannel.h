#ifndef __TianShan_EventGw_AMQP_MessageChannel_H__
#define __TianShan_EventGw_AMQP_MessageChannel_H__
#include <ZQ_common_conf.h>
#include "AMQPConfig.h"
#include "SafeStore.h"
#include "MessageRecord.h"
#include <NativeThread.h>
#include <Locks.h>
#include <Log.h>
#include "SystemUtils.h"
#include "amqp.h"
//#define MessageChannel_ExitTimeoutMSec 5000
namespace EventGateway{
namespace AMQP{

class MessageTransporter;
class MessageChannel:virtual public ZQ::common::NativeThread
{
    friend class MessageTransporter;
public:

	enum ExchangeType
	{
		Direct = 0,
		Fanout = 1,
		Topic = 2
	};

    MessageChannel(MessageTransporter& messageTransporter, ZQ::common::Log& log, const ChannelConfig& config, const Freeze::ConnectionPtr& dbConn);
    virtual ~MessageChannel();

    // push a new message to the channel
    void push(const Message& msg);
protected:
    // the message sending thread
    virtual int run();

    // connection status callback
	virtual void OnConnected(const std::string& notice);
    virtual void OnConnectionLost(const std::string& notice);

	void checkConnection();
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

	void buildConnectLostLists();

private:
    ZQ::common::Log& _log; // logger instance

    ChannelConfig _config;
    std::string _name;
	int			_chId;

    bool _quit;
    SYS::SingleObject _hNotifyNewMessage;

    // the internal message queue
    SafeQueue<MessageRecord>* _queue;
    ZQ::common::Mutex _lockMessage;

    bool _nConnOk; // connection status
	std::string		_exchange;

	//direct, fanout,topic
	ExchangeType    _exchangeType;
	std::string     _routingKey;
	std::string		_queueName;

	MessageTransporter& _messageTransporter;

	amqp_connection_state_t _conn;
	amqp_socket_t *_socket;

	amqp_basic_properties_t _msgProps;


	std::vector<int> _connectLostCode;

public:
	int getChId(){ return _chId;};
	std::string& getExchange(){ return _exchange;};
	int  getExchangeType(){ return _exchangeType;};
	std::string& getQueue(){ return _queueName;};

	bool connect();
	bool close();
	bool publish(const std::string& message, const amqp_basic_properties_t& props,amqp_boolean_t mandatory= 0, amqp_boolean_t immediate = 0);

private:
	std::string converExchangeType(const ExchangeType&  exchangeType);

	void MessageChannel::buildTestMessage();
	void buildMessageProperties(amqp_basic_properties_t& props);
    
};

} // namespace AMQP
} // namespace EventGateway
#endif
