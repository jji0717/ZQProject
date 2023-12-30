// PurchaseItemImpl.h: interface for the PurchaseItemImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PURCHASEITEMIMPL_H__E768F297_36CD_4755_9C87_BC5B22CAE09C__INCLUDED_)
#define AFX_PURCHASEITEMIMPL_H__E768F297_36CD_4755_9C87_BC5B22CAE09C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChODDefines.h"
#ifdef USE_OLD_NS
#	include "ChannelOnDemand.h"
#else
#	include "TsAppChOD.h"
#endif
#include "ChannelItemDict.h"
#include "ChODSvcEnv.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace ZQChannelOnDemand {

class PurchaseItemImpl : public NS_PREFIX(ChannelOnDemand::PurchaseItemAssoc), public IceUtil::AbstractMutexI<IceUtil::Mutex>  
{
public:
	PurchaseItemImpl(ChODSvcEnv& env);
	virtual ~PurchaseItemImpl();

    virtual ::Ice::Int getCtrlNum(const ::Ice::Current& = ::Ice::Current()) const;

    virtual NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx) getPurchase(const ::Ice::Current& = ::Ice::Current()) const;

    virtual ::std::string getChannelItemKey(const ::Ice::Current& = ::Ice::Current()) const;

    virtual NS_PREFIX(ChannelOnDemand::ChannelItemEx) getChannelItem(const ::Ice::Current& = ::Ice::Current()) const;

    virtual void destroy(const ::Ice::Current& = ::Ice::Current());

	virtual NS_PREFIX(ChannelOnDemand::PurchaseItemData) getData(const ::Ice::Current& = ::Ice::Current()) const;

protected:
	ChODSvcEnv& _env;

};

typedef ::IceInternal::Handle< PurchaseItemImpl> PurchaseItemImplPtr;

}

#endif // !defined(AFX_PURCHASEITEMIMPL_H__E768F297_36CD_4755_9C87_BC5B22CAE09C__INCLUDED_)
