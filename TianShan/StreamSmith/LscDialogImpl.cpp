// LscDialogImpl.cpp: implementation of the LscDialogImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "global.h"
#include "StreamSmithSite.h"
#include "LscDialogImpl.h"
#include <Log.h>
#include <RtspRelevant.h>
#include "SystemUtils.h"
#include <TimeUtil.h>

using namespace ZQ::common;

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE
#ifdef ZQ_OS_LINUX
	#ifndef stricmp
	#define stricmp strcasecmp
	#endif
#endif

//utility function
lsc::ResponseCode convertStringToStatusCode(const char* value)
{
	lsc::ResponseCode retCode = lsc::LSC_OK; 
	if ( stricmp( ResponseOK , value ) == 0 ) 
	{
		retCode = lsc::LSC_OK;
	}
	else if ( stricmp( ResponseBadRequest , value )  == 0 )
	{
		retCode = lsc::LSC_BAD_REQUEST;
	}
	else if ( stricmp( ResponseUnauthorized , value) == 0 ) 
	{
		retCode = lsc::LSC_NO_PERMISSION;
	}
	else if( stricmp( ResponseNotEnoughBandwidth , value ) == 0 )
	{
		retCode = lsc::LSC_NO_RESOURCES;
	}
	else if ( stricmp( ResponseNotImplement, value) == 0 ) 
	{
		retCode = lsc::LSC_NO_IMPLEMENT;
	}
	else if( stricmp( ResponseSessionNotFound , value) == 0 )
	{
		retCode = lsc::LSC_BAD_STREAM;
	}
	else if ( stricmp(ResponseInvalidRange,value)  == 0 )
	{
#pragma message(__MSGLOC__"how about LSC_BAD_STOP")
		retCode = lsc::LSC_BAD_START;
	}
	else if( stricmp ( ResponseInternalError, value ) == 0 )
	{
		retCode = lsc::LSC_SERVER_ERROR;
	}
	else
	{
		retCode = lsc::LSC_UNKNOWN;
	}

	return retCode;
}
lsc::ServerMode ConvertStringToServerMode(const char* value)
{
	#pragma message(__MSGLOC__"NOT Implement yet")
	return lsc::LSC_MODE_EOS;
}

/*------------------------------------------------------------------------
LscConnection
------------------------------------------------------------------------*/

LscConnection::LscConnection(IDataCommunicatorPtr conn , int connType )
:_conn(conn)
{
	_iConnType = connType;
	_ref = 0;
}
bool LscConnection::close()
{
	if ( _iConnType == COMM_TYPE_TCP ) 
	{
		_conn->close();
	}
	//DO NOT Close socket in UDP state because the connection is the server
	//socket connection,and it's not the real connection
	return true;
}

void	LscConnection::setPeerInfo  ( const std::string& strPeerIP , const std::string& strPeerPort )
{
	_strPeerIP = strPeerIP;
	_iPeerPort = atoi(strPeerPort.c_str());
}

void	LscConnection::setLocalInfo ( const std::string& strLocalIP, const std::string& strLocalPort )
{
	_strLocalIP = strLocalIP;
	_iLocalPort = atoi(strLocalPort.c_str());
}
void	LscConnection::setPeerInfo (const CommAddr& peer )
{
	memcpy(&_peerInfo,&peer,sizeof(_peerInfo));
}
uint32 LscConnection::recv (uint8* buf, const uint32 maxlen)
{
	return (uint32 )_conn->read(reinterpret_cast<int8*>(buf), (int )maxlen);

}
uint32 LscConnection::send (const uint8* buf, const uint32 maxlen)
{
	return (uint32 )_conn->write(reinterpret_cast<const int8*>(buf), (int )maxlen, 10000);
}


/*------------------------------------------------------------------------
LSCP Dialog
------------------------------------------------------------------------*/

LscDialog::LscDialog(  RtspSessionMgr& sessionMgr  )
:_sessionMgr(sessionMgr)
{
	_conn = NULL;
	_strSavedmsg = "";
	_reservedMsg = NULL;
	
}
LscDialog::~LscDialog ()
{
}

