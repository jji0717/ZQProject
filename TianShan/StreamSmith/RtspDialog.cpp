// RtspDialog.cpp: implementation of the RtspDialog class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)
#include "global.h"
#include "StreamSmithSite.h"
#include "RtspSession.h"
#include "RtspDialog.h"

#include <vector>
#include <deque>
#include "SystemUtils.h"

#ifdef _RTSP_PROXY
	#include "rtspProxyConfig.h"
#else
	#include "StreamSmithConfig.h"
#endif




#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

using namespace ZQ::common;
ZQ::common::Mutex ParseProcThrd::_lock;
std::deque<ParseProcThrd::PARSEREQUEST>	ParseProcThrd::_req_que;
bool		ParseProcThrd::_bQuit;
#ifdef ZQ_OS_MSWIN
HANDLE		ParseProcThrd::_hEvent;
#else
sem_t		ParseProcThrd::_hEvent;
#endif
std::vector<ParseProcThrd*>ParseProcThrd::	_parse_thd;
ZQ::common::Mutex ParseProcThrd::_buflock;
std::deque<RTSPBUFFER>	ParseProcThrd::_rtsp_buf_que;
std::deque<RTSPBUFFER>	ParseProcThrd::_rtsp_bigbuf_que;

char* MonthIndexToString[] ={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

#ifdef ZQ_OS_MSWIN
bool ParseProcThrd::create(int nThreadCount, int nThreadPriority)
{
	_bQuit = false;
	_hEvent = CreateSemaphore(NULL, 0, 100000, NULL);

	_parse_thd.resize(nThreadCount);
	for(int i=0;i<nThreadCount;i++)
	{
		_parse_thd[i] = new ParseProcThrd();
		if(nThreadPriority!=THREAD_PRIORITY_NORMAL)
		{
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, TRUE, _parse_thd[i]->id());
			if (!SetThreadPriority(hThread, nThreadPriority))
			{
				glog(ZQ::common::Log::L_ERROR, "Failed to set thread [%d] priority to %d",
					_parse_thd[i]->id(), nThreadPriority);
			}
			else
			{
				glog(ZQ::common::Log::L_INFO, "Set thread [%d] priority to %d successful",
					_parse_thd[i]->id(), nThreadPriority);
			}
			if (hThread!=INVALID_HANDLE_VALUE)
				CloseHandle(hThread);
		}
		
		_parse_thd[i]->start();
	}

	return true;
}

#else
bool ParseProcThrd::create(int nThreadCount, int nThreadPriority)
{
	_bQuit = false;
	int re = sem_init(&_hEvent, 0, 0);
	if(re != 0)
	{
		glog(ZQ::common::Log::L_ERROR,"ParseProcThrd::create() semaphore init failed string[%s]",strerror(errno));
		return false;
	}

	_parse_thd.resize(nThreadCount);
	for(int i=0;i<nThreadCount;i++)
	{
		_parse_thd[i] = new ParseProcThrd();
		////////////////
		// set thread priority
		///////////////
		
		_parse_thd[i]->start();
	}

	return true;
}
#endif

void ParseProcThrd::close()
{
	//set bQuit to ytue so that all thread can quit
	_bQuit = true;
	glog(ZQ::common::Log::L_INFO, "ParseProcThrd to uninitialize");				

#ifdef ZQ_OS_MSWIN
	if (_hEvent && _hEvent != INVALID_HANDLE_VALUE)
	{		
		for( int k= 0 ;k <(int)_parse_thd.size() ; k++)
		{
			ReleaseSemaphore(_hEvent , 1, NULL );
		}
		for( int i=0 ; i < (int)_parse_thd.size();i++ )
		{
			_parse_thd[i]->waitHandle(1000);
			delete _parse_thd[i];
		}

		CloseHandle(_hEvent);

		_hEvent = NULL;
	}
#else
	int nval = 0;
	try
	{
		if(sem_getvalue(&_hEvent, &nval) == 0)
		{
			for( int k= 0 ;k <(int)_parse_thd.size() ; k++)
			{
				sem_post(&_hEvent);
			}
			for( int i=0 ; i < (int)_parse_thd.size();i++ )
			{
				_parse_thd[i]->waitHandle(1000);
				delete _parse_thd[i];
			}
		}
		sem_destroy(&_hEvent);		
	}
	catch(...){}
	
#endif

	_buflock.enter();
	RTSPBUFFER buf;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ParseProcThrd, "big_rtsp_buf_allocated[%d], rtsp_buf_allocated[%d]"),
		_rtsp_bigbuf_que.size(), _rtsp_buf_que.size());				

	while(!_rtsp_bigbuf_que.empty())
	{
		buf=_rtsp_bigbuf_que.front();
		_rtsp_bigbuf_que.pop_front();
		if (buf.buf)
		{
			delete buf.buf;
			buf.buf=NULL;
		}
	}
	while(!_rtsp_buf_que.empty())
	{
		buf=_rtsp_buf_que.front();
		_rtsp_buf_que.pop_front();
		if (buf.buf)
		{
			delete buf.buf;
			buf.buf=NULL;
		}
	}

	_buflock.leave();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ParseProcThrd, "close() uninitialized"));				
}

