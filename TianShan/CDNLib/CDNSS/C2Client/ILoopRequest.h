#ifndef ZQ_C2CLIENT_LOOP_REQUEST_INTERFACE_H
#define ZQ_C2CLIENT_LOOP_REQUEST_INTERFACE_H
#include <Pointer.h>

namespace ZQ{
    namespace StreamService{
        class ILoopRequest : public virtual ZQ::common::SharedObject
        {
        public:
            typedef ZQ::common::Pointer<ILoopRequest> Ptr;
            ILoopRequest(){}
            virtual ~ILoopRequest(){}

            virtual int getCurrentLoopIndex() = 0;
            virtual void onLoop() = 0;
        };
    }
}
#endif