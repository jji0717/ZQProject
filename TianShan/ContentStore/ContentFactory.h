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
// Ident : $Id: ContentFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentFactory.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     08-12-10 12:56 Yixin.tian
// 
// 2     08-11-14 16:20 Yixin.tian
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// 
// 2     08-07-21 10:35 Hui.shao
// check in the works of the weekend
// 
// 1     08-07-11 16:29 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_ContentFactory_H__
#define __ZQTianShan_ContentFactory_H__

#include "ZQ_common_conf.h"

#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace ContentStore {
class ContentStoreImpl;
class ContentFactory : public Ice::ObjectFactory
{
	friend class ContentStoreImpl;
	
public:

	ContentFactory(ContentStoreImpl& lib);
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();
	
	typedef IceUtil::Handle<ContentFactory> Ptr;
	
protected:
	ContentStoreImpl& _store;
};
		
}} // namespace

#endif //__ZQTianShan_ContentFactory_H__

