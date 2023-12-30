#ifndef _C2CLIENT_H
#define _C2CLIENT_H

#include <LocateRequest.h>
#include <GetRequest.h>
#include <TransferDelete.h>
#include "SimpleXMLParser.h"
#include <IndexFileParser.h>

#ifdef ZQ_OS_MSWIN
#pragma comment(lib, "ws2_32.lib")
#endif//ZQ_OS_MSWIN

namespace ZQ{
namespace StreamService{
    typedef std::map<std::string, std::string> AttrMap;

    class C2ClientAsync;
    typedef ZQ::common::Pointer<C2ClientAsync> C2ClientAsyncPtr;
    class C2ClientBind;
    typedef ZQ::common::Pointer<C2ClientBind> C2ClientBindPtr;
    class C2ClientBind: public IC2RequestCallBack
    {
    public:
        C2ClientBind(ZQ::common::Log& log, RequestParams& params);
        virtual void onLocate(const LocateResponseData& resp);
        virtual void onData(const char* data, const size_t& size);
        virtual void onRecvComplete();
        virtual void onTransferDelete();
        virtual void onError(C2RequestErrorCategory category, const int& err, const std::string& msg);

        // response to c2ss
        virtual void OnC2LocateResponse(const std::string& contentName, const AttrMap& locRespParamters) = 0;
        virtual void OnC2GetResponse(const std::string& contentName, const AttrMap& locRespParamters) = 0;
        //virtual void OnC2IndexHeader(std::string& contentName, const uint8* buf, const int len) = 0;
        virtual void OnError(int errCode, const std::string& errMsg) = 0;

    private:
        bool parseIndex(std::string& contentName, const char* indexData, size_t dataSize, \
            ZQ::IdxParser::IndexData& idxData);

    public:
        ZQ::common::Log&    _log;
        ZQ::common::Mutex   _mutex;
        LocateRequest::Ptr	_locateClientPtr;
        GetRequest::Ptr 	_getClientPtr;

        RequestParams&       _params;
        std::string         _indexData;
    };

    //sync c2client
    class C2ClientSync : public ZQ::common::HttpClient, public ZQ::common::SharedObject
    {
    public:
        typedef std::map<std::string, std::string> AttrMap;
        typedef ZQ::common::Pointer<C2ClientSync> Ptr;
        C2ClientSync(ZQ::common::Log& log, const std::string& addr, const unsigned int port = 10080, const int timeout = 10*1000); //timeout default 10s
        ~C2ClientSync();

        bool sendLocateRequest(const std::string& url, const std::string& contentName, const std::string& subType, AttrMap& reqProp, AttrMap& respProp);
        bool sendGetRequest(const std::string& url, const std::string& contentName, AttrMap& reqProp, AttrMap& respProp);

    private:
        int  setLocateRequestBody();
        int  setGetRequestBody();

        bool parseIndex(std::string contentName, const char* indexData, size_t dataSize, ZQ::IdxParser::IndexData& idxData);

        int64 setTimeout(int64 timeout = 10*1000); //default timeout 10s
        bool parseResponse(const SimpleXMLParser::Node* root, LocateResponseData& respData, std::string& error);
        bool setRevcMap(LocateResponseData& respData);

    private:
        ZQ::common::Log&		_log;

        std::string				_addr;
        unsigned int			_port;

        AttrMap					_sendMap;
        AttrMap					_recvMap;

        int64						_timeout; //ms
    };
} // namespace StreamService
} // namespace ZQ
#endif //_C2CLIENT_H
