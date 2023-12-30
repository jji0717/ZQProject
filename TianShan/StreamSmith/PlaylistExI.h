#ifndef _ZQ_STREAMSMITH_PlaylistEx_Implement_H__
#define	_ZQ_STREAMSMITH_PlaylistEx_Implement_H__



#include "StreamSmithadmin.h"
#include "PlaylistInternalUse.h"
#include <IceUtil/IceUtil.h>
#include <NativeThreadPool.h>
#include <TianShanDefines.h>

#include <variant.h>

namespace ZQ
{
namespace StreamSmith
{
class Playlist;


//AbstractMutexI
//class PlaylistExI:public TianShanIce::Streamer::InternalPlaylistEx,public IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>//,public ZQ::StreamSmith::PlaylistI
class PlaylistExI:public TianShanIce::Streamer::InternalPlaylistEx,public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>//,public ZQ::StreamSmith::PlaylistI
{
public:
	PlaylistExI();
	PlaylistExI(const Ice::Identity& id,
				const ::std::string& strGuid,
				const ::TianShanIce::Streamer::PlaylistAttr& attr,
				ZQADAPTER_DECLTYPE objAdapter);
	~PlaylistExI();
public:
	typedef IceUtil::Handle<PlaylistExI> Ptr;

	::std::string		GetGuid();
public:
    ::TianShanIce::Streamer::PlaylistAttr getAttr(const ::Ice::Current& = ::Ice::Current());

    void updateAttr(const ::TianShanIce::Streamer::PlaylistAttr& plAttr, 
					const ::Ice::Current& = ::Ice::Current());


	virtual void allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx&, const ::Ice::Current& = ::Ice::Current()) ;
	
    virtual void destroy(const ::Ice::Current& = ::Ice::Current());
	virtual void destroy2(::TianShanIce::Properties& feedback, const ::Ice::Current& ic = ::Ice::Current());
	
	virtual ::std::string lastError(const ::Ice::Current& = ::Ice::Current()) const;
	
    virtual ::std::string getId(const ::Ice::Current& = ::Ice::Current()) const;    
	
