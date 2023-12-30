// PlaylistEventSinkImpl.cpp: implementation of the PlaylistEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlaylistEventSinkImpl.h"
#include "TianShanIce.h"
#include "Log.h"
#include "ChODSvcEnv.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


#define LOG_MODULE_NAME			"StreamEvent"


namespace ZQChannelOnDemand {

StreamEventSinkImpl::StreamEventSinkImpl(ChODSvcEnv& env)
	:_env(env)
{

}

StreamEventSinkImpl::~StreamEventSinkImpl()
{

}

void StreamEventSinkImpl::ping(::Ice::Long, const ::Ice::Current& c)
{

}

void StreamEventSinkImpl::OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const ::TianShanIce::Properties&, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PID(%s) OnEndOfStream"), playlistId.c_str());	

	_env.OnEndOfStream(playlistId);
}

void StreamEventSinkImpl::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const ::TianShanIce::Properties&, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PID(%s) OnBeginningOfStream"), playlistId.c_str());	
}

void StreamEventSinkImpl::OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PID(%s) OnExit with code %d, reason %s"), 
		playlistId.c_str(), exitCode, reason.c_str());	

	_env.OnStreamExit(playlistId);
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
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//

PlaylistEventSinkImpl::PlaylistEventSinkImpl(ChODSvcEnv& env)
	: _env(env)
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

	_env.OnEndOfItem(playlistId, prevUserCtrlNum);
}

}