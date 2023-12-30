#ifndef __TianShan_EventGw_MQTT_MessageChannel_H__
#define __TianShan_EventGw_MQTT_MessageChannel_H__
#include <ZQ_common_conf.h>
#include "MQTTConfig.h"
#include "SafeStore.h"
#include "MessageRecord.h"
#include <NativeThread.h>
#include <Locks.h>
#include <Log.h>
#include "SystemUtils.h"
#include "Message.h"
#include "MessageRecord.h"

extern "C" {
	#include "MQTTClient.h"
}
#define URI_TCP "tcp://"

//#define MessageChannel_ExitTimeoutMSec 5000
namespace EventGateway{
namespace MQTT{
	
#define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL

class MessageTransporter;
class MessageChannel: virtual public ZQ::common::NativeThread
{
    friend class MessageTransporter;

	typedef std::map<std::string, std::string> Properties;
public:	
	
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
    std::string  _name;
	std::string	 _chId;
	int          _qos;
    bool         _quit;
    SYS::SingleObject _hNotifyNewMessage;

    // the internal message queue
    SafeQueue<MessageRecord>* _queue;
    ZQ::common::Mutex _lockMessage;

    bool _nConnOk; // connection status
	std::string		_topicName;
	std::string     _queueName;

	MessageTransporter& _messageTransporter;
	
	MQTTClient _connect;
	Properties _msgProps;
	std::vector<int> _connectLostCode;

public:
	std::string getChId(){ return _chId;};
	std::string& getTopicNmae(){ return _topicName;};
	std::string& getQueue(){ return _queueName;};

	bool connect();
	bool close();
	bool publish(const std::string& message);
private:

	void buildTestMessage();
	void buildMessageProperties(Properties& props);
    
};

} // namespace MQTT
} // namespace EventGateway
#endif
