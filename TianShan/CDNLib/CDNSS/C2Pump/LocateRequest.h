#ifndef _C2_LOCATE_REQUEST_H
#define _C2_LOCATE_REQUEST_H
#include "RequestHandle.h"
#include "SimpleXMLParser.h"

namespace ZQ{
namespace StreamService{

    class LocateRequest : public RequestHandle
    {
    public:
        typedef ZQ::common::Pointer<LocateRequest> Ptr;
        LocateRequest(ZQ::common::Log& log, IC2RequestCallBack::Ptr requestCB, RequestParams params, std::string instanceKey = "", int64 bufferReqID = -1);
        virtual ~LocateRequest();

        SLABPOOL_DEFINE(LocateRequest)
        
        bool process();

    public:
        virtual void onHttpDataReceived( size_t size );
        virtual bool onHttpBody( const char* data, size_t size);

    private:
        std::string generateLocateBody();
        bool parseResponse(const SimpleXMLParser::Node* root, LocateResponseData& respData, std::string& error);

    private:
        static ZQ::common::AtomicInt _atomicID;
        std::string     _paid;
        std::string     _pid;
    };

}//namespace ZQ
} //namespace StreamService
#endif