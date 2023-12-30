#ifndef _C2_TRANSFER_DELETE_REQUEST_H
#define _C2_TRANSFER_DELETE_REQUEST_H
#include <RequestHandle.h>

namespace ZQ{
namespace StreamService{

    class TransferDelete : public RequestHandle
    {
    public:
        typedef ZQ::common::Pointer<TransferDelete> Ptr;
        TransferDelete(ZQ::common::Log& log, IC2RequestCallBack::Ptr requestCB, RequestParams params, std::string instanceKey = "", int64 bufferReqID = -1);
        virtual ~TransferDelete();

        SLABPOOL_DEFINE(TransferDelete)
        
        bool process();

    private:
        std::string generateTransferDeleteBody(const std::string& transferID);

    public:
        virtual void onHttpDataReceived( size_t size );
        virtual bool onHttpBody( const char* data, size_t size);

    private:
        static ZQ::common::AtomicInt _atomicID;
    };

}//namespace ZQ
} //namespace StreamService
#endif