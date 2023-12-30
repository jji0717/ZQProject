#ifndef __SessionTimerCommand_H__
#define __SessionTimerCommand_H__

#include "NativeThreadPool.h"
#include "XML_SessCtxHandler.h"
#include "SessionWatchDog.h"

class OnTimerRequest : public ZQ::common::ThreadRequest
{
public:
	/// constructor
	OnTimerRequest(::ZQ::common::NativeThreadPool &pool, ::ZQ::common::FileLog &fileLog, SessionMap &sessionMap, const ::std::string sessId);
	OnTimerRequest(::ZQ::common::NativeThreadPool &pool, ::ZQ::common::FileLog &fileLog, SessionMap &sessionMap,uint16 uCSeq);
	~OnTimerRequest();

protected: // impls of ScheduleTask
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
	
protected:
	uint64			_usedMilli;
	::std::string	_sessId;
	uint16			_uCSeq;
	SessionMap		&_sessionMap;
	::ZQ::common::FileLog	*_log;
};

#endif // __SessionTimerCommand_H__