bool ParseProcThrd::processReq(PARSEREQUEST& parseReq)
{	
	static const char* badReqMsg = 
		"RTSP/1.0 400 Bad request" CRLF CRLF;
	static int nBadReqLen = strlen(badReqMsg);

	RtspConnectionPtr	conn = parseReq.pConn;
	int bytesDecoded=0;
	RtspMsg* msg = NULL;		

	//dump the error rtsp message
	{
		char ipaddr[0x20];
		uint16 port=0;
		if (conn->getRemoteIP(ipaddr, sizeof(ipaddr), &port) == NULL) 
		{
			ipaddr[0] = '\0';
		}

		char hint[0x200];
		sprintf(hint, "processReq() SOCKET: "FMT64" %s:%hu", conn->getConnectionIdentity(), ipaddr, port);
		glog.hexDump(Log::L_DEBUG, parseReq.pReq.buf, parseReq.pReq.dataLen, hint, true);
	}

	do
	{
		try 
		{
			msg = RtspMsgParser::preParse( parseReq.pReq.buf, parseReq.pReq.dataLen, bytesDecoded);			
		}
#ifdef ZQ_OS_MSWIN
		catch (...) 
		{
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%04x] %s RTSP preParse() occurred a exception"), GetCurrentThreadId(), conn->getLogHint());
			break;
		}
			
		if( msg->getCompletionFlag() != RTSP_MSG_COMPLETE )
		{
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%04x] %s not a complete package"), GetCurrentThreadId(), conn->getLogHint());			
			break;
		}
			
		RtspRequest* req = DYNAMIC_CAST<RtspRequest*>(msg);
		if (req == NULL) 
		{				
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%04x] %s invalid rtsp message"), GetCurrentThreadId(), conn->getLogHint());						
			break;
		}

		try 
		{
			req->parse(); // parse request header
		}
		catch (...) 
		{
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%04x] %s RTSP parse() occurred a exception"), GetCurrentThreadId(), conn->getLogHint());					
			break;
		}
#else
		catch (...) 
		{
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%08x] %s RTSP preParse() occurred a exception"), pthread_self(), conn->getLogHint());
			break;
		}
			
		if( msg->getCompletionFlag() != RTSP_MSG_COMPLETE )
		{
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%08x] %s not a complete package"), pthread_self(), conn->getLogHint());			
			break;
		}
			
		RtspRequest* req = DYNAMIC_CAST<RtspRequest*>(msg);
		if (req == NULL) 
		{				
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%08x] %s invalid rtsp message"), pthread_self(), conn->getLogHint());						
			break;
		}

		try 
		{
			req->parse(); // parse request header
		}
		catch (...) 
		{
			glog(Log::L_ERROR, CLOGFMT(ParseProcThrd, "ThreadID[%08x] %s RTSP parse() occurred a exception"), pthread_self(), conn->getLogHint());					
			break;
		}

#endif

		RtspClientRequest* cliReq = new RtspClientRequest(req, conn, parseReq.dwRecvTime);

        //post response directly when method is OPTIONS
        if (cliReq->getVerb() == RTSP_OPTIONS_MTHD)
        {
			cliReq->addRef();
            char szSeq[256];
            memset(szSeq,0,sizeof(szSeq));
            uint16 seqBufLen = sizeof(szSeq);
            const char* pSeq = cliReq->getHeader("CSeq",szSeq,&seqBufLen);

            IServerResponse* pResponse = cliReq->getResponse();
            if(pResponse)
            {
                pResponse->printf_preheader("RTSP/1.0 200 OK");
                pResponse->setHeader("CSeq", pSeq);
                pResponse->setHeader("Public", "SETUP, TEARDOWN, PLAY, PAUSE");
                pResponse->post();
				cliReq->release();
                return true;
            }else{
				cliReq->release();
                return false;
            }
        }
		
		{
			ClientRequestGuard gd(cliReq);

			cliReq->setHeader("SYS#ConnID", (char*)conn->getConnIdent());
			cliReq->setHeader("SYS#LocalServerIP",(char*)conn->getLocalIp());
			cliReq->setHeader("SYS#LocalServerPort",(char*)conn->getLocalPort());

			ZQ::StreamSmith::StreamSmithSite::getDefaultSite()->postRequest(cliReq, IClientRequest::FixupRequest);				
		}
		
		return true;
	}while(0);

	conn->send((const uint8* )badReqMsg, nBadReqLen);
	if (msg)
	{
		delete msg;
	}

	//dump the error rtsp message
	char tmp[64];

#ifdef ZQ_OS_MSWIN
	sprintf(tmp, "ThreadID[%04x] BadRequest", GetCurrentThreadId());
#else
	sprintf(tmp, "ThreadID[%08x] BadRequest", (unsigned int)pthread_self());
#endif

	glog.hexDump(Log::L_INFO, parseReq.pReq.buf, parseReq.pReq.dataLen, tmp);
	
	return false;
}

int ParseProcThrd::run(void)
{
#ifdef ZQ_OS_MSWIN
	glog(Log::L_INFO, CLOGFMT(ParseProcThrd, "ThreadID[%d] RTSP parser thread entered"), GetCurrentThreadId());	
#else
	glog(Log::L_INFO, CLOGFMT(ParseProcThrd, "ThreadID[%08x] RTSP parser thread entered"), pthread_self());	
#endif

	while(!_bQuit)
	{
#ifdef ZQ_OS_MSWIN
		DWORD dwRet = WaitForSingleObject(_hEvent, INFINITE);
		if( _bQuit )
			break;
		if ( dwRet != WAIT_OBJECT_0 )
			break;
#else
		int Ret = sem_wait(&_hEvent);
		if( _bQuit )
			break;
		if ( Ret != 0)
		{
			if(errno == EINTR)
				continue;
			break;
		}
#endif

		while(_req_que.size()&&!_bQuit)
		{
			PARSEREQUEST parseReq;

			{
				ZQ::common::Guard<Mutex> op(_lock);
				if (!_req_que.empty())
				{
					parseReq = _req_que.front();
					_req_que.pop_front();
				}
				else
				{
					break;
				}
			}

			//do something
			processReq(parseReq);

			//free allocated resource
			freeRtspBuf(parseReq.pReq);
			parseReq.pConn = NULL;
		};

	}

#ifdef ZQ_OS_MSWIN
	glog(Log::L_INFO, CLOGFMT(ParseProcThrd, "ThreadID[%d] RTSP parser thread left"), GetCurrentThreadId());	
#else
	glog(Log::L_INFO, CLOGFMT(ParseProcThrd, "ThreadID[%08x] RTSP parser thread left"), pthread_self());	
#endif
	return 0;
}

