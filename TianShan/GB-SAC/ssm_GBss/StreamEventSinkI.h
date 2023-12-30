// StreamEventSinkI.h: interface for the StreamEventSinkI class.

#ifndef __EVENT_IS_VODI5_STREAM_EVENT_SINK_IMPL_H__
#define __EVENT_IS_VODI5_STREAM_EVENT_SINK_IMPL_H__

#include "Log.h"

#include "SsmGBss.h"
#include "CRGSessionImpl.h"

/// SRM announce code 
// end of stream  2101
// begin of stream 1100
// state change 1200
// speed change 1300
// Session in progress 2200
// terminate session 2300

namespace GBss
{

class Environment;

enum GBssStreamEventType
{
	ON_END_OF_STREAM,
	ON_BEGINGNING_OF_STREAM,
	ON_SPEED_CHANGED,
	ON_STATE_CHANGED,
	ON_EXIT,
	ON_EXIT2
};

class StreamEventSinkI : public TianShanIce::Streamer::StreamEventSink  
{
public:
	typedef IceInternal::Handle<StreamEventSinkI> Ptr;

	StreamEventSinkI(ZQ::common::Log& fileLog, Environment& env);

	virtual ~StreamEventSinkI();

	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());

	void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current()) const;

	void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props,const ::Ice::Current& ic= ::Ice::Current()) const;

	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props,const ::Ice::Current& ic = ::Ice::Current()) const;

	void OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const TianShanIce::Properties& props,const ::Ice::Current& = ::Ice::Current()) const ;

	void OnExit(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;

	void OnExit2(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const TianShanIce::Properties&, const ::Ice::Current& ic = ::Ice::Current()) const;

	void sessionInProgressAnnounce(GBss::CRGSessionImpl& sessionContext);

	void terminatedAnnounce(GBss::CRGSessionImpl& sessionContext);

private:

	inline std::string generatorNoticeString(const std::string& strNoticeCode, 
		const std::string& strNoticeString, const std::string& strEventDate) const;

	void sendANNOUNCE(const ::std::string& proxy, const ::std::string& uid, 
		GBssStreamEventType GBssEventType, TianShanIce::Properties& extendProps, const ::Ice::Current& ic) const;

private:
	ZQ::common::Log& _fileLog;
	Environment& _env;
};

typedef StreamEventSinkI::Ptr StreamEventSinkIPtr;

class PlayListEventSinkI : public TianShanIce::Streamer::PlaylistEventSink
{
public:
	PlayListEventSinkI(ZQ::common::Log& fileLog, Environment& env);

	virtual ~PlayListEventSinkI();

	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());

	virtual void OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, 
		::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, 
		const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic = ::Ice::Current()) const;
private:
	Environment& _env;
	ZQ::common::Log& _fileLog;
};

} // end StreamEventSinkI

#endif // end __EVENT_IS_VODI5_STREAM_EVENT_SINK_IMPL_H__
