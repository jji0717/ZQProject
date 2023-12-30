#include "MQTTMessageChannel.h"
#include "MQTTMessageTransporter.h"
#include <TimeUtil.h>

#define MessageChannel_ExitTimeoutMSec      5000
#define MessageChannel_ResendIntevalMSec    2000
#define MessageChannel_ReConnectIntevalMSec 5000

#define MQTTLOGFMT(_C, _X) "%-18s [%s:%s/%p] " _X, #_C, _name.c_str(), _chId.c_str(), this

namespace EventGateway{
namespace MQTT{
	using namespace ZQ::common;

	std::string  mqttStatusErrorStr(int code)
	{
		std::string error= "unknown";
		switch(code)
		{
		case MQTTCLIENT_SUCCESS:
			error = "MQTTCLIENT_SUCCESS";
			break;
		case MQTTCLIENT_FAILURE:
			error = "MQTTCLIENT_FAILURE";
			break;
		case MQTTCLIENT_PERSISTENCE_ERROR:
			error = "MQTTCLIENT_PERSISTENCE_ERROR";
			break;
		case MQTTCLIENT_DISCONNECTED:
			error = "MQTTCLIENT_DISCONNECTED";
			break;
		case MQTTCLIENT_MAX_MESSAGES_INFLIGHT:
			error = "MQTTCLIENT_MAX_MESSAGES_INFLIGHT";
			break;
		case MQTTCLIENT_BAD_UTF8_STRING:
			error = "MQTTCLIENT_BAD_UTF8_STRING";
			break;
		case MQTTCLIENT_NULL_PARAMETER:
			error = "MQTTCLIENT_NULL_PARAMETER";
			break;
		case MQTTCLIENT_TOPICNAME_TRUNCATED:
			error = "MQTTCLIENT_TOPICNAME_TRUNCATED";
			break;
		case MQTTCLIENT_BAD_STRUCTURE:
			error = "MQTTCLIENT_BAD_STRUCTURE";
			break;
		case MQTTCLIENT_BAD_QOS:
			error = "MQTTCLIENT_BAD_QOS";
			break;
		case MQTT_BAD_SUBSCRIBE:
			error = "MQTT_BAD_SUBSCRIBE";
			break;	
		default:
			break;
		}
		return error;
	}

	MessageChannel::MessageChannel(MessageTransporter& messageTransporter, ZQ::common::Log& log, const ChannelConfig& config, const Freeze::ConnectionPtr& dbConn)
		: _messageTransporter(messageTransporter),_log(log), _config(config), _queue(0),_qos(1)
	{
		_name = _config.name;
		_chId   = _config.id;
		_queueName = _config.name;
		_topicName =  _config.name;
		_qos = _config.QOS;
		_nConnOk = false;
		_connect = NULL;
		_queue = new SafeQueue<MessageRecord>(dbConn, _config.name); // may throw here
		_quit = false;
		buildConnectLostLists();
		buildMessageProperties(_msgProps);
		notifyNewMessage(); // start the serve cycle
		start();
	}
	MessageChannel::~MessageChannel()
	{
		// stop the sending thread
		_quit = true;
		notifyNewMessage(); // signal to quit thread
		_log(Log::L_INFO, MQTTLOGFMT(MessageChannel, "signal the working thread to quit..."));
		if(waitHandle(MessageChannel_ExitTimeoutMSec))
		{
			// the thread can't quit normally
			_log(Log::L_WARNING, MQTTLOGFMT(MessageChannel, "The thread can't quit normally in %d milliseconds, terminate it manually."), MessageChannel_ExitTimeoutMSec);
			terminate(0);
		}

		close();
		if(_queue)
		{
			try{
				delete _queue;
			} catch (...){}
			_queue = NULL;
		}
	}
	static void showMQTTMessage(const Message& msg, std::string& txt) {
		txt.clear();
		std::ostringstream buf;
		if(msg.size() == 1 && msg.begin()->first.empty()) {
			buf << "TextMessage{" << msg.begin()->second << "}";
		} else {
			buf << "MapMessage{";
			for(Message::const_iterator it = msg.begin(); it != msg.end(); ++it) {
				buf << it->first << ":" << it->second << ";";
			}
			buf << "}";
		}
		std::cout << buf;
		buf.str().swap(txt);
	}

