#ifndef __ZQTianShan_TMVSStreamIMPL_H__
#define __ZQTianShan_TMVSStreamIMPL_H__

//include ice header
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <Ice/ServantManagerF.h>

//environment header
#include "TMVSSEnv.h"

//include std header
#include <string>

//include ZQCommon header
#include "NativeThread.h"
#include "NativeThreadPool.h"

namespace ZQTianShan{
namespace VSS{
namespace TM{

class TMVStreamImpl : public ::TianShanIce::Streamer::TMVStreamServer::TMVStream, 
					  public ::IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	TMVStreamImpl(TMVSSEnv &env);
	virtual ~TMVStreamImpl();

	//implement TMVStreamImpl local function
public: 
	///renew the path ticket status to sync with weiwoo
	void renewPathTicket(const ::Ice::Current& c);

	bool checkSoapClientSession();

	
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

	virtual void play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr& amd, const ::Ice::Current& c)
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

	virtual void seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr& amd,
		::Ice::Long offset, 
		::Ice::Int startPos, 
		const ::Ice::Current& c)
		throw (::TianShanIce::ServerError,
		::TianShanIce::InvalidParameter);

	virtual void playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr& amd,
		::Ice::Float newSpeed, 
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

	virtual void skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& amd,
		::Ice::Int where, 
		bool bPlay,
		const ::Ice::Current& c)
		throw (::TianShanIce::InvalidParameter, 
		::TianShanIce::ServerError, 
		::TianShanIce::InvalidStateOfArt);

	virtual void seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& amd,
		::Ice::Int UserCtrlNum, 
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

	virtual void playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& amd,
		::Ice::Int UserCtrlNum,
		::Ice::Int timeOffset,
		::Ice::Short from,
		::Ice::Float newSpeed,
		const ::TianShanIce::StrValues& expectedProps,
		const ::Ice::Current& c)
		throw (::TianShanIce::ServerError,
		::TianShanIce::InvalidStateOfArt, 
		::TianShanIce::InvalidParameter);

	virtual ::TianShanIce::Properties getProperties(const ::Ice::Current&) const;

	virtual void setProperties(const ::TianShanIce::Properties&, const ::Ice::Current&);

	//implement ::TianShanIce::Streamer::TMVStreamServer::TMVStream
public: 
	///get the TMV OnDemandSessionID
	///@return the TMV OnDemandSessionID, should be ident.name
	virtual ::std::string getOnDemandSessionId(const ::Ice::Current& c) const;

	///get session id assigned by the target TMV Streaming Server
	///@return the session id assigned by the target TMV Streaming Server
	virtual ::std::string getSessionId(const ::Ice::Current& c) const;

	///get URL to control the stream
	///@return the stream control URL
	virtual ::std::string getCtrlURL(const ::Ice::Current& c) const;

public:
	TMVSSEnv &_env;
	TMVSoapClientSession *_soapClientSession;
	//TMVSSProxy *_soapClientSession;
	::ZQ::common::FileLog &_logFile;
	::ZQ::common::NativeThreadPool &_pool;
};

typedef IceUtil::Handle<TMVStreamImpl> TMVStreamImplPtr;

}//namespace TM
}//namespace VSS
}//namespace ZQTianShan
#endif __ZQTianShan_TMVSStreamIMPL_H__