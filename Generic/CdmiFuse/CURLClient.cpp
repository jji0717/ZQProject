//#include "StdAfx.h"
#include <stdio.h>
#include "CURLClient.h"
#include <algorithm>
#include "urlstr.h"
#include "Guid.h"
#include "Locks.h"
#include "SystemUtils.h"
#include "InetAddr.h"

#ifdef ZQ_OS_LINUX
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/tcp.h>
#	include <netdb.h>
#	define _snprintf snprintf 
#    define CURL_SOCKET_ERROR EAGAIN
#else
#	define CURL_SOCKET_ERROR WSAEWOULDBLOCK
#endif//ZQ_OS

#ifndef max
#  define max(A,B) ((A>B)?A:B)
#  define min(A,B) ((A>B)?B:A)
#endif

namespace ZQ
{
namespace common
{

#ifndef MAPSET
#  define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAPSET

	// -----------------------------
	// class CURLClientCompleteCmd
	// -----------------------------
	class CURLClientCompleteCmd : public ZQ::common::ThreadRequest
	{
	public:
		CURLClientCompleteCmd(ZQ::common::NativeThreadPool& thpool, CURLClient* curlClient, CURLcode retCode)
			: ThreadRequest(thpool), _curlClient(curlClient),_retCode(retCode)
		{
		}

	protected:
		CURLClient* _curlClient; 
		CURLcode	_retCode;
		virtual int run()
		{
			try {
				if (_curlClient)
					_curlClient->OnTxnCompleted(_retCode);

				return 0;
			}
			catch(...) {}
			return -1;
		}

		void final(int retcode =0, bool bCancelled =false)
		{
			delete this;
		}
	};
//////////////////////////////////////////////////////
//////////Class CurlClientManager/////////////////////////
//////////////////////////////////////////////////////
	class CurlClientManager: public ZQ::common::NativeThread
	{
	public:
		CurlClientManager(void);
		virtual ~CurlClientManager(void);

	public: 
		bool    add(CURLClient* curlClient);
		size_t  size();
	private:
		void onCompleted(CURL * curlHandle, CURLcode code);
	public:
		void wakeup();
		void quit();
	protected: // impl of NativeThread

		bool _bQuit;
		SYS::SingleObject _hWakeupEvent;

		virtual int run(void);
		virtual void final(void);

	protected:
		CURLM*	_multiHandle;

		typedef std::map<CURL*, CURLClient*> CURLClients;
		CURLClients       _curlClients;
		ZQ::common::Mutex _lockClientMap;
	};

CurlClientManager::CurlClientManager(): NativeThread(), _bQuit(false),_multiHandle(NULL)
{
}

CurlClientManager::~CurlClientManager(void) 
{
	try
	{	
		quit();
		if(_multiHandle) 
			curl_multi_cleanup(_multiHandle);
//		printf("~CurlMgr() destory\n");
	}
	catch (...){
	}
}

size_t  CurlClientManager::size()
{
	return _curlClients.size();
}

bool CurlClientManager::add(CURLClient* curlClient)
{
	bool bRet = false;
	try
	{	
		if(_multiHandle) 
		{	
			CURL* curl = curlClient->getCurl();

			if(curl && CURLM_OK == curl_multi_add_handle(_multiHandle, curl))
			{
				ZQ::common::MutexGuard guard(_lockClientMap);
				MAPSET(CURLClients, _curlClients, curl, curlClient);
				bRet = true;

				if(_curlClients.size() == 1)
					wakeup();
			}
		}
	}
	catch (...){
	}
	return bRet;
}

void CurlClientManager::wakeup()
{
	_hWakeupEvent.signal();
}

void CurlClientManager::quit()
{
	_bQuit = true;
	wakeup();
}

void CurlClientManager::final(void)
{
	{
		ZQ::common::MutexGuard guard(_lockClientMap);
		_curlClients.clear();
	}

	quit();
}

int  CurlClientManager::run()
{
	_multiHandle = curl_multi_init();
	if(!_multiHandle)
		return 0;

	int nextSleep = 10*1000;
	while(!_bQuit)
	{
		while(_curlClients.size() <=0 && !_bQuit)
		{
			_hWakeupEvent.wait(nextSleep);
		}

		if(_bQuit)
			break;

		nextSleep = 10*1000;

		int still_running = 1;
		while(still_running && _curlClients.size() > 0)
		{
			if (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(_multiHandle, &still_running))
				continue;

			if(still_running <= 0)
			{
				int Q = 0;
				CURLMsg *msg;
				while ((msg = curl_multi_info_read(_multiHandle, &Q))) 
				{
					if (msg->msg == CURLMSG_DONE) {
						CURL *e = msg->easy_handle;
						/*curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
						fprintf(stderr, "R: %d - %s <%s>\n",
						msg->data.result, curl_easy_strerror(msg->data.result), url);*/
						onCompleted(e, msg->data.result);
						//printf("completed:\n");
						curl_multi_remove_handle(_multiHandle, e);
						//curl_easy_cleanup(e);
					}
					else {
						printf("E: CURLMsg (%d)\n", msg->msg);
					}
				}
				break;
			}

			long curl_timeo = -1;
			struct timeval timeout;
			int rc; /* select() return code */

			fd_set fdread;
			fd_set fdwrite;
			fd_set fdexcep;
			int maxfd;

			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcep);

			/* set a suitable timeout to play around with */
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			curl_multi_timeout(_multiHandle, &curl_timeo);
			if(curl_timeo >= 0) {
				timeout.tv_sec = curl_timeo / 1000;
				if(timeout.tv_sec > 1)
					timeout.tv_sec = 1;
				else
					timeout.tv_usec = (curl_timeo % 1000) * 1000;
			}

			/* get file descriptors from the transfers */
			curl_multi_fdset(_multiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);

			rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep,&timeout);
			if(rc < 0)
			{
				/* select error */
				still_running = 0;
//				printf("select() returns error, this is badness\n");
				quit();
			}

			int Q = 0;
			CURLMsg *msg;
			while ((msg = curl_multi_info_read(_multiHandle, &Q))) 
			{
				if (msg->msg == CURLMSG_DONE) {
					CURL *e = msg->easy_handle;
					onCompleted(e, msg->data.result);
					curl_multi_remove_handle(_multiHandle, e);
				}
				else {
					printf("CURLMsg (%d)\n", msg->msg);
				}
			}
		}

