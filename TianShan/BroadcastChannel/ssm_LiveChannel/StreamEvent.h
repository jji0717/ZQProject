#ifndef __TianShanS1_StreamEvent_H__
#define __TianShanS1_StreamEvent_H__

#include "./Environment.h"

namespace LiveChannel
{
	class StreamEvent : public EventSinkI, public TianShanIce::Streamer::StreamEventSink  
	{
	public:
		StreamEvent(Environment& env, StreamEventDispatcher& eventDispatcher);
		virtual ~StreamEvent();
		void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
		void OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props ,const ::Ice::Current& ic= ::Ice::Current()) const;
		void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props ,  const ::Ice::Current& ic= ::Ice::Current()) const;
		void OnSpeedChanged(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props , const ::Ice::Current& ic = ::Ice::Current()) const;
		void OnStateChanged(const ::std::string& proxy, const ::std::string& playlistId, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties& props ,  const ::Ice::Current& ic= ::Ice::Current()) const ;
		void OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;
		void OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::TianShanIce::Properties& props, const ::Ice::Current& = ::Ice::Current()) const;

	}; // class StreamEvent

} // namespace LiveChannel

#endif // #define __TianShanS1_StreamEvent_H__

