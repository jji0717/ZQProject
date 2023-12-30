#include "./PlaylistEvent.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
{
	
	PlaylistEvent::PlaylistEvent(Environment& env, StreamEventDispatcher& eventDispatcher)
		:EventSinkI(env, eventDispatcher)
	{
	}

	PlaylistEvent::~PlaylistEvent()
	{
	}

	void PlaylistEvent::ping(::Ice::Long lv, const ::Ice::Current& ic)
	{
	}

	void PlaylistEvent::OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, 
		::Ice::Int userCtrlNum,::Ice::Int prevCtrlNum, 
		const ::TianShanIce::Properties& prty,
		const ::Ice::Current& ic) const
	{
		sendEvent(proxy, playlistId, streamEventITEMSTEP, prty, ic);
	}

} // namespace TianShanS1

