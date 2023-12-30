#ifndef _c2_client_http_protocol_header_file_apple__
#define _c2_client_http_protocol_header_file_apple__

#include <ZQ_common_conf.h>
#include <map>
#include <string>
#include <urlstr.h>
#include <DataPostHouse/DataCommunicatorUnite.h>
#include <boost/function.hpp>

#ifdef ZQ_OS_LINUX
	#ifndef stricmp
		#define	stricmp strcasecmp
	#endif

	#ifndef strnicmp
		#define strnicmp strncasecmp
	#endif
#endif

class ICaseLess: std::binary_function<std::string, std::string, bool> 
{
public:
	result_type operator()( const first_argument_type& a, const second_argument_type& b) const
	{
		return (stricmp( a.c_str(), b.c_str()) < 0);
	}
};

#define STRVALID(x) ( x && x[0] != 0)

typedef boost::function< bool ( const char* data ,size_t size ,bool bComplete ) > BodyContentDataEvent ;

struct HttpProto
{		
	enum proto
	{
		HTTP_PROTO_NULL		= 0,
		HTTP_PROTO_10		= 1,
		HTTP_PROTO_11		= 2,
		HTTP_PROTO_DESC		= 3,
	};
	proto	mProto;
	std::string mStr;
	HttpProto():mProto(HTTP_PROTO_NULL){}
	std::string toString() const
	{
		switch( mProto )
		{			
		case HTTP_PROTO_10:		return "HTTP/1.0";
		case HTTP_PROTO_11:		return "HTTP/1.1";
		case HTTP_PROTO_DESC:	return mStr;
		default:				return "UNKNOWN";
		}
	}
	proto fromString( const char* str )
	{
		mProto = HTTP_PROTO_NULL;
		if( !STRVALID(str) ) return mProto;

		if( stricmp( str, "HTTP/1.0") == 0 )
		{
			mProto = HTTP_PROTO_10;
		}
		else if( stricmp( str, "HTTP/1.1")  == 0 )
		{
			mProto = HTTP_PROTO_11;
		}
		else
		{
			mProto = HTTP_PROTO_DESC;
			mStr = str;
		}
		return mProto;
	}
	proto fromString( const std::string& str )
	{
		return fromString( str.c_str() );
	}
	proto getType() const
	{
		return mProto;
	}
	bool isValid() const
	{
		return mProto != HTTP_PROTO_NULL;
	}
};

inline bool isStringDigital( const char* p)
{
	while(p)
	{
		if( !isdigit(*p) )
			return false;
		p++;
	}
	return true;
}

struct HttpMethod 
{
	enum method
	{
		HTTP_METHOD_NULL,
		HTTP_METHOD_GET,
		HTTP_METHOD_PUT,
		HTTP_METHOD_RESPONSE
	};
	method	mMethod;	
	std::string mStr;
	HttpMethod():mMethod(HTTP_METHOD_NULL){}
	std::string toString() const
	{
		switch ( mMethod)
		{
		case HTTP_METHOD_GET:	return "GET";
		case HTTP_METHOD_PUT:	return "PUT";
		case HTTP_METHOD_RESPONSE: return mStr;
		default:				return "UNKNOWN";
		}
	}
	method fromString( const std::string& str )
	{
		return fromString( str.c_str() );
	}
	method fromString( const char* str)
	{
		mMethod = HTTP_METHOD_NULL;		
		if( !STRVALID(str) ) return mMethod;
		if( stricmp( str , "put") == 0 )
			mMethod = HTTP_METHOD_PUT;
		else if( stricmp( str , "get") == 0 )
			mMethod = HTTP_METHOD_GET;
		else if( strstr(str,"HTTP/") == str )
		{
			mMethod = HTTP_METHOD_RESPONSE;
			mStr = str;
		}		
		return mMethod;
	}
	method getMethod() const
	{
		return mMethod;
	}
	bool isValid() const
	{
		return mMethod != HTTP_METHOD_NULL;
	}	
};


class HttpMessage : public ZQ::DataPostHouse::SharedObject
{
public:
	HttpMessage(void);
	virtual ~HttpMessage(void);
public:
	void updateMethod( const std::string& methodStr );
	void updateMethod( HttpMethod m );
	
	//void updateUrl( const ZQ::common::URLStr& url );
	void updateUrl( const std::string& url );
	
	void updateProtocol( const std::string& protocol );
	void updateProtocol( HttpProto p );

	void updateHeader( const std::string& key , const std::string& value );

	///add content data to HttpMessage will have the data sent to peer immediately
	bool addContentData( const char*& data , size_t& size ,BodyContentDataEvent sender);

	///this routine will help you to get the real data you want
	///
	bool getContentData( const char*& data , size_t& size , BodyContentDataEvent receiver );

	bool isBodyContentChunked( ){return mbChunkedData;}
	
	void setBodyContentType( bool bChunkedData ) { mbChunkedData = bChunkedData ;}

	//represent HttpMessage in string format without content body
	std::string toString( );

	std::string getUrl() const;

	std::string getMethod() const;

	std::string getProtocol() const;

	std::string getHeader( const std::string& key ) const;

protected:
	
	friend class HttpDialog;

	enum MessageParseStage
	{
		HTTP_PARSE_STAGE_NULL,
		HTTP_PARSE_STAGE_STARTLINE,
		HTTP_PARSE_STAGE_HEADER,
		HTTP_PARSE_STAGE_BODY
	};
	MessageParseStage	mParseStage;

private:

	enum ChunkDataState
	{
		CHUNK_STATE_HEADER,
		CHUNK_STATE_BODY,
		CHUNK_STATE_TAIL
	};

	HttpProto		mProto;
	HttpMethod		mMethod;
	std::string		mUrl;
	typedef std::map< std::string , std::string , ICaseLess > HEADERKVMAP;
	HEADERKVMAP		mHeaders;
	bool			mbChunkedData;
	int64			mBodyContentSize;
	int64			mCurrentUsedBodyContentSize;
	ChunkDataState	mChunkState;
};

typedef ZQ::DataPostHouse::ObjectHandle<HttpMessage> HttpMessagePtr;

#endif//_c2_client_http_protocol_header_file_apple__
