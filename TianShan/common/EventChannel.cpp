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
// Ident : $Id: EventChannel.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : this is a sample implementation of EventChannel access for your
//         for your reference, you may enhances it for your own purposes
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/EventChannel.cpp $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 24    10-10-26 12:01 Hui.shao
// merged HQ's change as of 6/25/2010 from V1.10
// 
// 23    10-09-23 15:30 Xiaohui.chai
// add sentinel monitoring for eventchannel
// 
// 22    09-12-16 18:47 Fei.huang
// + remove warning msg for Ice 33
// 
// 22    09-12-11 10:48 Fei.huang
// * seperate code for Ice 33
// 
// 21    08-08-14 15:00 Hui.shao
// merged from 1.7.10
// 
// 21    08-07-17 15:15 Hui.shao
// removed content provision events
// 
// 20    08-03-20 11:59 Yixin.tian
// modify sem_trywait to sem_timedwait for linux os
// 
// 19    08-03-07 10:14 Yixin.tian
// WIN32 replaced by ZQ_OS_MSWIN
// 
// 18    08-03-05 14:17 Yixin.tian
// merge for linux
// 
// 17    07-12-26 11:02 Guan.han
// 
// 16    07-12-17 12:29 Guan.han
// change the way to throw exception
// 
// 15    07-12-14 15:38 Xiaohui.chai
// 
// 14    07-08-14 12:22 Hongquan.zhang
// 
// 13    07-05-23 13:28 Hui.shao
// added _IceThrow with category and errorcode
// 
// 12    07-03-13 17:11 Hongquan.zhang
// 
// 11    06-09-28 10:31 Hongquan.zhang
// 
// 10    06-09-26 14:53 Hongquan.zhang
// 
// 9     06-09-25 17:19 Hongquan.zhang
// 
// 8     06-09-22 9:15 Ken.qian
// 
// 7     06-09-06 16:21 Ken.qian
// 
// 6     06-08-31 17:54 Hui.shao
// 
// 5     06-08-31 17:45 Hui.shao
// 
// 4     06-08-10 17:03 Hongquan.zhang
// 
// 3     06-07-31 15:36 Hui.shao
// 
// 2     06-07-24 18:34 Hui.shao
// ===========================================================================

#ifndef WITH_ICESTORM
#  define WITH_ICESTORM
#endif // WITH_ICESTORM

#include "EventChannel.h"
#include "TianShanDefines.h"
#include "Sentinel.h"

#ifdef _TEST_PROGRAM_
#include <iostream>
using namespace std;
#endif