RtspConnection::RtspConnection(IDataCommunicatorPtr communicator) : _mainConn(communicator), _ref(0)
{	  
	glog(Log::L_DEBUG, "RtspConnection()\tCreate Rtsp Connection with id[%llu]", 
			_mainConn->getCommunicatorId());

	CommAddr peer;
	CommAddr local;
	communicator->getCommunicatorAddrInfo(local, peer);
	
	char	szPeerInfo[256]={0};
	char	szLocalInfo[256]={0};
	
#ifdef ZQ_OS_MSWIN
	DWORD  dwPeerInfoSize = sizeof(szPeerInfo);
	DWORD  dwLocalInfoSize = sizeof(szLocalInfo);
	WSAAddressToStringA((sockaddr*)&peer.u.addr, peer.addrLen, NULL, szPeerInfo, &dwPeerInfoSize);
	WSAAddressToStringA((sockaddr*)&local.u.addr,local.addrLen, NULL, szLocalInfo, &dwLocalInfoSize);
#else
	size_t peerbufs = sizeof(szPeerInfo);
	int peerPort = 0;
	size_t localbufs = sizeof(szLocalInfo);
	int localPort = 0;
	bool bre = false;
	char chport[10] = {0};
	bre = sockaddrToString((struct ::sockaddr*)&peer.u.addr, szPeerInfo, peerbufs,peerPort);
	if(!bre)
		glog(Log::L_ERROR, CLOGFMT(RtspConnection, "RtspConnection() sockaddrToString convent peer xockaddr failed"));
	sprintf(chport,":%d",peerPort);
	strcat(szPeerInfo,chport);
	
	bre = sockaddrToString((struct ::sockaddr*)&local.u.addr, szLocalInfo, localbufs,localPort);
	if(!bre)
		glog(Log::L_ERROR, CLOGFMT(RtspConnection, "RtspConnection() sockaddrToString convent local xockaddr failed"));
	memset(chport,0,sizeof(chport));
	sprintf(chport,":%d",localPort);
	strcat(szLocalInfo,chport);
#endif

	char* pColon = NULL;
	pColon = strrchr(szPeerInfo,':');
	if (pColon) 
	{
		_strPeerIP.assign( szPeerInfo , pColon - szPeerInfo);
		_strPeerPort = pColon+1;
		_iPeerPort = atoi(_strPeerPort.c_str());
	}
	
	pColon = strrchr(szLocalInfo,':');
	if ((pColon)) 
	{
		_strLocalIP.assign(szLocalInfo,pColon-szLocalInfo);
		_strLocalPort = pColon+1;
		_iLocalPort = atoi(_strLocalPort.c_str());
	}
	
	//set current connection identity to clientRequest
	sprintf(_szConnIdent,FMT64U, _mainConn->getCommunicatorId());
	sprintf(_szLogHint, "RtspConn() SOCKET: "FMT64" %s:%s", communicator->getCommunicatorId(), _strPeerIP.c_str(), _strPeerPort.c_str());

	glog(Log::L_INFO, CLOGFMT(RtspConnection, "RtspConnection() connected:\t SOCKET connected from client [%s:%s] to server [%s:%s] and id[%llu]"),
		_strPeerIP.c_str(), _strPeerPort.c_str(),
		_strLocalIP.c_str(),_strLocalPort.c_str(),
		_mainConn->getCommunicatorId() );
	
	setConnectionProtocol(IConnectionInternal::CONNECTION_PROTCOL_TYPE_RTSP);
}

RtspConnection::~RtspConnection()
{
	int64 commId = _mainConn->getCommunicatorId();
	{
		ZQ::common::MutexGuard guard(*this);
		_mainConn->close();
	}
	glog(Log::L_INFO, CLOGFMT(RtspConnection, "destructor: client [%s:%s] to server [%s:%s] and id[%llu]"),
		_strPeerIP.c_str(), _strPeerPort.c_str(),
		_strLocalIP.c_str(),_strLocalPort.c_str(),
		commId );
}

bool RtspConnection::close()
{
	ZQ::common::MutexGuard guard(*this);
	glog(Log::L_INFO, CLOGFMT(RtspConnection, "close() this(%llu), _lastSessionId = %s"),
			_mainConn->getCommunicatorId(), _lastSessionId.c_str());	
	_mainConn->close();
	return true;
}

bool RtspConnection::assocSession(RtspSession* sess)
{
//	ZQ::common::MutexGuard guard(*this);
//	if(sess == NULL)
//	{
//		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) 
//		{
//			glog(Log::L_DEBUG, "RtspConnection::assocSession():\t "
//				"NULL RtspSession pointer passed in " );
//		}
//		return false;
//	}
//	std::string strSessID=sess->getSessionID();
//	if( _sessMap.find(strSessID)!=_sessMap.end() )
//	{
//		//already associated
//		return true;
//	}
//	sess->reference();
//	_sessMap.insert( SESSMAP::value_type(strSessID,sess) );	
//	return true;
	return true;
}

