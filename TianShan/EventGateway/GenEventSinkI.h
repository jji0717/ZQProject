#ifndef __TIANSHAN_EVENTGW_GENEVENTSINKI_H__
#define __TIANSHAN_EVENTGW_GENEVENTSINKI_H__
#include <ZQ_common_conf.h>
#include "EventGwHelper.h"
#include <Locks.h>
#include <TsEvents.h>
#include <set>
#include <list>
#include "NativeThread.h"
#include "SystemUtils.h"

namespace EventGateway
{

class GenHelpersOnEvent : public ZQ::common::NativeThread
{
public:
	typedef struct _HelpersCtx
	{				
		std::string   _category;
		Ice::Int      _eventId;
		std::string   _eventName;
		std::string   _stampUTC;
		std::string   _sourceNetId;
		TianShanIce::Properties  _params;
		Ice::Current  _current;

		_HelpersCtx(){}

		_HelpersCtx(
			const std::string& category,
			Ice::Int eventId,
			const std::string& eventName,
			const std::string& stampUTC,
			const std::string& sourceNetId,
			const TianShanIce::Properties& params,
			const Ice::Current& current)
			:_category(category), _eventId(eventId), _eventName(eventName), 
			_stampUTC(stampUTC), _sourceNetId(sourceNetId), _params(params), _current(current)
		{}
	} HelpersCtx;

	typedef struct _ListElement
	{
		HelpersCtx _ctx;
		std::set<IGenericEventHelper*> _helpers;

		_ListElement(){}

		_ListElement(HelpersCtx& ctx, std::set<IGenericEventHelper*>& helpers)
			:_ctx(ctx), _helpers(helpers)
		{} 
	} ListElement;

	GenHelpersOnEvent(ZQ::common::Log& log);
	~GenHelpersOnEvent();

	virtual int  run();
	void stop();

	int execOnEvent(HelpersCtx& ctx, std::set<IGenericEventHelper*> & helpers);

private:
	std::list< ListElement > _execOnEventList;// list transaction safe
	ZQ::common::Mutex  _lockOnEventList;
	SYS::SingleObject  _signalExec;

	uint32  _thrdId;
	int     _bRunning;
	ZQ::common::Log &_log;
};

// multi helper version
class GenEventSinkI : public TianShanIce::Events::GenericEventSink
{
public:
    explicit GenEventSinkI(ZQ::common::Log&);
    virtual ~GenEventSinkI();
    typedef ::IceInternal::Handle< GenEventSinkI > Ptr;
    virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);
    virtual void post(
        const ::std::string& category,
        ::Ice::Int eventId,
        const ::std::string& eventName,
        const ::std::string& stampUTC,
        const ::std::string& sourceNetId,
        const ::TianShanIce::Properties& params,
        const ::Ice::Current& /* = ::Ice::Current */
        );

    void add(IGenericEventHelper *pHelper);
    void remove(IGenericEventHelper *pHelper);
    size_t count();

private:
    ZQ::common::Log &_log;
    typedef std::set<IGenericEventHelper*> GenHelperCollection;
    GenHelperCollection _helpers;
    ZQ::common::Mutex   _lockHelpers;

	GenHelpersOnEvent  _thrHelpersOnEvent;
};

} // namespace EventGateway
#endif

