#ifndef __DataStreamImpl_h__
#define __DataStreamImpl_h__

#include <DataStream.h>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <DSServantLocatorImpl.h>
#include <TsSRM.h>
#include "DataDef.h"
#include "DataPusher.h"
#include "datastreamcfg.h"

extern ZQ::common::Config::Loader<DataStreamCfg> gDataStreamConfig;
namespace TianShanIce  {
namespace Streamer     {
namespace DataOnDemand {

class DataStreamServiceImpl;

::Ice::Identity createMuxItemIdentity(const std::string& space, 
                                      const std::string& strmName, 
                                      const std::string& itemName);

::Ice::Identity createStreamIdentity(const std::string& space, 
                                     const std::string& name);


class NameToStream {
public:
	NameToStream(const Ice::ObjectAdapterPtr& adapter) :
	  _adapter(adapter)
	{

	}

	TianShanIce::Streamer::DataOnDemand::DataStreamPrx operator()(const std::string& space, 
	const std::string& name)
	{
	  return TianShanIce::Streamer::DataOnDemand::DataStreamPrx::uncheckedCast(
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

	MuxItemPrx operator()(const std::string& space, 
		const std::string& strmName, const std::string& itemName)
	{
	  return TianShanIce::Streamer::DataOnDemand::MuxItemPrx::uncheckedCast(
		  _adapter->createProxy(createMuxItemIdentity(space, strmName, itemName)));
	}

private:

	const Ice::ObjectAdapterPtr _adapter;
};

class DataStreamServiceImpl;
class MuxItemImpl;

class DataStreamImpl : virtual public DataStream, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {

	friend class DataStreamServiceImpl;

public:
	typedef std::map<std::string, MuxItemImpl* > MuxItemDict;

	DataStreamImpl(ZQADAPTER_DECLTYPE& adapter,
		::Freeze::DSServantLocatorPtr& dsServantLocator, 
		DataStreamServiceImpl* parent, 
		const std::string& space);
	virtual ~DataStreamImpl();

	bool init(const ::TianShanIce::Streamer::DataOnDemand::StreamInfo& info);

	// Stream
	virtual void allotPathTicket(
		const ::TianShanIce::Transport::PathTicketPrx&,
		const Ice::Current& = Ice::Current());

	virtual void destroy(const Ice::Current& = Ice::Current());

	virtual ::std::string lastError(
		const Ice::Current& = Ice::Current()) const;

	virtual ::Ice::Identity getIdent(
		const Ice::Current& = Ice::Current()) const;

	virtual void setConditionalControl(
		const ::TianShanIce::Streamer::ConditionalControlMask&, 
		const ::TianShanIce::Streamer::ConditionalControlPrx&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual bool play(const Ice::Current& = Ice::Current());

	virtual bool setSpeed(::Ice::Float,
		const Ice::Current& = Ice::Current());

	virtual bool pause(const Ice::Current& = Ice::Current());

	virtual bool resume(const Ice::Current& = Ice::Current());

	virtual ::TianShanIce::Streamer::StreamState getCurrentState(
		const Ice::Current& = Ice::Current()) const;

	virtual TianShanIce::SRM::SessionPrx getSession(
		const Ice::Current& = Ice::Current());

	virtual void setMuxRate(::Ice::Int, ::Ice::Int, ::Ice::Int, 
		const ::Ice::Current& = ::Ice::Current());

	virtual bool allocDVBCResource(::Ice::Int, ::Ice::Int, 
		const ::Ice::Current& = ::Ice::Current());
	
	virtual ::Ice::Long seekStream(::Ice::Long, ::Ice::Int, 
		const ::Ice::Current& = ::Ice::Current());


	virtual void playEx(::Ice::Long&, ::Ice::Float&, 
		const ::Ice::Current& = ::Ice::Current());
	
    virtual void pauseEx(::Ice::Long&, const ::Ice::Current& = ::Ice::Current());
	
    virtual void setSpeedEx(::Ice::Float, ::Ice::Long&, ::Ice::Float&,
							const ::Ice::Current& = ::Ice::Current());


	// DataStream
    virtual ::std::string getName(
		const Ice::Current& = Ice::Current()) const;

    virtual ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo(
		const Ice::Current& = Ice::Current()) const;

	virtual ::Ice::Int control(::Ice::Int, const ::std::string&, 
		const ::Ice::Current& = Ice::Current());

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(
						     const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo&,
						     const Ice::Current& = Ice::Current());

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&,
						  const Ice::Current& = Ice::Current());

    virtual ::Ice::StringSeq listMuxItems(
		const Ice::Current& = Ice::Current()) const;

	virtual void setProperties(const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Properties getProperties(
		const ::Ice::Current& = ::Ice::Current()) const;

	void DataStreamImpl::ping(const ::Ice::Current& = ::Ice::Current()) const;

	bool removeMuxItem(const ::std::string&);

	virtual void commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr&, const ::Ice::Current& = ::Ice::Current());
	virtual ::TianShanIce::Streamer::StreamInfo playEx(::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());
	virtual ::TianShanIce::Streamer::StreamInfo pauseEx(const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());

	void play_async(const TianShanIce::Streamer::AMD_Stream_playPtr &,const Ice::Current &);
	void seekStream_async(const TianShanIce::Streamer::AMD_Stream_seekStreamPtr &,Ice::Long,Ice::Int,const Ice::Current &);
	void playEx_async(const TianShanIce::Streamer::AMD_Stream_playExPtr &,Ice::Float,Ice::Long,Ice::Short,const TianShanIce::StrValues &,const Ice::Current &);

	const std::string& getSpaceName()
	{
		return _space;
	}

	const std::string getCacheDir()
	{
		if (_cacheDir.empty()) {
			_cacheDir = gDataStreamConfig.catchDir;
			_cacheDir = _cacheDir + "\\" + _space;
		}

		return _cacheDir;
	}

protected:
	void clearDataSource();
	bool findMuxItemByStreamId(int streamId);

protected:

	StreamInfo							_info;
	DataStreamServiceImpl*				_parent;
	MuxItemDict							_muxItems;
	TianShanIce::Properties				_props;
	TianShanIce::Streamer::StreamState	_state;
	std::string							_space;
	::DataStream::TransferAddress		_transferAddr;

	std::string							_lastError;
	::TianShanIce::SRM::SessionPrx		_session;
	ZQADAPTER_DECLTYPE&				    _adapter;
	::Freeze::DSServantLocatorPtr&    _dsServantLocator;
	::DataStream::DataPusher*			_dataPusher;
	::DataStream::DataSource**			_dataSources;
	volatile bool						_destroy;
	std::string							_cacheDir;
};

} // namespace DataOnDemand {
}
}

#endif
