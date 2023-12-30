#ifndef _tianshan_cdnlib_cdnss_c2streamer_libasync_bridge_header_file_h__
#define _tianshan_cdnlib_cdnss_c2streamer_libasync_bridge_header_file_h__

#include <assert.h>
#include <map>
#include <boost/thread.hpp>
#include <libasync/http.h>
#include <urlstr.h>
#include <NativeThreadPool.h>
#include "./C2HttpHandler.h"

using namespace LibAsync;

namespace C2Streamer {

class C2AsyncHttpHandlerFactory;

class PoorSemaphore {
public:
	PoorSemaphore(unsigned int v) {
		int rc = sem_init(&mSem, 0, v);
		assert(rc == 0 );
	}
	~PoorSemaphore() {
		int rc = sem_destroy(&mSem);
		assert(rc==0);
	}
	void post() {
		int rc = sem_post(&mSem);
		assert(rc == 0 );
	}
	void wait() {
		while(true) {
			int rc = sem_wait(&mSem);
			if (rc < 0 && (errno == EINTR || errno == EAGAIN ) )
				continue;
			assert(rc == 0);
			break;
		}
	}
private:
	sem_t 		mSem;
};

class C2AsyncHttpHandler : public virtual LibAsync::IHttpHandler,  public ZQHttp::IConnection, public ZQHttp::IRequest, public ZQHttp::IResponse {
public:
	C2AsyncHttpHandler(C2AsyncHttpHandlerFactory& fac, ZQ::common::Log& logger, HttpProcessor::Ptr processor, 
			ZQHttp::IRequestHandlerFactory* realFactory, 
			ZQ::common::NativeThreadPool& pool);
	virtual ~C2AsyncHttpHandler();
	typedef ZQ::common::Pointer<C2AsyncHttpHandler> Ptr;
protected:
	virtual void setStatus(int statusCode, const char* reasonPhrase = NULL);
    // set a NULL value to clear the header
    virtual void setHeader(const char* hdr, const char* val);
    virtual bool headerPrepared();
    virtual bool addContent(const char* data, size_t len);
    virtual bool complete();

    virtual void sendDefaultErrorPage(int statusCode);
    virtual const char* getLastError() const;

	virtual void onWritable() {}
	virtual uint32 setFlags(uint32 flags, bool enable=true){}

protected:
    virtual ZQHttp::Method method() const;
    virtual const char* version() const;
    virtual const char* uri() const;
    virtual const char* getFullUri() const;
    virtual const char* queryArgument(const char* q) const;
    virtual std::map<std::string, std::string> queryArguments() const;
    virtual const char* header(const char* h) const;


protected:
    virtual bool getLocalEndpoint(std::string& address, int& port) const;
    virtual bool getRemoteEndpoint(std::string& address, int& port) const;
    virtual int64 getId() const;
    virtual bool setCommOption(int opt, int val);

protected:
	virtual void onHttpDataSent( size_t size);

	virtual void onHttpDataReceived( size_t size );

	virtual bool onHttpMessage( const HttpMessagePtr msg);
	
	virtual bool onHttpBody( const char* data, size_t size);

	virtual void onHttpComplete();

	virtual void onHttpError( int error );


private:
	friend class C2HttpHandlerRunner;
	void waitSignal();
	void signal();

	void callC2HttpHandler();
	void postponeClose();

	class C2HttpHandlerRunner : public ZQ::common::ThreadRequest {
	public:
		C2HttpHandlerRunner(C2AsyncHttpHandler::Ptr handler, ZQ::common::NativeThreadPool& pool)
			:ZQ::common::ThreadRequest(pool), mHandler(handler) {
		}
		virtual ~C2HttpHandlerRunner() {
		}
		int run() {
			mHandler->callC2HttpHandler();
			return 0;
		}
		void final( int,bool ) {
			delete this;
		}
	private:
		C2AsyncHttpHandler::Ptr mHandler;
	};
	class HttpProcessorCleaner : public LibAsync::AsyncWork {
	public:
		HttpProcessorCleaner( HttpProcessor::Ptr processor)
			:LibAsync::AsyncWork(processor->getLoop()),
			mHttpProcessor(processor) {}

		virtual ~HttpProcessorCleaner() {}
		typedef ZQ::common::Pointer<HttpProcessorCleaner> Ptr;
	private:
		virtual void onAsyncWork () {
			mHttpProcessor = NULL;
		}
	private:
		HttpProcessor::Ptr	mHttpProcessor;
	};

private:
	C2AsyncHttpHandlerFactory&		mFac;
	ZQ::common::NativeThreadPool&	mThreadPool;
	HttpMessagePtr					mRequestMessage;
	HttpMessagePtr					mResponseMessage;
	HttpProcessor::Ptr				mHttpProcessor;

	ZQHttp::IRequestHandlerFactory*	mRealFactory;
	ZQHttp::IRequestHandler*		mRealHandler;
	ZQ::common::URLStr				mUrl;

	int 							mLastError;
	AsyncBuffer						mRecvBuffer;

	PoorSemaphore 					mSema;

	ZQ::common::Log&				mLogger;
	bool							mRequestEnd;
	bool							mResponseHeaderSent;
};

class C2AsyncHttpHandlerFactory : public LibAsync::IHttpHandlerFactory {
public:
	C2AsyncHttpHandlerFactory( LibAsync::HttpServer& server, ZQ::common::Log& logger,  
			ZQHttp::IRequestHandlerFactory* realFactory, size_t threadpoolSize);
	virtual ~C2AsyncHttpHandlerFactory() {
	}
protected:
	virtual IHttpHandler::Ptr create(HttpProcessor::Ptr processor);
private:
	LibAsync::HttpServer&				mServer;
	ZQ::common::NativeThreadPool		mThreadPool;
	ZQHttp::IRequestHandlerFactory*		mRealFactory;
	ZQ::common::Log&					mLogger;
};
							

}//namespace C2Streamer

#endif//_tianshan_cdnlib_cdnss_c2streamer_libasync_bridge_header_file_h__

