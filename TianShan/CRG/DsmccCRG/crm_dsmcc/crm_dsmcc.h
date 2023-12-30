#ifndef __Crm_Dsmcc_H__
#define __Crm_Dsmcc_H__

#include "TianShanDefines.h"
#include "gateway_interface.h"
#include "FileLog.h" 
#include <TianShanIceHelper.h>
#include <TianShanIce.h>
#include "crmDmsccCfg.h"
#include <map>
#include <string>
#include <utility>
#include <stdlib.h>
#ifdef ZQ_OS_MSWIN
#include <tchar.h>
#endif
#include "StreamEventSinker.h"
#include "EventSink.h"
#include "../DsmccDefine.h"
#include "SystemUtils.h"

using namespace ZQ::CLIENTREQUEST;
using namespace ZQ::DSMCC;

namespace  ZQ {
namespace CLIENTREQUEST {

class DSMCC_Environment;

class SessionInfoCache
{
public:
	SessionInfoCache( DSMCC_Environment& env );
	virtual ~SessionInfoCache();
	bool	init(int64 timeout  ,int nodeCount  = 16 );
	
	/// update cache streaming information, npt in count in millisecond
	void	updateInfo( const std::string& sessId, int64 npt, float speed);
	void	updateInfo( const std::string& sessId, bool paused );
	bool	getInfo( const std::string& sessId, int64& npt, float& speed );
	void	deleteInfo( const std::string& sessId );
	bool	isSessionExpired( const std::string& sessId );

protected:

	size_t	sessionId2Index( const std::string& sessId );

private:
	DSMCC_Environment&	mEnv;
	struct SessionStreamingInfo
	{
		float	speed;		
		int64	npt;
		int64	lastupdate;
		int64	lasttouch;
		bool	paused;
		SessionStreamingInfo()
		{
			speed	=	0.0f;
			npt		= 0;
			lastupdate = 0;
			lasttouch = 0;
			paused	= false;
		}
	};
	
	typedef std::map<std::string, SessionStreamingInfo> SESSIONINFOCACHEMAP;

	struct CacheMapNode
	{
		SESSIONINFOCACHEMAP	cacheMap;
		ZQ::common::Mutex	locker;
	};
	CacheMapNode*	mNodes;
	int				mNodeCount;
	int64			mExpireInterval;
};
// class DSMCC_Environment
class DSMCC_Environment
{
public:
	// friend class
	typedef IceUtil::Handle<DSMCC_Environment> Ptr ;
	DSMCC_Environment()  ;
	~DSMCC_Environment() {} ;

	// configuration
	bool loadConfig();

	ProcessResult doFixupRequest(RequestPtr request) ;
	ProcessResult doContentHandler(RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess );
	ProcessResult doFixupResponse(RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess );
	void          doExpired(TianShanIce::ClientRequest::SessionPrx sess);
	void          sendCSIR(const ::std::string& playListId, int reasonCode);

	//
	bool doInit(Gateway& gw, ZQADAPTER_DECLTYPE objAdapter, const char* configpath, const char* loggerpath);
	void doUninit();
	//
	bool	loadConfig(const char* confPath );
	bool	initLogger(const char* logfolder );

	ZQ::common::FileLog		mMainLogger;
	ZQ::common::FileLog		mEventLogger;
	ZQ::common::FileLog		mIceLogger;

	void updateLastEventRecvTime(int64 t);
	int64 getLastEventRecvTime() const;

	Gateway*				mGateway ;
	Ice::CommunicatorPtr   _communicator;
	Ice::ObjectAdapterPtr  _objAdapter;
	
	SessionInfoCache		mStreamingInfoCache;

protected:
	
	bool					initEventSink( );

private:
	ZQ::common::SysLog		_sysLog;
	std::string			   _logPath;
	std::string			   _logDir;
	std::string			   _cfgDir;
	std::string			   _configPath;

	bool bQuited ;			
	std::string			   mErrMsg;
	int64					mLastEventActiveTime;
	StreamEventSinker		mStreamEventReceiver;
	EventSinkPtr			mEventSink;
	PlaylistEventSinkIPtr    mPlaylistEventSink;
	PauseTimeoutEventSinkIPtr mPauseTimeoutEventSink;

private:
	void	setErroMsg(const char* fmt , ... );

#ifdef ZQ_OS_MSWIN
	// event source name // event identifier // event category// count of insert strings  // insert strings
	void    reportvents(LPTSTR pszSrcName,DWORD dwEventID,WORD wCategory, WORD cInserts, LPCTSTR *szMsg);    
#endif
    	
};

// class RequestHandler
class RequestHandler : public Ice::Object
{
	friend class clientses_SetupRHandler;
protected: 
	// no public constructor
	RequestHandler(DSMCC_Environment& env, RequestPtr req, TianShanIce::ClientRequest::SessionPrx sess, const char* methodName="");
public:
	typedef IceUtil::Handle<RequestHandler> Ptr;
	typedef std::map<std::string, std::string> Properties;

