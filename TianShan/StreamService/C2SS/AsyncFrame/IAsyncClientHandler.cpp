#include "IAsyncClientHandler.hpp"
#include "IMgr.hpp"

#ifndef ULONG_MAX
#define ULONG_MAX     0xffffffffUL  /* maximum unsigned long value */
#endif

//--------------start HttpServer------------------
HttpServer::HttpServer(IHttpProc::Ptr proc)
    :_proc(proc)
{
	setState(Idle, Idle);
}

HttpServer::~HttpServer()
{
	//_proc = 0;
};

CommonTcpCommuncator::Ptr  HttpServer::getConn(void) const
{
	ZQ::common::MutexGuard guard(_serverMutex);
	return _ownConn;
}

int  HttpServer::attachConn(CommonTcpCommuncator::Ptr conn)
{
	ZQ::common::MutexGuard guard(_serverMutex);
	_ownConn = conn;
	return true;
}


int HttpServer::shutDown(void)
{
	//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpClient, "Httpclient manual shutDown(), dst[%s] or dstPort[%d] errCode[%d]"), _dstTo.c_str(), _dstPort, _errCode);

	CommonTcpCommuncator::Ptr conn = _ownConn;

	_ownConn = NULL;
	setErrorCode(HTTP_MANUAL_SHUTWODN);
	_proc = NULL;
	if(!conn->unInit())
	{
		//_log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpClient, "Httpclient manual shutDown(), conn unInit failed, dst[%s] or dstPort[%d] errCode[%d]"), _dstTo.c_str(), _dstPort, _errCode);
		return false;
	}

	return true;
}

int HttpServer::attachProc(IHttpProc::Ptr proc)
{
	ZQ::common::MutexGuard guard(_serverMutex);
	if (_proc)
		return false;

	_proc   = proc;
	return true;
}

int  HttpServer::strncasecmp(const char *s1, char *s2, size_t n)
{
	unsigned char  c1, c2;

	while (n) 
	{
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
		c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

		if (c1 == c2) 
		{
			if (c1) 
			{
				n--;
				continue;
			}

			return 0;
		}

		return c1 - c2;
	}

	return 0;
}

int  HttpServer::getHttpVersion(std::string& version) const
{
	ZQ::common::MutexGuard guard(_serverMutex);
	version = _httpReqVersion;
	return true;
}
int  HttpServer::getURI(std::string& uri) const 
{
	ZQ::common::MutexGuard guard(_serverMutex);
	uri = _httpReqURI;
	return true;
}
int  HttpServer::getMethod(std::string& method) const 
{
	ZQ::common::MutexGuard guard(_serverMutex);
	method = _httpReqMethod;
	return true;
}
unsigned long HttpServer::getContentLength(void) const
{
	ZQ::common::MutexGuard guard(_serverMutex);
	return _contentLength;
}
unsigned long HttpServer::getReqContentSize(void) const
{
	return _httpReqBodyLength;//would increase
}
int  HttpServer::getReqHeader(HttpServer::HttpHeadMap& httpHead) const
{
	ZQ::common::MutexGuard guard(_serverMutex);
	httpHead = _httpReqHeader;
	return true;
}
int  HttpServer::getMessageBody(std::string& messageBody) const
{
	ZQ::common::MutexGuard guard(_serverMutex);
	messageBody = _httpReqMessageBody;
	return true;
}

int  HttpServer::setState(HttpServerState current, HttpServerState previous)
{
	_currentState  = current;
	_previousState = previous;
	if (Idle == current)
		this->setErrorCode(HTTP_OK);//reset to default
	else if (ERROR_QUIT == current)
	{
		switch(previous)
		{
		default:
			this->setErrorCode(HTTP_ERR);
			break;
		case RequestLine:
			this->setErrorCode(HTTP_TYPE_ERR);
			break;
		case RequestHeader:
			this->setErrorCode(HTTP_HDR_ERR);
			break;
		case MessageBody:
			this->setErrorCode(HTTP_MESSAGE_ERR);
			break;
		case RequestData_ChunkSize:
			this->setErrorCode(HTTP_CHUNK_BODY_ERR);
			break;
		case RequestData_ChunkBody:
			this->setErrorCode(HTTP_CHUNK_SIZE_ERR);
			break;
		case RequestData_Trailer:			
			this->setErrorCode(HTTP_CHUNK_TRAILER_ERR);
			break;
		}
	}

	return true;
}

int  HttpServer::httpParseHeader(const char *key, const char *val)
{
	if(!key || !val)//add to header map
		return false;

	int supportHeader = false;
	if (!strncasecmp(key, "Host", 4))
	{ 
		supportHeader = true;
	}
#ifndef WITH_LEANER
	else if (!strncasecmp(key, "Content-Type", 12))
	{ 
		supportHeader = true;
	}
#endif
	else if (!strncasecmp(key, "Content-Length", 14))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Content-Encoding", 16))
	{ 
		if (!strncasecmp(val, "deflate", 7))
			supportHeader = true;
		else if (!strncasecmp(val, "gzip", 4))
			supportHeader = true;
	}
#ifdef WITH_ZLIB
	else if (!strncasecmp(key, "Accept-Encoding", 15))
	{
		supportHeader = true;
	}
#endif
	else if (!strncasecmp(key, "Transfer-Encoding", 17))
	{ 
		if (!strncasecmp(val, "chunked", 7))
		{
			supportHeader = true;
		}
	}
	else if (!strncasecmp(key, "Connection", 10))
	{ 
		if (!strncasecmp(val, "keep-alive", 10))
		{
			supportHeader = true;
		}
		else if (!strncasecmp(val, "close", 5))
		{
			supportHeader = true;
		}
	}
#ifndef WITH_LEAN
	else if (!strncasecmp(key, "Authorization", 13))
	{ 
		if (!strncasecmp(val, "Basic *", 7))
		{ 
			supportHeader = true;
		}
	}
	else if (!strncasecmp(key, "WWW-Authenticate", 16))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Expect", 6))
	{ 
		if (!strncasecmp(val, "100-continue", 12))
		{ 
			supportHeader = true;
		}
	}
#endif
	else if (!strncasecmp(key, "Location", 8))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "X-Forwarded-For", 15))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Accept-Ranges", 13))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Cache-Control", 13))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Server", 6))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Date", 4))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "TianShan-Notice", 15))
	{ 
		supportHeader = true;
	}

	if (supportHeader)
		_httpReqHeader[key] = val;

	return supportHeader;
}

