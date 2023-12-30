// **********************************************************************
//
// 
//
// **********************************************************************

#ifndef __ZQTianShan_NSSIMPL_H__
#define __ZQTianShan_NSSIMPL_H__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <NSS.h>
#include <NSSEx.h>
#include "NSSEnv.h"
#include <SessionIdx.h>
#include "NSSEnv.h"
#include "NSSConfig.h"
#include "NSSEventSinkI.h"

//const static ::std::string client_mac = "client_mac";
//const static ::std::string destination = "destination";	
//const static ::std::string client_port = "client_port";
//const static ::std::string bandwidth = "bandwidth";
//const static ::std::string sop_name = "sop_name";

namespace ZQTianShan{
namespace NSS{

class NGODStreamImpl;
typedef IceUtil::Handle<NGODStreamImpl> NGODStreamImplPtr;

class NGODStreamServiceImpl;
typedef IceUtil::Handle<NGODStreamServiceImpl> NGODStreamServiceImplPtr;

class NGODStreamImpl : public ::TianShanIce::Streamer::NGODStreamServer::NGODStreamEx, 
					   public ::IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	NGODStreamImpl(NSSEnv &env);
	virtual ~NGODStreamImpl();

	void renewPathTicket();


public: 
	//impl of local function
	//bool addToSessionGroup();
	//void InitRTSPSession();
	void setRtspClientSession(RTSPClientSession *rtspClientSession);

	//impl of NGODStreamEx
	///get the NGOD client volume
	///@return the NGOD client volume
	//implement NGODStreamServer::NGODStreamEx

	//virtual ::std::string getVolume(const ::Ice::Current& c);
	virtual ::std::string getVolume(const ::Ice::Current& c) const;

 	///get the TransportHeader list
	///@return ngod TransportHeader list   
    virtual ::TianShanIce::Streamer::NGODStreamServer::transHeaderList getTransportHeader(const ::Ice::Current& c) const;
	//virtual ::TianShanIce::Streamer::NGODStreamServer::transHeaderList getTransportHeader(const ::Ice::Current& c);

 	///get the asset id list
	///@return the asset id list   
	virtual ::TianShanIce::Streamer::NGODStreamServer::ItemList getAssetId(const ::Ice::Current& c) const;
    
public:
	//impl of stream
	virtual void allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx& ticket, 
								 const ::Ice::Current& c)
								 throw (::TianShanIce::Transport::ExpiredTicket, 
										::TianShanIce::ServerError, 
										::TianShanIce::NotSupported);
  
    virtual void destroy(const ::Ice::Current& c)
						 throw (::TianShanIce::ServerError);
   
    virtual ::std::string lastError(const ::Ice::Current& c) const;
  
    virtual ::Ice::Identity getIdent(const ::Ice::Current& c) const;

