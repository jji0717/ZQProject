// StreamEventSinkI.h: interface for the StreamEventSinkI class.

#ifndef __EVENT_IS_VODI5_STREAM_EVENT_SINK_IMPL_H__
#define __EVENT_IS_VODI5_STREAM_EVENT_SINK_IMPL_H__

#include "Log.h"

#include "SsmOpenVBO.h"
#include <TianShanIceHelper.h>
#include <IceUtil/IceUtil.h>
#include <NativeThreadPool.h>

/// SRM announce code 
// end of stream  2101
// begin of stream 1100
// state change 1200
// speed change 1300
// Session in progress 2200
// terminate session 2300
#define OPENVBO_ANNOUNCE_ENDOFSTREAM							"2101"
#define OPENVBO_ANNOUNCE_ENDOFSTREAM_STRING						"End-of-Stream Reached"

#define OPENVBO_ANNOUNCE_BEGINOFSTREAM						    "2104"
#define OPENVBO_ANNOUNCE_BEGINOFSTREAM_STRING					"Start-of-Stream Reached"

#define OPENVBO_ANNOUNCE_SCALE_CHANGE						"8801"
#define OPENVBO_ANNOUNCE_SCALE_CHANGE_STRING				"Scale Changed"

#define OPENVBO_ANNOUNCE_STATE_CHANGE						"8802"
#define OPENVBO_ANNOUNCE_STATE_CHANGE_STRING				"State Changed"

#define OPENVBO_ANNOUNCE_SESSION_IN_PROCESS					"5700"
#define OPENVBO_ANNOUNCE_SESSION_IN_PROCESS_STRING			"Session In Progress"

#define OPENVBO_ANNOUNCE_TERMINATE_SESSION						"5402"
#define OPENVBO_ANNOUNCE_TERMINATE_SESSION_STRING				"Client Session Terminated"

#define OPENVBO_ANNOUNCE_ITEMSTEPPED							"2103"
#define OPENVBO_ANNOUNCE_ITEMSTEPPED_STRING						"Item Stepped"

#define OPENVBO_ANNOUNCE_ERROR_READING_CONTENT					"4400"
#define OPENVBO_ANNOUNCE_ERROR_READING_CONTENT_STRING			"Error Reading Content Data"

#define OPENVBO_ANNOUNCE_DOWNSTREAM_FAILURE						"5401"
#define OPENVBO_ANNOUNCE_DOWNSTREAM_FAILURE_STRING				"Downstream Failure"

#define OPENVBO_ANNOUNCE_INTERNAL_SERVER_ERROR					"5502"
#define OPENVBO_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING			"Internal Server Error"

#define OPENVBO_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT				"5602"
#define OPENVBO_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT_STRING		"Bandwidth Exceeded Limit"

#define OPENVBO_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE			"5200" 
#define OPENVBO_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING	"Server Resources Unavailable"

#define OPENVBO_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE			"6001" 
#define OPENVBO_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE_STRING	"Stream Bandwidth Exceeds That Available"

#define OPENVBO_ANNOUNCE_DOWNSTREAM_UNREACHABLE					"6004"
#define OPENVBO_ANNOUNCE_DOWNSTREAM_UNREACHABLE_STRING			"Downstream Destination Unreachable"

#define OPENVBO_ANNOUNCE_UNABLE_ENCRPT							"6005" 
#define OPENVBO_ANNOUNCE_UNABLE_ENCRPT_STRING					"Unable to Encrypt one or more Components"

#define OPENVBO_ANNOUNCE_PAUSE_TIMEOUT							"2105"
#define OPENVBO_ANNOUNCE_PAUSE_TIMEOUT_STRING					"Pause Timeout Reached"

