#include <assert.h>
#include "C2HttpLibAsyncBridge.h"

namespace C2Streamer {
C2AsyncHttpHandler::C2AsyncHttpHandler( C2AsyncHttpHandlerFactory& fac, ZQ::common::Log& logger, 
		HttpProcessor::Ptr processor, ZQHttp::IRequestHandlerFactory* realFactory,
		ZQ::common::NativeThreadPool& pool)
:mFac(fac),
mThreadPool(pool),
mHttpProcessor(processor),
mRealFactory(realFactory),
mRealHandler(NULL),
mLastError(0),
mSema(1),
mLogger(logger),
mRequestEnd(false),
mResponseHeaderSent(false)	{
	mResponseMessage = new HttpMessage(HTTP_RESPONSE);
	mRecvBuffer.len = 4096;
	mRecvBuffer.base = (char*)malloc(mRecvBuffer.len);
}

C2AsyncHttpHandler::~C2AsyncHttpHandler() {
	free( mRecvBuffer.base );
	if(mRealFactory && mRealHandler) {
		mRealFactory->destroy(mRealHandler);
	}
}

void C2AsyncHttpHandler::setStatus( int code, const char* reason) {	
	mResponseMessage->code(code);
	if( reason )
		mResponseMessage->status(reason);
	else
		mResponseMessage->status( HttpMessage::code2status(code));
}

void C2AsyncHttpHandler::setHeader( const char* header, const char* value) {
	mResponseMessage->header( header, value );
	if( strcasecmp(header,"Connection") == 0 && strcasecmp(value,"Keep-Alive")==0) {
		mResponseMessage->keepAlive(true);
	} else if( strcasecmp(header,"Content-Length") == 0 ) {
		uint64 bodylength = 0;
		sscanf(value,"%lu",&bodylength);
		mResponseMessage->contentLength(bodylength);
	} else if( strcasecmp(header,"Transfer-Encoding") == 0 && strstr(value,"chunked") != NULL ) {
		mResponseMessage->chunked(true);
	}
}

bool C2AsyncHttpHandler::headerPrepared() {
	waitSignal();
	if(mLastError != 0) {
		signal();//a dummy signal which let later complete call can proceed
		return false;
	}
	if( mResponseMessage->chunked()) {//hack for c2client which is used to get file content from remote machine
		static const std::string header_lastmodified = "Last-Modified";
		if( mResponseMessage->header(header_lastmodified).empty()) {
			std::string strNow = LibAsync::HttpMessage::httpdate();
			mResponseMessage->header(header_lastmodified, strNow);
			mResponseMessage->header("Date",strNow);
		}
	}
	mLogger(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncHttpHandler,"raw message: %s"), mResponseMessage->toRaw().c_str() );
	mHttpProcessor->beginSend(mResponseMessage);
	mResponseHeaderSent = true;
	return mLastError == 0;
}

bool C2AsyncHttpHandler::addContent( const char* data, size_t len ) {
	waitSignal();
	if(mLastError != 0) {
		mLogger(ZQ::common::Log::L_ERROR, CLOGFMT(C2AsyncHttpHandler,"addContent() [%p] the lastError[%d]."), this, mLastError);
		signal();//a dummy signal which let later complete call can proceed
		return false;
	}
	AsyncBuffer buf;
	buf.base = (char*)data;
	buf.len = len;
	mHttpProcessor->sendBody(buf);
	mLogger(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncHttpHandler,"addContent() [%p] send body with lastError[%d]."), this, mLastError);
	return mLastError == 0;
}

void C2AsyncHttpHandler::postponeClose() {
	HttpProcessorCleaner::Ptr p = new HttpProcessorCleaner(mHttpProcessor);
	
	assert( mHttpProcessor->__getRef() > 1 );
	mHttpProcessor = NULL;
	
	p->queueWork();
}

bool C2AsyncHttpHandler::complete( ) {
	waitSignal();
	if(mLastError != 0) {
		postponeClose();
		return false;
	} 

	mHttpProcessor->endSend();

	{
		// after invoking this, no more operation can be performed on this instance
		// so it's safe to waitSignal here
		if(mResponseMessage->chunked()) {
			waitSignal();
		}
	}
	bool bOK = mLastError == 0 ;
	postponeClose();
	return bOK;
}

void C2AsyncHttpHandler::sendDefaultErrorPage( int ) {
}

const char* C2AsyncHttpHandler::getLastError( ) const {
	return "";
}

ZQHttp::Method C2AsyncHttpHandler::method() const {
	switch( mRequestMessage->method() ) {
	case HTTP_GET:
		return ZQHttp::GET;
	case HTTP_PUT:
		return ZQHttp::PUT;
	case HTTP_DELETE:
		return ZQHttp::M_DELETE;
	case HTTP_POST:
		return ZQHttp::POST;
	default:
		break;
	}
	return ZQHttp::UNKNOWN;
}

const char* C2AsyncHttpHandler::version() const{
	return "HTTP/1.1";// hard code a dummy value, http_parser ignore the http version part
}

const char* C2AsyncHttpHandler::uri() const {
	ZQ::common::URLStr url(mRequestMessage->url().c_str());
	return url.getPath();
}

