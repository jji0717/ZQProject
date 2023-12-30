#ifndef __dodappI_h__
#define __dodappI_h__

#include <DataAppEx.h>
#include <Freeze/Freeze.h>
#include "TianShanDefines.h"
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {

::Ice::Identity createDataStreamIdentity(const std::string& name);
::Ice::Identity createDataPublishPointIdentity(const std::string& name);


class NameToDataStream {
public:
	NameToDataStream(const Ice::ObjectAdapterPtr& adapter) :
	  _adapter(adapter)
	  {

	  }

	  TianShanIce::Application::DataOnDemand::DataStreamExPrx operator()(const std::string& name)
	  {
		  return TianShanIce::Application::DataOnDemand::DataStreamExPrx::uncheckedCast(
			  _adapter->createProxy(createDataStreamIdentity(name)));
	  }

private:

	const Ice::ObjectAdapterPtr _adapter;
};

class NameToDataPublishPoint {
public:
	NameToDataPublishPoint(const Ice::ObjectAdapterPtr& adapter) :
	  _adapter(adapter)
	  {

	  }

	  TianShanIce::Application::DataOnDemand::DataPublishPointPrx operator()(const std::string& name)
	  {
		  return TianShanIce::Application::DataOnDemand::DataPublishPointPrx::uncheckedCast(
			  _adapter->createProxy(createDataPublishPointIdentity(name)));
	  }

private:

	const Ice::ObjectAdapterPtr _adapter;
};

//////////////////////////////////////////////////////////////////////////

class DataTunnelPurchaseImpl : public DataTunnelPurchase , public IceUtil::AbstractMutexI<IceUtil::RecMutex>{
public:

	DataTunnelPurchaseImpl();

	virtual ~DataTunnelPurchaseImpl();

    virtual ::TianShanIce::SRM::SessionPrx getSession(
								const ::Ice::Current& = ::Ice::Current()) const;

    virtual void provision(const ::Ice::Current& = ::Ice::Current());

    virtual void render(const ::TianShanIce::Streamer::StreamPrx&,
								const ::TianShanIce::SRM::SessionPrx&, 
								const ::Ice::Current& = ::Ice::Current());
	
	virtual void detach(const ::std::string&, const ::TianShanIce::Properties&, 
							const ::Ice::Current& = ::Ice::Current());


	virtual void bookmark(const ::std::string&, 
								const ::TianShanIce::SRM::SessionPrx&,
								const ::Ice::Current& = ::Ice::Current());

    virtual ::Ice::Int getParameters(const ::TianShanIce::StrValues&, 
										const ::TianShanIce::ValueMap&, 
										::TianShanIce::ValueMap&, 
								const ::Ice::Current& = ::Ice::Current()) const;

	virtual void destroy(const ::Ice::Current& = ::Ice::Current());
};

class DataTunnelServiceImpl : public DataTunnelService {
public:
	DataTunnelServiceImpl(ZQADAPTER_DECLTYPE& adapter, Ice::CommunicatorPtr& ic,
		::Freeze::EvictorPtr& evictor);
	virtual ~DataTunnelServiceImpl();
	
     bool init();

	

    virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::Application::PurchasePrx createPurchase(
		const ::TianShanIce::SRM::SessionPrx&, 
		const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());
public:
	::Freeze::EvictorPtr&	_evictor;
	ZQADAPTER_DECLTYPE& _adapter;
	Ice::CommunicatorPtr& _ic;
	::TianShanIce::Application::PurchasePrx _purchase;
	
};
typedef ::IceInternal::Handle<DataTunnelServiceImpl> DataTunnelServiceImplPtr;
// implemented
class DataTunnelAppFactory: public Ice::ObjectFactory {
	
public:
	virtual Ice::ObjectPtr create(const std::string& type);

	virtual void destroy()
	{
	}
};
}  /// end namespace DataOnDemand {
} /// end namespace Application 
} /// end namespace TianshanIce 
#endif

