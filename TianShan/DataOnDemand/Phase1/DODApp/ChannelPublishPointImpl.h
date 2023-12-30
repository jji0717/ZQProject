// ChannelPublishPointImpl.h: interface for the ChannelPublishPointImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELPUBLISHPOINTIMPL_H__6ED6596D_044F_4EFE_9168_3B41ACBB9134__INCLUDED_)
#define AFX_CHANNELPUBLISHPOINTIMPL_H__6ED6596D_044F_4EFE_9168_3B41ACBB9134__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <DODAppEx.h>
#include <Freeze/Freeze.h>
#include "DODAppImpl.h"
#include "Util.h"
#include <list>

class ActiveChannel;

namespace DataOnDemand {

template <class Base>
class ChannelPublishPointImpl : public Base	{
public:

	ChannelPublishPointImpl();
	virtual ~ChannelPublishPointImpl();

    virtual ::std::string getName(const Ice::Current&) const;

    virtual ChannelType getType(const Ice::Current&) const;

    virtual ChannelInfo getInfo(const Ice::Current&) const;

    virtual ::TianShanIce::Properties getProperties(const Ice::Current&) const;

    virtual void setProperties(const ::TianShanIce::Properties&,
			       const Ice::Current&);

	virtual void destroy(const Ice::Current&);

	virtual void getCacheInfo(CacheType& type, std::string& addres, 
		const Ice::Current&);
	
	DestLinks getDestLinks(const Ice::Current&);

	virtual void linkDest(const ::std::string&,
			  const DestinationExPrx&,
			  ::Ice::Long,
			  const Ice::Current&);

    virtual void unlinkDest(const ::std::string&,
			    const Ice::Current&);
	virtual void activate(const ::Ice::Current& );
	
public:
	bool init();
	
protected:
	ChannelPublishPointPrx getThisPrx(Ice::ObjectAdapterPtr adapter);
	ActiveChannel* getActiveChannel();

protected:
	ChannelPublishPointPrx	_thisPrx;
};

} // namespace DataOnDemand {

#include "ChannelPublishPointImpl.cpp"

#endif // !defined(AFX_CHANNELPUBLISHPOINTIMPL_H__6ED6596D_044F_4EFE_9168_3B41ACBB9134__INCLUDED_)
