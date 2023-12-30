#if !defined(__ZQTIANSHAN_GBSVC_HTTP_REQUEST_FROM_CLIENT_H__)
#define __ZQTIANSHAN_GBSVC_HTTP_REQUEST_FROM_CLIENT_H__

#include "SocketClientContext.h"

namespace ZQTianShan {	  
namespace GBServerNS { 

typedef struct _XMLFileInf
{
public:
	typedef enum _XmlParserStatus
	{
		SUCCEED                = 0,
		INPUT_XML_EMPTY           ,
		READ_FAILED               ,	
		GET_ROOTPREFERENCE_FAILED ,
		GET_HEADER_FAILED         ,
		GET_BODY_FAILED           ,
		OPCODE_FAILED             ,
		STATUS_COUNT
	} XmlParserStatus;
	
	static char * _parserStatus[STATUS_COUNT + 1];

public:
	unsigned int  _xmlBufSize;
	std::string   _xmlBuf;
	std::string   _opCodeInXml;
	std::map<std::string, std::string > _xmlParseResult;

public:
	int parseXmlBuf(void);

}XMLFileInf;

class HttpContext
{
public:
	typedef enum _HttpStatus
	{
		HTTP_CONTINUE               ,
		HTTP_OK                     ,
		HTTP_BAD_REQUEST            ,
		HTTP_FORBIDDEN              ,
		HTTP_NOT_FOUND              ,
		HTTP_METHOD_NOT_ALLOWED     ,
		HTTP_INTERNAL_SERVER_ERROR  ,
		HTTP_NOT_IMPLEMENTED        ,
		HTTP_STATUS_END
	} HttpStatus;

	typedef enum _HttpRequst
	{
	  HTTP_POST_FROM_CLIENT,
	  HTTP_POST_TO_CLIENT,
	  HTTP_REQUEST_END
	}HttpRequst;

	typedef struct	_HttpStatusContent
	{
		HttpStatus  status;
		const char*	statusCode;
		const char* reasonPhrase;
	}HttpStatusContent;

public:
	HttpContext(ZQ::common::NativeThreadPool&  releasePool, ZQ::common::Log& log);

	int  obtainHttpContent(char * buf, int bufSize);
	int  parseHttpContent(void);
	int  reponseHttpClient(ClientSocketContext* dataKey);
	
	HttpStatus getHttpStatus(void){return _httpStatus;}

private:
	std::string  preHttpStatusLine(void);
	std::string  preHttpGeneralHeader(void);
	std::string  preHttpResponseHeader(void);
	std::string  preHttpEntityHeader(unsigned int messageBodySize);
	std::string  preHttpMessageBody(void);

	std::string  preHttpResponse(void);
	
private:
	HttpRequst     _httpRequst;
	HttpStatus     _httpStatus;
	XMLFileInf     _contentInf;
	
	ZQ::common::Log &     _log;
	ZQ::common::Mutex     _lockXmlParse;
	ZQ::common::NativeThreadPool&  _releasePool;

	static const int _httpStatusTblSize = HTTP_STATUS_END + 1;
	static HttpStatusContent _httpStatusTbl[_httpStatusTblSize];
};

class HttpFeedBackContext
{
public:
	HttpFeedBackContext(XMLFileInf& contentInf, unsigned int ngbCmdCode);
	std::string  preHttpFeedBack(void);

private:
	std::string  httpMessageBody();
	std::string  httpStatusLine();
	std::string  httpEntityHeader(unsigned int messageBodySize);

private:
	int         _ngbCmdCode;
	XMLFileInf  _contentInf;
};

class HttpContextUtil
{
public:
   	static std::string currentDateString(void);
	static std::string int2str(int size);
};

}//GBServerNS
}//	ZQTianShan
#endif//__ZQTIANSHAN_GBSVC_HTTP_REQUEST_FROM_CLIENT_H__