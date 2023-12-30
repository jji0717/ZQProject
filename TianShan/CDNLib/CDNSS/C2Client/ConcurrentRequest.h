#ifndef ZQ_C2CLIENT_CONCURRENT_REQUEST_H
#define ZQ_C2CLIENT_CONCURRENT_REQUEST_H
#include "C2Client.h"
#include "C2ResultCollection.h"
#include "C2ClientConf.h"
#include "ILoopRequest.h"
#include <NativeThread.h>

namespace ZQ{
    namespace StreamService{
        class ConcurrentRequest : public ZQ::common::NativeThread, public virtual ZQ::common::SharedObject
        {
        public:
            typedef ZQ::common::Pointer<ConcurrentRequest> Ptr;
            ConcurrentRequest(ZQ::common::Config::Loader<ZQ::StreamService::C2ClientConf>& conf,
                ZQ::common::Log& log, ILoopRequest::Ptr cb, int loopIndex);
            virtual ~ConcurrentRequest();

            void printStatisticalData();

            virtual int run(void);

        private:
            int _currLoop;
            int _client;
            int _fileNumber;
            bool _bCompleted;
            ZQ::common::Log& _log;
            ZQ::common::Config::Loader<ZQ::StreamService::C2ClientConf>& _conf;
            C2ResultCollection::Ptr _resultCollectionPtr;
            ILoopRequest::Ptr       _cb;
        };
    }
}
#endif