void RtspConnection::sessiondownNotify(const std::string& strSessID)
{
//	ZQ::common::MutexGuard guard(*this);
//	SESSMAP::iterator it = _sessMap.find(strSessID);
//	if( it != _sessMap.end() )
//	{
//		it->second->release();
//		_sessMap.erase(it);
//	}
}
void RtspConnection::connectionDown()
{
//	//notify all associated session
//	ZQ::common::MutexGuard guard(*this);
//	char szBuf[256];
//	ZeroMemory( szBuf,sizeof(szBuf) );
//	sprintf(szBuf,"%llu",_mainConn.getConnectionIdentity());
//	std::string	connIdent = szBuf;
//	SESSMAP::iterator it =_sessMap.begin();
//	for( ; it != _sessMap.end() ; it++ )
//	{
//		try
//		{
//			it->second->connectionDownNotify(szBuf);
//		}
//		catch(...){}
//	}
}

//////////////////////////////////////////////////////////////////////////

#define REQ_COUNT_INC()			{ZQ::common::MutexGuard gd(req_countLock); ++req_count;}
#define REQ_COUNT_DEC()			{ZQ::common::MutexGuard gd(req_countLock); --req_count;}
#define PRINT_REQ_COUNT(PRE)	\
	glog(Log::L_DEBUG, CLOGFMT(REQC, PRE "REQ_COUNT = %d, this = %p"), req_count, this)

static ZQ::common::Mutex req_countLock;
static volatile long req_count = 0;

//////////////////////////////////////////////////////////////////////////

RtspClientRequest::RtspClientRequest(RtspRequest* req, 
									 RtspConnectionPtr conn, int64 dwRecvTime) : 
	  _rtspReq(req), _conn(conn), _response(*this, dwRecvTime),_ref(0),_requestInitTime(dwRecvTime)
{
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) {
		REQ_COUNT_INC();
		PRINT_REQ_COUNT("RtspClientRequest constructor");
		glog(Log::L_DEBUG, CLOGFMT(RtspClientRequest, "RtspClientRequest() this(%p), _conn = %p"), this, conn.get());
	}

	
	_site = ZQ::StreamSmith::StreamSmithSite::getDefaultSite();
	_phase = FixupRequest;
	// InitializeCriticalSection(&_connCritSec);
	char strRecvTime[128];sprintf(strRecvTime,"%lld",dwRecvTime);
	setContext("SYS.RequestTime",strRecvTime);
	setHeader( KEY_MESSAGE_RECVTIME , strRecvTime);
	setHeader("Server", (char*)_GlobalObject::getServerHdr());
	
	_response.setHeader("CSeq", req->getCSeq().getDataBuf());
	_response.setHeader("Session", req->getSessionId().getDataBuf());
	_response.setHeader("Server", _GlobalObject::getServerHdr());
#pragma message(__MSGLOC__"TODO:add connection identity here")
	
}

void RtspClientRequest::trimString(char* str)
{
	int firstNonspace;
	int lastNonspace;
	char* c = str;

	while (*c) {
		if (*c != '\x20')
			break;

		c ++;
	}

	firstNonspace = c - str;

	int origLen = strlen(str);

	c = str + origLen - 1;
	
	while (c >= str) {
		if (*c != '\x20')
			break;
		c --;
	}

	lastNonspace = c - str;
	int len = lastNonspace - firstNonspace + 1;
	if (firstNonspace == 0 && origLen == len)
		return;
	else {
		memmove(str, str + firstNonspace, len);
		str[len] = 0;
	}
}

RtspClientRequest::~RtspClientRequest()
{
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) {
		REQ_COUNT_DEC();
		PRINT_REQ_COUNT("RtspClientRequest destruct ");
	}

	glog(Log::L_DEBUG, CLOGFMT(RtspConnection, "~RtspClientRequest() this=%p, release _conn=%llu"), this, _conn->getConnectionIdentity());
	
	if (_conn)
		_conn = NULL;

	if (_rtspReq)
	{
		delete _rtspReq;
		_rtspReq = NULL;
	}

#ifdef _DEBUG
	_conn = NULL;
	_site = NULL;
	_rtspReq = NULL;
#endif

	// _rtspReq = NULL;
	// DeleteCriticalSection(&_connCritSec);
}

//////////////////////////////////////////////////////////////////////////

uint32 RtspServerResponse::post()
{
	char buf[512];
	IConnectionInternalPtr conn;
	conn = _rtspReq.getConnection();
	if (conn == NULL)
		return 0;

	std::string res;
	Data theData, theName;

	bool bFoundDateInHeader = false;
	const Data startLine = _rtspResponse.getStartLine();
	res += startLine.getData();
	res += CRLF;
	u_int32_t count = _rtspResponse.getAddedHdrCount();
	for (u_int32_t i = 0; i < count ; i ++) 
	{		
		_rtspResponse.getAddedDataByIndex(i, NULL, theName, theData);
		if (theData.length() == 0) 
		{//if the data is NULL ,do not response to client
			continue;
		}
		if (!bFoundDateInHeader)
		{
#ifdef ZQ_OS_MSWIN
			if (stricmp(theName.getData (),"Date") == 0) 
#else
			if (strcasecmp(theName.getData (),"Date") == 0) 
#endif
			{
				bFoundDateInHeader = true;
			}
		}
		
		res += theName.getData();
		res += ": ";
		res += theData.getData();
		res += CRLF;
		
	}
	if (!bFoundDateInHeader) 
	{
		res += "Date";
		res += ": ";
		//get gmt time
		char szBuf[512];
		long	tzBias = 0;
		//get gmt time
#ifdef _RTSP_PROXY
		if ( gRtspProxyConfig.lEnableUseLocaltimeInDatHeader >= 1) 
		{
			
			tzBias = gRtspProxyConfig.lCurTimeZone;
			
#else
		if(gStreamSmithConfig.lEnableUseLocaltimeInDatHeader >= 1)
		{	
			tzBias = gStreamSmithConfig.lCurTimeZone;
#endif
#ifdef ZQ_OS_MSWIN
			SYSTEMTIME st;
			GetLocalTime(&st);
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03d GMT%s%02d",
				st.wDay,
				MonthIndexToString[st.wMonth-1],
				st.wYear,
				st.wHour,
				st.wMinute,
				st.wSecond,
				st.wMilliseconds,
				tzBias >= 0 ?"+":"-",
				tzBias);
			
		}
		else
		{
			SYSTEMTIME st;
			GetSystemTime (&st);
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03d GMT",
				st.wDay,
				MonthIndexToString[st.wMonth-1],
				st.wYear,
				st.wHour,
				st.wMinute,
				st.wSecond,
				st.wMilliseconds);
		}
#else
			struct timeval tval;
			gettimeofday(&tval,(struct timezone*)NULL);
			struct tm* ptm = NULL;
		   	ptm = localtime(&tval.tv_sec);	
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03ld GMT%s%02ld",
				ptm->tm_mday,
				MonthIndexToString[ptm->tm_mon],
				ptm->tm_year,
				ptm->tm_hour,
				ptm->tm_min,
				ptm->tm_sec,
				tval.tv_usec/1000,
				tzBias >= 0 ?"+":"-",
				tzBias);
		}
		else
		{
			struct timeval tval;
			gettimeofday(&tval,(struct timezone*)NULL);
			struct tm* ptm = NULL;
		   	ptm = gmtime(&tval.tv_sec);
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03ld GMT",
				ptm->tm_mday,
				MonthIndexToString[ptm->tm_mon],
				ptm->tm_year+1900,
				ptm->tm_hour,
				ptm->tm_min,
				ptm->tm_sec,
				tval.tv_usec/1000);
		}
