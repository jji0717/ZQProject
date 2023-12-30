#ifndef __ZQ_EventClientConnector_H__
#define __ZQ_EventClientConnector_H__

#include "EventClient.h"
#include "ClientConnectorMessenger.h"

namespace ZQ {
namespace EventClient {

#define DEFAULT_ENDPOINT_TopicManager	"default -p 10100"
#define SERVICE_NAME_TopicManager		"TianShanEvents/TopicManager"

//dispatch-thread
class EventDispatcher : public ZQ::common::NativeThread
{
public:
	enum stDispatchEvent
	{
		NOEVENT= 0,
		Event_Connected,
		Event_ConnectionLost,
		ENUM_COUNT
	};

	static char *errorString[ENUM_COUNT + 1];

public:
	explicit EventDispatcher(EventClient &eventclient, ZQ::common::Log & log);

	virtual bool start(void);
	virtual int  run(void);

	void exitDispatchThread(void);
	int  suspendDispatchThread(void);

public:
	bool  setDispatchRunning(int nRunning = true){return (_bRunning = nRunning);}
	bool  updateEvent(stDispatchEvent eEvent){return (_stEvent = eEvent);}

protected:
	bool OnTopicManagerLost(void);
	bool OnTopicManagerConnect(void);

protected:
	bool refreshSubscribers(void);
	bool refreshPublishers(void);

private:
	EventClient &             _eventClient;
	ZQ::common::Log &         _log;
	long                     _microSec;
	bool                     _bRunning;
	stDispatchEvent           _stEvent;
};//end class EventDispatcher


//reconnect-thread
class EventConnector : public ZQ::common::NativeThread
{
public:
	explicit EventConnector(EventClient &eventclient, ZQ::common::Log& log);
	virtual ~EventConnector(void);

public:
	virtual bool start(void);
	virtual int  run(void);
	
	void exitConnectorThread(void);
	int  suspendConnectorThread(void);

	bool setThreadRunning(int nRunning = true);
	bool setTopicManagerEndpoint(const ::std::string& topicManagerEndpoint);

protected:
	int     checkStateMain(void);
	int     checkTopicManageState(void);
	int     checkSessionState(void);
	int     echoMessage(void);//messenger lost ,means that all session connect lost
	int     checkSubscribers(void);//subscribers would be kicked out from topic
	int     checkSubscriberSingle(ZQ::EventClient::EventClient::sessionPropertys::iterator &it);
	int     checkSubscribersByTopicName(ZQ::EventClient::EventClient::sessionPropertysMap::iterator &it);
	int     getConnectorTopicManager(void);

private:
	long                     _nLockWaitTime;
	long                     _nThreadTick;
	bool                     _bRunning;
	ClientConnectorMessenger   _stMessenger;

private:
	ZQ::common::Log&                 _log;
	EventClient &                   _eventClient;
};//end class EventConnector

	}///endnamespace ZQ::EventClient
}///endnamespace ZQ
#endif//__ZQ_EventClientConnector_H__