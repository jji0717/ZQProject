// EventClient.cpp : Defines the entry point for the DLL application.
//

#include "EventClient.h"
#include "EventConnector.h"

#ifdef TEST_LIB
#include "EventClientMementoOri.h"
#endif


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


namespace ZQ{
	namespace EventClient{

///---------------------------------------
/// nested class GenEventSinkImpl                                   
///----------------------------------------
class GenEventSinkImpl : public TianShanIce::Events::GenericEventSink
{
public:
	explicit GenEventSinkImpl(GenericEventSession& genEventSession);
	virtual ~GenEventSinkImpl();
	typedef ::IceInternal::Handle< GenEventSinkImpl > Ptr;
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);
	virtual void post( const ::std::string& category, ::Ice::Int eventId,
		const ::std::string& eventName,	 const ::std::string& stampUTC,
		const ::std::string& sourceNetId, const ::TianShanIce::Properties& params,
		const ::Ice::Current&  = ::Ice::Current());

private:
	GenericEventSession&  _genEventSession;
};

		
//----------------------------
//start of class EventClient                                           
//----------------------------
EventClientException::EventClientException(const std::string &what_arg) throw()
	:ZQ::common::Exception(what_arg)
{
}

EventClientException::~EventClientException() throw(){}


EventClient::EventClient(ZQ::common::Log& log, Ice::ObjectAdapterPtr& adapter, 
	const ::TianShanIce::Properties& properties/*=TianShanIce::Properties()*/) throw(EventClientException)
	try:_log(log), _adapter(adapter)
{
	_topicManager = 0;
	_threadConnector = new EventConnector(*this, _log);
	_threadDispatcher = new EventDispatcher(*this, _log);

#ifdef TEST_LIB
	_clientMemoto = new EventClientMementoOri(*this);
#endif

	_log(ZQ::common::Log::L_INFO, CLOGFMT(EventClient, "EventClient instance Created"));
}
catch (...) 
{
	if (NULL != _threadConnector)
	{
		_threadConnector->exitConnectorThread();
			delete _threadConnector;
		_threadConnector = NULL;
	}

	if (NULL != _threadDispatcher)
	{
		_threadDispatcher->exitDispatchThread();
		delete _threadDispatcher;
		_threadDispatcher = NULL;
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(EventClient, "Create EventClient fail "));
	//throw EventClientException("Create EventClient fail "); 
};

EventClient::~EventClient(void)
{
	if (NULL != _threadConnector)
	{
		_threadConnector->setThreadRunning(false);
		EventConnector * pThreadCheckTemp = _threadConnector;
		_threadConnector = NULL;
		pThreadCheckTemp->exitConnectorThread();
		delete pThreadCheckTemp;
	}

	if (NULL != _threadDispatcher)
	{
		_threadDispatcher->setDispatchRunning(false);
		EventDispatcher * pThreadDispatchTemp = _threadDispatcher;
		_threadDispatcher = NULL;
		pThreadDispatchTemp->exitDispatchThread();
		delete pThreadDispatchTemp;
	}

	for (std::vector<EventSession* >::iterator it = _eventSessionList.begin();
		it != _eventSessionList.end();)
	{
		delete *it;
		_eventSessionList.erase(it);//Don't use clear() instead of erase in case other place iterator
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(EventClient, "Quite EventClient "));
}

bool EventClient::connect(const ::std::string& topicManagerEndpoint)//need another try
{
	::std::string _topicManagerEndpoint = topicManagerEndpoint.empty() ? DEFAULT_ENDPOINT_TopicManager : topicManagerEndpoint;
	if(!_adapter)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "EventClient connect failed, input adapter is null"));
		return false;
	}

	_adapter->activate();
	if (NULL == _threadDispatcher)
		_threadDispatcher = new EventDispatcher(*this, _log);

	if (NULL == _threadConnector)// start EventConnector
		_threadConnector = new EventConnector(*this, _log);
	else
		_threadConnector->setTopicManagerEndpoint(topicManagerEndpoint);//will get manager auto by Connector

	if (NULL != _threadConnector)// start EventConnector
		_threadConnector->start();

	if (NULL != _threadDispatcher)// start EventDispatcher
		_threadDispatcher->start();

	return TRUE;
}

void EventClient::disconnect()
{
	try
	{
		if (NULL != _threadConnector)// suspend EventConnector
			_threadConnector->suspendConnectorThread();

		if (NULL != _threadDispatcher)// suspend EventDispatcher
			_threadDispatcher->suspendDispatchThread();

		_eventSessionList.clear();

		{//leave lock as quickly as possable
			::ZQ::common::MutexGuard guard(_lockSubscribers);
			_subscribers.erase(_subscribers.begin(), _subscribers.end());// if needed, we should unsubscribe all of this before erase
		}

		{
			::ZQ::common::MutexGuard guard(_lockPublishers);
			_publishers.erase(_publishers.begin(), _publishers.end());
		}

		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "EventClient disconnect called"));
	}
	catch (...)
	{			
	}
}

