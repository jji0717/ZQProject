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
// Ident : $Id: CacheFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheFactory.cpp $
// 
// 3     8/09/12 12:13p Hui.shao
// 
// 2     6/27/12 6:55p Li.huang
// 
// 1     1/17/12 11:44a Hui.shao
// ===========================================================================

#include "CacheFactory.h"
#include "CacheStoreImpl.h"
#include "Log.h"

#define MOLOG	(_store._log)

namespace ZQTianShan {
namespace ContentStore {

CacheFactory::CacheFactory(CacheStoreImpl& store)
:_store(store)
{
	if (_store._adapter)
	{
		Ice::CommunicatorPtr ic = _store._adapter->getCommunicator();
		
		ic->addObjectFactory(this, TianShanIce::Storage::TopFolder::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Storage::CacheTask::ice_staticId());
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheFactory, "factories added onto communicator"));
	}
}

Ice::ObjectPtr CacheFactory::create(const std::string& type)
{
	if (TianShanIce::Storage::TopFolder::ice_staticId() == type)
		return new TopFolderImpl(_store);

	if (TianShanIce::Storage::CacheTask::ice_staticId() == type)
		return new CacheTaskImpl(_store);

	MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CacheFactory, "create(%s) type unknown"), type.c_str());
	return NULL;
}

void CacheFactory::destroy()
{
	//MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheFactory, "destroy()"));
}

}} // namespace