		_hWakeupEvent.wait(nextSleep);
	}

	return 0;
}

void CurlClientManager::onCompleted(CURL * curl, CURLcode code)
{
	ZQ::common::MutexGuard guard(_lockClientMap);
	CURLClients::iterator  itorClient = _curlClients.find(curl);
	if(itorClient != _curlClients.end())
	{
		try {
			(new CURLClientCompleteCmd(itorClient->second->getThreadPool(), itorClient->second, code))->start();
		}
		catch (...) 
		{}
		_curlClients.erase(curl);
	}
}

//////////////////////////////////////////////////
//////////Class CURLClient///////////////////////
/////////////////////////////////////////////////

CurlClientManager gCurlClientMgr;

#include <openssl/ssl.h>
ZQ::common::Mutex g_opensslLocker;

void openssl_locking_callback(int mode, int type, const char *file,	int line)
{
	if (mode & CRYPTO_LOCK)
	{
		g_opensslLocker.enter();
	}
	else
	{
		g_opensslLocker.leave();
	}
}

unsigned long openssl_thread_id(void)
{
	unsigned long ret;
#ifdef ZQ_OS_MSWIN
	ret = GetCurrentThreadId();
#else
	ret=(unsigned long)pthread_self();
#endif
	return(ret);
}


void CURLClient::startCurlClientManager()
{
	CRYPTO_set_id_callback(openssl_thread_id);
	CRYPTO_set_locking_callback(openssl_locking_callback);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	gCurlClientMgr.start();
}

void CURLClient::stopCurlClientManager()
{
	gCurlClientMgr.quit();
	gCurlClientMgr.waitHandle(-1);
	curl_global_cleanup();
}
size_t CURLClient::getCURLClientSize()
{
    return gCurlClientMgr.size();
}
#define CLOG  (_log)
// #define CURLFMT( _X) CLOGFMT(CURLClient, "client[%s]so[%x,%s] " _X), _clientId.c_str(), _fd, _strDesc.c_str()
#define CURLFMT( _X) CLOGFMT(CURLClient, "client[%s]so[%s] " _X), _clientId.c_str(), _strDesc.c_str()

std::string CURLClient::_fromInst;

CURLClient::CURLClient(char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, uint32 flag, HTTPMETHOD method, char* bindIp, std::string clientId)
:_curl(NULL), _fd(0), _chunk(NULL), _url(url), _log(log), _flags(flag), _thrdpool(thrdpool),_clientId(clientId), 
_bindIp(bindIp), _method(method), _connectTimeout(5000) ,_timeout(20000), _stampTransferTimeout(0),
_reqOffset(0), _respOffset(0)
{
	if(_clientId.empty())
	{
		// un-assigned user's session id, generate one
		char buf[80];
		ZQ::common::Guid guid;
		guid.create();
		guid.toCompactIdstr(buf, sizeof(buf) -2);
		_clientId = buf;
	}

	if (_fromInst.empty())
	{
		ZQ::common::MutexGuard g(g_opensslLocker); // borrow g_opensslLocker
		if (_fromInst.empty())
		{
			char buf[256], *p = buf;
#ifdef ZQ_OS_LINUX
			p += snprintf(buf, sizeof(buf)-2, "p%d@", getpid());
#endif
			gethostname(p, buf + sizeof(buf)-2 -p); buf[sizeof(buf)-2] ='\0';
			_fromInst = buf;
		}
	}

    memset(_cbErrBuf, 0, sizeof(_cbErrBuf));
	_strResStatus = "";
	_resend = false;
	CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("created curl client with url[%s] localbind[%s]"), _url.c_str(), _bindIp.c_str());
}

CURLClient::~CURLClient(void)
{
	try
	{	
		if(_curl)
		{
			if (sTraceAny & _flags)
				CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("~CURLClient() destoryed (%d)"), SYS::getLastErr());
			curl_easy_cleanup(_curl);
			if (CURL_SOCKET_ERROR == SYS::getLastErr())
			{
				if (_fd)
				{
					SYS::sleep(10);
#ifdef ZQ_OS_LINUX
					close(_fd);
#else
					closesocket(_fd);	
#endif//ZQ_OS
				}
			}
			/* free the custom headers */
			if(_chunk)
				curl_slist_free_all(_chunk);
			_chunk = NULL;
			//	printf("~CURLClient() destory\n");
		}
	}
	catch (...)
	{
	}
}

