#include "stdafx.h"
#include <DODAppImpl.h>
#include <string.h>
#include "DataPublisherImpl.h"
#include "DestinationImpl.h"
#include "FolderChannelImpl.h"
#include "MessageChannelImpl.h"

//////////////////////////////////////////////////////////////////////////
using ZQ::common::Log;

DataOnDemand::DODAppServiceImpl::DODAppServiceImpl()
{
}
bool
DataOnDemand::DODAppServiceImpl::init()
{
	Ice::Identity ident;
	ident.category = "DODPurchase";
	ident.name = "DataOndemand_DODApp";
	try
	{
		DataOnDemand::DODPurchasePtr dodpurptr = 
			new DODPurchaseImpl();
		_adapter->add(dodpurptr, ident);
		
		purchase = DataOnDemand::DODPurchasePrx::uncheckedCast(
			_adapter->createProxy(ident));
	}
	catch (const ::Ice::Exception & ex)
	{
		glog(ZQ::common::Log::L_ERROR, 
			"DODAppServiceImpl::init() Ice::Exceptionerrorcode = %s", 
			ex.ice_name().c_str());
		return false;
	}
    catch(...)
	{
      glog(ZQ::common::Log::L_ERROR, "DODAppServiceImpl::init() error");
	  return false;
	}
	return true;
}
DataOnDemand::DODAppServiceImpl::~DODAppServiceImpl()
{
    glog(ZQ::common::Log::L_INFO, "DODAppServiceImpl::~DODAppServiceImpl()");
}
::TianShanIce::Application::PurchasePrx 
 DataOnDemand::DODAppServiceImpl::createPurchase(
							 const ::TianShanIce::SRM::SessionPrx&,
							 const ::TianShanIce::Properties&,
							 const ::Ice::Current&)
{  
	glog(ZQ::common::Log::L_INFO, "create Purchase  object success");

	return purchase;
}

::std::string 
DataOnDemand::DODAppServiceImpl::getAdminUri(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented();
}
::TianShanIce::State 
DataOnDemand::DODAppServiceImpl::getState(const ::Ice::Current& )
{	
	return ::TianShanIce::stInService;
}
//////////////////////////////////////////////////////////////////////////

DataOnDemand::DODPurchaseImpl::DODPurchaseImpl()

{
	glog(ZQ::common::Log::L_INFO, "Construct Purchase object success");
}

DataOnDemand::DODPurchaseImpl::~DODPurchaseImpl()
{
	glog(ZQ::common::Log::L_INFO, "Disconstruct Purchase object success");
}


void
 DataOnDemand::DODPurchaseImpl::render(const ::TianShanIce::Streamer::StreamPrx& sPrx,
						const ::TianShanIce::SRM::SessionPrx& sessionPrx,
						const ::Ice::Current&)
{
   	glog(ZQ::common::Log::L_INFO, "DODPurchaseImpl::render success");
}

void 
DataOnDemand::DODPurchaseImpl::destroy(const ::Ice::Current&)
{
   	glog(ZQ::common::Log::L_INFO, "DODPurchaseImpl::destroy success");
}

::TianShanIce::SRM::SessionPrx 
DataOnDemand::DODPurchaseImpl::getSession(const ::Ice::Current&) const
{
	::TianShanIce::SRM::SessionPrx sPrx;

	throw ::TianShanIce::NotImplemented();

	return sPrx;
}

void 
DataOnDemand::DODPurchaseImpl::provision(const ::Ice::Current&)
{
	glog(ZQ::common::Log::L_INFO, "DODPurchaseImpl::provision success");
}

void 
DataOnDemand::DODPurchaseImpl::detach(const ::std::string&, 
									  const ::TianShanIce::Properties&, 
									const ::Ice::Current&)
{
	glog(ZQ::common::Log::L_INFO, "DODPurchaseImpl::detach success");
}

void 
DataOnDemand::DODPurchaseImpl::bookmark(const ::std::string& title, 
							   const ::TianShanIce::SRM::SessionPrx& sess, 
							   const ::Ice::Current& c)
{
	glog(ZQ::common::Log::L_INFO, "DODPurchaseImpl::bookmark success");

}
::Ice::Int 
DataOnDemand::DODPurchaseImpl::getParameters(const ::TianShanIce::StrValues&, 
								 const ::TianShanIce::ValueMap&, 
								 ::TianShanIce::ValueMap&, 
								 const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_INFO, "DODPurchaseImpl::getParameters() success");
	
//   	throw ::TianShanIce::NotImplemented();
	
	return 0;
}
//////////////////////////////////////////////////////////////////////////
::Ice::Identity 
DataOnDemand::createDestinationIdentity(const std::string& name)
{
	Ice::Identity ident;
	ident.category = "Destination";
	ident.name = name;
	return ident;
}

::Ice::Identity 
DataOnDemand::createChannelIdentity(const std::string& name)
{
	Ice::Identity ident;
	ident.category = "Channel";
	ident.name = name;
	return ident;
}

//////////////////////////////////////////////////////////////////////////

Ice::ObjectPtr 
DataOnDemand::DODAppFactory::create(const std::string& type)
{
	if (type == "::DataOnDemand::DataPublisherEx") {

		return new ::DataOnDemand::DataPublisherImpl;

	} else if (type == "::DataOnDemand::DestinationEx") {

		return new ::DataOnDemand::DestinationImpl;

	} else if (type == "::DataOnDemand::FolderChannelEx") {

		return new ::DataOnDemand::FolderChannelImpl;

	} else if (type == "::DataOnDemand::MessageChannelEx") {

		return new ::DataOnDemand::MessageChannelImpl;

	}

	assert(false);
	return NULL;
}
