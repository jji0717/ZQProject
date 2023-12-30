#ifndef _zq_cdn_ss_http_bridge_header_file_h__
#define _zq_cdn_ss_http_bridge_header_file_h__

#include <ZQ_common_conf.h>
#include <Log.h>
#include <HttpEngine.h>
#include <HttpClient.h>
#include "SimpleXMLParser.h"
#include <Locks.h>
#include <Log.h>
#include <map>
#include <string>
#include <vector>
#include <set>


namespace ZQ
{
namespace StreamService
{
class CdnSsEnvironment;


bool				getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::string& value);
bool				getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , int32& value);
bool				getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , int64& value);
bool				getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , float& value);
bool				getValueEx( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::string& value);


class CDNHttpHandler: public ZQHttp::IRequestHandler
{
public:
	CDNHttpHandler( CdnSsEnvironment* environment );
	virtual ~CDNHttpHandler( );
public:
	virtual bool onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp) ;
	virtual bool onPostData(const ZQHttp::PostDataFrag& frag) ;
	virtual bool onPostDataEnd() ;
	virtual void onRequestEnd() ;
	virtual void onBreak() ;

protected:

	virtual bool		run( ) = 0;

	bool				parseResultXml( const std::string& xml );

	void				generateReponseHeader( int code , size_t contentLength , const char* reason = NULL );


protected:

	ZQHttp::IResponse*				mResponse;
	const ZQHttp::IRequest*			mRequest;
	std::string						mXmlContent;
	SimpleXMLParser					mXmlParser;
	ZQ::common::Log*				mLogger;
	CdnSsEnvironment*				env;
};

class TranferUpdateIngressCapacity : public CDNHttpHandler
{
public:
	TranferUpdateIngressCapacity(  CdnSsEnvironment* environment );
	virtual ~TranferUpdateIngressCapacity( );

protected:
	virtual bool		run( ) ;


private:

};


class TransferUpdateState : public CDNHttpHandler
{
public:
	TransferUpdateState(  CdnSsEnvironment* environment );
	virtual ~TransferUpdateState( );

public:
	
	virtual bool		run( ) ;


private:

};

class TranferUpdateResource : public CDNHttpHandler
{
public:
	TranferUpdateResource(  CdnSsEnvironment* environment  );
	virtual ~TranferUpdateResource( );

protected:
	
	virtual bool		run( ) ;

private:

	

};

class CDNHttpFactory : public ZQHttp::IRequestHandlerFactory
{
public:
	CDNHttpFactory(  CdnSsEnvironment* environment );
	virtual ~CDNHttpFactory( );
protected:
	virtual ZQHttp::IRequestHandler* create(const char* uri);
	virtual void destroy( ZQHttp::IRequestHandler*);
private:
	 CdnSsEnvironment*	env;  
};

class CDNMiniXmlBuilder
{
public:
	CDNMiniXmlBuilder( );
	virtual ~CDNMiniXmlBuilder( );

public:
	
	void	create( const std::string& rootNode );

	void	addParameter( const std::string& key , const std::string& value );

	std::string exportXml( );

	void	reset( );
private:
	typedef struct _tagParameter 
	{
		std::string		nodeName;
		std::string		value;
	}Parameter;
	typedef std::vector<Parameter> ParameterS;
	ParameterS			mParas;
	std::string			mRootName;
};

class C2HttpClient : public ZQ::common::HttpClient
{
public:
	C2HttpClient(  CdnSsEnvironment* environment ,const std::string& contextKey  );
	virtual ~C2HttpClient( );

public:

	void						setProxyUrl( const std::string& url );

	virtual int32				invoke( const std::string& uri ) = 0;

	const std::string&			getLastError( ) const;

	int							lastErrorCode( ) ;

	int32						sendRequest( const std::string& uri, bool getOrPost = false );

	void						setPeerAddress( const std::string& peerIp, const std::string& peerPort );
protected:

	void						initHttpHeader( );

	virtual bool				sendHeader( ) = 0;

	virtual bool 				sendBodyContent( ) = 0;

	virtual bool				getBodyContent( ) = 0;

	virtual bool				processBodyContent( ) = 0;

protected:

	CdnSsEnvironment*			env;

	std::string					mContextKey;

	std::string					mLastError;
	std::string					mPeerIp;
	std::string					mPeerPort;

	std::string					mUri;
	
};

class CDNIndexGetter : public C2HttpClient
{
public:
	CDNIndexGetter( CdnSsEnvironment* environment ,const std::string& contextKey ,
		const std::string& upstreamUrl,
		size_t sizeWanted = 1024 * 8 );

	virtual ~CDNIndexGetter();
	
	virtual int32				invoke( const std::string& uri );

	const char*					getIndexData( size_t& dataSize ) const {
		dataSize = mDataSize;
		return mBuffer;
	}
protected:

	virtual bool				sendHeader( ) ;

	virtual bool 				sendBodyContent( );

	virtual bool				getBodyContent( );

	virtual bool				processBodyContent( );

private:
	std::string mUpstreamUrl;
	char*		mBuffer;
	size_t		mWantedSize;
	size_t		mDataSize;
};

class CDNHttpClient : public C2HttpClient
{
public:
	CDNHttpClient( CdnSsEnvironment* environment ,const std::string& contextKey );
	virtual ~CDNHttpClient( );

protected:

	virtual bool		sendHeader( ) ;

	virtual bool 		sendBodyContent( );

	virtual bool		getBodyContent( );

	virtual bool		processBodyContent( );

	//check the xml returned by http server is correct or not
	virtual bool		processXML( const std::string& result ){return false;}

