#include "EventConnector.h"

namespace ZQ {
namespace EventClient {

//-----------------------------------
// class  EventConnector     start                                   
//-----------------------------------
EventConnector::EventConnector(EventClient &eventclient, ZQ::common::Log& log)
	:_stMessenger(_log), _eventClient(eventclient), _log(log)
{
	setThreadRunning(true);
	_nThreadTick = 1000;
	_nLockWaitTime = 1000;
	//TODO: both connector and dispatcher should be instanced and started by EventClient
}

EventConnector::~EventConnector(void)
{
	setThreadRunning(false);			
};

bool EventConnector::start(void)
{
	typedef ZQ::common::NativeThread LocalType;

	_nThreadTick = 1000;
	setThreadRunning(true);
	LocalType::start();
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(reconnect-thread,"reconnect-thread start"));

	return true;
};

void EventConnector::exitConnectorThread(void)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(reconnect-thread,"reconnect-thread exit"));
	if (!_bRunning)
		NativeThread::exit();
}

int EventConnector::suspendConnectorThread(void)
{
	if (_bRunning)
	{
		NativeThread::suspend();
		setThreadRunning(false);
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "reconnect-thread suspend"));
	}
	else
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "reconnect-thread not running, suspend failed"));
	}
	
	return true;
}

bool EventConnector::setThreadRunning(int nRunning)
{ 
	return (_bRunning = nRunning);
}

bool EventConnector::setTopicManagerEndpoint(const ::std::string& topicManagerEndpoint)
{
	_eventClient._topicManagerEndpoint.empty();
	_eventClient._topicManagerEndpoint = topicManagerEndpoint;
	_stMessenger.setMessengerEndpoint(topicManagerEndpoint);

	return true;
}

int EventConnector::run(void)
{
#define MAX_PING_INTERVAL       (30)
#define DEFAULT_PING_INTERVAL   (60)
#define MIN_PING_INTERVAL       (2)

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(reconnect-thread,"reconnect-thread running"));
	unsigned long nextSleep = 0;
	unsigned int nRetryOnError =0;
	_eventClient._bConnected = false;

	while (_bRunning)
	{
		_eventClient._hLock.wait(1000);
		if (nextSleep >0)
			IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(nextSleep));

		try{
			if (!checkStateMain())
			{
				if (nRetryOnError <=0)
					nRetryOnError =1;
				else
					nRetryOnError *=2;
			}
			else 
				nRetryOnError =0;

		}
		catch(...)
		{

		}

		if (nRetryOnError <=0 && ! _eventClient._bConnected)
		{
			//TODO: client[xxx] just connected to EventChannel[endpoint]
			// TODO: log and ask the dispatcher to dispatch Connected event to all sessions
			_eventClient._hLock.signal();
			_eventClient._threadDispatcher->updateEvent(EventDispatcher::Event_Connected);
		}
		else if (_eventClient._bConnected && nRetryOnError>0)
		{
			//TODO: client[xxx] lost connection to EventChannel[endpoint]
			// TODO: ask the dispatcher to dispatch ConnectionLost event to all sessions
			_eventClient._hLock.signal();
			_eventClient._threadDispatcher->updateEvent(EventDispatcher::Event_ConnectionLost);
		}

		_eventClient._bConnected = (nRetryOnError <=0);

		nextSleep = DEFAULT_PING_INTERVAL; // TODO should be configurable, default 30sec

		if (nRetryOnError>0)
		{
			nextSleep = MIN_PING_INTERVAL; // TODO: saying 2sec
			for (int i =0; i <nRetryOnError; i++)
				nextSleep *=2;
		}

		if (nextSleep > MAX_PING_INTERVAL)
			nextSleep = MAX_PING_INTERVAL;
	}

	return 0;
};

int EventConnector::checkStateMain(void)
{
	int nRev = true;
	std::vector<EventSession* > & SessionList = _eventClient._eventSessionList;
	if (SessionList.empty())
	{// no session, nop
		return nRev;
	}

	if (!_eventClient._topicManager)
	{
		nRev = getConnectorTopicManager();
		_eventClient._adapter->activate();
		return nRev;
	}

	if(!checkTopicManageState())//sequence important
	{
		nRev = false;//recheck as quickly as possible
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(reconnect-thread, "reconnect-thread connection lost, by ping"));
	}

	if (nRev && !echoMessage()) // TODO: echoMessage()
	{
		nRev = false;
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(reconnect-thread, "reconnect-thread connection lost with eventchannel"));
	}

	if (nRev != _eventClient._bConnected)
		return nRev;

	if (!checkSessionState())
	{
		nRev = false;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "reconnect-thread session need response"));
	}
	else if(!checkSubscribers())
	{
		nRev = false;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "reconnect-thread Subscribers connected lost"));
	}

	return nRev;
}


