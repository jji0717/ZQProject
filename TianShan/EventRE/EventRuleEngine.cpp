#include "EventRuleEngine.h"
#include "EventREConfig.h"
#include "EventRERequest.h"
#include "EventSinkImpl.h"

EventRuleEngine::EventRuleEngine(Log& log, NativeThreadPool& thpool, int depth, Ice::CommunicatorPtr& communicator)
: RuleEngine(log, thpool, depth), _evtAdap(NULL), _communicator(communicator)
{
}

EventRuleEngine::~EventRuleEngine()
{
}

bool EventRuleEngine::init()
{

	// create listen event adapter
	try 
	{
		_evtAdap = _communicator->createObjectAdapterWithEndpoints("EventRuleEngineAdapter", _config.ListenEventEndPoint);
		_evtAdap->activate();
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngine, "create adapter for listen event caught %s, endpoint is %s"), 
			ex.ice_name().c_str(), _config.ListenEventEndPoint.c_str());
		return false;
	}

	ConnectEventChannelRequest* pConnEventChannel = new ConnectEventChannelRequest(*this, _thpool);
	pConnEventChannel->start();

	return true;
}

void EventRuleEngine::unInit()
{

}

void EventRuleEngine::subscribeEvents(const ::std::string& topic)
{
	if (!_eventChannel)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngine, "not connected with event channel"));
		return;
	}
	TianShanIce::Properties qos;
	if(topic == ::TianShanIce::Streamer::TopicOfStream)
	{
		TianShanIce::Streamer::StreamEventSinkPtr _evtStream = new StreamEventSinkImpl(*this);
		_eventChannel->sink(_evtStream, qos);
		return;
	}
	else if(topic == ::TianShanIce::Streamer::TopicOfPlaylist)
	{
		TianShanIce::Streamer::PlaylistEventSinkPtr _evtPlaylist = new PlaylistEventSinkImpl(*this);
		_eventChannel->sink(_evtPlaylist, qos);
		return;
	}
	else if(topic == ::TianShanIce::Streamer::TopicOfStreamProgress)
	{
		::TianShanIce::Streamer::StreamProgressSinkPtr _evtStreamProgress = new StreamProgressSinkImpl(*this);
		_eventChannel->sink(_evtStreamProgress);
		return;
	}
	else if(topic == ::TianShanIce::SRM::TopicOfSession)
	{
		::TianShanIce::SRM::SessionEventSinkPtr _evtSession = new SessionEventSinkImpl(*this);
		_eventChannel->sink(_evtSession, qos);
		return;
	}
	else if(topic == ::TianShanIce::Events::TopicOfGenericEvent)
	{
		::TianShanIce::Events::GenericEventSinkPtr _evtGeneric = new GenericEventSinkImpl(*this);
		_eventChannel->sink(_evtGeneric, qos);
		return;
	}
}

bool EventRuleEngine::ConnectEventChannel()
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(EventRuleEngine, "do connectEventChannel()"));

	try
	{
		_eventChannel = new TianShanIce::Events::EventChannelImpl(_evtAdap, _config.TopicMgrEndPoint.c_str());
		EventREConfig::Topics::iterator iter = _config.topics.begin();
		if(_config.topics.empty())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngine, "no topic specified"));
			return false;
		}
		for (; iter != _config.topics.end(); ++iter) 
		{
			subscribeEvents(iter->name);
			glog(ZQ::common::Log::L_INFO, CLOGFMT(EventRuleEngineSVC, "EventRuleEngine::ConnectEventChannel() topic <%s> subscribed"), (iter->name).c_str());
		}
		_eventChannel->start();
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngine, "connectEventChannel(%s) caught(%s: %s)")
			, _config.TopicMgrEndPoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngine, "connectEventChannel(%s) caught(%s)")
			, _config.TopicMgrEndPoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngine, "connectEventChannel(%s) caught unexpect exception")
			, _config.TopicMgrEndPoint.c_str());
		return false;
	}

	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(EventRuleEngine, "connectEventChannel() successfully"));
	return true;
}

