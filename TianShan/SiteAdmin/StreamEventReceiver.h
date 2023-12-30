
#ifndef _SITEADMIN_STREAMEVENT_RECEIVER_H__
#define _SITEADMIN_STREAMEVENT_RECEIVER_H__

#include "ZQ_common_conf.h"

#include "Locks.h"
#include <map>
#include <string>

#include "Ice/Ice.h"
#include "IceUtil/IceUtil.h"
#include "IceStorm/IceStorm.h"
#include "EventChannel.h"
#include "TsStreamer.h"

#include "EventSenderManager.h"

namespace ZQTianShan {
namespace Site {


class StreamEventSinkI:public TianShanIce::Streamer::StreamEventSink
{
public:
	StreamEventSinkI(EventSenderManager& ssMan);
	void ping(::Ice::Long iL, const ::Ice::Current& ic= ::Ice::Current());

	void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current())const;
    	
    void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current())const;
    
    void OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current())const;
    
	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current())const;
	
	void OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& = ::Ice::Current()) const ;

    virtual void OnExit2(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::TianShanIce::Properties& props, const ::Ice::Current&) const;
private:
	EventSenderManager&	mEventManager;
};

class PlaylistEventSinkI : public TianShanIce::Streamer::PlaylistEventSink
{
public:
	PlaylistEventSinkI(EventSenderManager& ssMan);
	void ping(::Ice::Long iL, const ::Ice::Current& ic= ::Ice::Current())
	{
		
	}
	virtual void OnItemStepped(const ::std::string& proxy, const ::std::string& uid,
									::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, 
									const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& = ::Ice::Current()) const;


private:
	EventSenderManager&	mEventManager;
};
}}//namespace ZQTianShan::Site

#endif//_SITEADMIN_STREAMEVENT_RECEIVER_H__
