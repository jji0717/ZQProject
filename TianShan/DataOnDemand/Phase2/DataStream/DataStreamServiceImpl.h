// DataStreamServiceImpl.h: interface for the DataStreamServiceImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATASTREAMSERVICEIMPL_H__707240FA_6B5A_4F89_AD11_D955DA02C544__INCLUDED_)
#define AFX_DATASTREAMSERVICEIMPL_H__707240FA_6B5A_4F89_AD11_D955DA02C544__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Ice/Ice.h>
#include <DataStream.h>
#include <IceUtil/IceUtil.h>

namespace DataOnDemand {

class DataStreamImpl;

class DataStreamServiceImpl: virtual public DataStreamService, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
	
public:
	DataStreamServiceImpl(Ice::ObjectAdapterPtr& adapter);
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
		const ::Ice::Current& = ::Ice::Current());


	virtual ::DataOnDemand::DataStreamPrx getStream(
    const ::std::string&,
		const ::std::string&,
		const Ice::Current& = ::Ice::Current());

	virtual TianShanIce::StrValues listStrems(const std::string& space, 
	  const Ice::Current& = ::Ice::Current()) const;

	virtual void clear(const ::std::string& space, 
		const Ice::Current& = ::Ice::Current());

	virtual void setProperies(const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Properties getProperties(
		const ::Ice::Current& = ::Ice::Current()) const;

	virtual void ping(const std::string& space, 
		const ::Ice::Current& = ::Ice::Current()) const;
	
	virtual void destroy(const ::Ice::Current& = ::Ice::Current());

	virtual void removeStream(const ::std::string&, const ::std::string&);

	virtual void checkTimeout(Ice::Int msec);

	void queryReplicas_async(const TianShanIce::AMD_ReplicaQuery_queryReplicasPtr &,
		const std::string &,const std::string &,bool,const Ice::Current &);

protected:
	
	typedef std::map<std::string, DataStreamImpl* > DataStreamDict;

	struct SpaceInfo {
		u_long			lastUpdate;
		long			updateCount;
		DataStreamDict	streams;
	};

	typedef std::map<std::string, SpaceInfo>	SpaceInfoMap;

protected:

	SpaceInfoMap::iterator createSpace(const std::string& space);

	void clearStreamDict(DataStreamDict& streams);

	DataStreamDict* getStreamDict(const std::string& spaceName) const
	{
		SpaceInfoMap::iterator spaceIt = _spaceInfos.find(spaceName);
		if (spaceIt == _spaceInfos.end()) {

			// log warning
			return NULL;
		}

		return &spaceIt->second.streams;
	}

protected:

	TianShanIce::Properties	_props;
	mutable SpaceInfoMap	_spaceInfos;
	::Ice::ObjectAdapterPtr	_adapter;
	bool					_destroy;
	int						_rateCount;
};

} // namespace DataOnDemand {

#endif // !defined(AFX_DATASTREAMSERVICEIMPL_H__707240FA_6B5A_4F89_AD11_D955DA02C544__INCLUDED_)
