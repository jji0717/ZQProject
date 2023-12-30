#ifndef __dsmcc_gateway_plugin_definition_header_file_h__
#define __dsmcc_gateway_plugin_definition_header_file_h__

#include <ZQ_common_conf.h>
#include <string>
#include <Log.h>
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TianShanIce.h>
#include <TsStreamer.h>
#include <TsSRM.h>
#include <TsApplication.h>

#include <TianShanDefines.h>

#include "clientrequest.h"
#include "DsmccDefine.h"
#include <lsc_common.h>

namespace ZQ
{
namespace CLIENTREQUEST
{

enum GATEWAYCOMMAND
{
	COMMAND_NULL,
	
	COMMAND_SETUP,
	COMMAND_SETUP_RESPONSE,

	COMMAND_DESTROY,
	COMMAND_DESTROY_RESPONSE,

	COMMAND_PLAY,
	COMMAND_PLAY_RESPONSE,

	COMMAND_JUMP,
	COMMAND_JUMP_RESPONSE,

	COMMAND_RESUME,
	COMMAND_RESUME_RESPONSE,

	COMMAND_RESET,
	COMMAND_RESET_RESPONSE,

	COMMAND_PAUSE,
	COMMAND_PAUSE_RESPONSE,

	COMMAND_STATUS,
	COMMAND_STATUS_RESPONSE,

	COMMAND_SESS_IN_PROGRESS,
	COMMAND_RELEASE_INDICATION,
	COMMAND_RELEASE_RESPONSE,

	COMMAND_PROCEEDING_INDICATION,

	COMMAND_DONE_RESPONSE,//send out done lscp message

	COMMAND_MAXCOMMAND,
};

class Message: public IceUtil::Shared
{
public:
	virtual ~Message(){}

	virtual GATEWAYCOMMAND	getCommand( ) const = 0 ;

	virtual const std::map<std::string,std::string>& getProperties( ) const = 0;
};

typedef IceUtil::Handle<Message> MessagePtr;

class WritableMessage: public Message
{
public:
	
	virtual void			setCommand( GATEWAYCOMMAND cmd ) = 0;

	virtual void			setProperties( const std::map<std::string,std::string>& props ) = 0;

	virtual void			setResources( const ZQ::DSMCC::DsmccResources& resources ) = 0;
};

typedef IceUtil::Handle<WritableMessage> WritableMessagePtr;


class Response : public IceUtil::Shared
{
public:
	// TODO: every response instance must HOLD a reference to gatewaysession, or else 
	//       the session may be evicted out from memory
	virtual ~Response(){}

	/// post response data back to client
	/// false if some thing wrong happened
	virtual bool complete(uint32 resultCode) =0;

	virtual WritableMessagePtr getMessage( ) = 0;

};

typedef IceUtil::Handle<Response> ResponsePtr;

class Request : public IceUtil::Shared
{
public:
	virtual ~Request(){}
	
	virtual int64		getConnectionId( ) const = 0;

	virtual bool		getLocalInfo( std::string& ip, std::string& port ) const = 0 ;
	virtual bool		getPeerInfo( std::string& ip, std::string& port ) const = 0 ;

	virtual	int64		getRequestInitTime( ) const = 0 ; // ZQ::common::now()
	virtual int64		getRequestStartRunningTime( ) const = 0;// ZQ::common::now()	

	virtual ResponsePtr	getResponse( ) = 0 ;

	virtual MessagePtr	getMessage( ) = 0;
};

typedef IceUtil::Handle<Request> RequestPtr;


class ServerRequest : public IceUtil::Shared
{
public:
	///ServerRequest should have a reference to GwSession
	virtual ~ServerRequest(){}

	virtual WritableMessagePtr getMessage( ) = 0 ;

	virtual void	updatePeerInfo( const std::string& ip, const std::string& port ) = 0;

	///post data to client through the connection on which the announce was created
	virtual bool	complete() = 0;

};
typedef IceUtil::Handle<ServerRequest> ServerRequestPtr;


enum RequestProcessPhase
{
	PHASE_NULL,
	PHASE_FIXUP_REQUEST,
	PHASE_CONTENT_HANDLER,
	PHASE_FIXUP_RESPONSE,
};
enum ProcessResult
{
	RESULT_PROCESSED,
	RESULT_PHASE_DONE,
	RESULT_DONE,
};
/// These three function should be implemented by plugin
/// For a request issued by client, 3 stage must be 
typedef ProcessResult (*FIXUPREQUEST)( RequestPtr request );
typedef ProcessResult (*CONTENTHANDLER)( RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess  );
typedef ProcessResult (*FIXUPRESPONSE)( RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess );

typedef void (*ONRESTORE)( TianShanIce::ClientRequest::SessionPrx sess );
typedef void (*ONTIMER)( TianShanIce::ClientRequest::SessionPrx sess);

class Gateway
{
public:
	virtual ~Gateway(){}
	
	// create gateway session via sessId and clientId
	// @param sessId id of the new session, if this is a empty string, gateway will generate a new id for you
	// @param clientId client identifier for creating the session instance, can be an empty string
	// @return the new gateway session instance proxy, null if failed.
	// NOTE: if you pass a sessId which has already been existed in gateway, null is returned
	virtual TianShanIce::ClientRequest::SessionPrx	createSession( const std::string& sessId , const std::string& clientId ) = 0;

	// open a gateway session by session id	
	virtual TianShanIce::ClientRequest::SessionPrx	openSession( const std::string& id ) = 0;

	// retrieve all relative session's id in a vector according to client id
	virtual std::vector<TianShanIce::ClientRequest::SessionPrx>		findSessionsByClient( const std::string& clientId ) = 0;

	//destroy session via sessionId
	virtual void				destroySession( const std::string& id ) = 0;

	virtual void				updateTimer( const std::string& sessionId, int64 interval ) = 0;

	virtual uint32				sessionId2StreamHandle( const std::string& sessId ) const= 0 ;

	virtual std::string			streamHandle2SessionId( uint32 streamHandle ) const = 0;

	virtual std::string			streamSessId2ClientSessionId( const std::string& streamSessId ) const = 0;

	// create a server request via connection id which can be got from request instance
	// if the connection is not existed, null is returned
	virtual ServerRequestPtr	createServerRequest( int64 connId , TianShanIce::ClientRequest::SessionPrx sess ) = 0;

	virtual void				postRequest( RequestPtr req , RequestProcessPhase stage ) = 0;

	virtual void				registerFixupRequestStage( FIXUPREQUEST proc ) = 0;
	virtual void				registerContentHandlerStage( CONTENTHANDLER proc, const std::string& handlerName ) = 0;
	virtual void				registerFixupResponseStage( FIXUPRESPONSE proc ) = 0;

	virtual void				unregisterFixupRequestStage( FIXUPREQUEST proc ) = 0;
	virtual void				unregisterContentHandlerStage( CONTENTHANDLER proc, const std::string& handlerName ) = 0;
	virtual void				unregisterFixupResponseStage( FIXUPRESPONSE proc ) = 0;

	virtual void				registerOntimerProc( ONTIMER ) = 0;
};

// name of the module initialize should be "ModuleInit"
typedef bool (*MODULEINIT)( Gateway& gw, ZQADAPTER_DECLTYPE objAdapter, const char* configpath, const char* loggerpath );

// name of the module uninitialize should be ModuleUninit
typedef bool (*MODULEUNINIT)( Gateway& gw );


#define SESS_RESERVE_KEY(x)	".reserved_"#x

#define SESS_PROP_REMOTE_IP		SESS_RESERVE_KEY(remoteip)
#define SESS_PROP_REMOTE_PORT	SESS_RESERVE_KEY(remoteport)
#define SESS_PROP_STREAMHANDLE	SESS_RESERVE_KEY(streamHandle)
#define SESS_PROP_STREAMSESSIONID SESS_RESERVE_KEY(streamSessionId)
#define SESS_PROP_LSC_UDP	    SESS_RESERVE_KEY(LscUDP)
#define SESS_PROP_LSC_TCP	    SESS_RESERVE_KEY(LscTCP)
#define SESS_PROP_LSC_CONNID    SESS_RESERVE_KEY(LscConnId)
#define SESS_PROP_DSMCC_CONNID  SESS_RESERVE_KEY(DsmccConnId)

}}//namespace ZQ::DSMCC


#endif//__dsmcc_gateway_plugin_definition_header_file_h__
