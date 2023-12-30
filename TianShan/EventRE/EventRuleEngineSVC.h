#ifndef __TianShan_EventRuleEngine_Service_H__
#define __TianShan_EventRuleEngine_Service_H__

#include "ZQ_common_conf.h"
#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include "EventChannel.h"
#include "IceLog.h"
#include "EventRuleEngine.h"

class EventRuleEngineSVC : public ZQ::common::BaseZQServiceApplication
{
public:
	EventRuleEngineSVC();
	virtual ~EventRuleEngineSVC();
	virtual HRESULT OnInit(void);
	virtual HRESULT OnStop(void);
	virtual HRESULT OnStart(void);
	virtual HRESULT OnUnInit(void);		
private:	
	Ice::CommunicatorPtr							_communicator;
	::TianShanIce::common::IceLogIPtr				_icelog;
	Ice::PropertiesPtr								_properties;
	EventRuleEngine*								_pRuleEngine;
	// native thread pool	
	ZQ::common::NativeThreadPool*					_pThreadPool;
};
#endif
