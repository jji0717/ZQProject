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
// Name  : BcastChFactory.cpp
// Author : Li.Huang
// Date  : 
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastChFactory.cpp $
// 
// 2     10/17/14 3:37p Li.huang
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

#include "BcastChFactory.h"
#include "Log.h"
#include "BcastPublishPointImpl.h"
#include "NVODChannelPublishPointImpl.h"
#include "BcastPurchaseImpl.h"
#include "BroadCastChannelEnv.h"
#include "ChannelItemAssocImpl.h"

namespace ZQBroadCastChannel {

	//////////////////////////////////////////////////////////////////////////
	// class BcastChFactory
	//////////////////////////////////////////////////////////////////////////

	BcastChFactory::BcastChFactory(BroadCastChannelEnv& env)
		                                               
		:_env(env)
	{
		if (_env._adapter)
		{
			Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChFactory, "add factory into communicator"));

			ic->addObjectFactory(this, TianShanIce::Application::Broadcast::BcastPublishPointEx::ice_staticId());
			ic->addObjectFactory(this, TianShanIce::Application::Broadcast::NVODChannelPublishPointEx::ice_staticId());
			ic->addObjectFactory(this, TianShanIce::Application::Broadcast::BcastPurchase::ice_staticId());	
			ic->addObjectFactory(this, TianShanIce::Application::Broadcast::ChannelItemAssoc::ice_staticId());	
			ic->addObjectFactory(this, TianShanIce::Application::Broadcast::FilterItems::ice_staticId());	

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChFactory, "add factory completed"));
		}
	}

	Ice::ObjectPtr BcastChFactory::create(const std::string& type)
	{
		//	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChFactory, "create %s object"), type.c_str());

		if (TianShanIce::Application::Broadcast::BcastPublishPointEx::ice_staticId() == type)
			return new BcastPublishPointImpl(_env);

		if (TianShanIce::Application::Broadcast::NVODChannelPublishPointEx::ice_staticId() == type)
			return new NVODChannelPublishPointImpl(_env);

		if (TianShanIce::Application::Broadcast::BcastPurchase::ice_staticId() == type)
			return new BcastPurchaseImpl(_env);

		if (TianShanIce::Application::Broadcast::ChannelItemAssoc::ice_staticId() == type)
			return new ChannelItemAssocImpl(_env);

		if (TianShanIce::Application::Broadcast::FilterItems::ice_staticId() == type)
			return new FilterItemsImpl(_env);

		glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastChFactory, "create(%s) type unknown"), type.c_str());
		return NULL;
	}

	void BcastChFactory::destroy()
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChFactory, "destroy()"));
	}

} // namespace