    virtual void setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask, 
									   const ::TianShanIce::Streamer::ConditionalControlPrx& condCtrl, 
									   const ::Ice::Current& c);
    
    virtual bool play(const ::Ice::Current& c)
					  throw (::TianShanIce::ServerError, 
							 ::TianShanIce::InvalidStateOfArt);
    
    virtual bool setSpeed(::Ice::Float, 
						  const ::Ice::Current& c)
						  throw (::TianShanIce::InvalidParameter, 
								 ::TianShanIce::ServerError, 
								 ::TianShanIce::InvalidStateOfArt);
    
    virtual bool pause(const ::Ice::Current& c)
					   throw (::TianShanIce::ServerError, 
							  ::TianShanIce::InvalidStateOfArt);
    
    virtual bool resume(const ::Ice::Current& c)
						throw (::TianShanIce::ServerError, 
							   ::TianShanIce::InvalidStateOfArt);
    
    virtual ::TianShanIce::Streamer::StreamState 
		getCurrentState(const ::Ice::Current& c) const
						throw (::TianShanIce::ServerError);
    
    virtual ::TianShanIce::SRM::SessionPrx 
		getSession(const ::Ice::Current& c)
			   	   throw (::TianShanIce::ServerError);
    
    virtual void setMuxRate(::Ice::Int nowRate, 
							::Ice::Int maxRate, 
							::Ice::Int minRate, 
							const ::Ice::Current& c);
    
    virtual bool allocDVBCResource(::Ice::Int serviceGroupID, 
								   ::Ice::Int bandWidth,
								   const ::Ice::Current& c)
								   throw (::TianShanIce::ServerError);
    
    virtual ::Ice::Long seekStream(::Ice::Long offset, 
							   	   ::Ice::Int startPos, 
								   const ::Ice::Current& c)
								   throw (::TianShanIce::ServerError,
										  ::TianShanIce::InvalidParameter);
    
	virtual ::TianShanIce::Streamer::StreamInfo playEx(::Ice::Float, 
													   ::Ice::Long,
													   ::Ice::Short,
													   const ::TianShanIce::StrValues&,
													   const ::Ice::Current&)
													   throw (::TianShanIce::ServerError,
													   ::TianShanIce::InvalidStateOfArt);

	virtual ::TianShanIce::Streamer::StreamInfo pauseEx(const ::TianShanIce::StrValues&, 
														const ::Ice::Current&)
														throw (::TianShanIce::ServerError, 
														::TianShanIce::InvalidStateOfArt);
    /*virtual void playEx(::Ice::Long& timeOffset, 
						::Ice::Float& currentSpeed, 
						const ::Ice::Current& c) 
						throw (::TianShanIce::ServerError, 
							   ::TianShanIce::InvalidStateOfArt);
    
    virtual void pauseEx(::Ice::Long& timeOffset, 
						 const ::Ice::Current& c)
						 throw (::TianShanIce::ServerError, 
								::TianShanIce::InvalidStateOfArt);*/
    
    virtual void setSpeedEx(::Ice::Float newSpeed, 
							::Ice::Long& timeOffset, 
							::Ice::Float& currentSpeed, 
							const ::Ice::Current& c)
							throw (::TianShanIce::InvalidParameter, 
								   ::TianShanIce::ServerError, 
								   ::TianShanIce::InvalidStateOfArt);

	virtual void commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& amdStream, 
							  const ::Ice::Current& c)
							throw (::TianShanIce::ServerError);
    
	//impl of playlist
	virtual ::std::string getId(const ::Ice::Current& c) const;
   
    virtual bool getInfo(::Ice::Int mask, 
						 ::TianShanIce::ValueMap& var, 
						 const ::Ice::Current& c);
   
    virtual ::Ice::Int insert(::Ice::Int userCtrlNum, 
							  const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
							  ::Ice::Int whereUserCtrlNum,
							  const ::Ice::Current& c)
							  throw (::TianShanIce::InvalidParameter, 
									 ::TianShanIce::ServerError, 
									 ::TianShanIce::InvalidStateOfArt);
   
    virtual ::Ice::Int pushBack(::Ice::Int userCtrlNum, 
								const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo, 
								const ::Ice::Current& c)
								throw (::TianShanIce::InvalidParameter, 
									   ::TianShanIce::ServerError, 
									   ::TianShanIce::InvalidStateOfArt);
   
    virtual ::Ice::Int size(const ::Ice::Current& c) const;
   
    virtual ::Ice::Int left(const ::Ice::Current& c) const;
   
    virtual bool empty(const ::Ice::Current& c) const
					   throw (::TianShanIce::ServerError, 
							  ::TianShanIce::InvalidStateOfArt);
   
    virtual ::Ice::Int current(const ::Ice::Current& c) const
							   throw (::TianShanIce::ServerError, 
									  ::TianShanIce::InvalidStateOfArt);
   
    virtual void erase(::Ice::Int whereUserCtrlNum, 
					   const ::Ice::Current& c)
					   throw (::TianShanIce::InvalidParameter, 
							  ::TianShanIce::ServerError, 
							  ::TianShanIce::InvalidStateOfArt);
   
    virtual ::Ice::Int flushExpired(const ::Ice::Current& c)
									throw (::TianShanIce::ServerError, 
										   ::TianShanIce::InvalidStateOfArt);
   
    virtual bool clearPending(bool includeInitedNext, 
							  const ::Ice::Current& c)
						  	  throw (::TianShanIce::ServerError, 
									 ::TianShanIce::InvalidStateOfArt);
   
    virtual bool isCompleted(const ::Ice::Current& c)
							throw (::TianShanIce::ServerError, 
								   ::TianShanIce::InvalidStateOfArt);
   
    virtual ::TianShanIce::IValues getSequence(const ::Ice::Current& c) const;
   
    virtual ::Ice::Int findItem(::Ice::Int userCtrlNum,
								::Ice::Int from, 
								const ::Ice::Current& c)
								throw (::TianShanIce::InvalidParameter, 
									   ::TianShanIce::ServerError, 
									   ::TianShanIce::InvalidStateOfArt);
   
    virtual bool distance(::Ice::Int to, 
						  ::Ice::Int from, 
						  ::Ice::Int& dist, 
						  const ::Ice::Current& c)
						  throw (::TianShanIce::InvalidParameter, 
								 ::TianShanIce::ServerError, 
								 ::TianShanIce::InvalidStateOfArt);
   
    virtual bool skipToItem(::Ice::Int where, 
							bool bPlay,
							const ::Ice::Current& c)
							throw (::TianShanIce::InvalidParameter, 
								   ::TianShanIce::ServerError, 
								   ::TianShanIce::InvalidStateOfArt);
   
    virtual bool seekToPosition(::Ice::Int UserCtrlNum, 
								::Ice::Int timeOffset, 
								::Ice::Int startPos, 
								const ::Ice::Current& c)
								throw (::TianShanIce::InvalidParameter, 
									   ::TianShanIce::ServerError, 
									   ::TianShanIce::InvalidStateOfArt);
   
    virtual void enableEoT(bool enable, 
						   const ::Ice::Current& c)
						   throw (::TianShanIce::ServerError, 
								  ::TianShanIce::InvalidStateOfArt);

	virtual ::TianShanIce::Streamer::StreamInfo playItem(::Ice::Int UserCtrlNum,
														 ::Ice::Int timeOffset,
														 ::Ice::Short from,
														 ::Ice::Float newSpeed,
														 const ::TianShanIce::StrValues& expectedProps,
														 const ::Ice::Current& c)
														 throw (::TianShanIce::ServerError,
																::TianShanIce::InvalidStateOfArt, 
																::TianShanIce::InvalidParameter);
   
