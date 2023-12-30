
#ifndef _ZQ_StreamService_Stream_implement_header_file_h__
#define _ZQ_StreamService_Stream_implement_header_file_h__

#include <NativeThreadPool.h>
#include "SsStreamBase.h"
#include "EventSender.h"
#include "SsServiceImpl.h"
#include <time.h>

namespace ZQ
{
namespace StreamService
{

struct StreamParams;

class SsStreamImpl : public SsStreamBase , public SsContext
{
public:
	typedef IceUtil::Handle<SsStreamImpl> Ptr;

	SsStreamImpl( SsServiceImpl& serviceImpl ,
		SsEnvironment* environment ,
		const TianShanIce::Properties& res ,
		const Ice::Identity& id );
	SsStreamImpl(SsServiceImpl& serviceImpl , SsEnvironment* environment );
	virtual ~SsStreamImpl( );

	void			updateTimer( );

public:

	/*SsPlaylist*/
	virtual void onSessionStateChanged(::TianShanIce::Streamer::StreamState,  const std::string&, ::Ice::Long, const TianShanIce::Properties& props, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void onSessionSpeedChanged(::Ice::Float,  const std::string&, ::Ice::Long, const TianShanIce::Properties& props, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void onSessionExpired( const std::string&, ::Ice::Long, const TianShanIce::Properties& props,const ::Ice::Current& = ::Ice::Current()) ;
	virtual void onSessionProgress(const ::std::string&, ::Ice::Long, ::Ice::Long, const TianShanIce::Properties& props,const ::Ice::Current& = ::Ice::Current()) ;
	virtual void onTimer(::TianShanIce::Streamer::Timertype, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void onRestore(const ::Ice::Current& = ::Ice::Current());
	virtual bool getRunningSession(::std::string&, ::std::string&, ::Ice::Float&, ::TianShanIce::Streamer::StreamState&, const ::Ice::Current& = ::Ice::Current()) const ;
	virtual std::string getAttribute(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const ;
	virtual bool renewTicket( Ice::Int interval , const Ice::Current& = Ice::Current() );
	
	/*playlistEx*/
	virtual void attachSession(const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void setDestination(const ::std::string&, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void setDestMac(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void setSourceStrmPort(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void setPID(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	/*playlist*/
	virtual ::std::string getId(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual bool getInfo(::Ice::Int, ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::Ice::Int insert(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;	
	virtual ::Ice::Int pushBack(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::Ice::Int size(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual ::Ice::Int left(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual bool empty(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual ::Ice::Int current(const ::Ice::Current& = ::Ice::Current()) const;
	virtual void erase(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::Ice::Int flushExpired(const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool clearPending(bool, const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool isCompleted(const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::TianShanIce::IValues getSequence(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual ::Ice::Int findItem(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool distance(::Ice::Int, ::Ice::Int, ::Ice::Int&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool skipToItem(::Ice::Int, bool, const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool seekToPosition(::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void enableEoT(bool, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::TianShanIce::Streamer::StreamInfo playItem(::Ice::Int, ::Ice::Int, ::Ice::Short, ::Ice::Float, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::TianShanIce::Streamer::PlaylistItemSetupInfoS getPlaylistItems(const ::Ice::Current& = ::Ice::Current()) const ;

	/*Stream*/
	virtual void allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void destroy(const ::Ice::Current& = ::Ice::Current()) ;
	virtual std::string lastError(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual void setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask&, const ::TianShanIce::Streamer::ConditionalControlPrx&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool play(const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool setSpeed(::Ice::Float, const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool pause(const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool resume(const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::TianShanIce::Streamer::StreamState getCurrentState(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual ::TianShanIce::SRM::SessionPrx getSession(const ::Ice::Current& = ::Ice::Current()) ;
	virtual void setMuxRate(::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual bool allocDVBCResource(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::Ice::Long seekStream(::Ice::Long, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::TianShanIce::Streamer::StreamInfo playEx(::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::TianShanIce::Streamer::StreamInfo pauseEx(const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current()) ;
    virtual ::TianShanIce::SRM::ResourceMap getResources(const ::Ice::Current& c) const;

	virtual void destroy2(::TianShanIce::Properties& feedback, const ::Ice::Current& c);

	///AMD function 
	virtual void play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr&, ::Ice::Long, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	virtual void playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr&, ::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());
	virtual void skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr&, ::Ice::Int, bool, const ::Ice::Current& = ::Ice::Current());
	virtual void seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr&, ::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
    virtual void playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr&, ::Ice::Int, ::Ice::Int, ::Ice::Short, ::Ice::Float, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());

protected:
	
	bool			getItemAttribute( iter it );
	
	virtual const std::string& id( ) const;

	virtual void	updateContextProperty( const std::string& key , const std::string& value );
	virtual void	updateContextProperty( const std::string& key , int32 value ) ;
	virtual void	updateContextProperty( const std::string& key , int64 value ) ;
	virtual void	updateContextProperty( const std::string& key , float value ) ;

	virtual void	updateContextProperty( const TianShanIce::Properties& props );

	virtual bool	hasContextProperty( const std::string& key ) const;

	virtual std::string getContextProperty( const std::string& key ) const;

	virtual const TianShanIce::Properties& getContextProperty( ) const;

	virtual const TianShanIce::SRM::ResourceMap&	getContextResources( ) const ;

	virtual void  updateContextResources( const TianShanIce::SRM::ResourceMap& res ) ;

	virtual	void	removeProperty( const std::string& key ) ;

	virtual std::string getStreamingPort ( ) const;

protected:

	void			onTimerDPC( );

	void			onTimerDestroyPlaylist( );

	void			onTimerRenewTicket( );

	void			onTimerLoadItem( );

	void			setTimer( Ice::Long t , TianShanIce::Streamer::Timertype type =  TianShanIce::Streamer::TIMERDPC );

	bool			restorePortalSession( iter it , TianShanIce::Streamer::StreamState& state , float& scale );

protected:

	void			fillEventData( EventData& data , Ice::Int seq , Ice::Int type );
	
	///event generator
	void			fireItemSteppedMsg( constIter itFrom , constIter itTo , Ice::Int curItemOffset = 0 ,const TianShanIce::Properties& props = TianShanIce::Properties()) ;

	void			firePlaylistDoneMsg(  const TianShanIce::Properties& props ) ;

	void			fireScaleChangedMsg( const Ice::Float& scaleOld , const Ice::Float& scaleNew ,const TianShanIce::Properties& props  );
	
	void			fireStateChangedMsg( const TianShanIce::Streamer::StreamState& stateOld , 
									const TianShanIce::Streamer::StreamState& stateNew,
									const TianShanIce::Properties& props );

	void			firePlaylistDestroyMsg( constIter it , const Ice::Int& code , const std::string& reason ,const TianShanIce::Properties& props  );

	void			fireProgressMsg( constIter it , const Ice::Long& curTimeOffset , const Ice::Long& totalDuration ,const TianShanIce::Properties& props );
	
private:
	
	void			onPlaylistDone( const TianShanIce::Properties& props = TianShanIce::Properties() );

	void			updateLocalRecord( const StreamParams& paras , const TianShanIce::Streamer::StreamState& newState );

protected:

	iter			doPlaySession(  iter it, Ice::Short from, Ice::Long timeOffset , Ice::Float scale ,
									const int32& expectMask , TianShanIce::Streamer::StreamInfo& info);

	//only affect 
	iter			doLoadItem(   iter it, 
									Ice::Long timeoffset , 
									Ice::Float scale ,									
									StreamParams& paras,
									std::string& streamId );

	iter			doRepositionItem( iter it ,
										Ice::Long timeoffset , 
										Ice::Float scale , 
										StreamParams& paras );
	iter			doChangeScale( iter it,
									Ice::Float scale , 
									StreamParams& paras );
	
	bool			doPauseSession( iter it , const int32& expectMask , TianShanIce::Streamer::StreamInfo& info );
	
	bool			doDestroySession( iter it );
	bool			doDestroySession( const std::string& sessId );

protected:

	friend class	SsStreamRequest;

	friend class	SsStreamCommitRequest;

	bool			isInEOTArea( ) ; //is current in EOT area

	bool			contentIsInService( constIter it );

	void			updateContentState( iter it , Ice::Int state );
	
	bool			addItemToList( const TianShanIce::Streamer::ItemSessionInfo& newItem , Ice::Int where );

	bool			deleteItem( Ice::Int userCtrlNum );

	bool			stepList( iter itFrom , iter itTo );

	bool			jumpList( Ice::Int targetCtrlNum );
	
	void			throwException( int32 errCode , const std::string& logPrefix);

	std::string		getStreamPort( ) const;

	std::string		getStreamSessId( constIter it ) const;
	void			updateStreamSessId( iter it , const std::string& sessId );

	int32			getExpectParamMask( const TianShanIce::StrValues& expectedProps ) const;
	int32			getExpectParamMask( const int32& mask ) const;


	bool			constructStreamInfo(	const StreamParams& para , 
											const int32& expectMask , 
											iter itEffect,
											TianShanIce::Streamer::StreamInfo& info );	

	
	bool			canChangeScale( const Ice::Float& newSpeed );

	bool			canChangeScaleInNormalPlaylistMode( const Ice::Float& newSpeed );
	
	bool			convertPositionToStreamWide( Ice::Int& userCtrlNum , Ice::Long& offset , Ice::Short& from );
	bool			convertPositionToStreamWide( iter it, Ice::Long& offset , Ice::Short& from );
	bool			convertPositionToStreamWide( Ice::Long& offset , Ice::Short& from  );
	
	iter			convertPositionToItemWide( Ice::Long& offset , Ice::Short& from , bool newDirection );
	iter			convertPositionToItemWide( Ice::Int& userCtrlNum , Ice::Long& offset , Ice::Short& from , bool newDirection );
	iter			convertPositionToItemWide( iter it , Ice::Long& offset , Ice::Short& from , bool newDirection );
	
	bool			convertResultToStreamWide( constIter it , Ice::Long& offset );
	iter			convertResultToItemWide( Ice::Long& offset );
	iter			convertResultToItemWide( iter it , Ice::Long& offset );

	//only used for normal playlist
	bool			primeNextItem( );

	Ice::Long		getSessTimeLeft(  Ice::Long* passed = NULL );

	std::string		dumpStreamParas( const StreamParams& paras );

	void			analyzeInstanceState( );

	iter			findNextCriticalStartItem( time_t& t );

	Ice::Long		calculateDuration( iter itFirst , iter itLast ) ;

private:

	void			clearItemSessionInfo( TianShanIce::Streamer::ItemSessionInfo& info );

};



///AMD base
class SsStreamRequest : public ZQ::common::ThreadRequest
{
public:
	SsStreamRequest( SsStreamImpl::Ptr stream , SsEnvironment* environment , const Ice::Current& c);
	virtual ~SsStreamRequest( )
	{

	}
	
	void execute();	

public:
	
	void			final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}

protected:	
	SsEnvironment*		env;
	Ice::Current		mIceCurrent;
	SsStreamImpl::Ptr	streamImpl;
};


class SsStreamPlayRequest : public SsStreamRequest
{
public:
	SsStreamPlayRequest( SsStreamImpl::Ptr stream,
						SsEnvironment* environment ,
						const Ice::Current& c,
						const ::TianShanIce::Streamer::AMD_Stream_playPtr& callback)
						:SsStreamRequest(stream,environment,c),
						mCallback(callback)
	{

	}
	virtual ~SsStreamPlayRequest( )
	{

	}

private:
	int			run( );
	const ::TianShanIce::Streamer::AMD_Stream_playPtr	mCallback;
	
};

class SsStreamSeekStreamRequest : public SsStreamRequest
{
public:
	SsStreamSeekStreamRequest(  SsStreamImpl::Ptr stream,
								SsEnvironment* environment ,
								const Ice::Current& c,
								const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr& callback ,
								const Ice::Long& offset,
								const Ice::Int& startPos)
								:SsStreamRequest(stream , environment , c),
								mCallback(callback),
								mOffset(offset),
								mStartPos(startPos)
	{

	}
	virtual ~SsStreamSeekStreamRequest( )
	{
	}

private:
	int		run( );
	const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr				mCallback;
	Ice::Long															mOffset;
	Ice::Int															mStartPos;
};

class SsStreamPlayExRequest : public SsStreamRequest
{
public:
	SsStreamPlayExRequest(  SsStreamImpl::Ptr stream,
							SsEnvironment* environment ,
							const Ice::Current& c,
							const ::TianShanIce::Streamer::AMD_Stream_playExPtr& callback,
							const Ice::Float& newSpeed,
							const Ice::Long& offset,
							const Ice::Short& from,
							const TianShanIce::StrValues& expectedProps )
							:SsStreamRequest(stream,environment,c),
							mCallback(callback),
							mNewSpeed(newSpeed),
							mOffset(offset),
							mFrom(from),
							mExpectedProps(expectedProps)
	{

	}
	virtual ~SsStreamPlayExRequest( )
	{

	}

private:

	int			run( );
	const ::TianShanIce::Streamer::AMD_Stream_playExPtr					mCallback;
	Ice::Float															mNewSpeed;
	Ice::Long															mOffset;
	Ice::Short															mFrom;
	TianShanIce::StrValues												mExpectedProps;
};


class SsStreamSkipToItemRequest : public SsStreamRequest
{
public:
	SsStreamSkipToItemRequest( SsStreamImpl::Ptr stream,
								SsEnvironment* environment ,
								const Ice::Current& c,
								const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& callback,
								const Ice::Int& where,
								const bool& bPlay)
								:SsStreamRequest(stream,environment,c),
								mCallback(callback),
								mWhere(where),
								mbPlay(bPlay)
	{

	}
	virtual ~SsStreamSkipToItemRequest( )
	{

	}

private:
	int			run( );
	const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr			mCallback;
	Ice::Int															mWhere;
	bool																mbPlay;
};


class SsStreamSeekToPositionRequest : public SsStreamRequest
{
public:
	SsStreamSeekToPositionRequest(  SsStreamImpl::Ptr stream,
									SsEnvironment* environment ,
									const Ice::Current& c,
									const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& callback,
									const Ice::Int& userCtrlNum ,
									const Ice::Int& timeOffset ,
									const Ice::Int& startPos)
									:SsStreamRequest(stream,environment,c),
									mCallback(callback),
									mUserCtrlNum(userCtrlNum),
									mTimeOffset(timeOffset),
									mStartPos(startPos)
	{

	}

	virtual ~SsStreamSeekToPositionRequest( )
	{

	}

private:
	int			run( );
	const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr		mCallback;
	Ice::Int															mUserCtrlNum;
	Ice::Int															mTimeOffset;
	Ice::Int															mStartPos;
};


class SsStreamPlayItemRequest : public SsStreamRequest
{
public:
	SsStreamPlayItemRequest( SsStreamImpl::Ptr stream,
							SsEnvironment* environment ,
							const Ice::Current& c,
							const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& callback,
							const Ice::Int&	userCtrlNum,
							const Ice::Int& timeOffset ,
							const Ice::Short& from,
							const Ice::Float& newSpeed ,
							const TianShanIce::StrValues& expectedProps )
							:SsStreamRequest(stream,environment,c),
							mCallback(callback),
							mUserCtrlNum(userCtrlNum),
							mTimeOffset(timeOffset),
							mFrom(from),
							mNewSpeed(newSpeed),
							mExpectedProps(expectedProps)
	{

	}
	virtual ~SsStreamPlayItemRequest( )
	{

	}

private:
	int			run( );
	const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr				mCallback;
	Ice::Int															mUserCtrlNum;
	Ice::Int															mTimeOffset;
	Ice::Int															mFrom;
	Ice::Float															mNewSpeed;
	TianShanIce::StrValues												mExpectedProps;
};

class SsStreamCommitRequest : public SsStreamRequest
{
public:
	SsStreamCommitRequest(  SsStreamImpl::Ptr stream , 
							SsEnvironment* environment,
							const Ice::Current& c,
							TianShanIce::Streamer::AMD_Stream_commitPtr callback)
							:SsStreamRequest(stream,environment,c),
							mCallback(callback)
							
	{

	}
	virtual ~SsStreamCommitRequest( )
	{

	}

private:
	int			run(void);
	TianShanIce::Streamer::AMD_Stream_commitPtr							mCallback;
};

#define STREAMWIDE_ATTR_STREAMID	"StreamService.StreamID"

}}//namespace ZQ::StreamService

#endif//_ZQ_StreamService_Stream_implement_header_file_h__

