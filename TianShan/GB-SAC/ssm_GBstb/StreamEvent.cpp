#include "./StreamEvent.h"
#include <TianShanIceHelper.h>

extern ZQ::common::Log* s1log;

namespace TianShanS1
{

	StreamEvent::StreamEvent(Environment& env, StreamEventDispatcher& eventDispatcher)
		:EventSinkI(env, eventDispatcher)
	{
	}

	StreamEvent::~StreamEvent()
	{
	}

	void StreamEvent::ping(::Ice::Long lv, const ::Ice::Current& ic)
	{
	}

	void StreamEvent::OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props , const ::Ice::Current& ic) const
	{
		sendEvent(proxy, playlistId, streamEventENDOFSTREAM, props, ic);
	}

	void StreamEvent::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props , const ::Ice::Current& ic) const
	{
		sendEvent(proxy, playlistId, streamEventBEGINOFSTREAM, props, ic);
	}

	void StreamEvent::OnSpeedChanged(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props , const ::Ice::Current& ic) const
	{
		TianShanIce::Properties tmpProps = props;
		ZQTianShan::Util::updatePropertyData(tmpProps, "prevSpeed", prevSpeed);
		ZQTianShan::Util::updatePropertyData(tmpProps, "currentSpeed", currentSpeed);
		sendEvent(proxy, playlistId, streamEventSPEEDCHANGE, tmpProps, ic);
	}

	void StreamEvent::OnStateChanged(const ::std::string& proxy, const ::std::string& playlistId, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties& props ,  const ::Ice::Current& ic) const 
	{
		TianShanIce::Properties tmpProps = props;
		ZQTianShan::Util::updatePropertyData(tmpProps, "prevState", (int)prevState);
		ZQTianShan::Util::updatePropertyData(tmpProps, "currentState", (int)currentState);
		sendEvent(proxy, playlistId, streamEventSTATECHANGE, tmpProps, ic);
	}

	void StreamEvent::OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic) const
	{
		TianShanIce::Properties props;
		sendEvent(proxy, playlistId, streamEventEXIT, props, ic);
	}

	void StreamEvent::OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::TianShanIce::Properties& props, const ::Ice::Current& ic) const
	{
		sendEvent(proxy, playlistId, streamEventEXIT2, props, ic);
	}

} // namespace TianShanS1