void LscDialog::onCommunicatorSetup(IDataCommunicatorPtr communicator)
{
	CommAddr peer;
	CommAddr local;

	communicator->getCommunicatorAddrInfo(local, peer);
	char	szPeerInfo[256]={0};
	char	szLocalInfo[256]={0};
	
#ifdef ZQ_OS_MSWIN	
	DWORD	dwPeerInfoSize = sizeof(szPeerInfo);
	DWORD	dwLocalInfoSize = sizeof(szLocalInfo);
	WSAAddressToStringA((sockaddr*)&peer.u.addr, peer.addrLen, NULL, szPeerInfo, &dwPeerInfoSize);
	WSAAddressToStringA((sockaddr*)&local.u.addr, local.addrLen, NULL, szLocalInfo, &dwLocalInfoSize);
#else
	size_t peerbufs = sizeof(szPeerInfo);
    int peerPort = 0;
    size_t localbufs = sizeof(szLocalInfo);
    int localPort = 0;
    bool bre = false;
    char chport[10] = {0};
    bre = sockaddrToString((struct ::sockaddr*)&peer.u.addr, szPeerInfo, peerbufs,peerPort);
    if(!bre)
        glog(Log::L_ERROR,"RtspConnection() sockaddrToString convent peer xockaddr failed");
    sprintf(chport,":%d",peerPort);
    strcat(szPeerInfo,chport);

    bre = sockaddrToString((struct ::sockaddr*)&local.u.addr, szLocalInfo, localbufs,localPort);
    if(!bre)
        glog(Log::L_ERROR,"RtspConnection() sockaddrToString convent local xockaddr failed");
    memset(chport,0,sizeof(chport));
    sprintf(chport,":%d",localPort);
    strcat(szPeerInfo,chport);
#endif

	char* pColon = NULL;
	pColon = strrchr(szPeerInfo,':');
	if (pColon) 
	{
		_strPeerIP.assign( szPeerInfo , pColon - szPeerInfo);
		_strPeerPort = pColon+1;
	}
	
	pColon = strrchr(szLocalInfo,':');
	if ((pColon)) 
	{
		_strServerIP.assign(szLocalInfo,pColon-szLocalInfo);
		_strServerPort = pColon+1;
	}
	
	glog(Log::L_DEBUG, "LscDialog::onCommunicatorSetup()  SOCKET connect from client [%s][%s] to server [%s][%s]",
		_strPeerIP.c_str(), _strPeerPort.c_str(),
		_strServerIP.c_str(),_strServerPort.c_str() );
	
	_conn = LscConnectionPtr( new LscConnection(communicator, COMM_TYPE_TCP));

	
	//notify session manager there is a new connection
	_sessionMgr.onconnectionSetup(_conn);
	_conn->setPeerInfo(_strPeerIP,_strPeerPort);
	_conn->setLocalInfo(_strServerIP,_strServerPort);
	_conn->setConnectionProtocol ( IConnectionInternal::CONNECTION_PROTCOL_TYPE_LSCP);
	assert(_conn);
	
}

