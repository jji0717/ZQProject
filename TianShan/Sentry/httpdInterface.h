
#ifndef __SENTRY_SERVICE_EMBED_HTTP_SERVICE_INTERFACE_H__
#define __SENTRY_SERVICE_EMBED_HTTP_SERVICE_INTERFACE_H__

#include "Log.h"
#include <string>
#include <map>

#define CGI_AUTH_TYPE			"AUTH_TYPE"
#define CGI_CONTENT_LENGTH		"CONTENT_LENGTH"
#define CGI_CONTENT_TYPE		"CONTENT_TYPE"
#define CGI_GATEWAY_INTERFACE	"GATEWAY_INTERFACE"
#define CGI_PATH_INFO			"PATH_INFO"
#define CGI_PATH_TRANSLATED		"PATH_TRANSLATED"
#define CGI_QUERY_STRING		"QUERY_STRING"
#define CGI_REMOTE_ADDR			"REMOTE_ADDR"
#define CGI_REMOTE_HOST			"REMOTE_HOST"
#define CGI_REMOTE_IDENT		"REMOTE_IDENT"
#define CGI_REMOTE_USER			"REMOTE_USER"
#define CGI_REQUEST_METHOD		"REQUEST_METHOD"
#define CGI_SCRIPT_FILENAME		"SCRIPT_FILENAME"
#define CGI_SCRIPT_NAME			"SCRIPT_NAME"
#define CGI_SERVER_NAME			"SERVER_NAME"
#define CGI_SERVER_ADDR			"SERVER_ADDR"
#define CGI_SERVER_PORT			"SERVER_PORT"
#define CGI_SERVER_PROTOCOL		"SERVER_PROTOCOL"
#define CGI_SERVER_SOFTWARE		"SERVER_SOFTWARE"
#define CGI_SYSTEM_ROOT			"SystemRoot"
//cgi enviroment

enum HTTPMETHOD
{
	M_UNKNOWN = 99,
	M_OPTIONS = 0,
	M_GET,
	M_HEAD,
	M_POST,
	M_PUT,
	M_DELETE,
	M_TRACE,
	M_CONNECT
};

enum ResourceType
{
	RT_UNKNOWN = 0,
	RT_HYPERTEXT,				//HTML
	RT_STYLE_SHEET,				
	RT_IMAGE,					//image file
	RT_PERL_SCRIPT,				//script
	RT_DYNAMIC_LINK_LIB,		//dynamic library
	RT_EXECUTABLE,				//executable file
	RT_VIDEO,					
	RT_AUDIO,
	RT_BATCH,
	RT_SYSTEMFUNCTION
};

enum  HttpStatusCodes
{
	// Informational 100 - 1nn
	HSC_CONTINUE					= 100,
	HSC_SWITCHING_PROTOCOLS,
	// Successful 200 - 2nn
	HSC_OK							= 200,
	HSC_CREATED,
	HSC_ACCEPTED,
	HSC_NON_AUTHORITATIVE_INFORMATION,
	HSC_NO_CONTENT,
	HSC_RESET_CONTENT,
	HSC_PARTIAL_CONTENT,
	// Redirection 300 - 3nn
	HSC_MULTIPLE_CHOICES			= 300,
	HSC_MOVED_PERMANENTLY,
	HSC_FOUND,
	HSC_SEE_OTHER,
	HSC_NOT_MODIFIED,
	HSC_USE_PROXY,
	HSC_TEMPORARY_REDIRECT = 307,
	// Client Error 400 - 4nn
	HSC_BAD_REQUEST					= 400,
	HSC_UNAUTHORIZED,
	HSC_PAYMENT_REQUIRED,
	HSC_FORBIDDEN,
	HSC_NOT_FOUND,
	HSC_METHOD_NOT_ALLOWED,
	HSC_NOT_ACCEPTABLE,
	HSC_PROXY_AUTHENTICATION_REQUIRED,
	HSC_REQUEST_TIMEOUT,
	HSC_CONFLICT,
	HSC_GONE,
	HSC_LENGTH_REQUIRED,
	HSC_PRECONDITION_FAILED,
	HSC_REQUEST_ENTITY_TOO_LARGE,
	HSC_REQUEST_URI_TOO_LARGE,
	HSC_UNSUPPORTED_MEDIA_TYPE,
	HSC_REQUEST_RANGE_NOT_SATISFIABLE,
	HSC_EXPECTATION_FAILED,
	// Server Error 500 - 5nn
	HSC_INTERNAL_SERVER_ERROR		= 500,
	HSC_NOT_IMPLEMENTED,
	HSC_BAD_GATEWAY,
	HSC_SERVICE_UNAVAILABLE,
	HSC_GATEWAY_TIMEOUT,
	HSC_HTTP_VERSION_NOT_SUPPORTED
};


