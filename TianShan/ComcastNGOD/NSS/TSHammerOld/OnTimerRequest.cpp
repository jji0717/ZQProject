#include "OnTimerRequest.h"

OnTimerRequest::OnTimerRequest(::ZQ::common::NativeThreadPool &pool, ::ZQ::common::FileLog &fileLog,SessionMap &sessionMap, const ::std::string sessId)
:ThreadRequest(pool)
,_log(&fileLog)
,_sessionMap(sessionMap)
,_sessId(sessId)
,_uCSeq(0)
{
}

OnTimerRequest::OnTimerRequest(::ZQ::common::NativeThreadPool &pool, ::ZQ::common::FileLog &fileLog, SessionMap &sessionMap,uint16 uCSeq)
:ThreadRequest(pool)
,_log(&fileLog)
,_sessionMap(sessionMap)
,_uCSeq(uCSeq)
{
	_sessId.clear();
}

OnTimerRequest::~OnTimerRequest()
{

}
bool OnTimerRequest::init()
{
	_usedMilli = GetTickCount();
	XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "start() GUID: [%s](CSeq[%d])"), _sessId.c_str(), _uCSeq);
	return true;
}

int OnTimerRequest::run(void)
{
	XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "Session[%s] of CSeq[%d] run()"), _sessId.c_str(), _uCSeq);

	SessionHandler *pSessionHandler = _sessionMap.getSessionHandler(_sessId);
	if (pSessionHandler != NULL)
	{
		//TODO: call OnTimer() function
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "Session[%s] onTimer"), _sessId.c_str());
		pSessionHandler->OnTimer();
	}
	else
	{
		XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "Session[%s] not found"), _sessId.c_str());
		pSessionHandler = _sessionMap.getSessionHandler(_uCSeq);

		if (pSessionHandler != NULL)
		{
			//TODO: call OnTimer() function
			XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "Session CSeq[%d] onTimer"), _uCSeq);
			pSessionHandler->OnTimer();
		}
		else
			XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "Session CSeq[%d] not found"), _uCSeq);
	}

	return 1;
}

void OnTimerRequest::final(int retcode, bool bCancelled)
{
	_usedMilli = GetTickCount() - _usedMilli;
	XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnTimerRequest, "Session timeout leave, GUID: [%s] or CSeq[%d], used time: [%lld]"), _sessId.c_str(), _uCSeq, _usedMilli);
	delete this;
}