bool LscDialog::onRead(const int8 *buffer, size_t bufSize)
{
	char hint[0x200];
	
	sprintf(hint, "%s:%s", _strPeerIP.c_str(),_strPeerPort.c_str());
	
	glog.hexDump(Log::L_DEBUG, buffer, bufSize, hint);
/*
	DWORD dw2 = GetTickCount();
	DWORD dw3 = GetTickCount();
*/	
	int64 dw2 = ZQ::common::now();

	IStreamSmithSite* defSite = ZQ::StreamSmith::StreamSmithSite::getDefaultSite();
	assert(defSite);	
	
	//set current connection identity to clientRequest
	char szConnIdent[256];
	sprintf(szConnIdent,FMT64U,_conn->getConnectionIdentity());

	void* pBuf = (void*) buffer;
	lsc::lscMessage* newMsg = NULL;
	int nSize =  static_cast<int>(bufSize); // TODO : ?????? antoher solution
	while ( nSize > 0 ) 
	{
		try
		{
			newMsg = lsc::ParseMessage( pBuf ,nSize, _reservedMsg );
		}
		catch (const lsc::errorMessageException& err) 
		{
			glog(ZQ::common::Log::L_ERROR , CLOGFMT(LscDialog,"catch a exception:%s"),err.what ());
			//Should I post a bad request to client
			continue ;
		}
		if (!newMsg->Parse ()) 
		{
			glog(ZQ::common::Log::L_ERROR , CLOGFMT(LscDialog,"Can't parse the lscMessage"));
#pragma message(__MSGLOC__"Should I post a bad request to client ?")
			continue;
		}
		LscClientRequest* pRequest =  new LscClientRequest(newMsg , _conn , dw2);
		assert(pRequest!=NULL);
		if(!pRequest->ParseLscIntoRtspHeader () )
		{
			glog(ZQ::common::Log::L_ERROR , CLOGFMT(LscDialog,"Parse message failed,should I post a bad request to client ?"));
			//should I post a bad request to client
			delete pRequest;
			pRequest = NULL;
			continue;
		}

		{
			ClientRequestGuard gd(pRequest);			
			defSite->postRequest (pRequest,IClientRequest::FixupRequest);			
		}
	}
	return true;
}
void LscDialog::onError()
{
	//just ignor
}

void LscDialog::onCommunicatorDestroyed(IDataCommunicatorPtr communicator)
{
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL1) 
	{
		glog(Log::L_DEBUG, "RtspDialog::onCommunicatorDestroyed[%lld]",communicator->getCommunicatorId());
	}
	
	RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
	sessionMgr->onConnectDestroyed(_conn);	
	_conn->close();	
}

void LscDialog::onWritten(size_t bufSize)
{

}


/*------------------------------------------------------------------------
LscResponse
------------------------------------------------------------------------*/
LscResponse::LscResponse(LscClientRequest& request , int64 dwReceivedTime)
:_request(request)
{
	_dwRecvTime = dwReceivedTime;	
	memset( &_msg, 0, sizeof(_msg) );
	//memcpy( &_msg.jump.header , &_request._lscMessage->GetLscMessageContent ().jump.header ,sizeof(_msg.jump.header) );
}

LscResponse::~LscResponse ()
{	
}
void LscResponse::setLscMsg(const lsc::LSCMESSAGE& msg)
{
	_msg.jump.header = msg.jump.header;
	_msg.jump.header.opCode |= lsc::LSC_REPLY_FLAG;	
}

void LscResponse::printf_preheader(const char* content)
{
	//glog(ZQ::common::Log::L_ERROR , CLOGFMT(LscResponse,"printf_preheader() DO NOT invoke this method in LSC protocol"));

}

void LscResponse::printf_postheader(const char* content)
{
	//glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscResponse,"printf_postheader() DO NOT invoke this method in LSC protocol"));
	lsc::StandardHeader_t& stdHeader = _msg.jump.header;
	switch( _msg.jump.header.opCode) 
	{
	case lsc::LSC_DONE:
	case lsc::LSC_PAUSE_REPLY:
	case lsc::LSC_RESUME_REPLY:
	case lsc::LSC_STATUS_REPLY:
	case lsc::LSC_RESET_REPLY:
	case lsc::LSC_JUMP_REPLY:
	case lsc::LSC_PLAY_REPLY:
		{
			stdHeader.statusCode = convertStringToStatusCode (content);
		}
		break;
	default:
		{
		}
	}
	
}

