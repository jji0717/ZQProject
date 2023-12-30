#include "C2ResultCollection.h"
#include <TimeUtil.h>

namespace ZQ
{
	namespace StreamService
	{
		C2ResultCollection::C2ResultCollection(ZQ::common::Log& log, ILoopRequest::Ptr cb, int64 totalNum)
            : _log(log), _loopRequestCB(cb), _processingNum(0), _successNum(0),
            _failedNum(0), _totalNum(totalNum), _bStop(false), _startTime(0), _lastSessionNumber(0), _processBarPointNum(0), _bFirst(true)
		{
            _startTime = ZQ::common::TimeUtil::now();
		}

		C2ResultCollection::~C2ResultCollection()
		{
            if(_loopRequestCB)
            {
                _loopRequestCB = NULL;
            }
		}

        int64 C2ResultCollection::getCurrSessionNum()
        {
            ZQ::common::MutexGuard gd(_mutex);
            return _processingNum;
        }

		void C2ResultCollection::printStatisticalData()
        {
            ZQ::common::MutexGuard gd(_mutex);
            std::string strProcessBar;
            std::string prompt;
            if(_lastSessionNumber == getCurrSessionNum() && !_bFirst)
            {
                prompt = "downloading...  ";
                _processBarPointNum = (++_processBarPointNum) % 4;
                switch(_processBarPointNum)
                {
                case 0:
                    strProcessBar = "|";
                    break;
                case 1:
                    strProcessBar = "/";
                    break;
                case 2:
                    strProcessBar = "-";
                    break;
                case 3:
                    strProcessBar = "\\";
                    break;
                default:
                    break;
                }
            }

            int64 usedTime = (ZQ::common::TimeUtil::now() - _startTime) / 1000;
            printf("[loop:%d] current session number: %ld\tused time[%ld]s  %s\033[32m%s\033[0m\r", _loopRequestCB->getCurrentLoopIndex(), getCurrSessionNum(), usedTime, prompt.c_str(), strProcessBar.c_str());
            fflush(stdout);
            _lastSessionNumber = getCurrSessionNum();
            _bFirst = false;
        }

        void C2ResultCollection::onStart(const int64 id, const StatisticData statisticData)
        {
            ZQ::common::MutexGuard gd(_mutex);
            _statisticDatas.insert(Collectors::value_type(std::make_pair(id, statisticData)));
            if (statisticData.currRetryNum == 0)
            {
                _processingNum++;
            }
        }

        void C2ResultCollection::onProcess(const int64 id, const StatisticData statisticData)
        {
            ZQ::common::MutexGuard gd(_mutex);
            _statisticDatas[id] = statisticData;
        }
        void C2ResultCollection::onSuccess(const int64 id, const StatisticData statisticData)
        {
            ZQ::common::MutexGuard gd(_mutex);
           /* Collectors::iterator it = _statisticDatas.find(id);
            if (it != _statisticDatas.end())
            {
                _statisticDatas.erase(it);
            }*/
            _statisticDatas[id] = statisticData;
            _successNum++;
            _processingNum--;
            printf("[loop:%d - %ld/%ld/%ld/%ld] file[%s] md5[%s] bitrate[%ld]bps received size[%ld] used time[%ld] successful\n",
                _loopRequestCB->getCurrentLoopIndex(), _failedNum, _successNum, _processingNum, _totalNum, statisticData.filename.c_str(), statisticData.md5.c_str(),
                statisticData.recvBitrate, statisticData.receivedSize, statisticData.usedtime);

            if (_successNum + _failedNum == _totalNum)
            {
                int64 usedTime = ZQ::common::TimeUtil::now() - _startTime;
                printf("\nloop: %d completed, success[%ld] failed[%ld] used time[%ld]ms\n\n", _loopRequestCB->getCurrentLoopIndex(), _successNum, _failedNum, usedTime);
                _loopRequestCB->onLoop();
            }
        }
        void C2ResultCollection::onError(const int64 id, const StatisticData statisticData)
        {
            ZQ::common::MutexGuard gd(_mutex);
            /*Collectors::iterator it = _statisticDatas.find(id);
            if (it != _statisticDatas.end())
            {
                _statisticDatas.erase(it);
            }*/
            _statisticDatas[id] = statisticData;
            _failedNum++;
            if (statisticData.locateSuccess)
            {
                _processingNum--;
            }

            printf("[loop:%d - %ld/%ld/%ld/%ld] Error occured, file[%s] [%d:%s] failed\n", _loopRequestCB->getCurrentLoopIndex(), _failedNum, _successNum, _processingNum, _totalNum, statisticData.filename.c_str(), statisticData.errorCode, statisticData.errMessage.c_str());

            if (_successNum + _failedNum == _totalNum)
            {
                int64 usedTime = ZQ::common::TimeUtil::now() - _startTime;
                printf("\nloop: %d completed, success[%ld] failed[%ld] used time[%ld]ms\n\n", _loopRequestCB->getCurrentLoopIndex(), _successNum, _failedNum, usedTime);
                _loopRequestCB->onLoop();
            }
        }
	}
}
