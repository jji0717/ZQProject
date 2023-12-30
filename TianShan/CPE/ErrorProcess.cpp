
#include "Log.h"
#include "ErrorProcess.h"
#include <assert.h>
#include "CPECfg.h"

#define MOLOG (glog)

using namespace ZQ::common;

ErrorProcess::ErrorProcess(ErrorProcFunc* pProcFunc)
{
	_errProcFunc = pProcFunc;
	_bPrepareRestart = false;
//	_log = &ZQ::common::NullLogger;
}

void ErrorProcess::process(bool success, const std::string& strError, const std::string& strCode)
{
	if (_bPrepareRestart || (!success && isErrorNeedRestart(strError, strCode)))
	{
		tryRestartApp();
	}
}

bool ErrorProcess::isErrorNeedRestart( const std::string& strError, const std::string& strCode )
{
	std::vector<CriticalProvisionError>::iterator iter;
	for (iter = _gCPECfg.criticalErrors.begin(); iter != _gCPECfg.criticalErrors.end(); ++iter) 
	{
		if (strstr(strError.c_str(), iter->keyword.c_str()))
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ErrorProc, "Critical error found: [%s, code: %s]"), strError.c_str(), strCode.c_str());
			return true;
		}
	}

	return false;
}

void ErrorProcess::setQuery( ErrorProcFunc* pProcFunc )
{
	_errProcFunc = pProcFunc;
}

bool ErrorProcess::tryRestartApp()
{
	assert(_errProcFunc != NULL);

	_errProcFunc->stopReceiveRequest();
	if (_errProcFunc->canRestartApplication())
	{	
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ErrorProc, "try to restart application by critical error"));
		_errProcFunc->restartApplication();
		return true;
	}
	
	_bPrepareRestart = true;
	return false;
}

void ErrorProcess::setLog( ZQ::common::Log* pLog)
{
	ZQ::common::setGlogger(pLog);
}

int ErrorProcessCmd::run(void)
{
	_errProc.process(_success, _strError, _strCode);
	return -1;
}

void ErrorProcessCmd::final(int retcode, bool bCancelled)
{
	delete this;
}

