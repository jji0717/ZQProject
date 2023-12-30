#include "GenEventSinkI.h"
namespace EventGateway{

using namespace ZQ::common;
GenEventSinkI::GenEventSinkI(ZQ::common::Log& log)
:_log(log), _thrHelpersOnEvent(log)
{
   _thrHelpersOnEvent.start();
}
GenEventSinkI::~GenEventSinkI()
{
}
void GenEventSinkI::ping(::Ice::Long timestamp, const ::Ice::Current&)
{
    _log(Log::L_DEBUG, CLOGFMT(GenEventSinkI, "ping(): timestamp=%lld"), timestamp);
}

void GenEventSinkI::post(
    const ::std::string& category,
    ::Ice::Int eventId,
    const ::std::string& eventName,
    const ::std::string& stampUTC,
    const ::std::string& sourceNetId,
    const Properties& params,
    const ::Ice::Current&
    )
{
    ZQ::common::MutexGuard sync(_lockHelpers);
	GenHelpersOnEvent::HelpersCtx ctx(category, eventId, eventName, stampUTC, sourceNetId, params, Ice::Current());
	_thrHelpersOnEvent.execOnEvent(ctx, _helpers);

    _log(Log::L_DEBUG, CLOGFMT(GenEventSinkI, "post(): category[%s], eventId[%d], eventName[%s], stampUTC[%s], sourceNetId[%s]")
        , category.c_str(), eventId, eventName.c_str(), stampUTC.c_str(), sourceNetId.c_str());
}

void GenEventSinkI::add(IGenericEventHelper *pHelper)
{
    if(NULL == pHelper)
        return;

    ZQ::common::MutexGuard sync(_lockHelpers);
    if(_helpers.insert(pHelper).second)
    {
        _log(Log::L_INFO, CLOGFMT(GenEventSinkI, "added helper [%p]"), pHelper);
    }
    else
    {
        _log(Log::L_WARNING, CLOGFMT(GenEventSinkI, "requested to add duplicate helper [%p]"), pHelper);
    }
}

void GenEventSinkI::remove(IGenericEventHelper *pHelper)
{
    if(NULL == pHelper)
        return;
    ZQ::common::MutexGuard sync(_lockHelpers);
    // search in the helper collection
    if(1 == _helpers.erase(pHelper))
    {
        _log(Log::L_INFO, CLOGFMT(GenEventSinkI, "removed helper [%p]"), pHelper);
    }
    else
    {
        _log(Log::L_WARNING, CLOGFMT(GenEventSinkI, "requested to remove unkown helper [%p]"), pHelper);
    }
}

size_t GenEventSinkI::count()
{
    ZQ::common::MutexGuard sync(_lockHelpers);
    return _helpers.size();
}

GenHelpersOnEvent::GenHelpersOnEvent(ZQ::common::Log& log)
:_log(log), _bRunning(true), _thrdId(0)
{
}

GenHelpersOnEvent::~GenHelpersOnEvent()
{
     _bRunning = false;
	 _signalExec.signal();
}

void GenHelpersOnEvent::stop()
{
	_bRunning = false;
	_signalExec.signal();
}

int GenHelpersOnEvent::execOnEvent(HelpersCtx& ctx, std::set<IGenericEventHelper*> & helpers)
{
	int listSize = 0;

	{
		ZQ::common::MutexGuard syncList(_lockOnEventList);
		_execOnEventList.push_back( ListElement(ctx, helpers) );
		listSize = _execOnEventList.size();
	}

	_signalExec.signal();
	return listSize;
}

int GenHelpersOnEvent::run()
{
	_thrdId = id();
	_log(ZQ::common::Log::L_INFO, CLOGFMT(GenHelpersOnEvent, "(%d) GenHelpersOnEvent thread enter."), _thrdId);

	while (_bRunning)
	{
		if(_execOnEventList.empty())
			_signalExec.wait();

		if (!_bRunning)
			break;

		ListElement execElement;
		
		{
			ZQ::common::MutexGuard syncList(_lockOnEventList);
			if(!_execOnEventList.empty())
			{
				execElement = _execOnEventList.front();
				_execOnEventList.pop_front();
			}
		}
		
		HelpersCtx& ctx = execElement._ctx;
		std::set<IGenericEventHelper*>& helpers = execElement._helpers;
		for(std::set<IGenericEventHelper*>::const_iterator it = helpers.begin(); it != helpers.end(); ++it)
		{
			try
			{
				(*it)->onEvent(ctx._category, ctx._eventId, ctx._eventName, ctx._stampUTC, ctx._sourceNetId, ctx._params);
			}
			catch(...)
			{
				_log(Log::L_WARNING, CLOGFMT(GenHelpersOnEvent, "onEvent(): catch unknown exception, category[%s], eventId[%d], eventName[%s], stampUTC[%s], sourceNetId[%s] helper[%p]")
					, ctx._category.c_str(), ctx._eventId, ctx._eventName.c_str(), ctx._stampUTC.c_str(), ctx._sourceNetId.c_str(), *it);
			}
		}

		_log(Log::L_DEBUG, CLOGFMT(GenHelpersOnEvent, "onEvent(): dispatch category[%s], eventId[%d], eventName[%s], stampUTC[%s], sourceNetId[%s] ")
			, ctx._category.c_str(), ctx._eventId, ctx._eventName.c_str(), ctx._stampUTC.c_str(), ctx._sourceNetId.c_str());
	}
	
	_log(ZQ::common::Log::L_INFO, CLOGFMT(GenHelpersOnEvent, "(%d) GenHelpersOnEvent thread leave."), _thrdId);
	return true;
}

} // namespace EventGateway

