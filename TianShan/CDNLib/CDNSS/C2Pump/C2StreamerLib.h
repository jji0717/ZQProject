
#ifndef _tianshan_cdnss_c2streamer_lib_header_file_h__
#define _tianshan_cdnss_c2streamer_lib_header_file_h__

#include <ZQ_common_conf.h>
#include <string>
#include <vector>
#include <map>

#include <DataPostHouse/common_define.h>
#include <HttpEngine/HttpEngine.h>
#include <HttpEngine/HttpEngineInterface.h>
#include <Pointer.h>

#include "C2StreamerEnv.h"

namespace C2Streamer
{

const int32 errorCodeOK						= 200;
const int32 errorCodeCreated				= 201;
const int32 errorCodePartialContent			= 206;

const int32 errorCodeBadRequest				= 400;
const int32 errorCodeBadCredentials			= 401;
const int32 errorCodeContentNotFound		= 404;
const int32 errorCodeNotAllowed				= 405;
const int32 errorCodeNotAccpetable			= 406;
const int32 errorCodeRequestTimeout			= 408;
const int32 errorCodeRequestConflict		= 409;
const int32 errorCodeSessionGone			= 410;
const int32 errorCodeBadRange				= 416;
const int32 errorCodeNoShadow               = 417;

const int32 errorCodeInternalError			= 500;
const int32 errorCodeNotImplemented			= 501;
const int32 errorCodeServerUnavail			= 503;
const int32 errorCodeGatewayTimeout			= 504;
const int32 errorCodeVersionNotSupported	= 505;

const int32 errorWorkingInProcess 			= -2;
const int32 errorCodeNotEnoughMemory        = -3;
const int32 errorCodeHeadError              = -4;
const int32 errorCodeFailToOpenFile         = -5;
const int32 errorCodeFailToReadIndex        = -6;
const int32 errorCodeFailToReadContent      = -7;
const int32 errorCodeSkipFileHeader         = -8;
const int32 errorCodeFailToSeek             = -9;
const int32 errorCodeExceed                 = -10;
const int32 errorCodeNotFull                = -11;
const int32 errorCodeEOF                    = -12;



class C2ResponseHandler : virtual public ZQ::common::SharedObject
{
public:
	C2ResponseHandler(){}
	virtual ~C2ResponseHandler(){}
	typedef ZQ::common::Pointer<C2ResponseHandler> Ptr;
public:
	virtual void	updateStartline(int statusCode, const std::string& desc){ }

	virtual void	updateHeader(const std::string& key, const std::string& value){ }

	virtual bool	flushHeader(ssize_t bodySize = -1) { return false; }

	virtual bool	updateContentLength(size_t length) { return false; }

	virtual bool	complete() { return false; }

	virtual std::string lastError() const { return ""; }

	virtual bool	isConnectionBroken() const { return false; }

	virtual void	setConnectionBroken(bool bBroken){ }

	virtual int64	getConnectionId() const { return 0; }

	virtual void	setConnectionId(int64 id) { }

	virtual void	setCommOption(int opt, int value) { }

	virtual LibAsync::EventLoop*	getLoop() const { return NULL; }
public:
	virtual int		addBodyContent(const char* data, size_t size) = 0;
	virtual bool	registerWrite(ZQHttp::IChannelWritable::Ptr cb) = 0;
	virtual bool    setLocalAddr(const std::string& ip, unsigned short port = 0) { return false; }
};

#ifndef ZQ_CDN_UMG
class C2HttpResponseHanlder : public C2ResponseHandler
{
public:
	C2HttpResponseHanlder( ZQHttp::IResponse* handler , ZQHttp::IConnection* conn, LibAsync::EventLoop* loop );
	virtual ~C2HttpResponseHanlder();

public:

	virtual void	updateStartline( int statusCode , const std::string& desc );

	virtual void	updateHeader( const std::string& key , const std::string& value );

