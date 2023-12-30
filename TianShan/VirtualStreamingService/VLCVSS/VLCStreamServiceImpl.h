#ifndef __ZQTianShan_VLCVSSIMPL_H__
#define __ZQTianShan_VLCVSSIMPL_H__

#include "VLCStreamImpl.h"
#include "VLCVSS.h"

namespace ZQTianShan{
namespace VSS{
namespace VLC{

class VLCStreamServiceImpl : public TianShanIce::Streamer::VLCStreamServer::VLCStreamService, 
							 public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>,
							 public ZQ::common::NativeThread
{
public:
	VLCStreamServiceImpl(ZQ::common::FileLog &LogFile,
						 ::Ice::CommunicatorPtr& communicator,
						 int32 &evictorSize,
						 VLCVSSEnv &env);
	virtual ~VLCStreamServiceImpl();

	void uninitializeService()
	{
		::Ice::Identity ident = _env._adapter->getCommunicator()->stringToIdentity(DBFILENAME_VLCSession);
		_env._adapter->remove(ident);
		terminate();
	}

	int run(void);

	// impl of StreamService
	// ----------------------
	/// create a stream instance by a given path
	///@param[in] pathTicket access to the path ticket that the new stream to use
	///@return	  access to a stream instance that is newly created
    virtual ::TianShanIce::Streamer::StreamPrx createStream(const ::TianShanIce::Transport::PathTicketPrx& pathTicket, const ::Ice::Current& c)
		throw (::TianShanIce::InvalidParameter, 
			   ::TianShanIce::ServerError, 
			   ::TianShanIce::InvalidStateOfArt);

	/// list the managed Streamer devices in this service
	///@return a collection of descriptors of the streamers
    virtual ::TianShanIce::Streamer::StreamerDescriptors listStreamers(const ::Ice::Current& c)
		throw (::TianShanIce::ServerError);

	/// get a network-wide unique id of this StreamService instance,
	/// normally this value could be read from the configuration of the StreamSerivce
	///@return a network-wide unique id
	///@note no white space is allowed in the id string
    virtual ::std::string getNetId(const ::Ice::Current& c) const
		throw (::TianShanIce::ServerError);

	// impl of TMVStreamService
	// ----------------------
	virtual void getStreamStat_async(const ::TianShanIce::Streamer::VLCStreamServer::AMD_VLCStreamService_getStreamStatPtr&, const ::TianShanIce::StrValues&, const ::std::string&, const ::Ice::Current& c);

	virtual ::TianShanIce::Streamer::VLCStreamServer::VLCStreamPrx findStreamByOnDemandSession(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Streamer::VLCStreamServer::VLCStreamPrx findStreamBySession(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
    
	// impl of base service
	//
	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

	//impl of TianShanIce
	virtual void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr&, const ::std::string&, const ::std::string&, bool, const ::Ice::Current&);
   
private:
	VLCVSSEnv &_env;
	::ZQ::common::FileLog &_logFile;
	::ZQ::common::Guid _guid;

	HANDLE _quitHandle;

	TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
		const TianShanIce::SRM::ResourceType& type,
		const std::string& strkey);
};

void* getVariantValue(::TianShanIce::Variant &val);

void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal);

typedef IceUtil::Handle<VLCStreamServiceImpl> VLCStreamServiceImplPtr;

const int iDefaultEvictorSize = 100;

}//namespace VLC

}//namespace VSS

}//namespace ZQTianShan

#endif __ZQTianShan_VLCVSSIMPL_H__