#endif

		res += szBuf;
		res += CRLF;
	}
	
	theData = _rtspResponse.getMsgBody();
	if (theData.length()) {
		res += "Content-Length";
		res += ": ";
		sprintf(buf, "%d", theData.length());
		res += buf;
		res += CRLF;
		res += CRLF;
		res += theData.getData();
	}
	else
		res += CRLF;

	u_int32_t len = res.length();

	char hint[0x200];
//	SOCKET socket = (SOCKET )conn->getSocket();
	sprintf(hint, "SOCKET: "FMT64" ", conn->getConnectionIdentity());

	if (conn->isActive()) 
	{
		int64 timeTemp = ZQ::common::now();
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspServerResponse, "post() sending data thru COMM[%lld]"), conn->getConnectionIdentity());
		if (conn->send((uint8* )res.c_str(), len) <= 0)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(RtspServerResponse, "post() failed to send data thru COMM[%lld]"), conn->getConnectionIdentity());			
		}
		else
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspServerResponse, "post() sent thru COMM[%lld] and took [%lld]ms"),conn->getConnectionIdentity(), ZQ::common::now()-timeTemp);
	} 
	else 
	{
		glog(Log::L_DEBUG, CLOGFMT(RtspServerResponse, "post() COMM[%lld] is closed"), conn->getConnectionIdentity() );		
	}



	char ipaddr[0x20];
	uint16 port;
	if (conn->getRemoteIP(ipaddr, sizeof(ipaddr), &port) == NULL)
	{
		glog(Log::L_ERROR, CLOGFMT(RtspServerResponse, "post() RtspServerResponse::post() getRemoteIP() failed"));
		return 0;
	}

	//record session ID and seq for log use
	Data DataSessionID,DataSeq;
	CharData DataVerb;
	DataSessionID =  _rtspResponse.getAddedHdrBodyData("Session");
	DataSeq  = _rtspResponse.getAddedHdrBodyData("CSeq");	
	uint32	rtspVerb = (uint32)_rtspReq.getVerb();
	DataVerb = RtspUtil::getMethodInString(_rtspReq.getVerb());

	int64 timeCost = ZQ::common::now();
	int64 timeCostTotal = timeCost;
	if( timeCost > _constructTime )
	{
		timeCost = timeCost-_constructTime;		
	}
	else
	{
		timeCost = 0;
	}
	if( timeCostTotal > _recvTime )
	{
		timeCostTotal = timeCostTotal-_recvTime;
	}
	else
	{
		timeCostTotal = 0;	
	}

	char* pVerString = (char*)DataVerb.getPtr();
	if (!(pVerString&&strlen(pVerString)>0)) 
	{
		pVerString="outbond";
	}

    {
        RtspPerformance::RequestRecord record;
        record.processedTimeMSec	= (uint32) timeCost;
        record.durationMSec			= (uint32) timeCostTotal;
		record.rtspVerb				= rtspVerb;
		int responseCode = 0;
		int versionMajor = 0;
		int versionMinor = 0;
		sscanf(startLine.getData(),"RTSP/%d.%d %d",&versionMajor, &versionMinor, &responseCode);
		record.bSucceeded			= ( responseCode/100 == 2 );
        _GlobalObject::perfmon.collect(record);
    }
	glog(ZQ::common::Log::L_INFO, CLOGFMT(RtspServerResponse, "Request processed: session[%s] seq[%s] verb[%s] duration=[%lld/%lld]msec startline [%s] socket(%lld %s:%hu)"),
				DataSessionID.getData(),DataSeq.getData(),pVerString,
				timeCost,timeCostTotal,startLine.getData(),
				conn->getConnectionIdentity(),ipaddr,port);
	_GlobalObject::responseCounters.addCount(pVerString, startLine.getData());

	sprintf(hint, "SOCKET: "FMT64" %s:%hu", conn->getConnectionIdentity(), ipaddr, port);
	glog.hexDump(Log::L_INFO, res.c_str(), len, hint, true);

	conn = NULL;

	return len;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

RtspDialog::RtspDialog(RtspSessionMgr& sessionMgr):
	_sessionMgr(sessionMgr)

