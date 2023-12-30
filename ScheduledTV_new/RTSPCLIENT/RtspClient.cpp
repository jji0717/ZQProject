// RtspClient.cpp: implementation of the RtspClient class.
//
//////////////////////////////////////////////////////////////////////

#include "RtspClient.h"
#include "RtspDaemon.h"
#include "../STVMainHeaders.h"
#include "../MainCtrl/ScheduleTV.h"
#include "Log.h"

extern ScheduleTV gSTV;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RtspClient::RtspClient(RtspDaemon& daemon)
	:_daemon(daemon)
{
	_host		= "";
	_port		= RTSP_DEFAULT_PORT;
	_nsec		= RTSP_DEFAULT_NSEC;

	_sd			= INVALID_SOCKET;
	_status		= CLIENT_DISCONNECT;
	_envMask	= ENVMASK_EXCP | ENVMASK_SEND;
	_isProcessing = false;
	_sendingORrecving = 0;
	_traceFlag	= false;
	
	_szSess		= "";
	_szSeq		= "0";
	_dwHb		= RTSP_DEFAULT_HB;
	_dwPurId	= 0;
	_dwHomeId	= LISTTYPE_UNKNOWN;
	
	_hProCmpl	= ::CreateEvent(NULL, false, false, NULL);

}

RtspClient::~RtspClient()
{
	if(_status!=CLIENT_DISCONNECT)
		close();
	
	_sd			= INVALID_SOCKET;
	_status		= CLIENT_DISCONNECT;
	_isProcessing = false;
	::CloseHandle(_hProCmpl);
}

//////////////////////////////////////////////////////////////////////////
// connection operations
//////////////////////////////////////////////////////////////////////////

bool RtspClient::open(const char* host, int port/* =RTSP_DEFAULT_PORT */, DWORD nsec/* =RTSP_DEFAULT_NSEC */)
{
	if(_status!= CLIENT_DISCONNECT)
	{
		glog(ZQ::common::Log::L_ERROR, "[%06d]RtspClient::open()  Connection already opened", _dwPurId);
		return false;
	}

	_host = host;
	_port = port;
	_nsec = nsec;

	// create socket
	_sd = RtspSocket::socket();
	if((int)_sd<=0)
	{
		glog(ZQ::common::Log::L_ERROR, "[%06d]RtspClient::open()  Could not create socket (%d)", _dwPurId, RtspSocket::getError(_sd));
		_sd = INVALID_SOCKET;
		return false;
	}
	
	trace("+------------------------------------------------------------------+");
	trace("Socket %d created", _sd);
	trace("+------------------------------------------------------------------+");
	
	// set socket to non-blocking
	if(!RtspSocket::setNonBlocking(_sd))
	{
		glog(ZQ::common::Log::L_ERROR, "[%06d]RtspClient::open()  Could not set socket to non-blocking IO mode (%d)", _dwPurId, RtspSocket::getError(_sd));
		close();
		return false;
	}

	trace("+------------------------------------------------------------------+");
	trace("Socket %d set to non-blocking mode", _sd);
	trace("+------------------------------------------------------------------+");
	
	// connect to server, this may cause an error of WSAWOULDBLOCK (10035).  It's normal for non-blocking socket.
	if(!RtspSocket::connect(_sd, _host, _port))
	{
		glog(ZQ::common::Log::L_ERROR, "[%06d]RtspClient::open()  Could not connect to %s:%d server (%d)", _dwPurId, _host.c_str(), _port, RtspSocket::getError(_sd));
		close();
		return false;
	}

	_status = CLIENT_CONNECTED;
	
	return true;
}

bool	RtspClient::close()
{
	if(_isProcessing)
	{
		::SetEvent(_hProCmpl);
	}
	
	if((int)_sd<=0)
	{
		return true;
	}

	trace("+------------------------------------------------------------------+");
	trace("Socket %d closed", _sd);
	trace("+------------------------------------------------------------------+");

	_status = CLIENT_DISCONNECT;
	RtspSocket::close(_sd);
	_sd = INVALID_SOCKET;
	
	_sendingORrecving = 0;
	_szSess		= "";
	_szSeq		= "0";
	_msgRequest.clearMessage();
	_msgResponse.clearMessage();
	
	return true;
}

