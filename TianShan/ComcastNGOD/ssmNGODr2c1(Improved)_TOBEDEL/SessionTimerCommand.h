#ifndef __SessionTimerCommand_H__
#define __SessionTimerCommand_H__

#include "../../common/TianShanDefines.h"
#include "NativeThreadPool.h"
#include <Ice/Ice.h>

class ssmNGODr2c1;

class SessionTimerCommand : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    SessionTimerCommand(ssmNGODr2c1& env, const ::Ice::Identity& sessIdent);

protected: // impls of ScheduleTask
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
	
protected:
	::Ice::Long _usedMilli;
	ssmNGODr2c1&		_env;
	::Ice::Identity		_identSess;
};

#endif // __SessionTimerCommand_H__

