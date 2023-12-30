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
// Name  : BcastChFactory.h
// Author : Li.Huang
// Date  : 
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastChFactory.h $
// 
// 2     5/30/14 2:41p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================

#ifndef __BcastChFactory_h__
#define __BcastChFactory_h__

#include "ZQ_common_conf.h"
#include <Freeze/Freeze.h>

namespace ZQBroadCastChannel
{
	class BroadCastChannelEnv;

	class BcastChFactory : public Ice::ObjectFactory
	{
	public:
		BcastChFactory(BroadCastChannelEnv& env);

		// Operations from ObjectFactory
		virtual Ice::ObjectPtr create(const std::string&);
		virtual void destroy();

		typedef IceUtil::Handle<BcastChFactory> Ptr;

	protected:
		BroadCastChannelEnv & _env;
	};


} // namespace

#endif /// end define __BcastChFactory_h__