bool	RtspClient::reset()
{
	// notify daemon to ignore this socket
	_daemon.updateEnv(this, ENVMASK_NONE, false);

	close();

	bool ret = open(_host.c_str(), _port, _nsec);

	return ret;
}

//////////////////////////////////////////////////////////////////////////
// client operations
//////////////////////////////////////////////////////////////////////////

bool	RtspClient::sendMsg(const RtspRequest& msgin, RtspResponse& msgout)
{
	bool bRet = true;
	msgout.clearMessage();

	// check if someone else is already sending a message
	if(_isProcessing)
	{
		glog(ZQ::common::Log::L_WARNING, "[%06d]RtspClient::sendMsg()  Could not send request, there is already a request processing", _dwPurId);
		return false;
	}

	// begin to lock critical operations, such as OnException()
	ZQ::common::MutexGuard	tmpGd(_lkWaitBlock);

	if(!OnRequest(msgin))
	{
		glog(ZQ::common::Log::L_WARNING, "[%06d]RtspClient::sendMsg()  Could  not prepare request", _dwPurId);
		return false;
	}

	_isProcessing = true;	// set process flag
	_sendingORrecving = 1;	// set sending flag

	std::string msgType = const_cast<RtspRequest&>(msgin).getCommandtype();	// get request type

	// notify daemon to wakeup
	_daemon.updateEnv(this, ENVMASK_SEND|ENVMASK_EXCP, true);

	//////////////////////////////////////////////////////////////////////////
	
	int code = ::WaitForSingleObject(_hProCmpl, _nsec);
	if(code == WAIT_TIMEOUT)
	{
		glog(ZQ::common::Log::L_WARNING, "[%06d]RtspClient::sendMsg()  Could not get response within %d milli-seconds", _dwPurId, _nsec);
		bRet = false;
	}
	else if(code == WAIT_OBJECT_0)
	{
		bRet = true;
	}
	else
	{
		int errorcode = GetLastError();
		glog(ZQ::common::Log::L_ERROR, "[%06d]RtspClient::sendMsg()  ::WaitForSingleObject() error %d", _dwPurId, errorcode);
		bRet = false;
	}

	// stop waiting and return
	if(!bRet)
	{
		_isProcessing = false;
		_sendingORrecving = 0;
		return bRet;
	}

	//////////////////////////////////////////////////////////////////////////

	// If no response got, maybe it is because the request is 'TEARDOWN'.
	// In this case, it is normal to have no response since the socket is closed by server.
	if(_msgResponse.isEmpty())
	{
		_isProcessing = false;
		_sendingORrecving = 0;
		if(msgType == "TEARDOWN")
		{
			return true;
		}
		else
		{
			glog(ZQ::common::Log::L_WARNING, "[%06d]RtspClient::sendMsg()  response is empty", _dwPurId);
			return false;
		}
	}

	// parse response
	msgout = _msgResponse;
	std::string msgStatus = msgout.getCommandstatus();
	
	if(msgStatus != "200")
	{
		_isProcessing = false;
		_sendingORrecving = 0;
		glog(ZQ::common::Log::L_DEBUG, "[%06d]RtspClient::sendMsg()  response not successful, code=%s", _dwPurId, msgStatus.c_str());
		return false;
	}
		
	bRet = true;
	if(msgType == "SETUP")
	{
		// get session_id and time-out
		std::string sessionstring = "";
		std::string tmpsession	= "";

		sessionstring= msgout.getHeaderField(KEY_SESSION);
		size_t	semicolon = sessionstring.find_first_of(';');

		if(semicolon != std::string.npos) 
		{	// with timeout
			tmpsession = sessionstring.substr(0,semicolon);
			RtspMsgHeader timeoutH;
			RtspMsgHeader::parseNewHeader(KEY_SESSION,sessionstring.c_str(), timeoutH);
			_dwHb = atol( timeoutH.getSubHeaderField(KEY_TIMEOUT).c_str());
			_daemon.updateHb(_dwHb);
		}
		else
		{// without timeout
			tmpsession = sessionstring;
		}

		_szSess=tmpsession;
		_msgRequest.setHeaderField(KEY_SESSION, _szSess.c_str());

		_status = CLIENT_READY;
		
	}	// if(msgType == "SETUP")
	else if(msgType == "PLAY")
	{
		_status = CLIENT_PLAYING;
	}
	else if(msgType == "PAUSE")
	{
		_status = CLIENT_SUSPEND;
	}

	_isProcessing = false;
	_sendingORrecving = 0;
	return bRet;
}

