#pragma once
#include <TsApplication.h>
#include "Environment.h"

class HsnTreeAppImpl  : public TianShanIce::Application::AppService
{
public:
	HsnTreeAppImpl(HSNTree::Environment& env);
	virtual ~HsnTreeAppImpl();

	typedef ::IceInternal::Handle<HsnTreeAppImpl> Ptr;

	virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Application::PurchasePrx createPurchase(const ::TianShanIce::SRM::SessionPrx&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());

public:
//	static ::TianShanIce::Application::PurchasePrx purPrx;
	HSNTree::Environment& _env;
};
typedef HsnTreeAppImpl::Ptr HsnTreeAppImplPtr;