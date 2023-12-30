#ifndef _ZQ_C2Streamer_CacheServer_Request_H
#define _ZQ_C2Streamer_CacheServer_Request_H
#include <eventloop.h>
#include <TianShanIce.h>
#include <TsStreamer.h>
#include <Pointer.h>

namespace C2Streamer
{
typedef std::vector<std::string> Strings;
struct ConnInfo
{
    std::string peerIP;
    int         peerPort;
    std::string localIP;
    int         localPort;
    std::string uri;
    std::string connID;
};

struct CacheLocatorRequestData
{
    std::string assetId;
    std::string providerId;
    std::string subType;
    std::string extension; // file extension of the member file
    int64 transferRate;

    std::string clientTransfer;
    int64 ingressCapacity;

    Strings exclusionList;
    std::string range;
    int64 transferDelay;
    CacheLocatorRequestData() {
        transferRate = 0;
        ingressCapacity = 0;
        transferDelay = 0;
    }
};

struct CacheLocatorResponseData
{
    std::string transferPort;
    std::string transferID;
    std::string transferTimeout;
    std::string availableRange;
    std::string openForWrite;
    std::string portNumber;
    std::string idxContentGeneric;
    std::string idxContentSubfiles;
};

class C2StreamerEnv;
class C2Service;
class HandlerC2Locate;
//typedef ZQ::common::Pointer<HandlerC2Locate> LocatePtr;
class C2LocateCB : public LibAsync::AsyncWork {
public:
    typedef ZQ::common::Pointer<C2LocateCB> Ptr;
    C2LocateCB( LibAsync::EventLoop* loop, HandlerC2Locate* req, const std::string connid);
    virtual ~C2LocateCB();

    bool                callbackDone() const {
        ZQ::common::MutexGuard gd(mLocker);
        return mbCbDone;
    }

    bool                reqValid() const {
        ZQ::common::MutexGuard gd(mLocker);
        return mbReqValid;
    }

    bool                tendToDestroyReq( ) {
        ZQ::common::MutexGuard gd(mLocker);
        if(mbCbDone)
            return false;
        mbReqValid = false;
        return true;
    }
	void                setLocateRequestPtrToNull();
    void                postResponse( int code, const std::string& message );

protected:

    virtual void onAsyncWork( );

private:
	HandlerC2Locate*		mLocateRequestPtr;
    bool               		mbReqValid;
    bool               		mbCbDone;
    ZQ::common::Mutex   	mLocker;
    int                     mHttpCode;
    std::string             mMessage;
	std::string             mConnId;
	int                     mPostCount;
};

class CacheServerRequest : public ZQ::common::ThreadRequest
{
public:
    CacheServerRequest(C2StreamerEnv& env, C2Service& svc, C2LocateCB::Ptr locateCB, ConnInfo connInfo, const std::string& content, const ZQHttp::IRequest* req);
    virtual ~CacheServerRequest();

    virtual int run();
    bool resolveObject(const std::string& identifier, std::string& providerId, std::string& assetId, std::string& extension) const;

private:
    bool parseRequestData(std::string content, CacheLocatorRequestData& reqData);
    bool getStreamProperty(::TianShanIce::Streamer::StreamPrx stream, ::TianShanIce::Properties& props);
    std::string generateResponseXML(const CacheLocatorResponseData& respData);
    bool sendResponseData(const ::TianShanIce::Properties& props);
	bool locateForward( std::string& sResponse );

private:
    C2StreamerEnv&      mEnv;
    C2Service&          mSvc;
    C2LocateCB::Ptr     mLocateCb;
    std::string         mContent;
    std::string         mRespData;
    ConnInfo            mConnInfo;
	CacheLocatorRequestData reqData;
	CacheLocatorResponseData respData;
    ::TianShanIce::Streamer::StreamPrx mStream;
	std::string         mForwardUrl;
	int                 mForwardStatus;
	const ZQHttp::IRequest*  mRequest;
};

} // namespace C2Streamer
#endif //_ZQ_C2Streamer_CacheServer_Request_H
