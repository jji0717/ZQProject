#ifndef ZQ_C2CLIENT_PRINT_HANDLE_H
#define ZQ_C2CLIENT_PRINT_HANDLE_H
#include <NativeThread.h>
#include "C2ResultCollection.h"
#include "LoopRequest.h"

namespace ZQ{
namespace StreamService
{
    class OutputHandle : public ZQ::common::NativeThread, public virtual ZQ::common::SharedObject
    {
    public:
        typedef ZQ::common::Pointer<OutputHandle> Ptr;
        OutputHandle(ZQ::common::Log& log, int32 printInterval, LoopRequest::Ptr loopRequestPtr);
        virtual ~OutputHandle();

    public:
        virtual int run();
        bool isQuit();

    private:
        void outputStatisticData();

    private:
        ZQ::common::Log&  _log;
        int32 _printInterval;
        bool  _bQuit;
        LoopRequest::Ptr _loopRequestPtr;
    };
}
}

#endif
