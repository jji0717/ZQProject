#include "stdafx.h"
#include <DataAppImpl.h>
#include <string.h>
#include "DataPointPublisherImpl.h"
#include "DataStreamExImpl.h"
#include "FolderExImpl.h"
#include "MessageQueueExImpl.h"

//////////////////////////////////////////////////////////////////////////
using ZQ::common::Log;
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {
	DataTunnelServiceImpl::DataTunnelServiceImpl(ZQADAPTER_DECLTYPE& adapter, Ice::CommunicatorPtr& ic,
		::Freeze::EvictorPtr& evictor):
_adapter(adapter), _ic(ic),_evictor(evictor)
{
}
bool
DataTunnelServiceImpl::init()
{
	Ice::Identity ident;
	ident.category = Servant_DataTunnel;
	ident.name = "DataTunnelPurchase";

	Ice::ObjectPrx objprx;
	try
	{			
		if(_evictor->hasObject(ident) == NULL)
		{	
			DataTunnelPurchaseImpl* dodpurptr = 
				new DataTunnelPurchaseImpl();
			objprx = _evictor->add(dodpurptr, ident);
			_purchase = ::TianShanIce::Application::PurchasePrx::uncheckedCast(objprx);
		}  
		else
		{
			objprx = _adapter->createProxy(ident);	
           _purchase = ::TianShanIce::Application::PurchasePrx::checkedCast(objprx);
		}		
		if(_purchase == NULL)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelServiceImpl,
				"init() Invaild DataTunnelPurchase proxy"));
			return false;
		}
	}
	catch (const ::Ice::Exception & ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelServiceImpl,
			"init() caught ice exception '%s'"), 
			ex.ice_name().c_str());
		return false;
	}
    catch(...)
	{
      glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelServiceImpl,
		  "init() caught unknown exception (%d)"), GetLastError());
	  return false;
	}
	return true;
}
DataTunnelServiceImpl::~DataTunnelServiceImpl()
{
    glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelServiceImpl,
		"~DataTunnelServiceImpl()"));
}
::TianShanIce::Application::PurchasePrx 
DataTunnelServiceImpl::createPurchase(
							 const ::TianShanIce::SRM::SessionPrx&,
							 const ::TianShanIce::Properties&,
							 const ::Ice::Current&)
{  
//	glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelServiceImpl,"create Purchase  object success"));
	return _purchase;
}

::std::string 
DataTunnelServiceImpl::getAdminUri(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented();
}
::TianShanIce::State 
DataTunnelServiceImpl::getState(const ::Ice::Current& )
{	
	return ::TianShanIce::stInService;
}
//////////////////////////////////////////////////////////////////////////

DataTunnelPurchaseImpl::DataTunnelPurchaseImpl()

{
//	glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelPurchaseImpl,"construct Purchase object success"));
}

DataTunnelPurchaseImpl::~DataTunnelPurchaseImpl()
{
//	glog(ZQ::common::Log::L_INFO,CLOGFMT(DataTunnelPurchaseImpl,"disconstruct Purchase object success"));
}


void
DataTunnelPurchaseImpl::render(const ::TianShanIce::Streamer::StreamPrx& sPrx,
						const ::TianShanIce::SRM::SessionPrx& sessionPrx,
						const ::Ice::Current&)
{
//   	glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelPurchaseImpl, "render success"));
}

void 
DataTunnelPurchaseImpl::destroy(const ::Ice::Current&)
{
//   	glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelPurchaseImpl,"destroy success"));
}

::TianShanIce::SRM::SessionPrx 
DataTunnelPurchaseImpl::getSession(const ::Ice::Current&) const
{
	::TianShanIce::SRM::SessionPrx sPrx;

	throw ::TianShanIce::NotImplemented();

	return sPrx;
}

void 
DataTunnelPurchaseImpl::provision(const ::Ice::Current&)
{
//	glog(ZQ::common::Log::L_INFO,  CLOGFMT(DataTunnelPurchaseImpl,"provision success"));
}

void 
DataTunnelPurchaseImpl::detach(const ::std::string&, 
									  const ::TianShanIce::Properties&, 
									const ::Ice::Current&)
{
//	glog(ZQ::common::Log::L_INFO,  CLOGFMT(DataTunnelPurchaseImpl,"detach success"));
}

void 
DataTunnelPurchaseImpl::bookmark(const ::std::string& title, 
							   const ::TianShanIce::SRM::SessionPrx& sess, 
							   const ::Ice::Current& c)
{
//	glog(ZQ::common::Log::L_INFO,  CLOGFMT(DataTunnelPurchaseImpl,"bookmark success"));
}
::Ice::Int 
DataTunnelPurchaseImpl::getParameters(const ::TianShanIce::StrValues&, 
								 const ::TianShanIce::ValueMap&, 
								 ::TianShanIce::ValueMap&, 
								 const ::Ice::Current& c) const
{
	glog(ZQ::common::Log::L_INFO,  CLOGFMT(DataTunnelPurchaseImpl, "getParameters() success"));
	
//   throw ::TianShanIce::NotImplemented();
	
	return 0;
}
//////////////////////////////////////////////////////////////////////////
::Ice::Identity 
createDataStreamIdentity(const std::string& name)
{
	Ice::Identity ident;
	ident.category = Servant_DataTunnel;
	ident.name = name;
	return ident;
}

::Ice::Identity 
createDataPublishPointIdentity(const std::string& name)
{
	Ice::Identity ident;
	ident.category = Servant_DataTunnel;
	ident.name = name;
	return ident;
}

//////////////////////////////////////////////////////////////////////////

Ice::ObjectPtr 
DataTunnelAppFactory::create(const std::string& type)
{
	if (type == TianShanIce::Application::DataOnDemand::DataPointPublisherEx::ice_staticId()) {

		return new TianShanIce::Application::DataOnDemand::DataPointPublisherImpl();

	} else if (type == TianShanIce::Application::DataOnDemand::DataStreamEx::ice_staticId()) {

		return new TianShanIce::Application::DataOnDemand::DataStreamExImpl();

	} else if (type == TianShanIce::Application::DataOnDemand::FolderEx::ice_staticId()) {

		return new TianShanIce::Application::DataOnDemand::FolderExImpl();

	} else if (type == TianShanIce::Application::DataOnDemand::MessageQueueEx::ice_staticId()){

		return new TianShanIce::Application::DataOnDemand::MessageQueueExImpl();
	}
	else if(type ==TianShanIce::Application::DataOnDemand::DataTunnelPurchase::ice_staticId())
	{
		return new 	TianShanIce::Application::DataOnDemand::DataTunnelPurchaseImpl;

	}

	assert(false);
	return NULL;
}
}  /// end namespace DataOnDemand {
} /// end namespace Application 
} /// end namespace TianshanIce 