	virtual ~RequestHandler() ;
	virtual ProcessResult doFixupRequest() =0;
	virtual ProcessResult doContentHandler() = 0;
	virtual ProcessResult doFixupRespone() =0;

	virtual void  postResponse(GATEWAYCOMMAND cmd, uint32 resultCode);
	// virtual void  composeResponse(uint32 resultCode);

	char			_szBuf[2048] ;
	bool			_canProcess;

	Properties loc_resposedata;

private:
protected:
	DSMCC_Environment&	mEnv;
	RequestPtr			_req;
	int64        _connId;

	TianShanIce::ClientRequest::SessionPrx _clientSession;  
	std::string        _method, _clientSessId;

	TianShanIce::Streamer::StreamPrx openAttachedStream();
	TianShanIce::SRM::SessionPrx     openAttachedServerSession();
protected:
	TianShanIce::Streamer::StreamPrx _stream;
	std::string _streamId;
	TianShanIce::Streamer::StreamInfo _streaminfo;

	TianShanIce::SRM::SessionPrx _svrSess;
	std::string _svrSessId ,_streamHandle;
	Ice::Long  _startTime ,_usedTime ;
	ProtocolType _crmProtocolType;
};

//#define HLOGFMT(_X)     LOGFMT("Handle[%s] csess[%s]" _X), _method.c_str(), _clientSessId.c_str()
#define HLOGFMT(_X)     LOGFMT("[%08X]/Handle[%s]/csess[%s]" _X),(uint32)(__THREADID__) ,_method.c_str(), _clientSessId.empty()?"0":_clientSessId.c_str()
#define hlog  (mEnv.mMainLogger)
#define elog  (mEnv.mEventLogger)

// class clientses_SetupRHandler
//	DSM-CC
//1   // refer to RTSP SETUP 
class clientses_SetupRHandler : public RequestHandler 
{
public:
	typedef IceUtil::Handle<clientses_SetupRHandler> Ptr ;
	clientses_SetupRHandler() ;
	clientses_SetupRHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess);
	~clientses_SetupRHandler();
	virtual ProcessResult doFixupRequest();
	virtual ProcessResult doContentHandler() ;
	virtual ProcessResult doFixupRespone();
	//virtual void  composeErrorResponse(const char* errorInfo );
protected:
	void addPrivateData(const std::string& key, TianShanIce::Variant& var);
	void addPrivateData(const std::string& key, const std::string& val);
	bool newServerSession();
	bool postResponse(int16 dsmccErr);
	void postCSPI();
	void getSessionMgr(TianShanIce::SRM::SessionManagerPrx& sessMgrPrx, std::string& sessMgrEp);
protected:  //other class members
	uint16       _dsmccErrorCode;
    ::TianShanIce::ValueMap _privateData;
	//dsmcc HardHeader
	std::string  _reserved1;
	//dsmcc setuprequest
	std::string _assetId, _nodeGroupId, _url, _txnId, _serverId, _assetPl, _assetIdPayLoad;
	ZQ::DSMCC::DsmccResources _dsmccResources;
	::TianShanIce::Properties _confirmParams, _requestParams;
	bool _bSuccess;
	int16 _stupidCodeForHenNan;
private:	

};

// class clientses_InProgressHandler
//	DSM-CC CSIP
class clientses_InProgressRHandler : public RequestHandler 
{
public:
	typedef IceUtil::Handle<clientses_InProgressRHandler> Ptr ;
	clientses_InProgressRHandler();
	clientses_InProgressRHandler(DSMCC_Environment& env, RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess);
	~clientses_InProgressRHandler();

	virtual ProcessResult doFixupRequest() { return RESULT_PROCESSED; }
	virtual ProcessResult doContentHandler() ;
	virtual ProcessResult doFixupRespone() { return RESULT_PROCESSED; }
	//virtual void  composeErrorResponse(const char* errorInfo );

protected:

protected:  //other class members
	uint16  _dsmccErrorCode;
	::TianShanIce::Properties _confirmParams;
};

// class clientses_ReleaseRHandler
// 2  // refer to RTSP TEARDOWN
class  clientses_ReleaseRHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<clientses_ReleaseRHandler>  Ptr ;
	clientses_ReleaseRHandler() ;
	clientses_ReleaseRHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess) ;

	~clientses_ReleaseRHandler();

	virtual ProcessResult doFixupRequest();
	virtual ProcessResult doContentHandler() ;
	virtual ProcessResult doFixupRespone();
	// virtual ProcessResult process() ;
	virtual void  composeErrorResponse(uint32 resultCode /* = NULL */);
	virtual void  client_SessionDestroy() ;
	void postCSRI();