int  HttpServer::regAsyncSendEvent(void)
{
	_ownConn->doSendAsync(NULL);
	return true;
}

int  HttpServer::regAsyncRecvEvent(void)
{
	_ownConn->doRecvAsync(NULL);
	return true;
}

int HttpServer::syncSendRaw(char* raw, unsigned long rawLen)
{
	int size = 0;
	if(NULL != raw)
		size = _ownConn->doSendSync(raw, rawLen);

	return size;
}

int HttpServer::onRecvEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	ZQ::common::MutexGuard guard(_serverMutex);
	if (!_ownConn)
		return false;

	_request += buffer;
	while (!_request.empty() || FinalAssemble <= _currentState)
	{
		switch(_currentState)
		{
		case Idle:
			{ // search for the request line
				this->setState(RequestLine, _currentState);
				_httpReqBodyLength = 0;
				_httpReqMessageBody.clear();
				_httpReqHeader.clear();
				_httpReqURI.clear();
				_httpReqMethod.clear();
				_httpReqVersion.clear();
			}//fall
		case RequestLine:
			{// Method SP Request-URL SP HTTP-Version CRLF 
				const char* buf  = _request.c_str();
				size_t method    = _request.find_first_not_of(" \r\n");
				if(std::string::npos == method
					|| _request.size() < method + 13 )// 13 = min(requestLine)
				{
					_request.clear();
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}
                //1.1 parser method
				if (0 == strncasecmp(buf + method, "POST", 4))
					_httpReqMethod = "POST";
				else if (0 == strncasecmp(buf + method, "GET", 3))
					_httpReqMethod = "GET";
				else 
				{
					_request      = _request.substr(method + 1);
					this->setState(ERROR_QUIT, _currentState);// not support others method
					continue;
				}

				size_t requestLineEnd  = _request.find_first_of("\r\n", method + 1);
				size_t methodEnd      = _request.find_first_of("  ", method + 3);//3 = sizeof(min(method))
				if (std::string::npos == methodEnd || std::string::npos == requestLineEnd)
				{//data not recv completely
					this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
					continue;
				}

				if (methodEnd > requestLineEnd)
				{
					_request      = _request.substr(requestLineEnd + 1);
					this->setState(ERROR_QUIT, _currentState);//request line error
					continue;
				}
                //1.2 parser request URI
				size_t requestURI = _request.find_first_not_of("  ", methodEnd);
				if (std::string::npos == requestURI)
				{//not find request URI, and it is less than requestLineEnd
					_request      = _request.substr(requestLineEnd + 1);
					this->setState(ERROR_QUIT, _currentState);//request line error
					continue;
				}

				size_t requestURIend = _request.find_first_of("  ", requestURI);
				if (std::string::npos == requestURIend || requestURIend > requestLineEnd)
				{//not find request URI end mark
					_request      = _request.substr(requestLineEnd + 1);
					this->setState(ERROR_QUIT, _currentState);//request line error
					continue;
				}
				_httpReqURI = _request.substr(requestURI, requestURIend - requestURI);

                //1.3 parser http version
				if (0 == strncasecmp(buf + requestURIend + 1, "HTTP/1.1", 8))//1 = sizeof(SP)
					_httpReqVersion = "HTTP/1.1";
				else if (0 == strncasecmp(buf + requestURIend + 1, "HTTP/1.0", 8))
					_httpReqVersion = "HTTP/1.0";
				else 
				{
					_request      = _request.substr(requestLineEnd);
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}

				_request = _request.substr(requestLineEnd);
				this->setState(RequestHeader, _currentState);
			}//fall
		case RequestHeader:
			{//message-header = field-name ":"[ field-value ]
				size_t headerStart     = _request.find_first_not_of("  \r\n");
				size_t messageBody     = _request.find("\r\n\r\n");
				if (std::string::npos == messageBody)
				{//currently message body has not been recved
					if(std::string::npos != headerStart)
						_request = _request.substr(headerStart);

					size_t fieldName     = _request.find_first_of(":");
					size_t fieldValueEnd = _request.find_first_of("\r\n");
					if (std::string::npos == fieldName || std::string::npos == fieldValueEnd)
					{//data not recv completely
						this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
						continue;
					}

					std::string fieldNameStr  = _request.substr(0, fieldName);
					std::string fieldValueStr = _request.substr(fieldName + 1, fieldValueEnd - fieldName - 2);
					if (this->httpParseHeader(fieldNameStr.c_str(), fieldValueStr.c_str()))
					{
						_request = _request.substr(fieldValueEnd + 1);
					}
				}
				else
				{//message body has been recved
					if(std::string::npos == messageBody && std::string::npos == headerStart )
					{
						this->setState(ERROR_QUIT, _currentState);
						continue;//error, it must to get \r\n
					}

					if(headerStart >= messageBody)
					{
						this->setState(MessageBody, _currentState);
						continue;
					}

					_request          = _request.substr(headerStart);
					size_t fieldName  = _request.find_first_of(":");
					if (std::string::npos == fieldName || fieldName >= messageBody)
					{
						this->setState(ERROR_QUIT, _currentState);
						continue;//ERROR
					}
					size_t fieldValueStart       = _request.find_first_not_of("  :", fieldName);
					size_t fieldValueEnd         = _request.find_first_of("\r\n", fieldName);
					size_t fieldNameStart        = _request.find_first_not_of("  \t\r\n");
					size_t fieldNameEnd          = _request.find_first_of("  :", fieldNameStart);
					std::string fieldNameStr  = _request.substr(fieldNameStart, fieldNameEnd - fieldNameStart);
					std::string fieldValueStr = _request.substr(fieldValueStart, fieldValueEnd - fieldValueStart);
					
					_request = _request.substr(fieldValueEnd);
					if (this->httpParseHeader(fieldNameStr.c_str(), fieldValueStr.c_str()))
					{
						//_request = _request.substr(fieldValueEnd);
					}
				}
				continue;
			}//fall
		case MessageBody:
			{ // receive the request data with content length
				size_t messageBody          = _request.find("\r\n\r\n");
				if (std::string::npos != messageBody)
					messageBody = strlen("\r\n\r\n");
				else
					messageBody = 0;

				HttpHeadMap::iterator it = _httpReqHeader.find("Content-Length");
				if (it ==_httpReqHeader.end())
				{
					HttpHeadMap::iterator itChunck = _httpReqHeader.find("Transfer-Encoding");
					if (itChunck != _httpReqHeader.end())
					{
						this->setState(ERROR_QUIT, _currentState);
						continue;//error, it must have length
					}else{
						_request = _request.substr(messageBody, _request.length());
						this->setState(RequestData_ChunkSize, _currentState);
						continue;
					}
				}

				_contentLength     = ::atoi(it->second.c_str());				
				_needContentLength = _contentLength + messageBody - _httpReqBodyLength;
				if(0 == _contentLength){
					this->setState(FinalAssemble, _currentState);//succeed		
				}else if (_needContentLength <= _request.size()){
					_httpReqMessageBody = _request.substr(messageBody, _needContentLength - messageBody);
					_request = _request.substr(_needContentLength);
					this->setState(FinalAssemble, _currentState);//succeed
				} else{
					_httpReqMessageBody += _request.substr(messageBody);//wait for another data
					_request.clear();
					this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
					continue;
				}
			}
			break;
		case RequestData_ChunkSize:
			{ // try get the chunk size
				size_t chunkStart = _request.find_first_of("\r\n");
				if (std::string::npos == chunkStart)
				{//data not recv completely
					this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
					continue;
				}

				unsigned long chunkSize = strtoul(_request.c_str(), NULL, 16);// parse the HEX string
				if(0 == chunkSize)
					this->setState(RequestData_Trailer, _currentState);//chunk end but next would be trailer
				else if(ULONG_MAX == chunkSize)
				{ // overflow, rare case
					//_log(ZQ::common::Log::L_ERROR, PARSINGLOGFMT("Chunk size overflow. chunksize=%s."), chunkSize);
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}
				else
					this->setState(RequestData_ChunkBody, _currentState);
			}//fall
		case RequestData_ChunkBody:
			{ // get the chunk body
				size_t chunkStart = _request.find_first_of("\r\n");
				unsigned long chunkSize = strtoul(_request.c_str(), NULL, 16);
				unsigned long chunkEnd  = chunkSize + chunkStart + strlen("\r\n") ;

				this->setState(RequestData_ChunkSize, _currentState);//reset to chunksize for next parser
				if (_request.size() < chunkEnd)//not recv completely
					goto ForceQuit;

				_httpReqMessageBody += _request.substr(chunkStart + strlen("\r\n"), chunkEnd);
				_request = _request.substr(chunkEnd + 1, _request.size());
				if (2048 < _httpReqMessageBody.size() && _proc)
				{
					_proc->onData(_httpReqHeader, _httpReqMessageBody);
					_httpReqMessageBody.clear();//not buffer for big data
				}

				continue;//next loop
			}
		case RequestData_Trailer:
			{
				HttpHeadMap::iterator itTrailer = _httpReqHeader.find("Trailer");
				HttpHeadMap::iterator itTE      = _httpReqHeader.find("TE");//if not include TE, abandon trailer

				this->setState(FinalAssemble, _currentState);//if any error, we just ignore it
				if (itTrailer ==_httpReqHeader.end() && itTE == _httpReqHeader.end())
					continue;//to FinalAssemble
				//read entity-header
				//	whle(entity-header not empty){
				//	append entity-header to existing header fields
				//    read entity-header
				//}
				std::string trailerHeader(itTrailer->second);
				while (!_request.empty())
				{
					size_t entityHeaderPos = _request.find_first_of("\r\n");
					if (std::string::npos == entityHeaderPos)
						break;//ERROR but ignore

					std::string entityHeader = _request.substr(0, entityHeaderPos);
					size_t fieldName         = _request.find_first_of(":");

					_request                = _request.substr(entityHeaderPos);//smaller request data
					if (std::string::npos == fieldName)
						continue;//ERROR but ignore

					size_t fieldValueStart       = entityHeader.find_first_not_of("  :", fieldName);
					size_t fieldValueEnd         = entityHeader.find_first_of("\r\n", fieldName);
					size_t fieldNameStart        = entityHeader.find_first_not_of("  \t\r\n");
					size_t fieldNameEnd          = entityHeader.find_first_of("  :", fieldNameStart);
					std::string fieldNameStr  = entityHeader.substr(fieldNameStart, fieldNameEnd - fieldNameStart);
					std::string fieldValueStr = entityHeader.substr(fieldValueStart, fieldValueEnd - fieldValueStart);
					if (std::string::npos != trailerHeader.find(fieldValueStr))//just add which trailer marked
						this->httpParseHeader(fieldNameStr.c_str(), fieldValueStr.c_str());
				}//inter while
			}//fall
		case FinalAssemble:
			{//gzip ...
				_httpReqBodyLength += _httpReqMessageBody.size();
				this->setState(ParseCompleted, _currentState);//not implement currently
			}//fall
		case ParseCompleted:
			{
				//handler complete event
				if (_proc)
					_proc->onComplete(_httpReqHeader, _httpReqMessageBody);
				else if(NULL != _ownConn)
					_ownConn->doSendSync(buffer, bufSize);//just test echo

				if (_errCode != HTTP_MANUAL_SHUTWODN)//Idle will clear all data
					this->setState(Idle, _currentState);// for next http server parser
				else 
					goto ForceQuit;

				continue;
			}
		case DATA_NOT_RECV_COMPLETE:
			{
				_httpReqBodyLength += _httpReqMessageBody.size();
				if (_proc)
					_proc->onData(_httpReqHeader, _httpReqMessageBody);
				else if(_ownConn)
					_ownConn->doSendSync(buffer, bufSize);//just test echo

				if (_errCode != HTTP_MANUAL_SHUTWODN)
				{
					_httpReqMessageBody.clear();//not buffer for big data
					this->setState(_previousState, _previousState);//restore for next parse
				}
				goto ForceQuit;
			}
		case ERROR_QUIT:
			{
				//handle error event
				if (_proc)
					_proc->onError(_errCode);
				else if(_ownConn)
					_ownConn->doSendSync(buffer, bufSize);//just test echo

				if (_errCode != HTTP_MANUAL_SHUTWODN)
					this->setState(Idle, _currentState);// for next http server parser

				goto ForceQuit;
			}
		} // switch end
	} // for end

