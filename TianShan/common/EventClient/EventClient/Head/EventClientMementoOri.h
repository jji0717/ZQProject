#ifndef __ZQ_EventClientMementoOri_H__
#define __ZQ_EventClientMementoOri_H__

#include "EventClient.h"

namespace ZQ {
	namespace EventClient{

class EventClientMementoOri
{
public:
	EventClientMementoOri(EventClient& eventClient):_eventClient(eventClient){}
	~EventClientMementoOri(){};


	Ice::ObjectAdapterPtr EventClientMementoOri::getClientAdapter(void)
	{ 
		return (_eventClient._adapter);
	}

	IceStorm::TopicManagerPrx  EventClientMementoOri::getClientTopicManager(void)
	{
		return (_eventClient._topicManager);
	}

	std::vector<EventSession* > &  EventClientMementoOri::getEventSessionList(void)
	{
		return (_eventClient._eventSessionList);
	}

	EventClient::sessionPropertysMap  EventClientMementoOri::getSubScribers(void)
	{
		return (_eventClient._subscribers);
	}

	EventClient::sessionPropertysMap &  EventClientMementoOri::getPublishers(void)
	{
		return (_eventClient._publishers);
	}


	bool  EventClientMementoOri::connect(const ::std::string& topicManagerEndpoint)
	{
		return _eventClient.connect(topicManagerEndpoint);
	}

	bool  EventClientMementoOri::disconnect(void)
	{
		 _eventClient.disconnect();
		 return true;
	}

public:
	bool  setClientTopicManager(IceStorm::TopicManagerPrx topicManager){return (_eventClient._topicManager = topicManager);}

private:
	EventClient & _eventClient;
};

	}//endnamespace ZQ::EventClient
}///endnamespace ZQ

#endif //__ZQ_EventClientMementoOri_H__