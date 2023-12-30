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

// Branch: $Name:BcastPurchaseImpl.h$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastPurchaseImpl.h $
// 
// 4     5/30/14 4:43p Li.huang
// 
// 3     5/23/14 2:35p Li.huang
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
#ifndef __BcastPurchaseImpl_h__
#define __BcastPurchaseImpl_h__

#include <TsAppBcast.h>
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>

namespace ZQBroadCastChannel
{ 
class BroadCastChannelEnv;

class BcastPurchaseImpl : public TianShanIce::Application::Broadcast::BcastPurchase,
          public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	BcastPurchaseImpl(BroadCastChannelEnv& bcastChenv);
	~BcastPurchaseImpl();
public:

	//implement purchase interface
	virtual TianShanIce::SRM::SessionPrx getSession(const Ice::Current&)const;

	virtual void provision(const Ice::Current&);

	virtual void render(const TianShanIce::Streamer::StreamPrx&,
		const TianShanIce::SRM::SessionPrx&,
		const Ice::Current&);

	virtual void detach(const ::std::string&,
		const TianShanIce::Properties&,
		const Ice::Current&);

	virtual void bookmark(const ::std::string&,
		const TianShanIce::SRM::SessionPrx&,
		const Ice::Current&);

	virtual ::Ice::Int getParameters(const TianShanIce::StrValues&,
		const TianShanIce::ValueMap&,
		TianShanIce::ValueMap&,
		const Ice::Current&)const;

	TianShanIce::Application::PlaylistInfo getPlaylistInfo(const ::Ice::Current& c) const;

protected:
	BroadCastChannelEnv& _env;
};
}
#endif ///end define  __BcastPurchaseImpl_h__
