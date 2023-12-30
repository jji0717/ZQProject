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
// Ident : $Id: CacheFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheFactory.h $
// 
// 2     6/28/12 2:53p Hui.shao
// 
// 1     1/17/12 11:44a Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_CacheFactory_H__
#define __ZQTianShan_CacheFactory_H__

#include "ZQ_common_conf.h"

#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace ContentStore {
class CacheStoreImpl;
class CacheFactory : public Ice::ObjectFactory
{
	friend class CacheStoreImpl;
	
public:

	CacheFactory(CacheStoreImpl& store);
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();
	
	typedef IceUtil::Handle<CacheFactory> Ptr;
	
protected:
	CacheStoreImpl& _store;
};
		
}} // namespace

#endif //__ZQTianShan_CacheFactory_H__