enum ContentTypes 
{
	CTI_UNKNOWN = 0,
	CTI_TEXTHTML,
    CTI_TEXTJAVASCRIPT,
	CTI_TEXTCSS,
	CTI_TEXTPLAIN,
	CTI_IMAGEGIF,
	CTI_IMAGEJPEG,
	CTI_VIDEOMPEG,
	CTI_VIDEOMSVIDEO,
	CTI_VIDEOREALMEDIA,
	CTI_APPLICATIONOCTETSTREAM,
    CTI_TEXTXML,
};



#define	CT_TEXTHTML					"text/html"
#define CT_TEXTJAVASCRIPT           "text/javascript"
#define	CT_TEXTCSS					"text/css"
#define CT_TEXTPLAIN				"text/plain"
#define	CT_IMAGEGIF					"image/gif"
#define	CT_IMAGEJPEG				"image/jpeg"
#define CT_VIDEOMPEG				"video/mpeg"
#define	CT_VIDEOMSVIDEO				"video/x-msvideo"
#define	CT_VIDEOREALMEDIA			"video/x-realvideo"
#define	CT_APPLICATIONOCTETSTREAM	"application/octet-stream"
#define CT_TEXTXML                  "text/xml"


#define HS_CONTINUE							"Continue"				// 100
#define HS_SWITCHING_PROTOCOLS				"Switching Protocols"
// Successful 200 - 2nn
#define HS_OK								"OK"					// 200
#define HS_CREATED							"Created"
#define HS_ACCEPTED							"Accepted"
#define HS_NON_AUTHORITATIVE_INFORMATION	"Non-Authoritative Information"
#define HS_NO_CONTENT						"No Content"
#define HS_RESET_CONTENT					"Reset Content"
#define HS_PARTIAL_CONTENT					"Partial Content"
// Redirection 300 - 3nn
#define HS_MULTIPLE_CHOICES					"Multiple Choices"		// 300
#define HS_MOVED_PERMANENTLY				"Moved Permanently"
#define HS_FOUND							"Found"
#define HS_SEE_OTHER						"See Other"
#define HS_NOT_MODIFIED						"Not Modified"
#define HS_USE_PROXY						"Use Proxy"
#define HS_TEMPORARY_REDIRECT				"Temporary Redirect"
// Client Error 400 - 4nn
#define HS_BAD_REQUEST						"Bad Request"			// 400
#define HS_UNAUTHORIZED						"Unauthorized"
#define HS_PAYMENT_REQUIRED					"Payment Required"
#define HS_FORBIDDEN						"Forbidden"
#define HS_NOT_FOUND						"Not Found"
#define HS_METHOD_NOT_ALLOWED				"Method Not Allowed"
#define HS_NOT_ACCEPTABLE					"Not Acceptable"
#define HS_PROXY_AUTHENTICATION_REQUIRED	"Proxy Authentication Required"
#define HS_REQUEST_TIMEOUT					"Request Timeout"
#define HS_CONFLICT							"Conflict"
#define HS_GONE								"Gone"
#define HS_LENGTH_REQUIRED					"Length Required"
#define HS_PRECONDITION_FAILED				"Precondition Failed"
#define HS_REQUEST_ENTITY_TOO_LARGE			"Request Entity Too Large"
#define HS_REQUEST_URI_TOO_LARGE			"Request-URI Too Large"
#define HS_UNSUPPORTED_MEDIA_TYPE			"Unsupported Media Type"
#define HS_REQUEST_RANGE_NOT_SATISFIABLE	"Requested Range Not Satisfiable"
#define HS_EXPECTATION_FAILED				"Expectation Failed"