	virtual int		addBodyContent( const char* data , size_t size );

	virtual bool	flushHeader( ssize_t bodySize = -1 );
	
	virtual bool	updateContentLength( size_t length);

	virtual bool	complete();
	
	virtual std::string lastError() const;

	virtual bool	isConnectionBroken() const;
	
	virtual void	setConnectionBroken( bool bBroken);
	
	virtual int64	getConnectionId() const;
	
	virtual void	setConnectionId( int64 id );
	
	virtual void	setCommOption(int opt , int value );

	virtual bool	registerWrite( ZQHttp::IChannelWritable::Ptr cb );

	virtual LibAsync::EventLoop*	getLoop() const { return mEventLoop; }

private:
	ZQHttp::IConnection*	mResponseConnection;
	ZQHttp::IResponse*		mResponseHandler;
	bool					mbComplete;
	bool					mbFlushed;
	bool					mbConnBroken;
	int64					mConnId;
	LibAsync::EventLoop*	mEventLoop;
	ZQ::common::Mutex		mLocker;
};

typedef ZQ::common::Pointer<C2HttpResponseHanlder> C2HttpResponseHanlderPtr;
#endif

const char* convertErrCodeToString( int32 errCode );

bool		isSuccessCode( int32 errorCode );

enum SessionState
{
	SESSION_STATE_NULL			= 0,
	SESSION_STATE_IDLE			= 1,
	SESSION_STATE_ACTIVE		= 2,
	SESSION_STATE_DELETED		= 3
};

const char* convertSessionStateToString( const SessionState& state );

enum C2Method
{
	METHOD_NULL					= 0,
	METHOD_TRANSFER_INIT		= 1 << 0,
	METHOD_TRANSFER_TERM		= 1 << 2,
	METHOD_TRANSFER_RUN			= 1 << 3,
	METHOD_SESSION_STATUS		= 1 << 4,
	METHOD_RESOURCE_STATUS		= 1 << 5,

	METHOD_IC_UPDATE			= 1 << 6,
	METHOD_SESS_UPDATE			= 1 << 7,
	METHOD_RES_UPDATE			= 1 << 8,

	METHOD_HLS_GET				= 1 << 9,
	METHOD_UDP_INIT             = 1 << 10,
	METHOD_UDP_RUN		        = 1 << 11,
	METHOD_UDP_EVENT			= 1 << 12
};

struct SessionProperty {
	bool	queryIndex;

	SessionProperty() {
		queryIndex = true;
	}
};

const char* convertC2MethodToString( const C2Method& method );

struct RequestParam : virtual public ZQ::common::SharedObject
{
	std::string			requestHint;

	C2Method			method;
	std::string			sessionId;

	bool				connBroken;	// if request connection is broken, no more action should be performed on this request

	LibAsync::EventLoop*	evtLoop;

	int64           transferRate;

	SessionProperty		sessProp;

	const ConfWriter*	configWriter;
	const ConfUrlRule*	configUrlRule;

	C2StreamerEnv&		c2env;

	RequestParam( C2StreamerEnv& env )
		:c2env(env) {
		method		= METHOD_NULL;
		connBroken	= false;
		evtLoop		= NULL;
		configWriter = NULL;
		configUrlRule = NULL;
		transferRate = 0;
	}

	RequestParam( C2StreamerEnv& env, const std::string& urlPrefix )
		:c2env(env){
		method		= METHOD_NULL;
		connBroken	= false;
		evtLoop		= NULL;
		configWriter = NULL;
		configUrlRule = NULL;
		std::string prefix = urlPrefix;
		const ConfPerSessionConfig* perSessConf = env.mConfig.getPerSessConf( prefix);
		if(!perSessConf) {
			if(prefix != "*") {
				perSessConf = env.mConfig.getPerSessConf("*");
			}
			if(!perSessConf) {
				perSessConf = env.mConfig.getFirstPerSessConf();
			}
		}
		if(perSessConf) {
			configWriter = &(perSessConf->writer);
			configUrlRule = &(perSessConf->urlRule);
			env.getLogger()->debug("get reader[%d] for prefix[%s]", configUrlRule->readerType, prefix.c_str() );
		} else {
			assert( false && "no default per session config");
		}
	}

