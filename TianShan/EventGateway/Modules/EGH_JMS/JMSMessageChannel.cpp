#include "JMSMessageChannel.h"
#include <TimeUtil.h>

#define MessageChannel_ExitTimeoutMSec      5000
#define MessageChannel_ResendIntevalMSec    2000
namespace EventGateway{
namespace JMS{
using namespace ZQ::common;
using namespace ZQ::JndiClient;
MessageChannel::MessageChannel(ClientContext& context, ZQ::common::Log& log, const ChannelConfig& config, const Freeze::ConnectionPtr& dbConn)
:JmsSession(context, (config.dstType.empty() ? (config.destination.find("queue") == 0 ? JmsSession::DT_Queue : JmsSession::DT_Topic) : (config.dstType.find("queue") == 0 ? JmsSession::DT_Queue : JmsSession::DT_Topic)), config.destination, true, false)
    , _log(log), _config(config), _queue(0), connOk_(true)
{
    _name = _config.name;
    // create the safe queue
    _queue = new SafeQueue<MessageRecord>(dbConn, _config.name); // may throw here

    // create message properties
    buildMessageProperties(msgProperties_);
    // set producer options
    if(_config.optionEnabled)
    {
        setProducerOptions(5, _config.TTL);
    }

    _quit = false;
    notifyNewMessage(); // start the serve cycle
    start();
}
MessageChannel::~MessageChannel()
{
    // stop the sending thread
    _quit = true;
    notifyNewMessage(); // signal to quit thread
    _log(Log::L_INFO, JMSLOGFMT(MessageChannel, "signal the working thread to quit..."));
    if(waitHandle(MessageChannel_ExitTimeoutMSec))
    {
        // the thread can't quit normally
        _log(Log::L_WARNING, JMSLOGFMT(MessageChannel, "The thread can't quit normally in %d milliseconds, terminate it manually."), MessageChannel_ExitTimeoutMSec);
        terminate(0);
    }

    if(_queue)
    {
        try{
            delete _queue;
        } catch (...){}
        _queue = NULL;
    }
}
static void showJMSMessage(const Message& msg, std::string& txt) {
    txt.clear();
	Message msgTemp = msg;
	if(msgTemp.find(MsgReceivedTimeStamp) != msgTemp.end())
		msgTemp.erase(MsgReceivedTimeStamp);

    std::ostringstream buf;
    if(msgTemp.size() == 1 && msgTemp.begin()->first.empty()) {
        buf << "TextMessage{" << msgTemp.begin()->second << "}";
    } else {
        buf << "MapMessage{";
        for(Message::const_iterator it = msgTemp.begin(); it != msgTemp.end(); ++it) {
            buf << it->first << ":" << it->second << ";";
        }
        buf << "}";
    }
    buf.str().swap(txt);
}

void MessageChannel::push(const Message& msg)
{
    ZQ::common::MutexGuard sync(_lockMessage);
    _queue->push(msg);
    notifyNewMessage();
    _log(Log::L_DEBUG, JMSLOGFMT(MessageChannel, "push() New message arrived, [%u] pending messages in the queue."), _queue->size());
}

int MessageChannel::run()
{ // the message sending thread
    _log(Log::L_INFO, JMSLOGFMT(MessageChannel, "working thread enter."));
    timeout_t waitTimeoutMSec = TIMEOUT_INF; // the timeout of waiting new message
    bool sentOK = false;
    int64 ttl = _config.optionEnabled ? _config.TTL : 0;
    int64 firstRetryStamp = 0;
    while(true)
    {
        _hNotifyNewMessage.wait(waitTimeoutMSec);

        Message msg;
        // take new message
        {
            ZQ::common::MutexGuard sync(_lockMessage);

            if(sentOK)
            {
                if(!_queue->empty())
                {
                    _queue->pop();
                }
                else
                {
                    // can not happen;
                }
                // update the status
                sentOK = false;
                waitTimeoutMSec = TIMEOUT_INF; // reset the timeout for new nessage

                _log(Log::L_DEBUG, JMSLOGFMT(MessageChannel, "run() Current message processing finished. [%u] pending messages in the queue."), _queue->size());
            }

            // we can quit the thread safely now
            if(_quit)
                break;

            if(_queue->empty())
                continue; // wait for new message's arrival

			msg = _queue->front();
        }

        // now we got the message, send it
		sentOK = send(msg);
		std::string txt;
		showJMSMessage(msg, txt);

		if (sentOK)
        {
            firstRetryStamp = 0; // reset the retry timer
            notifyNewMessage(); // need next cycle to remove the message from the queue
            _log(Log::L_INFO, JMSLOGFMT(MessageChannel, "message sent:%s"), txt.c_str());
			continue;
        }

		// can't send message, may the connection broken
		// determine the interval for next retry
		waitTimeoutMSec = connOk_ ? MessageChannel_ResendIntevalMSec : TIMEOUT_INF; // retry after interval

		if (ttl <= 0)
			continue; // TTL not specified, means retry for ever

		// discard the message if the retry time exceed ttl
		if (msg.find(MsgReceivedTimeStamp) != msg.end())
		{
			int64 msgReceivedTime = ZQ::common::TimeUtil::ISO8601ToTime(msg[MsgReceivedTimeStamp].c_str());
			//ZQ::common::TimeUtil::Iso2Time64(msg[MsgReceivedTimeStamp].c_str(), msgReceivedTime);
			if ((msgReceivedTime + ttl) < ZQ::common::now())
			{
				sentOK = true; // just fake a successfully sending
				notifyNewMessage(); // need next cycle to remove the message from the queue
				_log(Log::L_WARNING, JMSLOGFMT(MessageChannel, "Discard the message due to can't send message. TTL:%lld, message:%s. ,messageReceivedTime[%s]"), ttl, txt.c_str(), msg[MsgReceivedTimeStamp].c_str());
				continue;
			}
		}

		_log(Log::L_WARNING, JMSLOGFMT(MessageChannel, "send failed, will retry in [%d]msec: %s"), waitTimeoutMSec, txt.c_str());
	} // while(true)

    _log(Log::L_INFO, JMSLOGFMT(MessageChannel, "working thread leave."));
    return 0;
}

bool MessageChannel::send(const Message &msg)
{
#pragma message(__MSGLOC__"How to deal with empty message?")

	Message msgTemp = msg;
	if(msgTemp.find(MsgReceivedTimeStamp) != msgTemp.end())
		msgTemp.erase(MsgReceivedTimeStamp);

    if(msgTemp.size() == 1 && msgTemp.begin()->first.empty())
    { // treat as text message
        return sendTextMessage(msgTemp.begin()->second, msgProperties_);
    }
    else
    { // treat as map message
        JmsSession::MapMessage jmsMsg;
        for(Message::const_iterator it = msgTemp.begin(); it != msgTemp.end(); ++it) {
            JmsSession::setProperty(jmsMsg, it->first, it->second);
        }
        return sendMapMessage(jmsMsg, msgProperties_);
    }
}

void MessageChannel::notifyNewMessage()
{
    _hNotifyNewMessage.signal();
}

void MessageChannel::buildMessageProperties(ZQ::JndiClient::ClientContext::Properties& props)
{
    props.clear();
    { // set the string properties
        StringProperties::iterator it = _config.msgPropertiesString.begin();
        for(; it != _config.msgPropertiesString.end(); ++it)
        {
            JmsSession::setProperty(props, it->first, it->second);
        }
    }
    { // set the int properties
        IntProperties::iterator it = _config.msgPropertiesInt.begin();
        for(; it != _config.msgPropertiesInt.end(); ++it)
        {
            JmsSession::setIntegerProperty(props, it->first.c_str(), it->second);
        }
    }
}

void MessageChannel::OnConnected(const std::string& notice) {
    connOk_ = true;
    _log(Log::L_INFO, JMSLOGFMT(MessageChannel, "Connected: %s"), notice.c_str());
}
void MessageChannel::OnConnectionLost(const std::string& notice) {
    connOk_ = false;
    _log(Log::L_INFO, JMSLOGFMT(MessageChannel, "ConnectionLost: %s"), notice.c_str());
}
} // namespace JMS
} // namespace EventGateway
