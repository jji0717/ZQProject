#ifndef __ZQ_ClientConnectorMessenger_H__
#define __ZQ_ClientConnectorMessenger_H__

#include <TianShanDefines.h>
#include <TsEvents.h>
#include <IceStorm/IceStorm.h>
#include "ZQ_common_conf.h"
#include "SystemUtils.h"
#include "Log.h"

namespace ZQ {
	namespace EventClient {

class ClientConnectorMessenger : public TianShanIce::Events::BaseEventSink
{
public:
	explicit ClientConnectorMessenger(ZQ::common::Log& log);
	virtual ~ClientConnectorMessenger();

	virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);	
	int checkMessengerState(void);

public:
	int   setMessengerEndpoint(::std::string topicManagerEndpoint);
	long  resetLoseHeartBeat(long nLoseHeartBeat = 0);//call by reconnect-thread

private:
	int   checkMessengerResponse(void);

private:
	long    _waitBeatTime;
	long    _loseHeartBeat;
	long    _minBeat;

private:
	SYS::SingleObject          _hEvent;
	ZQ::common::Log&           _log;

private:
	typedef   TianShanIce::Events::BaseEventSinkPrx  SinkPrx;
	Ice::CommunicatorPtr        _communicator;
	IceStorm::TopicManagerPrx   _topicMgr;
	::std::string              _topicManagerEndpoint;
	Ice::ObjectAdapterPtr       _pAdapter;
	::std::string              _adapterId;
	::std::string              _messagerId;
	::std::string              _topicName;
	::Ice::ObjectPrx            _messengerPrx;
	SinkPrx                    _publisherPrx;
};

	}///endnamespace ZQ::EventClient
}///endnamespace ZQ
#endif//__ZQ_ClientConnectorMessenger_H__