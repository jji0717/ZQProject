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
// Ident : $Id: MetaLibFactory.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MetaLib/MetaLibFactory.cpp $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 2     09-04-02 11:15 Hui.shao
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================

#include "MetaLibFactory.h"
#include "MetaLibImpl.h"
#include "Log.h"

namespace ZQTianShan {
namespace MetaLib {

MetaLibFactory::MetaLibFactory(MetaLibImpl& lib)
:_lib(lib)
{
	if (_lib._adapter)
	{
		Ice::CommunicatorPtr ic = _lib._adapter->getCommunicator();
		
		_lib._log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLibFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, TianShanIce::Repository::LibMetaObject::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Repository::LibMetaValue::ice_staticId());
	}
}

Ice::ObjectPtr MetaLibFactory::create(const std::string& type)
{
//	_lib._log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLibFactory, "create obj of %s"), type.c_str());
	
	if (TianShanIce::Repository::LibMetaObject::ice_staticId() == type)
		return new LibMetaObjectImpl(_lib);

	if (TianShanIce::Repository::LibMetaValue::ice_staticId() == type)
		return new LibMetaValueImpl(_lib);
	
	_lib._log(ZQ::common::Log::L_WARNING, CLOGFMT(MetaLibFactory, "create(%s) type unknown"), type.c_str());
	return NULL;
}

void MetaLibFactory::destroy()
{
//	_lib._log(ZQ::common::Log::L_DEBUG, CLOGFMT(MetaLibFactory, "destroy()"));
}

}} // namespace