private:
	//dsmcc HardHeader
	std::string  _reserved1;
	//dsmcc setuprelease
	std::string  loc_txnId;
	std::string  loc_crmetadatasessionId ;      
	std::string  loc_crmetadatasessionReason; 

	//std::map<std::string,std::string> loc_resposedata ;
	std::string _retContent ;

};

// class clientses_ReleaseResponseHandler      ---do nothing
//4 
class  clientses_ReleaseResponseHandler : public RequestHandler 
{
public:
	typedef IceUtil::Handle<clientses_ReleaseResponseHandler> Ptr ;
	clientses_ReleaseResponseHandler() ;
	clientses_ReleaseResponseHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess) ;

	~clientses_ReleaseResponseHandler() ;
	virtual ProcessResult doFixupRequest();
	virtual ProcessResult doContentHandler();
	virtual ProcessResult doFixupRespone();

	//virtual void  composeErrorResponse(const char* errorInfo /* = NULL */);
	//void initMdMember_data() ;
	//long int max_gyue(long int x,long int y) ;
	//void  toFenshu(float& flPar,std::string &Numerator ,std::string& Denominator ) ;
protected:  
public:  	
private:	
};

////////////////////////////////////////////////////////////////////////////////////
//  LSCP
////////////////////////////////////////////////////////////////////////////////////
// class lscp_PlayHandler  LSC_PLAY and LSC_RESUME 
class  lscp_PlayHandler : public RequestHandler //LSC_RESUME
{
	//...
public:
	typedef IceUtil::Handle<lscp_PlayHandler> Ptr;
	lscp_PlayHandler() ;
	lscp_PlayHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess) ;
	~lscp_PlayHandler() ;
	virtual ProcessResult doFixupRequest();
	virtual ProcessResult doContentHandler() ;
	virtual ProcessResult doFixupRespone();

	virtual void  composeErrorResponse(uint32 resultCode);
	void initMdMember_data() ;
	static long int max_gyue(long int x,long int y) ;
	static void  toFenshu(float& flPar,std::string &Numerator ,std::string& Denominator ) ;

private:

	std::string  md_PlayStartNpt	  ;
	std::string  md_PlayStopNpt	  ;
	std::string  md_PlayNumerator	  ;
	std::string  md_PlayDenominator ;

	std::string  md_header_LStreamHandle;
	std::string  md_header_LTransactionId;
	std::string  md_header_LStatusCode;
	std::string  md_header_LVersion;
	std::string  md_header_LOpCode;

	//LSC_Response
	std::string  md_res_CurrentNpt ;
	std::string  md_res_Numerator;
	std::string  md_res_Denominator;
	std::string  md_res_Mode ;
	uint32  mdecimalint;
//	char    mdecimalstr[32] ;
	
protected:  
	bool fixupScale( float& requestScale );
public:  	
};

// -----------------------------
// class lscp_PauseHandler
// -----------------------------
class lscp_PauseHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<lscp_PauseHandler> Ptr;
	lscp_PauseHandler() ;
	lscp_PauseHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess) ;
	~lscp_PauseHandler() ;
	virtual ProcessResult doFixupRequest();
	virtual ProcessResult doContentHandler() ;
	virtual ProcessResult doFixupRespone();
	virtual void  composeErrorResponse(uint32 resultCode /* = NULL */);
	void initMdMember_data() ;

private:
	//
	std::string  pa_header_LStreamHandle;
	std::string  pa_header_LTransactionId;
	std::string  pa_header_LStatusCode;
	std::string  pa_header_LVersion;
	std::string  pa_header_LOpCode;
	//
	std::string  ps_ResponseCurrentNpt;	
	std::string  ps_ResponseNumerator ;
	std::string  ps_ResponseDenominator;
	std::string  ps_ResponseMode;
	//
	std::string  pa_md_PauseStopNpt ;

protected:  

public:  	

};

// -----------------------------
// class lscp_StatusHandler
// -----------------------------
class  lscp_StatusHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<lscp_StatusHandler> Ptr ;
	lscp_StatusHandler() ;
	lscp_StatusHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess);
	~lscp_StatusHandler(); 

	virtual void  composeErrorResponse(uint32 resultCode /* = NULL */);
	void initMdMember_data() ;

	virtual ProcessResult doFixupRequest();
	virtual ProcessResult doContentHandler() ;
	virtual ProcessResult doFixupRespone();

protected:

	ProcessResult getStreamingInfo( bool& bContinue );

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

public:  	
	
private:
	StreamSessionInfo status_SSInfo ;
	//
	std::string status_header_LStreamHandle ;
	std::string status_header_LTransactionId   ;
	std::string status_header_LStatusCode  ;
	std::string status_header_LVersion  ;
	std::string status_header_LOpCode  ;

	//
	std::string status_ResponseCurrentNpt ;
	std::string status_ResponseNumerator ;
	std::string status_ResponseDenominator ;
	std::string status_ResponseMode ;
	bool		mbInfoFromCache;
	
	//
};

} } //namespaces
#endif //__Crm_Dsmcc_H__