const char* C2AsyncHttpHandler::getFullUri() const {
	return mRequestMessage->url().c_str();
}

const char* C2AsyncHttpHandler::queryArgument( const char* q) const {
	ZQ::common::URLStr url(mRequestMessage->url().c_str());
	return url.getVar(q);
}

std::map<std::string, std::string> C2AsyncHttpHandler::queryArguments() const {
	ZQ::common::URLStr url(mRequestMessage->url().c_str());
	return url.getEnumVars();
}

const char* C2AsyncHttpHandler::header( const char* h ) const {
	return mRequestMessage->header(h).c_str();
}

bool C2AsyncHttpHandler::getLocalEndpoint( std::string& addr, int& port ) const {
	unsigned short localPort = 0 ;
	if(!mHttpProcessor->getLocalAddress(addr, localPort))
		return false;
	port = localPort;
	return true;
}

bool C2AsyncHttpHandler::getRemoteEndpoint( std::string& addr, int& port ) const {
	unsigned short peerPort = 0;
	if(!mHttpProcessor->getPeerAddress(addr,peerPort))
		return false;
	port = peerPort;
	return true;
}

void C2AsyncHttpHandler::waitSignal( ) {
	mSema.wait();
}

void C2AsyncHttpHandler::signal() {
	mSema.post();
}

void C2AsyncHttpHandler::onHttpDataSent( size_t size ) {
	mLogger(ZQ::common::Log::L_INFO,CLOGFMT(C2AsyncHttpHandler,"onHttpDataSent() [%p] entry."), this);
	signal();
}

void C2AsyncHttpHandler::onHttpDataReceived( size_t size ) {
	if(mRequestMessage && !mRequestEnd)
		mHttpProcessor->recvBody( mRecvBuffer );
	if(mRequestEnd) {
		int threadCount = mThreadPool.size();
		int pendingCount = mThreadPool.pendingRequestSize();
		int activeCount = mThreadPool.activeCount();
		if( pendingCount > 1 ) {
			mLogger(ZQ::common::Log::L_INFO,CLOGFMT(C2AsyncHttpHandler,"thread pool size[%d], active count[%d], pending size[%d] "),
					threadCount, activeCount, pendingCount);
		}
		(new C2HttpHandlerRunner(this, mThreadPool))->start();
	}
}

void C2AsyncHttpHandler::callC2HttpHandler() {
	mRealHandler->onRequestEnd();
	mLogger(ZQ::common::Log::L_INFO,CLOGFMT(C2AsyncHttpHandler,"request [%s] accepted"),
			mRequestMessage->url().c_str() );
}

bool C2AsyncHttpHandler::onHttpMessage( const HttpMessagePtr msg ) {
	mRequestMessage = msg;
	mRealHandler = mRealFactory->create(msg->url().c_str());
	mLogger(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncHttpHandler,"got a request: %s"),msg->toRaw().c_str());
	if(!mRealHandler) {
		mLogger(ZQ::common::Log::L_WARNING, CLOGFMT(C2AsyncHttpHandler,"url[%s] is rejected by C2Handler"),msg->url().c_str());
		return false;
	}
	if(!mRealHandler->onConnected(*this)) {
		mLogger(ZQ::common::Log::L_WARNING, CLOGFMT(C2AsyncHttpHandler,"request [%s] is rejected by C2Handler"),msg->url().c_str());
		return false;
	}
	return mRealHandler->onRequest(*this,*this);	
}

bool C2AsyncHttpHandler::onHttpBody( const char* data, size_t size) {
	ZQHttp::PostDataFrag pd;
	pd.data = data;
	pd.len = size;
	return mRealHandler->onPostData(pd);
}

void C2AsyncHttpHandler::onHttpComplete() {
	mRequestEnd = true;
}

void C2AsyncHttpHandler::onHttpError( int err ) {
	mLastError = err;
	mLogger(ZQ::common::Log::L_ERROR, CLOGFMT(C2AsyncHttpHandler,"[%p] got an error[%d], system error[%d]"),this, err, errno);
	mRealHandler->onBreak();
	signal();
}

int64 C2AsyncHttpHandler::getId() const {
	return 0;
}

bool C2AsyncHttpHandler::setCommOption( int opt, int val) {
	switch( opt ) {
	case ZQHttp_OPT_WriteBufSize:
		{
			mHttpProcessor->setSendBufSize(val);
		}
		break;
	default:
		break;
	}
	return true;
}

C2AsyncHttpHandlerFactory::C2AsyncHttpHandlerFactory( HttpServer& server, ZQ::common::Log& logger, 
		ZQHttp::IRequestHandlerFactory* realFactory, size_t threadpoolSize)
:mServer(server),
mRealFactory(realFactory),
mLogger(logger){
	mThreadPool.resize(threadpoolSize);
}

IHttpHandler::Ptr C2AsyncHttpHandlerFactory::create( HttpProcessor::Ptr processor ) {
	return new C2AsyncHttpHandler(*this, mLogger, processor, mRealFactory, mThreadPool );
}



}//namespace C2Streamer