	bool setConfigViaUrl( const std::string& url ) {
		const ConfPerSessionConfig* perSessConf = c2env.mConfig.getPerSessConf( url );
		if(!perSessConf) {
			c2env.getLogger()->error("could not find suitable url config for [%s]", url.c_str());
			return false;
		}
		configWriter = &(perSessConf->writer);
		configUrlRule = &(perSessConf->urlRule);
		c2env.getLogger()->debug("get reader[%d] for url[%s]", configUrlRule->readerType, url.c_str() );
		return true;
	}

	const SessionProperty& prop() const {
		return sessProp;
	}

	virtual ~RequestParam()	{
	}

	const ConfWriter* getConfWriter() const {
		return configWriter;
	}

	const ConfUrlRule* getConfUrlRule() const {
		return configUrlRule;
	}
};

typedef ZQ::common::Pointer<RequestParam> RequestParamPtr;

class C2HttpHandler;
struct RequestResponseParam : public ZQ::common::SharedObject
{
	int32						errorCode;
	std::string					errorText;
#ifndef ZQ_CDN_UMG
	//C2HttpResponseHanlderPtr	responseHandler;
	C2ResponseHandler::Ptr		responseHandler;
#endif
	C2HttpHandler*				httpHandler;
	bool						isSuccess() const
	{
		return errorCode >= 200 && errorCode <= 299;
	}

	/// response invoke C2HttpResponseHanlder::
	bool 					response( );

	void					setLastErr( int32 errCode , const char* fmt , ... );
	void					setLastErr( const RequestParamPtr& request , int32 errCode , const char* fmt , ... );

	RequestResponseParam()
	{
		errorCode = 200; 
		httpHandler = NULL;
	}
	virtual ~RequestResponseParam(){}
};

typedef ZQ::common::Pointer<RequestResponseParam> RequestResponseParamPtr;


struct TransferRange 
{
	bool		bStartValid;
	bool		bEndValid;
	int64		startPos;
	int64		endPos;
	TransferRange()
	{
		bStartValid = false;
		bEndValid	= false;
		startPos	= 0;
		endPos		= 0;
	}
	std::string toString() const;
	int64		rangeSize() const;
    bool parse(const std::string&);
};

enum SESSION_TYPE
{
	SESSION_TYPE_NULL		= 0,
	SESSION_TYPE_NORMAL		= 1,	//普通青年， 一切照旧
	SESSION_TYPE_SHADOW		= 2,	//文艺青年， 假的session，不发出任何的event ? 自动timeout
	SESSION_TYPE_COMPLEX	= 3,	//2B青年，   需要支持在一个session中可以下载多个文件

	SESSION_TYPE_SCHANGEREQ	= 1 << 17	//火星人
};

#define BASE_SESSION_TYPE(x) ((x)&0xFFFF)

struct UdpStreamInitRequestParam : public RequestParam 
{
	std::string 	udpClientAddress;
	unsigned short 	udpClientPort;

	std::string     udpServerAddress;
	//unsigned short 	udpServerPort;
	
	std::string fileName;
	TransferRange  	requestRange;
	int32           transferDelay;
	int32           requestType;
	int32           udpTimeOut;
	int64          ingressCapacity;
	std::string		userAgent;

	UdpStreamInitRequestParam(C2StreamerEnv& env)
	:RequestParam(env) {
		udpClientPort = 0;
		//udpServerPort = 0;
		fileName    = "";
		transferDelay = 0;
		requestType = SESSION_TYPE_NORMAL;
		udpTimeOut = 300000;
		ingressCapacity = 0;
	}

