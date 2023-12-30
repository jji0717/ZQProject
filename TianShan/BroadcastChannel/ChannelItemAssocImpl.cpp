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

// Branch: $Name:ChannelItemAssocImpl.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/ChannelItemAssocImpl.cpp $
// 
// 3     10/17/14 3:37p Li.huang
// 
// 2     5/30/14 4:43p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     10-03-19 11:46 Li.huang
// remove evictor lock
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "ChannelItemAssocImpl.h"

namespace ZQBroadCastChannel
{ 
ChannelItemAssocImpl::ChannelItemAssocImpl(BroadCastChannelEnv& bcastChenv):
_env(bcastChenv)
{
}

ChannelItemAssocImpl::~ChannelItemAssocImpl(void)
{
}

::Ice::Int
ChannelItemAssocImpl::getCtrlNum(const Ice::Current& current)const
{
	Lock lock(*this);

	return playlistCtrlNum;
}

::std::string
ChannelItemAssocImpl::getChannelItemKey(const Ice::Current& current)const
{
	Lock lock(*this);

	return channelItemKey;	
}

TianShanIce::Application::Broadcast::ChannelItemEx
ChannelItemAssocImpl::getChannelItem(const Ice::Current& current)const
{
	Lock lock(*this);

	LockT<RecMutex> lk(_env._dictLock);
	ChannelItemDict::iterator it = _env._pChannelItemDict->find(channelItemKey);
	if ( it == _env._pChannelItemDict->end())
	{
		TianShanIce::ServerError ex;
		ex.message = "Failed to find channel item " + channelItemKey;
		throw ex;
	}

	return it->second;	
}
TianShanIce::Application::Broadcast::BcastPublishPointExPrx
ChannelItemAssocImpl::getBcastPublishPointEx(const Ice::Current& current)const
{
	Lock lock(*this);

	TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcastPPPrx;

	Ice::ObjectPrx prx = _env._adapter->createProxy(bcastPPIdent);
	if (!prx)
	{
		return NULL;
	}

	bcastPPPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(prx);

	return bcastPPPrx;
}
void
ChannelItemAssocImpl::destroy(const Ice::Current& current)
{
	try
	{
		_env._evitChannelItemAssoc->remove(ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ChannelItemAssoc, "remove channel item assoc caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}
}
TianShanIce::Application::Broadcast::ChannelItemAssocData
ChannelItemAssocImpl::getData(const ::Ice::Current& c) const
{
	TianShanIce::Application::Broadcast::ChannelItemAssocData data;
	data.ident = ident;
	data.bcastPPIdent = bcastPPIdent;
	data.channelItemKey = channelItemKey;
	data.playlistCtrlNum = playlistCtrlNum;
	data.lastModified = lastModified;
	return data;
}
}
