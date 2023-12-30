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
// Ident : $Id: WeiwooFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/WeiwooFactory.cpp $
// 
// 2     3/07/11 5:00p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:08 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 8     07-04-12 14:02 Hongquan.zhang
// 
// 7     07-03-13 17:12 Hongquan.zhang
// 
// 6     06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 5     9/21/06 5:11p Hui.shao
// 
// 4     06-07-17 14:47 Hui.shao
// initial impl of session manager
// 
// 3     06-07-14 14:06 Hui.shao
// 
// 2     06-07-12 13:38 Hui.shao
// added logs
// 
// 1     06-07-05 15:25 Hui.shao
// ===========================================================================

#include "WeiwooSvcEnv.h"
#include "WeiwooFactory.h"
#include "SessionImpl.h"
#include "Log.h"

namespace ZQTianShan {
namespace Weiwoo {

WeiwooFactory::WeiwooFactory(WeiwooSvcEnv& env)
:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(WeiwooFactory, "add factory onto communicator"));
		
		//ic->addObjectFactory(this, TianShanIce::SRM::AppMount::ice_staticId());
		
		ic->addObjectFactory(this, TianShanIce::SRM::Session::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::SRM::SessionEx::ice_staticId());
	}
}

Ice::ObjectPtr WeiwooFactory::create(const std::string& type)
{
//	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(WeiwooFactory, "create obj of %s"), type.c_str());

//	if (TianShanIce::SRM::AppMount::ice_staticId() == type)
//		return new AppMountImpl(_env);
	
	if (TianShanIce::SRM::Session::ice_staticId() == type
		|| TianShanIce::SRM::SessionEx::ice_staticId() == type)
		return new SessionImpl(_env);

	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(WeiwooFactory, "create(%s) type unknown"), type.c_str());
    return NULL;
}

void WeiwooFactory::destroy()
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(WeiwooFactory, "destroy()"));
}

}} // namespace
