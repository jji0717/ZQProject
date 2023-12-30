// ChannelPublishPointImpl.cpp: implementation of the ChannelPublishPointImpl class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "global.h"

#include "ChannelPublishPointImpl.h"

#ifndef _CHANNEL_PUBLISHPOINT_IMPL_CPP_
#define _CHANNEL_PUBLISHPOINT_IMPL_CPP_

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

template <class Base>
DataOnDemand::ChannelPublishPointImpl<Base>::ChannelPublishPointImpl()
{

}

template <class Base>
DataOnDemand::ChannelPublishPointImpl<Base>::~ChannelPublishPointImpl()
{

}

template <class Base>
::std::string
DataOnDemand::ChannelPublishPointImpl<Base>::getName(
	const Ice::Current& current) const
{
    return myInfo.name;
}

template <class Base>
::DataOnDemand::ChannelType
DataOnDemand::ChannelPublishPointImpl<Base>::getType(
	const Ice::Current& current) const
{
    return myType;
}

template <class Base>
::DataOnDemand::ChannelInfo
DataOnDemand::ChannelPublishPointImpl<Base>::getInfo(
	const Ice::Current& current) const
{
    return myInfo;
}

template <class Base>
::TianShanIce::Properties
DataOnDemand::ChannelPublishPointImpl<Base>::getProperties(
	const Ice::Current& current) const
{
    return myProps;
}

template <class Base>
void
DataOnDemand::ChannelPublishPointImpl<Base>::setProperties(
						  const ::TianShanIce::Properties& props,
						  const Ice::Current& current)
{
	myProps = props;
}

template <class Base>
void
DataOnDemand::ChannelPublishPointImpl<Base>::destroy(const Ice::Current& current)
{
	DataOnDemand::DestLinks::iterator iter;
	::DataOnDemand::DestinationExPrx destinationprx;
    ChannelToDestAssoc destinfo;
	std::string destname;
	try
	{
		while(!myDestLinks.empty())
		{
			iter = myDestLinks.begin();
			destname = iter->first;
			destinfo = iter->second;
			destinationprx = destinfo.dest;
			try
			{	
				destinationprx->detachChannel(myInfo.name);	
			}
			catch (::TianShanIce::InvalidParameter* ex)
			{
				glog(ZQ::common::Log::L_DEBUG,
					"[channelname = %s]ChannelPublishPointImpl<Base>::destroy error"
					"::TianShanIce::InvalidParameter errorcode = %s",
					myInfo.name.c_str(), ex->ice_name().c_str());
			}	
		}
	}
	catch (...)
	{
		 	glog(ZQ::common::Log::L_DEBUG,
			"[channelname = %s]ChannelPublishPointImpl<Base>::destroy error",
			myInfo.name.c_str());
	}

	myParent->removeChannel(myInfo.name);
	activeChannelManager.remove(myInfo.name);

	glog(ZQ::common::Log::L_DEBUG,
		"[channelname = %s]ChannelPublishPointImpl<Base>::destroy !",
		myInfo.name.c_str());
}

template <class Base>
DataOnDemand::DestLinks 
DataOnDemand::ChannelPublishPointImpl<Base>::getDestLinks(const Ice::Current&)
{
	return myDestLinks;
}

template <class Base>
void
DataOnDemand::ChannelPublishPointImpl<Base>::linkDest(const ::std::string& name,
					 const ::DataOnDemand::DestinationExPrx& dest,
					 ::Ice::Long lastUpdate,
					 const Ice::Current& current)
{
    ChannelToDestAssoc channeltodestassoc;
	channeltodestassoc.destName = name;
	channeltodestassoc.dest = dest;
	channeltodestassoc.lastUpdate = lastUpdate;
	std::pair<DestLinks::iterator, bool> ir;
	ir = myDestLinks.insert(DestLinks::value_type(
		name, channeltodestassoc));
	if (!ir.second) {
		throw ObjectExistException();
	}
}

template <class Base>
void
DataOnDemand::ChannelPublishPointImpl<Base>::unlinkDest(const ::std::string& name,
					const Ice::Current& current)
{
	myDestLinks.erase(name);
}


template <class Base>
void 
DataOnDemand::ChannelPublishPointImpl<Base>::activate(
	const ::Ice::Current& current)
{
	activeChannelManager.create(getThisPrx(current.adapter));
}

template <class Base>
void 
DataOnDemand::ChannelPublishPointImpl<Base>::getCacheInfo(CacheType& type, 
													std::string& addres, 
													const Ice::Current& current)
{
	
}

template <class Base>
DataOnDemand::ChannelPublishPointPrx 
DataOnDemand::ChannelPublishPointImpl<Base>::getThisPrx(
	Ice::ObjectAdapterPtr adapter)
{
	if (_thisPrx == NULL) {
		_thisPrx = ChannelPublishPointPrx::uncheckedCast(
			adapter->createProxy(createChannelIdentity(myInfo.name)));
	}

	return _thisPrx;
}

template <class Base>
ActiveChannel* 
DataOnDemand::ChannelPublishPointImpl<Base>::getActiveChannel()
{
	ActiveChannel* activeChannel = activeChannelManager.get(myInfo.name);
	assert(activeChannel);

	return activeChannel;
}

template <class Base>
bool 
DataOnDemand::ChannelPublishPointImpl<Base>::init()
{
	return true;
}

#endif // #ifdef _CHANNEL_PUBLISHPOINT_IMPL_CPP_