void CURLClient::setTimeout(uint connectTimeout, uint timeout)
{  
	_connectTimeout = connectTimeout;
	_timeout =  timeout;
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("set connectTimeout[%u]ms timeout[%u]ms"), _connectTimeout, _timeout);
}

#define SET_LAST_ERR(_ERR)   if (_ERR >CURLE_OK) { _lastErrCode = _ERR; }

void CURLClient::OnTxnCompleted(CURLcode code)
{
	_stampTransferTimeout =0; // reset the timeout
	SET_LAST_ERR(code);
	std::string strResStatus;
	int stauscode = getStatusCode(strResStatus);
	CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnTxnCompleted[%s] ResponseStatus[%d : %s]ErrorCB[%s]"),
		errorStr(code).c_str(), stauscode, strResStatus.c_str(), _cbErrBuf);
}

std::string CURLClient::errorStr(int err)
{
// thread unsafe
//	static ZQ::common::Mutex _glocker;
//	ZQ::common::MutexGuard g(_glocker);
//		_lastErrCode = err;
	char buf[512]="";

	if (err>0)
		snprintf(buf, sizeof(buf)-2, "err(%d) %s", err, curl_easy_strerror((CURLcode)err));

	return buf;
}

#define SETOPT_ASSERT_ERR(_BY, _OPT, _VAL)  if (CURLE_OK != (_lastErrCode = curl_easy_setopt(_curl, _OPT, _VAL))) { \
	CLOG(ZQ::common::Log::L_ERROR, CURLFMT(#_BY "() setopt(" #_OPT ") failed: %s"), errorStr(_lastErrCode).c_str()); \
	return false; }

bool CURLClient::init()
{
	try
	{
		if(NULL == _curl)
			_curl = curl_easy_init();
		else  curl_easy_reset(_curl);

		_lastErrCode = CURLE_OK;
       /// set Http Request Type
		if (_curl == NULL)
		{
			CLOG(ZQ::common::Log::L_INFO, CURLFMT("Failed to create CURL connection"));
			return false;
		}

/*
SETOPT_ASSERT_ERR(init, CURLOPT_HTTPGET, 0L);
			SETOPT_ASSERT_ERR(init, CURLOPT_POST, 0L);
			SETOPT_ASSERT_ERR(init, CURLOPT_UPLOAD, 0L);
			SETOPT_ASSERT_ERR(init, CURLOPT_PUT, 0L);
			SETOPT_ASSERT_ERR(init, CURLOPT_CUSTOMREQUEST, 0L);
*/

		if(_method == HTTP_GET)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_HTTPGET, 1L);
		}
		else if(_method == HTTP_POST)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_POST, 1L);
		}
		else if(_method == HTTP_PUT)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_UPLOAD, 1L);
			SETOPT_ASSERT_ERR(init, CURLOPT_PUT, 1L);
		}
		else if(_method == HTTP_DEL)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_CUSTOMREQUEST, "DELETE");
		}
		else if(_method == HTTP_HEAD)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_CUSTOMREQUEST, "HEAD");
		}
		else
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("unkonwn http request (%d)"), _method);
			return false;
		}
		/// End set Http Request Type

		SETOPT_ASSERT_ERR(init, CURLOPT_NOSIGNAL , 1);

		/// set http error cbbuf
		SETOPT_ASSERT_ERR(init, CURLOPT_ERRORBUFFER, _cbErrBuf); 

		/// set http url
		SETOPT_ASSERT_ERR(init, CURLOPT_URL, (char*)_url.c_str());

		SETOPT_ASSERT_ERR(init, CURLOPT_FOLLOWLOCATION, 1);

		SETOPT_ASSERT_ERR(init, CURLOPT_TIMEOUT_MS, (long)_timeout);

		SETOPT_ASSERT_ERR(init, CURLOPT_CONNECTTIMEOUT_MS, (long)_connectTimeout);

		if(_flags & sfEnableOutgoingDataCB)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_READFUNCTION, cbReadData);
			SETOPT_ASSERT_ERR(init, CURLOPT_READDATA, this);
		}

		SETOPT_ASSERT_ERR(init, CURLOPT_WRITEFUNCTION, cbReceived);
		SETOPT_ASSERT_ERR(init, CURLOPT_WRITEDATA, this);

//		SETOPT_ASSERT_ERR(init, CURLOPT_OPENSOCKETFUNCTION, cbOpenSocket);
//		SETOPT_ASSERT_ERR(init, CURLOPT_OPENSOCKETDATA, this);
		SETOPT_ASSERT_ERR(init, CURLOPT_SOCKOPTFUNCTION , cbSockopt);
		SETOPT_ASSERT_ERR(init, CURLOPT_SOCKOPTDATA, this);

