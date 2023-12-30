#ifndef __ZQ_EventClientTest_H__
#define __ZQ_EventClientTest_H__


#include "EventClient.h"
//#include "EventClientTestMain.h"
#include <iostream>
#include <Windows.h>
#include "stdio.h"

using namespace std;
using namespace ZQ::EventClient;

#define  TEST_LIB

class EventClientTestMain;

class MyCppSession : public ::ZQ::EventClient::GenericEventSession
{
public:
	MyCppSession(ZQ::common::Log& log, EventClient& client, const std::string& topicName, TianShanIce::Properties& properties, bool asProducer =true, bool asConsumer=false, EventClientTestMain *testMain = NULL)
		throw(::ZQ::EventClient::EventClientException);

	virtual ~MyCppSession();

	virtual void OnConnected(const std::string& notice);
	virtual void OnConnectionLost(const std::string& notice);

	virtual void OnMessage(Message& message);

private:
    ZQ::common::Log&      _log;
	EventClientTestMain *  _testMain;
	std::string           _sessionId;
	std::string           _topicName;
	std::string           _strId;
};



#endif //__ZQ_EventClientTest_H__