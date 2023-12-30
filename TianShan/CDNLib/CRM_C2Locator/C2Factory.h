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
// Ident : $Id: C2Factory.h $
// Branch: $Name:  $
// Author: LXM
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CDNLib/CRM_C2Locator/C2Factory.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 2     09-12-30 18:08 Fei.huang
// * port to linux server
// 
// 1     09-05-22 9:28 Xiaoming.li
// initial check in
// 

#ifndef __ZQTianShan_C2Factory_H__
#define __ZQTianShan_C2Factory_H__

//include ice header
#include <Freeze/Freeze.h>

//include tianshan common header
#include "ZQ_common_conf.h"
#include "FileLog.h"

namespace ZQTianShan{
namespace CDN{

class C2Env;
#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class C2Factory : public Ice::ObjectFactory
{
friend class C2Env;

public:

	C2Factory(C2Env& env);

	typedef IceUtil::Handle<C2Factory> Ptr;
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string& type);
	virtual void destroy();

protected:
	C2Env& _env;
};

}//namespace CDN

}//namespace ZQTianShan

#endif //__ZQTianShan_C2Factory_H__

