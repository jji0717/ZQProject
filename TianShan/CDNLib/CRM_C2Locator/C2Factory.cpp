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
// Ident : $Id: C2Factory.cpp $
// Branch: $Name:  $
// Author: LXM
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CDNLib/CRM_C2Locator/C2Factory.cpp $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 3     09-12-30 18:08 Fei.huang
// * port to linux server
// 
// 2     09-05-22 11:19 Xiaoming.li
// 
// 1     09-05-22 9:28 Xiaoming.li
// initial check in


#include "C2Env.h"
#include "C2Factory.h"
#include "TransferSessionImpl.h"

namespace ZQTianShan{
namespace CDN{

C2Factory::C2Factory(C2Env& env)
:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		ic->addObjectFactory(this, ::TianShanIce::SCS::TransferSession::ice_staticId());
	}
}

Ice::ObjectPtr C2Factory::create(const std::string& type)
{
	if (::TianShanIce::SCS::TransferSession::ice_staticId() == type)
		return new ::TianShanIce::SCS::TransferSessionImpl(_env);

    return NULL;
}

void C2Factory::destroy()
{
}

}//namespace CDN
}//namespace ZQTianShan