ForceQuit:

	return true;
}

int HttpServer::onSendEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	if (NULL != _proc)
	{
		ZQ::common::MutexGuard guard(_serverMutex);
		_proc->onDeliver();
	}

	return true;
}

int HttpServer::onErrorEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	if (NULL != _proc)
	{
		ZQ::common::MutexGuard guard(_serverMutex);
		IHttpProc::Ptr proc = _proc;
		
		_proc = NULL;
		proc->onError(_errCode);
	}

	return true;
}

int HttpServer::onCloseEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	if (NULL != _proc)
	{
		ZQ::common::MutexGuard guard(_serverMutex);
		IHttpProc::Ptr proc = _proc;

		_proc = NULL;
		proc->onClose(true);
	}

	return true;
}
//--------------end HttpServer------------------


//--------------start HttpClient------------------
HttpClient::HttpClient(IMgr* mgr, IAsyncServer* server)
: _log(* mgr->_log)
{
	this->setState(Idle, _currentState);
	_responseBodyLength = 0;
	Message* msg     = new Message(this);
	_ownConn = new ConnectCommuncator(server, mgr, msg);
}

HttpClient::HttpClient(ZQ::common::Log& log)
: _log(log)
{
	this->setState(Idle, _currentState);
	_responseBodyLength = 0;
}

