
#ifndef _tianshan_cdnss_c2streamer_lib_header_file_h__
#define _tianshan_cdnss_c2streamer_lib_header_file_h__

#include <ZQ_common_conf.h>
#include <string>
#include <vector>
#include <map>

#include <DataPostHouse/common_define.h>
#include <HttpEngine/HttpEngine.h>

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
const int32 errorCodeSessionGone			= 410;
const int32 errorCodeBadRange				= 416;

const int32 errorCodeInternalError			= 500;
const int32 errorCodeNotImplemented			= 501;
const int32 errorCodeServerUnavail			= 503;
const int32 errorCodeGatewayTimeout			= 504;
const int32 errorCodeVersionNotSupported	= 505;

#ifndef ZQ_CDN_UMG
class C2HttpResponseHanlder : public ZQ::DataPostHouse::SharedObject
{
public:
	C2HttpResponseHanlder( ZQHttp::IResponse* handler , ZQHttp::IConnection* conn);
	virtual ~C2HttpResponseHanlder();

public:

	void	updateStartline( int statusCode , const std::string& desc );

	void	updateHeader( const std::string& key , const std::string& value );

	bool	addBodyContent( const char* data , size_t size );

	bool	flushHeader( ssize_t bodySize = -1 );
	
	bool	updateContentLength( size_t length);

	bool	complete();
	
	std::string lastError() const;

	bool	isConnectionBroken() const;
	
	void	setConnectionBroken( bool bBroken);
	
	int64	getConnectionId() const;
	
	void	setConnectionId( int64 id );
	
	void	setCommOption(int opt , int value );
private:
	ZQHttp::IConnection*	mResponseConnection;
	ZQHttp::IResponse*		mResponseHandler;
	bool					mbComplete;
	bool					mbFlushed;
	bool					mbConnBroken;
	int64					mConnId;
	ZQ::common::Mutex		mLocker;
};

typedef ZQ::DataPostHouse::ObjectHandle<C2HttpResponseHanlder> C2HttpResponseHanlderPtr;
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

	METHOD_HLS_GET				= 1 << 9
};

const char* convertC2MethodToString( const C2Method& method );

struct RequestParam : virtual public ZQ::DataPostHouse::SharedObject
{
	std::string			requestHint;

	C2Method			method;
	std::string			sessionId;

	bool				connBroken;	// if request connection is broken, no more action should be performed on this request

	RequestParam()
	{
		method		= METHOD_NULL;
		connBroken	= false;
	}
	virtual ~RequestParam()
	{
	}
};
typedef ZQ::DataPostHouse::ObjectHandle<RequestParam> RequestParamPtr;

struct RequestResponseParam : public ZQ::DataPostHouse::SharedObject
{
	int32						errorCode;
	std::string					errorText;
#ifndef ZQ_CDN_UMG
	C2HttpResponseHanlderPtr	responseHandler;
#endif
	bool					isSuccess() const
	{
		return errorCode >= 200 && errorCode <= 299;
	}

	void					setLastErr( int32 errCode , const char* fmt , ... );
	void					setLastErr( const RequestParamPtr& request , int32 errCode , const char* fmt , ... );

	RequestResponseParam()
	{
		errorCode = 200; 
	}
	virtual ~RequestResponseParam(){}
};

typedef ZQ::DataPostHouse::ObjectHandle<RequestResponseParam> RequestResponseParamPtr;


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
	int32					transferRate;
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


	TransferInitRequestParam()
	{
		transferRate				= 0;
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

typedef ZQ::DataPostHouse::ObjectHandle<TransferInitRequestParam> TransferInitRequestParamPtr;

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
typedef ZQ::DataPostHouse::ObjectHandle<TransferInitResponseParam> TransferInitResponseParamPtr;


struct TransferTermRequestParam : public RequestParam
{
	std::string				clientTransfer;	
};
typedef ZQ::DataPostHouse::ObjectHandle<TransferTermRequestParam> TransferTermRequestParamPtr;

struct TransferTermResponseParam : public RequestResponseParam
{

};
typedef ZQ::DataPostHouse::ObjectHandle<TransferTermResponseParam> TransferTermResponseParamPtr;

struct  IngressCapacityUpdateRequestParam : public RequestParam
{
	std::string				clientTransfer;
	int64					ingressCapacity;
	int64					extraIngressCapacity;
	IngressCapacityUpdateRequestParam()
	{
		ingressCapacity			= 0;
		extraIngressCapacity	= 0;
	}
};

typedef ZQ::DataPostHouse::ObjectHandle<IngressCapacityUpdateRequestParam> Ptr;


struct IngressCapacityUpdateResponseParam : public RequestResponseParam
{
	
};

typedef ZQ::DataPostHouse::ObjectHandle<IngressCapacityUpdateResponseParam> IngressCapacityUpdateResponseParamPtr;

struct SessionStatusRequestParam : public RequestParam
{
	std::vector<std::string>	clientTransfers;	// Contains the ClientTransfer address of a client whose sessions should be reported on
	bool						includeAggregate;	//
	
	SessionStatusRequestParam()
	{
		includeAggregate		= true;
	}
};
typedef ZQ::DataPostHouse::ObjectHandle<SessionStatusRequestParam> SessionStatusRequestParamPtr;

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

typedef ZQ::DataPostHouse::ObjectHandle<SessionStatusResponseParam> SessionStatusResponseParamPtr;

struct ResourceStatusRequestParam : public RequestParam
{
	std::vector<std::string>					portNames; // Specifies the name of a transfer port to report on.  If no PortName elements are specified, then the status of all ports will be reported.  Multiple PortName elements can be specified as well

};
typedef ZQ::DataPostHouse::ObjectHandle<ResourceStatusRequestParam> ResourceStatusRequestParamPtr;

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
typedef ZQ::DataPostHouse::ObjectHandle<ResourceStatusResponseParam> ResourceStatusResponseParamPtr;

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
	SessionTransferParam()
	{
		ingressCapacity = 0;
		clientPort		= 0;
		transferDelay	= 0;
		bHasTransferDelay = false;
		seekIFrame		= false;
		viaProxy 		= false;
	}
};
typedef ZQ::DataPostHouse::ObjectHandle<SessionTransferParam> SessionTransferParamPtr;

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
typedef ZQ::DataPostHouse::ObjectHandle<SessionTransferResponseParam> SessionTransferResponseParamPtr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct TransferIngressCapacityUpdateParam : public RequestParam
{
	std::string					clientTransfer;	//Contains the IP address of the client
	int64						ingressCapacity;
	TransferIngressCapacityUpdateParam()
	{
		ingressCapacity	= 0;
	}
};

