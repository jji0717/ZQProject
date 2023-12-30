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
// Name  : ChannelPublishPointImpl.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-21
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChannelPublishPointImpl.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 13    09-02-06 17:27 Haoyuan.lu
// 
// 12    08-11-18 10:46 Haoyuan.lu
// 
// 11    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 10    08-03-05 12:15 Guan.han
// 
// 10    08-03-04 18:53 Guan.han
// 
// 9     08-01-07 18:48 Guan.han
// 
// 8     07-12-12 12:07 Guan.han
// 
// 7     07-07-06 14:48 Jie.zhang
// add a confige: enablechannelmaxdurationcheck and add some logic to
// avoid null playlist handle
// 
// 6     06-10-23 10:04 Jie.zhang
// 
// 5     06-09-26 11:18 Jie.zhang
// 
// 4     06-09-20 14:32 Jie.zhang
// ===========================================================================

#ifndef	__CHANNELPUBLISHPOINTIMPL_H__ 
#define __CHANNELPUBLISHPOINTIMPL_H__

#include "ChODDefines.h"
#include "ChannelOnDemandEx.h"
#include "ChannelItemDict.h"
#include "ChODSvcEnv.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#ifdef USE_OLD_NS
#  define  CI_NS_PREFIX(_CLS) ChannelOnDemand::_CLS
#else
#  define  CI_NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#endif // USE_OLD_NS

namespace ZQChannelOnDemand {

//////////////////////////////////////////////////////////////////////////
// implementation for ChannelPublishPoint
//////////////////////////////////////////////////////////////////////////

class ChannelPublishPointImpl : public NS_PREFIX(ChannelOnDemand::ChannelPublishPoint), public IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>
{
	friend class ChannelPublisherImpl;
	
	typedef ::IceInternal::Handle< ChannelPublishPointImpl> Ptr;

public:
	// constructors/destructors
	ChannelPublishPointImpl(ChODSvcEnv& env);
    ~ChannelPublishPointImpl();

	// channel operations
	virtual ::std::string getName(const ::Ice::Current& c) const;
	
	virtual ::std::string getDesc(const ::Ice::Current& c) const;

	virtual void setDesc(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	
	virtual ::TianShanIce::StrValues getItemSequence(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& c) const;
	
	virtual void setProperties(const ::TianShanIce::Properties& newProps, const ::Ice::Current& c);
	
	virtual CI_NS_PREFIX(ChannelItem) findItem(const ::std::string& itemName, const ::Ice::Current& c) const;

	virtual void appendItem(const CI_NS_PREFIX(ChannelItem)& newItem, const ::Ice::Current& c);
	
	virtual void insertItem(const ::std::string& atItemName, const CI_NS_PREFIX(ChannelItem)& newItem, const ::Ice::Current& c);

	virtual void replaceItem(const ::std::string& oldName, const CI_NS_PREFIX(ChannelItem)& newItem, const ::Ice::Current& c);
	
	virtual void removeItem(const ::std::string& itemName, const ::Ice::Current& c);
	
	virtual void destroy(const ::Ice::Current& c);

    virtual ::Ice::Int getMaxBitrate(const ::Ice::Current& = ::Ice::Current()) const;

    virtual void setMaxBitrate(::Ice::Int, const ::Ice::Current& = ::Ice::Current());

	virtual ::std::string getOnDemandName(const ::Ice::Current& = ::Ice::Current()) const;

	virtual void setOnDemandName(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

    virtual void restrictReplica(const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::StrValues listReplica(const ::Ice::Current& = ::Ice::Current()) const;

#ifdef USE_OLD_NS

	virtual void enable(bool, const ::Ice::Current& c);

	virtual bool ifEnable(const ::Ice::Current& c) const;

#else

	virtual ::std::string getType(const ::Ice::Current& c) const;

	virtual void allowDemand(bool, const ::Ice::Current& c);

	virtual bool isAvailableOnDemand(const ::Ice::Current& c) const;

	virtual void appendItemAs(const CI_NS_PREFIX(ChannelItem)& newItem, const ::std::string& newName, const ::Ice::Current& c);

	virtual void insertItemAs(const ::std::string& atItemName, const CI_NS_PREFIX(ChannelItem)& newItem, const ::std::string& newName, const ::Ice::Current& c);

#endif // USE_OLD_NS

protected:

	// event callbacks	
	virtual void OnChannelDestroyed();
//	void checkChannelDuration(LONGLONG lastStartTime);

	ChODSvcEnv& _env;

};
		

}	// end of namespace ZQChannelOnDemand


#endif