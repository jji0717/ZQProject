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
// Ident : $Id: CPEFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPEFactory.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 2     08-12-12 13:44 Yixin.tian
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_CPEFactory_H__
#define __ZQTianShan_CPEFactory_H__

#include "ZQ_common_conf.h"

#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace CPE {

class CPEEnv;
#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class CPEFactory : public Ice::ObjectFactory
{
	friend class CPEEnv;

public:

    CPEFactory(CPEEnv& env);

    // Operations from ObjectFactory
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

	typedef IceUtil::Handle<CPEFactory> Ptr;

protected:
	CPEEnv& _env;
};

}} // namespace

#endif //__ZQTianShan_CPEFactory_H__
