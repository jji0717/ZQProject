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

// Branch: $Name:BcastPurchaseImpl.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastPurchaseImpl.cpp $
// 
// 5     10/17/14 3:37p Li.huang
// 
// 4     10/15/14 4:23p Li.huang
// 
// 3     5/30/14 4:43p Li.huang
// 
// 2     4/23/14 2:28p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "BroadCastChannelEnv.h"
#include "BcastPurchaseImpl.h"

namespace ZQBroadCastChannel
{

BcastPurchaseImpl::BcastPurchaseImpl(BroadCastChannelEnv& bcastChenv):_env(bcastChenv)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPurchaseImpl, "construct Purchase object success"));
}

BcastPurchaseImpl::~BcastPurchaseImpl(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPurchaseImpl,"disconstruct Purchase object success"));
}

TianShanIce::SRM::SessionPrx
BcastPurchaseImpl::getSession(const Ice::Current& current)const
{
    throw TianShanIce::NotImplemented();
}

void
BcastPurchaseImpl::provision(const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPurchaseImpl, "provision success"));
}

void
BcastPurchaseImpl::render(const TianShanIce::Streamer::StreamPrx& stream,
											const TianShanIce::SRM::SessionPrx& sess,
											const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPurchaseImpl, "render success"));
}

void
BcastPurchaseImpl::detach(const ::std::string& sessId,
											const TianShanIce::Properties& params,
											const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPurchaseImpl, "detach success"));
}

void
BcastPurchaseImpl::bookmark(const ::std::string& title,
											  const TianShanIce::SRM::SessionPrx& sess,
											  const Ice::Current& current)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPurchaseImpl, "bookmark success"));
}

::Ice::Int
BcastPurchaseImpl::getParameters(const TianShanIce::StrValues& expectedParameter,
								const TianShanIce::ValueMap& inParams,
								TianShanIce::ValueMap& outParams,
								const Ice::Current& current) const
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastPurchaseImpl, "getParameters success"));
	return 0;
}

TianShanIce::Application::PlaylistInfo BcastPurchaseImpl::getPlaylistInfo(const ::Ice::Current& c) const
{
	TianShanIce::Application::PlaylistInfo plInfo;

#pragma message("TODO: Implement code to flush logs etc. in the face of an unhandled exception")
	ZQTianShan::_IceThrow<TianShanIce::NotSupported>(glog, "BcastPurchase", 500, "getPlaylistInfo() not implemented");
	return  plInfo;
}
}