/*	    code = curl_easy_setopt(_curl, CURLOPT_CLOSESOCKETFUNCTION, cbSocketClosed);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_CLOSESOCKETFUNCTION: %s"), lastError(code).c_str());
			return false;
		}

		code = curl_easy_setopt(_curl, CURLOPT_CLOSESOCKETDATA, this);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_CLOSESOCKETDATA: %s"), lastError(code).c_str());
			return false;
		}*/

		if(_flags & sfEnableProgressCB)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_NOPROGRESS, 0L);
			SETOPT_ASSERT_ERR(init, CURLOPT_PROGRESSFUNCTION, cbProgress);
			SETOPT_ASSERT_ERR(init, CURLOPT_PROGRESSDATA, this);
		}
		
		SETOPT_ASSERT_ERR(init, CURLOPT_HEADERFUNCTION, cbReadHeader); 
		SETOPT_ASSERT_ERR(init, CURLOPT_HEADERDATA, this);

		if (sTraceAny & _flags)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_DEBUGFUNCTION, cbDebug);
			SETOPT_ASSERT_ERR(init, CURLOPT_DEBUGDATA, this);
			SETOPT_ASSERT_ERR(init, CURLOPT_VERBOSE, 1L);
		}

		if(_flags & sfEnableChunkdPost)
		{
			SETOPT_ASSERT_ERR(init, CURLOPT_CHUNK_BGN_FUNCTION, cbChunkBegin);
			SETOPT_ASSERT_ERR(init, CURLOPT_CHUNK_DATA, this);
			SETOPT_ASSERT_ERR(init, CURLOPT_CHUNK_END_FUNCTION, cbChunkEnd);
		}

		SETOPT_ASSERT_ERR(init, CURLOPT_SSL_VERIFYPEER, 0L);
		SETOPT_ASSERT_ERR(init, CURLOPT_SSL_VERIFYHOST, 0L);

 /*		if(_flags &sfEnableSSL)
		{

			code = curl_easy_setopt(_curl, CURLOPT_SSL_CTX_FUNCTION , cbSSLctx);
			if(code != CURLE_OK)
			{
				CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_SSL_CTX_FUNCTION: %s"), lastError(code).c_str());
				return false;
			}
			code = curl_easy_setopt(_curl, CURLOPT_SSL_CTX_DATA, this);
			if(code != CURLE_OK)
			{
				CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_SSL_CTX_DATA: %s"), lastError(code).c_str());
				return false;
			}
		}

		code = curl_easy_setopt(_curl, CURLOPT_FNMATCH_FUNCTION, cbFNMatch);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_FNMATCH_FUNCTION: %s"), lastError(code).c_str());
			return false;
		}
		code = curl_easy_setopt(_curl, CURLOPT_FNMATCH_DATA, this);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_FNMATCH_DATA: %s"), lastError(code).c_str());
			return false;
		}
		code = curl_easy_setopt(_curl, CURLOPT_SSL_CTX_FUNCTION, cbSSLctx);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_SSL_CTX_FUNCTION: %s"), lastError(code).c_str());
			return false;
		}
		code = curl_easy_setopt(_curl, CURLOPT_SSL_CTX_DATA, this);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_SSL_CTX_DATA: %s"), lastError(code).c_str());
			return false;
		}

		code = curl_easy_setopt(_curl, CURLOPT_IOCTLFUNCTION, cbIOCtl);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_IOCTLFUNCTION: %s"), lastError(code).c_str());
			return false;
		}
		code = curl_easy_setopt(_curl, CURLOPT_IOCTLDATA, this);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_IOCTLDATA: %s"), lastError(code).c_str());
			return false;
		}
		code = curl_easy_setopt(_curl, CURLOPT_SEEKFUNCTION, cbSeek);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_SEEKFUNCTION: %s"), lastError(code).c_str());
			return false;
		}
		code = curl_easy_setopt(_curl, CURLOPT_SEEKDATA, this);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to set CURLOPT_SEEKDATA: %s"), lastError(code).c_str());
			return false;
		}

	    code = curl_easy_setopt(_curl, CURLOPT_INTERLEAVEFUNCTION, cbInterleaver);
		printf("CURLOPT_INTERLEAVEFUNCTION (%d)\n", code);
		code = curl_easy_setopt(_curl, CURLOPT_INTERLEAVEDATA, this);
		printf("CURLOPT_INTERLEAVEDATA (%d)\n", code);
*/
	}
	catch (...)
	{
		CLOG(ZQ::common::Log::L_WARNING, CURLFMT("initialize curl client failed"));
		return false;
	}

	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("initialized curl client successfully"));

	return true;
}

bool CURLClient::setLocalIpPort(char* bindIp)
{
   _bindIp = bindIp;
 //  _bindPort = port;

   return true;
}

CURL* CURLClient::getCurl() const
{
	return _curl;
}

//the error message when transfer occur error
std::string CURLClient::getErrorMessage() const
{
	return (std::string)(_cbErrBuf);
}

ZQ::common::NativeThreadPool& CURLClient::getThreadPool() const
{ 
	return _thrdpool;
}

std::string CURLClient::getClientSessionId() const
{
    return _clientId;
}

int CURLClient::setRequestBody(const ZQ::common::BufferList::Ptr pReqBody)
{
	int totLen = 0;
	if (_pReqBodyBuf || !pReqBody) // reject to overwrite the existing request
		return -1;

	_pReqBodyBuf = pReqBody;
	_reqOffset =0;
	size_t len = _pReqBodyBuf->length();

	if (_curl && _method == HTTP_POST && _pReqBodyBuf)
	{
		// SETOPT_ASSERT_ERR(setReqData, CURLOPT_POSTFIELDS, _pReqBody);
		SETOPT_ASSERT_ERR(setReqData, CURLOPT_POSTFIELDSIZE, (curl_off_t)len);
	}
	else if(_curl && _method == HTTP_PUT)
	{
		SETOPT_ASSERT_ERR(setReqData, CURLOPT_INFILESIZE_LARGE, (curl_off_t) len);
	}

	return len;
}

