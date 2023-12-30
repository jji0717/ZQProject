#ifndef __ZQEventRuleEngine_ThreadRequest_H__
#define __ZQEventRuleEngine_ThreadRequest_H__

#include <NativeThread.h>
#include <NativeThreadPool.h>
#include "EventRuleEngine.h"
#include "SystemUtils.h"

class ConnectEventChannelRequest : public ZQ::common::ThreadRequest
{
public:
	ConnectEventChannelRequest(EventRuleEngine& ruleEng, NativeThreadPool& pool);
	~ConnectEventChannelRequest();

protected: // impls of ScheduleTask
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
	EventRuleEngine&	_ruleEng;
    SYS::SingleObject	_event;
	bool				_bExit;
}; // class ConnectEventChannelRequest
#endif

