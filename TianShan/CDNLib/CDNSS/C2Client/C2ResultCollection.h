#ifndef _ZQ_C2_RESULT_COLLECTION
#define _ZQ_C2_RESULT_COLLECTION
#include "C2Client.h"
#include "ILoopRequest.h"

namespace ZQ{
    namespace StreamService{

        typedef std::map<int64, StatisticData> Collectors;
        class C2ResultCollection : public IC2CallBack
        {
        public:
            typedef ZQ::common::Pointer<C2ResultCollection> Ptr;
            C2ResultCollection(ZQ::common::Log& log, ILoopRequest::Ptr cb, int64 totalNum);
            virtual ~C2ResultCollection();

            void printStatisticalData();
            int64 getCurrSessionNum();

            void onStart(const int64 id, const StatisticData statisticData);
            void onProcess(const int64 id, const StatisticData statisticData);
            void onSuccess(const int64 id, const StatisticData statisticData);
            void onError(const int64 id, const StatisticData statisticData);

        private:
             ZQ::common::Mutex _mutex;
             ZQ::common::Log&  _log;
             bool   _bStop;
             bool   _bFirst;
             int32  _printInterval;
             int64  _processingNum;
             int64  _successNum;
             int64  _failedNum;
             int64  _totalNum;
             int64  _startTime;
             int64  _lastSessionNumber;
             int64  _processBarPointNum;
             Collectors _statisticDatas;
             ILoopRequest::Ptr _loopRequestCB;
        };
    }
}

#endif
