#ifndef __TIANSHAN_EVENTGATEWAY_SERVICE_H__
#define __TIANSHAN_EVENTGATEWAY_SERVICE_H__

#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include <Ice/Ice.h>

namespace EventGateway{
class EventGw;
class ModuleManager;

class EventGwService : public ZQ::common::BaseZQServiceApplication
{
public:
    EventGwService();
    virtual ~EventGwService();
    virtual HRESULT OnInit(void);
    virtual HRESULT OnStop(void);
    virtual HRESULT OnStart(void);
    virtual HRESULT OnUnInit(void);
    // virtual void OnSnmpSet(const char*);

	void doEnumSnmpExports();

public:
	uint32 getLogLevel_Ice();
	void   setLogLevel_Ice(const uint32& newLevel);

    
private:

    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;

    EventGw *_gw;
    ModuleManager *_moduleMgr;
    ZQ::common::FileLog *_iceTrace;
};
} // namespace EventGateway
#endif