std::string RtspClient::getMulticastMac(const char* ipaddr)
{
	std::string dynamicMac_str="";
	std::string IPAddress = ipaddr;
	std::string dynamicMacHead = "01005E";
	int dynamicMac_int = 0x00000000;
	int ipZone[4];

	// separate ip address by dot
	for(int i=0;i<3;i++) {
		size_t dot = IPAddress.find_first_of('.');
		if(dot==std::string::npos)
		{
			return dynamicMac_str;
		}
		ipZone[i] = atoi( IPAddress.substr(0,dot).c_str());
		IPAddress = IPAddress.substr(dot+1,IPAddress.size()-dot-1);
	}
	ipZone[3] = atoi( IPAddress.c_str());

	dynamicMac_int = ipZone[0];
	dynamicMac_int = dynamicMac_int << 8;
	dynamicMac_int = dynamicMac_int + ipZone[1];
	dynamicMac_int = dynamicMac_int << 8;
	dynamicMac_int = dynamicMac_int + ipZone[2];
	dynamicMac_int = dynamicMac_int << 8;
	dynamicMac_int = dynamicMac_int + ipZone[3];

	dynamicMac_int = dynamicMac_int & (0x00FFFFFF >> 1);

	char hexstr[32];
	sprintf(hexstr,"%x",dynamicMac_int);
	dynamicMac_str = hexstr;
	dynamicMac_str = "000000" + dynamicMac_str;
	dynamicMac_str = dynamicMac_str.substr(dynamicMac_str.size()-6,6);
	dynamicMac_str = dynamicMacHead + dynamicMac_str;

	char macstr[32];
	strncpy(macstr, dynamicMac_str.c_str(), 32);
	strupr(macstr);
	dynamicMac_str = macstr;

	return dynamicMac_str;
}

//////////////////////////////////////////////////////////////////////////
// message callbacks
//////////////////////////////////////////////////////////////////////////

bool	RtspClient::OnRequest(const RtspRequest& req)
{
	// backup sent message
	_msgRequest = req;
	
	// fill "CSeq" and "Session" fields if necessary
	if(_msgRequest.getHeaderField(KEY_CSEQ)=="")
		_msgRequest.setHeaderField(KEY_CSEQ, _szSeq.c_str());

	incSeq();
	
	if(_status>=CLIENT_READY && _msgRequest.getHeaderField(KEY_SESSION)=="" && !_szSess.empty())
		_msgRequest.setHeaderField(KEY_SESSION, _szSess.c_str());

	// clear waiting message
	_msgResponse.clearMessage();
	
	return	true;
}

bool	RtspClient::OnResponse(const RtspResponse& res)
{
	_msgResponse = res;	// store into response
	::SetEvent(_hProCmpl);	// notify sendMsg() - got response
	return true;
}

