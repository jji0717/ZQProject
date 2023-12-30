#ifndef C2REQUEST_H
#define C2REQUEST_H
#include "LocateRequest.h"
#include "GetRequest.h"
#include "TransferDelete.h"

#include <IndexFileParser.h>

#ifdef ZQ_OS_LINUX
#include <AioFile.h>
#elif defined ZQ_OS_MSWIN
#include <AioFile_win.h>
#endif

namespace ZQ{
namespace StreamService{

    enum C2RequestPhase
    {
        phaseInit,
        phaseLocate,
        phaseGet,
        phaseTransferDelete,
        phaseDone
    };

    class IReadCB : virtual public ZQ::common::SharedObject
    {
    public:
        typedef ZQ::common::Pointer<IReadCB> Ptr;
        virtual ~IReadCB() { }

        virtual void onRead( const std::vector<C2Streamer::Buffer*>& bufs) = 0;
        virtual void onReadComplete( const std::string& key) = 0;
        virtual void onIndexInfo( C2Streamer::AssetAttribute::Ptr attr ) = 0;
        virtual void onLatency(std::string& fileName, int64 offset, int64 time) = 0; 
        virtual void onError(int err) = 0;
    };

    class AsyncRequest : public LibAsync::AsyncWork
    {
    public:
        AsyncRequest(RequestHandle::Ptr requestHandle, LibAsync::EventLoop* eventloop)
            : LibAsync::AsyncWork(*eventloop), _requestHandle(requestHandle)
        {
        }
        virtual ~AsyncRequest() {}

        void onAsyncWork()
        {
            _requestHandle->process();
        }

    private:
        RequestHandle::Ptr _requestHandle;
    };

    class C2ReadFile : public IC2RequestCallBack
    {
    public:
        typedef ZQ::common::Pointer<C2ReadFile> Ptr;
        C2ReadFile(IReadCB::Ptr readClient, ZQ::common::Log& log, RequestParams params, int64 bufferReqID = 0);
        virtual ~C2ReadFile();
    public:
        bool    addBuffer( const std::vector<C2Streamer::Buffer*>& bufs );
        int     getInstanceID();
        void    setBufferReqID(int64 reqid) { _bufferReqID = reqid;}
        int64   getBufferReqID() {return _bufferReqID;}

    private:
        C2Streamer::Buffer::ErrorCategory  convertErrorCategoty(C2RequestErrorCategory category);
        std::string getCurrPhaseStr();

        bool checkTempBuffer();
        bool startRequest();
        bool retry();
        int  getMaxRetry();

    public:
        virtual void onLocate(const LocateResponseData& resp);
        virtual void onData(const char* data, const size_t& size, bool error = false);
        virtual void onRecvComplete();
        virtual void onTransferDelete();
        virtual void onError(C2RequestErrorCategory category, const int& err, const std::string& msg);

    private:      
        static ZQ::common::AtomicInt _atomicID;
        int                 _requestInstID;
        std::string         _instanceKey;
        ZQ::common::Log&    _log;
        IReadCB::Ptr        _cb;
        RequestParams       _params;

        RequestHandle::Ptr  _requestHandle;
        C2RequestPhase      _phase;
        ZQ::common::Mutex   _mutex;
        
        std::vector<C2Streamer::Buffer*>   _readerBufs;

        std::string     _key;
        std::string     _tmpBuf;

        int             _currRetryCount;
        int64           _bufferReqID;
        std::string     _alignAbandonData;

    };

    class C2QueryIndex : public IC2RequestCallBack
    {
    public:
        typedef ZQ::common::Pointer<C2QueryIndex> Ptr;
        C2QueryIndex(IReadCB::Ptr readClient, ZQ::common::Log& log, RequestParams params, \
            C2Streamer::AssetAttribute::Ptr attr);

        virtual ~C2QueryIndex();

    public:
        bool startRequest();
        int  getInstanceID();
        int64   getReqID() {return _reqID;}

    private:
        bool parseIndex(std::string& contentName, const char* indexData, size_t dataSize, \
            ZQ::IdxParser::IndexData& idxData);
        C2Streamer::AssetAttribute::LASTERR   convertErrorCategoty(C2RequestErrorCategory category);

    public:
        virtual void onLocate(const LocateResponseData& resp);
        virtual void onData(const char* data, const size_t& size, bool error = false);
        virtual void onRecvComplete();
        virtual void onTransferDelete();
        virtual void onError(C2RequestErrorCategory category, const int& err, const std::string& msg);

    private:
        static ZQ::common::AtomicInt _atomicID;
        int                 _queryIndexInstID;
        int64               _reqID;
        std::string         _instanceKey;
        ZQ::common::Log&    _log;
        IReadCB::Ptr        _cb;
        RequestParams       _params;

        RequestHandle::Ptr  _requestHandle;
        C2RequestPhase      _phase;
        ZQ::common::Mutex   _mutex;


        LocateResponseData  _locateData;
        std::string         _indexData;
        C2Streamer::AssetAttribute::Ptr _assetAttr;
        std::vector<C2Streamer::Buffer*>   _readerBufs;
    };
}//namespace ZQ
} //namespace StreamService
#endif