{
	_strSavedMsg.reserve(2048);
}

RtspDialog::~RtspDialog()
{
#ifdef _DEBUG
	_conn = NULL;
#endif
}


void RtspDialog::onCommunicatorSetup(IDataCommunicatorPtr communicator)
{
	_conn = RtspConnectionPtr( new RtspConnection(communicator) );
	assert(_conn);
	_sessionMgr.onconnectionSetup(_conn);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(RtspDialog,"onCommunicatorSetup() %lld / %s"),
		communicator->getCommunicatorId(), _conn->getLogHint());
}

long MAX_REQPACKAGE_SIZE = 16*1024;

bool RtspDialog::onRead(const int8 *buffer, size_t bufSize)
{
	
	int64 dwRecvTime = ZQ::common::now();

	const char* szLogHint=_conn->getLogHint();
#ifdef _RTSP_PROXY
	if (gRtspProxyConfig.lLogHexDump)
	{
		glog.hexDump(Log::L_INFO, buffer, bufSize, szLogHint);
	}
#endif//_RTSP_PROXY
	string strBuf;
	if (_strSavedMsg.size())
	{
		strBuf=_strSavedMsg;
		strBuf.append((const char*)buffer, bufSize);
		_strSavedMsg = "";
	}	
	
    int bytesNeedtoDecode = strBuf.size()?strBuf.size():bufSize;
	const char* curPos = (const char* )(strBuf.size()?strBuf.data():buffer);

	int dwIndex=0;
	int bytesDecoded = 0;
	int bytesLineSymbols = bytesNeedtoDecode;
    while( bytesDecoded < bytesNeedtoDecode )
    {	
		bool bCompletePkg;
		int nBytesSkipped;	//if there was some invalid \r\n at the begging, for some exception case 
		try 
		{
			bCompletePkg = RtspMsgParser::chopping( curPos,
										   bytesNeedtoDecode,
										   bytesDecoded,nBytesSkipped);
			bytesLineSymbols -= bytesDecoded; //line symbols would not be decoded
			bytesLineSymbols -= nBytesSkipped;// minus line symbols num which store before current.
		}
		catch (...) 
		{
			glog(Log::L_ERROR, CLOGFMT(RtspDialog, "%s RTSP chopping() caught an exception"), szLogHint);
			break;			
		}
		
#if defined(DEBUG)
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspDialog, "%s OnRead() recv_byte[%d], req_count[%d], bytesNeedtoDecode[%d], bytesDecoded[%d], nBytesSkipped[%d]"),
			szLogHint, bufSize, dwIndex, bytesNeedtoDecode, bytesDecoded, nBytesSkipped);
#endif//DEBUG

		if (bCompletePkg)
		{
			RTSPBUFFER rtspBuf;
			ParseProcThrd::getRtspBuf(bytesDecoded, rtspBuf);
			rtspBuf.dataLen = bytesDecoded-nBytesSkipped;
			memcpy(rtspBuf.buf, curPos+nBytesSkipped, bytesDecoded-nBytesSkipped);			
			ParseProcThrd::pushReq(rtspBuf, _conn, dwRecvTime);

			if (nBytesSkipped)
			{
				//warning
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(RtspDialog, "%s onRead() req_index[%d] req_length[%d] bytes_skipped[%d]"),
					szLogHint, dwIndex, rtspBuf.dataLen, nBytesSkipped);
			}

			dwIndex++;
		}		
        else
        {
			_strSavedMsg.assign(curPos, bytesNeedtoDecode);		
			break;
        }

		curPos += bytesDecoded;

		bytesNeedtoDecode -= bytesDecoded;
		bytesDecoded = 0;
    }

	int64 dwSpent = ZQ::common::now() - dwRecvTime;
	if (dwIndex<=0 && bytesLineSymbols >0 && bytesLineSymbols <8)
		dummyPing();

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspDialog, "%s OnRead() recv_byte[%d], req_count[%d], bytesNeedtoDecode[%d] took[%lld]ms"),
		szLogHint, bufSize, dwIndex, bytesNeedtoDecode, dwSpent);
	return true;
}

void RtspDialog::dummyPing()
{
#if !defined(_RTSP_PROXY)
	  return;
#else 
	 if (!gRtspProxyConfig.lDummyPing) 
	 {
		 glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspDialog, "dummyPing() ignore"));
		 return;
	 }

	std::string sessId = _conn->getLastSessionID();
	if( sessId.empty() )
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspDialog, "dummyPing() conn[%s] session[NULL]"), _conn->getConnIdent());
		return;
	}

	static uint i = 0;
	i = (++i) % 10000;
	char buf[64];

	RtspConnectionPtr pConn = _conn;

	RtspRequest* req = new RtspRequest();
	req->setStartLine("GET_PARAMETER * RTSP/1.0");

	RtspClientRequest* cliReq = new RtspClientRequest(req, _conn, SYS::getTickCount() );
	cliReq->setHeader("Session",  (char*)sessId.c_str());
	cliReq->setHeader("CSeq",     itoa(i+ 9990000, buf, 10));
	cliReq->setHeader("x-reason", "PING per dummy message");

	cliReq->setHeader("SYS#ConnID",         (char*)pConn->getConnIdent());
	cliReq->setHeader("SYS#LocalServerIP",  (char*)pConn->getLocalIp());
	cliReq->setHeader("SYS#LocalServerPort",(char*)pConn->getLocalPort());
	ZQ::StreamSmith::StreamSmithSite::getDefaultSite()->postRequest(cliReq, IClientRequest::FixupRequest);	

	glog(ZQ::common::Log::L_INFO, CLOGFMT(RtspDialog, "dummyPing() conn[%s] session[%s] cseq[%d]"), pConn->getConnIdent(), (char*)sessId.c_str(), i+ 9990000);
