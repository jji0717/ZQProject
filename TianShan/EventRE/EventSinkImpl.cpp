#include "EventSinkImpl.h"
#include "EventRuleEngine.h"
#include "TimeUtil.h"
#include "TsRepository.h"

//////////////////////////////////////////////////////////////////////////
//StreamEventSinkImpl
//////////////////////////////////////////////////////////////////////////
StreamEventSinkImpl::StreamEventSinkImpl(EventRuleEngine& ruleEngine)
:_ruleEngine(ruleEngine)
{

}

StreamEventSinkImpl::~StreamEventSinkImpl()
{

}

void StreamEventSinkImpl::ping(::Ice::Long, const ::Ice::Current& c)
{

}

void StreamEventSinkImpl::OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const ::TianShanIce::Properties& prop, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PID(%s) OnEndOfStream"), playlistId.c_str());	
}

void StreamEventSinkImpl::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const ::TianShanIce::Properties& prop, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PID(%s) OnBeginningOfStream"), playlistId.c_str());
}

void StreamEventSinkImpl::OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PID(%s) OnExit with code %d, reason %s"), 
		playlistId.c_str(), exitCode, reason.c_str());	
}

void StreamEventSinkImpl::OnSpeedChanged(const ::std::string&, const ::std::string&, ::Ice::Float, ::Ice::Float, const ::TianShanIce::Properties&, const ::Ice::Current&) const
{

}

void StreamEventSinkImpl::OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const ::TianShanIce::Properties&, const ::Ice::Current&) const
{

}

void StreamEventSinkImpl::OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int exitCode, const ::std::string& reason, const ::TianShanIce::Properties& props, const ::Ice::Current& c) const
{

}

//////////////////////////////////////////////////////////////////////////
//PlaylistEventSinkImpl
//////////////////////////////////////////////////////////////////////////
PlaylistEventSinkImpl::PlaylistEventSinkImpl(EventRuleEngine& ruleEngine)
: _ruleEngine(ruleEngine)
{
}

PlaylistEventSinkImpl::~PlaylistEventSinkImpl()
{
	/// 
}

void PlaylistEventSinkImpl::ping(::Ice::Long, const ::Ice::Current&)
{

}

void PlaylistEventSinkImpl::OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int curUserCtrlNum,::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current&) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PID(%s) OnEndOfItem, CtrlNum %d"), playlistId.c_str(), prevUserCtrlNum);	
}

//////////////////////////////////////////////////////////////////////////
//GenericEventSinkImpl
//////////////////////////////////////////////////////////////////////////
int64 GenericEventSinkImpl::allcount = 0;
GenericEventSinkImpl::GenericEventSinkImpl(EventRuleEngine& ruleEngine)
: _ruleEngine(ruleEngine)
{
}

GenericEventSinkImpl::~GenericEventSinkImpl()
{
	/// 
}

void GenericEventSinkImpl::ping(::Ice::Long, const ::Ice::Current&)
{

}

void GenericEventSinkImpl::post(
						  const ::std::string& category,
						  ::Ice::Int eventId,
						  const ::std::string& eventName,
						  const ::std::string& stampUTC,
						  const ::std::string& sourceNetId,
						  const ::TianShanIce::Properties& params,
						  const ::Ice::Current&
						  )
{

	//////////////////////////////////////////////////////////////////////////
	//for test only
	//////////////////////////////////////////////////////////////////////////
// 	ZQ::common::Action::Properties prop;
// 	STL_MAPSET(ZQ::common::Action::Properties, prop, "ProviderAssetId", "test123C0L0");
// 	STL_MAPSET(ZQ::common::Action::Properties, prop, "ProviderId", "1");
// 	STL_MAPSET(ZQ::common::Action::Properties, prop, "SubType", ".FF");
// 	STL_MAPSET(ZQ::common::Action::Properties, prop, "DemandedBy", "192.168.81.111");
// 	STL_MAPSET(ZQ::common::Action::Properties, prop, "RemoteLocator", "http://192.168.81.103:10000/");
// 	STL_MAPSET(ZQ::common::Action::Properties, prop, "streamcount", "200");
// 	_ruleEngine.OnEvent("ContentLocator", "RemoteAssetResolved", stampUTC, sourceNetId, prop);
	std::string requestName = "";
//	if(category == "ContentLocator" && eventName == "RemoteAssetResolved")
	{
		allcount++;

		std::string streamcount_str;

		::TianShanIce::Properties::const_iterator iter = params.find("ProviderId");
		if(iter != params.end())
		{
			requestName += iter->second;
		}

		iter = params.find("ProviderAssetId");
		if(iter != params.end())
		{
			requestName += iter->second;
		}

		iter = params.find("SubType");
		if(iter != params.end())
			requestName += iter->second;

		iter = params.find("DemandedBy");
		if(iter != params.end())
			requestName += iter->second;

		iter = params.find("RemoteLocator");
		if(iter != params.end())
			requestName += iter->second;
	}
	_ruleEngine.OnEvent(category, eventName, stampUTC, sourceNetId, params, requestName);
}

//////////////////////////////////////////////////////////////////////////
//StreamProgressSinkImpl
//////////////////////////////////////////////////////////////////////////
StreamProgressSinkImpl::StreamProgressSinkImpl(EventRuleEngine& ruleEngine)
: _ruleEngine(ruleEngine)
{
}

StreamProgressSinkImpl::~StreamProgressSinkImpl()
{
	/// 
}

void StreamProgressSinkImpl::ping(::Ice::Long, const ::Ice::Current&)
{

}

void StreamProgressSinkImpl::OnProgress(const ::std::string& proxy, const ::std::string& id, ::Ice::Int done, ::Ice::Int total, ::Ice::Int step, ::Ice::Int totalsteps, const ::std::string& comment, const TianShanIce::Properties& prop, const ::Ice::Current&) const
{
}

//////////////////////////////////////////////////////////////////////////
//SessionEventSinkImpl
//////////////////////////////////////////////////////////////////////////
SessionEventSinkImpl::SessionEventSinkImpl(EventRuleEngine& ruleEngine)
: _ruleEngine(ruleEngine)
{
}

SessionEventSinkImpl::~SessionEventSinkImpl()
{
	/// 
}

void SessionEventSinkImpl::OnNewSession(const ::std::string&, const ::std::string&, const ::Ice::Current&)
{
}

void SessionEventSinkImpl::OnDestroySession(const ::std::string&, const ::Ice::Current&)
{
}

void SessionEventSinkImpl::OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::State, ::TianShanIce::State, const ::Ice::Current&)
{
}

void SessionEventSinkImpl::ping(::Ice::Long, const ::Ice::Current&)
{

}
