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

// Branch: $Name:BcastAppServiceImpl.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastAppServiceImpl.cpp $
// 
// 4     10/17/14 3:37p Li.huang
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
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "BroadCastChannelEnv.h"
#include "BcastPurchaseImpl.h"
#include "BcastAppServiceImpl.h"

namespace ZQBroadCastChannel
{ 

BcastAppServiceImpl::BcastAppServiceImpl(BroadCastChannelEnv& bcastChenv):
_env(bcastChenv)
{
   
}

BcastAppServiceImpl::~BcastAppServiceImpl(void)
{

}
bool BcastAppServiceImpl::init()
{
	Ice::ObjectPrx objprx;
	try
	{	
		Ice::Identity ident;
		{
			ident.category = ICE_BcastChannelPurchase;
			ident.name = "BcastChannelPurchase";		
			if(_env._evitPurchase->hasObject(ident) == NULL)
			{	
				BcastPurchaseImpl* bcastpurchase = new BcastPurchaseImpl(_env);

				objprx = _env._evitPurchase->add(bcastpurchase, ident);
				_purchase = TianShanIce::Application::PurchasePrx::uncheckedCast(objprx);
			}  
			else
			{
				objprx = _env._adapter->createProxy(ident);	
				_purchase = TianShanIce::Application::PurchasePrx::checkedCast(objprx);
			}		
			if(_purchase == NULL)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastAppServiceImpl,
					"init() Invaild BcastChannelPurchase proxy"));
				return false;
			}
		}
	}
	catch (const ::Ice::Exception & ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastAppServiceImpl,
			"init() caught ice exception '%s'"), 
			ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastAppServiceImpl,
			"init() caught unknown exception (%d)"), SYS::getLastErr());
		return false;
	}
	return true;
}
TianShanIce::Application::PurchasePrx
BcastAppServiceImpl::createPurchase(const TianShanIce::SRM::SessionPrx& weiSession,
													  const TianShanIce::Properties& siteProperties,
													  const Ice::Current& current)
{
	std::string broadcastPP ="";
	try
	{
		TianShanIce::ValueMap::iterator itorVmap;
		TianShanIce::ValueMap vmap = weiSession->getPrivateData();

		if(vmap.size() > 0)
		{
          itorVmap = vmap.find(RESKEY_BcastPPName);
		  if(itorVmap != vmap.end())
		  {
			TianShanIce::Variant&var = itorVmap->second;
			 broadcastPP = var.strs[0];
		  }
		}		   
	}
	catch(Ice::Exception&ex)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastAppServiceImpl, 
			"fail to get broadcast datapublish name, ice exception '%s'"),ex.ice_name().c_str());
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastAppServiceImpl, "[%s]create Purchase object successfully"), broadcastPP.c_str());
	return _purchase;
}

::std::string 
BcastAppServiceImpl::getAdminUri(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented();
}
TianShanIce::State 
BcastAppServiceImpl::getState(const ::Ice::Current& )
{	
	return TianShanIce::stInService;
}

}