HttpClient::~HttpClient()
{
	using namespace ZQ::common;
	//_msg->_clientHandler = NULL;
	_log(Log::L_DEBUG, CLOGFMT(HttpClient, "~HttpClient() errCode[%d]"), _errCode);
	if (_ownConn)
		_ownConn->unInit();

	ZQ::common::MutexGuard guard(_clientMutex);
	//	_proc = NULL;
	//_proc    = _proc;
	_ownConn = NULL;
	_httpSendMethod.clear();
	_httpSendURI.clear();
	_httpSendHeader.clear();
	_httpRecvHeader.clear();
	_requestMessageBody.clear();
	_response.clear();
	_httpRecvVersion.clear();
	_httpRecvStatusCode.clear();
	_reasonPhrase.clear();
	_responseMessageBody.clear();
};

ConnectCommuncator::Ptr HttpClient::getConn(void) const
{
	ZQ::common::MutexGuard guard(_clientMutex);
	return _ownConn;
}

int  HttpClient::attachConn(ConnectCommuncator::Ptr conn)
{
	ZQ::common::MutexGuard guard(_clientMutex);
	_ownConn = conn;
	return true;
}

int HttpClient::attachProc(IHttpProc::Ptr proc, const char* dstTo, unsigned int dstPort)
{
	using namespace ZQ::common;
	ZQ::common::MutexGuard guard(_clientMutex);
	if (!dstTo || dstPort > 65535 || _errCode == HTTP_MANUAL_SHUTWODN)
	{
		_log(Log::L_DEBUG, CLOGFMT(HttpClient, "attachProc() err, dst[NULL] or dstPort[%d] errCode[%d]"), dstPort, _errCode);
		return false;
	}

	_proc    = proc;
	_dstTo   = dstTo;
	_dstPort = dstPort;

	if(!_ownConn->init(dstTo, dstPort))
	{
		_log(Log::L_DEBUG, CLOGFMT(HttpClient, "attachProc() conn init failed, dst[%s] or dstPort[%d] errCode[%d]"), dstTo, dstPort, _errCode);
		return false;
	}

	if(regAsyncSendEvent())
	{
		_log(Log::L_DEBUG, CLOGFMT(HttpClient, "Succeed attachProc(), dst[%s] or dstPort[%d] errCode[%d]"), dstTo, dstPort, _errCode);
		_isRegSendEvent = true;
		return true;
	}
	
	_proc = NULL;
	_log(Log::L_DEBUG, CLOGFMT(HttpClient, "Failed to regAsyncSend in attachProc(), dst[%s] or dstPort[%d] errCode[%d]"), dstTo, dstPort, _errCode);
	return false;
}

