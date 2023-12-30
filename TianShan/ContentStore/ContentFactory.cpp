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
// Ident : $Id: ContentFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentFactory.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 5     09-12-22 17:46 Hui.shao
// abstracted folder from volume
// 
// 4     09-01-23 12:31 Hongquan.zhang
// 
// 3     08-11-07 11:00 Jie.zhang
// add common log define to unify log using style
// 
// 2     08-10-07 17:09 Hui.shao
// added volume and impl
// 
// 2     08-07-21 11:51 Hui.shao
// check in the works of last weekend
// 
// 1     08-07-11 16:29 Hui.shao
// ===========================================================================

#include "ContentFactory.h"
#include "ContentImpl.h"
#include "Log.h"

#define MOLOG	(_store._log)
#define MOEVENT	(_store._eventlog)

namespace ZQTianShan {
namespace ContentStore {

ContentFactory::ContentFactory(ContentStoreImpl& store)
:_store(store)
{
	if (_store._adapter)
	{
		Ice::CommunicatorPtr ic = _store._adapter->getCommunicator();
		
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, TianShanIce::Storage::Content::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Storage::UnivContent::ice_staticId());

		ic->addObjectFactory(this, TianShanIce::Storage::Volume::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Storage::VolumeEx::ice_staticId());
	}
}

Ice::ObjectPtr ContentFactory::create(const std::string& type)
{
	if (TianShanIce::Storage::Content::ice_staticId() == type || TianShanIce::Storage::UnivContent::ice_staticId() == type)
		return new ContentImpl(_store);

	if (TianShanIce::Storage::Volume::ice_staticId() == type || TianShanIce::Storage::VolumeEx::ice_staticId() == type || 
		TianShanIce::Storage::Folder::ice_staticId() == type)
		return new VolumeImpl(_store);

	MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentFactory, "create(%s) type unknown"), type.c_str());
	return NULL;
}

void ContentFactory::destroy()
{
	//MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentFactory, "destroy()"));
}

}} // namespace