void LscResponse::setHeader( const char* key , const char* value )
{
#pragma message(__MSGLOC__"LscResponse::setHeader NOT IMPLEMENT")

	if ( !( key && strlen(key) > 0 )) 
	{
		assert(false);
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscResponse,"setHeader() Invalid Key"));
		return ;
	}
	if ( stricmp( LSC_HEADER_CSEQ , key ) == 0 ) 
	{//CSeq
		lsc::StandardHeader_t& stdHeader = _msg.jump.header;
		stdHeader.transactionId = (uint8)atoi(value);		
	}
	else if ( stricmp( LSC_HEADER_SESSION , key) == 0 ) 
	{//Session
		lsc::StandardHeader_t& stdHeader = _msg.jump.header;
		stdHeader.streamHandle = atoi(value);
	}	
	else if ( stricmp( LSC_HEADER_SCALE,key) == 0 )
	{//Scale
		float scale = (float)atof(value);
		short numerator = (short)(scale * 1000);
		unsigned short denominator = 1000;
		switch( _msg.jump.header.opCode )
		{
		case lsc::LSC_PLAY_REPLY:
		case lsc::LSC_JUMP_REPLY:				 
		case lsc::LSC_RESET_REPLY:			
		case lsc::LSC_STATUS_REPLY:
		case lsc::LSC_RESUME_REPLY:
		case lsc::LSC_PAUSE_REPLY:
		case lsc::LSC_DONE:
			{				
				_msg.response.data.numerator = numerator;
				_msg.response.data.denominator = denominator;
			}
			break;
		default:
			{
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscResponse,"current lsc message[%d] do not have scale"),_msg.jump.header.opCode);
				return ;
			}
			break;
		}
	}
	else if ( stricmp (LSC_HEADER_RANGE , key ) == 0 ) 
	{//range
		switch( _msg.jump.header.opCode )
		{
		case lsc::LSC_DONE:
        case lsc::LSC_PAUSE_REPLY:
        case lsc::LSC_RESUME_REPLY:
        case lsc::LSC_STATUS_REPLY:
        case lsc::LSC_RESET_REPLY:
        case lsc::LSC_JUMP_REPLY:
        case lsc::LSC_PLAY_REPLY:
			{
				//
				float fNpt = 0.0f;
				sscanf(value,"npt=%f-",&fNpt);
				glog(ZQ::common::Log::L_DEBUG,CLOGFMT(LscResponse,"convert range to [%f]"),fNpt);
				_msg.response.data.currentNpt = (unsigned long)(fNpt*1000);
			}			
			break;
		default:
			{				
			}
			break;
		}
	}
//	else if ( stricmp (LSC_HEADER_STATUS , key) == 0 ) 
//	{//
//		
//	}
	
}

const char* LscResponse::get_preheader(IN OUT char* value , IN OUT uint16* valueLen )
{
	glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscResponse,"get_preheader() DO NOT invoke this method in LSC protocol"));
	return NULL;
}

const char* LscResponse::get_postheader(IN OUT char* value , IN OUT uint16* valueLen )
{
	glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscResponse,"get_postheader() DO NOT invoke this method in LSC protocol"));
	return NULL;
}

const char* LscResponse::getHeader(IN const char* key ,IN OUT char *value,IN OUT uint16* valueLen)
{
#pragma message(__MSGLOC__"LscResponse::getHeader NOT IMPLEMENT")
	return NULL;
}

uint32 LscResponse::post()
{
	IConnectionInternalPtr conn = _request.getConnection();
	if (!conn) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscResponse,"post() no connection is availble for posting the data back to client"));
		return 0;
	}
	//_msg.response.header.hton ();
	//_msg.response.data.hton();
	
	_msg.response.hton ();

	uint32 ret = conn->send ((uint8*)&_msg,sizeof(lsc::ResponseMessage_t));
	if (ret != sizeof(lsc::ResponseMessage_t)) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscResponse,"send out data failed"));
	}
	conn = NULL;
	return ret;
}


/*------------------------------------------------------------------------
LscClientRequest
------------------------------------------------------------------------*/


LscClientRequest::LscClientRequest (	lsc::lscMessage* lscMsg,
										LscConnectionPtr conn, 
										int64 dwRecvTime  )
										:_response(*this,dwRecvTime),
										_ref(0),
										_requestInitTime(dwRecvTime)
{
	_lscMessage = lscMsg;
	_lscConn = conn;
	assert(_lscConn!=NULL);

	//get the default streamsmith site
	_site = ZQ::StreamSmith::StreamSmithSite::getDefaultSite();
	 _response.setLscMsg(lscMsg->GetLscMessageContent());
}
LscClientRequest::~LscClientRequest ()
{
	if ( _lscMessage != NULL ) 
	{
		delete _lscMessage;
		_lscMessage = NULL;
	}
}

