#ifndef __TianShan_EventChannelSvc_H__
#define __TianShan_EventChannelSvc_H__

#include <ZQDaemon.h>
#include "EmbeddedIceStorm.h"
#include "Sentinel.h"

class EventChannelService : public ZQ::common::ZQDaemon
{
public:
    EventChannelService();
    ~EventChannelService();

    virtual bool OnInit(void);
    virtual bool OnStart(void);
    virtual void OnStop(void);
    virtual void OnUnInit(void);		
    void restartIceStorm();
private:	
    EmbeddedIceStorm *_pIceStorm;
    ZQ::common::FileLog *_pIceTraceLog;
    EventChannel::Sentinel *_pSentinel;
    bool _bInService;
};

#endif