/*
//////
//SRM
#define OPENVBO_ANNOUNCE_SRM_BEGINOFSTREAM						"1100"
#define OPENVBO_ANNOUNCE_SRM_BEGINOFSTREAM_STRING				"Start-of-Stream Reached"

#define OPENVBO_ANNOUNCE_SRM_STATE_CHANGE						"1200"
#define OPENVBO_ANNOUNCE_SRM_STATE_CHANGE_STRING				"State Changed"

#define OPENVBO_ANNOUNCE_SRM_SCALE_CHANGE						"1300"
#define OPENVBO_ANNOUNCE_SRM_SCALE_CHANGE_STRING				"Scale Changed"

#define OPENVBO_ANNOUNCE_SRM_SESSION_IN_PROCESS					"2200"
#define OPENVBO_ANNOUNCE_SRM_SESSION_IN_PROCESS_STRING			"Session In Progress"

#define OPENVBO_ANNOUNCE_SRM_TERMINATE_SESSION					"2300"
#define OPENVBO_ANNOUNCE_SRM_TERMINATE_SESSION_STRING			"Client Session Terminated"

//STB
#define OPENVBO_ANNOUNCE_STB_BEGINOFSTREAM						"2104"
#define OPENVBO_ANNOUNCE_STB_BEGINOFSTREAM_STRING				"Start-of-Stream Reached"

#define OPENVBO_ANNOUNCE_STB_STATE_CHANGE						"8802"
#define OPENVBO_ANNOUNCE_STB_STATE_CHANGE_STRING				"State Changed"

#define OPENVBO_ANNOUNCE_STB_SCALE_CHANGE						"8801"
#define OPENVBO_ANNOUNCE_STB_SCALE_CHANGE_STRING				"Scale Changed"

#define OPENVBO_ANNOUNCE_STB_SESSION_IN_PROCESS					"2200"
#define OPENVBO_ANNOUNCE_STB_SESSION_IN_PROCESS_STRING			"Session In Progress"

#define OPENVBO_ANNOUNCE_STB_TERMINATE_SESSION					"5402"
#define OPENVBO_ANNOUNCE_STB_TERMINATE_SESSION_STRING			"Client Session Terminated"

#define OPENVBO_ANNOUNCE_ITEMSTEPPED							"2103"
#define OPENVBO_ANNOUNCE_ITEMSTEPPED_STRING						"Item Stepped"

//error
#define OPENVBO_ANNOUNCE_ERROR_READING_CONTENT					"4400"
#define OPENVBO_ANNOUNCE_ERROR_READING_CONTENT_STRING			"Error Reading Content Data"

#define OPENVBO_ANNOUNCE_DOWNSTREAM_FAILURE						"5401"
#define OPENVBO_ANNOUNCE_DOWNSTREAM_FAILURE_STRING				"Downstream Failure"

#define OPENVBO_ANNOUNCE_INTERNAL_SERVER_ERROR					"5502"
#define OPENVBO_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING			"Internal Server Error"

#define OPENVBO_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT				"5602"
#define OPENVBO_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT_STRING		"Bandwidth Exceeded Limit"

#define OPENVBO_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE			"5200" 
#define OPENVBO_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING	"Server Resources Unavailable"

#define OPENVBO_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE			"6001" 
#define OPENVBO_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE_STRING	"Stream Bandwidth Exceeds That Available"

#define OPENVBO_ANNOUNCE_DOWNSTREAM_UNREACHABLE					"6004"
#define OPENVBO_ANNOUNCE_DOWNSTREAM_UNREACHABLE_STRING			"Downstream Destination Unreachable"

#define OPENVBO_ANNOUNCE_UNABLE_ENCRPT							"6005" 
#define OPENVBO_ANNOUNCE_UNABLE_ENCRPT_STRING					"Unable to Encrypt one or more Components"
*/

#ifdef ZQ_OS_MSWIN
#define LINETERM "\r\n"
#else
#define LINETERM "\n"
#endif

