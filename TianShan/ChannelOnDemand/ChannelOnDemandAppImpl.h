// ChannelOnDemandAppImpl.h: interface for the ChannelOnDemandAppImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELONDEMANDAPPIMPL_H__2B67B240_C078_4B73_A44C_8FF7BD63B1D6__INCLUDED_)
#define AFX_CHANNELONDEMANDAPPIMPL_H__2B67B240_C078_4B73_A44C_8FF7BD63B1D6__INCLUDED_

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


#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace ZQChannelOnDemand {

class ChODSvcEnv;

class ChannelOnDemandAppImpl  : public NS_PREFIX(ChannelOnDemand::ChannelOnDemandApp)
{
public:
	ChannelOnDemandAppImpl(ChODSvcEnv& env);
	virtual ~ChannelOnDemandAppImpl();

	typedef ::IceInternal::Handle< ChannelOnDemandAppImpl> Ptr;

    virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::Application::PurchasePrx createPurchase(const ::TianShanIce::SRM::SessionPrx&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());

	virtual NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx) createPurchaseByCR(const ::TianShanIce::Properties&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());

protected:
	ChODSvcEnv& _env;


};

}
#endif // !defined(AFX_CHANNELONDEMANDAPPIMPL_H__2B67B240_C078_4B73_A44C_8FF7BD63B1D6__INCLUDED_)
