#ifndef _tianshan_cdnss_c2streamer_http_handler_header_file_h__
#define _tianshan_cdnss_c2streamer_http_handler_header_file_h__

#include <HttpEngine/HttpEngine.h>
#include <DataPostHouse/common_define.h>
#include "../SimpleXMLParser.h"
#include "C2StreamerLib.h"


namespace C2Streamer
{

class C2Service;
class C2StreamerEnv;
class C2Session;
class C2SessionManager;

typedef ZQ::DataPostHouse::ObjectHandle<C2Session> C2SessionPtr;


class C2HttpHandler : public ZQHttp::IRequestHandler 
{
public:
	C2HttpHandler( C2StreamerEnv& env , C2Service& svc);
	virtual ~C2HttpHandler(void);

public:
	virtual bool onConnected( ZQHttp::IConnection& conn);
	virtual bool onRequest(	const ZQHttp::IRequest& req, ZQHttp::IResponse& resp) ;
	virtual bool onPostData( const ZQHttp::PostDataFrag& frag) ;
	virtual bool onPostDataEnd() ;
	virtual void onRequestEnd() ;
	virtual void onBreak() ;

	bool		 postResponse();

	bool		 isConnectionBroken() const;
protected:
	virtual void	prepareResponseMsg( );
	virtual bool	process( );
	virtual bool	parseRequest( );
	virtual void	processFailed( );
	virtual void	postResponseFaied();

	void			setStandardHeader( const RequestResponseParamPtr& responsePara , bool bIncludeBodyType = true );

protected:
	C2StreamerEnv&					mEnv;
	C2Service&						mSvc;
	const ZQHttp::IRequest*			mRequest;
	C2HttpResponseHanlderPtr		mResponse;
	std::string						mXmlContent;
	SimpleXMLParser					mXmlParser;	
	bool							mbBreaked;
	
	RequestResponseParamPtr			mResponseParam;
	RequestParamPtr					mRequestParam;	
	ZQHttp::IConnection*			mConn;
};


class HandlerTransferInit : public C2HttpHandler
{
public:
	HandlerTransferInit( C2StreamerEnv& env , C2Service& svc);
	virtual ~HandlerTransferInit();
protected:
	virtual bool	process( );
	virtual bool	parseRequest( );
	virtual void	processFailed( );
	virtual void	postResponseFaied();	
	virtual void	prepareResponseMsg( );
	void			composeBody( std::ostringstream& oss) const;
};

class HanlderTransferTerminate : public C2HttpHandler
{
public:
	HanlderTransferTerminate( C2StreamerEnv& env , C2Service& svc);
	virtual ~HanlderTransferTerminate();
protected:
	virtual bool	process( );
	virtual bool	parseRequest( );
	virtual void	prepareResponseMsg( );
	void			composeBody( std::ostringstream& oss) const;
};

class HandlerAssetAttributeQeury : public C2HttpHandler {
public:
	HandlerAssetAttributeQeury( C2StreamerEnv& env, C2Service& svc );
	virtual ~HandlerAssetAttributeQeury();
private:
	virtual bool    process( );
};

class HanlderSessionStatus : public C2HttpHandler
{
public:
	HanlderSessionStatus( C2StreamerEnv& env , C2Service& svc);
	virtual ~HanlderSessionStatus();
protected:
	virtual bool	process( );
	virtual bool	parseRequest( );
	virtual void	prepareResponseMsg( );
	void			composeBody( std::ostringstream& oss );
};

class HandlerResourceStatus : public C2HttpHandler
{
public:
	HandlerResourceStatus( C2StreamerEnv& env , C2Service& svc);
	virtual ~HandlerResourceStatus();
protected:
	virtual bool	process( );
	virtual bool	parseRequest( );
	virtual void	prepareResponseMsg( );
	void			composeBody( std::ostringstream& oss );
};

class HandlerSessionTransfer : public C2HttpHandler
{
public:
	HandlerSessionTransfer( C2StreamerEnv& env , C2Service& svc , const std::string& url );
	virtual ~HandlerSessionTransfer();
protected:
	virtual bool	process( );
	virtual bool	parseRequest( );
	virtual void	prepareResponseMsg( );
	std::string		getConnectionPeerInfo();
	virtual void 	onBreak();
protected:
	std::string		mUrl;
};

class HandlerGetFile : public HandlerSessionTransfer
{
public:
	HandlerGetFile( C2StreamerEnv& env , C2Service& svc , const std::string& url );
	virtual ~HandlerGetFile();
	
protected:
	virtual bool	process( );
	virtual bool	parseRequest();
	virtual void	prepareResponseMsg( );
	void			fixupFilenameAndRange( std::string& filename, TransferRange& rang );

protected:
	
	bool createSession( );
	
	
private:
	int				mLastErrorCode;
	std::string		mSessionId;	
	int32			mSessionInitErrorCode;
	std::string		mGetFileUrl;
};


class HandlerHLS : public C2HttpHandler 
{
public:
	HandlerHLS( C2StreamerEnv& env, C2Service& svc, const std::string& uri );
	virtual ~HandlerHLS();
protected:
	virtual bool	process( );
	virtual bool	parseRequest();
	virtual void	prepareResponseMsg( );
private:
	std::string		mUri;
};

class HttpHanlderFactory : public ZQHttp::IRequestHandlerFactory
{
public:
	HttpHanlderFactory( C2StreamerEnv& env , C2Service& svc);
	virtual ~HttpHanlderFactory();

protected:
	
	virtual ZQHttp::IRequestHandler* create(const char* uri);

	virtual void destroy( ZQHttp::IRequestHandler*);

private:
	 C2StreamerEnv&				mEnv;
	 C2Service&					mSvc;
};

}//namespace C2Streamer

#endif//_tianshan_cdnss_c2streamer_http_handler_header_file_h__
