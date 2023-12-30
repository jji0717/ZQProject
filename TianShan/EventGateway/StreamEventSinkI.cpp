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

// Branch: $Name:StreamEventSinkI.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EventGateway/StreamEventSinkI.cpp $
// 
// 3     9/25/14 3:29p Li.huang
// 
// 2     9/25/14 11:38a Li.huang
// 
// 1     9/25/14 11:30a Li.huang
// add stream and playlist event
// 
// ===========================================================================
// StreamEventSinkI.cpp: implementation of the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////
#include "StreamEventSinkI.h"
#include "TianShanIce.h"
#include "Log.h"
#include "ZQ_common_conf.h"
#include "TimeUtil.h"
namespace EventGateway
{

#define SOURCE_NET				"testNetId"  
#define SOURCE_NETID_KEY		"sourceNetId"
#define TIMESTAMP_KEY			"timeStamp"

#define PlayList_Key            "playlistId"
#define Proxy_Key               "proxy"

#define CurrentState_Key        "currentState"
#define PrevState_Key           "prevState"

#define CurrentSpeed_Key        "currentSpeed"
#define PrevSpeed_Key           "prevSpeed"

#define CurrentUserCtrlNum_Key  "currentUserCtrlNum"
#define PreUserCtrlNum_Key      "preUserCtrlNum"

