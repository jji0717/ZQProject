#ifndef __CURLClient_H__
#define __CURLClient_H__

#include <curl/curl.h>
#include "Log.h"
#include "Pointer.h"
#include "NativeThreadPool.h"
#include "Guid.h"
#include "Locks.h"
#include "SystemUtils.h"
#include "BufferPool.h"

#include <string>
#include <map>
#include <vector>

#ifndef FLAG
#  define FLAG(_BIT) (1 << _BIT)
#endif // FLAG

#ifdef ZQ_OS_MSWIN
#	pragma comment(lib, "wsock32.lib") // VC link wsock32
#	pragma comment(lib, "libcurl_imp.lib")
#	pragma comment(lib, "libeay32.lib")
#endif // ZQ_OS_MSWIN

namespace ZQ
{
namespace common
{
		typedef std::map <std::string, std::string>StringMap;

		class CURLClient: virtual public ZQ::common::SharedObject
		{
		public:
			typedef ZQ::common::Pointer < CURLClient > Ptr;

			typedef enum
			{
				HTTP_GET = 0,
				HTTP_POST,
				HTTP_PUT,
				HTTP_DEL,
				HTTP_HEAD
			}HTTPMETHOD;

			CURLClient(char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, uint32 flag = 0, HTTPMETHOD method = HTTP_GET, char* bindIp = "0.0.0.0", std::string clientId ="");
			virtual ~CURLClient(void);

		public:

			void setTimeout(uint connectTimeout, uint timeout);
			bool init();
            
			bool setLocalIpPort(char* bindIp);
			bool sendRequest(CURLcode& errorCode, bool bSync = false);
			
			int setHeader(const std::string& key, const std::string& strValue);
			StringMap getResponseHeaders() const;
			
			long  getStatusCode(std::string& strStatus); //httpclient response status code;
			std::string getErrorMessage() const; //the error message when transfer occur error

			int setRequestBody(const ZQ::common::BufferList::Ptr reqBody);
			int setResponseBody(ZQ::common::BufferList::Ptr respBody);
            ZQ::common::BufferList::Ptr getResponseBody();

			// this message will duplicate a buf to keep the input request data
			// bool  setReqData(const char* data, size_t size);//request data info.

			// this method directly take the input buffer without memory copying
			// bool  setReqBody(const char* reqBody, size_t size);

			// this method directly take the input buffer without memory copying
			// int  setRespBuf(char* respBody, size_t size);
			// const char* getRespBuf(size_t& size);

			CURL* getCurl() const; // operator CURL& const( return *_curl; );
            
			std::string getClientSessionId() const;
			std::string getDesc() const { return _strDesc; }

			ZQ::common::NativeThreadPool& getThreadPool() const;
			static std::string errorStr(int err=-1);

			bool getWorkingURL(std::string& url, long& reDirectCount);
			void resetURL(const std::string& newURL, HTTPMETHOD method=HTTP_GET, uint32 flags=0);

		protected:
			///This function gets called by libcurl as soon as it needs to read data in order to send it to the peer. 
			///The data area pointed at by the pointer ptr may be filled with at most size multiplied with nmemb number of bytes. 
			///Your function must return the actual number of bytes that you stored in that memory area. Returning 0 
			///will signal end-of-file to the library and cause it to stop the current transfer. 
			virtual int OnPrepareOutgoingData(void *data, size_t size);

			///This function gets called by libcurl as soon as there is data received that needs to be saved. 
			///The size of the data pointed to by ptr is size multiplied with nmemb, it will not be zero terminated.
			///Return the number of bytes actually taken care of. If that amount differs from the amount passed 
			///to your function, it'll signal an error to the library. This will abort the transfer and return CURLE_WRITE_ERROR. 
			virtual int  OnDataArrived(char *data, size_t size);

			///This function gets called by libcurl instead of the socket(2) call. 
			///The callback's purpose argument identifies the exact purpose for this particular socket
			virtual curl_socket_t  OnOpenSocket(curlsocktype purpose, struct curl_sockaddr *address);

			///This function gets called by libcurl after the socket() call but before the connect() call. 
			///The callback's purpose argument identifies the exact purpose for this particular socket: 
			virtual int  OnSockopt(curl_socket_t curlfd, curlsocktype purpose);