int EventConnector::getConnectorTopicManager(void)
{
	Ice::ObjectAdapterPtr adapter = _eventClient._adapter;
	int nRev = true;

	try 
	{
		_eventClient._topicManager = IceStorm::TopicManagerPrx::checkedCast(adapter->getCommunicator()->stringToProxy(_eventClient._topicManagerEndpoint));
	}
	catch (const ::Ice::UserException & ex) 
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "reconnect-thread try to get topic manager by input endpoint failed "));
	}
	catch (...) {}

	if(!_eventClient._topicManager)// if this is not a full proxy string, then trust it as a endpoint
	{
		try {
			std::string proxy = SERVICE_NAME_TopicManager ":" ;
			proxy += _eventClient._topicManagerEndpoint;
			_eventClient._topicManager = IceStorm::TopicManagerPrx::checkedCast(adapter->getCommunicator()->stringToProxy(proxy));
		}
		catch (...)
		{
			_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "reconnect-thread try to get topic manager by default endpoint failed"));
			nRev = false;//unlock coroutine thread to handle topicManager problem
		}
	}

	if (!nRev)//for response OnConnected / OnConnectionLost
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "connected to EventChannel[%s]  failed"), _eventClient._topicManagerEndpoint.c_str());
	}
	else
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "connected to EventChannel[%s] successfully"), _eventClient._topicManagerEndpoint.c_str());
		_stMessenger.resetLoseHeartBeat(0);
	}

	adapter->activate();
	return nRev;
}

int EventConnector::checkTopicManageState(void)
{
	int nRev = true;
	try
	{
		_eventClient._topicManager->ice_ping();
	}
	catch (...)
	{
		_eventClient._topicManager = NULL;
		nRev = false;
	}
	
	return nRev;
}

int EventConnector::checkSessionState(void)
{
	int nRev = true;
	std::vector<EventSession* >& eventSessionHead = _eventClient._eventSessionList;
	for (std::vector<EventSession* >::iterator it = eventSessionHead.begin(); it != eventSessionHead.end(); ++it)
	{
		if (!(*it)->getConnected())
		{
			nRev = false;
			break;// Any connected lost will make EventDispatcher thread wakeup then work.
		}
	}

	return nRev;
}

int EventConnector::echoMessage(void)
{
	int nRev = _stMessenger.checkMessengerState();
	if (!nRev)
	{
		_eventClient._topicManager = NULL;//for get topicmanager again
	}

	return nRev;
}

int EventConnector::checkSubscribers(void)
{
	EventClient::sessionPropertysMap::iterator   pos;
	EventClient::sessionPropertys::iterator      iteratorPos;
	EventClient::sessionPropertys               subscriberSingle;
	EventClient::sessionPropertysMap &          subscribers    = _eventClient._subscribers;
	int nRev = true;

	if (subscribers.empty() || subscribers.size() < 1)
	{
		return nRev;
	}

	for (pos = subscribers.begin(); nRev && pos != subscribers.end(); ++pos)
	{
		if (!checkSubscribersByTopicName(pos))
		{
			nRev = false;
			break;
		}
	}

	return nRev;
}

int EventConnector::checkSubscribersByTopicName(ZQ::EventClient::EventClient::sessionPropertysMap::iterator &it)
{
	EventClient::sessionPropertys &         subscribersByTopicName = (*it).second;
	EventClient::sessionPropertys::iterator  iteratorPos;
	int nRev = true;

	for (iteratorPos = subscribersByTopicName.begin(); iteratorPos != subscribersByTopicName.end(); ++iteratorPos)
	{
		if (!checkSubscriberSingle(iteratorPos))
		{
			nRev = false;
			break;
		}
	}

   return nRev;
}

int EventConnector::checkSubscriberSingle(ZQ::EventClient::EventClient::sessionPropertys::iterator &it)
{
	EventClient::sessionProperty & sessionProperty = (*it).second;
	::Ice::ObjectPrx & sink = sessionProperty.objPrx;
	int nRev = true;

	try
	{
		sink->ice_ping();
	}
	catch(::Ice::UserException & ex)
	{
		nRev = false;
	}
	catch (...)
	{
		nRev = false;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(reconnect-thread, "checkSubscriberSingle topicName[%s]  failed"), it->first.c_str());
	}

	return nRev;
}
// class EventConnector    end 
//--------------------------------


///-------------------------------
//class EventDispatcher   start                                         
//--------------------------------
EventDispatcher::EventDispatcher(EventClient &eventclient, ZQ::common::Log& _log)
	:_eventClient(eventclient), _log(_log)
{
	_stEvent = NOEVENT;
	_microSec = 1000;
	setDispatchRunning(true);
}

bool EventDispatcher::start(void)
{
	setDispatchRunning(true);
	NativeThread::start();
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(dispatch-thread, "dispatch-thread start"));

	return TRUE;
}

void EventDispatcher::exitDispatchThread(void)
{
	if (!_bRunning)
	{
		setDispatchRunning(false);
		NativeThread::exit();
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(dispatch-thread, "dispatch-thread exit"));
	}
}

