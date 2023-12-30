#ifndef __DataStreamImpl_h__
#define __DataStreamImpl_h__

#include <DataStreamEx.h>
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>

namespace DataOnDemand
{

class DataStreamServiceImpl;

class StreamerFactory: public Ice::ObjectFactory {
public:

	StreamerFactory()
	{

	}

	//
	// Operations from ObjectFactory
	//
	virtual Ice::ObjectPtr create(const std::string& type);

	virtual void destroy()
	{

	}

private:

};

::Ice::Identity createStreamIdentity(const std::string& space, 
                                     const std::string& name);
::Ice::Identity createMuxItemIdentity(const std::string& space, 
                                      const std::string& strmName, 
                                      const std::string& itemName);

class NameToStream {
public:
	NameToStream(const Ice::ObjectAdapterPtr& adapter) :
	  _adapter(adapter)
	  {

	  }

	  DataOnDemand::DataStreamExPrx operator()(const std::string& space, 
      const std::string& name)
	  {
		  return DataOnDemand::DataStreamExPrx::uncheckedCast(
			  _adapter->createProxy(createStreamIdentity(space, name)));
	  }

private:

	const Ice::ObjectAdapterPtr _adapter;
};

class NameToMuxItem {
public:
	NameToMuxItem(const Ice::ObjectAdapterPtr& adapter) :
	  _adapter(adapter)
	  {

	  }

	  DataOnDemand::MuxItemExPrx operator()(const std::string& space, 
      const std::string& strmName, 
		  const std::string& itemName)
	  {
		  return DataOnDemand::MuxItemExPrx::uncheckedCast(
			  _adapter->createProxy(createMuxItemIdentity(space, strmName, itemName)));
	  }

private:

	const Ice::ObjectAdapterPtr _adapter;
};

class DataStreamImpl;

class MuxItemImpl : virtual public MuxItemEx, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:

	MuxItemImpl(::Freeze::EvictorPtr _evictor);
	virtual ~MuxItemImpl();
	
	bool init(const MuxItemInfo& info);

    virtual ::std::string getName(const Ice::Current&) const;

    virtual void notifyFullUpdate(const ::std::string&, const Ice::Current&);

    virtual void notifyFileAdded(const ::std::string&, const Ice::Current&);

    virtual void notifyFileDeleted(const ::std::string&, const Ice::Current&);

    virtual ::DataOnDemand::MuxItemInfo getInfo(const Ice::Current&) const;

    virtual void destroy(const Ice::Current&);

	virtual void setProperies(const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Properties getProperties(
		const ::Ice::Current& = ::Ice::Current()) const;

	void activate(const ::Ice::Current& current);

protected:
	::Freeze::EvictorPtr				_evictor;
};

class DataStreamServiceImpl;
class DataStreamImpl : virtual public DataStreamEx, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:

	DataStreamImpl(::Freeze::EvictorPtr _evictor);
	virtual ~DataStreamImpl();

	bool init(const ::DataOnDemand::StreamInfo& info, unsigned long sessionId);

	// Stream
	virtual void allotPathTicket(
		const ::TianShanIce::Transport::PathTicketPrx&,
		const Ice::Current&);

	virtual void destroy(const Ice::Current&);

	virtual ::std::string lastError(const Ice::Current&) const;

	virtual ::Ice::Identity getIdent(const Ice::Current&) const;

	virtual void setConditionalControl(
		const ::TianShanIce::Streamer::ConditionalControlMask&, 
		const ::TianShanIce::Streamer::ConditionalControlPrx&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual bool play(const Ice::Current&);

	virtual bool setSpeed(::Ice::Float,
		const Ice::Current&);

	virtual bool pause(const Ice::Current&);

	virtual bool resume(const Ice::Current&);

	virtual ::TianShanIce::Streamer::StreamState getCurrentState(
		const Ice::Current&) const;

	virtual ::TianShanIce::SRM::SessionPrx getSession(const Ice::Current&);

	virtual void setMuxRate(::Ice::Int, ::Ice::Int, ::Ice::Int, 
		const ::Ice::Current& = ::Ice::Current());

	virtual bool allocDVBCResource(::Ice::Int, ::Ice::Int, 
		const ::Ice::Current& = ::Ice::Current());


	// DataStream
    virtual ::std::string getName(const Ice::Current&) const;

    virtual ::DataOnDemand::StreamInfo getInfo(const Ice::Current&) const;

	virtual ::Ice::Int control(::Ice::Int, const ::std::string&, 
		const ::Ice::Current&);

    virtual ::DataOnDemand::MuxItemPrx createMuxItem(
		const ::DataOnDemand::MuxItemInfo&,
		const Ice::Current&);

    virtual ::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&,
						  const Ice::Current&);

    virtual ::Ice::StringSeq listMuxItems(const Ice::Current&) const;

	virtual void setProperies(const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Properties getProperties(
		const ::Ice::Current& = ::Ice::Current()) const;

	virtual void ping(const Ice::Current&) const
	{

	}

	virtual void removeMuxItem(const ::std::string&, const ::Ice::Current& );
	virtual void activate(const ::Ice::Current& );	

protected:
	void setSessionId(unsigned long sessionId);
	unsigned long getSessionId();

protected:
	std::string							_lastError;
	::TianShanIce::SRM::SessionPrx	_session;
	::Freeze::EvictorPtr				_evictor;
};

class DataStreamServiceImpl : virtual public DataStreamServiceEx, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:

	DataStreamServiceImpl();
	virtual ~DataStreamServiceImpl();

	bool init();

	virtual std::string	getAdminUri(
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::State getState(const ::Ice::Current& );

	virtual ::TianShanIce::Streamer::StreamPrx createStream(
		const ::TianShanIce::Transport::PathTicketPrx&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Streamer::StreamerDescriptors listStreamers(
		const ::Ice::Current& = ::Ice::Current());

	virtual ::std::string getNetId(
		const ::Ice::Current& = ::Ice::Current()) const;
    
    virtual ::DataOnDemand::DataStreamPrx createStreamByApp(
		const ::TianShanIce::Transport::PathTicketPrx&,
		const ::std::string&,
		const ::DataOnDemand::StreamInfo&,
		const Ice::Current&);


	virtual ::DataOnDemand::DataStreamPrx getStream(
    const ::std::string&,
		const ::std::string&,
		const Ice::Current&);

	virtual TianShanIce::StrValues listStrems(const std::string& space, 
	  const Ice::Current&) const;

  virtual void clear(const ::std::string& space, const Ice::Current&);

	virtual void setProperies(const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Properties getProperties(
		const ::Ice::Current& = ::Ice::Current()) const;

	virtual void ping(const std::string& space, const Ice::Current&) const;

	void DataStreamService::destroy(const Ice::Current &)
	{

	}

	virtual void removeStream(const ::std::string&, const ::std::string&, 
		const ::Ice::Current& );

	virtual void activate(const ::Ice::Current& );

	virtual void checkTimeout(Ice::Int msec, const ::Ice::Current& );

protected:

	unsigned long generateSessionId();
	DataStreamServiceExPrx getThisPrx();

public:
	static ::Freeze::EvictorPtr		_evictor;
	static ::Ice::ObjectAdapterPtr	_adapter;

protected:
	
	struct SpaceInfo {
		u_long	lastUpdate;
		long	updateCount;
	};

	typedef std::map<std::string, SpaceInfo>	SpaceInfoMap;

	mutable SpaceInfoMap	_spaceInfoMap;
};

}

#endif