namespace TianShanIce {
namespace Events {

#ifndef 	_DEBUG
void subscr_err(char*fmt, ...)
{
}
#else
#  define subscr_err printf
#endif // _DEBUG

EventChannelImpl::EventChannelImpl(Ice::ObjectAdapterPtr adapter, const char* topicManagerEndpoint, bool createTopicIfNotExist)
: _adapter(adapter), _topicManager(NULL), _bCreateTopicIfNotExist(createTopicIfNotExist)
{
	if (NULL == topicManagerEndpoint || strlen(topicManagerEndpoint) <=0)
		topicManagerEndpoint = DEFAULT_ENDPOINT_TopicManager;

    if(!_adapter)
		::ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(EXPFMT(EventChannel, 300, "EventChannelImpl() null adapter"));

	// first, we trust the topicManagerEndpoint as a full proxy string
	try {
		_topicManager = IceStorm::TopicManagerPrx::checkedCast(_adapter->getCommunicator()->stringToProxy(topicManagerEndpoint));
	}
	catch (...) {}

    if(!_topicManager)
	{
		// if this is not a full proxy string, then trust it as a endpoint
		std::string proxy = SERVICE_NAME_TopicManager ":" ;
		proxy += topicManagerEndpoint;
		_topicManager = IceStorm::TopicManagerPrx::checkedCast(_adapter->getCommunicator()->stringToProxy(proxy));
	}
	
    if(!_topicManager)
		::ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(EXPFMT(EventChannel, 301, "invalid topicManagerEndpoint: %s"), topicManagerEndpoint);

	_bQuit=false;
	_isNetWorkOK=false;
#ifdef ZQ_OS_MSWIN
	_hNotifyBadConn=CreateEvent(NULL,FALSE,FALSE,NULL);
#else
	if(sem_init(&_semNotifyBadConn,0,0) == -1)
		perror("sem_init");
#endif
	start();
}

EventChannelImpl::~EventChannelImpl()
{
	_bQuit=true;
#ifdef ZQ_OS_MSWIN
	SetEvent(_hNotifyBadConn);
	Sleep(1);
#else
	sem_post(&_semNotifyBadConn);
	usleep(1000);
#endif

#ifdef _TEST_PROGRAM_
	DWORD dwResult=GetTickCount();
#endif
	clean();
#ifdef _TEST_PROGRAM_
	cout<<"Clean action use time "<<GetTickCount()-dwResult<<endl;
#endif
#ifdef ZQ_OS_MSWIN
	CloseHandle(_hNotifyBadConn);
#else
	sem_destroy(&_semNotifyBadConn);
#endif
}

::TianShanIce::StrValues EventChannelImpl::protectedSink(const ::TianShanIce::Events::BaseEventSinkPrx& sink, const ::TianShanIce::StrValues& topics, const ::TianShanIce::Properties& qos)
{
	return protectedSinkEx(::Ice::ObjectPrx::checkedCast(sink), topics, qos);
}

::TianShanIce::StrValues EventChannelImpl::protectedSinkEx(const ::Ice::ObjectPrx& sink, const ::TianShanIce::StrValues& topics, const ::TianShanIce::Properties& qos)
{
    ::TianShanIce::StrValues topicsSubscribed;

	if (!_topicManager || !_adapter || !sink || topics.size() <=0)
		return topicsSubscribed;
	
    // subscribe for each topic
    for(::TianShanIce::StrValues::const_iterator it = topics.begin(); it < topics.end(); ++it)
    {
		IceStorm::TopicPrx topic;

		if ((*it).empty())
			continue;

		try
		{
			topic = _topicManager->retrieve(*it);
		}
		catch(const IceStorm::NoSuchTopic& e)
		{
			subscr_err("NoSuchTopic: %s\n", e.name.c_str());

			// if the topic doesn't exist and is not asked to create one, continue to next topic
			if (!_bCreateTopicIfNotExist)
				continue;

			// create a new topic on the topic manager
			topic = _topicManager->create(*it);
		}

		// subscribe the topic
#if ICE_INT_VERSION / 100 >= 303
        topic->subscribeAndGetPublisher(qos, sink);
#else
		topic->subscribe(qos, sink);
#endif

		// register the newly subscribed object into _subscribers
		Lock sync(*this);

		try
		{
			// be sure the previous subscriber is cleaned before overwrite
			if (_subscribers.end() != _subscribers.find(*it) && _subscribers[*it].prx != sink)
				topic->unsubscribe(_subscribers[*it].prx);
		}
		catch(...) 
		{
			_isNetWorkOK=false;
		}
		SubScribeProper proper;
		proper.prx=sink;
		proper.qos=qos;
		_subscribers[*it] = proper;
		
		topicsSubscribed.push_back(*it);
	}
	_isNetWorkOK=true;
	return topicsSubscribed;
}

void EventChannelImpl::RefreshSubscribe()
{
	for(Map::const_iterator it=_subscribers.begin();it!=_subscribers.end();it++)
	{
		IceStorm::TopicPrx topic;
		try
		{
			topic = _topicManager->retrieve(it->first);
		}
		catch(const IceStorm::NoSuchTopic& e)
		{
			subscr_err("NoSuchTopic: %s\n", e.name.c_str());
			
			// if the topic doesn't exist and is not asked to create one, continue to next topic
			if (!_bCreateTopicIfNotExist)
				continue;
			
			// create a new topic on the topic manager
			topic = _topicManager->create(it->first);
		}
		catch (...) 
		{
			_isNetWorkOK=false;
			return;
		}
		try
		{
			topic->unsubscribe(it->second.prx);
#if ICE_INT_VERSION / 100 >= 303
            topic->subscribeAndGetPublisher(it->second.qos,it->second.prx);
#else
			topic->subscribe(it->second.qos,it->second.prx);
#endif
		}
		catch (...) 
		{
			_isNetWorkOK=false;
			return;
		}
		_isNetWorkOK=true;		
	}
}
void EventChannelImpl::clean()
{
    Lock sync(*this);

    for(Map::const_iterator it = _subscribers.begin(); it != _subscribers.end(); it++)
    {
		try
		{
            IceStorm::TopicPrx topic = _topicManager->retrieve(it->first);
			topic->unsubscribe(it->second.prx);
		}
		catch(const IceStorm::NoSuchTopic& e)
		{
			subscr_err("NoSuchTopic: %s\n", e.name.c_str());
		}
    }

	_subscribers.clear();
}

bool EventChannelImpl::sink(::TianShanIce::Streamer::StreamEventSinkPtr& sink, const ::TianShanIce::Properties& qos)
{
	// Set the requested quality of service with default value if the parameter qos is not provided
    IceStorm::QoS workQos;
	IceStorm::QoS::const_iterator it = qos.find("reliability");
    workQos["reliability"] = (qos.end() != it) ? it->second : "oneway";

    ::TianShanIce::StrValues topics;
	topics.push_back(::TianShanIce::Streamer::TopicOfStream);

	topics = protectedSink(::TianShanIce::Events::BaseEventSinkPrx::checkedCast(_adapter->addWithUUID(sink)), topics, workQos);

	return topics.size() >0;
}

bool EventChannelImpl::sink(::TianShanIce::Streamer::PlaylistEventSinkPtr& sink, const ::TianShanIce::Properties& qos)
{
	// Set the requested quality of service with default value if the parameter qos is not provided
    IceStorm::QoS workQos;
	IceStorm::QoS::const_iterator it = qos.find("reliability");
    workQos["reliability"] = (qos.end() != it) ? it->second : "oneway";

    ::TianShanIce::StrValues topics;
	//topics.push_back(::TianShanIce::Streamer::TopicOfStream);
	topics.push_back(::TianShanIce::Streamer::TopicOfPlaylist);

	topics = protectedSink(::TianShanIce::Events::BaseEventSinkPrx::checkedCast(_adapter->addWithUUID(sink)), topics, workQos);

	return topics.size() >0;
}

bool EventChannelImpl::sink(::TianShanIce::Streamer::StreamProgressSinkPtr& sink)
{
	// set the requested quality of service
    IceStorm::QoS workQos;
    workQos["reliability"] = "batch";

    ::TianShanIce::StrValues topics;
	topics.push_back(::TianShanIce::Streamer::TopicOfStreamProgress);

	topics = protectedSink(::TianShanIce::Events::BaseEventSinkPrx::checkedCast(_adapter->addWithUUID(sink)), topics, workQos);

	return topics.size() >0;
}

bool EventChannelImpl::sink(::TianShanIce::SRM::SessionEventSinkPtr& sink, const ::TianShanIce::Properties& qos)
{
	// set the requested quality of service
    IceStorm::QoS workQos;
    workQos["reliability"] = "batch";

    ::TianShanIce::StrValues topics;
	topics.push_back(::TianShanIce::SRM::TopicOfSession);

	::Ice::ObjectPrx obj = _adapter->addWithUUID(sink);

//	::TianShanIce::SRM::SessionEventSinkPrx proxy2 = ::TianShanIce::SRM::SessionEventSinkPrx::checkedCast(obj);
//	::TianShanIce::Events::BaseEventSinkPrx proxy = ::TianShanIce::Events::BaseEventSinkPrx::checkedCast(obj);

	topics = protectedSinkEx(obj, topics, workQos);

	return topics.size() >0;
}

/*
bool EventChannelImpl::sink(::TianShanIce::Storage::ProvisionStateChangeSinkPtr& sink, const ::TianShanIce::Properties& qos)
{
	// set the requested quality of service
    IceStorm::QoS workQos;
    workQos["reliability"] = "twoway ordered";

    ::TianShanIce::StrValues topics;
	topics.push_back(::TianShanIce::Storage::TopicOfProvisionStateChange);

	::Ice::ObjectPrx obj = _adapter->addWithUUID(sink);
	topics = protectedSinkEx(obj, topics, workQos);

	return topics.size() >0;
}

bool EventChannelImpl::sink(::TianShanIce::Storage::ProvisionProgressSinkPtr& sink, const ::TianShanIce::Properties& qos)
{
	// set the requested quality of service
    IceStorm::QoS workQos;
    workQos["reliability"] = "oneway";

    ::TianShanIce::StrValues topics;
	topics.push_back(::TianShanIce::Storage::TopicOfProvisionProgress);

	::Ice::ObjectPrx obj = _adapter->addWithUUID(sink);
	topics = protectedSinkEx(obj, topics, workQos);

	return topics.size() >0;
}
*/

bool EventChannelImpl::sink(::TianShanIce::Events::GenericEventSinkPtr& sink, const TianShanIce::Properties& qos, const std::string& topicName )
{
	IceStorm::QoS workQos;
	workQos["reliability"] = "oneway";

	TianShanIce::StrValues topics;
	topics.push_back( topicName );

	::Ice::ObjectPrx obj = _adapter->addWithUUID(sink);
	topics = protectedSinkEx(obj , topics ,workQos);
	
	return topics.size() > 0;
}

void EventChannelImpl::NotifyBadConnection()
{
	_isNetWorkOK=false;
#ifdef ZQ_OS_MSWIN
	SetEvent(_hNotifyBadConn);
#else
	sem_post(&_semNotifyBadConn);
#endif
}

class BadConnDetector: public ::EventChannel::Sentinel::ExternalControlData {
public:
    BadConnDetector(EventChannelImpl* ec):ec_(ec) {}
    virtual void reportBadConnection() {
        ec_->NotifyBadConnection();
    }
private:
    EventChannelImpl* ec_;
};
int EventChannelImpl::run()
{
    ZQ::common::Log dummyLog;
    BadConnDetector detector(this);
    ::EventChannel::Sentinel sentinel(dummyLog, _adapter->getCommunicator()->proxyToString(_topicManager), &detector);
    sentinel.start();
	do 
	{
#ifdef ZQ_OS_MSWIN
		DWORD	dwRet=WaitForSingleObject(_hNotifyBadConn,1000);
#else
		struct timespec ts;
		struct timeb tb;
		
		ftime(&tb);	
		ts.tv_sec = tb.time + 1;
		ts.tv_nsec = tb.millitm * 1000000;
		sem_timedwait(&_semNotifyBadConn,&ts);
		
#endif
		if(!_bQuit)
		{
			if(!_isNetWorkOK)
				RefreshSubscribe();
			else
			{
				try
				{
					_topicManager->ice_ping();
				}
				catch (...) 
				{
					_isNetWorkOK=false;
#ifdef ZQ_OS_MSWIN
					SetEvent(_hNotifyBadConn);
#else
					sem_post(&_semNotifyBadConn);
#endif
				}
			}
		}		
	} while(!_bQuit);
    sentinel.stop();
	return 1;
}

}} // namespace
