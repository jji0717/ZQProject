#include "OutputHandle.h"

namespace ZQ{
namespace StreamService
{
    OutputHandle::OutputHandle(ZQ::common::Log& log, int32 printInterval, LoopRequest::Ptr loopRequestPtr)
        : _log(log), _printInterval(printInterval), _loopRequestPtr(loopRequestPtr), _bQuit(false)
    {

    }

    OutputHandle::~OutputHandle()
    {
        if(_loopRequestPtr)
        {
            _loopRequestPtr = NULL;
        }
    }

    void OutputHandle::outputStatisticData()
    {
        _loopRequestPtr->printStatisticalData();
    }

    bool OutputHandle::isQuit()
    {
         return _bQuit;
    }

    int OutputHandle::run()
    {
        while(!_loopRequestPtr->isStop())
        {
            outputStatisticData();
            SYS::sleep(_printInterval);
        }
        _bQuit = true;
        return 0;
    }
}
}
