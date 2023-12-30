// simulate the IceBox header
// hack from <IceBox/IceBox.h>
#ifndef __TianShan_EventChannel_SimBox_H__
#define __TianShan_EventChannel_SimBox_H__

#include <Ice/Ice.h>
#include <IceUtil/Config.h>

#if ICE_INT_VERSION/100 > 303
#error **CAUTION** Must care the implementation of IceBox!
#endif
namespace IceBox
{

class Service : virtual public ::Ice::LocalObject
{
public:

    virtual void start(const ::std::string&, const ::Ice::CommunicatorPtr&, const ::Ice::StringSeq&) = 0;

    virtual void stop() = 0;
};
}
#endif