#ifndef _tianshan_ngod_announce_header_file_h__
#define _tianshan_ngod_announce_header_file_h__

#include "StreamSmithModule.h"
#include "ClientRequest.h"
#include "NgodHelper.h"

namespace NGOD
{

class NgodEnv;
class NgodSessionI;

class AnnounceRequest;

typedef IceUtil::Handle<AnnounceRequest> AnnounceRequetPtr;

class AnnounceRequest : public Ice::LocalObject
{
public:
	AnnounceRequest( NgodEnv& env , NgodSessionManager& manager );
	virtual ~AnnounceRequest(void);
public:
	
	bool		init( const std::string& sessId , const std::string& connId );

	void		setStartline( const std::string& startLine );
	void		setHeader( const std::string& key , const std::string& value );
	void		setHeader( const std::string& key , int32 value );
	void		setHeader( const std::string& key , int64 value );
	void		setHeader( const std::string& key , float value );
	void		setBody( const std::string& body );
	bool		post( );

	void		attach( IServerRequest* serverRequest );

protected:

	void		updateStartline( const std::string& url );
	void		updateNotice( const std::string code , const std::string& reason , const std::string& offset );
	void		updateRequire( bool bR2 , NgodProtoclVersionCode verCode );

protected:

	NgodEnv&				mEnv;
	NgodSessionManager&		mSessManager;
	IServerRequest*			mServerRequest;
	std::string				mNoticeStr;
};

struct StreamAnnounceInfo 
{
	std::string					connectionId;
	NgodProtoclVersionCode		r2VerCode;
	NgodProtoclVersionCode		c1VerCode;
	int64						npt;
	int32						seq;
	float						scale;
	TianShanIce::Streamer::StreamState		state;
	std::string					sessId;
	std::string					odsessId;
	std::string					groupId;
	std::string					originalUrl;
	std::string					serverIp;
	std::string                 primaryItemNPT;

//data in props
	std::string					nptPrimary;
	std::string					curPAssetId;
	std::string					curProviderId;
	std::string					prevPAssetId;
	std::string					prevProviderId;
	std::string					ItemCurName;
	std::string					ItemPrevName;
	std::string					curflags;
	std::string					prevflags;
	std::string					curdur;
	std::string					prevdur;
	std::string					perRequest;
	std::string					totalVideodur;
	std::string                 extItemInfo;
	std::string					totalDur;
};

class AnnounceEndOfStream;
typedef IceUtil::Handle<AnnounceEndOfStream> AnnounceEndOfStreamPtr;
class AnnounceEndOfStream : public AnnounceRequest
{
public:
	AnnounceEndOfStream( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceEndOfStream();

public:

	bool	postAnnounce(  const StreamAnnounceInfo&  info );
	
};

class AnnounceBeginOfStream;
typedef IceUtil::Handle<AnnounceBeginOfStream> AnnounceBeginOfStreamPtr;
class AnnounceBeginOfStream :public AnnounceRequest
{
public:
	AnnounceBeginOfStream( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceBeginOfStream();
public:
	bool	postAnnounce(  const StreamAnnounceInfo&  info );
};

class AnnounceScaleChange;
typedef IceUtil::Handle<AnnounceScaleChange> AnnounceScaleChangePtr;
class AnnounceScaleChange : public AnnounceRequest
{
public:
	AnnounceScaleChange( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceScaleChange();

public:
	
	bool postAnnounce( const StreamAnnounceInfo& info );
};

class AnnounceStateChange;
typedef IceUtil::Handle<AnnounceStateChange> AnnounceStateChangePtr;
class AnnounceStateChange : public AnnounceRequest
{
public:
	AnnounceStateChange( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceStateChange();
public:

	bool postAnnounce( const StreamAnnounceInfo& info );

};

struct TransitionAnnounceInfo 
{
	int		prevCtrlNum;
	int		currentCtrlNum;
};
class AnnounceTransition;
typedef IceUtil::Handle<AnnounceTransition> AnnounceTransitionPtr;
class AnnounceTransition : public AnnounceRequest
{
public:
	AnnounceTransition( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceTransition();
public:
	bool postAnnounce( const StreamAnnounceInfo& info , const TransitionAnnounceInfo& transition );

protected:
	void outputTransition( const StreamAnnounceInfo& info, const TransitionAnnounceInfo& transition );
};

class AnnounceSkipItem : public AnnounceRequest
{
public:
	AnnounceSkipItem( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceSkipItem();
public:

	bool postAnnounce( const StreamAnnounceInfo& info, const TransitionAnnounceInfo& transition );

};

class AnnounceTrickNoConstrained : public AnnounceRequest
{
public:
	AnnounceTrickNoConstrained( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceTrickNoConstrained();
public:

	bool postAnnounce( const StreamAnnounceInfo& info, const TransitionAnnounceInfo& transition );

};

class AnnounceTrickConstrained : public AnnounceRequest
{
public:
	AnnounceTrickConstrained( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceTrickConstrained();
public:

	bool postAnnounce( const StreamAnnounceInfo& info, const TransitionAnnounceInfo& transition );

};

struct ErrorAnnounceInfo 
{

};
class AnnounceError;
typedef IceUtil::Handle<AnnounceError> AnnounceErrorPtr;
class AnnounceError : public AnnounceRequest
{
public:
	AnnounceError( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceError();

public:
	
	void setErrorReason( const std::string& reason );
	bool postAnnounce( const StreamAnnounceInfo& info , const std::string& noticeCode , const std::string& noticeReason );
private:
	std::string mErrorReason;
};

class AnnounceInprogress : public AnnounceRequest
{
public:
	AnnounceInprogress( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceInprogress();

public:
	
	bool postAnnounce( const StreamAnnounceInfo& info );

};

class AnnounceTerminate : public AnnounceRequest
{
public:
	AnnounceTerminate( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnounceTerminate();

public:

	bool postAnnounce( const StreamAnnounceInfo& info );

};

class AnnouncePauseTimeout : public AnnounceRequest
{
public:
	AnnouncePauseTimeout( NgodEnv& env  , NgodSessionManager& manager );
	virtual ~AnnouncePauseTimeout();
public:
	bool postAnnounce( const StreamAnnounceInfo& info );
};

}//namespace NGOD

#endif//_tianshan_ngod_announce_header_file_h__