int CURLClient::setResponseBody(const ZQ::common::BufferList::Ptr pRespBody)
{
	int bufSz = 0;
	if (_pRespBodyBuf || !pRespBody) // reject to overwrite the existing buffer
		return -1;

	_pRespBodyBuf = pRespBody;
	_respOffset =0;
	return _pReqBodyBuf->size();
}

ZQ::common::BufferList::Ptr CURLClient::getResponseBody()
{
	return _pRespBodyBuf;
}

bool CURLClient::sendRequest(CURLcode& errorCode, bool bSync)
{	
	bool bRet = true;
	errorCode = CURLE_OK;
	_cbErrBuf[0] = _cbErrBuf[1] = '\0';
	if (!_userAgent.empty())
		setHeader("User-Agent",_userAgent.c_str());
	if (!_fromInst.empty())
		setHeader("From",_fromInst.c_str());

	if(!_curl)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("send request failed with NULL curl handle"));
		return false;
	}

	if (_flags & sfEnableChunkdPost)
	{
		if (_headers.end() == _headers.find("x-aqua-file-truncate")) // eliminates truncate from "chunked"
		{
			setHeader("Transfer-Encoding", "chunked");
			setHeader("Content-Length", "");
		}
	}

	if (!_resend)
	{
		std::map<std::string, std::string>::iterator itorHeader;
		for(itorHeader = _headers.begin(); itorHeader != _headers.end(); itorHeader++)
		{
			char strBuf[1024] = "";
			_snprintf(strBuf, sizeof(strBuf)-1, "%s: %s",itorHeader->first.c_str(), itorHeader->second.c_str());
			_chunk = curl_slist_append(_chunk, strBuf);
		}

		_resend = true;

		if(_chunk)
		{
			errorCode = curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _chunk);
			if(errorCode !=  CURLE_OK)
			{
				CLOG(ZQ::common::Log::L_ERROR, CURLFMT("sendRequest() failed to set CURLOPT_HTTPHEADER: %s"), errorStr(errorCode).c_str());
				return false;
			}
		}	
	}

	SETOPT_ASSERT_ERR(init, CURLOPT_URL, (char*)_url.c_str());

	if(!bSync)
	{
		bRet =  gCurlClientMgr.add(this);	
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("request sent"));
	}
	else
	{
		CURLcode code =  curl_easy_perform(_curl);
		if(code != CURLE_OK)
		{
			CLOG(ZQ::common::Log::L_ERROR, CURLFMT("request error: %s"), errorStr(code).c_str());
			bRet =  false;
			errorCode = code;
		}	
		else
		{
			char* bufLocalIp = NULL;
			long localPort = 0;
			curl_easy_getinfo(_curl, CURLINFO_LOCAL_IP,&bufLocalIp);
			curl_easy_getinfo(_curl, CURLINFO_LOCAL_PORT,&localPort);

			if(bufLocalIp != NULL)
				CLOG(ZQ::common::Log::L_INFO, CURLFMT("bind localIpPort[%s:%d]"), bufLocalIp, (int)localPort);
		}
	}

	return bRet;
}

int CURLClient::setHeader(const std::string& key, const std::string& strValue)
{
	if(key.empty())
	{
		_headers.clear();
		return 0;
	}

//	CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("set Header [%s] = [%s]"), key.c_str(), strValue.c_str());

	if(_headers.find(key) != _headers.end())
		_headers[key] =  strValue;
	else
		_headers.insert(StringMap::value_type(key, strValue));
	return (int)_headers.size();
}

StringMap CURLClient::getResponseHeaders()const
{
	return _respHeaders;
}

long  CURLClient::getStatusCode(std::string& strStatus)
{
	CURLcode code;
	if(!_curl)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("getStatusCode() NULL curl handle"));
       return -1;
	}

	long retcode = -1;
	code = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE , &retcode); 
	if ( (code != CURLE_OK))
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to get CURLINFO_RESPONSE_CODE: %s"), errorStr(code).c_str());
        retcode = -1;
	}
    strStatus = _strResStatus;
	return retcode;
}

int CURLClient::OnPrepareOutgoingData(void* bufFlushTo, size_t size)
{
	if (NULL == bufFlushTo || size < 1 || !_pReqBodyBuf)
		return 0;

	if (_timeout >0  && (HTTP_PUT == _method || HTTP_POST == _method))
	{
		int64 stampNow = ZQ::common::now();
		if (_stampTransferTimeout <=0 )
			_stampTransferTimeout = stampNow + _timeout;
		else if (stampNow > _stampTransferTimeout)
			return CURL_READFUNC_ABORT; // terminate the PUT/POST if timed out
	}

	size_t nBytes = _pReqBodyBuf->read((uint8*)bufFlushTo, _reqOffset, size);
	_reqOffset += nBytes;

	if ((sTraceAny & _flags) && (size != nBytes))
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnPrepareOutgoingData() partial buffer filled: size %d, processed %d, newoffset %d"), size, nBytes, _reqOffset);

	return nBytes;
}

