#ifndef __TianShanS1_PlaylistEvent_H__
#define __TianShanS1_PlaylistEvent_H__

#include "./Environment.h"

namespace TianShanS1
{
	class PlaylistEvent : public EventSinkI, public TianShanIce::Streamer::PlaylistEventSink  
	{
	public: 
		typedef IceInternal::Handle<SessionContextImpl> Ptr;
		PlaylistEvent(Environment& env, StreamEventDispatcher& eventDispatcher);
		virtual ~PlaylistEvent();
		void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
		void OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, 
			::Ice::Int userCtrlNum,::Ice::Int prevCtrlNum, 
			const ::TianShanIce::Properties& prty,
			const ::Ice::Current& ic = ::Ice::Current()) const;

	}; // class PlaylistEvent

} // namespace TianShanS1

#endif // define __TianShanS1_PlaylistEvent_H__

