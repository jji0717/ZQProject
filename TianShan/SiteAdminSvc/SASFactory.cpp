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
// Ident : $Id: SASFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SASFactory.cpp $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 6     07-12-10 18:47 Hui.shao
// moved event out of txn
// 
// 5     07-08-01 10:59 Hongquan.zhang
// 
// 5     07-07-27 11:24 Hongquan.zhang
// 
// 4     07-07-25 16:00 Hongquan.zhang
// 
// 4     07-07-24 12:05 Hongquan.zhang
// 
// 3     07-05-23 13:29 Hui.shao
// use wrapped ZQ Adapter
// 
// 2     07-03-28 18:35 Hui.shao
// ===========================================================================

#include "SASFactory.h"
#include "SiteAdminImpl.h"
#include "Log.h"

namespace ZQTianShan {
namespace Site {

SASFactory::SASFactory(SiteAdminSvcEnv& env)
:_env(env)
{
	if ((Ice::ObjectAdapterPtr&)_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SASFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, TianShanIce::Site::AppMount::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Site::LiveTxn::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Site::TxnEvent::ice_staticId());
	}
}

Ice::ObjectPtr SASFactory::create(const std::string& type)
{
//	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SASFactory, "create obj of %s"), type.c_str());

	if (TianShanIce::Site::AppMount::ice_staticId() == type)
		return new AppMountImpl(_env);

	if (TianShanIce::Site::LiveTxn::ice_staticId() == type)
		return new LiveTxnImpl(_env);

	if (TianShanIce::Site::TxnEvent::ice_staticId() == type)
		return new TxnEventImpl(_env);

	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SASFactory, "create(%s) type unknown"), type.c_str());
    return NULL;
}

void SASFactory::destroy()
{
//	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SASFactory, "destroy()"));
}

}} // namespace