int  CURLClient::OnDataArrived(char *data, size_t size)
{
	if (_timeout >0  && (HTTP_GET == _method))
	{
		int64 stampNow = ZQ::common::now();
		if (_stampTransferTimeout <=0 )
			_stampTransferTimeout = stampNow + _timeout;
		else if (stampNow > _stampTransferTimeout)
			return 0; // terminate the GET if timed out by returning a number other than size
	}

	if (!_pRespBodyBuf)
		return 0;

	size_t nBytes = _pRespBodyBuf->fill((uint8*)data, _respOffset, size);
	if (size != nBytes)
		CLOG(ZQ::common::Log::L_WARNING, CURLFMT("OnDataArrived() received data %d, filled into [%d +%d], size mismatched"), size, _respOffset, nBytes);
	else if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnDataArrived() received data %d, filled into [%d +%d]"), size, _respOffset, nBytes);

	_respOffset += nBytes;

	return nBytes;
}

curl_socket_t  CURLClient::OnOpenSocket(curlsocktype purpose, struct curl_sockaddr *address)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnOpenSocket() family:%d, socktype:%d, protocol:%d"), address->family, address->socktype, address->protocol);

	_fd = socket(address->family, address->socktype, address->protocol);
	
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	if( 0 == getsockname(_fd, (struct sockaddr *)&addr, &len))
	{
		char buf[40];
		snprintf(buf, sizeof(buf)-2, "%s/%d",ZQ::common::InetHostAddress(addr.sin_addr).getHostAddress(), ntohs(addr.sin_port));
		_strDesc = buf;
	}
	
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnOpenSocket() so[%s] opened"), _strDesc.c_str());
	return  _fd;
}

int  CURLClient::OnSockopt(curl_socket_t curlfd, curlsocktype purpose)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnSockopt() Socket[%x] SocketType[%s] bindIp[%s]"), curlfd, (purpose == CURLSOCKTYPE_IPCXN ? "CURLSOCKTYPE_IPCXN": "CURLSOCKTYPE_ACCEPT"), _bindIp.c_str());

	_fd = (int)curlfd;

	int64 stampNow = ZQ::common::now(), stampLast = stampNow;

	struct linger linger;

	if (true)
	{
		linger.l_onoff = 1;
		linger.l_linger = 0; // 5;
	}
	else
		linger.l_onoff = linger.l_linger = 0;

	int nret = setsockopt(_fd, SOL_SOCKET, SO_LINGER, (char *)&linger,	(socklen_t)sizeof(linger));
#ifdef  ZQ_OS_MSWIN
	if (nret != 0)
	{
		CLOG(ZQ::common::Log::L_WARNING, CURLFMT("failed to set sockopt linger errorcode[%d]"), WSAGetLastError());
	}
#else
	if (nret != 0 && EINPROGRESS != nret)
	{
		if (errno && EINPROGRESS != errno)
			CLOG(ZQ::common::Log::L_WARNING, CURLFMT("failed to set sockopt linger ret[%d] errorcode[%d]"), nret, errno);
	}
#endif

	stampNow = ZQ::common::now();
	int durLinger = (int) (stampNow - stampLast);
	stampLast = stampNow;

	unsigned int reuseaddr = 1; 

	nret = setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseaddr, sizeof(reuseaddr));
#ifdef  ZQ_OS_MSWIN
	if(nret != 0)
	{
		CLOG(ZQ::common::Log::L_WARNING, CURLFMT("failed to set sockopt reuseaddr errorcode[%d]"), WSAGetLastError());
	}
#else
	if (nret != 0 && EINPROGRESS != nret)
	{
		if (errno && EINPROGRESS != errno)
			CLOG(ZQ::common::Log::L_WARNING, CURLFMT("failed to set sockopt reuseaddr ret[%d] errorcode[%d]"), nret, errno);
	}
#endif

	// disable TCP nagle
	char disableNagle=1;
	setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &disableNagle, sizeof(char)); 

	stampNow = ZQ::common::now();
	int durReuse = (int) (stampNow - stampLast);
	stampLast = stampNow;

	if (!_bindIp.empty() && 0!=_bindIp.compare("0.0.0.0"))
	{
		InetHostAddress bindaddr(_bindIp.c_str());

		if(!bindaddr.isAnyAddress())
		{
			if (sTraceAny & _flags)
				CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("binding[%s]"), _bindIp.c_str());

			// bind address
			InetAddress::inetaddr_t ia = bindaddr.getAddress(); 
			sockaddr sa;
			memset(&sa, 0x00, sizeof(sa));
			sa.sa_family    = ia.family;

			sockaddr_in* psad= (sockaddr_in*) &sa.sa_data;
			psad->sin_family = ia.family;
			psad->sin_addr   = ia.addr.a;
			psad->sin_port    = htons(0);

			//int rc = bind( _fd, &sa, sizeof(sa));
			int rc = bind( _fd, (sockaddr*)psad, sizeof(*psad));
			//		freeaddrinfo(pAddrInfo);

			if (0 != rc)
			{
				CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to bind[%s] with error code[%d,%s]"), _bindIp.c_str(), rc, gai_strerror(rc));
				return 1;
			}
		}
	}

	stampNow = ZQ::common::now();
	int durBind = (int) (stampNow - stampLast);
	stampLast = stampNow;

	_strDesc = "";
	struct sockaddr_in sin; 
	socklen_t len = sizeof(sin);

	if (getsockname(_fd, (struct sockaddr *)&sin, &len) != -1)
	{
		char buf[40];
		snprintf(buf, sizeof(buf)-2, "%s/%d", ZQ::common::InetHostAddress(sin.sin_addr).getHostAddress(), ntohs(sin.sin_port));
		_strDesc = buf;
	}

	stampNow = ZQ::common::now();
	int durSockname = (int) (stampNow - stampLast);
	stampLast = stampNow;

	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnSockopt() bind local[%s] successful, duration[%d,%d,%d,%d]"), _strDesc.c_str(), durLinger, durReuse, durBind, durSockname);

	return 0;
}