int  HttpClient::setState(HttpClientState current, HttpClientState previous)
{
	_currentState  = current;
	_previousState = previous;
	if (Idle == current)
		this->setErrorCode(HTTP_OK);
	else if (ERROR_QUIT == current)
	{
		switch(previous)
		{
		default:
			this->setErrorCode(HTTP_ERR);
			break;
		case StatusLine:
			this->setErrorCode(HTTP_TYPE_ERR);
			break;
		case ResponseHeader:
			this->setErrorCode(HTTP_HDR_ERR);
			break;
		case MessageBody:
			this->setErrorCode(HTTP_MESSAGE_ERR);
			break;
		case ResponseData_ChunkSize:
			this->setErrorCode(HTTP_CHUNK_BODY_ERR);
			break;
		case ResponseData_ChunkBody:
			this->setErrorCode(HTTP_CHUNK_SIZE_ERR);
			break;
		case ResponseData_Trailer:			
			this->setErrorCode(HTTP_CHUNK_TRAILER_ERR);
			break;
		}
	}

	return true;
}

int HttpClient::onRecvEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	ZQ::common::MutexGuard guard(_clientMutex);
	if (!_ownConn)
		return false;

	_response.append(buffer, bufSize);
	while (!_response.empty() || FinalAssemble <= _currentState)
	{
		switch(_currentState)
		{
		case Idle:
			{ // search for the response line
				this->setState(StatusLine, _currentState);
				_responseBodyLength = 0;
				_needContentLength  = 0;
				_httpRecvHeader.clear();
				_httpRecvVersion.clear();
				_httpRecvStatusCode.clear();
				_responseMessageBody.clear();
			}//fall
		case StatusLine:
			{// HTTP-Version SP Status-Code SP Reason-Phrase CRLF 
				const char* buf    = _response.c_str();
				size_t statusLine  = _response.find_first_not_of(" \r\n");
				if(std::string::npos == statusLine
					|| _response.size() < statusLine + 13 )// 13 = min(StatusLine)
				{
					_response.clear();
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}

				buf += statusLine;
				if (0 == strncasecmp(buf, "HTTP/1.1", 8))
					_httpRecvVersion = "HTTP/1.1";
				else if (0 == strncasecmp(buf, "HTTP/1.0", 8))
					_httpRecvVersion = "HTTP/1.0";
				else 
				{
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}

				size_t statusCode    = _response.find_first_not_of("  ", statusLine + 8);//8 = sizeof(HTTP-Version)
				size_t reasonPhrase  = _response.find_first_not_of("  ", statusLine + 8 + 4 + 1);// 4 = sizeof(SP) + sizeof(Status-Code)
				size_t statusLineEnd = _response.find_first_of("\r\n", statusLine + 8 + 4 + 1 + 1);// 1 = sizeof(SP)
				if (std::string::npos == statusCode || std::string::npos == reasonPhrase 
					|| std::string::npos == statusLineEnd)
				{
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}

				if(statusCode == (statusLine + 8 + 1)
					&& statusCode == (reasonPhrase - 4) )
				{
					_httpRecvStatusCode = _response.substr(statusCode, 3);// 3 = sizeof(Status-Code)
					_reasonPhrase       = _response.substr(reasonPhrase, statusLineEnd - reasonPhrase);
				}
				else
				{
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}

				_response = _response.substr(statusLineEnd + 2);
				this->setState(ResponseHeader, _currentState);
			}//fall
		case ResponseHeader:
			{//message-header = field-name ":"[ field-value ]
				size_t headerStart     = _response.find_first_not_of("  \r\n");
				size_t messageBody     = _response.find("\r\n\r\n");
				if (std::string::npos == messageBody)
				{//currently message body has not been recved
					if(std::string::npos != headerStart)
						_response = _response.substr(headerStart);
					
					size_t fieldName     = _response.find_first_of(":");
					size_t fieldValueEnd = _response.find_first_of("\r\n");
					if (std::string::npos == fieldName || std::string::npos == fieldValueEnd)
					{//response data not recv completely
						this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
						continue;
					}

					std::string fieldNameStr  = _response.substr(0, fieldName);
					std::string fieldValueStr = _response.substr(fieldName + 1, fieldValueEnd - fieldName - 2);
					if (this->httpParseHeader(fieldNameStr.c_str(), fieldValueStr.c_str()))
					{
						_response = _response.substr(fieldValueEnd + 1);
					}
				}
				else
				{//message body has been recved
					if(std::string::npos == messageBody && std::string::npos == headerStart)
					{
						this->setState(ERROR_QUIT, _currentState);
						continue;//error, it must to get \r\n
					}
					
					if(headerStart >= messageBody)
					{
						this->setState(MessageBody, _currentState);
						continue;
					}
					
					_response         = _response.substr(headerStart);
					size_t fieldName  = _response.find_first_of(":");
					if (std::string::npos == fieldName || fieldName >= messageBody)
					{
						this->setState(ERROR_QUIT, _currentState);
						continue;//ERROR
					}
					size_t fieldValueStart       = _response.find_first_not_of("  :", fieldName);
					size_t fieldValueEnd         = _response.find_first_of("\r\n", fieldName);
					size_t fieldNameStart        = _response.find_first_not_of("  \t\r\n");
					size_t fieldNameEnd          = _response.find_first_of("  :", fieldNameStart);
					std::string fieldNameStr  = _response.substr(fieldNameStart, fieldNameEnd - fieldNameStart);
					std::string fieldValueStr = _response.substr(fieldValueStart, fieldValueEnd - fieldValueStart);
					
					_response = _response.substr(fieldValueEnd);
					if (this->httpParseHeader(fieldNameStr.c_str(), fieldValueStr.c_str()))
					{
						//_response = _response.substr(fieldValueEnd);
					}
				}
				continue;
			}//fall
		case MessageBody:
			{ // receive the response data with content length
				size_t messageBody = _response.find("\r\n\r\n");
				if (std::string::npos != messageBody)
					messageBody = strlen("\r\n\r\n");
				else
					messageBody = 0;

				HttpHeadMap::iterator it = _httpRecvHeader.find("Content-Length");
				if (it ==_httpRecvHeader.end())
				{
                    HttpHeadMap::iterator itChunck = _httpRecvHeader.find("Transfer-Encoding");
					if (itChunck != _httpRecvHeader.end())
					{
						this->setState(ERROR_QUIT, _currentState);
						continue;//error, it must have length
					}else{
						_response = _response.substr(messageBody, _response.length());
						this->setState(ResponseData_ChunkSize, _currentState);
						continue;
					}
				}

				_contentLength     = ::atoi(it->second.c_str());				
				_needContentLength = _contentLength + messageBody - _responseBodyLength;
				if(0 == _contentLength){
					this->setState(FinalAssemble, _currentState);//succeed		
				}else if (_needContentLength <= _response.size()){
					_responseMessageBody = _response.substr(messageBody, _needContentLength - messageBody);
					_response = _response.substr(_needContentLength);
					this->setState(FinalAssemble, _currentState);//succeed
				} else{
					_responseMessageBody += _response.substr(messageBody);//wait for another data
					_response.clear();
					this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
					continue;
				}
			}
			break;
		case ResponseData_ChunkSize:
			{ // try get the chunk size
				size_t chunkStart = _response.find_first_of("\r\n");
				if (std::string::npos == chunkStart)				
				{//response data not recv completely
					this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
					continue;
				}

				unsigned long chunkSize = strtoul(_response.c_str(), NULL, 16);// parse the HEX string
				if(0 == chunkSize)
					this->setState(ResponseData_Trailer, _currentState);//chunk end but next would be trailer
				else if(ULONG_MAX == chunkSize)
				{ // overflow, rare case
					//_log(ZQ::common::Log::L_ERROR, PARSINGLOGFMT("Chunk size overflow. chunksize=%s."), chunkSize);
					this->setState(ERROR_QUIT, _currentState);
					continue;
				}
				else
					this->setState(ResponseData_ChunkBody, _currentState);
			}//fall
		case ResponseData_ChunkBody:
			{ // get the chunk body
				size_t chunkStart = _response.find_first_of("\r\n");
				unsigned long chunkSize = strtoul(_response.c_str(), NULL, 16);
				unsigned long chunkEnd  = chunkSize + chunkStart + strlen("\r\n") ;

				this->setState(ResponseData_ChunkSize, _currentState);//reset to chunksize for next parser
				if (_response.size() < chunkEnd)//not recv completely
				{
					this->setState(DATA_NOT_RECV_COMPLETE, _currentState);
					continue;
				}

				_responseMessageBody += _response.substr(chunkStart + strlen("\r\n"), chunkEnd);
				_response = _response.substr(chunkEnd + 1, _response.size());
				if (2048 < _responseMessageBody.size() && _proc)
				{
					_proc->onData(_httpRecvHeader, _responseMessageBody);
					_responseMessageBody.clear();//not buffer for big data
				}
				continue;//next loop
			}
		case ResponseData_Trailer:
			{
				HttpHeadMap::iterator itTrailer = _httpRecvHeader.find("Trailer");
				HttpHeadMap::iterator itTE      = _httpRecvHeader.find("TE");//if not include TE, abandon trailer

				this->setState(FinalAssemble, _currentState);//if any error, we just ignore it
				if (itTrailer ==_httpRecvHeader.end() && itTE == _httpRecvHeader.end())
					continue;//to FinalAssemble
				//read entity-header
				//	whle(entity-header not empty){
				//	append entity-header to existing header fields
				//    read entity-header
				//}
				std::string trailerHeader(itTrailer->second);
				while (!_response.empty())
				{
					size_t entityHeaderPos = _response.find_first_of("\r\n");
					if (std::string::npos == entityHeaderPos)
						break;//ERROR but ignore

					std::string entityHeader = _response.substr(0, entityHeaderPos);
					size_t fieldName         = _response.find_first_of(":");

					_response                = _response.substr(entityHeaderPos);//smaller response data
					if (std::string::npos == fieldName)
							continue;//ERROR but ignore

					size_t fieldValueStart       = entityHeader.find_first_not_of("  :", fieldName);
					size_t fieldValueEnd         = entityHeader.find_first_of("\r\n", fieldName);
					size_t fieldNameStart        = entityHeader.find_first_not_of("  \t\r\n");
					size_t fieldNameEnd          = entityHeader.find_first_of("  :", fieldNameStart);
					std::string fieldNameStr  = entityHeader.substr(fieldNameStart, fieldNameEnd - fieldNameStart);
					std::string fieldValueStr = entityHeader.substr(fieldValueStart, fieldValueEnd - fieldValueStart);
					if (std::string::npos != trailerHeader.find(fieldValueStr))//just add which trailer marked
						this->httpParseHeader(fieldNameStr.c_str(), fieldValueStr.c_str());
				}//inter while
			}//fall
		case FinalAssemble:
			{//gzip ...
				_responseBodyLength += _responseMessageBody.size();// last recv would be DATA_NOT_RECV_COMPLETE, so using "+="
			    this->setState(ParseCompleted, _currentState);//not implement currently
			}//fall
		case ParseCompleted:
			{
				//handler complete event
				if (_proc)
					_proc->onComplete(_httpRecvHeader, _responseMessageBody);
				else if(NULL != _ownConn)
					_ownConn->doSendSync(buffer, bufSize);//just test echo

				if(_errCode != HTTP_MANUAL_SHUTWODN)
					this->setState(Idle, _currentState);// for next httpclient parser
				else
					goto ForceQuit;

				continue;
			}
		case DATA_NOT_RECV_COMPLETE:
			{
				_responseBodyLength += _responseMessageBody.size();
				if (_proc)
					_proc->onData(_httpRecvHeader, _responseMessageBody);
				else if(NULL != _ownConn)
					_ownConn->doSendSync(buffer, bufSize);//just test echo

				if(_errCode != HTTP_MANUAL_SHUTWODN)
				{
					_responseMessageBody.clear();
					this->setState(_previousState, _previousState);//restore for next httpclient parser
				}

				goto ForceQuit;
			}
		case ERROR_QUIT:
			{
				//handle error event
				if (_proc)
					_proc->onError(_errCode);
				else if(NULL != _ownConn)
					_ownConn->doSendSync(buffer, bufSize);//just test echo

				if(_errCode != HTTP_MANUAL_SHUTWODN)
					this->setState(Idle, _currentState);// for next httpclient parser
				
				goto ForceQuit;
			}
		} // switch end
	} // for end