#endif//_RTSP_PROXY
}

void RtspDialog::onWritten(size_t bufSize)
{

}

void RtspDialog::onError()
{
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL1) {
		glog(Log::L_DEBUG,CLOGFMT(RtspDialog, "onError()"));
	}

#if _TIMEOUT_DISCONNECT
	::std::string sessionID = _conn->getLastSessionID();
	if (sessionID.empty()) {

		glog(Log::L_DEBUG, CLOGFMT(RtspDialog, "onError() conn[%p] close with empty sessId"), _conn);
		_conn->close();

	} else {

		RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
		IClientSession* session = sessionMgr->findSession(sessionID.c_str());
		if (session) {
			session->release();
			return;
		} else {

			glog(Log::L_DEBUG, CLOGFMT(RtspDialog, "onError() conn[%p] after session released"), _conn);
			_conn->close();
		}
	}
#endif
	
}

void RtspDialog::onCommunicatorDestroyed(IDataCommunicatorPtr communicator)
{
	if( !_conn)
		return;

	fireTeardownForConnectionLostRequest();
	//glog(Log::L_INFO, "RtspDialog::onCommunicatorDestroyed()[%llu]",communicator->getCommunicatorId());
	glog(ZQ::common::Log::L_INFO, CLOGFMT(RtspDialog,"onCommunicatorDestroyed() %lld / %s"),
		communicator->getCommunicatorId(),_conn->getLogHint());
	
#pragma message(__MSGLOC__"TODO:connection断掉的时候如何通知Session呢")

	RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
	sessionMgr->onConnectDestroyed(_conn);
	_conn->connectionDown();
	_conn = NULL;
}

void RtspDialog::fireTeardownForConnectionLostRequest()
{
#if !defined(_RTSP_PROXY)
	return;
#else 
	if (!gRtspProxyConfig.lDummyPing) 
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspDialog, "fireTeardownForConnectionLostRequest() conn[%s] is nop destroyed, leave"), _conn->getConnIdent() );	
		return;
	}

	std::string sessId = _conn->getLastSessionID();

	if( sessId.empty() )
		return;

	RtspConnectionPtr pConn = _conn;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(RtspDialog, "fireTeardownForConnectionLostRequest() conn[%s] is destroyed"), pConn->getConnIdent() );	

	RtspRequest* req = new RtspRequest();
	req->setStartLine("TEARDOWN * RTSP/1.0");

	RtspClientRequest* cliReq = new RtspClientRequest(req, _conn, SYS::getTickCount() );
	cliReq->setHeader("Session",(char*)sessId.c_str());
	cliReq->setHeader("CSeq","9999999");
	cliReq->setHeader("x-reason","Teardown session due to connection lost");

	cliReq->setHeader("SYS#ConnID", (char*)pConn->getConnIdent());
	cliReq->setHeader("SYS#LocalServerIP",(char*)pConn->getLocalIp());
	cliReq->setHeader("SYS#LocalServerPort",(char*)pConn->getLocalPort());

	ZQ::StreamSmith::StreamSmithSite::getDefaultSite()->postRequest(cliReq, IClientRequest::FixupRequest);	
#endif   //_RTSP_PROXY
}

//////////////////////////////////////////////////////////////////////////
// class RtspServerRequest

RtspServerRequest::RtspServerRequest(IConnectionInternalPtr conn,const char* clientSessID) :
  _conn(conn)
{
	_strClientSessionID=clientSessID;
	Data svrHdr(_GlobalObject::getServerHdr());
	_rtspRequest.setAddedHdrBodyData("Server", svrHdr);
	_ref = 0;
	reference();
}
  
RtspServerRequest::~RtspServerRequest()
{
	_conn = NULL;
}

	/// output start line of request
int RtspServerRequest::printCmdLine(const char* startline)
{
	_rtspRequest.setStartLine(startline);
	return strlen(startline);
}
	/// output headers of request
