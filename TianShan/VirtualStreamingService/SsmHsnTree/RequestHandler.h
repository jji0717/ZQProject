#ifndef __HSNTree_RequestHandler_H__
#define __HSNTree_RequestHandler_H__

#include "Environment.h"
#include "SessionContextImpl.h"

namespace HSNTree
{
#define ClientRequestPrivateDataPrefix "ClientRequest."

	class SmartRequestHandler;
	class RequestHandler : public Ice::Object
	{
		friend class Environment;
		friend class SmartRequestHandler;

	public:
		
		typedef IceUtil::Handle<RequestHandler> Ptr;

		// to check if can post response
		bool needModifyResponse() {	return _bNeedModifyResponse;	} 
		
		// used to post the response to client
		void postResponse() {	_pResponse->post();	}
		
		enum ReturnType
		{
			RETURN_SYNC,
			RETURN_ASYNC
		};
		void setReturnType( ReturnType type );

		ReturnType getReturnType( ) const ;

		Ice::Long getStartTime( ) const;

		std::string dumpTSStreamInfo( const ::TianShanIce::Streamer::StreamInfo& info ) const;

	protected:
		Ice::Long									_startTime;
		
		ReturnType									_returnType;

		// constructor and desstructor
		RequestHandler(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~RequestHandler();

		std::string getRequestHeader(const char* pHeaderStr, int logLevel = DebugLevel);
		void getContentBody();
		std::string getResponseHeader(const char* pHeaderStr);
		void setResponseHeader(const char* pHeaderStr, const char* strValue);

		void composeErrorResponse(const char* errorInfo = NULL);		
		void composeRightResponse();

		std::string getUrl();

		std::string getReqIdent() {	return _reqIdent;	}
		std::string getPhase() {	return _phase;	}
		std::string getSession() {	return _session;	}
		std::string getMethod() {	return _method;	}
		std::string getSequence() {	return _sequence;	}		

	protected: 
		char										_szBuf[2048]; // char buffer for request
		
		std::string									_reqContent; // stores the request's content body
		std::string									_session; // stores client session id
		std::string									_method; // current request type
		std::string									_sequence; // current request sequence
		std::string									_reqIdent; // request identity
		std::string									_phase;
		// the status line in a sucessful response.
		// will be initialized in constructor with the value of "RTSP/1.0 200 OK"
		std::string									_ok_statusline;
		std::string									_error_statusline;
		
		Environment&								_env; // environment
		
		IStreamSmithSite*							_pSite; // site object
		IServerResponse*							_pResponse; // response object
		IClientRequest*								_pRequest; // request object
				
		bool										_bNeedModifyResponse; // indicates whether to post the response after current phase has done or not
		bool										_bProcessSuccessfully; // indicates whether the request has been processed successfully
		
		Ice::Long									_usedTime; // in milli seconds, indicates how long it has taken to process the request

	}; // end class RequestHandler

	class FixupRequest : public RequestHandler
	{
	public:
		typedef IceUtil::Handle<FixupRequest> Ptr;
	protected: 
		FixupRequest(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupRequest();

	public: 
		virtual bool process() = 0;

	protected: 
		IClientRequestWriter*						_pRequestWriter;
		
	}; // class FixupHandler

	class ContentHandler : public RequestHandler
	{
	public: 
		
		typedef IceUtil::Handle<ContentHandler> Ptr;

		ContentHandler(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~ContentHandler();

	public: 

		virtual bool process() = 0;		

	protected: 
		bool renewSession();

		bool getWeiwooPrx();
		bool getStreamPrx();
		bool getPurchasePrx();

		bool getStreamState(TianShanIce::Streamer::StreamState& strmState, std::string& statDescription);

		bool getStreamPlayInfo(std::string& scale, Ice::Int& iCurrentPos, Ice::Int& iTotalPos);
		bool getPlaylistPlayInfo(std::string& scale, Ice::Int& ctrlNum, Ice::Int& offset);

		bool UtcTime2PlayInfo(const std::string& utcTime, ::Ice::Int& ctrlNum, ::Ice::Int& offset, ::Ice::Int& startPos);
		bool PlayInfo2UtcTime(const ::Ice::Int& ctrlNum, const ::Ice::Int& offset, std::string& utcTime);
		
		// session context manager
		bool openSessionCtx(const Ice::Identity& ident);
		bool saveSessionCtx(SessionContextImplPtr pSessionContext);
		bool removeSessionCtx(const Ice::Identity& ident);

		void updateProperty( const std::string& key , const std::string& value );

		std::string getProperty( const std::string& key ) const;

#define		LAST_SCALE_STATUS	SYS_PROP(LastScaleStatus)

	protected: 		

		IClientRequestWriter*						_pRequestWriter;
		
		SessionData									_cltSessCtx; // client session context
		SessionContextPrx							_cltSessPrx; // client session proxy
		TianShanIce::SRM::SessionPrx				_srvrSessPrx; // weiwoo session proxy
		TianShanIce::Streamer::StreamPrx			_streamPrx;
		TianShanIce::Application::PurchasePrx		_purchasePrx;

	}; // class ContentHandler

	class FixupResponse : public RequestHandler
	{
	public:
		
		typedef IceUtil::Handle<FixupResponse> Ptr;

	protected: 
		FixupResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~FixupResponse();

	public: 
		virtual bool process() = 0;

	}; // class FixupResponse

#define HandlerFmt(_C, _X) CLOGFMT(_C, "Req(%s)Sess(%s)Seq(%s)Mtd(%s:%s) " _X), _reqIdent.c_str(), _session.c_str(), _sequence.c_str(), _phase.c_str(), _method.c_str()

} // namespace HSNTree

#endif // #define __HSNTree_RequestHandler_H__

