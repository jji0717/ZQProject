#ifndef __I_ASYNC_CLIENT_HANDLER_HPP__
#define __I_ASYNC_CLIENT_HANDLER_HPP__

#include <new>
#include <list>
#include <string>
#include <map>
#include "Locks.h"
#include "ICommunicator.hpp"
#include "IAsyncServer.hpp"

class IAsyncClientHandler : public ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer<IAsyncClientHandler>  Ptr;

	virtual ~IAsyncClientHandler(){};
	virtual int onRecvEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)  = 0;
	virtual int onSendEvent(char* buffer, unsigned int bufSize, ICommuncator* conn)  = 0;
	virtual int onErrorEvent(char* buffer, unsigned int bufSize, ICommuncator* conn) = 0;
	virtual int onCloseEvent(char* buffer, unsigned int bufSize, ICommuncator* conn) = 0;
	virtual int attachConn(CommonTcpCommuncator::Ptr conn){return false;};
};

class IHttpProc : public ZQ::common::SharedObject
{
public:
	enum NextAct{
		FORCE_QUIT      ,
		CONTINUE        ,
		NEXT_ACT_COUNT
	};

	typedef ZQ::common::Pointer<IHttpProc>  Ptr;
	typedef std::map<std::string, std::string>  HttpHeadMap;

	virtual ~IHttpProc(){};	

	virtual NextAct onData(const HttpHeadMap& httpHead, const std::string& data)     = 0;
	virtual NextAct onComplete(const HttpHeadMap& httpHead, const std::string& data) = 0;
	virtual NextAct onDeliver(void)  = 0;

	virtual void onClose(int reason) = 0;
	virtual void onError(int errCode) = 0;
};

class HttpServer : public IAsyncClientHandler
{
public:
	typedef std::map<std::string, std::string>  HttpHeadMap;

	enum HttpServerState
	{
		Idle                  = 0,
		RequestLine              ,
		RequestHeader            ,
		MessageBody              ,
		RequestData_ChunkSize    ,
		RequestData_ChunkBody    ,
		RequestData_Trailer      , 
		FinalAssemble            ,
		ParseCompleted           ,
		ERROR_QUIT               ,
		DATA_NOT_RECV_COMPLETE   ,
		STATE_LIMIT
	};

	typedef enum ErrCode
	{
		HTTP_OK		           = 0,
		HTTP_ERR		          ,
		HTTP_TYPE_ERR             ,
		HTTP_HDR_ERR              ,
		HTTP_MESSAGE_ERR          ,	
		HTTP_CHUNK_BODY_ERR       ,	
		HTTP_CHUNK_SIZE_ERR       ,	
		HTTP_CHUNK_TRAILER_ERR    ,	
		HTTP_TCP_ERROR            ,
		HTTP_SSL_ERROR            ,
		HTTP_ZLIB_ERROR           ,
		HTTP_MANUAL_SHUTWODN      ,
		HTTP_ERR_COUNT
	} ErrCode;

	enum HttpMethod
	{
		HTTP_POST	= 2000,
		HTTP_GET          ,
		HTTP_NOT_IMPLEMENTED
	};

public:	
	typedef ZQ::common::Pointer<HttpServer>  Ptr;

	HttpServer(IHttpProc::Ptr proc = IHttpProc::Ptr());
	virtual ~HttpServer();

	int  shutDown(void);
	int  syncSendRaw(char* raw, unsigned long rawLen);

	int           getHttpVersion(std::string& version) const;
	int           getURI(std::string& uri) const;
	int           getMethod(std::string& method) const;
	unsigned long getContentLength(void) const; //Content length that http header marked
	unsigned long getReqContentSize(void) const;//How much size has recved
	int           getReqHeader(HttpHeadMap& httpHead) const;
	int           getMessageBody(std::string& messageBody) const;
	ErrCode       getErrorCode(void)const {	return _errCode; };

	CommonTcpCommuncator::Ptr  getConn(void) const;

	int           attachProc(IHttpProc::Ptr proc);
    virtual int   attachConn(CommonTcpCommuncator::Ptr conn);

private:
	virtual int onRecvEvent(char* buffer, unsigned int bufSize, ICommuncator* conn) ;
	virtual int onSendEvent(char* buffer, unsigned int bufSize, ICommuncator* conn) ;
	virtual int onErrorEvent(char* buffer, unsigned int bufSize, ICommuncator* conn);
	virtual int onCloseEvent(char* buffer, unsigned int bufSize, ICommuncator* conn);
	
	int  strncasecmp(const char *s1, char *s2, size_t n);
	int  httpParseHeader(const char *key, const char *val);

	int  setState(HttpServerState current, HttpServerState previous);
	int  setErrorCode(ErrCode err){ _errCode = err; return true; }
	int  regAsyncSendEvent(void);
	int  regAsyncRecvEvent(void);

private:
	CommonTcpCommuncator::Ptr  _ownConn;
	std::string                _dstTo;
	unsigned int               _dstPort;

	IHttpProc::Ptr            _proc;
	ZQ::common::Mutex      _serverMutex;
	ErrCode                _errCode;

	//for response
	std::string            _httpRespMethod;
	std::string            _httpRespStatusCode;
	std::string            _reasonPhrase;
	std::string            _httpRespVersion;
	HttpHeadMap            _httpRespHeader;
	std::string            _respMessageBody;

	//for request
	HttpServerState        _currentState;
	HttpServerState        _previousState;
	std::string            _request;
	std::string            _httpReqMethod;
	std::string            _httpReqURI;
	std::string            _httpReqVersion;
	std::string            _httpReqMessageBody;
	unsigned long          _httpReqBodyLength;
	unsigned long          _needContentLength;
	unsigned long          _contentLength;
	HttpHeadMap            _httpReqHeader;
};


