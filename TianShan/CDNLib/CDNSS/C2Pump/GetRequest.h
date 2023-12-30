#ifndef _C2_GET_REQUEST_H
#define _C2_GET_REQUEST_H
#include "RequestHandle.h"

namespace ZQ{
namespace StreamService{

    class GetRequest : public RequestHandle
    {
    public:
        typedef ZQ::common::Pointer<GetRequest> Ptr;

        GetRequest(ZQ::common::Log& log, IC2RequestCallBack::Ptr requestCB, RequestParams params, std::string instanceKey = "", int64 bufferReqID = -1);

        virtual ~GetRequest();

        SLABPOOL_DEFINE(GetRequest)
        
        bool process();

        void waitBuffer(int64 timeout);
        void continueRecv();

    public:
        virtual void onHttpDataReceived( size_t size );
        virtual bool onHttpBody( const char* data, size_t size);
        virtual void onHttpError( int error );
        virtual void onTimer();

    private:
        static ZQ::common::AtomicInt _atomicID;
        char        _data[RECV_BUFFER_SIZE];
        size_t      _dataSize;
        bool        _bWaitBuffer;
        uint64      _receivedDataSize;

        ZQ::common::Mutex   _mutex;
    };

}//namespace ZQ
} //namespace StreamService
#endif