curl_socket_t  CURLClient::OnCloseSocket(curl_socket_t item)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnCloseSocket() so[%d]"),item);

	return 1;
}

int  CURLClient::OnProgress(double dltotal, double dlnow, double ultotal, double ulnow)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnProgress: dltotal:%.2f, dlnow:%.2f, ultotal:%.2f, ulnow:%.2f"), dltotal, dlnow, ultotal, ulnow);
	return 0;
}

int  CURLClient::OnHeader(void *data, size_t size)
{
	std::string header;

	header.append((char*)data, size - 2);
	// unnecessary logging as OnDebug has covered CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("Response Header: %s"),header.c_str());

	int npos = header.find(':');
	if(npos > 0)
	{
		std::string strKey = header.substr(0, npos);
		std::string strValue = header.substr(npos + 1);
		npos = strValue.find_first_not_of(" ");
		if (npos > 0)
			strValue = strValue.substr(npos);
		MAPSET(StringMap, _respHeaders, strKey, strValue);
	}
	else 
	{
		if(header.find("HTTP") != std::string::npos)
			_strResStatus =  header;
	}

	return (int)size;
}

long CURLClient::OnChunkBegin(const void *transfer_info, int remains)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("OnChunkBegin remains(%d)"), remains);

	return remains;
}

long CURLClient::OnChunkEnd()
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnChunkEnd"));
	return 0;
}
int  CURLClient::OnFNMatch(const char * strPattern, const char * strMatch)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnFNMatch"));
	return 0;
}

int CURLClient::OnDebug(CURL* curl, curl_infotype infoType, unsigned char *data, size_t size) 
{
//	CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnDebug"));
	const char *text = "";
	std::string strDebug;
	bool bNeed2Dump =false;
	ZQ::common::Log::loglevel_t llevel = (ZQ::common::Log::loglevel_t)CLOG.getVerbosity();

	if (0 == (sTraceAny & _flags))
		return (int)size;
   
	switch (infoType) 
	{
	case CURLINFO_TEXT:
		if (0 == (sfTraceInfo & _flags))
			return 0;

		if(size > 2)
			size = size - 1;
		strDebug.append((char*)data, size);
		CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("curltrac: %s"), strDebug.c_str());
		return 0;
	
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		if (llevel == ZQ::common::Log::L_DEBUG)
			bNeed2Dump = true;
		text = "curlheadsent";

		break;
	case CURLINFO_DATA_OUT:
		if (0 == (sfTraceSend & _flags))
			return 0;

		if (size<100 && llevel == ZQ::common::Log::L_DEBUG)
			bNeed2Dump = true;

		text = "curlsent";
		break;

	case CURLINFO_HEADER_IN:
//		if (llevel == ZQ::common::Log::L_DEBUG)
			bNeed2Dump = true;
 //        text = "curlheadrecv";

		 break;
	case CURLINFO_DATA_IN:
		if (0 == (sfTraceRecv & _flags))
			return 0;
		text = "curlrecv";
		break;
	}

	if (bNeed2Dump)
		dump(text, data, size, true);
	else
		CLOG(llevel, CURLFMT("%s: %d bytes data"), text, size);

	return (int)size;
}

void CURLClient::dump(const char *text, unsigned char *ptr, size_t size, bool nohex)
{
	size_t i;
	size_t c;

	unsigned int width=0x10;

	if(nohex)
		// without the hex output, we can fit more on screen
		width = 0x40;

	char buf[4096] = "";
	std::string strInfo;

	snprintf(buf, sizeof(buf)-1,  "%s(%u): ", text, size);
	strInfo += std::string(buf);
	memset(buf, sizeof(buf), 0);

	for(i=0; i<size; i+= width)
	{
		if(!nohex) {
			// hex not disabled, show it
			for(c = 0; c < width; c++)
				if(i+c < size)
				{
					snprintf(buf, sizeof(buf)-1, "%02x ", ptr[i+c]);
					strInfo += std::string(buf);
					memset(buf, sizeof(buf), 0);
				}
				else
				{
					strInfo += "   ";
				}
		}

		for(c = 0; (c < width) && (i+c < size); c++) 
		{
			// check for 0D0A; if found, skip past and start a new line of output
			if (nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A)
			{
				i+=(c+2-width);
				break;
			}
			snprintf(buf, sizeof(buf)-1, "%c",(ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
			strInfo += std::string(buf);
			memset(buf, sizeof(buf), 0);
			// check again for 0D0A, to avoid an extra \n if it's at width
			if (nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A)
			{
				i+=(c+3-width);
				strInfo += "..";
				break;
			}
		}
	}

    CLOG(ZQ::common::Log::L_DEBUG, CURLFMT("%s"), strInfo.c_str());
}

CURLcode CURLClient::OnSSLctx(CURL *curl, void *sslctx)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnSSLctx"));
	return CURLE_OK;
}

curlioerr CURLClient::OnIOCtl(CURL *handle, int cmd)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnIOCtl"));
	return CURLIOE_OK;
}