int  EventDispatcher::suspendDispatchThread(void)
{
	if (_bRunning)
	{
		NativeThread::suspend();
		setDispatchRunning(false);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(dispatch-thread, "dispatch-thread suspend"));
	}
	else
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(dispatch-thread, "dispatch-thread not running, suspend failed"));
	}

	return true;
}

bool EventDispatcher::OnTopicManagerLost(void)// tell every session that topic manager lost
{
	std::vector<EventSession* >& eventSessionHead = _eventClient._eventSessionList;
	std::string notice("Topic manager connect losted ");
	for (std::vector<EventSession* >::iterator it = eventSessionHead.begin();
		it != eventSessionHead.end(); ++it)
	{
		if ((*it)->isFirstLogin() || (*it)->getConnected())//sequence is important
		{
			(*it)->setConnected(false);
			(*it)->OnConnectionLost(notice);
		}
	}

	refreshPublishers();//it will empty all publishers objPrx to null
	return true;
}

bool EventDispatcher::OnTopicManagerConnect(void)
{ 
	std::vector<EventSession* >& eventSessionHead = _eventClient._eventSessionList;
	std::string notice("Topic manager connected ");
	for (std::vector<EventSession* >::iterator it = eventSessionHead.begin();
		it != eventSessionHead.end(); ++it)
	{
		if ((*it)->isFirstLogin() || !(*it)->getConnected())
		{
			(*it)->setConnected(true);
			(*it)->OnConnected(notice);
		}
	}

	refreshPublishers();//sequence is very important
	refreshSubscribers();
	return true;
}

bool EventDispatcher::refreshSubscribers(void)
{
	IceStorm::TopicManagerPrx         topicManager = _eventClient._topicManager;
	EventClient::sessionPropertysMap   subscribers = _eventClient._subscribers;
	EventClient::sessionPropertys *    PropertyMap = NULL;
	if (!topicManager)
		return false;

	for (EventClient::sessionPropertysMap::iterator Iter = subscribers.begin(); 
		Iter != subscribers.end(); ++Iter)
	{
		PropertyMap = &((*Iter).second);
		for (EventClient::sessionPropertys::iterator IterPro = (*PropertyMap).begin(); IterPro != (*PropertyMap).end(); ++IterPro)
		{
			SubscriberSingle((*Iter).first, topicManager, (*IterPro).second.qos, (*IterPro).second.objPrx);
		}
	}

	_eventClient._adapter->activate();
	return true;
}

bool EventDispatcher::refreshPublishers(void)
{
	IceStorm::TopicManagerPrx         topicManager = _eventClient._topicManager;
	EventClient::sessionPropertysMap   publishers = _eventClient._publishers;
	EventClient::sessionPropertys *    PropertyMap = NULL;

	for (EventClient::sessionPropertysMap::iterator Iter = publishers.begin(); Iter != publishers.end(); ++Iter)
	{
		PropertyMap = &((*Iter).second);
		for (EventClient::sessionPropertys::iterator IterPro = (*PropertyMap).begin(); IterPro != (*PropertyMap).end(); ++IterPro)
		{
			(*IterPro).second.objPrx = NULL;
			(*IterPro).second.objPrx = PublisherSingle((*Iter).first, topicManager);//publish and restore objprx;
		}
	}

	return true;
}

char*  EventDispatcher::errorString[EventDispatcher::stDispatchEvent::ENUM_COUNT + 1] = {
	"NOEVENT",
	"Event_Connected",
	"Event_ConnectionLost",
	"ENUM_COUNT"
};

 int  EventDispatcher::run(void)
 {			
	 _log(ZQ::common::Log::L_DEBUG, CLOGFMT(dispatch-thread, "dispatch-thread running"));
	 stDispatchEvent eEventHandle = NOEVENT;
	 while (_bRunning)
	 {
		 try	
		 {
			 _eventClient._hLock.wait(TIMEOUT_INF);
			 _eventClient._hLock.signal();
			 eEventHandle = _stEvent;
			 updateEvent(NOEVENT);

			 switch(eEventHandle)
			 {
			 case Event_Connected:
				 // TODO: for each session, call OnConnected()
				 OnTopicManagerConnect();
				 break;

			 case Event_ConnectionLost:
				 // TODO: for each session, call OnConnectionLost(), reset the publisher to null if session._asProcedurer=true
				 OnTopicManagerLost();
				 break;

			 case NOEVENT:
				 break;

			 default:
				 break;
			 }

			 _log(ZQ::common::Log::L_INFO, CLOGFMT(EventDispatcher, "run() event[%s] dispatched"), errorString[eEventHandle]);

			 IceUtil::ThreadControl::sleep(IceUtil::Time::microSeconds(_microSec));
		 } catch (...)
		 {
			 _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventDispatcher, "dispatch-thread restart in exeption"));
		 }
	 }

	 _log(ZQ::common::Log::L_DEBUG, CLOGFMT(dispatch-thread, "dispatch-thread quit"));
	 return TRUE;
 }

 //
//class EventDispatcher end
//

}///endnamespace ZQ::EventClient
}///endnamespace ZQ