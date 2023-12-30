#ifndef __TIANSHAN_EVENTGATEWAY_SERVICE_H__
#define __TIANSHAN_EVENTGATEWAY_SERVICE_H__
#include "ZQDaemon.h"
#include <Ice/Ice.h>

namespace EventGateway{
class EventGw;
class ModuleManager;

class EventGwService : public ZQ::common::ZQDaemon
{
public:
    EventGwService();
    virtual ~EventGwService();
    virtual bool OnInit();
    virtual bool OnStart();
    virtual void OnStop();
    virtual void OnUnInit();
    
private:

    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;

    EventGw *_gw;
    ModuleManager *_moduleMgr;
    ZQ::common::FileLog *_iceTrace;
};
} // namespace EventGateway
#endif