bool EventClient::subscribe(const std::string sessionId, const ::Ice::ObjectPrx& subscriber, 
	const std::string& topicName, ::TianShanIce::Properties& properties /*= TianShanIce::Properties()*/)
{
	typedef   std::map<std::string, sessionPropertys>::iterator        LocalIterator;
	int nRev = TRUE;
	IceStorm::TopicPrx topic = NULL;
	::IceStorm::QoS Qos = properties;

	sessionProperty sessionproperty;
	sessionproperty.objPrx = subscriber;
	sessionproperty.qos = properties;

	SubscriberSingle(topicName, _topicManager, Qos, subscriber);

	_log(ZQ::common::Log::L_INFO, CLOGFMT(EventClient, "subscriber[%s] added to topicName[%s]"), sessionId.c_str(), topicName.c_str());
	::ZQ::common::MutexGuard guard(_lockSubscribers);// leave lock as quckly as possible
	LocalIterator subIter = _subscribers.find(topicName);
	if (subIter == _subscribers.end())
	{
		sessionPropertys sessionpropertys;
		sessionpropertys[sessionId] = sessionproperty;
		_subscribers[topicName] = sessionpropertys;
	}
	else
	{
		sessionPropertys &subTopicNameList = (*subIter).second;
		MAPSET(sessionPropertys, subTopicNameList, sessionId, sessionproperty);
	}

	return nRev;
}

Ice::ObjectPrx EventClient::addPublisher(const std::string sessionId, const std::string& topicName, ::TianShanIce::Properties& properties/* = TianShanIce::Properties()*/)
{
	typedef   TianShanIce::Events::GenericEventSinkPrx               LocalType;
	typedef   std::map<std::string, sessionPropertys>::iterator        LocalIterator;
	Ice::ObjectPrx publisher = NULL;
	try
	{
		publisher = PublisherSingle(topicName, _topicManager);
		LocalType checkedPub = LocalType::uncheckedCast(publisher);
	}
	catch (const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "addPublisher() adding publisher[%s] to topic[%s] caught %s"), sessionId.c_str(), topicName.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "addPublisher() adding publisher[%s] to topic[%s] caught exception"), sessionId.c_str(), topicName.c_str());
	}

	sessionProperty sessionproperty;
	sessionproperty.objPrx = publisher;
	sessionproperty.qos = properties;

	::ZQ::common::MutexGuard guard(_lockPublishers);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventClient, "registering publisher[%s](%p) to topic[%s]"), sessionId.c_str(), publisher.get(), topicName.c_str());
	LocalIterator pubIter = _publishers.find(topicName);
	if (pubIter == _publishers.end())
	{
		sessionPropertys sessionpropertys;
		sessionpropertys[sessionId] = sessionproperty;
		_publishers[topicName] = sessionpropertys;
	}
	else
	{
		sessionPropertys &pubTopicNameList = (*pubIter).second;
		MAPSET(sessionPropertys, pubTopicNameList, sessionId, sessionproperty);
	}

	return publisher;
}

Ice::ObjectPrx EventClient::getPublish(const std::string topicName, const std::string sessionId)
{
	typedef   std::map<std::string, sessionPropertys>::iterator        LocalIterator;
	typedef   std::map<std::string, sessionProperty >::iterator        LocalIterPro;

	Ice::ObjectPrx prxRev(NULL);
	::ZQ::common::MutexGuard guard(_lockPublishers);
	LocalIterator pubIter = _publishers.find(topicName);
	if (_publishers.end() == pubIter)
	{
		return prxRev;
	}

	sessionPropertys &pubTopicNameList = (*pubIter).second;
	LocalIterPro pubPro = pubTopicNameList.find(sessionId);
	if (pubTopicNameList.end() != pubPro)
	{
		prxRev = (*pubPro).second.objPrx;
	}

	if (NULL == prxRev.get())
	{
		prxRev = PublisherSingle(topicName, _topicManager);
		(*pubPro).second.objPrx = prxRev;
	}

	return prxRev;
}
//-----------------------------------
//end   of class EventClient                                           
//-----------------------------------



///----------------------------------
//start of class EventSession                                          
//-----------------------------------
EventSession::EventSession(EventClient& client, const std::string &topicName, const ::IceStorm::QoS &Qos)
	:_client(client), _log(client._log), _SessionTopicName(topicName), _Qos(Qos)
{
	_isFirstLogin = true;
};
//------------------------------
//end   of class EventSession                                          
//------------------------------



///---------------------------------------
///start of class GenericEventSession                                   
///----------------------------------------
GenericEventSession::GenericEventSession(EventClient& client,
										 const std::string topicName, TianShanIce::Properties& properties, bool asProducer/* =true*/, bool asConsumer/*=false*/) 
										 try:EventSession(client, topicName, properties), _topicName(topicName)
{
	TianShanIce::Properties tmp = properties;
	setSessionId(IceUtil::generateUUID());
	Ice::ObjectPrx sink = NULL;
	_topicName = topicName;
	_bDisconnect = false;
	_topicManager = _client._topicManager;
	_client._eventSessionList.push_back(this);
	_asProducer = asProducer;
	_asConsumer = asConsumer;

	if(asProducer)
	{	
		_client.addPublisher(getSessionId(), _topicName);
	}

	if(asConsumer)
	{
		_topicName = topicName;
		sink = _client._adapter->addWithUUID(new GenEventSinkImpl(*this));
		_client.subscribe(getSessionId(), sink, _topicName, properties);
	}
}
catch (...) 
{
	_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "GenericEventSession() create failed!"));
};