			///This function gets called by libcurl instead of the close(3) or closesocket(3) call
			///when sockets are closed (not for any other file descriptors)
			///Return 0 to signal success and 1 if there was an error
			virtual curl_socket_t  OnCloseSocket(curl_socket_t item);

			int  OnDebug(CURL* curl, curl_infotype infoType, unsigned char *data, size_t size); 
			///This function gets called by libcurl instead of its internal equivalent with a frequent interval 
			///during operation (roughly once per second or sooner) no matter 
			///if data is being transferred or not. Unknown/unused argument values passed to the 
			///callback will be set to zero (like if you only download data, the upload size will remain 0). 
			///Returning a non-zero value from this callback will cause libcurl to abort the transfer 
			///and return CURLE_ABORTED_BY_CALLBACK. 
			virtual int  OnProgress(double dltotal, double dlnow, double ultotal, double ulnow);

            ///This function gets called by libcurl as soon as it has received header data.
			///The header callback will be called once for each header and only complete header lines are passed on to the callback
			virtual int  OnHeader(void *data, size_t size);
			
			// the following are uncertain callbacks that available at cURL
			virtual long OnChunkBegin(const void* transfer_info, int remains);
			virtual long OnChunkEnd();
			
			virtual int  OnFNMatch(const char * strPattern, const char * strMatch);
			virtual CURLcode OnSSLctx(CURL *curl, void *sslctx); 
			virtual curlioerr OnIOCtl(CURL *handle, int cmd);
			virtual int OnSeek(curl_off_t offset, int origin); 
			virtual size_t OnInterleaver(void *ptr, size_t size);

		public:
			virtual void OnTxnCompleted(CURLcode code);

		protected:
			std::string _url;   /// connect http url
			StringMap _headers;///http headers
			StringMap _respHeaders;/// http response headers
			std::string _strResStatus; /// http response status string
			std::string _strDesc;

			int  _fd;  ///the socket fd;
			char _cbErrBuf[CURL_ERROR_SIZE]; // libCURL Error call back string
//			std::string _cbErrBuf;

			ZQ::common::BufferList::Ptr _pReqBodyBuf;
			// BufDescList::iterator  _itReqBodyBuf;
			size_t                     _reqOffset;
			// std::string _reqData; //put or post  data
			// const char* _pReqBody;
			// size_t      _reqSize, _reqOffset;// left size for post data
			int64       _stampTransferTimeout;

			// about the buffer to receive incoming response body
			ZQ::common::BufferList::Ptr _pRespBodyBuf;
			size_t                     _respOffset;
			// BufDescList::iterator  _itRespBodyBuf;

//			std::string _reponseBuf;
//			char*		_pRespBuf;
//			size_t      _respBufSize, _respTail;// left size for post data

			std::string _clientId; //client sessionId for printf log
			HTTPMETHOD  _method; /// http send request type: GET PUT POST DELETE

			std::string _bindIp;/// localip by socket bind
//			int         _bindPort;/// localport by socket bind

			uint        _timeout; ///receive data timeout
			uint        _connectTimeout;///timeout for http client connnect to server

			std::string _userAgent;

			struct curl_slist *_chunk ;

			ZQ::common::Log& _log;
			ZQ::common::NativeThreadPool& _thrdpool;

			CURL* _curl;

			int         _lastErrCode;
			uint32		_flags;

		    bool        _resend;
			static std::string _fromInst;

		public:
			typedef enum _CURLFlags
			{
				sfTraceInfo             = FLAG(0),
				sfTraceSend             = FLAG(1),
				sfTraceRecv             = FLAG(2),
				sfEnableProgressCB      = FLAG(4),
				sfEnableOutgoingDataCB  = FLAG(5),
				sfEnableChunkdPost      = FLAG(6),
				sfEnableSSL             = FLAG(7),
			} CURLFlags;
#define sTraceAny   (0xff)

		private:
			void dump(const char *text, unsigned char *ptr, size_t size, bool nohex);

		public:
			static int cbReadData(char *data, size_t size, size_t nmemb, void* pCtx);
			static int cbReceived(char *data, size_t size, size_t nmemb, void* pCtx);
			static curl_socket_t cbOpenSocket(void* pCtx, curlsocktype purpose, struct curl_sockaddr *address);
			static int cbSocket(void* pCtx, curl_socket_t curlfd, curlsocktype purpose);
			static int cbSocketClosed(void* pCtx, curl_socket_t item);
			static int cbProgress(void* pCtx, double dltotal, double dlnow, double ultotal, double ulnow);
			static int cbReadHeader(char *data, size_t size, size_t nmemb, void* pCtx);

