// #include "PurchaseImpl.h"
#include "Environment.h"

PurchaseImpl::PurchaseImpl(void)
{
}

PurchaseImpl::~PurchaseImpl()
{
}

::TianShanIce::SRM::SessionPrx PurchaseImpl::getSession(const ::Ice::Current&) const
{
	glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::getSession()");
	return NULL;
}

void PurchaseImpl::provision(const ::Ice::Current&)
{
	glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::provision()");
}

void PurchaseImpl::render(const ::TianShanIce::Streamer::StreamPrx& sPrx, const ::TianShanIce::SRM::SessionPrx& sessionPrx, const ::Ice::Current&)
{
	glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::render()");
}

void PurchaseImpl::detach(const ::std::string& sessId, const ::TianShanIce::Properties& params, const ::Ice::Current&c)
{
	glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::detach()");
}

void PurchaseImpl::bookmark(const ::std::string& title, const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c)
{
	glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::bookmark()");
}

::Ice::Int PurchaseImpl::getParameters(const ::TianShanIce::StrValues& params, const ::TianShanIce::ValueMap& vMap_in, ::TianShanIce::ValueMap& vMap_out, const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::getParameters()");
	return 1;
}