int CURLClient::OnSeek(curl_off_t offset, int origin)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnSeek"));

	return (int)offset;
}

size_t CURLClient::OnInterleaver(void *ptr, size_t size)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnInterleaver"));
	
	return size;
}
int CURLClient::cbReceived(char *data, size_t size, size_t nmemb, void* pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	size_t sz = size * nmemb;

	if (NULL == pClient)
		return (int)sz;

	return pClient->OnDataArrived(data, sz);
}

curl_socket_t CURLClient::cbOpenSocket(void *pCtx, curlsocktype purpose, struct curl_sockaddr *address)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return 1;
	return pClient->OnOpenSocket(purpose, address);
}

int CURLClient::cbSocketClosed(void *pCtx, curl_socket_t item)
{
//	int nret = closesocket(item);
//	printf("cbSocketClosed[%x] [%d]\n", item, nret);
	return 0;
}

int CURLClient::cbProgress(void *pCtx, double dltotal, double dlnow, double ultotal, double ulnow)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return 0;
	return pClient->OnProgress(dltotal, dlnow, ultotal, ulnow);
}

int CURLClient::cbReadHeader(char *data, size_t size, size_t nmemb, void* pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return 0;
	size_t sizes = size * nmemb;
	return pClient->OnHeader(data, sizes);
}

long CURLClient::cbChunkBegin(const void *transfer_info, void *pCtx, int remains)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return remains;
	return pClient->OnChunkBegin(transfer_info, remains);
}

long CURLClient::cbChunkEnd(void *pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return 0;
	return pClient->OnChunkEnd();
}

int  CURLClient::cbFNMatch(void *pCtx, const char* strPattern, const char* strMatch)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return 0;
	return pClient->OnFNMatch(strPattern, strMatch);
}

int  CURLClient::cbDebug(CURL *curl, curl_infotype infoType,unsigned char *data, size_t size, void *pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return (int)size;

	return pClient->OnDebug(curl, infoType, data, size);
}

CURLcode CURLClient::cbSSLctx(CURL *curl, void *sslctx, void *pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return CURLE_UNSUPPORTED_PROTOCOL;
	return pClient->OnSSLctx(curl, sslctx);
}

curlioerr CURLClient::cbIOCtl(CURL *handle, int cmd, void *pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return CURLIOE_UNKNOWNCMD;
	return pClient->OnIOCtl(handle, cmd);
}

int CURLClient::cbSeek(void *pCtx, curl_off_t offset, int origin)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return origin;
	return pClient->OnSeek(offset, origin);
}

int CURLClient::cbSockopt(void *pCtx, curl_socket_t curlfd, curlsocktype purpose)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return 1;
	return pClient->OnSockopt(curlfd, purpose);
}

size_t CURLClient::cbInterleaver(void *ptr, size_t size, size_t nmemb, void *pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return size*nmemb;
	return pClient->OnInterleaver(ptr, size*nmemb);
}

int CURLClient::cbReadData(char *data, size_t size, size_t nmemb, void* pCtx)
{
	CURLClient* pClient = (CURLClient*) pCtx;
	if (NULL == pClient)
		return 0;
	return pClient->OnPrepareOutgoingData(data, size*nmemb);
}

bool CURLClient::getWorkingURL(std::string& url, long& reDirectCount)
{
	if(NULL == _curl)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("NULL curl handle"));
		return false;
	}

	char* strUrl;
	long code = 0;
	code = curl_easy_getinfo(_curl, CURLINFO_EFFECTIVE_URL , &strUrl); 
	if (code != CURLE_OK)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to get CURLINFO_EFFECTIVE_URL with error: %s"),  errorStr(code).c_str());
		return false;
	}

	url = strUrl;

	code = curl_easy_getinfo(_curl, CURLINFO_REDIRECT_COUNT , &reDirectCount); 
	if (code != CURLE_OK)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to get CURLINFO_REDIRECT_COUNT with error: %s"),  errorStr(code).c_str());
		return false;
	}

	return true;
}

void CURLClient::resetURL(const std::string& newURL, HTTPMETHOD method, uint32 flags)
{
	if (sTraceAny & _flags)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("changed url from [%s] to [%s]"),  _url.c_str(), newURL.c_str());

	_url = newURL;
	_method = method;
	_flags = flags;

	_headers.clear();
	_respHeaders.clear();/// http response headers
	_strResStatus =""; /// http response status string
	// _strDesc ="";
	memset(_cbErrBuf, 0x00, sizeof(_cbErrBuf)); // libCURL Error call back string

	_pReqBodyBuf =NULL;
	_reqOffset =0;
	_stampTransferTimeout =0;

	// about the buffer to receive incoming response body
	_pRespBodyBuf = NULL;
	_respOffset =0;
/*
	{
		// un-assigned user's session id, generate one
		char buf[80];
		ZQ::common::Guid guid;
		guid.create();
		guid.toCompactIdstr(buf, sizeof(buf) -2);
		_clientId = buf;
	}
*/

	_lastErrCode =0;
	_resend = false;

	if(_chunk)
		curl_slist_free_all(_chunk);
	_chunk = NULL;
}

}}
