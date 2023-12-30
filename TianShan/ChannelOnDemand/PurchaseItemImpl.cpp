// PurchaseItemImpl.cpp: implementation of the PurchaseItemImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PurchaseItemImpl.h"


#define LOG_MODULE_NAME			"PurchItem"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace ZQChannelOnDemand {


PurchaseItemImpl::PurchaseItemImpl(ChODSvcEnv& env)
	: _env(env)
{

}

PurchaseItemImpl::~PurchaseItemImpl()
{

}

::Ice::Int PurchaseItemImpl::getCtrlNum(const ::Ice::Current&) const
{
	Lock lock(*this);

	return playlistCtrlNum;
}

::std::string PurchaseItemImpl::getChannelItemKey(const ::Ice::Current& c) const
{
	Lock lock(*this);

	return channelItemKey;		
}

NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx) PurchaseItemImpl::getPurchase(const ::Ice::Current&) const
{
	Lock lock(*this);
	
	NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx) purchasePrx;
	
	Ice::ObjectPrx prx = _env._adapter->createProxy(purchaseIdent);
	if (!prx)
	{
		return NULL;
	}

	purchasePrx = NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx)::uncheckedCast(prx);

	return purchasePrx;
}

NS_PREFIX(ChannelOnDemand::ChannelItemEx) PurchaseItemImpl::getChannelItem(const ::Ice::Current&) const
{
	Lock lock(*this);

	LockT<RecMutex> lk(_env._dictLock);
	::ChannelOnDemand::ChannelItemDict::iterator it = _env._pChannelItemDict->find(channelItemKey);
	if ( it == _env._pChannelItemDict->end())
	{
		::TianShanIce::ServerError ex;
		ex.message = "Failed to find channel item " + channelItemKey;
		throw ex;
	}

	return it->second;	
}

void PurchaseItemImpl::destroy(const ::Ice::Current&)
{
	try
	{
//		LockT<RecMutex> lk(_env._evitPITLock);
		_env._evitPurchaseItemAssoc->remove(ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "remove purchase item caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}
}

NS_PREFIX(ChannelOnDemand::PurchaseItemData) PurchaseItemImpl::getData(const ::Ice::Current& c) const
{
	NS_PREFIX(ChannelOnDemand::PurchaseItemData) data;
	data.ident = ident;
	data.purchaseIdent = purchaseIdent;
	data.channelItemKey = channelItemKey;
	data.playlistCtrlNum = playlistCtrlNum;
	data.lastModified = lastModified;
	return data;
}

}