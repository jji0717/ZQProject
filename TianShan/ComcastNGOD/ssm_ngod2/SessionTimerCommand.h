#ifndef __SessionTimerCommand_H__
#define __SessionTimerCommand_H__

#include "../../common/TianShanDefines.h"
#include "NativeThreadPool.h"
#include <Ice/Ice.h>

class NGODEnv;

class SessionTimerCommand : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    SessionTimerCommand(NGODEnv& env, const ::Ice::Identity& sessIdent);

protected: // impls of ScheduleTask
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
	
protected:
	::Ice::Long _usedMilli;
	NGODEnv&		_env;
	::Ice::Identity		_identSess;
};

#endif // __SessionTimerCommand_H__