void LscClientRequest::addRef()
{
	ZQ::common::MutexGuard gd(_refLock);
	++_ref;
//	InterlockedIncrement(&_ref);
}
void LscClientRequest::release ()
{
//	long lRef = InterlockedDecrement(&_ref);
	long lRef = 0;	
	{
		ZQ::common::MutexGuard gd(_refLock);
		lRef = --_ref;
	}
	if( lRef == 0 )
		delete this;
}
bool LscClientRequest::setPhase(ProcessPhase phase)
{
	_phase = phase;
	return true;
}
IClientRequest::ProcessPhase LscClientRequest::getPhase ()
{
	return _phase;
}
void LscClientRequest::setUserCtxIdx(const char* userCtxIdx)
{
	_userContxt = (void*)userCtxIdx;
}

bool LscClientRequest::setSite(IStreamSmithSite* pSite) 
{
	_site = pSite;
	return true;
}

const char* LscClientRequest::write(const unsigned char* reqcontent, const uint32* plen)
{
#pragma message(__MSGLOC__"WRITE THE RAW REQUEST IS FORBIDDEN!")
	glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"Write() You Can NOT invoke this method in LSC Protocol"));
	assert(false);
	return NULL;
}
const char* LscClientRequest::read(const uint32 offset, const unsigned char* content, const uint32* plen)
{
#pragma message(__MSGLOC__"WRITE THE RAW REQUEST IS FORBIDDEN!")
	glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"read() You Can NOT invoke this method in LSC Protocol"));
	assert(false);
	return NULL;
}

void LscClientRequest::setContent(const unsigned char* content, const uint32 len)
{
	assert(false);
	glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"YOU Can NOT invoke this method in LSC protocol"));
}

void LscClientRequest::setHeader( const char* key , char* value )
{
	if ( !_lscMessage )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"setHeader() No lscMessage is valid"));
		return ;
	}
#pragma message(__MSGLOC__"NOT IMPLEMENT")
	if ( ! ( key && strlen(key)>0 ) )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"setHeader() Invalid key passed in"));
		return ;
	}

#pragma message(__MSGLOC__"DO Not allow change the request field")
//	if ( stricmp( LSC_HEADER_CSEQ , key ) == 0 )
//	{//CSeq		
//	}
//	else if ( stricmp( LSC_HEADER_SESSION , key )  == 0 )
//	{//Session
//	}
//	else if ( stricmp ( LSC_HEADER_SCALE , key ) == 0 ) 
//	{//scale
//	}
//	else if ( stricmp ( LSC_HEADER_RANGE , key ) == 0 ) 
//	{//range
//	}
//	else if ( stricmp ( LSC_HEADER_METHOD , key ) == 0 ) 
//	{//Method
//	}

	//at last,I must store the header into _header
	if (value == NULL ) 
	{
		_header.erase (key);
	}
	else
	{
		_header[key] = value;
	}

}
bool  LscClientRequest::setArgument(REQUEST_VerbCode verb, const char* uri, const char* protocol)
{
#pragma message(__MSGLOC__"only verb is availble ??? ")
	if ( !_lscMessage ) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"setArgument() No lscMessage is valid"));
		return false;
	}
#pragma message(__MSGLOC__"DO NOT Allow change verb in lsc request")	
//	lsc :: LSCMESSAGE& msg = (lsc :: LSCMESSAGE&)_lscMessage->GetLscMessageContent ();
//	lsc::StandardHeader_t& stdHeader = msg.jump.header;
//	switch( verb ) 
//	{
//	case RTSP_MTHD_PAUSE:
//		{
//			stdHeader.opCode = lsc::LSC_PAUSE;
//		}
//		break;
//	case RTSP_MTHD_PLAY:
//		{
//		}
//		break;
//	default:
//		break;
//	}
	return false;
}

IConnectionInternalPtr LscClientRequest::getConnection ()
{	
	if (!_lscConn)
		return NULL;
	
	// if connection is invalid, release the object
	if (!_lscConn->isActive()) 
	{
		return NULL;		
	}
	
	// connection still is active
	
	return _lscConn;
}