#define HS_INTERNAL_SERVER_ERROR			"Internal Server Error"	// 500
#define HS_NOT_IMPLEMENTED					"Not Implemented"
#define HS_BAD_GATEWAY						"Bad Gateway"
#define HS_SERVICE_UNAVAILABLE				"Service Unavailable"
#define HS_GATEWAY_TIMEOUT					"Gateway Timeout"
#define HS_HTTP_VERSION_NOT_SUPPORTED		"HTTP Version Not Supported"

#define HTTP_MAJOR_VER	1
#define HTTP_MINOR_VER	1
#define HTTP_VER		"HTTP/1.1"			// HTTP server version
//#define SERVER_VER		"Sentry/1.0"	// streaming server version
#define CGI_VER			"CGI/1.1"			// we support CGI V1.1



class IHttpYeoman
{
public:
    virtual ~IHttpYeoman() {}

public:
	///send data back to client
	///@return true if success false if fail
	///@param pBuf buffer address
	///@param buffer data size
	virtual bool				SendData(const char* pBuf , int iLen ) =0;


	///get http request header value by key
	///@return the value pointer,NULl if failed to query the key
	///@param the header
	virtual const char*			getHeaderValue(const char* key) =0;

	///build response header
	///@return the header length,0 if failed
	///@param buffer the buffer to hold the response header
	///@param bufLen buffer length
	///@param statusCode http status code
	///@param contentLength content body length
	virtual int					buildResponseHeader(char* buffer , int bufLen ,int statusCode , int64 contentLength = 0) =0;

	///return error message to client
	///@return true if success false if failed
	///@param statuscode error status code
	virtual bool				ErrorResponse( int statuscode) =0;


	///get http request uri
	///@return the uri ,NULL if failed
	virtual const char*			GetRequestQueryString( ) =0;


	///get http post data
	///@return the post data,NULL if no post data available
	virtual const char*			GetRequestPostData( ) =0;


	///set content creation time
	///@param fTime the content creation time
//	virtual void				SetCreationTime(FILETIME& fTime ) =0;


	///set content type
	///@param strContentType content type string
	virtual void				SetContentType(const std::string& strContentType) =0;

	///set status string
	///@param strStatusString status string
	virtual void				SetStatusString(const std::string& strStatusString) =0;


	///set location string
	///@param strLocationString location string
	virtual	void				SetLocationString( const std::string& strLocationString )  =0;
};
class IHttpResponse
{
public:
    virtual ~IHttpResponse() {}

public:
    virtual IHttpResponse& operator<< (const std::string&) = 0;
    virtual IHttpResponse& operator<< (const char*) = 0;
    virtual IHttpResponse& operator<< (char) = 0;
    virtual IHttpResponse& operator<< (int) = 0;
    virtual IHttpResponse& operator<< (uint32) = 0;
    virtual IHttpResponse& operator<< (int64) = 0;
#ifdef ZQ_OS_MSWIN
	virtual IHttpResponse& operator<< (DWORD) = 0;
#endif
    virtual void SetLastError(const char*) {}
    virtual const char* GetLastError() { return NULL; }
    virtual void setContentType(const char*){} // set the content type of the response data
    virtual void setHeader(const char* name, const char* value){} // set the addition header fields
};
class IHttpRequestCtx
{
public:
    virtual ~IHttpRequestCtx() {}

public:
    virtual const char* GetRootDir() = 0;
    virtual const char* GetRootURL() = 0;
    virtual const char* GetUri() = 0;
    virtual const char* GetRequestVar(const char*) = 0;
    typedef std::map< std::string, std::string > RequestVars;
    virtual RequestVars GetRequestVars() = 0;
    virtual IHttpResponse& Response() = 0;
    virtual HTTPMETHOD GetMethodType() = 0;
};
///dll initialize prototype
///the export function name must be "LibInit"
///if lib return false,no more function will be invoked
typedef bool (*LibInitialize) ( ZQ::common::Log* pLog);

///dll un-initialize prototype
///the export function name must be "LibUninit"
typedef void (*LibUnInitialize)();

///dll process prototype
typedef bool (*LibProcess)(IHttpRequestCtx*);

#endif//__SENTRY_SERVICE_EMBED_HTTP_SERVICE_INTERFACE_H__