typedef ZQ::DataPostHouse::ObjectHandle<TransferIngressCapacityUpdateParam> TransferIngressCapacityUpdateParamPtr;

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
typedef ZQ::DataPostHouse::ObjectHandle<TransferIngressCapacityUpdateResponseParam> TransferIngressCapacityResponseParamPtr;

struct TransferSessionStateUpdateParam : public RequestParam
{
	std::string					clientTransfer;
	std::string					transferId;
	SessionState				sessionState;

	TransferSessionStateUpdateParam()
	{
		sessionState = SESSION_STATE_NULL;
	}
};
typedef ZQ::DataPostHouse::ObjectHandle<TransferSessionStateUpdateParam> TransferSessionStateUpdateParamPtr;

struct  TransferSessionStateUpdateResponseParam : public RequestResponseParam
{

};

typedef ZQ::DataPostHouse::ObjectHandle<TransferSessionStateUpdateResponseParam> TransferSessionStateUpdateResponseParamPtr;

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
	ResourceStateUpdateParam()
	{
		tcpPortNumber		= 0;
		capacity			= 0;
		activeTransferCount = 0;
		activeBandiwidth	= 0;
		portState			= PORT_STATE_DOWN;
	}
};
typedef ZQ::DataPostHouse::ObjectHandle<ResourceStateUpdateParam> ResourceStateUpdateParamPtr;

struct ResourceStateUpdateResponseParam : public RequestResponseParam
{
};
typedef ZQ::DataPostHouse::ObjectHandle<ResourceStateUpdateResponseParam> ResourceStateUpdateResponseParamPtr;

struct HLSRequestParam : public RequestParam 
{
	std::string			url;
	std::string			contentName;
	std::string			subLevelName;
};

typedef ZQ::DataPostHouse::ObjectHandle<HLSRequestParam> HLSRequestParamPtr;

struct HLSResponseParam : public RequestResponseParam 
{
	std::string			content;
};
typedef ZQ::DataPostHouse::ObjectHandle<HLSResponseParam> HLSResponseParamPtr;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// C interface
int32		cTransferInit( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response );
int32		cTransferRun( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response )	;
int32		cTransferTerm( const TransferTermRequestParamPtr request , TransferTermResponseParamPtr response );
int32		cSessionStatus( const SessionStatusRequestParamPtr request , SessionStatusResponseParamPtr response );
int32		cResourceStatus( const ResourceStatusRequestParamPtr request , ResourceStatusResponseParamPtr response );

struct C2Event : virtual public ZQ::DataPostHouse::SharedObject
{
	C2Method			eventmethod;	
	C2Event()
	:eventmethod(METHOD_NULL)
	{
	}
};

typedef ZQ::DataPostHouse::ObjectHandle<C2Event> C2EventPtr;

struct PortStateUpdateEvent : public C2Event, public ResourceStateUpdateParam
{
	PortStateUpdateEvent()
	{
		eventmethod = METHOD_RES_UPDATE;
	}
};

typedef ZQ::DataPostHouse::ObjectHandle<PortStateUpdateEvent> PortStateUpdateEventPtr;

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
typedef ZQ::DataPostHouse::ObjectHandle<IngressCapacityUpdateEvent> IngressCapacityUpdatePtr;

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
typedef ZQ::DataPostHouse::ObjectHandle<TransferStateUpdateEvent> TransferStateUpdateEventPtr;

class C2EventSinker : public ZQ::DataPostHouse::SharedObject
{
public:
	virtual ~C2EventSinker(){};
	virtual int32	publish(const C2EventPtr request) = 0;
};
typedef ZQ::DataPostHouse::ObjectHandle<C2EventSinker> C2EventSinkerPtr;


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

}//namespace C2Streamer