bool	RtspClient::OnAnnounce(const RtspRequest& ann)
{
	RtspRequest AnnReq = ann;
	std::string NoticeStr = AnnReq.getHeaderField(KEY_SEACHANGENOTICE);
	
	// skip white space
	while(NoticeStr[0]==' ')
		NoticeStr = NoticeStr.substr(1, NoticeStr.length()-1);
	
	// get code
	NoticeStr = NoticeStr.substr(0,4);
	int NoticeCode = atoi(NoticeStr.c_str());

	// update channel status
	switch(NoticeCode) {
		case 2101:	// "End-of-Stream Reached"
		case 2104:	// "Start-of-stream Reached"
		case 5200:	// "Server Resources Unavailable"
		case 5502:	// "Internal Server Error"
		case 5403:	// "Server Shutting Down"
			{
				// send missing status feedback if necessary
				SSAENotification missingNoti;
				STVChannel* pCh = NULL;

				pCh = gSTV.getPlayListMan()->queryCh(_dwPurId);
				if(pCh == NULL)
				{
					GTRACEERR;
					glog(ZQ::common::Log::L_ERROR, "[%06d]RtspClient::OnAnnounce()  Could not find channel %ld", _dwPurId, _dwPurId);
					return false;
				}

				if(pCh->getLastFeedback(missingNoti))
				{
					if(missingNoti.wOperation== SAENO_PLAY)
					{	// last operation is start, with no stop, so send stop operation notification
						missingNoti.wOperation = SAENO_STOP;

						gSTV.OnSendAEStatus(_dwPurId, missingNoti);
					}
				}

				STVChannel::Stat currStat = pCh->getStatus();
				if(currStat==STVChannel::STAT_PLAYING || currStat==STVChannel::STAT_SUSPENDING)
				{
					pCh->setStatus(STVChannel::STAT_IDLE);
					pCh->clearStubs();
				}

			}
			
			break;
		default:
			break;
	}

	
	return true;
}

bool	RtspClient::OnException()
{
	// send missing status feedback if necessary
	SSAENotification missingNoti;
	STVChannel* pCh = NULL;
	
	pCh = gSTV.getPlayListMan()->queryCh(_dwPurId);
	if(pCh == NULL)
	{
		GTRACEERR;
		glog(ZQ::common::Log::L_ERROR, "[%06d]RtspClient::OnException()  Could not find channel %ld", _dwPurId, _dwPurId);
		return false;
	}
	
	if(pCh->getLastFeedback(missingNoti))
	{
		if(missingNoti.wOperation== SAENO_PLAY)
		{	// last operation is start, with no stop, so send stop operation notification
			missingNoti.wOperation = SAENO_STOP;

			gSTV.OnSendAEStatus(_dwPurId, missingNoti);
		}
	}

	if(/*_isProcessing &&*/ _msgRequest.getCommandtype()=="TEARDOWN")	
	{
		// Don't care about _isProcessing! Maybe we get this exception after "TEARDOWN" receives "200 OK"
		
		// the exception is caused by TEARDOWN, it is normal.
		// we will handle channel status and stubs somewhere else.
	}
	else
	{
		// client connection lost, reset channel status
		pCh->setStatus(STVChannel::STAT_IDLE);
		pCh->clearStubs();
	}	
	
	// close connection
	close();

	return true;
}
//////////////////////////////////////////////////////////////////////////
// message handling functions
//////////////////////////////////////////////////////////////////////////

int		RtspClient::handleExcp(int errcode/* =0 */)
{
	int err = 0;

	if(errcode==0)	// if errcode is not provided, go get it
	{
		err = RtspSocket::getError(_sd);
	}
	else
	{
		err = errcode;
	}
	
	trace("+------------------------------------------------------------------+");
	trace("Socket %d got exception %d", _sd, err);
	trace("+------------------------------------------------------------------+");

	if(_isProcessing && RtspSocket::nonFatalError(err))
	{
		if(_sendingORrecving==1)	// is waiting to send
			return ENVMASK_SEND | ENVMASK_EXCP;
		else if(_sendingORrecving==2)	// is waiting to recv
			return ENVMASK_RECV | ENVMASK_EXCP;
		else	// must be something wrong, do nothing?
			return ENVMASK_RECV | ENVMASK_EXCP;
	}
	else if(!_isProcessing && RtspSocket::nonFatalError(err))
	{
		return ENVMASK_RECV | ENVMASK_EXCP;	// that's ok, non-fatal error
	}
	else
	{
		OnException();	// here is really exception caught
		return ENVMASK_NONE;
	}
}