			static long cbChunkBegin(const void *transfer_info, void *pCtx, int remains);
			static long cbChunkEnd(void *pCtx);
			static int  cbFNMatch(void *pCtx, const char* strPattern, const char* strMatch);

			///	Pass a pointer to a function that matches the following prototype: 
			///	int curl_debug_callback (CURL *, curl_infotype, char *, size_t, void *); 
			///	CURLOPT_DEBUGFUNCTION replaces the standard debug function used when CURLOPT_VERBOSE is in effect.
			///	This callback receives debug information, as specified with the curl_infotype argument. 
			///	This function must return 0. The data pointed to by the char * passed to this function WILL NOT be zero terminated,
			///	but will be exactly of the size as told by the size_t argument
			///CURLOPT_DEBUGFUNCTION CURLOPT_DEBUGDATA 
			static int  cbDebug(CURL *handle, curl_infotype infoType,unsigned char *data, size_t size, void *pCtx); 

			///CURLOPT_SSL_CTX_FUNCTION  CURLOPT_SSL_CTX_DATA 
			static CURLcode cbSSLctx(CURL *curl, void *sslctx, void *pCtx); 

			///CURLOPT_IOCTLFUNCTION CURLOPT_IOCTLDATA
			static curlioerr cbIOCtl(CURL *handle, int cmd, void *pCtx);

			///CURLOPT_SEEKFUNCTION  RLOPT_SEEKDATA
			static int cbSeek(void *pCtx, curl_off_t offset, int origin); 

			///CURLOPT_SOCKOPTFUNCTION CURLOPT_SOCKOPTDATA 
			static int cbSockopt(void *pCtx, curl_socket_t curlfd, curlsocktype purpose);

			///CURLOPT_INTERLEAVEFUNCTION CURLOPT_INTERLEAVEDATA
			static size_t cbInterleaver(void *ptr, size_t size, size_t nmemb, void *pCtx);

			/// CURLOPT_CONV_TO_NETWORK_FUNCTION 
			/// CURLOPT_CONV_FROM_NETWORK_FUNCTION 
			///	CURLOPT_CONV_FROM_UTF8_FUNCTION 
			/// CURLcode function(char *ptr, size_t length);
			public:
				static void startCurlClientManager();
				static void stopCurlClientManager();
				static size_t getCURLClientSize();
		};
 }
}
#endif //end if __CURLClient_H__

/*
CURLOPT_CHUNK_BGN_FUNCTION 
Pass a pointer to a function that matches the following prototype:
long function (const void *transfer_info, void *ptr, int remains). 
This function gets called by libcurl before a part of the stream is going to be transferred (if the transfer supports chunks). 
This callback makes sense only when using the CURLOPT_WILDCARDMATCH option for now. 

The target of transfer_info parameter is a "feature depended" structure. 
For the FTP wildcard download, the target is curl_fileinfo structure (see curl/curl.h).
The parameter ptr is a pointer given by CURLOPT_CHUNK_DATA. 
The parameter remains contains number of chunks remaining per the transfer. 
If the feature is not available, the parameter has zero value. 

Return CURL_CHUNK_BGN_FUNC_OK if everything is fine, 
CURL_CHUNK_BGN_FUNC_SKIP if you want to skip the concrete chunk or CURL_CHUNK_BGN_FUNC_FAIL to tell libcurl 
to stop if some error occurred. (This was added in 7.21.0) 

CURLOPT_CHUNK_END_FUNCTION 
Pass a pointer to a function that matches the following prototype: 
long function(void *ptr). This function gets called by libcurl as soon as a part of the stream has been transferred (or skipped). 
Return CURL_CHUNK_END_FUNC_OK if everything is fine or CURL_CHUNK_END_FUNC_FAIL 
to tell the lib to stop if some error occurred. (This was added in 7.21.0) 

CURLOPT_CHUNK_DATA 
Pass a pointer that will be untouched by libcurl and passed as the ptr argument to the 
CURL_CHUNK_BGN_FUNTION and CURL_CHUNK_END_FUNTION. (This was added in 7.21.0) 
*/