	bool				parseResultXml( const std::string& xml );
	
protected:

	CDNMiniXmlBuilder			mXmlBuilder;
	SimpleXMLParser				mXmlParser;

	std::string					mResult;
};

class C2LocateRequest : public CDNHttpClient
{
public:
	C2LocateRequest(  CdnSsEnvironment* environment ,
		const std::string& contextKey,
		const std::string& peerIp, const std::string& peerPort,
		const std::string& pid, const std::string& paid,
		const std::string& subtype,
		const std::string& clientTransfer,
		const std::string& transferRate,
		const std::string& ingressCapacity,
		const std::string& range="0-8191",
		const std::string& delay="0");
	virtual ~C2LocateRequest( );

public:

	virtual int32		invoke( const std::string& uri ) ;

	virtual bool 		sendBodyContent( );
	///get transfer id generated by Http server
	const std::string&	getTransferId( ) const;

	const std::string&	getDownloadServerIp( ) const;

	const std::string&	getDownloadServerPort( ) const;

protected:
	//check the xml returned by http server is correct or not
	virtual bool		processXML( const std::string& result );
private:
	std::string		mRequestBody;
	std::string		mDownloadServerIp;
	std::string		mDownloadServerPort;
	std::string		mTransferId;
};

class TransferInitiate : public CDNHttpClient
{
public:
	TransferInitiate(   CdnSsEnvironment* environment ,
					const std::string& contextKey,
					const std::string& clientTranfer , 
					const std::string& transferAddress ,
					const std::string& ingressCapacity,
					const std::string& allocatedTransferRate,
					const std::string& fileName,
					const std::string& transferRate,
					const std::string& transferTimeout,
					const std::string& range = "",
					const std::string& delay = "");
	virtual ~TransferInitiate( );

public:

	virtual int32		invoke( const std::string& uri ) ;

	///get transfer id generated by Http server
	const std::string&	getTransferId( ) const;

	const std::string&	getAvailRange( ) const;

	const std::string&	getOpenForWrite( ) const;

protected:
	//check the xml returned by http server is correct or not
	virtual bool		processXML( const std::string& result );

private:
	std::string			mTransferId;
	std::string			mAvailRange;
	std::string			mOpenForWrite;
};

class TransferTerminate : public  CDNHttpClient
{
public:
	TransferTerminate(   CdnSsEnvironment* environment ,
					const std::string& contextKey,
					const std::string& tranferId,
					const std::string& clientTransfer ="" );
	~TransferTerminate( );

public:

	virtual int32		invoke( const std::string& uri ) ;

protected:
	//check the xml returned by http server is correct or not
	virtual bool		processXML( const std::string& result );

};

class IngressCapacityUpdate : public CDNHttpClient
{
public:
	IngressCapacityUpdate(   CdnSsEnvironment* environment ,
							const std::string& contextKey,
							const std::string& clientTransfer , 
							const std::string& ingressCapacity,
							const std::string& allocatedIngressCapacity	);
	~IngressCapacityUpdate();
public:

	virtual int32		invoke( const std::string& ip , const std::string& port ) ;
protected:
	//check the xml returned by http server is correct or not
	virtual bool		processXML( const std::string& result );

private:

	
};

class SessionStatusQuery : public CDNHttpClient
{
public:
	SessionStatusQuery(   CdnSsEnvironment* environment , 
						const std::string& contextKey,
						const std::string& clientAddress );
	virtual ~SessionStatusQuery( );
public:

	virtual int32		invoke( const std::string& uri ) ;

	typedef struct _SessionAttr 
	{
		std::string			transferId;
		std::string			fileName;
		std::string			transferAddress;
		std::string			clientTransfer;
		std::string			transferPort;//this should be the port name such as eth0
		std::string			transferState;
		int32				timeInState;
		int32				transferRate;
		int64				transferBytes;
		_SessionAttr()
		{
			transferId			=	"";
			fileName			=	"";
			clientTransfer		=	"";
			transferAddress		=	"";
			transferPort		=	"";
			transferState		=	"";
			timeInState			=	0;
			transferRate		=	0;
			transferBytes		=	0;
		}
	}SessionAttr ;
	typedef std::vector<SessionAttr>	SessionAttrS;

	const SessionAttrS&	getSessions( ) const;
protected:
	//check the xml returned by http server is correct or not
	virtual bool		processXML( const std::string& result );

private:
	SessionAttrS		mSessions;
};

class ResourceStatusQuery : public CDNHttpClient
{
public:
	ResourceStatusQuery( CdnSsEnvironment* environment ,const std::string& contextKey);
	~ResourceStatusQuery();

public:
	void				addPortName( const std::string& portName);

	virtual int32		invoke( const std::string& uri ) ;
	

	
	typedef struct _PortAttr 
	{
		std::string			name;
		std::vector<std::string>	transferAddressIpv4;
		std::vector<std::string>	transferAddressIpv6;
		int64				capacity;
		bool				bUp;
		int32				activeTransferCount;
		int64				activeBandwidth;
		_PortAttr()
		{
			bUp					=		false;
			activeBandwidth		=		0;
			activeTransferCount	=		0;
			capacity			=		0;
		}
	}PortAttr;
	typedef std::vector<PortAttr>	PortAttrs;

	bool				getPortAttrs( PortAttrs& attrs ) const;

protected:
	//check the xml returned by http server is correct or not
	virtual bool		processXML( const std::string& result );

	bool				getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::string& value);

private:
	
	std::vector<std::string>	mPortNames;

	PortAttrs					mPortAttrs;

};

}}

#endif//_zq_cdn_ss_http_bridge_header_file_h__

