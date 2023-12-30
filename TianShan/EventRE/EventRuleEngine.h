#ifndef __EVENTRULEENGINE_H__
#define __EVENTRULEENGINE_H__

#include "RuleEngine.h"
#include "Log.h"
#include "EventChannel.h"
#include "TianShanDefines.h"

using namespace ZQ::common;
using namespace IceUtil;

class EventRuleEngine : public ZQ::common::RuleEngine
{
public:
	EventRuleEngine(Log& log, NativeThreadPool& thpool, int depth, Ice::CommunicatorPtr& communicator);
	~EventRuleEngine();

	virtual void subscribeEvents(const ::std::string& topic);

	bool ConnectEventChannel();

	bool init();

	void unInit();

public:
	::Ice::ObjectAdapterPtr							_evtAdap;
	TianShanIce::Events::EventChannelImpl::Ptr		_eventChannel;
	Ice::CommunicatorPtr&							_communicator;
};

#endif
