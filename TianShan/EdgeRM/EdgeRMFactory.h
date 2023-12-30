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
// Ident : $Id: EdgeRMFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMFactory.h $
// 
// 2     5/23/13 4:00p Li.huang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-02-26 17:53 Hui.shao
// initial created
// ===========================================================================

#ifndef __ZQTianShan_EdgeRMFactory_H__
#define __ZQTianShan_EdgeRMFactory_H__

#include "../common/TianShanDefines.h"

#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace EdgeRM {

#define FACTOBJNAME(_CLASS) "ZQTianShan::" #_CLASS
class EdgeRMEnv;
class EdgeRMFactory : public Ice::ObjectFactory
{
	friend class EdgeRMEnv;

public:

    EdgeRMFactory(EdgeRMEnv& env);

    // Operations from ObjectFactory
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

	typedef IceUtil::Handle<EdgeRMFactory> Ptr;

protected:
	EdgeRMEnv& _env;
};

}} // namespace

#endif // __ZQTianShan_EdgeRMFactory_H__