bool LscClientRequest::getRemoteInfo(RemoteInfo& info)
{
	if (info.size != sizeof(RemoteInfo))
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"getRemoteInfo() Invalid input parameter"));
		return false;
	}
	IConnectionInternalPtr conn = getConnection();
	
	if (!conn)
		return false;

	if (!conn->isActive())
		return false;

	bool result = conn->getRemoteIP( info.ipaddr , info.addrlen , &info.port) != NULL;
	conn = NULL;
	return result;
}

const char* LscClientRequest::getClientSessionId()
{
	KEYVALUEPIR::const_iterator it  = _header.find ("Session");
	if ( it != _header.end () ) 
	{
		return it->second.c_str();
	}
	else
	{
		return NULL;
	}
}
IStreamSmithSite* LscClientRequest::getSite()
{
	return _site;
}

IServerResponse* LscClientRequest::getResponse()
{
	return (IServerResponse*)&_response;
}

const char* LscClientRequest::getContent(unsigned char* buf, uint32* maxLen)
{
	return NULL;
}

bool LscClientRequest::getTransportParam(IN const char* subkey, OUT char* value,IN OUT uint16* maxLen)
{
//	assert(NULL);
	return false;
}
const char* LscClientRequest::getHeader(IN const char* key, OUT char* value, IN OUT uint16* maxLen) 
{
	KEYVALUEPIR::const_iterator it = _header.find (key);
	if (it == _header.end ()) 
	{
		return NULL;
	}
	else
	{
		int iSize = *maxLen > ( it->second.length()+1 ) ? ( it->second.length()+1 ): *maxLen;
		memcpy (value,it->second.c_str(),iSize-1);
		value[iSize] = '\0';
		*maxLen = iSize-1;
		return value;
	}	
	return NULL;
}

const char* LscClientRequest::getProtocol( char* protocol , int len)
{
	const lsc::LSCMESSAGE& msg = _lscMessage->GetLscMessageContent ();
	const lsc::StandardHeader_t& stdHeader = msg.jump.header;
	snprintf(protocol,len-1,"%d", stdHeader.version );
	return protocol;
}



bool LscClientRequest::getLocalInfo(LocalInfo& info)
{
	if (info.size != sizeof(RemoteInfo))
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"getLocalInfo() Invalid input parameter"));
		return false;
	}

	IConnectionInternalPtr conn = getConnection();
	
	if (!conn)
		return false;
	
	if (!conn->isActive())
		return false;
	
	bool result = conn->getLocalIP( info.ipaddr, info.addrlen, 	&info.port) != NULL;
	
	conn = NULL;
	
	return result;
}

REQUEST_VerbCode LscClientRequest::getVerb ()
{
	return _verb;
}
const char* LscClientRequest::getStartline( char *buf , int bufLen )
{
	memset(buf,0,bufLen);
	return buf;
//	assert(false);
//	glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"getStartline() DO NOT Invoke this method in LSC protocol mode") );
	return NULL;
}

const char* LscClientRequest::getUri(char* uri, int len)
{
	memset(uri,0,len);
	return uri;
//	glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"getUri() DO NOT Invoke this method in LSC protocol mode") );
	return NULL;
}