ForceQuit:

	return true;
}

int HttpClient::onSendEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	if (_proc)
	{
		//ZQ::common::MutexGuard guard(_clientMutex);
		_isRegSendEvent = false;
		_proc->onDeliver();
	}

	return true;
}

int HttpClient::onErrorEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	ZQ::common::MutexGuard guard(_clientMutex);
	this->setErrorCode(HTTP_TCP_ERROR);
	if (_proc)
	{
		IHttpProc::Ptr proc = _proc;

		_proc = NULL;
		proc->onError(_errCode);
		_ownConn = NULL;
	}

	//delete this;

	return true;
}

int HttpClient::onCloseEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)
{
	ZQ::common::MutexGuard guard(_clientMutex);
	if (_proc)
	{
		IHttpProc::Ptr proc = _proc;

		_proc = NULL;
		proc->onClose(true);
		_ownConn = NULL;
	}

	//delete this;

	return true;
}

int HttpClient::shutDown(void)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpClient, "Httpclient manual shutDown(), dst[%s] or dstPort[%d] errCode[%d]"), _dstTo.c_str(), _dstPort, _errCode);

	CommonTcpCommuncator::Ptr conn = _ownConn;

	_ownConn = NULL;
	setErrorCode(HTTP_MANUAL_SHUTWODN);
	_proc = NULL;
	if(!conn->unInit())
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpClient, "Httpclient manual shutDown(), conn unInit failed, dst[%s] or dstPort[%d] errCode[%d]"), _dstTo.c_str(), _dstPort, _errCode);
		return false;
	}

	return true;
}