	UdpStreamInitRequestParam(C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix) {
		udpClientPort = 0;
		//udpServerPort = 0;
		fileName    = "";
		transferDelay = 0;
		requestType = SESSION_TYPE_NORMAL;
		udpTimeOut = 300000;
		ingressCapacity = 0;
	}
};

typedef ZQ::common::Pointer<UdpStreamInitRequestParam>  UdpStreamInitRequestParamPtr;

struct UdpStreamInitResponseParam : public RequestResponseParam 
{
	int32   			timeout;
	std::string 		confirmedUdpIp;
	TransferRange       availRange; 
	UdpStreamInitResponseParam(){
		timeout = 2000;
	}
};

typedef ZQ::common::Pointer<UdpStreamInitResponseParam> UdpStreamInitResponseParamPtr;

struct TransferInitRequestParam : public RequestParam
{
	std::string				fileName;				// file name of transfer content
	std::string				clientTransfer;			// client transfer address
	std::string				transferAddress;		// server transfer address

	std::set<std::string>	subFileExts;			// for file set downloading, sepcify the file extention name
													// valid only if requestType == SESSION_TYPE_COMPLEX

	int64					ingressCapacity;		// client ingress capacity
	int64					extraIngressCapcity;
	TransferRange			requestRange;
	int32					transferDelay;
	int64					allocatedTransferRate;	// specifies the portion of the client's Ingress Capacity that is allocated for this transfer server to service this session
	int32					transferTimeout;		// specifies the duration, in milliseconds, that this transfer initiation is valid
	int32					requestType;			
	//For NAT
	std::string				upStreamPeerIp;
	uint16					upStreamPeerPort;
	std::string				upStreamLocalIp;
	bool					isMainFile;
	std::string				indexFilePathname;

	bool					exposeIndex;

	TransferInitRequestParam( C2StreamerEnv& env)
	:RequestParam(env) {
		ingressCapacity				= 0;
		extraIngressCapcity			= 0;
		transferDelay				= 0;
		allocatedTransferRate		= 0;
		transferTimeout				= 500;
		requestType					= SESSION_TYPE_NORMAL;
		isMainFile					= false;
		exposeIndex 				= false;
	}

	TransferInitRequestParam( C2StreamerEnv& env, const std::string& urlPrefix )
	:RequestParam(env, urlPrefix) {
		ingressCapacity				= 0;
		extraIngressCapcity			= 0;
		transferDelay				= 0;
		allocatedTransferRate		= 0;
		transferTimeout				= 500;
		requestType					= SESSION_TYPE_NORMAL;
		isMainFile					= false;
		exposeIndex 				= false;
	}
};

typedef ZQ::common::Pointer<TransferInitRequestParam> TransferInitRequestParamPtr;

struct TransferInitResponseParam : public RequestResponseParam
{
	std::string				transferId;				// Contains the URI that the client will use to request the asset from the transfer server.
	TransferRange			availRange;				//
	bool					openForWrite;			// 
	int64					timeout;
	std::string				confirmedTransferPort;
	std::string				confirmedTransferIp;

	//for asset attribute
	std::string				baseinfo;
	std::string				memberinfo;

	TransferInitResponseParam()
	{
		openForWrite		= false;
		timeout				= 0;
	}
};
typedef ZQ::common::Pointer<TransferInitResponseParam> TransferInitResponseParamPtr;


struct TransferTermRequestParam : public RequestParam
{
	std::string				clientTransfer;	
	TransferTermRequestParam( C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix) {
	}
};
typedef ZQ::common::Pointer<TransferTermRequestParam> TransferTermRequestParamPtr;

struct TransferTermResponseParam : public RequestResponseParam
{
};
typedef ZQ::common::Pointer<TransferTermResponseParam> TransferTermResponseParamPtr;

struct  IngressCapacityUpdateRequestParam : public RequestParam
{
	std::string				clientTransfer;
	int64					ingressCapacity;
	int64					extraIngressCapacity;
	IngressCapacityUpdateRequestParam(C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix) {
		ingressCapacity			= 0;
		extraIngressCapacity	= 0;
	}
};

typedef ZQ::common::Pointer<IngressCapacityUpdateRequestParam> Ptr;


struct IngressCapacityUpdateResponseParam : public RequestResponseParam
{
	
};

typedef ZQ::common::Pointer<IngressCapacityUpdateResponseParam> IngressCapacityUpdateResponseParamPtr;

struct SessionStatusRequestParam : public RequestParam
{
	std::vector<std::string>	clientTransfers;	// Contains the ClientTransfer address of a client whose sessions should be reported on
	bool						includeAggregate;	//
	