	std::string getPropertiesString(const TianShanIce::Properties& props)
	{
		std::string strProps;
		for(TianShanIce::Properties::const_iterator itor = props.begin(); itor != props.end(); itor++)
		{
			strProps = itor->first + std::string(":") + itor->second + std::string(";");
		}
		return strProps;
	}
	std::string  streamStateToString(TianShanIce::Streamer::StreamState state)
	{
		std::string strState;
		switch(state)
		{
		case TianShanIce::Streamer::stsSetup:
			strState = "Setup";
			break;
		case TianShanIce::Streamer::stsStreaming:
			strState = "Streaming";
			break;
		case TianShanIce::Streamer::stsPause:
			strState = "Pause";
			break;
		case TianShanIce::Streamer::stsStop:
			strState = "Stop";
			break;
		default:
			strState = "Unknown";
		}
		return strState;
	}
	//////////////////////////////////////////////////////////////////////
	// Class BaseEventHelp
	//////////////////////////////////////////////////////////////////////
	BaseEventHelp::BaseEventHelp(ZQ::common::Log& log): _log(log), _thrHelpersOnEvent(log)
	{
		_thrHelpersOnEvent.start();
	}
	BaseEventHelp::~BaseEventHelp()
	{
		_thrHelpersOnEvent.stop();
	}
	void BaseEventHelp::postStreamEvent(
		const ::std::string& category,
		::Ice::Int eventId,
		const ::std::string& eventName,
		const ::std::string& stampUTC,
		const ::std::string& sourceNetId,
		const Properties& params) const
	{
		ZQ::common::MutexGuard sync(_lockHelpers);
		GenHelpersOnEvent::HelpersCtx ctx(category, eventId, eventName, stampUTC, sourceNetId, params, Ice::Current());

		BaseEventHelp*  pBase=const_cast< BaseEventHelp* > (this); 
		pBase->_thrHelpersOnEvent.execOnEvent(ctx, pBase->_helpers);

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "post(): category[%s], eventId[%d], eventName[%s], stampUTC[%s], sourceNetId[%s]")
			, category.c_str(), eventId, eventName.c_str(), stampUTC.c_str(), sourceNetId.c_str());
	}
	void BaseEventHelp::add(IGenericEventHelper *pHelper)
	{
		if(NULL == pHelper)
			return;

		ZQ::common::MutexGuard sync(_lockHelpers);
		if(_helpers.insert(pHelper).second)
		{
			_log(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkImpl, "added helper [%p]"), pHelper);
		}
		else
		{
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkImpl, "requested to add duplicate helper [%p]"), pHelper);
		}
	}

	void BaseEventHelp::remove(IGenericEventHelper *pHelper)
	{
		if(NULL == pHelper)
			return;
		ZQ::common::MutexGuard sync(_lockHelpers);
		// search in the helper collection
		if(1 == _helpers.erase(pHelper))
		{
			_log(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkImpl, "removed helper [%p]"), pHelper);
		}
		else
		{
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkImpl, "requested to remove unkown helper [%p]"), pHelper);
		}
	}

	size_t BaseEventHelp::count()
	{
		ZQ::common::MutexGuard sync(_lockHelpers);
		return _helpers.size();
	}

	//////////////////////////////////////////////////////////////////////
	// Class StreamEventSinkImpl
	//////////////////////////////////////////////////////////////////////
	StreamEventSinkImpl::StreamEventSinkImpl(ZQ::common::Log&log): _log(log), BaseEventHelp(log)
	{

	}
	StreamEventSinkImpl::~StreamEventSinkImpl()
	{

	}
	void StreamEventSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current& c)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "ping(): timestamp=%lld"), timestamp);
	}
	void StreamEventSinkImpl::OnEndOfStream(const ::std::string& proxy, const ::std::string& id, const TianShanIce::Properties& props,
											const ::Ice::Current& c) const
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "OnEndOfStream() proxy[%s] playlistId[%s] props[%s]"),proxy.c_str(),id.c_str(), getPropertiesString(props).c_str());	
		std::string category = "Stream";
		Ice::Int eventId = 2;
		std::string eventName = "OnEndOfStream";

		std::string stampUTC;
		TianShanIce::Properties::const_iterator itor = props.find(TIMESTAMP_KEY);
		if(itor == props.end())
		{
			char buf[65]= "";
			ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(),buf, sizeof(buf) -1);
			stampUTC  = buf;
		}
		else
			stampUTC = itor->second;

		std::string sourceNetId;
		itor = props.find(SOURCE_NETID_KEY);
		if(itor == props.end())
			sourceNetId = "TestNetId";
		else
			sourceNetId = itor->second;

		TianShanIce::Properties newProps = props;
		newProps[PlayList_Key] = id;
		newProps[Proxy_Key] = proxy;

		postStreamEvent(category, eventId, eventName, stampUTC, sourceNetId, newProps);
	}

	void StreamEventSinkImpl::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& id, 
												const TianShanIce::Properties& props, const ::Ice::Current& c) const
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "OnBeginningOfStream() proxy[%s] playlistId[%s] props[%s]"),proxy.c_str(),id.c_str(), getPropertiesString(props).c_str());	
		std::string category = "Stream";
		Ice::Int eventId = 1;
		std::string eventName = "OnBeginningOfStream";

		std::string stampUTC;
		TianShanIce::Properties::const_iterator itor = props.find(TIMESTAMP_KEY);
		if(itor == props.end())
		{
			char buf[65]= "";
			ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(),buf, sizeof(buf) -1);
			stampUTC  = buf;
		}
		else
			stampUTC = itor->second;

		std::string sourceNetId;
		itor = props.find(SOURCE_NETID_KEY);
		if(itor == props.end())
			sourceNetId = "TestNetId";
		else
			sourceNetId = itor->second;

		TianShanIce::Properties newProps = props;
		newProps[PlayList_Key] = id;
		newProps[Proxy_Key] = proxy;

		postStreamEvent(category, eventId, eventName, stampUTC, sourceNetId, newProps);	
	}

	void StreamEventSinkImpl::OnExit(const ::std::string& proxy, const ::std::string& id, 
									::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& c) const
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "PID(%s) OnExit with code %d, reason %s"), 
			id.c_str(), exitCode, reason.c_str());	

	}

	void StreamEventSinkImpl::OnSpeedChanged(const ::std::string& proxy, const ::std::string& id, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, 
											const ::TianShanIce::Properties& props, const ::Ice::Current& c)const
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "OnSpeedChanged() proxy[%s] playlistId[%s] prevSpeed[%f] currentSpeed[%f] props[%s]"),
			proxy.c_str(),id.c_str(), prevSpeed, currentSpeed, getPropertiesString(props).c_str());	
		std::string category = "Stream";
		Ice::Int eventId = 3;
		std::string eventName = "OnSpeedChanged";

		std::string stampUTC;
		TianShanIce::Properties::const_iterator itor = props.find(TIMESTAMP_KEY);
		if(itor == props.end())
		{
			char buf[65]= "";
			ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(),buf, sizeof(buf) -1);
			stampUTC  = buf;
		}
		else
			stampUTC = itor->second;

		std::string sourceNetId;
		itor = props.find(SOURCE_NETID_KEY);
		if(itor == props.end())
			sourceNetId = "TestNetId";
		else
			sourceNetId = itor->second;

		TianShanIce::Properties newProps = props;

		char buf[32];
		memset(buf, 0, sizeof(buf));
		sscanf(buf,  "%.2f", &currentSpeed);
		newProps[CurrentSpeed_Key] = std::string(buf);

		memset(buf, 0, sizeof(buf));
		sscanf(buf,  "%.2f", &prevSpeed);
		newProps[PrevSpeed_Key] = std::string(buf);

		newProps[PlayList_Key] = id;
		newProps[Proxy_Key] = proxy;

		postStreamEvent(category, eventId, eventName, stampUTC, sourceNetId, newProps);	
	}

    void StreamEventSinkImpl::OnStateChanged(const ::std::string& proxy, const ::std::string& id, ::TianShanIce::Streamer::StreamState prevState, 
											::TianShanIce::Streamer::StreamState currentState, const ::TianShanIce::Properties& props, 
											const ::Ice::Current& c)const
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkImpl, "OnStateChanged() proxy[%s] playlistId[%s] prevState[%d] currentState[%d] props[%s]"),
			proxy.c_str(),id.c_str(), prevState, currentState, getPropertiesString(props).c_str());	
		std::string category = "Stream";
		Ice::Int eventId = 4;
		std::string eventName = "OnStateChanged";

		std::string stampUTC;
		TianShanIce::Properties::const_iterator itor = props.find(TIMESTAMP_KEY);
		if(itor == props.end())
		{
			char buf[65]= "";
			ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(),buf, sizeof(buf) -1);
			stampUTC  = buf;
		}
		else
			stampUTC = itor->second;

		std::string sourceNetId;
		itor = props.find(SOURCE_NETID_KEY);
		if(itor == props.end())
			sourceNetId = "TestNetId";
		else
			sourceNetId = itor->second;

		TianShanIce::Properties newProps = props;
		
		newProps[CurrentState_Key] = streamStateToString(currentState);
		newProps[PrevState_Key] = streamStateToString(prevState);
		newProps[PlayList_Key] = id;
		newProps[Proxy_Key] = proxy;
		postStreamEvent(category, eventId, eventName, stampUTC, sourceNetId, newProps);	

	}

	void StreamEventSinkImpl::OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int exitCode, 
									const ::std::string& reason, const TianShanIce::Properties& props, const ::Ice::Current& c)const
	{

	}

	//////////////////////////////////////////////////////////////////////
	// Class PlaylistEventSinkImpl
	//////////////////////////////////////////////////////////////////////
	PlaylistEventSinkImpl::PlaylistEventSinkImpl(ZQ::common::Log&log): _log(log), BaseEventHelp(log)
	{

	}
	PlaylistEventSinkImpl::~PlaylistEventSinkImpl()
	{

	}
	void PlaylistEventSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current& c)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkImpl, "ping(): timestamp=%lld"), timestamp);
	}

	void PlaylistEventSinkImpl::OnItemStepped(const ::std::string& proxy, const ::std::string& id, ::Ice::Int currentUserCtrlNum,
		::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& c) const
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkImpl, "OnItemStepped() proxy[%s] playlistId[%s] CurrentUserCtrlNum[%d] prevUserCtrlNum[%d]props[%s]"),
			proxy.c_str(), id.c_str(), currentUserCtrlNum, prevUserCtrlNum, getPropertiesString(ItemProps).c_str());	

		std::string category = "PlayList";
		Ice::Int eventId = 5;
		std::string eventName = "OnItemStepped";

		std::string stampUTC;
		TianShanIce::Properties::const_iterator itor = ItemProps.find(TIMESTAMP_KEY);
		if(itor == ItemProps.end())
		{
			char buf[65]= "";
			ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(),buf, sizeof(buf) -1);
			stampUTC  = buf;
		}
		else
			stampUTC = itor->second;

		std::string sourceNetId;
		itor = ItemProps.find(SOURCE_NETID_KEY);
		if(itor == ItemProps.end())
			sourceNetId = "TestNetId";
		else
			sourceNetId = itor->second;

        
		TianShanIce::Properties newProps = ItemProps;
		char buf[32];
		memset(buf, 0, sizeof(buf));
		itoa(currentUserCtrlNum, buf , 10);
		newProps[CurrentUserCtrlNum_Key] = buf;

		memset(buf, 0, sizeof(buf));
		itoa(currentUserCtrlNum, buf , 10);
        newProps[PreUserCtrlNum_Key] = buf;

		newProps[PlayList_Key] = id;
		newProps[Proxy_Key] = proxy;

		postStreamEvent(category, eventId, eventName, stampUTC, sourceNetId, newProps);	
	}

}//end of EventGateway 