int  HttpClient::setSendHeader(char* key, char* val)
{
	if(key && val)//add or update header
		_httpSendHeader[key] = val;
	else if(NULL == val && key)//erase header
	{
		std::map<std::string,std::string>::iterator it = _httpSendHeader.find(key);
		if(it != _httpSendHeader.end())
			_httpSendHeader.erase(it);
	}
	else // clean map
	{
		if(_httpSendHeader.size())
			_httpSendHeader.clear();
	}

	return true;
}

int  HttpClient::setSendMethod(HttpMethod method)
{
	_httpSendMethod = method;
	return true;
}

int  HttpClient::setSendURI(char* sendURI)
{
	_httpSendURI = sendURI;
	return _httpSendURI.size();
}

int HttpClient::syncSendRaw(char* raw, unsigned long rawLen)
{
	ZQ::common::MutexGuard guard(_clientMutex);
	if (!raw)
		return -1;

	int size = _ownConn->doSendSync(raw, rawLen);
	if (size < rawLen)
	{
		if (_isRegSendEvent == false)
		{
			regAsyncSendEvent();
			_isRegSendEvent = true;
		}
	}

	return size;
}

int HttpClient::syncSendHttpRaw(char* messageBody, unsigned long bodyLen)
{
	ZQ::common::MutexGuard guard(_clientMutex);
	if (NULL == _ownConn)
		return false;

	//1. request line
	char buff[100];
	::sprintf(buff, "%d", bodyLen);
	std::string sp(" ");
	std::string colon(": ");//include sp
	std::string crlf("\r\n");
	std::string httpSend(_httpSendMethod);
	httpSend += sp + _httpSendURI;
	httpSend += sp + "HTTP/1.1" + crlf;

	HttpHeadMap::const_iterator it = _httpSendHeader.find("Content-Length");//we don't send "Transfer-Encoding" mode 
	if (it == _httpSendHeader.end())
	{
		this->setSendHeader("Content-Length", buff);
	}

	//2. request headers
	for (it = _httpSendHeader.begin(); it != _httpSendHeader.end(); ++it)
	{
		httpSend += it->first  + colon;
		httpSend += it->second + crlf;
	}

	httpSend += crlf;
	int size = _ownConn->doSendSync(httpSend.c_str(), httpSend.size());
	if (0 > size)
	{
		if (false == _isRegSendEvent)
		{
			int regRev = regAsyncSendEvent();
			_isRegSendEvent = true;//log
		}

		return size;
	}

	if(NULL != messageBody)
		size = _ownConn->doSendSync(messageBody, bodyLen);

	if (size < bodyLen && false == _isRegSendEvent)
	{
		int regRev = regAsyncSendEvent();
		_isRegSendEvent = true;//log
	}

	return (0 < size) ? (size + httpSend.size()) : size;
}