    virtual ::Ice::Int insert(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
   	
    virtual ::Ice::Int pushBack(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&, const ::Ice::Current& = ::Ice::Current()); 
	
	virtual ::Ice::Int size(const ::Ice::Current& = ::Ice::Current()) const; 
	
    virtual ::Ice::Int left(const ::Ice::Current& = ::Ice::Current()) const;
	
    virtual bool empty(const ::Ice::Current& = ::Ice::Current())const;
	
    virtual ::Ice::Int current(const ::Ice::Current& = ::Ice::Current()) const;
    
	virtual void erase(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	
    virtual ::Ice::Int flushExpired(const ::Ice::Current& = ::Ice::Current());
	
    virtual bool clearPending(bool, const ::Ice::Current& = ::Ice::Current());
	
    virtual bool isCompleted(const ::Ice::Current& = ::Ice::Current());
    
    virtual ::Ice::Int findItem(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	
    virtual bool distance(::Ice::Int, ::Ice::Int, ::Ice::Int&, const ::Ice::Current& = ::Ice::Current());
	
	virtual bool play(const ::Ice::Current& = ::Ice::Current());
	
    virtual bool setSpeed(::Ice::Float, const ::Ice::Current& = ::Ice::Current());
	
    virtual bool pause(const ::Ice::Current& = ::Ice::Current());
    
	virtual bool resume(const ::Ice::Current& = ::Ice::Current());
    
	virtual bool skipToItem(::Ice::Int, bool, const ::Ice::Current& = ::Ice::Current());
    
    virtual ::TianShanIce::Streamer::StreamState getCurrentState(const ::Ice::Current& = ::Ice::Current()) const; 
	
    virtual ::TianShanIce::SRM::SessionPrx getSession(const ::Ice::Current& = ::Ice::Current());
    
	virtual void attachSession(const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());

	virtual ::Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const;

	virtual bool seekToPosition(::Ice::Int cNum, ::Ice::Int timeOffset,::Ice::Int startPos, const ::Ice::Current& ic= ::Ice::Current());	

	virtual void setDestination(const ::std::string& strDestIP, ::Ice::Int destPort, const ::Ice::Current& ic = ::Ice::Current());
	
    virtual void setDestMac(const ::std::string& strDestMac, const ::Ice::Current& ic = ::Ice::Current());
	
	virtual void setMuxRate(::Ice::Int nowRate, ::Ice::Int maxRate, ::Ice::Int minRate, const ::Ice::Current& ic= ::Ice::Current());
	    
	virtual void setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask, const ::TianShanIce::Streamer::ConditionalControlPrx& prx, const ::Ice::Current& ic= ::Ice::Current());
	
	virtual bool getInfo(::Ice::Int mask, ::TianShanIce::ValueMap& var, const ::Ice::Current& = ::Ice::Current());

	virtual bool allocDVBCResource(::Ice::Int serviceGroupID, ::Ice::Int bandWidth, const ::Ice::Current& = ::Ice::Current());	

	virtual ::TianShanIce::SRM::ResourceMap getResources(const ::Ice::Current& c) const;
	
	///inherited from InternalPlaylistEx	

	virtual void setPathTicketProxy(const ::TianShanIce::Transport::PathTicketPrx& ticketProxy, const ::TianShanIce::Streamer::InternalPlaylistExPrx& plExPrx, const ::Ice::Current& ic= ::Ice::Current());
	
	virtual void renewPathTicket( const std::string& ticketProxy, ::Ice::Int iTime, const ::Ice::Current& ic= ::Ice::Current());

	TianShanIce::IValues getSequence(const ::Ice::Current& = ::Ice::Current()) const;

	::Ice::Long seekStream(::Ice::Long, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());

	virtual void setSourceStrmPort(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void enableEoT(bool bEnable, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Streamer::StreamInfo playEx(::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual ::TianShanIce::Streamer::StreamInfo pauseEx(const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void setPLaylistData(const TianShanIce::ValueMap& valMap ,  const ::Ice::Current& = ::Ice::Current() );

	virtual void setPID(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void attachClientSessionId(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual ::TianShanIce::Streamer::StreamInfo playItem(::Ice::Int UserCtrlNum, ::Ice::Int timeOffset, ::Ice::Short from, ::Ice::Float newSpeed, const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& = ::Ice::Current()) ;

	virtual ::TianShanIce::Streamer::PlaylistItemSetupInfoS getPlaylistItems(const ::Ice::Current& = ::Ice::Current()) const ;

	virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const 
	{
		IceUtil::RWRecMutex::RLock sync(*this);
		return attr.property;
	}

	virtual void setProperties(const ::TianShanIce::Properties& prop, const ::Ice::Current& = ::Ice::Current()) 
	{
		IceUtil::RWRecMutex::WLock sync(*this);
		attr.property = prop;
	}

	virtual ::Ice::Int pushBackEx(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfoS&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual void onTimer(const ::Ice::Current& = ::Ice::Current()) {
		//leave it now
		//may implement in the future
	}

	//////////////////////////////////////////////////////////////////////////
	virtual void play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr&, ::Ice::Long, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr&, ::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr&, ::Ice::Int, bool, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr&, ::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr&, ::Ice::Int, ::Ice::Int, ::Ice::Short, ::Ice::Float, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) ;

private:
	bool		translatePreEncryptionData(ZQ::common::Variant& varEcm , TianShanIce::ValueMap& valMap);

	std::string	dumpExpectProps(  const ::TianShanIce::StrValues& , ULONG& flag ) const;

public:
	enum
	{
		RENEW_OK,
		RENEW_NOOBJECT,
		RENEW_NOCONNECT
	};
	void		renewStatusReport(int status);

protected:
	enum _STREAMACTION
	{
		ACTION_PLAY,
		ACTION_PAUSE,
		ACTION_CHANGESPEEDFORWARD,
		ACTION_CHANGESPEEDBACKWARD,
		ACTION_SEEK
	};
	bool		ActionConfirm(int Action);
	bool		InternalActionConfirm(::TianShanIce::Streamer::ConditionalControlOption option,
										::TianShanIce::Streamer::ConditionalControlMask& requestMask);
	
private:
	//ZQ::StreamSmith::Playlist*							_pList;
	::TianShanIce::SRM::SessionPrx						_sessionPrx;
	::TianShanIce::Streamer::ConditionalControlPrx		_conditionCtrlPrx;
	::TianShanIce::Streamer::ConditionalControlMask		_conditionMask;
	ZQADAPTER_DECLTYPE									_objApdater;
	::TianShanIce::Transport::PathTicketPrx				_pathTicketPrx;
	
	bool												_bPlaylistShouldDestroy;
};


class RenewTicketRequest:public ZQ::common::ThreadRequest
{
public:
	RenewTicketRequest(ZQ::common::NativeThreadPool& pool,
						std::string& plePrx,
						std::string& ticketPrx,
						ZQADAPTER_DECLTYPE objAdpter,
						int time);
	~RenewTicketRequest();
	int run(void);
	void final(int retcode /* =0 */, bool bCancelled /* =false */);
private:
	int														renewTime;	
	std::string												_strPlexPrx;
	std::string												_strTicketPrx;
	ZQADAPTER_DECLTYPE										_objAdpter;	
};

class DestroyPlaylistRequest:public ZQ::common::ThreadRequest
{
public:
	DestroyPlaylistRequest(ZQ::common::NativeThreadPool&  pool , std::string& strPrx,ZQADAPTER_DECLTYPE objAdapter);
	~DestroyPlaylistRequest();
	int run(void);
	void final(int retcode,bool bCancelled);
private:
	std::string											_strPrx;
	ZQADAPTER_DECLTYPE									_objAdapter;
};

class ASyncDestryoPlaylist:protected ZQ::common::ThreadRequest
{
public:
	ASyncDestryoPlaylist(  );
	~ASyncDestryoPlaylist();
private:

};



}}//namespace
#endif
