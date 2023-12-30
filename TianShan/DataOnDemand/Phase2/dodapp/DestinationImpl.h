#pragma once

#include <DODAppEx.h>
#include <Freeze/Freeze.h>
#include "DODAppImpl.h"
#include "DataStream.h"
#include "Locks.h"
namespace DataOnDemand {

class DestinationImpl : public DestinationEx , 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
public:
	DestinationImpl();

    ~DestinationImpl();

	typedef ::IceInternal::Handle<DestinationImpl>DestinationImplPtr;
	
    virtual ::std::string getName(const Ice::Current&) const;

    virtual DestState getState(const Ice::Current&) const;

    virtual void attachChannel(const ::std::string&,
			       ::Ice::Int,
			       ::Ice::Int,
			       const Ice::Current&);

	virtual void getChannelAttachedInfo(const ::std::string&, ::Ice::Int&, 
		                               ::Ice::Int&, const Ice::Current&);

    virtual ::std::string getSessionId(const Ice::Current&);

    virtual void detachChannel(const ::std::string&,
			       const Ice::Current&);

    virtual DestInfo getInfo(const Ice::Current&) const;

    virtual void serve(const Ice::Current&);

    virtual void setProperies(const ::TianShanIce::Properties&,
			      const Ice::Current&);

    virtual ::TianShanIce::Properties getProperties(const Ice::Current&) const;

    virtual void stop(const Ice::Current&);

    virtual void pause(const Ice::Current&);

    virtual void destroy(const Ice::Current&);
    	
	virtual void removeChannel(const ::std::string&, 
		const ::Ice::Current& );
	
	virtual void activate(const ::Ice::Current& );

    virtual ::TianShanIce::StrValues listChannels(const ::Ice::Current&)const;
	
	virtual void getAttachedInfo(const ::std::string&, 
		::DataOnDemand::AttachedInfo&, 
		const ::Ice::Current& );

public:

	bool init();
	DataStreamPrx getStreamPrx();
	DataStreamPrx		_stream;
protected:
	inline DestinationExPrx getThisProxy(
		const Ice::ObjectAdapterPtr& adapter);

	void rebuildMuxItem(DataStreamPrx& strm);
	void notifyMuxItem();

protected:
	DestinationExPrx	_thisPrx;
};

}
