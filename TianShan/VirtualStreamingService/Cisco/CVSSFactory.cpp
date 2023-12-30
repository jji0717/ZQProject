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
// Ident : $Id: CVSSFactory.cpp $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ===========================================================================

#include "CVSSFactory.h"
#include "CVSSImpl.h"
#include "CVSSEnv.h"
#include "FileLog.h"

namespace ZQTianShan{
namespace CVSS{

CVSSFactory::CVSSFactory(CVSSEnv& env)
:_env(env)
{
	if (_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, ::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStream::ice_staticId());
	}
}

Ice::ObjectPtr CVSSFactory::create(const std::string& type)
{
	//envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSFactory, "create obj of %s"), type.c_str());

	if (::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStream::ice_staticId() == type)
		return new CiscoVirtualStreamImpl(_env);
    return NULL;
}

void CVSSFactory::destroy()
{
	//envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSFactory, "destroy()"));
}

}//namespace CVSS
}//namespace ZQTianShan
