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
// Ident : $Id: EventChannel.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : this is a sample implementation of EventChannel access for your
//         for your reference, you may enhances it for your own purposes
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/EventChannel.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 17    10-10-26 12:01 Hui.shao
// merged HQ's change as of 6/25/2010 from V1.10
// 
// 16    09-12-15 18:50 Junming.zheng
// modify clean() into public
// 
// 16    09-12-10 16:01 Junming.zheng
// modigy clean() into public
// 
// 15    08-12-09 17:12 Yixin.tian
// 
// 14    08-08-14 15:00 Hui.shao
// merged from 1.7.10
// 
// 14    08-07-17 15:15 Hui.shao
// removed content provision events
// 
// 13    08-03-20 11:58 Yixin.tian
// 
// 12    08-03-07 10:14 Yixin.tian
// WIN32 replaced by ZQ_OS_MSWIN
// 
// 11    08-03-05 14:17 Yixin.tian
// merge for linux
// 
// 10    07-12-14 15:38 Xiaohui.chai
// 
// 9     07-08-14 12:22 Hongquan.zhang
// 
// 8     07-03-13 17:11 Hongquan.zhang
// 
// 7     06-09-26 14:53 Hongquan.zhang
// 
// 6     9/21/06 6:47p Hui.shao
// ===========================================================================

#ifndef __TianShanIce_EventChannelImpl_H__
#define __TianShanIce_EventChannelImpl_H__

#include "TianShanDefines.h"
#include "NativeThread.h"
#include "TsEvents.h"
#include "TsStreamer.h"
#include "TsSRM.h"
#include "TsStorage.h"

#include <map>

#ifdef ZQ_OS_LINUX
extern "C" {
#include <semaphore.h>
#include <sys/timeb.h>
}
#endif

namespace TianShanIce {
namespace Events {

// -----------------------------
// local object EventChannelImpl
// -----------------------------
/// A sample implementation of EventChannel for your reference, you may enhances it
/// for your own purposes. This EventChannel are IceStorm service based
class EventChannelImpl : public EventChannel, public IceUtil::AbstractMutexI<IceUtil::Mutex>,public ZQ::common::NativeThread
{
public:

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle< EventChannelImpl> Ptr;
	
	/// constructor
	///@param[in] adapter the subscriber's adapter to receive events from IceStorm
	///@param[in] topicManagerEndpoint endpoint to connect the IceStorm topic manager of "TianShanEvents/TopicManager"
	///@param[in] createTopicIfNotExist true if any further topic hasn't existed in the topic manager
 	EventChannelImpl(Ice::ObjectAdapterPtr adapter, const char* topicManagerEndpoint = DEFAULT_ENDPOINT_TopicManager, bool createTopicIfNotExist=false);

	/// destructor
	~EventChannelImpl();

	/// sink the stream events defined in StreamEventSink
	///@param[in] sink pointer to the local StreamEventSink implementation
	///@param[in] qos IceStorm based QoS properties, if any field is not give, this method will fill in with the default QoS value
	///@return true if subscribe the event successfully
    virtual bool sink(::TianShanIce::Streamer::StreamEventSinkPtr& sink, const ::TianShanIce::Properties& qos);

	/// sink the stream progress defined in StreamProgressSink
	///@param[in] sink pointer to the local StreamProgressSink implementation
	///@return true if subscribe the event successfully
	///@note no QoS setting is allowed for this type of events
	virtual bool sink(::TianShanIce::Streamer::StreamProgressSinkPtr& sink);

	/// sink the playlist events defined in PlaylistEventSink
	///@param[in] sink pointer to the local PlaylistEventSink implementation
	///@param[in] qos IceStorm based QoS properties, if any field is not give, this method will fill in with the default QoS value
	///@return true if subscribe the event successfully
    virtual bool sink(::TianShanIce::Streamer::PlaylistEventSinkPtr& sink, const ::TianShanIce::Properties& qos);

	/// sink the session events defined in SessionEventSink
	///@param[in] sink pointer to the local SessionEventSink implementation
	///@param[in] qos IceStorm based QoS properties, if any field is not give, this method will fill in with the default QoS value
	///@return true if subscribe the event successfully
	virtual bool sink(::TianShanIce::SRM::SessionEventSinkPtr& sink, const ::TianShanIce::Properties& qos);

/*
	/// sink the ContentStore Provision StateChange events defined in SessionEventSink
	///@param[in] sink pointer to the local ProvisionStateChangeSinkPtr implementation
	///@param[in] qos IceStorm based QoS properties, if any field is not give, this method will fill in with the default QoS value
	///@return true if subscribe the event successfully
	virtual bool sink(::TianShanIce::Storage::ProvisionStateChangeSinkPtr& sink, const ::TianShanIce::Properties& qos);

	/// sink the ContentStore Provision Progress events defined in SessionEventSink
	///@param[in] sink pointer to the local ProvisionProgressSinkPtr implementation
	///@param[in] qos IceStorm based QoS properties, if any field is not give, this method will fill in with the default QoS value
	///@return true if subscribe the event successfully
	virtual bool sink(::TianShanIce::Storage::ProvisionProgressSinkPtr& sink, const ::TianShanIce::Properties& qos);
*/
	///Sink the generic events
	///@param[in] sink the local GenericEventSinkPtr implementation
	///@param[in] qos ice storm based qos properties,if any field is not give,this method will fill in with the default value
	///@return true if subscribe the event successfully
	virtual bool sink(::TianShanIce::Events::GenericEventSinkPtr& sink,const TianShanIce::Properties& qos, const std::string& topicName = TianShanIce::Events::TopicOfGenericEvent);


	/// native scan thread	
	int		run();

	///notify bad network connection
	void	NotifyBadConnection();

	virtual void clean();

protected:

	virtual ::TianShanIce::StrValues protectedSinkEx(const ::Ice::ObjectPrx& sink, const ::TianShanIce::StrValues& topics, const ::TianShanIce::Properties& qos);

	// implementation of TianShanIce::Events::EventChannel. General sink the events in EventChannel
	///@param[in] sink proxy to a BaseEventSinkPrx implementation
	///@param[in] topics a collection of topic names
	///@param[in] qos IceStorm based QoS properties, if any field is not give, this method will fill in with the default QoS value
	///@return true a collection of the topics that have been subscribed successfully
	virtual ::TianShanIce::StrValues protectedSink(const ::TianShanIce::Events::BaseEventSinkPrx& sink, const ::TianShanIce::StrValues& topics, const ::TianShanIce::Properties& qos);

	///refresh subscribe to server
	void	RefreshSubscribe();	
protected:

	::Ice::ObjectAdapterPtr	     _adapter;
	::IceStorm::TopicManagerPrx	 _topicManager;

	bool						 _bCreateTopicIfNotExist;

	typedef struct _tagProperty 
	{
		::Ice::ObjectPrx	prx;
		::TianShanIce::Properties qos;
	}SubScribeProper;
    //typedef std::map<std::string, ::Ice::ObjectPrx> Map;
	typedef std::map<std::string, SubScribeProper> Map;
	Map							_subscribers;

	bool						_bQuit;
	bool						_isNetWorkOK;
#ifdef ZQ_OS_MSWIN
	HANDLE						_hNotifyBadConn;
#else
	sem_t						_semNotifyBadConn;
#endif

};

}} // namespace

#endif // __TianShanIce_Subscriber_H__
