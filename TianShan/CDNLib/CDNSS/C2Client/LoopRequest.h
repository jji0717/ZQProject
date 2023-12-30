#ifndef ZQ_C2CLIENT_LOOP_REQUEST_H
#define ZQ_C2CLIENT_LOOP_REQUEST_H
#include "C2ClientConf.h"
#include "ILoopRequest.h"
#include "ConcurrentRequest.h"

namespace ZQ{
    namespace StreamService{
        class LoopRequest : public ILoopRequest
        {
        public:
            typedef ZQ::common::Pointer<LoopRequest> Ptr;
            LoopRequest(ZQ::common::Config::Loader<C2ClientConf>& conf, ZQ::common::Log& log);
            virtual ~LoopRequest();

            bool isStop();
            bool startRequest();
            void printStatisticalData();

            virtual void onLoop();
            virtual int getCurrentLoopIndex();

        private:
            int _currLoopIndex;
            int64 _startLoopTime;
            bool _bStop;
            ZQ::common::Log& _log;
            ZQ::common::Config::Loader<C2ClientConf>& _conf;
            ConcurrentRequest::Ptr  _concurrentRequestPtr;
        };
    }
}
#endif