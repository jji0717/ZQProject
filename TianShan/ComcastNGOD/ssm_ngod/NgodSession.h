
#ifndef _tianshan_ngod_session_implement_header_file_h__
#define _tianshan_ngod_session_implement_header_file_h__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include "./Ice/ngod.h"
#include "ClientRequest.h"
#include "AnnounceRequest.h"
#include "TianShanDefines.h"

class StreamerSelection;

namespace NGOD
{

class NgodEnv;
class NgodSessionManager;

class NgodSessionI : public NGOD::NgodSession, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	NgodSessionI( NgodEnv& env , NgodSessionManager& manager );
	virtual ~NgodSessionI(void);

	void					create( const std::string& sessId , const std::string& odSessId );

public:

	virtual void			updateTimer(const ::Ice::Current& = ::Ice::Current()) ;
	virtual void			onTimer(const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::std::string	getSessionId(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual ::std::string	getOndemandSessId(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual void			updateC1ConnectionId(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::Ice::Int		processRequest(const ::NGOD::NgodClientRequestPtr&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual ::Ice::Int		onStreamSessionEvent(::NGOD::StreamEventRoutine, const ::NGOD::StreamEventAttr&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual int				destroy( const ::NGOD::NgodClientRequestPtr& request, const ::Ice::Current& = ::Ice::Current() );
	virtual void			onRestore(const ::Ice::Current& = ::Ice::Current());
	
	void					errlog(  const NgodClientRequestIPtr& request , int errCode , const char* fmt ... );

	void					recordEvent_StreamCtrl( const std::string& targetState , const std::string& newState , const std::string& strNptStart , const std::string& scale , const std::string& streamResourceId, const std::string& userTime, int status = 200, const std::string& reqRange = "", const std::string& reqScale = "" );
	void					recordEvent_StreamEnd( const std::string& reason , const std::string& strNpt , const std::string& stopIdx );
	void					recordEvent_ItemStepped( const NGOD::StreamEventAttr& a);
	void					recordEvent_Reposition( const NGOD::StreamEventAttr& a);

	int64					adjustNptToAssetBased( int index , int64 npt );

	//support UserEvent
	bool					getPlayListInfo(std::string& strSpeed, std::string& strNPT, std::string& strAssetCtrlNum, std::string& strNewState);

	void					rebindStreamSession( const TianShanIce::Streamer::PlaylistPrx& stream, const std::string& streamSessId, const std::string& streamerNetId,  const ::Ice::Current& = ::Ice::Current());

protected:
	
	struct StreamSessionInfo 
	{
		Ice::Int		npt;
		Ice::Int		playTime;
		std::string		scale;
		Ice::Int		assetIndex;
		Ice::Int		assetNpt;
		StreamSessionInfo()
		{
			npt = 0 ;
			playTime = 0;
			assetIndex = 0;
			assetNpt = 0;
		}
	};

	///helper function
	virtual void			convertToCtxData(::NGOD::ctxData& data, const ::Ice::Current& = ::Ice::Current()) ;

	void					pingStreamSession( StreamSessionInfo& info , int64& npt);

protected:

	int						processSetup( const NgodRequestSetupPtr& request );
	int						processPlay( const NgodRequestPlayPtr& request );
	int						processPause( const NgodRequestPausePtr& request );
	int						processGetParam( const NgodRequestGetParameterPtr& request );
	int						processTeardown( const NgodRequestTeardownPtr& request );
	int						processAnnounceResp( const NgodRequestAnnounceResponsePtr& request );

	void					updateConnectionAndVerCode( const NGOD::NgodClientRequestPtr request );

	bool					isEventIndexValid( ::NGOD::StreamEventRoutine r, const ::NGOD::StreamEventAttr& a);

	void					prepareAnnounceStreamInfo( const ::NGOD::StreamEventAttr& a , StreamAnnounceInfo& info );
//info get data from a.props 
	void					getEventItemStepProps(const ::NGOD::StreamEventAttr& a , StreamAnnounceInfo& info );
protected:

	void					updateSessState( const TianShanIce::State& state );

	int32					sessAnnounceSeq( );
	int32					sessExpiredCount();

	
	int						getStreamSessionInfo( const NgodClientRequestIPtr& request , StreamSessionInfo& info );
	void					storeCurrentSsInfo( const StreamSessionInfo& info );
	bool					getCachedSsInfo(StreamSessionInfo& info, int64& stampAsOf);
	
public:
	virtual          int64 primaryToNpt(int64 npt, bool byPrimary, const char* sess=NULL); // about the stupid conversion between true npt and primary npt
	virtual          int64 nptToPrimary(int64 npt, bool byPrimary, const char* sess=NULL); // about the stupid conversion between true npt and primary npt

protected:

	virtual          void   fixupPlayParams(NgodRequestPlayPtr& request);

	TianShanIce::Streamer::StreamPrx	createStream( const NgodRequestSetupPtr& request , const TianShanIce::SRM::ResourceMap& resources , const StreamerSelection& streamSel );
	bool					renderStream( const NgodRequestSetupPtr& request , const StreamerSelection& streamerSel ,bool& bSkipVolume );	
	bool					getStreamSourceAddressInfo( const NgodRequestSetupPtr& request, SetupResponsePara& para );

	void					recordEvent( const SessionEventRecord& record );

	bool					getSetupStartPoint( Ice::Int& index , Ice::Long& offset );

	void					updateSetupStartPoint( Ice::Int index , Ice::Long offset );

	bool					isSessionStreaming();

	bool					getStreamState( TianShanIce::Streamer::StreamState& state );

	void					prepareAnnounceSessInfo( StreamAnnounceInfo& info );

	bool					recordStreamDestroyStatusAndUpdateTimer( bool update = false );

private:

	NgodEnv&				mEnv;
	NgodSessionManager&		mSessManager;
};
typedef IceInternal::Handle<NgodSessionI> NgodSessionIPtr;

std::string streamEventAttrToString(const ::NGOD::StreamEventRoutine r , const ::NGOD::StreamEventAttr& a );

}//namespace NGOD

#define PROP_CLIENT_USERAGENT	SYSKEY(UserAgent)

#endif //_tianshan_ngod_session_implement_header_file_h__

