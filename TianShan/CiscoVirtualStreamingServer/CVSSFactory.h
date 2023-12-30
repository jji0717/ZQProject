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
// Ident : $Id: CVSSFactory.h $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CiscoVirtualStreamingServer/CVSSFactory.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 1     08-12-15 9:07 Xiaoming.li
// initial checkin
// 
// 1     08-06-13 11:23 Xiaoming.li
// 
// 1     08-04-18 15:36 xiaoming.li
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_CVSSFactory_H__
#define __ZQTianShan_CVSSFactory_H__

#include "ZQ_common_Conf.h"
#include "common_structure.h"
#include "ZQThreadPool.h"
#include "FileLog.h"

#include <Freeze/Freeze.h>
#include <CVSS.h>

namespace ZQTianShan{
namespace CVSS{

#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class CVSSFactory : public Ice::ObjectFactory
{
	friend class CVSSEnv;

public:

	CVSSFactory(CVSSEnv& env);

	typedef IceUtil::Handle<CVSSFactory> Ptr;
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string& type);
	virtual void destroy();

protected:
	CVSSEnv& _env;
};

}//namespace CVSS

}//namespace ZQTianShan

#endif __ZQTianShan_CVSSFactory_H__
