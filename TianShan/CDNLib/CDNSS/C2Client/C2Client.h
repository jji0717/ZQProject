#ifndef _ZQ_C2Clinet_H
#define _ZQ_C2Clinet_H
#include "LocateRequest.h"
#include "GetRequest.h"
#include "TransferDelete.h"
#include <md5.h>

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

        struct StatisticData
        {
            std::string md5;
            std::string filename;
            int64 reqBitrate;
            int64 recvBitrate;
            int64 usedtime;
            int64 requetSize;
            int64 receivedSize;
            int32 errorCode;
            std::string errMessage;
            bool  locateSuccess;
            int   currRetryNum;
            int   maxRetryNum;

            StatisticData()
            {
                md5 = "";
                filename = "";
                reqBitrate = 0;
                recvBitrate = 0;
                usedtime = 0;
                requetSize = 0;
                receivedSize = 0;
                currRetryNum = 0;
                maxRetryNum = 0;
                errorCode = 0;
                errMessage = "";
                locateSuccess = false;
            }
        };

        class IC2CallBack : public virtual ZQ::common::SharedObject
        {
        public:
            typedef ZQ::common::Pointer<IC2CallBack> Ptr;
            IC2CallBack(){}
            virtual ~IC2CallBack(){}
            virtual void onStart(const int64 id, const StatisticData statisticData) = 0;
            virtual void onProcess(const int64 id, const StatisticData statisticData) = 0;
            virtual void onSuccess(const int64 id, const StatisticData statisticData) = 0;
            virtual void onError(const int64 id, const StatisticData statisticData) = 0;
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

        class C2Client : public IC2RequestCallBack
        {
        public:
            typedef ZQ::common::Pointer<C2Client> Ptr;
            C2Client(ZQ::common::Log& log, IC2CallBack::Ptr cb, RequestParams params, bool bSaveFile = false);
            virtual ~C2Client();

        public:
            bool startRequest();

        private:
            std::string getCurrPhaseStr();
            int  getMaxRetry();
            bool retry();

        public:
            virtual void onLocate(const LocateResponseData& resp);
            virtual void onData(const char* data, const size_t& size, bool error = false);
            virtual void onRecvComplete();
            virtual void onTransferDelete();
            virtual void onError(C2RequestErrorCategory category, const int& err, const std::string& msg);

        private:      
            static ZQ::common::AtomicInt _atomicID;
            int64   _id;
            bool    _bSaveFile;
            ZQ::common::Log&    _log;
            RequestHandle::Ptr  _requestHandle;
            C2RequestPhase      _phase;
            RequestParams       _params;
            int                 _currRetryCount;
            StatisticData       _statisticData;
            int64               _startTime;
            IC2CallBack::Ptr    _cb;
            int64               _startRange;
            int64               _endRange;
            int64               _abadonLength;
            ZQ::common::md5*    _md5;
        };
    } // namespace StreamService
}// namespace ZQ

#endif