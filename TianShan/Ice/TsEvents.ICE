// ===========================================================================
// Copyright (c) 2006 by
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
//
// Ident : $Id: TsEvents.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsEvents.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 8     10-10-21 14:10 Hongquan.zhang
// 
// 7     08-08-14 14:27 Hui.shao
// merge from 1.7.10
// 
// 7     08-08-11 18:45 Hui.shao
// 
// 6     07-06-14 11:37 Hui.shao
// added param stampUTC into GenericEvent
// 
// 5     07-05-23 13:26 Hui.shao
// added GenericEvent
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_Events_ICE__
#define __ZQ_TianShanIce_Events_ICE__

#include <TianShanIce.ICE>

module TianShanIce
{

// -----------------------------
// namespace Events
// -----------------------------
/// Events represents a media content storage within the TianShan architecture
module Events
{

// -----------------------------
// interface BaseEventSink
// -----------------------------
///@brief the base event sink object to receive events and notification
interface BaseEventSink
{
	/// a dumy ping call, you may do nothing in its body.
	void ping(long timestamp);
};

// -----------------------------
// interface GenericEventSink
// -----------------------------
/// topic of service state change events
const string TopicOfGenericEvent = "TianShan/Event/Generic"; 
const string TopicStreamRepositionEvent = "TianShan/Event/Streamer/SessionInItemReposition";
const string TopicStreamPauseTimeoutEvent = "TianShan/Event/Streamer/SessionPauseTimeout";

interface GenericEventSink extends TianShanIce::Events::BaseEventSink
{
	/// a generic event post
	///@param category the category name of the event
	///@param eventId the 4-decimal numberic event id within the category
	///@param eventName the short string name of the event
	///@param stampUTC  the UTC formatted (ISO 8601) timestamp of the event, it will be in timezone GMT. 
	///                  i.e. 20070523T124311.345Z
	///@param sourceNetId the network id where the event is fired. hostname is recommended if no network id
	///                   can be specified in this category
	///@param params	the <key, value> collections of the event if appliable
	void post(string category, int eventId, string eventName, string stampUTC, string sourceNetId, Properties params);
};

// -----------------------------
// interface ServiceStateSink
// -----------------------------
/// topic of service state change events
const string TopicOfServiceState = "TianShan/Event/ServiceState"; 

///@brief the event sink object to receive the service state change events with
/// TopicOfServiceState
interface ServiceStateSink
{
	/// event will be fired when the state of a service has been changed
	///@param[in] serviceproxy  the proxy string to access the service
	///@param[in] newState		the new state that the service has been changed to
	///@param[in] prevState		the previous state that the service has been changed
	///                         from, may not provided
	void OnStateChanged(string serviceproxy, State newState, State prevState);
};

// -----------------------------
// local class EventChannel
// -----------------------------
/// reprent a local class as a subscriber to acess TianShan Event Channel
local class EventChannel
{

	/// sink TianShan events
	///@param[in] sink	  access to the Sink object
	///@param[in] topics  the topics to subscribe
	///@param[in] QoS  the properties of QoS
	///@return a collection of topics has been subscribed successfully
	StrValues protectedSink(BaseEventSink* sink, StrValues topics, Properties QoS);
};

};
};

#endif // __ZQ_TianShanIce_Events_ICE__
