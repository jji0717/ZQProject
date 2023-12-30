#ifndef __CVSSIMPL_H__
#define __CVSSIMPL_H__

#include "CVSSEnv.h"

namespace ZQTianShan{

namespace CVSS{

class CiscoVirtualStreamImpl;
typedef IceUtil::Handle<CiscoVirtualStreamImpl> CiscoVirtualStreamImplPtr;

class CiscoVirtualStreamServiceImpl;
typedef IceUtil::Handle<CiscoVirtualStreamServiceImpl> CiscoVirtualStreamServiceImplPtr;

class CiscoVirtualStreamImpl : public ::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStream, 
							   public ::IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	CiscoVirtualStreamImpl(CVSSEnv &env);
	virtual ~CiscoVirtualStreamImpl();

public: 
	//impl of local function
	void setItemInfo(::TianShanIce::Streamer::CiscoVirtualStreamServer::stItem &item, ::Ice::Int userCtrlNum, const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo);
	void setPlayItem(::Ice::Long timeOffset,::Ice::Int startPos);
	void setRtspClientSession(CVSSRtspSession *rtspClientSession);
	void checkSessionStatus(const char* funcName) const
	{
		if (funcName == NULL)
			funcName = "Unknow Function";
		if (_cvssRTSPSession == NULL)
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",178,"%s: Ident(%s) failed to find sessionId", funcName, ident.name.c_str());
	}
	CVSSRtspSession *_cvssRTSPSession;

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

	virtual ::TianShanIce::Streamer::StreamInfo playItem(::Ice::Int, 
		::Ice::Int, 
		::Ice::Short, 
		::Ice::Float, 
		const ::TianShanIce::StrValues&, 
		const ::Ice::Current&);


public: //implement NGODStreamServer::NGODStream

	///get session id assigned by the target NGOD Streaming Server
	///@return the session id assigned by the target NGOD Streaming Server
	virtual ::std::string getSessionId(const ::Ice::Current& c) const;

	///get URL to control the stream
	///@return the stream control URL
	virtual ::std::string getCtrlURL(const ::Ice::Current& c) const;

	///get the ItemList list
	///@return ItemList list
	virtual ::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList getItemList(const ::Ice::Current& c) const;

	///get the current playlist index
	///@return the current playlist index
	virtual ::Ice::Long getCurPlayListIdx(const ::Ice::Current& c) const;

	///get the PathTicket proxy string
	///@return the PathTicket proxy string
	virtual::std::string getPathTicketStr(const ::Ice::Current& c) const;

	///get the client destination ip address
	///@return the ip address
	virtual ::std::string getDestination(const ::Ice::Current& c) const;

	///get the client port
	///@return the client port
	virtual ::Ice::Long getClientPort(const ::Ice::Current& c) const;

	///do heartbeat with RTSP server
	///@return the asset id list
	virtual void doHeartBeat(const ::Ice::Current& c) const;

	///play with RTSP server use the next item content id
	virtual void playNextItem(const ::Ice::Current& c);

	///renew the path ticket status to sync with weiwoo
	virtual void renewPathTicket(const ::Ice::Current& c);

public:
	CVSSEnv &_env;
	::ZQ::common::FileLog &_logFile;
	::ZQ::common::NativeThreadPool &_pool;
	SOCKET _index;
};


class CiscoVirtualStreamServiceImpl : public TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamService, 
							  public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	CiscoVirtualStreamServiceImpl(ZQ::common::FileLog &LogFile,
								  ::ZQ::common::NativeThreadPool &recvPool,
								  ::ZQ::common::NativeThreadPool &sendPool,
								  ::Ice::CommunicatorPtr& communicator,
								  ::std::string &strServerPath,
								  uint16 &uServerPort,
								  int32 &evictoreSize,
								  CVSSEnv &env);
	virtual ~CiscoVirtualStreamServiceImpl();

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
	virtual void getStreamStat_async(const ::TianShanIce::Streamer::CiscoVirtualStreamServer::AMD_CVStreamService_getStreamStatPtr&, const ::TianShanIce::StrValues&, const ::std::string&, const ::Ice::Current& c);
	virtual ::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx findStreamBySocket(::Ice::Long, const ::Ice::Current& c);
	virtual ::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx findStreamBySession(const ::std::string&, const ::Ice::Current& c);

	// impl of base service
	//
	virtual ::std::string getAdminUri(const ::Ice::Current& c);
	virtual ::TianShanIce::State getState(const ::Ice::Current& c);

	//impl of TianShanIce
	virtual void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr&, const ::std::string&, const ::std::string&, bool, const ::Ice::Current&);

private:
	CVSSRtspSession *InitRTSPSession(const ::TianShanIce::Transport::PathTicketPrx &_pathTicket, ::std::string &strServerIp, uint16 &iServerPort);
	CVSSRtspSession *InitRTSPSession(::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx cvStreamPrx, ::std::string &strServerIp, uint16 &iServerPort);
	bool InitConnection(CVSSRtspSession *sess);
	CVSSEnv &_env;
	::ZQ::common::FileLog &_logFile;
	::ZQ::common::Guid _guid;
	daemon_thread *_daemodThrdPtr;
	::ZQ::common::NativeThreadPool &_recvPool;
	::ZQ::common::NativeThreadPool &_sendPool;
	::std::string &_strServerPath;
	uint16 &_uServerPort;
};

void* getVariantValue(::TianShanIce::Variant &val);

void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal);

TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
										const TianShanIce::SRM::ResourceType& type,
										const std::string& strkey);

const int iDefaultEvictorSize = 100;

}//namespace CVSS

}//namespace ZQTianShan
#endif __CVSSIMPL_H__