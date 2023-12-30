#ifndef __ZQTianShan_TMVSSIMPL_H__
#define __ZQTianShan_TMVSSIMPL_H__

#include "TMVStreamImpl.h"
#include "TMVSSIce.h"

namespace ZQTianShan{
namespace VSS{
namespace TM{

class TMVStreamServiceImpl : public TianShanIce::Streamer::TMVStreamServer::TMVStreamService, 
							 public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>,
							 public ZQ::common::NativeThread
{
public:
	TMVStreamServiceImpl(ZQ::common::FileLog &LogFile,
						 //::ZQ::common::NativeThreadPool &pool,
						 ::Ice::CommunicatorPtr& communicator,
						 std::string &strServerPath,
						 uint16 &uServerPort,
						 std::string &strNotifyServer,
						 uint16 &usNotifyPort,
						 int32 &evictorSize,
						 TMVSSEnv &env);
	virtual ~TMVStreamServiceImpl();

	void uninitializeService()
	{
		::Ice::Identity ident = _env._adapter->getCommunicator()->stringToIdentity(DBFILENAME_TMVSession);
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
	virtual void getStreamStat_async(const ::TianShanIce::Streamer::TMVStreamServer::AMD_TMVStreamService_getStreamStatPtr&, const ::TianShanIce::StrValues&, const ::std::string&, const ::Ice::Current& c);

	virtual ::TianShanIce::Streamer::TMVStreamServer::TMVStreamPrx findStreamByOnDemandSession(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Streamer::TMVStreamServer::TMVStreamPrx findStreamBySession(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
    
	// impl of base service
	//
	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

	//impl of TianShanIce
	virtual void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr&, const ::std::string&, const ::std::string&, bool, const ::Ice::Current&);
   
private:
	TMVSSEnv &_env;
	::ZQ::common::FileLog &_logFile;
	::ZQ::common::Guid _guid;
	//::ZQ::common::NativeThreadPool &_pool;
	std::string &_strServerPath;
	uint16 &_uServerPort;
	std::string &_strNotifyServer;
	uint16 &_usNotifyPort;
};

typedef IceUtil::Handle<TMVStreamServiceImpl> TMVStreamServiceImplPtr;

//void* getVariantValue(::TianShanIce::Variant &val);

//void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal);

//TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
//										const TianShanIce::SRM::ResourceType& type,
//										const std::string& strkey);

const int iDefaultEvictorSize = 100;

}//namespace TM

}//namespace VSS

}//namespace ZQTianShan

#endif __ZQTianShan_TMVSSIMPL_H__