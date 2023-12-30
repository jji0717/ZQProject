// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.

// Branch: $Name:PlaylistEventSinkImpl.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/PlaylistEventSinkImpl.cpp $
// 
// 4     10/17/14 3:37p Li.huang
// 
// 3     6/03/14 2:31p Li.huang
// 
// 2     5/30/14 4:43p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     10-04-22 17:53 Li.huang
// modify due to interface changed
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
// PlaylistEventSinkImpl.cpp: implementation of the PlaylistEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////
#include "BroadCastChannelEnv.h"
#include "PlaylistEventSinkImpl.h"
#include "TianShanIce.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace ZQBroadCastChannel {

StreamEventSinkImpl::StreamEventSinkImpl(BroadCastChannelEnv& env)
	:_env(env)
{

}

StreamEventSinkImpl::~StreamEventSinkImpl()
{

}

void StreamEventSinkImpl::ping(::Ice::Long, const ::Ice::Current& c)
{

}
void StreamEventSinkImpl::OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "PID(%s) OnEndOfStream"), playlistId.c_str());	

	_env.OnEndOfStream(playlistId);
}

void StreamEventSinkImpl::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "PID(%s) OnBeginningOfStream"), playlistId.c_str());	
}

void StreamEventSinkImpl::OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "PID(%s) OnExit with code %d, reason %s"), 
		playlistId.c_str(), exitCode, reason.c_str());	

	_env.OnStreamExit(playlistId);
}

void StreamEventSinkImpl::OnSpeedChanged(const ::std::string&, const ::std::string&, ::Ice::Float, ::Ice::Float, const TianShanIce::Properties& props, const ::Ice::Current&) const
{

}

void StreamEventSinkImpl::OnStateChanged(const ::std::string&, const ::std::string&, TianShanIce::Streamer::StreamState, TianShanIce::Streamer::StreamState, const TianShanIce::Properties& props, const ::Ice::Current&) const
{

}
 
void StreamEventSinkImpl::OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int exitCode, const ::std::string& reason, const TianShanIce::Properties& props, const ::Ice::Current& c) const
{

}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//

PlaylistEventSinkImpl::PlaylistEventSinkImpl(BroadCastChannelEnv& env)
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

void PlaylistEventSinkImpl::OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int curUserCtrlNum,::Ice::Int prevUserCtrlNum, const TianShanIce::Properties& ItemProps, const ::Ice::Current&) const
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "PID(%s) OnEndOfItem, CtrlNum %d"), playlistId.c_str(), prevUserCtrlNum);	

	_env.OnEndOfItem(playlistId, prevUserCtrlNum);
}

}