	SessionStatusRequestParam( C2StreamerEnv& env, const std::string& urlPrefix ) 
	:RequestParam( env, urlPrefix) {
		includeAggregate		= true;
	}
};
typedef ZQ::common::Pointer<SessionStatusRequestParam> SessionStatusRequestParamPtr;

struct AggregateStatisticsParam 
{
	int32						activeSessions;
	int32						idleSessions;
	int32						totalSessions;
	int64						allocatedBandwidth;
	int64						totalBandwidth;
	uint64						bytesTransfered;
	int64						uptime;
	AggregateStatisticsParam()
	{
		activeSessions		= 0;
		idleSessions		= 0;
		totalSessions		= 0;
		allocatedBandwidth	= 0;
		totalBandwidth		= 0;
		bytesTransfered		= 0;
		uptime				= 0;
	}
};

struct SessionStatusInfo 
{
	std::string				transferId;			//
	std::string				fileName;
	std::string				clientTransfer;		// client transfer address
	std::string				transferAddress;	// server transfer address
	std::string				transferPortName;	// 
	SessionState			sessionState;
	int64					timeInState;		// Contains the amount of time (in microseconds) that the session has been in the state specified in the ��State�� element
	int64					transferRate;		// Contains the requested transfer rate of this session in bits per second
	int64					bytesTransfered;	// Contains the bytes transferred to the client on this session.  This will be 0 while idle.  When active, it will contain the number of bytes transferred to the client since the session transitioned to the active state.

};

struct SessionStatusResponseParam : public RequestResponseParam
{
	AggregateStatisticsParam	statistics;
	std::vector<SessionStatusInfo> sessionInfos;	
};

typedef ZQ::common::Pointer<SessionStatusResponseParam> SessionStatusResponseParamPtr;

struct ResourceStatusRequestParam : public RequestParam
{
	std::vector<std::string>					portNames; // Specifies the name of a transfer port to report on.  If no PortName elements are specified, then the status of all ports will be reported.  Multiple PortName elements can be specified as well
	ResourceStatusRequestParam( C2StreamerEnv& env, const std::string& urlPrefix) 
	:RequestParam( env, urlPrefix) {
	}

};
typedef ZQ::common::Pointer<ResourceStatusRequestParam> ResourceStatusRequestParamPtr;

enum StreamerPortState
{
	PORT_STATE_DOWN		= 0,
	PORT_STATE_UP		= 1
};

struct ResourceStatusInfo 
{
	std::string					portName;
	std::vector<std::string>	portAddressIpv4;	// Note that this may be in IPv4 or IPv6 format.  A port may have multiple addresses, but the same address will not be on multiple ports
	std::vector<std::string>	portAddressIpv6;
	int32						tcpPortNumber;	// Contains the TCP port number that the transfer server should be contacted on for transfer requests to this port.
	int64						capacity;		// Link capacity of this port, in bits per second.
	StreamerPortState			portState;
	int32						activeTransferCount; // Contains an integer value of the number of currently active transfers in progress on this port
	int64						activeBandwidth;	// Contains an integer value in bits per second of the sum of the  bandwidth of all active transfer sessions on this port

};
struct ResourceStatusResponseParam : public RequestResponseParam
{
	std::vector<ResourceStatusInfo> portInfos;
};
typedef ZQ::common::Pointer<ResourceStatusResponseParam> ResourceStatusResponseParamPtr;

//////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

struct SessionTransferParam : public RequestParam
{
	TransferRange				range;
	int64						ingressCapacity;	// 0 for not specified
	std::string					clientIp;
	std::string					requestFileExt;		// file extention in request
	int							clientPort;
	int32						transferDelay;
	bool						bHasTransferDelay;
	bool						seekIFrame;
	bool						viaProxy; // cache-control issue
	SessionTransferParam(C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix) {
		ingressCapacity = 0;
		clientPort		= 0;
		transferDelay	= 0;
		bHasTransferDelay = false;
		seekIFrame		= false;
		viaProxy 		= false;
	}
};
typedef ZQ::common::Pointer<SessionTransferParam> SessionTransferParamPtr;

struct SessionTransferResponseParam : public RequestResponseParam
{
	std::string 	filename; //file name for response message
	TransferRange	range;
	int64			fileSize;
	bool			openForWrite;
	int32			sessionType;
	bool			cacheable;
	SessionTransferResponseParam()
	{
		openForWrite = false; 
		sessionType = SESSION_TYPE_NORMAL;
		cacheable = false;
	}	
};
typedef ZQ::common::Pointer<SessionTransferResponseParam> SessionTransferResponseParamPtr;

//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

struct SessionUdpRequestParam : public RequestParam
{
	TransferRange				range;
	//int64						ingressCapacity;	// 0 for not specified
	std::string					udpClientAddress;
	unsigned short    			udpClientPort;