GenericEventSession::~GenericEventSession()
{
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventClient, "GenericEventSession released"));
}

bool GenericEventSession::sendMessage(Message message)
{
	if(!_asProducer)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "sendMessage() session[%s] is not publisher, skip msg: id[%d], event[%s]"), getSessionId().c_str(), message.id, message.eventName.c_str());
		return false;
	}

	if(!_bDisconnect)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "sendMessage() session[%s] publisher is not connected, skip msg: id[%d], event[%s]"), getSessionId().c_str(), message.id, message.eventName.c_str());
		return false;
	}

	//send to the eventchannel of topic
	TianShanIce::Events::GenericEventSinkPrx pubPrx = NULL;
	Ice::ObjectPrx sendPrx = _client.getPublish(_topicName, getSessionId());
	pubPrx = TianShanIce::Events::GenericEventSinkPrx::uncheckedCast(sendPrx);

	if (!pubPrx)
	{
		Ice::ObjectPrx sendPrx = _client.addPublisher(getSessionId(), _topicName);
		pubPrx = TianShanIce::Events::GenericEventSinkPrx::uncheckedCast(sendPrx);
	}

	if (!pubPrx)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "sendMessage() session[%s], publisher is null, skip msg: id[%d], event[%s]"), getSessionId().c_str(), message.id, message.eventName.c_str());
		return false;
	}

	try 
	{
		pubPrx->post(message.category, message.id, message.eventName, message.timestamp, message.sourceNetId, message.property);
	}			
	catch (const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "sendMessage() session[%s], publisher skip msg: id[%d], event[%s] caught %s"), getSessionId().c_str(), message.id, message.eventName.c_str(), ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventClient, "sendMessage() session[%s], publisher skip msg: id[%d], event[%s] caught unhandle exception"), getSessionId().c_str(), message.id, message.eventName.c_str());
		return false;
	}

	return true;
}

void GenericEventSession::OnMessage(Message& message)
{
}

void GenericEventSession::OnConnected(const std::string& notice)
{
    _bDisconnect = true;
}

void GenericEventSession::OnConnectionLost(const std::string& notice)
{
    _bDisconnect = false;
}

GenEventSinkImpl::GenEventSinkImpl(GenericEventSession& genEventSession)
	:_genEventSession(genEventSession)
{
	_genEventSession.storeEventSink(this);
};

GenEventSinkImpl::~GenEventSinkImpl()
{
}

void GenEventSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current&)
{
}

void GenEventSinkImpl::post( const ::std::string& category,
	::Ice::Int eventId,
	const ::std::string& eventName,
	const ::std::string& stampUTC,
	const ::std::string& sourceNetId,
	const ::TianShanIce::Properties& params,
	const ::Ice::Current& /* = ::Ice::Current */)
{
	GenericEventSession::Message message;	
	message.id = eventId;
	message.eventName = eventName;
	message.category = category;
	message.sourceNetId = sourceNetId;
	message.timestamp = stampUTC;
	message.property = params;
	_genEventSession.OnMessage(message);
};

//--------------------------------------
//end of class GenericEventSession                                     
//--------------------------------------

bool SubscriberSingle(const std::string &topicName, const IceStorm::TopicManagerPrx &topicManager, const ::IceStorm::QoS &Qos, const ::Ice::ObjectPrx &sink)
{
	IceStorm::TopicPrx topic = NULL;
	int nRev = true;

	try
	{
		topic = topicManager->retrieve(topicName);
	}
	catch(const IceStorm::NoSuchTopic&)
	{
		try
		{
			topic = topicManager->create(topicName);
		}
		catch(const IceStorm::TopicExists&)
		{
			nRev = false;
		}
		catch(...)
		{
			nRev = false;
		}
	}
	catch (...) {
		nRev = false;
	}

	if (!nRev)
	{
		return nRev;
	}

	try{
		topic->unsubscribe(sink);
		topic->subscribe(Qos, sink);
	}catch (...){}

	return nRev;
}

Ice::ObjectPrx  PublisherSingle(const std::string &topicName, const IceStorm::TopicManagerPrx &topicManager/*, const ::IceStorm::QoS &Qos*/)
{
	IceStorm::TopicPrx topic = NULL;
	Ice::ObjectPrx nRev(NULL);
	try
	{
		topic = topicManager->retrieve(topicName);
	}
	catch(const IceStorm::NoSuchTopic&){
		try
		{
			topic = topicManager->create(topicName);
		}catch(const IceStorm::TopicExists&){
			nRev = NULL;
		}
	}catch (...) {
		nRev = NULL;
	}

	if (!topic)
	{
		return nRev;
	}	

	try	{
		nRev = topic->getPublisher();
	}catch (...){
		nRev = NULL;
	}

	return nRev;
}

	}// end namespace EventClient
}// end namespace ZQ