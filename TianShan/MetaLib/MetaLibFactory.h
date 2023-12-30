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
// Ident : $Id: MetaLibFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MetaLib/MetaLibFactory.h $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 2     10-07-06 17:41 Yixin.tian
// merge for Linux OS
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_MetaLibFactory_H__
#define __ZQTianShan_MetaLibFactory_H__

#include "ZQ_common_conf.h"
#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace MetaLib {
	
class MetaLibImpl;	
class MetaLibFactory : public Ice::ObjectFactory
{
	friend class MetaLibImpl;
	
public:

	MetaLibFactory(MetaLibImpl& lib);
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();
	
	typedef IceUtil::Handle<MetaLibFactory> Ptr;
	
protected:
	MetaLibImpl& _lib;
};
		
}} // namespace

#endif// __ZQTianShan_MetaLibFactory_H__
