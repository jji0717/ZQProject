#ifndef _tianshan_cdnss_c2streamer_http_handler_header_file_h__
#define _tianshan_cdnss_c2streamer_http_handler_header_file_h__

#include <HttpEngine/HttpEngine.h>
#include <DataPostHouse/common_define.h>
#include "../SimpleXMLParser.h"
#include "AioFile.h"
#include "C2StreamerLib.h"
#include "CacheServerRequest.h"

namespace C2Streamer
{

class C2Service;
class C2StreamerEnv;
class C2Session;
class C2SessionManager;

typedef ZQ::common::Pointer<C2Session> C2SessionPtr;


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
	virtual int		process( );
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
	virtual int		process( );
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
	virtual int		process( );
	virtual bool	parseRequest( );
	virtual void	prepareResponseMsg( );
	void			composeBody( std::ostringstream& oss) const;
};

class HandlerAssetAttributeQuery;

class AssetQueryHandlerCallback : public LibAsync::AsyncWork, public IAsyncNotifySinker {
public:
	typedef ZQ::common::Pointer<AssetQueryHandlerCallback> Ptr;
	AssetQueryHandlerCallback(HandlerAssetAttributeQuery* handler, LibAsync::EventLoop* loop);
	virtual ~AssetQueryHandlerCallback();
private:
	virtual void 	onNotified();
	virtual void 	onAsyncWork();
private:
	HandlerAssetAttributeQuery*	mHandler;
};

class HandlerAssetAttributeQuery : public C2HttpHandler, public ZQHttp::IChannelWritable {
public:
	HandlerAssetAttributeQuery( C2StreamerEnv& env, C2Service& svc );
	virtual ~HandlerAssetAttributeQuery();
private:
	friend class AssetQueryHandlerCallback;
	virtual int 	process( );
	void			postProcess( );

	virtual void	onBreak();
	virtual void 	onWritable();
private:
	AssetAttribute::Ptr	mAssetAttr;
	std::string			mFileName;
	std::string 		mLeftMsg;
};

class HanlderSessionStatus : public C2HttpHandler
{
public:
	HanlderSessionStatus( C2StreamerEnv& env , C2Service& svc);
	virtual ~HanlderSessionStatus();
protected:
	virtual int		process( );
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
	virtual int		process( );
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
	virtual int		process( );
	virtual bool	parseRequest( );
	virtual void	prepareResponseMsg( );
	std::string		getConnectionPeerInfo();
	virtual void 	onBreak();
	virtual void	postResponseFaied();
protected:
	std::string		mUrl;
	int32			mLastErrorCode;
	std::string		mSessionId;
};

class HandlerGetFile : public HandlerSessionTransfer
{
public:
	HandlerGetFile( C2StreamerEnv& env , C2Service& svc , const std::string& url );
	virtual ~HandlerGetFile();

protected:
	virtual int		process( );
	virtual bool	parseRequest();
	//virtual void	prepareResponseMsg( );
	void			fixupFilenameAndRange( std::string& filename, TransferRange& rang );

protected:

	virtual bool createSession( );

	const std::string& requestUrl() const {
		return mGetFileUrl;
	}

private:
	int				mLastErrorCode;
	int32			mSessionInitErrorCode;
	std::string		mGetFileUrl;
};


class HandlerHLS : public HandlerGetFile
{
public:
	HandlerHLS( C2StreamerEnv& env, C2Service& svc, const std::string& uri );
	virtual ~HandlerHLS();
protected:
	virtual int		process( );
};

class HandlerC2Locate : public C2HttpHandler, public ZQHttp::IChannelWritable
{
public:
   // typedef ZQ::common::Pointer<HandlerC2Locate> LocatePtr;

	HandlerC2Locate( C2StreamerEnv& env, C2Service& svc);
	virtual ~HandlerC2Locate();

    void postProcess(int code, const std::string& message, const std::string& xSessID);
protected:

	virtual int 	process( );

	virtual void	onBreak( );
    virtual void    onWritable();
private:
	C2LocateCB::Ptr		mLocateCb;
    std::string         mLeftMsg;
    ConnInfo            mConnInfo;
};

typedef ZQ::common::Pointer<HandlerC2Locate> HandlerC2LocatePtr;

class HandlerDiagnosis : public C2HttpHandler {
public:
	HandlerDiagnosis( C2StreamerEnv& env, C2Service& svc );
	virtual ~HandlerDiagnosis();

protected:
	virtual int process( );

	void		setDefaultResponse( int code );
	int 		pickOutstandingBuffer( );
	void		methodNotFound( );
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
