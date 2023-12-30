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

};

class DODAppServiceImpl : public DODAppService {
public:
	
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