	void MessageChannel::push(const Message& msg)
	{
		ZQ::common::MutexGuard sync(_lockMessage);
		_queue->push(msg);
		notifyNewMessage();
		_log(Log::L_DEBUG, MQTTLOGFMT(MessageChannel, "push() New message arrived, [%u] pending messages in the queue."), _queue->size());
	}

	int MessageChannel::run()
	{ // the message sending thread
		_log(Log::L_INFO, MQTTLOGFMT(MessageChannel, "working thread enter."));
		timeout_t waitTimeoutMSec = 0; // the timeout of waiting new message
		int64 ttl = _config.optionEnabled ? _config.TTL : 0;
		bool sentOK = false;
		int64 firstRetryStamp = 0;
		while(true)
		{
			_hNotifyNewMessage.wait(waitTimeoutMSec);

			// we can quit the thread safely now
			if(_quit)
			{
				break;
			}

			if(!_nConnOk)
			{
				if(!connect())
				{
					waitTimeoutMSec =  MessageChannel_ReConnectIntevalMSec; // retry after interval
					//continue;
				}
			}

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
					waitTimeoutMSec = TIMEOUT_INF; // reset the timeout for new message

					_log(Log::L_DEBUG, MQTTLOGFMT(MessageChannel, "run() Current message processing finished. [%u] pending messages in the queue."), _queue->size());
				}

				// we can quit the thread safely now
				if(_quit)
				{
					break;
				}

				if(!_queue->empty())
				{
					msg = _queue->front();
				}
				else
				{
					//waitTimeoutMSec = 60*1000*15;
					//checkConnection();
					// buildTestMessage();
					// printf("一次测试消息发送结束,将休息3秒。。。\n");
					// SYS::sleep(30000);
					continue; // wait for new message's arrival
				}
			}

			// now we got the message, send it
			sentOK = send(msg);
			if(sentOK)
			{
				firstRetryStamp = 0; // reset the retry timer
				notifyNewMessage(); // need next cycle to remove the message from the queue
				std::string txt;
				showMQTTMessage(msg, txt);
				//_log(Log::L_INFO, MQTTLOGFMT(MessageChannel, "Sent out message:%s"), txt.c_str());
			}
			else
			{ // can't send message, may the connection broken
				if(ttl > 0) { // discard the message if the retry time exceed ttl
					if(firstRetryStamp > 0) {
						int64 retryCost = ZQ::common::now() - firstRetryStamp;
						if(retryCost > ttl) {
							// discard the message
							sentOK = true; // just fake a successfully sending
							firstRetryStamp = 0; // reset the retry timer
							notifyNewMessage(); // need next cycle to remove the message from the queue
							std::string txt;
							showMQTTMessage(msg, txt);
							_log(Log::L_WARNING, MQTTLOGFMT(MessageChannel, "Discard the message due to can't send message in %lld msec. TTL:%lld, message:%s"), retryCost, ttl, txt.c_str());
							continue;
						}
					} else { // first retry, set the timer
						firstRetryStamp = ZQ::common::now();
					}
				} else { // not discard the message
				}
				// pause the processing when the channel is closed
				waitTimeoutMSec = _nConnOk ? MessageChannel_ResendIntevalMSec : MessageChannel_ReConnectIntevalMSec; // retry after interval
				_log(Log::L_WARNING, MQTTLOGFMT(MessageChannel, "Failed to send message, retry in [%d] milliseconds."), waitTimeoutMSec);
			}
		} // while(true)
		_log(Log::L_INFO, MQTTLOGFMT(MessageChannel, "working thread leave."));
		return 0;
	}
	void MessageChannel::buildMessageProperties(Properties& props)
	{
		StringProperties::iterator it = _config.msgPropertiesString.begin();
		while(it!=  _config.msgPropertiesString.end())
		{
			MAPSET(Properties,props,it->first,it->second);
			++it;
		}
		_log(Log::L_DEBUG, MQTTLOGFMT(MessageChannel, "in buildMessageProperties()"));
	}
	bool MessageChannel::send(const Message &msg)
	{
		if(msg.size() == 1 && msg.begin()->first.empty())
		{ 
			// treat as text message
			//showMQTTMessage((Message)msg,(std::string)msg.begin()->second);
			return publish(msg.begin()->second);
		}
		return false;
	}

	void MessageChannel::notifyNewMessage()
	{
		_hNotifyNewMessage.signal();
	}

	void MessageChannel::OnConnected(const std::string& notice) {
		_nConnOk = true;
		_log(Log::L_INFO, MQTTLOGFMT(MessageChannel, "Connected: %s"), notice.c_str());
	}
	void MessageChannel::OnConnectionLost(const std::string& notice) {
		_nConnOk = false;
		_log(Log::L_INFO, MQTTLOGFMT(MessageChannel, "ConnectionLost: %s"), notice.c_str());
	}


	bool MessageChannel::close()
	{
		if(!_nConnOk)
			return true;
		int nReply;

		nReply = MQTTClient_disconnect(_connect,MessageChannel_ReConnectIntevalMSec);
		if(nReply != MQTTCLIENT_SUCCESS)
		{
			_log(ZQ::common::Log::L_ERROR, MQTTLOGFMT(MessageChannel, "faile to closing connection with error:%s"), mqttStatusErrorStr(nReply).c_str());
		}

		MQTTClient_destroy(&_connect);
		_nConnOk = false;
		return true;
	}

	bool MessageChannel::connect()
	{
		if(_nConnOk)
			return true;
		int subsqos = 2;
		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
		MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;
		int nReply = MQTTCLIENT_FAILURE;
		std::string clientId = _topicName + _chId;
		ServerConfig& serverConfig =  _messageTransporter.getServerConfig();
		char ports[6] ;
		itoa(serverConfig.port,ports,10);
		std::string address = URI_TCP + serverConfig.ip +":" + ports;
		char* serUri = (char*)address.data();
		if(!_connect)//if not create a client(_connect == NULL),will create a new
		{
			nReply = MQTTClient_create(&_connect,address.c_str(),clientId.c_str(),MQTTCLIENT_PERSISTENCE_NONE,NULL);
			if(nReply != MQTTCLIENT_SUCCESS)
			{
				_log(ZQ::common::Log::L_ERROR, MQTTLOGFMT(MessageChannel, "faile to create connection with error:%d"),(mqttStatusErrorStr(nReply)).c_str());
				//MQTTClient_destroy(&_connect);
				return false;
			}
		}
		conn_opts.keepAliveInterval = _config.keepAliveInterval;
		conn_opts.cleansession = _config.cleansession;
		conn_opts.reliable = _config.reliable;		
		conn_opts.connectTimeout = _config.connectTimeout;
		conn_opts.retryInterval = _config.retryInterval;
		conn_opts.serverURIcount = _config.serverURIcount;
		conn_opts.MQTTVersion = _config.MQTTVersion;
		conn_opts.username = serverConfig.user.c_str();
		conn_opts.password = serverConfig.passwd.c_str();
		conn_opts.MQTTVersion = MQTTVERSION_DEFAULT; //default MQTTVersion
		conn_opts.serverURIs = &serUri;

		//set will message
		conn_opts.will = &wopts;
		conn_opts.will->message = "will message";
		conn_opts.will->qos = _qos;
		conn_opts.will->retained = 1;
		conn_opts.will->topicName = "will topic";
		conn_opts.will = NULL;
		if ((nReply = MQTTClient_connect(_connect, &conn_opts)) != MQTTCLIENT_SUCCESS)
		{
			_log(ZQ::common::Log::L_ERROR,  MQTTLOGFMT(MessageChannel, "failed to connecting....,with error:%s"),(mqttStatusErrorStr(nReply)).c_str());
			//MQTTClient_destroy(&_connect);
 			return false;
		}
		else
		{
			_log(ZQ::common::Log::L_INFO,  MQTTLOGFMT(MessageChannel, "Connect to service %s OOOOKKK...."),address.c_str());
		}
		// 	/* subscribe so we can get messages back */
		/* 	 MQTTClient_subscribe(_connect, _topicName.c_str(), subsqos);*/

		_nConnOk = true;
		return true;
	}


	bool MessageChannel::publish(const std::string& message)
	{
		MQTTClient_deliveryToken dt;
		MQTTClient_message pubmsg = MQTTClient_message_initializer;
		int i = 0;
		int iterations = 50;
		int reply = MQTTCLIENT_FAILURE;

		//_log(ZQ::common::Log::L_DEBUG,  MQTTLOGFMT(MessageChannel, "%d messages at QoS %d"),iterations,_qos);
		pubmsg.payload = (void*)message.c_str();
		pubmsg.payloadlen =(int)message.size() ;
		pubmsg.qos = _qos;
		pubmsg.retained = 1;
		//reply = MQTTClient_publish(_connect, _topicName.c_str(), pubmsg.payloadlen, pubmsg.payload, pubmsg.qos, pubmsg.retained, &dt);
		reply = MQTTClient_publishMessage(_connect, _topicName.c_str(), &pubmsg, &dt);
		reply = MQTTClient_waitForCompletion(_connect, dt, _config.connectTimeout*1000);//check this message whether send successfully  or not
		if(reply != MQTTCLIENT_SUCCESS)
		{
			_log(ZQ::common::Log::L_ERROR,  MQTTLOGFMT(MessageChannel, "failed to publish message[%s] to rabbitmq with errorcode[%d:%s] topicName[%s]"),
				message.c_str(), reply, (mqttStatusErrorStr(reply)).c_str(), _topicName.c_str());
			{
				std::vector<int>::iterator itor = std::find(_connectLostCode.begin(), _connectLostCode.end(), reply);
				if(itor != _connectLostCode.end())
				{
					_log(ZQ::common::Log::L_WARNING,  MQTTLOGFMT(MessageChannel, "ready to reconnect,waiting please ......"));
					_nConnOk = false;
				}
			}
			return false;
		}
		// 	else
		// 	{
		// 		reply = MQTTClient_publishMessage(_connect, _queueName.c_str(), &pubmsg, &dt);
		// 		_log(ZQ::common::Log::L_ERROR,  MQTTLOGFMT(MessageChannel, "publish message[%s] to rabbitmq with errorcode[%d:%s]. queueName[%s], topicName[%s]"),
		// 			message.c_str(), reply, (mqttStatusErrorStr(reply)).c_str(), _queueName.c_str(),_topicName.c_str());
		// 	}
		// if(reply == MQTTCLIENT_SUCCESS)
		// {
			// printf("\nMessage publish on topic %s : %s\n", _queueName.c_str(),pubmsg.payload);
		// }
		// 	if (_qos > 0)
		// 	{
		// 		reply = MQTTClient_waitForCompletion(_connect, dt, 5000L);
		// 	}
		_log(ZQ::common::Log::L_INFO,  MQTTLOGFMT(MessageChannel, "published mqtt[%s]: %s MQTTClient_deliveryToken:%d"), _topicName.c_str(), message.c_str(),dt);
		return true;
	}

	void MessageChannel::buildConnectLostLists()
	{
		_connectLostCode.push_back(MQTTCLIENT_FAILURE); //-1
		_connectLostCode.push_back(MQTTCLIENT_PERSISTENCE_ERROR); //-2
		_connectLostCode.push_back(MQTTCLIENT_DISCONNECTED);// -3
		_connectLostCode.push_back(MQTTCLIENT_MAX_MESSAGES_INFLIGHT);// -4
		_connectLostCode.push_back(MQTTCLIENT_BAD_UTF8_STRING);// -5
		_connectLostCode.push_back(MQTTCLIENT_NULL_PARAMETER);//-6
		_connectLostCode.push_back(MQTTCLIENT_TOPICNAME_TRUNCATED);//-7
		_connectLostCode.push_back(MQTTCLIENT_BAD_STRUCTURE);//-8
		_connectLostCode.push_back(MQTTCLIENT_BAD_QOS);//-9
	}

	void MessageChannel::checkConnection()
	{
		int reply = MQTTClient_isConnected(_connect);
		if(reply == 1)
		{
			_log(ZQ::common::Log::L_DEBUG,  MQTTLOGFMT(MessageChannel, "Client still connected"));
		}
		else
		{
			_log(ZQ::common::Log::L_ERROR,  MQTTLOGFMT(MessageChannel, "Client lost connect"));
			_nConnOk = false;
		}
	}

	void MessageChannel::buildTestMessage()
	{
		// 	static int count = 0;
		// 	for(int i = 0 ; i < 10;++i)
		// 	{
		// 		char message[1024];
		// 		memset(message, 0, sizeof(message));
		// 		sprintf(message, "MQTT:[[%s:%s/%p]count:%u: test message] ", _name.c_str(), _chId.c_str(), this, count);
		// 
		// 		Message msg;
		// 		msg[""] = message;
		// 		push(msg);
		// 		count++;
		// 	}
	}
} // namespace MQTT
} // namespace EventGateway