class HttpClient : public IAsyncClientHandler
{
public:
	typedef std::map<std::string, std::string>  HttpHeadMap;

	enum HttpClientState
	{
		Idle                  = 0 ,
		StatusLine                ,
		ResponseHeader            ,
		MessageBody               ,
		ResponseData_ChunkSize    ,
		ResponseData_ChunkBody    ,
		ResponseData_Trailer      , 
		FinalAssemble             ,
		ParseCompleted            ,
		DATA_NOT_RECV_COMPLETE    ,
		ERROR_QUIT
	};

	typedef enum ErrCode
	{
		HTTP_OK		           = 0,
		HTTP_ERR		          ,
		HTTP_TYPE_ERR             ,
		HTTP_HDR_ERR              ,
		HTTP_MESSAGE_ERR          ,	
		HTTP_CHUNK_BODY_ERR       ,	
		HTTP_CHUNK_SIZE_ERR       ,	
		HTTP_CHUNK_TRAILER_ERR    ,	
		HTTP_TCP_ERROR            ,
		HTTP_SSL_ERROR            ,
		HTTP_ZLIB_ERROR           ,
		HTTP_MANUAL_SHUTWODN      ,
		HTTP_ERR_COUNT
	} ErrCode;

	enum HttpMethod
	{
		HTTP_POST	= 2000,
		HTTP_GET          ,
		HTTP_NOT_IMPLEMENTED
	};

public:
	typedef ZQ::common::Pointer<HttpClient>  Ptr;

	HttpClient(IMgr* mgr, IAsyncServer* server);
	HttpClient(ZQ::common::Log& log);

	virtual ~HttpClient();

	int           shutDown(void);
	int           setSendHeader(char* key, char* val);
	int           setSendMethod(HttpMethod method);
	int           setSendURI(char* sendURI);
	int           syncSendRaw(char* raw, unsigned long rawLen);
	int           syncSendHttpRaw(char* messageBody, unsigned long bodyLen);

	int           getHttpVersion(std::string& version)   const;
	int           getStatusCode(std::string& statusCode) const;
	int           getReason(std::string& reason) const;
	unsigned long getContentLength(void)   const;
	unsigned long getRecvContentSize(void) const;
	int           getRecvHeader(HttpHeadMap& httpHead) const;
	int           getMessageBody(std::string& messageBody);

	ErrCode       getErrorCode(void) const { return _errCode; };

	ConnectCommuncator::Ptr  getConn(void) const;
	
	virtual int attachConn(ConnectCommuncator::Ptr conn);
	int         attachProc(IHttpProc::Ptr proc, const char* dstTo, unsigned int dstPort);

private:
	virtual int onRecvEvent(char* buffer, unsigned int bufSize, ICommuncator* conn) ;
	virtual int onSendEvent(char* buffer, unsigned int bufSize, ICommuncator* conn) ;
	virtual int onErrorEvent(char* buffer, unsigned int bufSize, ICommuncator* conn);
	virtual int onCloseEvent(char* buffer, unsigned int bufSize, ICommuncator* conn);

	int  httpParseHeader(const char *key, const char *val);
	int  strncasecmp(const char *s1, char *s2, size_t n);
	int  setState(HttpClientState current, HttpClientState previous);
	int  setErrorCode(ErrCode err)	{	_errCode = err; 	return true;}
	
	int  regAsyncSendEvent(void);
	int  regAsyncRecvEvent(void);

private:
	ZQ::common::Log&         _log;
//	Message*                 _msg;
	std::string              _dstTo;
	unsigned int             _dstPort;
	IHttpProc::Ptr           _proc;
	ZQ::common::Mutex        _clientMutex;
	ConnectCommuncator::Ptr  _ownConn;
	ErrCode                  _errCode;
//for send
	std::string            _httpSendMethod;
	std::string            _httpSendURI;
	HttpHeadMap            _httpSendHeader;
	std::string            _requestMessageBody;
	bool                   _isRegSendEvent;

//for recv
	HttpClientState        _currentState;
	HttpClientState        _previousState;
	std::string            _response;
	std::string            _httpRecvVersion;
	std::string            _httpRecvStatusCode;
	std::string            _reasonPhrase;
	std::string            _responseMessageBody;
	unsigned long          _responseBodyLength;
	unsigned long          _needContentLength;
	unsigned long          _contentLength;
	HttpHeadMap            _httpRecvHeader;
};

class IAsyncClientFactory
{
public:
	virtual ~IAsyncClientFactory(){};

	virtual IAsyncClientHandler* create() = 0;
	virtual void destroy() = 0;
};

class CLASSINDLL_CLASS_DECL HttpClientFactory
{
public:
	HttpClientFactory(IMgr* mgr, IAsyncServer* asyncServer)
		:_mgr(mgr), _asyncServer(asyncServer)
	{};

	virtual ~HttpClientFactory(){};

	virtual HttpClient::Ptr create();

	virtual void destroy(){};

private:
	IMgr*         _mgr;
	IAsyncServer* _asyncServer;
}; // end class HttpClientFactory


class HttpServerFactory : public IAsyncClientFactory
{
public:
	HttpServerFactory(IAsyncServer* asyncServer)
		:_asyncServer(asyncServer)
	{};

	virtual ~HttpServerFactory(){};

	virtual IAsyncClientHandler* create()
	{
		return new (std::nothrow) HttpServer;
	};

	virtual void destroy(){};

private:
	IAsyncServer* _asyncServer;
}; // end class HttpServerFactory

#endif//__I_ASYNC_CLIENT_HANDLER_HPP__