public: //implement NGODStreamServer::NGODStream

   	///get the NGOD OnDemandSessionID
	///@return the NGOD OnDemandSessionID, should be ident.name
	virtual ::std::string getOnDemandSessionId(const ::Ice::Current& c) const;

	///get session id assigned by the target NGOD Streaming Server
	///@return the session id assigned by the target NGOD Streaming Server
	virtual ::std::string getNssSessionId(const ::Ice::Current& c) const;

	///get session group id assigned by this NGODStreamService
	///@return the session group id assigned by this NGODStreamService
	virtual ::std::string getSessionGroup(const ::Ice::Current& c) const;

	///get URL to control the stream
	///@return the stream control URL
	virtual ::std::string getCtrlURL(const ::Ice::Current& c) const;
   
public:
	NSSEnv &_env;
	NSSSessionGroupList *_nssSessionGroupList;
	//RTSPClientSession *_rtspClientSession;
	::ZQ::common::FileLog &_logFile;
	::ZQ::common::NativeThreadPool &_pool;
};

class NGODStreamServiceImpl : public TianShanIce::Streamer::NGODStreamServer::NGODStreamService, 
							  public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	NGODStreamServiceImpl(ZQ::common::FileLog &LogFile,
						  ::ZQ::common::NativeThreadPool &pool,
						  ::Ice::CommunicatorPtr& communicator,
						  string &strServerPath,
						  uint16 &uServerPort,
						  SessionGroup &sessionGroups,
						  int32 &evictorSize,
						  NSSEnv &env);
	virtual ~NGODStreamServiceImpl();

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

	// impl of NGODStreamService
	// ----------------------
	virtual void getStreamStat_async(const ::TianShanIce::Streamer::NGODStreamServer::AMD_NGODStreamService_getStreamStatPtr&, const ::TianShanIce::StrValues&, const ::std::string&, const ::Ice::Current& c);
	virtual ::TianShanIce::Streamer::NGODStreamServer::NGODStreamPrx findStreamByOnDemandSession(const ::std::string&, const ::Ice::Current& c);
    virtual ::TianShanIce::Streamer::NGODStreamServer::NGODStreamPrx findStreamByNssSession(const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    
	// impl of base service
	//
	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

	//impl of TianShanIce
	virtual void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr&, const ::std::string&, const ::std::string&, bool, const ::Ice::Current&);
   
private:
	NSSSessionGroup *InitSessionGroup(string &strSessionGroup, 
									  string &strServerPath, 
									  uint16 uServerPort);
	RTSPClientSession *InitRTSPSession(const ::TianShanIce::Transport::PathTicketPrx &_pathTicket);
	NSSEnv &_env;
	::ZQ::common::FileLog &_logFile;
	::ZQ::common::Guid _guid;
	ngod_daemon_thread *_nssDaemodThrdPtr;
	::ZQ::common::NativeThreadPool &_pool;
	NSSSessionGroupList _nssSessionGroupList;
	string &_strServerPath;
	uint16 &_uServerPort;
};

void* getVariantValue(::TianShanIce::Variant &val);

void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal);

TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
										const TianShanIce::SRM::ResourceType& type,
										const std::string& strkey);

const int iDefaultEvictorSize = 100;

}//namespace NSS

}//namespace ZQTianShan

#endif __ZQTianShan_NSSIMPL_H__