int		RtspClient::handleSend()
{
	if(!_isProcessing)	// no data to send
	{
		glog(ZQ::common::Log::L_DEBUG, "RtspClient::handleSend()  no data to send");
		return ENVMASK_RECV | ENVMASK_EXCP;
	}

	int bytes = 0, totalbytes = _msgRequest.toString().length();
	int ret=0;
	while(ret!= -1 && (bytes<totalbytes))
	{
		ret = RtspSocket::nbWrite(_sd, _msgRequest.toString(), &bytes);
	}

	if(ret==-1)
	{
		::SetEvent(_hProCmpl);	// notify sendMsg() - sending fail
		trace("Socket %d failed to send message: ", _sd);
		return ENVMASK_NONE;
	}
	else
	{
		trace("+------------------------------------------------------------------+");
		trace("Socket %d sent message: ", _sd);
		trace("+------------------------------------------------------------------+");
		trace("%s", _msgRequest.toString().c_str());
		trace("+------------------------------------------------------------------+");
	}
	_sendingORrecving = 2;
	return ENVMASK_RECV | ENVMASK_EXCP;
}

int		RtspClient::handleRecv()
{
	std::string instr = "";
	bool eof = false;
	int  errcode = 0;
	int ret=0;
	while(ret!=-1 && !eof)
	{
		ret = RtspSocket::nbRead(_sd, instr, &eof, errcode);
	}
	
	int bytessofar=0;
	if(ret==-1)
	{
		// something error happened on socket, usually WSAECONNRESET
		trace("Socket %d received -1 bytes data, error code: %d", _sd, errcode);
		return handleExcp(errcode);
	}
	else if(ret==0)
	{
		// If select() says socket has something, but recv() returns 0
		// and WSAGetLastError() also returns 0, this socket must have
		// already been dropped.
		// So we manually set the error code to "10054"
		return handleExcp(WSAECONNRESET);
	}
	else
	{
		trace("+------------------------------------------------------------------+");
		trace("Socket %d received message: ", _sd);
		trace("+------------------------------------------------------------------+");
		trace("%s", instr.c_str());
		trace("+------------------------------------------------------------------+");

		// in case there are more than 1 message got, parse each of them
		int msgnum=1;
		RtspMessage nextMsg;
		do {
			trace("parse message no.%d from socket %d", msgnum++, _sd);
			RtspMessage::parseNewMessage(instr.c_str(), nextMsg, &bytessofar);
			if(nextMsg.isEmpty())
				break;
			
			switch(nextMsg.isRequOrResp()) 
			{
			case RTSP_REQUEST_MSG:
				OnAnnounce(nextMsg);
				break;
			case RTSP_RESPONSE_MSG:
				if(_isProcessing)	
				{
					OnResponse(nextMsg);
				}
				else
				{	// not waiting for response anymore
				}
				break;
			default:
				break;
			}

		} while(bytessofar<ret-2);

	}
	
	return ENVMASK_RECV | ENVMASK_EXCP;
}

int		RtspClient::handleAlive()
{
	if(_isProcessing)	// is waiting for data
		return ENVMASK_RECV | ENVMASK_EXCP;

	int bytes = 0;
	int ret = 0;
	std::string	hbstr = RTSP_HB_MSG;	// this is the heartbeat msg, "\r\n" (CRLF)
	
	ret = RtspSocket::nbWrite(_sd, hbstr, &bytes);
	
	if(ret==-1)
	{
		trace("Socket %d failed to send heartbeat message", _sd);
	}
	else
	{
//		trace("Socket %d sent heartbeat message", _sd);
	}
				
	return ENVMASK_RECV | ENVMASK_EXCP;
}

//////////////////////////////////////////////////////////////////////////
// internal functions
//////////////////////////////////////////////////////////////////////////

void	RtspClient::trace(const char *fmt, ...)
{
	if(!_traceFlag)
	{
		return;	// trace flag not set, so do not log
	}

	char msg[2048]={0};
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	char* line = msg;
	
	do 
	{
		while((*line==CH_CR) || (*line==CH_LF))
			line++;

		char buff[512]={0};
		char* pos = strchr(line, '\r');
		if(pos == NULL)
		{
			strncpy(buff, line, 511) ;
		}
		else
		{
			strncpy(buff, line, pos-line);
		}
		
		glog(ZQ::common::Log::L_DEBUG, "(TRACE) - [%06d]  %s", _dwPurId, buff);
	} 
	while( line = strchr(line, '\r') );
	
}

