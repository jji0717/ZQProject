


#ifndef _ERROR_PROCESS_H_
#define _ERROR_PROCESS_H_

#include <string>
#include "NativeThreadPool.h"


class ErrorProcFunc
{
public:
	virtual ~ErrorProcFunc(){}
	virtual bool stopReceiveRequest() = 0;
	virtual bool restartApplication() = 0;
	virtual bool canRestartApplication() = 0;
};

class ErrorProcess
{
public:
	ErrorProcess(ErrorProcFunc* pProcFunc);

	void setQuery(ErrorProcFunc* pProcFunc);
	void setLog(ZQ::common::Log* pLog);

	void process(bool success, const std::string& strError, const std::string& strCode);

protected:
	bool isErrorNeedRestart(const std::string& strError, const std::string& strCode);

	bool tryRestartApp();

protected:
	bool			_bPrepareRestart;

	ErrorProcFunc*		_errProcFunc;
//	ZQ::common::Log*	_log;
};


class ErrorProcessCmd : public ZQ::common::ThreadRequest
{
public:
	/// constructor
	ErrorProcessCmd(ZQ::common::NativeThreadPool& pool, ErrorProcess& errProc, bool success, const std::string& strError, const std::string& strCode)
		:ThreadRequest(pool), _errProc(errProc)
	{
		_success = success;
		_strError = strError;
		_strCode = strCode;
	}

protected: // impls of ScheduleTask

	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected:

	ErrorProcess&	_errProc;
	bool			_success;
	std::string		_strError, _strCode;
};



#endif
