#include <EventChannel.h>

class EventRuleEngine;

class StreamEventSinkImpl : public ::TianShanIce::Streamer::StreamEventSink
{
public:
	StreamEventSinkImpl(EventRuleEngine& ruleEngine);
	virtual ~StreamEventSinkImpl();

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle<StreamEventSinkImpl> Ptr;

	virtual void OnEndOfStream(const ::std::string&, const ::std::string&, const ::TianShanIce::Properties& , const ::Ice::Current& = ::Ice::Current()) const;
	virtual void OnBeginningOfStream(const ::std::string&, const ::std::string&, const ::TianShanIce::Properties& , const ::Ice::Current& = ::Ice::Current()) const;
	virtual void OnExit(const ::std::string&, const ::std::string&, ::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;

	virtual void OnSpeedChanged(const ::std::string&, const ::std::string&, ::Ice::Float, ::Ice::Float, const ::TianShanIce::Properties& , const ::Ice::Current& = ::Ice::Current()) const;

	virtual void OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const ::TianShanIce::Properties& , const ::Ice::Current& = ::Ice::Current()) const;

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());

	virtual void OnExit2(const ::std::string&, const ::std::string&, ::Ice::Int, const ::std::string&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;

protected:
	EventRuleEngine& _ruleEngine;
};

class PlaylistEventSinkImpl : public ::TianShanIce::Streamer::PlaylistEventSink  
{
public:
	PlaylistEventSinkImpl(EventRuleEngine& ruleEngine);
	virtual ~PlaylistEventSinkImpl();

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle<PlaylistEventSinkImpl> Ptr;

	virtual void OnItemStepped(const ::std::string&, const ::std::string&, ::Ice::Int,::Ice::Int , const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());
protected:
	EventRuleEngine& _ruleEngine;
};

class GenericEventSinkImpl : public ::TianShanIce::Events::GenericEventSink  
{
public:
	GenericEventSinkImpl(EventRuleEngine& ruleEngine);
	virtual ~GenericEventSinkImpl();

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle<GenericEventSinkImpl> Ptr;

	virtual void post(
		const ::std::string& category,
		::Ice::Int eventId,
		const ::std::string& eventName,
		const ::std::string& stampUTC,
		const ::std::string& sourceNetId,
		const ::TianShanIce::Properties& params,
		const ::Ice::Current& /* = ::Ice::Current */
		);

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());
protected:
	EventRuleEngine& _ruleEngine;
	static int64 allcount;
};

class StreamProgressSinkImpl : public ::TianShanIce::Streamer::StreamProgressSink
{
public:
	StreamProgressSinkImpl(EventRuleEngine& ruleEngine);
	virtual ~StreamProgressSinkImpl();

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle<StreamProgressSinkImpl> Ptr;

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());

	virtual void OnProgress(const ::std::string&, const ::std::string&, ::Ice::Int, ::Ice::Int, ::Ice::Int, ::Ice::Int, const ::std::string&,const TianShanIce::Properties &, const ::Ice::Current& = ::Ice::Current()) const;

protected:
	EventRuleEngine& _ruleEngine;
};

class SessionEventSinkImpl : public ::TianShanIce::SRM::SessionEventSink
{
public:
	SessionEventSinkImpl(EventRuleEngine& ruleEngine);
	virtual ~SessionEventSinkImpl();

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle<SessionEventSinkImpl> Ptr;

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());

	virtual void OnNewSession(const ::std::string&, const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual void OnDestroySession(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual void OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::State, ::TianShanIce::State, const ::Ice::Current& = ::Ice::Current());

protected:
	EventRuleEngine& _ruleEngine;
};