int  HttpClient::regAsyncSendEvent(void)
{
    return ERROR_CODE_OPERATION_OK == _ownConn->doSendAsync(NULL);;
}

int  HttpClient::regAsyncRecvEvent(void)
{
	return ERROR_CODE_OPERATION_OK ==_ownConn->doRecvAsync(NULL);
}

int HttpClient::getHttpVersion(std::string& version) const
{
	ZQ::common::MutexGuard guard(_clientMutex);
	version = _httpRecvVersion;
	return true;
}
int HttpClient::getStatusCode(std::string& statusCode) const
{
	ZQ::common::MutexGuard guard(_clientMutex);
	statusCode = _httpRecvStatusCode;
	return true;
}
int HttpClient::getReason(std::string& reason) const
{
	ZQ::common::MutexGuard guard(_clientMutex);
	reason = _reasonPhrase;
	return true;
}

unsigned long HttpClient::getContentLength(void) const
{
	return _contentLength;
}

unsigned long HttpClient::getRecvContentSize(void) const
{
	ZQ::common::MutexGuard guard(_clientMutex);
	return _responseBodyLength;// would increase
}

int  HttpClient::getRecvHeader(HttpHeadMap& httpHead) const
{
	ZQ::common::MutexGuard guard(_clientMutex);
	//std::swap(_httpRecvHeader, httpHead);// for performance, once used
	httpHead = _httpRecvHeader;//we need header for parser
	return httpHead.size();
}

int  HttpClient::getMessageBody(std::string& messageBody)
{
	ZQ::common::MutexGuard guard(_clientMutex);
	std::swap(_responseMessageBody, messageBody);// for performance, once used
	//messageBody = _responseMessageBody;
	return _responseBodyLength;
}

int  HttpClient::strncasecmp(const char *s1, char *s2, size_t n)
{
	unsigned char  c1, c2;

	while (n) 
	{
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
		c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

		if (c1 == c2) 
		{
			if (c1) 
			{
				n--;
				continue;
			}

			return 0;
		}

		return c1 - c2;
	}

	return 0;
}

int  HttpClient::httpParseHeader(const char *key, const char *val)
{
	if(!key || !val)//add to header map
		return false;
		

	int supportHeader = false;
	if (!strncasecmp(key, "Host", 4))
	{ 
		supportHeader = true;
	}
#ifndef WITH_LEANER
	else if (!strncasecmp(key, "Content-Type", 12))
	{ 
        supportHeader = true;
	}
#endif
	else if (!strncasecmp(key, "Content-Length", 14))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Content-Encoding", 16))
	{ 
		if (!strncasecmp(val, "deflate", 7))
			supportHeader = true;
		else if (!strncasecmp(val, "gzip", 4))
			supportHeader = true;
	}
#ifdef WITH_ZLIB
	else if (!strncasecmp(key, "Accept-Encoding", 15))
	{
		supportHeader = true;
	}
#endif
	else if (!strncasecmp(key, "Transfer-Encoding", 17))
	{ 
		if (!strncasecmp(val, "chunked", 7))
		{
			supportHeader = true;
		}
	}
	else if (!strncasecmp(key, "Connection", 10))
	{ 
		if (!strncasecmp(val, "keep-alive", 10))
		{
			supportHeader = true;
		}
		else if (!strncasecmp(val, "close", 5))
		{
			supportHeader = true;
		}
	}
#ifndef WITH_LEAN
	else if (!strncasecmp(key, "Authorization", 13))
	{ 
		if (!strncasecmp(val, "Basic *", 7))
		{ 
			supportHeader = true;
		}
	}
	else if (!strncasecmp(key, "WWW-Authenticate", 16))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Expect", 6))
	{ 
		if (!strncasecmp(val, "100-continue", 12))
		{ 
			supportHeader = true;
		}
	}
#endif
	else if (!strncasecmp(key, "Location", 8))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "X-Forwarded-For", 15))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Accept-Ranges", 13))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Cache-Control", 13))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Server", 6))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "Date", 4))
	{ 
		supportHeader = true;
	}
	else if (!strncasecmp(key, "TianShan-Notice", 15))
	{ 
		supportHeader = true;
	}

	if (supportHeader)
		_httpRecvHeader[key] = val;

	return supportHeader;
}

HttpClient::Ptr      HttpClientFactory::create()
{
	HttpClient*     client    =  new (std::nothrow) HttpClient(*_mgr->_log);
	HttpClient::Ptr httpClientPtr =  client;
	Message*        msg       =  new Message(client);
	ConnectCommuncator::Ptr conn = new ConnectCommuncator(_asyncServer, _mgr, msg);

	msg->_selfConn = conn;
	client->attachConn(conn);
	_mgr->add(conn, httpClientPtr);

	return httpClientPtr;
};

//--------------end HttpClient------------------