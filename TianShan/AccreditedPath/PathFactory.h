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
// Ident : $Id: PathFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathFactory.h $
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
// 10    06-09-19 11:45 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_PathFactory_H__
#define __ZQTianShan_PathFactory_H__

#include "ZQ_common_conf.h"
#include <Freeze/Freeze.h>

class PathSvcEnv;

namespace ZQTianShan {
namespace AccreditedPath {

#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class PathFactory : public Ice::ObjectFactory
{
	friend class PathSvcEnv;

public:

	typedef IceUtil::Handle<PathFactory> Ptr;

    PathFactory(PathSvcEnv& env);

    // Operations from ObjectFactory
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

protected:
	
	PathSvcEnv& _env;
};


}} // namespace

#endif // __ZQTianShan_PathFactory_H__
