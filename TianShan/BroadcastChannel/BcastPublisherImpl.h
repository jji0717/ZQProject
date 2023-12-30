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

// Branch: $Name:BcastPublisherImpl.h$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastPublisherImpl.h $
// 
// 3     1/11/16 5:21p Dejian.fei
// 
// 2     5/30/14 4:43p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#ifndef __BcastPublisherImpl_h__
#define __BcastPublisherImpl_h__
#include <TsAppBcast.h>
#include <BcastChannelEx.h>
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>
#include "TianShanDefines.h"
#define		MAX_BATCH_ITERATOR_SIZE		512

namespace ZQBroadCastChannel
{ 
class BroadCastChannelEnv;
class BcastPublisherImpl : public TianShanIce::Application::Broadcast::BcastPublisher,
                           //public IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>
							public ICEAbstractMutexWLock
{
public:
	typedef ::IceInternal::Handle< BcastPublisherImpl> Ptr;
    BcastPublisherImpl(BroadCastChannelEnv& bcastChenv);
	~BcastPublisherImpl();

	///implement pointpublisher interface
	virtual TianShanIce::Application::PublishPointPrx publish(const ::std::string&,
		::Ice::Int,
		const ::std::string&,
		const Ice::Current&);

	virtual TianShanIce::Application::PublishPointPrx open(const ::std::string&,
		const Ice::Current&);

	virtual TianShanIce::StrValues list(const Ice::Current&);

	virtual void listPublishPointInfo_async(const TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr&,
		const TianShanIce::StrValues&,
		const Ice::Current&) const;

   ////implement BcastPublisher interface
	virtual TianShanIce::Application::Broadcast::BcastPublishPointPrx createBcastPublishPoint(const ::std::string&,
		const TianShanIce::SRM::ResourceMap&,
		const TianShanIce::Properties&,
		const ::std::string&,
		const Ice::Current&);

	virtual TianShanIce::Application::Broadcast::NVODChannelPublishPointPrx createNVODPublishPoint(const ::std::string&,
		const TianShanIce::SRM::ResourceMap&,
		::Ice::Short,
		::Ice::Int,
		const TianShanIce::Properties&,
		const ::std::string&,
		const Ice::Current&);
	virtual void addFilterItem(const TianShanIce::Application::ChannelItem&,
		const Ice::Current&);

	virtual void removeFilterItem(const ::std::string&,
		const Ice::Current&);

	virtual void listFilterItems_async(const TianShanIce::Application::Broadcast::AMD_BcastPublisher_listFilterItemsPtr&,
		const Ice::Current&)const;
protected:
	bool checkChannelItem(TianShanIce::Application::ChannelItem& channelItem);

protected:
	BroadCastChannelEnv& _env;
};

class FilterItemsImpl : public TianShanIce::Application::Broadcast::FilterItems,
	                //public IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>
					public ICEAbstractMutexWLock
{
public:
	FilterItemsImpl(BroadCastChannelEnv& bcastChenv);
	~FilterItemsImpl();

	typedef IceUtil::Handle<FilterItemsImpl> Ptr;

	virtual void addFilterItem(const ::std::string&, const Ice::Current&);

	virtual void removeFilterItem(const ::std::string&, const Ice::Current&);

	virtual TianShanIce::StrValues getFilterItemSequence(const Ice::Current&);

	TianShanIce::Application::ChannelItem findFilterItem(const ::std::string& itemName,
		                             const Ice::Current& current);
protected:
	BroadCastChannelEnv& _env;
};
}
#endif ///end define  __BcastPublisherImpl_h__
