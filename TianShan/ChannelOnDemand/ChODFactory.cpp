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
// Name  : ChODFactory.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-21
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChODFactory.cpp $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 8     09-02-06 17:24 Haoyuan.lu
// 
// 7     06-11-15 17:34 Jie.zhang
// 
// 6     06-10-24 18:43 Jie.zhang
// 
// 5     06-10-23 10:04 Jie.zhang
// 
// 4     06-09-20 14:32 Jie.zhang
// 
// 3     06-08-28 12:01 Bernie.zhao
// 1st draft done
// 
// 2     06-08-23 12:42 Bernie.zhao
// creation
// ===========================================================================

#include "ChODFactory.h"
#include "ChannelPublishPointImpl.h"
#include "Log.h"
#include "PurchaseItemImpl.h"
#include "PurchaseImpl.h"


#define LOG_MODULE_NAME			"ChODFactory"


namespace ZQChannelOnDemand {
		
//////////////////////////////////////////////////////////////////////////
// class ChODFactory
//////////////////////////////////////////////////////////////////////////

ChODFactory::ChODFactory(ChODSvcEnv& env)
	:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "add factory into communicator"));
		
		ic->addObjectFactory(this, NS_PREFIX(ChannelOnDemand::PurchaseItemAssoc)::ice_staticId());
		ic->addObjectFactory(this, NS_PREFIX(ChannelOnDemand::ChannelPublishPoint)::ice_staticId());
		ic->addObjectFactory(this, NS_PREFIX(ChannelOnDemand::ChannelPurchaseEx)::ice_staticId());
		ic->addObjectFactory(this, NS_PREFIX(ChannelOnDemand::ChannelPurchase)::ice_staticId());

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "add factory completed"));
	}
}

Ice::ObjectPtr ChODFactory::create(const std::string& type)
{
//	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "create %s object"), type.c_str());
	
	if (NS_PREFIX(ChannelOnDemand::PurchaseItemAssoc)::ice_staticId() == type)
		return new PurchaseItemImpl(_env);
	
	if (NS_PREFIX(ChannelOnDemand::ChannelPublishPoint)::ice_staticId() == type)
		return new ChannelPublishPointImpl(_env);

	if (NS_PREFIX(ChannelOnDemand::ChannelPurchaseEx)::ice_staticId() == type)
		return new PurchaseImpl(_env);

	if (NS_PREFIX(ChannelOnDemand::ChannelPurchase)::ice_staticId() == type)
		return new PurchaseImpl(_env);
	
	glog(ZQ::common::Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "create(%s) type unknown"), type.c_str());
	return NULL;
}

void ChODFactory::destroy()
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "destroy()"));
}
		
} // namespace
