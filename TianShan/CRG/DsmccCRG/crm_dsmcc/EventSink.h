#ifndef __crm_dsmcc_eventsink_header_file_h__
#define __crm_dsmcc_eventsink_header_file_h__
#include <TsStreamer.h>

namespace  ZQ {	namespace CLIENTREQUEST {

// -----------------------------
// class EventSink
// -----------------------------
class EventSink : public TianShanIce::Streamer::StreamEventSink  
{
public:
	EventSink(DSMCC_Environment& env) : _env(env) {}
	virtual ~EventSink() {}

public: // impl of StreamEventSink
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current()){}
	void OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props ,const ::Ice::Current& ic= ::Ice::Current()) const;
	void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props ,  const ::Ice::Current& ic= ::Ice::Current()) const;

	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props , const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnStateChanged(const ::std::string& proxy, const ::std::string& playlistId, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties& props ,  const ::Ice::Current& ic= ::Ice::Current()) const ;

	void OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::TianShanIce::Properties& props, const ::Ice::Current& = ::Ice::Current()) const;

	void issueLscDone(const std::string& streamId, const TianShanIce::Properties& streamParams, bool bEOS) const;
	void HeNanLscDone(const std::string& streamId, const TianShanIce::Properties& streamParams, const std::string& tag ) const;

	void issueCSRI(TianShanIce::ClientRequest::SessionPrx sess, int reasonCode);

protected: 
	DSMCC_Environment& _env;

}; // class EventSink

typedef IceUtil::Handle<EventSink> EventSinkPtr;

class PlaylistEventSinkI : public  TianShanIce::Streamer::PlaylistEventSink
{
public:
	PlaylistEventSinkI(DSMCC_Environment& env) : _env(env) {}
	virtual ~PlaylistEventSinkI() {};

	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& c);

	void OnItemStepped(const ::std::string& proxy, const ::std::string& id, ::Ice::Int currentUserCtrlNum,
		::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& c) const;
	/// safe pointer to this type of objects
protected: 
	DSMCC_Environment& _env;
};
typedef ::IceInternal::Handle<PlaylistEventSinkI> PlaylistEventSinkIPtr;

class PauseTimeoutEventSinkI : public ::TianShanIce::Events::GenericEventSink  
{
public:
	PauseTimeoutEventSinkI(DSMCC_Environment& env): _env(env) {};
	virtual ~PauseTimeoutEventSinkI(){};

	virtual void post(
		const ::std::string& category,
		::Ice::Int eventId,
		const ::std::string& eventName,
		const ::std::string& stampUTC,
		const ::std::string& sourceNetId,
		const ::TianShanIce::Properties& params,
		const ::Ice::Current& /* = ::Ice::Current */
		);

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current()){}
protected: 
	DSMCC_Environment& _env;
};
typedef ::IceInternal::Handle<PauseTimeoutEventSinkI> PauseTimeoutEventSinkIPtr;

}}//namespace ZQ::CLIENTREQUEST

#endif//__crm_dsmcc_eventsink_header_file_h__
