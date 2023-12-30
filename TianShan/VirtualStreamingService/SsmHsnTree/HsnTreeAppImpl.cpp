#include "HsnTreeAppImpl.h"

//::TianShanIce::Application::PurchasePrx HsnTreeAppImpl::purPrx = NULL;

HsnTreeAppImpl::HsnTreeAppImpl(HSNTree::Environment& env)
: _env(env)
{
}

HsnTreeAppImpl::~HsnTreeAppImpl()
{
}

::TianShanIce::Application::PurchasePrx HsnTreeAppImpl::createPurchase(const ::TianShanIce::SRM::SessionPrx& weiSession, const ::TianShanIce::Properties&, const ::Ice::Current&)
{
//	return HsnTreeAppImpl::purPrx;
	::TianShanIce::Application::PurchasePrx purPrx = NULL;
	char _szBuf[1024];
	try{
		Ice::Identity ident;
		ident.category = ServantType;

		::TianShanIce::ValueMap privData;
		privData = weiSession->getPrivateData();
		TianShanIce::ValueMap::iterator vMap_itor;
		vMap_itor = (privData).find(ClientRequestPrefix ClientSessionID);
		if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
			ident.name = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

		HSNTree::SessionContextPrx cltSessPrx = HSNTree::SessionContextPrx::checkedCast(_env._pAdapter->createProxy(ident));
		purPrx = ::TianShanIce::Application::PurchasePrx::checkedCast(cltSessPrx);
	}
	catch (Ice::ObjectNotExistException& ex)
	{
		snprintf(_szBuf, sizeof(_szBuf) - 1, "HsnTreeAppImpl createPurchase() caught %s ", ex.ice_name().c_str());
		glog(ZQ::common::Log::L_ERROR, "%s", _szBuf);
	}
	catch (Ice::Exception& ex)
	{
		snprintf(_szBuf, sizeof(_szBuf) - 1, "HsnTreeAppImpl createPurchase() caught %s", ex.ice_name().c_str());
		glog(ZQ::common::Log::L_ERROR, "%s", _szBuf);
	}
	catch(...)
	{
		snprintf(_szBuf, sizeof(_szBuf) - 1, "HsnTreeAppImpl createPurchase() caught unknown error");
		glog(ZQ::common::Log::L_ERROR, "%s", _szBuf);
	}
	return purPrx;
}

::std::string HsnTreeAppImpl::getAdminUri(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented();
}

::TianShanIce::State HsnTreeAppImpl::getState(const ::Ice::Current& )
{	
	return ::TianShanIce::stInService;
}