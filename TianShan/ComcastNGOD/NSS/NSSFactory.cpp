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
// Ident : $Id: NSSFactory.cpp $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ComcastNGOD/NSS/NSSFactory.cpp $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 2     08-08-13 11:00 Xiaoming.li
// 
// 1     08-06-13 11:23 Xiaoming.li
// 
// 1     08-04-18 15:45 xiaoming.li
// initial checkin
// ===========================================================================

#include "NSSFactory.h"
#include "NSSImpl.h"
#include "NSSEnv.h"
#include "FileLog.h"

namespace ZQTianShan{
namespace NSS{

NSSFactory::NSSFactory(NSSEnv& env)
:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, ::TianShanIce::Streamer::NGODStreamServer::NGODStreamEx::ice_staticId());
	}
}

Ice::ObjectPtr NSSFactory::create(const std::string& type)
{
	//envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSFactory, "create obj of %s"), type.c_str());

	if (::TianShanIce::Streamer::NGODStreamServer::NGODStreamEx::ice_staticId() == type)
		return new NGODStreamImpl(_env);

	//return new NGODStreamImpl(_env);
	//envlog(ZQ::common::Log::L_WARNING, CLOGFMT(NSSFactory, "create(%s) type unknown"), type.c_str());
    return NULL;
}

void NSSFactory::destroy()
{
	//envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSFactory, "destroy()"));
}

}//namespace NSS
}//namespace ZQTianShan
