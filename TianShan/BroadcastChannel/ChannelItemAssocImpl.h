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

// Branch: $Name:ChannelItemAssocImpl.h$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/ChannelItemAssocImpl.h $
// 
// 2     5/30/14 4:43p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#ifndef __ChannelItemAssocImpl_h__
#define __ChannelItemAssocImpl_h__

//#include <BcastChannelEx.h>
#include "BroadCastChannelEnv.h"
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>
namespace ZQBroadCastChannel
{ 
	class ChannelItemAssocImpl : public TianShanIce::Application::Broadcast::ChannelItemAssoc, public IceUtil::AbstractMutexI<IceUtil::Mutex>
	{
	public:
		ChannelItemAssocImpl(BroadCastChannelEnv& bcastChenv);
		~ChannelItemAssocImpl(void);

	public:

		virtual ::Ice::Int getCtrlNum(const Ice::Current&)const;

		virtual ::std::string getChannelItemKey(const Ice::Current&)const;

		virtual TianShanIce::Application::Broadcast::ChannelItemEx getChannelItem(const Ice::Current&)const;
        virtual TianShanIce::Application::Broadcast::BcastPublishPointExPrx getBcastPublishPointEx(const Ice::Current&)const;

		virtual TianShanIce::Application::Broadcast::ChannelItemAssocData getData(const ::Ice::Current&) const;

		virtual void destroy(const Ice::Current&);
	protected:
		ZQBroadCastChannel::BroadCastChannelEnv& _env;

	};
	typedef ::IceInternal::Handle< ChannelItemAssocImpl> ChannelItemAssocImplPtr;
}
#endif // end define __ChannelItemAssocImpl_h__