int RtspServerRequest::printHeader(char* header, char* value)
{
	if ( header && strlen(header)>0 ) 
	{
		if ( value ) 
		{
			_rtspRequest.setAddedHdrBodyData(header, value);
			return strlen(value);
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}
	/// output message body of request
int RtspServerRequest::printMsgBody(char* msg)
{
	_rtspRequest.setMsgBody(msg);
	return strlen(msg);
}
	/// post the message to a client
int RtspServerRequest::post()
{
	char buf[512];

	IConnectionInternalPtr conn = _conn;

	::std::string res;
	Data theData, theName;
	bool	bFoundDateInHeader = false;

	const Data startLine = _rtspRequest.getStartLine();
	res += startLine.getData();
	res += CRLF;
	u_int32_t count = _rtspRequest.getAddedHdrCount();
	for (u_int32_t i = 0; i < count ; i ++)
	{		
		_rtspRequest.getAddedDataByIndex(i, NULL, theName, theData);
		if (theData.length() == 0) 
		{//discard the field
			continue;
		}
		if (!bFoundDateInHeader)
		{
#ifdef ZQ_OS_MSWIN
			if (stricmp(theName.getData (),"Date") == 0) 
#else
			if (strcasecmp(theName.getData (),"Date") == 0) 
#endif
			{
				bFoundDateInHeader = true;
			}
		}
		res += theName.getData();
		res += ": ";
		res += theData.getData();
		res += CRLF;
	}
	if (!bFoundDateInHeader) 
	{
		res += "Date";
		res += ": ";
		char szBuf[512];
		long	tzBias = 0;
		//get gmt time
#ifdef _RTSP_PROXY
		if ( gRtspProxyConfig.lEnableUseLocaltimeInDatHeader >= 1) 
		{
		
			tzBias = gRtspProxyConfig.lCurTimeZone;
				 
#else
		if(gStreamSmithConfig.lEnableUseLocaltimeInDatHeader >= 1)
		{	
			tzBias = gStreamSmithConfig.lCurTimeZone;
#endif
#ifdef ZQ_OS_MSWIN
			SYSTEMTIME st;
			GetLocalTime(&st);
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03d GMT%s%02d",
				st.wDay,
				MonthIndexToString[st.wMonth-1],
				st.wYear,
				st.wHour,
				st.wMinute,
				st.wSecond,
				st.wMilliseconds,
				tzBias >= 0 ?"+":"-",
				tzBias);

		}
		else
		{
			SYSTEMTIME st;
			GetSystemTime (&st);
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03d GMT",
				st.wDay,
				MonthIndexToString[st.wMonth-1],
				st.wYear,
				st.wHour,
				st.wMinute,
				st.wSecond,
				st.wMilliseconds);
		}
#else
			struct timeval tval;
			gettimeofday(&tval,(struct timezone*)NULL);
			struct tm* ptm = NULL;
		   	ptm = localtime(&tval.tv_sec);
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03ld GMT%s%02ld",
				ptm->tm_mday,
				MonthIndexToString[ptm->tm_mon],
				ptm->tm_year,
				ptm->tm_hour,
				ptm->tm_min,
				ptm->tm_sec,
				tval.tv_usec/1000,
				tzBias >= 0 ?"+":"-",
				tzBias);
		}
		else
		{
			struct timeval tval;
			gettimeofday(&tval,(struct timezone*)NULL);
			struct tm* ptm = NULL;
		   	ptm = gmtime(&tval.tv_sec);
			
			szBuf[sizeof(szBuf)-1]='\0';		
			snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03ld GMT",
				ptm->tm_mday,
				MonthIndexToString[ptm->tm_mon],
				ptm->tm_year+1900,
				ptm->tm_hour,
				ptm->tm_min,
				ptm->tm_sec,
				tval.tv_usec/1000);
		}
#endif

		res += szBuf;
		res += CRLF;
	}
	
	theData = _rtspRequest.getMsgBody();
	if (theData.length()) {
		res += "Content-Length";
		res += ": ";
		sprintf(buf, "%d", theData.length());
		res += buf;
		res += CRLF;
		res += CRLF;
		res += theData.getData();
	}
	else
		res += CRLF;

	u_int32_t len = res.length();
	if (conn->isActive()) {
		if (conn->send((uint8* )res.c_str(), len) <= 0)
			glog(Log::L_INFO, CLOGFMT(RtspServerRequest, "post() failed to send data through COMM[%llu]"), conn->getConnectionIdentity() );
	} else {
		glog(Log::L_DEBUG, CLOGFMT(RtspServerRequest, "post() COMM[%llu] is closed"), conn->getConnectionIdentity() );
	}

	char hint[0x200];
//	SOCKET socket = (SOCKET )conn->getSocket();
	sprintf(hint, "sreq SOCKET: "FMT64" ", conn->getConnectionIdentity());

	char ipaddr[0x20];
	uint16 port;
	if (conn->getRemoteIP(ipaddr, sizeof(ipaddr), &port) == NULL) {

		glog(Log::L_ERROR, CLOGFMT(RtspServerRequest, "post() getRemoteIP() failed"));
		return 0;
	}

	Data DataSessionID,DataSeq;
	//_rtspRequest
	DataSessionID =  _rtspRequest.getAddedHdrBodyData("Session");
	DataSeq  = _rtspRequest.getAddedHdrBodyData("CSeq");
	
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RtspServerRequest, "out socket: %010lld %s:%hu [session:%s] [seq:%s]"),
				conn->getConnectionIdentity(), ipaddr,port,DataSessionID.getData(),DataSeq.getData());

	sprintf(hint, "SOCKET: %s:%hu", ipaddr, port);
	glog.hexDump(Log::L_INFO, res.c_str(), len, hint, true);

	_GlobalObject::responseCounters.addCount(std::string("ANNOUNCE"), NULL);

	return len;
}

  /// release itself
void RtspServerRequest::release()
{
	long r = 0;
	{
		ZQ::common::MutexGuard gd(_refLock);
		r = --_ref;
	}
	if(r==0)
	{
		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) 
		{
			glog(Log::L_DEBUG, CLOGFMT(RtspServerRequest, "release() this(%p), ref = %d, should be deleted"), 
				this, r);
		}
		delete this;
	}
	else
	{
		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) 
		{
			glog(Log::L_DEBUG, CLOGFMT(RtspServerRequest, "release():his(%p), ref = %d"), this, r);
		}
	}	
}
void RtspServerRequest::reference()
{
	long r = 0;
	{
		ZQ::common::MutexGuard gd(_refLock);
		r = ++_ref;
	}
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) {
		glog(Log::L_DEBUG, CLOGFMT(RtspServerRequest, "reference() this(%p), ref =%d"), 
			this, r);
	}
}

int RtspServerRequest::closeConnection()
{
	return _conn->close();
}

bool StreamSmithUtilityImpl::getVerbString(RTSP_VerbCode verb, 
										   char buf[], uint32 size)
{
	if (verb < 0 || verb >= RTSP_MTHD_UNKNOWN)
		return false;

	const CharData& r = RtspUtil::getMethodInString(verb);
	
	strncpy(buf, r.getPtr(), size);
	return true;
	
}

/*
bool StreamSmithUtilityImpl::getHeaderString(RTSP_HEADER header, 
											 char buf[], uint32 size)
{
	if (header < 0 || header >= RTSP_HDR_UNKNOWN)
		return false;

	const CharData& r = RtspUtil::getHeaderInString(header);
	
	strncpy(buf, r.getPtr(), size);
	return true;
}
*/

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE
