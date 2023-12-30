#ifndef __ZQ_EventClient_H__
#define __ZQ_EventClient_H__

#include "TianShanDefines.h"
#include "TsEvents.h"
#include "IceStorm\IceStorm.h"

#include "Log.h"
#include "Exception.h"
#include "SystemUtils.h"

#include <string>
#include <map>

#define TEST_LIB

namespace ZQ {
namespace EventClient {

#define  EventClient_EXPORTS

#ifdef EventClient_EXPORTS
#  define EventClient_API __EXPORT
#else
#  define EventClient_API __DLLRTL
#endif

class EventClient_API EventClient;
class EventClient_API EventSession;
class EventClient_API GenericEventSession;

class GenEventSinkImpl;
class EventConnector; 
class EventDispatcher; 

#ifdef TEST_LIB
class EventClientMementoOri;
#endif

// -----------------------------
// class EventClientException
// -----------------------------
/// A sub-hierarchy of Exception Jndi related.
class EventClientException : public ZQ::common::Exception
{
public:
	EventClientException(const std::string &what_arg) throw();
	virtual ~EventClientException() throw();
};

// -----------------------------
// class EventClient
// -----------------------------
/// the client context to a destination EventChannel service provider
class EventClient
{
public:
	friend class EventSession;
	friend class GenericEventSession; // TODO: should not be in the list
	friend class EventDispatcher;
	friend class EventConnector;

#ifdef TEST_LIB
	friend EventClientMementoOri;
#endif


public://protected programming way, for method get type information.
	typedef struct _sessionProperty 
	{ 
		::Ice::ObjectPrx objPrx;
		::TianShanIce::Properties qos;
	} sessionProperty;

	typedef std::map<std::string, sessionProperty> sessionPropertys; //sessionId,  sessionProprerty.
	typedef std::map<std::string, sessionPropertys>	sessionPropertysMap;//topicname, sessionProprertylist

public:

	/// constructor
	///@throw EventClientException if failed to initialize the context
	///@note initEventChannel() must be called piror to the initialization of EventClient
	EventClient(ZQ::common::Log& log, Ice::ObjectAdapterPtr& adapter, 
		const ::TianShanIce::Properties& properties=TianShanIce::Properties()) throw(EventClientException);

	virtual ~EventClient(void);

	/// connect the eventchannel service
	///@param topicManagerEndpoint eventchannel service endpoint
	bool connect(const ::std::string& topicManagerEndpoint);

	/// disconnect the eventchannel service
	void disconnect();

#ifdef TEST_LIB
	EventClientMementoOri * getMemento(void) { return _clientMemoto; }
#endif

protected:
	bool            subscribe(const std::string sessionId, const ::Ice::ObjectPrx& subscriber, const std::string& topicName, ::TianShanIce::Properties& properties = TianShanIce::Properties());
	Ice::ObjectPrx  addPublisher(const std::string sessionId, const std::string& topicName, ::TianShanIce::Properties& properties = TianShanIce::Properties());

	Ice::ObjectPrx  getPublish(const std::string topicName, const std::string sessionId);


protected:
	ZQ::common::Log& _log;

private:
	Ice::ObjectAdapterPtr        _adapter;
	IceStorm::TopicManagerPrx	 _topicManager;

	std::vector<EventSession* >  _eventSessionList;

	ZQ::common::Mutex    _lockPublishers;
	sessionPropertysMap  _publishers;      // topicname, publish map

	ZQ::common::Mutex    _lockSubscribers;
	sessionPropertysMap  _subscribers;     //topicname, subscribe map

private:
	SYS::SingleObject         _hLock;
	SYS::SingleObject         _hQuit;
	std::string          _topicManagerEndpoint;
	bool                _bConnected;
	EventConnector *     _threadConnector;
	EventDispatcher *    _threadDispatcher;

#ifdef TEST_LIB
	EventClientMementoOri * _clientMemoto;
#endif
};

class EventSession
{
public:
	EventSession(EventClient& client, const std::string &topicName, const ::IceStorm::QoS &Qos);
	virtual ~EventSession() {}

public:
	/// To sink the event of connection-established
	///@param message the text message about the establishing
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnConnected(const std::string& notice) = 0;

	/// To sink the event of connection-lost
	///@param message the text message about the connection lost
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnConnectionLost(const std::string& notice) = 0;

	virtual bool setConnected(bool bConnected) = 0;

	virtual bool getConnected(void) = 0;

public:
	std::string &       getSessionTopicName(void) {return _SessionTopicName;}
	std::string &       getSessionId(void)  { return _sessionId;}
	::IceStorm::QoS &   getSessionQos(void) { return _Qos;}
	int isFirstLogin(void)
	{
		int nTemp = _isFirstLogin;
		_isFirstLogin = false;
		return nTemp;
	}

public:
	ZQ::common::Log& _log;
	EventClient&    _client;

protected:
	bool   setSessionId(const std::string & sessionId) { _sessionId = sessionId; return true;}

private:
	std::string     _sessionId;
	std::string     _SessionTopicName;
	::IceStorm::QoS _Qos;
	int             _isFirstLogin;
};

// -----------------------------
// class GenericEventSession
// -----------------------------
/// the basic session bound to a destination topic
class GenericEventSession : public EventSession  // EventSession
{
	friend class GenEventSinkImpl;

public:
	typedef struct _Message
	{
		int                                   id;     //Event ID
		std::string                     category;	  //Event Category
		std::string                     timestamp;	  //timestamp
		std::string                     eventName;	  //Event name such as 'SessionInService'
		std::string                     sourceNetId;  //source net id,host name is recommended
		::TianShanIce::Properties        property;     //Event properties
	} Message;

public:
	/// constructor
	///@param[in] client context the the EventClient		
	///@param[in] topicName  the topic name of destination
	///@param[in] properties ice storm based qos properties
	///@param[in] asOridycer true if the session wishes to send the messages to EventChannel
	///@param[in] asConsumer true if the session wishes to listen the messages from EventChannel
	GenericEventSession(EventClient& client, const std::string topicName, TianShanIce::Properties& properties, bool asProducer =true, bool asConsumer=false);

	/// destructor
	virtual ~GenericEventSession();

public:

	/// send a message to the destination topic, valid only when asProducer=true in the constructor
	///@param message the message to send
	///@return true if the sending gets succeeded
	///@note for those messages that failed to send, it is up to the caller to consider whether the retries are necessary
	bool sendMessage(Message message);

	/// To sink  messages from the destination topic, only valid if asConsumer=true when initialization
	///@param message the message received
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnMessage(Message& message);

	/// To sink the event of connection-established
	///@param message the text message about the establishing
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnConnected(const std::string& notice);

	/// To sink the event of connection-lost
	///@param message the text message about the connection lost
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnConnectionLost(const std::string& notice);

public:
	std::string       getSessionId(){return EventSession::getSessionId();};
	std::string       getTopicName(){return EventSession::getSessionTopicName();};
	bool              getConnected(void){return _bDisconnect;}

public:
	bool             setConnected(bool bConnected = true){return (_bDisconnect = bConnected);}
	void             storeEventSink(TianShanIce::Events::GenericEventSinkPtr genEventSinkPtr){	_GenEventSinkPtr = genEventSinkPtr;	}

protected:
	bool _bDisconnect;
	bool _asProducer;
	bool _asConsumer;

	::std::string            _topicName;
	ZQ::common::Mutex         _lockSendMesage;	
	::TianShanIce::Properties  _Properties;

	TianShanIce::Events::GenericEventSinkPtr _GenEventSinkPtr;
	IceStorm::TopicManagerPrx	 _topicManager;
};

bool SubscriberSingle(const std::string &topicName, const IceStorm::TopicManagerPrx &topicManager, const ::IceStorm::QoS &Qos, const ::Ice::ObjectPrx &sink);

Ice::ObjectPrx PublisherSingle(const std::string &topicName, const IceStorm::TopicManagerPrx &topicManager/*, const ::IceStorm::QoS &Qos*/);

} }///endnamespace ZQ::EventClient

#endif //__ZQ_EventClient_H__
