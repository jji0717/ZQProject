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
// Ident : $Id: PathFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathFactory.cpp $
// 
// 2     3/07/11 4:54p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 16    07-03-14 12:33 Hongquan.zhang
// 
// 15    06-12-25 16:37 Hui.shao
// 
// 14    06-09-19 11:48 Hui.shao
//
// 7     06-06-12 15:56 Hui.shao
// added file header
// ===========================================================================

#include "PathSvcEnv.h"
#include "PathFactory.h"
#include "PathManagerImpl.h"
#include "Log.h"

namespace ZQTianShan {
namespace AccreditedPath {

PathFactory::PathFactory(PathSvcEnv& env)
:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PathFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, TianShanIce::Transport::PathTicket::ice_staticId());

		ic->addObjectFactory(this, TianShanIce::Transport::StorageLink::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Transport::StorageLinkEx::ice_staticId());

		ic->addObjectFactory(this, TianShanIce::Transport::StreamLink::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Transport::StreamLinkEx::ice_staticId());
	}
}

Ice::ObjectPtr PathFactory::create(const std::string& type)
{
	if (TianShanIce::Transport::StorageLink::ice_staticId() == type
		|| TianShanIce::Transport::StorageLinkEx::ice_staticId() == type)
		return new StorageLinkImpl(_env);

	if (TianShanIce::Transport::StreamLink::ice_staticId() == type
		|| TianShanIce::Transport::StreamLinkEx::ice_staticId() == type)
		return new StreamLinkImpl(_env);

	if (TianShanIce::Transport::PathTicket::ice_staticId() == type)
		return new ADPathTicketImpl(_env);

	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(PathFactory, "create(%s) type unknown"), type.c_str());
    return NULL;
}

void PathFactory::destroy()
{
}

}} // namespace
