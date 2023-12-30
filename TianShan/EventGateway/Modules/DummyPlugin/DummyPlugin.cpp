// DummyPlugin.cpp : Defines the entry point for the DLL application.
//
#include <ZQ_common_conf.h>
#include <EventGwHelper.h>
#include <TsEvents.h>
#include <FileLog.h>

#define EGH_DummyPlugin_Log_Name "EGH_Dummy.log"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

class Printer: public TianShanIce::Events::GenericEventSink
{
    virtual void ping(::Ice::Long timestamp, const ::Ice::Current& /* = ::Ice::Current */)
    {
        using namespace ZQ::common;
        glog(Log::L_DEBUG, CLOGFMT(Printer, "ping(): timestamp=%lld"), timestamp);
    }
    virtual void post(
        const ::std::string& category,
        ::Ice::Int eventId,
        const ::std::string& eventName,
        const ::std::string& stampUTC,
        const ::std::string& souceNetId,
        const ::TianShanIce::Properties& params,
        const ::Ice::Current& /* = ::Ice::Current */
        )
    {
        using namespace ZQ::common;
        glog(Log::L_DEBUG, CLOGFMT(Printer, "post(): category[%s], eventId[%d], eventName[%s], stampUTC[%s], sourceNetId[%s]")
            , category.c_str(), eventId, eventName.c_str(), stampUTC.c_str(), souceNetId.c_str());
    }
};

class DummyGenHelper : public EventGateway::IGenericEventHelper
{
    void onEvent(
        const ::std::string& category,
        ::Ice::Int eventId,
        const ::std::string& eventName,
        const ::std::string& stampUTC,
        const ::std::string& souceNetId,
        const EventGateway::Properties& params
        )
    {
        using namespace ZQ::common;
        glog(Log::L_DEBUG, CLOGFMT(DummyGenHelper, "onEvent(): category[%s], eventId[%d], eventName[%s], stampUTC[%s], sourceNetId[%s]")
            , category.c_str(), eventId, eventName.c_str(), stampUTC.c_str(), souceNetId.c_str());
    }
};

#ifdef DUMMYPLUGIN_EXPORTS
#define DUMMYPLUGIN_API __EXPORT
#else
#define DUMMYPLUGIN_API __DLLRTL
#endif

DummyGenHelper _helper;
EventGateway::IEventGateway* _gw;
Ice::ObjectPrx _printer;
ZQ::common::FileLog *_log;
extern "C"
{
DUMMYPLUGIN_API bool EventGw_Module_Entry_init(EventGateway::IEventGateway* gateway)
{
    _gw = gateway;
    // create module's log
    try{
        _log = NULL;
        _log = new ZQ::common::FileLog(
        (_gw->getLogFolder() + EGH_DummyPlugin_Log_Name).c_str(),
        ZQ::common::Log::L_DEBUG
        );

		ZQ::common::setGlogger(_log);
    }catch (...) {
        return false;
    }
    gateway->subscribe(&_helper, "TianShan/Event/Generic");
    Ice::ObjectPtr obj = new Printer();
    _printer = gateway->getAdapter()->addWithUUID(obj);
    gateway->subscribe(_printer, "TianShan/Event/Generic");
    return true;
}
DUMMYPLUGIN_API void EventGw_Module_Entry_uninit()
{
    _gw->unsubscribe(&_helper, "TianShan/Event/Generic");
    _gw->unsubscribe(_printer, "TianShan/Event/Generic");
    _gw->getAdapter()->remove(_printer->ice_getIdentity());
    if(_log)
    {
        try
        {
            delete _log;
        }
        catch(...)
        {	
        }
    }
}
}
