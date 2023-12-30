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
// Ident : $Id: EdgeRMFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMFactory.cpp $
// 
// 3     11/02/12 1:08p Li.huang
// 
// 2     10/31/12 2:47p Li.huang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 2     09-08-07 9:54 Xiaoming.li
// add S6 handler
// 
// 1     09-02-26 17:53 Hui.shao
// initial created
// ===========================================================================

#include "EdgeRMFactory.h"
#include "EdgeRMImpl.h"
#include "Log.h"
#include "EdgeRMEnv.h"
namespace ZQTianShan {
namespace EdgeRM {

#define envlog (_env._log)

EdgeRMFactory::EdgeRMFactory(EdgeRMEnv& env)
:_env(env)
{
	if ((Ice::ObjectAdapterPtr&)_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, TianShanIce::EdgeResource::EdgeDevice::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::EdgeResource::EdgeDeviceEx::ice_staticId());

		ic->addObjectFactory(this, TianShanIce::EdgeResource::EdgeChannel::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::EdgeResource::EdgeChannelEx::ice_staticId());

		ic->addObjectFactory(this, TianShanIce::EdgeResource::Allocation::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::EdgeResource::AllocationEx::ice_staticId());
	}
}

Ice::ObjectPtr EdgeRMFactory::create(const std::string& type)
{
//	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMFactory, "create obj of %s"), type.c_str());

	if (TianShanIce::EdgeResource::EdgeDevice::ice_staticId() == type || TianShanIce::EdgeResource::EdgeDeviceEx::ice_staticId() == type)
		return new EdgeDeviceImpl(_env);

	if (TianShanIce::EdgeResource::EdgeChannel::ice_staticId() == type || TianShanIce::EdgeResource::EdgeChannelEx::ice_staticId() == type)
		return new EdgeChannelImpl(_env);

	if (TianShanIce::EdgeResource::Allocation::ice_staticId() == type || TianShanIce::EdgeResource::AllocationEx::ice_staticId() == type)
		return new AllocationImpl(_env);

	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMFactory, "create(%s) type unknown"), type.c_str());
    return NULL;
}

void EdgeRMFactory::destroy()
{
//	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMFactory, "destroy()"));
}

}} // namespace
