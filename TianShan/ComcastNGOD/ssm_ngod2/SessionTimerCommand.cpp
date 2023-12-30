#include "SessionTimerCommand.h"
#include "NGODEnv.h"
#include "ContextImpl.h"

SessionTimerCommand::SessionTimerCommand(NGODEnv& env, const ::Ice::Identity& sessIdent)
: _env(env), ThreadRequest(*(env._pThreadPool)), _identSess(sessIdent)
{
}

bool SessionTimerCommand::init()
{
	_usedMilli = ZQTianShan::now();
	_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionTimerCommand, "start() GUID: [%s]"), _identSess.name.c_str());
	return true;
}

int SessionTimerCommand::run(void)
{
	_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionTimerCommand, "Session[%s] run()"), _identSess.name.c_str());

	try
	{		
		NGODr2c1::ContextPrx pContextPrx = NGODr2c1::ContextPrx::uncheckedCast(_env._pEventAdapter->createProxy(_identSess));
		pContextPrx->onTimer();
		return 0;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		return -1;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_env._fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionTimerCommand, "sess[%s] exception occurs: %s:%s"), _identSess.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		_env._fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionTimerCommand, "sess[%s] exception occurs: %s"), _identSess.name.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		_env._fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionTimerCommand, "caught unexpect exception when session ontimer()"));
	}

	// when reaches here, an exception might occur, when re-post a timer command to ensure no action is dropped
	try
	{
		_env._pSessionWatchDog->watchSession(_identSess, (long) (_ngodConfig._rtspSession._timeout) * 1000);
	}
	catch(...)
	{
	}

	return -2;
}

void SessionTimerCommand::final(int retcode, bool bCancelled)
{
	_usedMilli = ZQTianShan::now() - _usedMilli;
	_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionTimerCommand, "Session timeout leave, GUID: [%s], used time: [%lld]"), _identSess.name.c_str(), _usedMilli);
	delete this;
}

