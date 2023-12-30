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
// Name  : ChODFactory.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-21
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChODFactory.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// ===========================================================================

#ifndef __CHODFACTORY_H__
#define __CHODFACTORY_H__

#include "ZQ_common_Conf.h"

#include <Freeze/Freeze.h>

namespace ZQChannelOnDemand {
		
#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS
		
class ChODFactory : public Ice::ObjectFactory
{
	friend class ChODSvcEnv;
	
public:
	
	ChODFactory(ChODSvcEnv& env);
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();
	
	typedef IceUtil::Handle<ChODFactory> Ptr;
	
protected:
	ChODSvcEnv& _env;
};
		

} // namespace

#endif