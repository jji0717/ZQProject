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
//
// Name  : ChannelPublisherImpl.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-23
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChannelPublisherImpl.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 14    09-10-08 16:11 Haoyuan.lu
// 
// 13    09-02-06 17:16 Haoyuan.lu
// 
// 12    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 11    08-03-05 12:15 Guan.han
// 
// 11    08-03-04 18:53 Guan.han
// 
// 10    08-01-15 15:52 Guan.han
// added listChannelInfo() shell
// 
// 9     08-01-02 11:42 Guan.han
// 
// 8     07-12-12 12:07 Guan.han
// 
// 7     06-10-23 10:04 Jie.zhang
// 
// 6     06-09-26 11:18 Jie.zhang
// 
// 5     06-09-20 14:32 Jie.zhang
// ===========================================================================

#ifndef _CHANNELPUBLISHERIMPL_H__
#define _CHANNELPUBLISHERIMPL_H__

#include "ChODDefines.h"
#ifdef USE_OLD_NS
#	include "ChannelOnDemand.h"
#else
#	include "TsAppChOD.h"
#endif
#include "ChannelOnDemandEx.h"
#include "ChannelItemDict.h"
#include "NativeThread.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>


//////////////////////////////////////////////////////////////////////////
//define to enable expire scan
//#define  ENABLE_CHANNEL_EXPIRE_MANAGER


namespace ZQChannelOnDemand {
	

#define		MAX_BATCH_ITERATOR_SIZE		512

#define		DEFAULT_MONITOR_INTERVAL	60			// 60 seconds
#define		INFINITE_MONITOR_INTERVAL	INFINITE

class ChannelPublisherImpl;
class ChODSvcEnv;

class ChannelItemMoniter : public ZQ::common::NativeThread
{
public:
	ChannelItemMoniter(ChannelPublisherImpl& publisher);
	~ChannelItemMoniter();

public:
	// overrided member functions of NativeThread
	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);

public:
	// other operations
	void signalStop();
	void setInterval(DWORD interval);

private:
	ChannelPublisherImpl&	_publisher;
	bool	_bQuit;
	DWORD	_interval;
	HANDLE	_hSignal;
};

//////////////////////////////////////////////////////////////////////////
// implementation for ChannelPublishPoint
//////////////////////////////////////////////////////////////////////////

class ChannelPublisherImpl : public NS_PREFIX(ChannelOnDemand::ChannelPublisherEx)
{
	friend class ChannelItemMoniter;
public:
	typedef ::IceInternal::Handle<ChannelPublisherImpl> Ptr;

	// constructors/destructors
	ChannelPublisherImpl(ChODSvcEnv& env);
	~ChannelPublisherImpl();
#ifdef USE_OLD_NS
	// derived from ChannelPublisher which defined in ChannelOnDemand.ICE
	virtual ::ChannelOnDemand::ChannelPublishPointPrx publish(const ::std::string&, ::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual ::ChannelOnDemand::ChannelPublishPointPrx publishEx(const ::std::string&, const ::std::string&, ::Ice::Int, const ::TianShanIce::Properties&, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual ::ChannelOnDemand::ChannelPublishPointPrx open(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual ::TianShanIce::StrValues list(const ::Ice::Current& = ::Ice::Current());
	virtual void listChannelInfo_async(const ::ChannelOnDemand::AMD_ChannelPublisher_listChannelInfoPtr& amdCB, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNames, const ::Ice::Current& c) const;
#else
	// derived from ChannelPublisher which defined in TsAppChOD.ICE/TsApplication.ICE
	virtual ::TianShanIce::Application::PublishPointPrx publish(const ::std::string&, ::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
    virtual ::TianShanIce::Application::OnDemandPublishPointPrx publishEx(const ::std::string&, const ::std::string&, ::Ice::Int, const ::TianShanIce::Properties&, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
    virtual ::TianShanIce::Application::PublishPointPrx open(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
    virtual ::TianShanIce::StrValues list(const ::Ice::Current& = ::Ice::Current());
    virtual void listOnDemandPointInfo_async(const ::TianShanIce::Application::AMD_OnDemandPublisher_listOnDemandPointInfoPtr& amdCB, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNames, const ::Ice::Current& c) const;
	virtual void listPublishPointInfo_async(const ::TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr&, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::std::string getChannelByStream(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
#endif //USE_OLD_NS
#ifdef ENABLE_CHANNEL_EXPIRE_MANAGER
public:
	// server operations

	/// set expired item monitor thread scan interval  
	///@param[in]	interval	the sleep interval, in seconds, must be larger than DEFAULT_MONITOR_INTERVAL
	///@remarks	If this value is not set explicitly, default value is 10 seconds.
	///Or, to disable the thread, set to INFINITE_MONITOR_INTERVAL.  If the value specified is smaller than
	///DEFAULT_MONITOR_INTERVAL, the function will automatically adjust the value to be DEFAULT_MONITOR_INTERVAL
	void setMonitorInterval(DWORD interval);


	/// set expired item monitor thread trace flag
	///@param[in]	traceflag	if this is set to true, all the expired items will
	///be logged when they are being removed from the channel
	void setMonitorTraceFlag(bool traceflag);
	
protected:

	// internal methods
	int _scanForExpire();
	ChannelItemMoniter	_moniter;

	DWORD	_moniterInterval;
	bool	_monitorTraceFlag;

#endif

protected:
	::std::string _getItemPrefix(std::string itemkey);

	ChODSvcEnv& _env;
};

}	// ZQChannelOnDemand

#endif