bool LscClientRequest::ParseLscIntoRtspHeader ( )
{
	if (!_lscMessage) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"ParseLscIntoRtspHeader() Invalid lsc message instance,program logical error"));
		return false;
	}

	_header.clear ();


	//change the lsc message into rtsp format
	/*-----------------------------------------------------------------*/	
	//add standard header value into RTSP format
	const lsc::LSCMESSAGE& msg = _lscMessage->GetLscMessageContent ();
	const lsc::StandardHeader_t& stdHeader = msg.jump.header;
	
	switch( _lscMessage->GetLscMessageOpCode() ) 
	{
	case lsc::LSC_PAUSE:
		_verb = RTSP_MTHD_PAUSE;
		{
			if (! AddHeaderField (LSC_HEADER_RANGE,msg.pause.stopNpt) )
				return false;
		}
		break;

	case lsc::LSC_PLAY:				
	case lsc::LSC_JUMP:		
	case lsc::LSC_RESUME:
		_verb = RTSP_MTHD_PLAY;
		{
			if (!AddScaleField(msg.play.data.numerator,msg.play.data.denominator) ||
				!AddRangeField(LSC_HEADER_RANGE , msg.play.data.startNpt , msg.play.data.stopNpt ) )
			{
				return false;
			}
		}
		break;
	case lsc::LSC_STATUS:
		_verb = RTSP_MTHD_GET_PARAMETER;	//This can be treated as GET_PARAMETER but not heart beat
		{
#pragma message(__MSGLOC__"IN TianShan Spec, no parameter mean HEART-BEAT")
		}
		break;

	case lsc::LSC_RESET:
#pragma message(__MSGLOC__"I do not known what's RESET command")
#pragma message(__MSGLOC__"A reset request instructs the LSCP Server to reset the state machine to the Open mode.")
		break;

	case lsc::LSC_PAUSE_REPLY:
	case lsc::LSC_RESUME_REPLY:
	case lsc::LSC_STATUS_REPLY:
	case lsc::LSC_RESET_REPLY:
	case lsc::LSC_JUMP_REPLY:
	case lsc::LSC_PLAY_REPLY:
	case lsc::LSC_DONE:
		_verb = RTSP_MTHD_RESPONSE;
		break;
	default:
		_verb =  RTSP_MTHD_UNKNOWN;
		break;
	}
	
	AddHeaderField ( LSC_HEADER_CSEQ , stdHeader.transactionId );
	AddSessionField (stdHeader.streamHandle );

	return true;
	
}
bool LscClientRequest::AddSessionField(unsigned int value)
{
	char	szBuf[128];
	memset(szBuf,0,sizeof(szBuf));
	snprintf(szBuf,sizeof(szBuf)-1,"%012u",value);
	_header[std::string(LSC_HEADER_SESSION)] = std::string(szBuf);
	return true;
}
bool LscClientRequest::AddRangeField(const char* key ,unsigned int value ,unsigned int value2)
{
	char	szBuf[128];
	char	szValue[32];
	char	szValue2[32];

	memset(szValue,0,sizeof(szValue));
	memset(szValue2,0,sizeof(szValue2));
	memset(szBuf,0,sizeof(szBuf));

	if ( value == lsc::LSC_POSITION_EOS ) 
	{
		//set it to empty
		strcpy( szValue , "" );
	}
	else if ( value >= 0x80000000 ) 
	{	
		if ( value == lsc::LSC_POSITION_NOW ) 
		{
			strcpy(szValue,"now");
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"Invalid Range paramter %d"),value);
		}
	}
	else 
	{
		sprintf( szValue , "%u" , value);
	}

	if ( value2 == lsc::LSC_POSITION_EOS ) 
	{
		//set it to empty
		strcpy(szValue2,"");
	}
	else if (value2 >= 0x80000000) 
	{	
		if (value2 == lsc::LSC_POSITION_NOW ) 
		{
			strcpy(szValue2,"now");
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"Invalid Range paramter %d"),value2);
		}
	}
	else
	{
		sprintf (szValue2,"%u",value2);
	}
	
	snprintf( szBuf , sizeof(szBuf)-1 , "npt= %s - %s", szValue , szValue2 );

	_header[std::string(key)] = std::string(szBuf);
	
	return true;
}

bool LscClientRequest::AddHeaderField( const char* key ,unsigned int value )
{

	char	szBuf[128];
	memset(szBuf,0,sizeof(szBuf));
	snprintf(szBuf,sizeof(szBuf)-1,"%u",value);
	_header[std::string(key)] = std::string(szBuf);

	return true;
	
}

bool LscClientRequest::AddScaleField(short numerator , unsigned short denominator)
{
	char szBuf[128];
	memset(szBuf,0,sizeof(szBuf));
	if ( denominator != 0) 
	{
		float fScale = (float)numerator /(float) denominator;
		snprintf(szBuf,sizeof(szBuf)-1,"%f",fScale);
		_header[ std::string(LSC_HEADER_SCALE) ] = std::string(szBuf);
		return true;
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(LscClientRequest,"Invalid LSC_MESSAGE WITH scale[%d|%d]"),numerator,denominator);
		return false;
	}	
}