	std::string					requestFileExt;		// file extention in request
	//int							clientPort;
	//int32						transferDelay;
	//bool						bHasTransferDelay;
	//bool						seekIFrame;
	//bool						viaProxy; // cache-control issue
	SessionUdpRequestParam(C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix) {
		//ingressCapacity = 0;
		udpClientPort			= 0;
		//transferDelay	= 0;
		//bHasTransferDelay = false;
		//seekIFrame		= false;
		//viaProxy 		= false;
	}
};
typedef ZQ::common::Pointer<SessionUdpRequestParam> SessionUdpRequestParamPtr;

struct SessionUdpResponseParam : public RequestResponseParam
{
	std::string 	filename; //file name for response message
	TransferRange	range;
	int64			fileSize;
	bool			openForWrite;
	//int32			sessionType;
	//bool			cacheable;
	SessionUdpResponseParam()
	{
		openForWrite = false; 
		//sessionType = SESSION_TYPE_NORMAL;
		//cacheable = false;
	}	
};
typedef ZQ::common::Pointer<SessionUdpResponseParam> SessionUdpResponseParamPtr;

enum	 UdpSessionState {
	UDPSTATE_NONE,
	UDPSTATE_INIT,
	UDPSTATE_PLAY,
	UDPSTATE_PAUSE,
	UDPSTATE_DONE,
};

const char* UdpSessionStateToString( UdpSessionState state);

struct SessionUdpControlRequestParam : public RequestParam {
	enum	ControlCode {
		CONTROL_NULL,
		CONTROL_LOAD,
		CONTROL_PLAY,
		CONTROL_PAUSE,
		CONTROL_UNLOAD,
		CONTROL_INFOQUERY
	} 				ctrlCode;

	std::string		assetName; // only valid for CONTROL_LOAD
	int64			subSessionId; // valid except CONTROL_LOAD
	int64			timeOffset; // <0 if play now

	int64 			inTimeOffset;
	int64			outTimeOffset;
	float			scale; // 0.0f if no scale changed
	UdpSessionState	state;

	SessionUdpControlRequestParam( C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix),
	ctrlCode(CONTROL_NULL),
	timeOffset(-1),
	inTimeOffset(0),
	outTimeOffset(0),
	scale(0.0f),
	state(UDPSTATE_NONE){
	}
};

typedef ZQ::common::Pointer<SessionUdpControlRequestParam> SessionUdpControlRequestParamPtr;

struct SessionUdpControlResponseParam : public RequestResponseParam {
	std::string		filename; // filename is coming from the result of parsing request
	int64			subSessionId;
	int64			timeOffset;
	int64			inTimeOffset;
	int64			outTimeOffset;
	int64			dataOffset; // NOT valid if ctrlCode == CONTROL_INFOQUERY
	int64			outDataOffset;
	int64			duration; // only valid if request's ctrlCode is CONTROL_INFOQUERY
	float 			scale;
	int64 			bitrate;

	SessionUdpControlResponseParam()
	:subSessionId(0),
	timeOffset(0),
	inTimeOffset(0),
	outTimeOffset(0),
	dataOffset(0),
	outDataOffset(0),
	duration(0),
	scale(0.0f),
	bitrate(0){
	}
};
typedef ZQ::common::Pointer<SessionUdpControlResponseParam> SessionUdpControlResponseParamPtr;





//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct TransferIngressCapacityUpdateParam : public RequestParam
{
	std::string					clientTransfer;	//Contains the IP address of the client
	int64						ingressCapacity;
	TransferIngressCapacityUpdateParam( C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix)
	{
		ingressCapacity	= 0;
	}
};

typedef ZQ::common::Pointer<TransferIngressCapacityUpdateParam> TransferIngressCapacityUpdateParamPtr;

struct TransferIngressCapacityUpdateResponseParam : public RequestResponseParam 
{
	int64						allocatedIngressCapacity;
	int64						extraIngressCapacity;
	TransferIngressCapacityUpdateResponseParam()
	{
		allocatedIngressCapacity	= 0;
		extraIngressCapacity		= 0;
	}
};
typedef ZQ::common::Pointer<TransferIngressCapacityUpdateResponseParam> TransferIngressCapacityResponseParamPtr;

struct TransferSessionStateUpdateParam : public RequestParam
{
	std::string					clientTransfer;
	std::string					transferId;
	SessionState				sessionState;

	TransferSessionStateUpdateParam( C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix)
	{
		sessionState = SESSION_STATE_NULL;
	}
};
typedef ZQ::common::Pointer<TransferSessionStateUpdateParam> TransferSessionStateUpdateParamPtr;

struct  TransferSessionStateUpdateResponseParam : public RequestResponseParam
{

};

typedef ZQ::common::Pointer<TransferSessionStateUpdateResponseParam> TransferSessionStateUpdateResponseParamPtr;

struct ResourceStateUpdateParam : public RequestParam
{
	std::string					portName;
	std::vector<std::string>	portAddressV4;
	std::vector<std::string>	portAddressV6;
	int32						tcpPortNumber;
	int64						capacity;	// Link capacity of this port, in bits per second
	StreamerPortState			portState;	//
	int32						activeTransferCount;
	int64						activeBandiwidth;
	ResourceStateUpdateParam(C2StreamerEnv& env, const std::string& urlPrefix)
	:RequestParam(env, urlPrefix)
	{
		tcpPortNumber		= 0;
		capacity			= 0;
		activeTransferCount = 0;
		activeBandiwidth	= 0;
		portState			= PORT_STATE_DOWN;
	}
};
typedef ZQ::common::Pointer<ResourceStateUpdateParam> ResourceStateUpdateParamPtr;

struct ResourceStateUpdateResponseParam : public RequestResponseParam
{
};
typedef ZQ::common::Pointer<ResourceStateUpdateResponseParam> ResourceStateUpdateResponseParamPtr;

struct HLSRequestParam : public RequestParam 
{
	std::string			url;
	std::string			contentName;
	std::string			subLevelName;
};

typedef ZQ::common::Pointer<HLSRequestParam> HLSRequestParamPtr;

struct HLSResponseParam : public RequestResponseParam 
{
	std::string			content;
};
typedef ZQ::common::Pointer<HLSResponseParam> HLSResponseParamPtr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// C interface

int32		cUdpInit( const UdpStreamInitRequestParamPtr request, UdpStreamInitResponseParamPtr response);
int32		cUdpRun( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response );

int32		cTransferInit( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response );
int32		cTransferRun( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response )	;
int32		cTransferTerm( const TransferTermRequestParamPtr request , TransferTermResponseParamPtr response );
int32		cSessionStatus( const SessionStatusRequestParamPtr request , SessionStatusResponseParamPtr response );
int32		cResourceStatus( const ResourceStatusRequestParamPtr request , ResourceStatusResponseParamPtr response );
int32		cUdpInfoQuery( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response  );

struct C2Event : virtual public ZQ::common::SharedObject
{
	C2Method			eventmethod;	
	C2Event()
	:eventmethod(METHOD_NULL)
	{
	}
};

typedef ZQ::common::Pointer<C2Event> C2EventPtr;

struct PortStateUpdateEvent : public C2Event, public ResourceStateUpdateParam
{
	PortStateUpdateEvent( C2StreamerEnv& env )
	:ResourceStateUpdateParam(env,"")

	{
		eventmethod = METHOD_RES_UPDATE;
	}
};

typedef ZQ::common::Pointer<PortStateUpdateEvent> PortStateUpdateEventPtr;

struct IngressCapacityUpdateEvent : public C2Event
{
	std::string				clientTransfer;
	int64					ingressCapacity;
	IngressCapacityUpdateEvent()
	{
		eventmethod = METHOD_IC_UPDATE;
		ingressCapacity = 0;
	}
};
typedef ZQ::common::Pointer<IngressCapacityUpdateEvent> IngressCapacityUpdatePtr;

struct TransferStateUpdateEvent : public C2Event
{
	std::string				clientTransfer;
	std::string				transferId;
	SessionState			sessionState;
	TransferStateUpdateEvent()
	{
		eventmethod = METHOD_SESS_UPDATE;
		sessionState = SESSION_STATE_NULL;
	}
};
typedef ZQ::common::Pointer<TransferStateUpdateEvent> TransferStateUpdateEventPtr;

struct C2UdpSessionEvent : public C2Event {
	enum Event {
		UDPEVENT_STATE_CHANGE,
		UDPEVENT_SCALE_CHANGE,
		UDPEVENT_SESSION_DONE
	} 					event;
	int64				subSessId;
	union {
		UdpSessionState	state;
		float			scale;
	}oldStatus,newStatus;

	C2UdpSessionEvent()
		:subSessId(0) {
		eventmethod = METHOD_UDP_EVENT;
	}
};

typedef ZQ::common::Pointer<C2UdpSessionEvent> C2UdpSessionEventPtr;

class C2EventSinker : public ZQ::DataPostHouse::SharedObject
{
public:
	virtual ~C2EventSinker(){};
	virtual int32	publish(const C2EventPtr request) = 0;
};
typedef ZQ::common::Pointer<C2EventSinker> C2EventSinkerPtr;


C2EventSinkerPtr updateEventReceiver( C2EventSinkerPtr& newEventReceiver , C2Method mask );

void updateSessionTimer( const std::string& sessId , int64 interval );

struct C2ServiceLoad
{
	int64	localStreamBWMax;
	int64	localStreamBW;
	int64	natStreamBWMax;
	int64	natStreamBW;
};

bool getC2Load( const std::string& upStreamIp, C2ServiceLoad& load );

struct CacheBufferStatusInfo {
	int64 	bufId;
	int64 	reqId;
	CacheBufferStatusInfo():
	bufId(0),
	reqId(0){
	}
};


}//namespace C2Streamer

#endif//_tianshan_cdnss_c2streamer_lib_header_file_h__
