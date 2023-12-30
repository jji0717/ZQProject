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
// Ident : $Id: WeiwooFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/WeiwooFactory.h $
// 
// 3     3/09/11 4:42p Hongquan.zhang
// 
// 2     3/07/11 5:00p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:08 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// ===========================================================================

#ifndef __ZQTianShan_ADPFactory_H__
#define __ZQTianShan_ADPFactory_H__

#include "ZQ_common_conf.h"
#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace Weiwoo {

class WeiwooSvcEnv;

#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class WeiwooFactory : public Ice::ObjectFactory
{
	friend class WeiwooSvcEnv;

public:

    WeiwooFactory(WeiwooSvcEnv& env);

    // Operations from ObjectFactory
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

	typedef IceUtil::Handle<WeiwooFactory> Ptr;

protected:
	WeiwooSvcEnv& _env;
};

}} // namespace

#endif //__ZQTianShan_ADPFactory_H__

