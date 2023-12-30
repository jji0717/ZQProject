
#ifndef __EVENTRE_SERVICE_H__
#define __EVENTRE_SERVICE_H__

#include "EventRuleEngine.h"
#include "ZQDaemon.h"
#include "IceLog.h"
#include <Ice/Ice.h>

class EventREService : public ZQ::common::ZQDaemon
{
public:
    EventREService();
    ~EventREService();

    virtual bool OnInit(void);
    virtual bool OnStart(void);
    virtual void OnStop(void);
    virtual void OnUnInit(void);

private:
	Ice::CommunicatorPtr							_communicator;
	::TianShanIce::common::IceLogIPtr				_icelog;
	Ice::PropertiesPtr								_properties;
	EventRuleEngine*								_pRuleEngine;
	// native thread pool	
	ZQ::common::NativeThreadPool*					_pThreadPool;

};

#endif //__EVENTRE_SERVICE_H__