namespace EventISVODI5
{

class Environment;

enum OpenVBOStreamEventType
{
	ON_END_OF_STREAM,
	ON_BEGINGNING_OF_STREAM,
	ON_SPEED_CHANGED,
	ON_STATE_CHANGED,
	ON_ITEM_STEP,
	ON_EXIT,
	ON_EXIT2,
	ON_PAUSE_TIMEOUT,
};

class StreamEventDispatchRequest: public ZQ::common::ThreadRequest
{
public:
	StreamEventDispatchRequest(Environment& env, ZQ::common::NativeThreadPool& pool, const std::string& proxy, const std::string& uid, OpenVBOStreamEventType openVBOEventType, TianShanIce::Properties extendProps, ZQ::common::Log& fileLog);		
	virtual ~StreamEventDispatchRequest();

protected:
	int		run();		
	void	final(int retcode =0, bool bCancelled =false);
private:
	Environment&					_env;
	std::string						_proxy;
	std::string						_uid;
	OpenVBOStreamEventType			_openVBOEventType;
	TianShanIce::Properties			_props;
	ZQ::common::Log& _fileLog;
};

class StreamEventDispatcher : public Ice::Object
{
public:
	typedef IceUtil::Handle<StreamEventDispatcher> Ptr;
	StreamEventDispatcher(ZQ::common::Log& fileLog, Environment& env);
	virtual ~StreamEventDispatcher();
	void start();
	void stop( );	

	void pushEvent(const std::string& proxy, const std::string& uid, OpenVBOStreamEventType openVBOEventType, TianShanIce::Properties& extendProps);

private:

	ZQ::common::Log&				_fileLog;
	Environment&					_env;
	ZQ::common::NativeThreadPool	mPool;
};
typedef StreamEventDispatcher::Ptr StreamEventDispatcherPtr;

class EventSinkI 
{
public:
	EventSinkI( ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher );
	virtual ~EventSinkI();

protected:

	void sendANNOUNCE(const ::std::string& proxy, const ::std::string& uid, OpenVBOStreamEventType openVBOEventType, TianShanIce::Properties extendProps, const ::Ice::Current& ic) const;

protected:
	ZQ::common::Log& _fileLog;
	Environment& _env;
	StreamEventDispatcher& mEventDispatcher;
};

class StreamEventSinkI : public EventSinkI , public TianShanIce::Streamer::StreamEventSink  
{
public:
	typedef IceInternal::Handle<StreamEventSinkI> Ptr;

	StreamEventSinkI(ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher);

	virtual ~StreamEventSinkI();

	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());

	void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current()) const;

	void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props,const ::Ice::Current& ic= ::Ice::Current()) const;

	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props,const ::Ice::Current& ic = ::Ice::Current()) const;

	void OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const TianShanIce::Properties& props,const ::Ice::Current& = ::Ice::Current()) const;

	void OnExit(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;

	void OnExit2(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const TianShanIce::Properties&, const ::Ice::Current& ic = ::Ice::Current()) const;

	void sendANNOUNCE_SessionInProgress(SsmOpenVBO::CRGSession& sessionContext);

	void sendANNOUNCE_Terminated(SsmOpenVBO::CRGSession& sessionContext);

};

typedef StreamEventSinkI::Ptr StreamEventSinkIPtr;

class PlayListEventSinkI : public EventSinkI , public TianShanIce::Streamer::PlaylistEventSink
{
public:
	PlayListEventSinkI(ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher);

	virtual ~PlayListEventSinkI();

	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());

	virtual void OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, 
		::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, 
		const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic = ::Ice::Current()) const;
};

class PauseTimeoutSinkI : public EventSinkI , public ::TianShanIce::Events::GenericEventSink  
{
public:
	PauseTimeoutSinkI(ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher);

	virtual ~PauseTimeoutSinkI();

	virtual void post(
		const ::std::string& category,
		::Ice::Int eventId,
		const ::std::string& eventName,
		const ::std::string& stampUTC,
		const ::std::string& sourceNetId,
		const ::TianShanIce::Properties& params,
		const ::Ice::Current& /* = ::Ice::Current */
		);

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current()) {}
};

} // end StreamEventSinkI

#endif // end __EVENT_IS_VODI5_STREAM_EVENT_SINK_IMPL_H__
