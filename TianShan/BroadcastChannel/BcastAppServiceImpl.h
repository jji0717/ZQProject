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

// Branch: $Name:BcastAppServiceImpl.h$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastAppServiceImpl.h $
// 
// 3     5/30/14 4:43p Li.huang
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
#ifndef __BcastAppServiceImpl_h__
#define __BcastAppServiceImpl_h__

#include "TsAppBcast.h"

namespace ZQBroadCastChannel
{ 
class BroadCastChannelEnv;
class BcastAppServiceImpl : public TianShanIce::Application::AppService
{
public:
	typedef ::IceInternal::Handle< BcastAppServiceImpl> Ptr;
	BcastAppServiceImpl(BroadCastChannelEnv& bcastChenv);
	~BcastAppServiceImpl(void);
public:
	virtual TianShanIce::Application::PurchasePrx createPurchase(const TianShanIce::SRM::SessionPrx&,
		const TianShanIce::Properties&,
		const Ice::Current&);
	bool init();

	virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());

	virtual TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());
protected:
	 BroadCastChannelEnv& _env;
	TianShanIce::Application::PurchasePrx  _purchase;
};
}
#endif	///endif __BcastAppServiceImpl_h__