void LscClientRequest::setContext(const char* key  , const char* value) 
{
	if( key && key[0]!=0 )
	{
		if(value)
		{
			_contextMap[key]= value;
		}
		else
		{
			_contextMap.erase(key);
		}
	}
}

const char* LscClientRequest::getContext( const char* key )
{
	if( ! ( key && key[0] != 0 ))
	{
		return NULL;
	}
	std::map<std::string,std::string>::const_iterator it = _contextMap.find(key);
	if( it != _contextMap.end() )
	{
		return it->second.c_str();
	}
	else
	{
		return NULL;
	}
}

/*------------------------------------------------------------------------
LscServerRequest
------------------------------------------------------------------------*/
LscServerRequest::LscServerRequest(IConnectionInternalPtr conn,const char* clientSessID)
:_ref(0)
{
	assert(conn != NULL);
	_conn = conn;
	//initialize lsc message	
	memset(&_msg,0,sizeof(_msg));
	_msg.jump.header.opCode = lsc::LSC_DONE;
	_msg.jump.header.version = lsc::protocolVersion;
	if (!clientSessID) 
	{
		throw "Invalid clientSessionID passed in";
	}
	_msg.jump.header.streamHandle =(uint32)atoi(clientSessID);

	_strClientSessionID = clientSessID;
}

LscServerRequest::~LscServerRequest ()
{
	_conn = NULL;
}

void LscServerRequest::reference ()
{
	ZQ::common::MutexGuard gd(_refLock);
	++_ref;
}

void LscServerRequest::release ()
{
	long lRef = 0;
	{
		ZQ::common::MutexGuard gd(_refLock);
		lRef = --_ref;

	}
	if (lRef == 0) 
	{
		delete this;
	}
}
int LscServerRequest::printCmdLine(const char* startline)
{
	return 0;
}
int LscServerRequest::printMsgBody(char* msg)
{
	return 0;
}
int LscServerRequest::printHeader(char* key, char* value)
{
	glog(ZQ::common::Log::L_DEBUG,CLOGFMT(LscServerRequest,"printHeader() enter with key[%s] value[%s]"),key,value);
	if ( stricmp( LSC_HEADER_CSEQ , key ) == 0 ) 
	{//CSeq
		lsc::StandardHeader_t& stdHeader = _msg.jump.header;
		stdHeader.transactionId = (uint8)atoi(value);		
	}
	else if ( stricmp( LSC_HEADER_SESSION , key) == 0 ) 
	{//Session
		lsc::StandardHeader_t& stdHeader = _msg.jump.header;
		stdHeader.streamHandle = atoi(value);
	}	
	else if ( stricmp( LSC_HEADER_SCALE,key) == 0 )
	{//Scale
		float scale = (float)atof(value);
		short numerator = (short)(scale * 1000);
		unsigned short denominator = 1000;
	
		_msg.response.data.numerator = numerator;
		_msg.response.data.denominator = denominator;	
	}
	else if ( stricmp (LSC_HEADER_RANGE , key ) == 0 ) 
	{//range		
		float fNpt = 0.0f;
		sscanf(value,"npt=%f-",&fNpt);
		glog(ZQ::common::Log::L_DEBUG,CLOGFMT(LscResponse,"convert range to [%f]"),fNpt);
		_msg.response.data.currentNpt = (unsigned long)(fNpt*1000);
	}
	else if ( stricmp (LSC_HEADER_STATUS , key) == 0 ) 
	{//server status
		_msg.response.header.statusCode =	convertStringToStatusCode(value);
	}
	else if ( stricmp( LSC_HEADER_SERVERMODE , key ) == 0 ) 
	{//server mode
		_msg.response.data.mode = ConvertStringToServerMode (value);
	}
	
	return 1;
}
int LscServerRequest::closeConnection()
{
	_conn->close ();
	return 1;
}
const char* LscServerRequest::getClientSessionID()
{
	return _strClientSessionID.c_str();
}

int LscServerRequest::post()
{
	assert( _conn != NULL );
	_msg.response.header.hton ();
	_msg.response.data.hton ();

	uint32 ret = _conn->send ( (uint8*)&_msg , sizeof(_msg.response) );

	return ret;
}

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE
