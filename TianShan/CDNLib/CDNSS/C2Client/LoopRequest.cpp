#include "LoopRequest.h"
#include <TimeUtil.h>

namespace ZQ
{
    namespace StreamService
    {
        LoopRequest::LoopRequest(ZQ::common::Config::Loader<C2ClientConf>& conf, ZQ::common::Log& log)
            : _conf(conf), _log(log), _currLoopIndex(0), _concurrentRequestPtr(NULL), _startLoopTime(0)
        {
        }

        LoopRequest::~LoopRequest()
        {
            if(_concurrentRequestPtr)
            {
                _concurrentRequestPtr = NULL;
            }
        }

        int LoopRequest::getCurrentLoopIndex()
        {
            return _currLoopIndex;
        }

        bool LoopRequest::isStop()
        {
            return _bStop;
        }

        void LoopRequest::printStatisticalData()
        {
            if (_concurrentRequestPtr)
            {
                _concurrentRequestPtr->printStatisticalData();
            }
        }

        bool LoopRequest::startRequest()
        {
            _startLoopTime = ZQ::common::TimeUtil::now();
            if (++_currLoopIndex > _conf.loop)
            {
                _bStop = true;
                if(_concurrentRequestPtr)
                {
                    _concurrentRequestPtr = NULL;
                }
                printf("all loop[%d] done!\n", _conf.loop);
                _log(ZQ::common::Log::L_INFO, CLOGFMT(LoopRequest, "startRequest() current loop[%d] reached max loop[%d]"), _currLoopIndex, _conf.loop);
                return false;
            }
            _concurrentRequestPtr = new ConcurrentRequest(_conf, _log, this, _currLoopIndex);

            if (!_concurrentRequestPtr)
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(LoopRequest, "startRequest() _concurrentRequestPtr is NULL"));
                return false;
            }
            return _concurrentRequestPtr->start();
        }

        void LoopRequest::onLoop()
        {
            printf("start next loop ... \n");
            int64 usedTime = ZQ::common::TimeUtil::now() - _startLoopTime;
            _log(ZQ::common::Log::L_INFO, CLOGFMT(LoopRequest, "onLoop() last loop used time[%ld]ms, start next loop"), usedTime);
            startRequest();
        }
    }
}
