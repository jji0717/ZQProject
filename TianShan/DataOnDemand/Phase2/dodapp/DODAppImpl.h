#ifndef __dodappI_h__
#define __dodappI_h__

#include <DODAppEx.h>
#include <Freeze/Freeze.h>
namespace DataOnDemand {

::Ice::Identity createDestinationIdentity(const std::string& name);
::Ice::Identity createChannelIdentity(const std::string& name);


class NameToDestination {
public:
	NameToDestination(const Ice::ObjectAdapterPtr& adapter) :
	  _adapter(adapter)
	  {

	  }

	  DataOnDemand::DestinationExPrx operator()(const std::string& name)
	  {
		  return DataOnDemand::DestinationExPrx::uncheckedCast(
			  _adapter->createProxy(createDestinationIdentity(name)));
	  }

private:

	const Ice::ObjectAdapterPtr _adapter;
};

class NameToChannel {
public:
	NameToChannel(const Ice::ObjectAdapterPtr& adapter) :
	  _adapter(adapter)
	  {

	  }

	  DataOnDemand::ChannelPublishPointPrx operator()(const std::string& name)
	  {
		  return DataOnDemand::ChannelPublishPointPrx::uncheckedCast(
			  _adapter->createProxy(createChannelIdentity(name)));
	  }

private:

	const Ice::ObjectAdapterPtr _adapter;
};

//////////////////////////////////////////////////////////////////////////

class DODPurchaseImpl : public DODPurchase {
public:

	DODPurchaseImpl();

	virtual ~DODPurchaseImpl();

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

class DODAppServiceImpl : public DODAppService {
public:
	DODAppServiceImpl();
	virtual ~DODAppServiceImpl();
	
     bool init();

	typedef ::IceInternal::Handle<DODAppServiceImpl> Ptr;

    virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::Application::PurchasePrx createPurchase(
		const ::TianShanIce::SRM::SessionPrx&, 
		const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());
public:
	::Ice::ObjectAdapterPtr _adapter;
	::TianShanIce::Application::PurchasePrx purchase;
	
};

// implemented
class DODAppFactory: public Ice::ObjectFactory {
	
public:
	virtual Ice::ObjectPtr create(const std::string& type);

	virtual void destroy()
	{
	}
};
}
#endif

