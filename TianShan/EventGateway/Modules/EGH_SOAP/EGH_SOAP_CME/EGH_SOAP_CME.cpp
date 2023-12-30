// EGH_SOAP_CME.cpp : Defines the entry point for the DLL application.
//

#include "CMESOAPClientImpl.h"
#include "StreamEventSinkImpl.h"
#include "PlaylistEventSinkImpl.h"
#include <EventGwHelper.h>
#include <FileLog.h>

static EventGateway::IEventGateway* _gw;
static ZQ::common::FileLog *_log;
static EventGateway::CMESOAPHelper::ClientImpl *_client;
static void reset()
{
    _gw = NULL;
    _log = NULL;
    _client = NULL;
}
static void clear()
{
    if(_client)
    {
        delete _client;
        _client = NULL;
    }

    if(_log)
    {
		ZQ::common::setGlogger(NULL);
        try
        {
            delete _log;
        }
        catch(...)
        {
        }
        _log = NULL;
    }
}

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        reset();
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        clear();
		break;
	}
    return TRUE;
}
#endif


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EGH_SOAP_CME_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EGH_SOAP_CME_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef EGH_SOAP_CME_EXPORTS
#define EGH_SOAP_CME_API __EXPORT
#else
#define EGH_SOAP_CME_API __DLLRTL
#endif

#define THIS_MODULE_NAME "EGH_SOAP_CME"

ZQ::common::Config::Loader <EventGateway::CMESOAPHelper::CMESOAPConfig> gConfig(THIS_MODULE_NAME ".xml");

static Ice::ObjectPrx _streamEventSink;
static Ice::ObjectPrx _playlistEventSink;
extern "C"
{
EGH_SOAP_CME_API bool EventGw_Module_Entry_init(EventGateway::IEventGateway* gateway)
{
    reset();
    _gw = gateway;

	ZQ::common::setGlogger(&_gw->superLogger());
    // load config
    gConfig.setLogger(&_gw->superLogger());
    if(!gConfig.loadInFolder(_gw->getConfigFolder().c_str()))
    {
        return false;
    }
    // create module's log
    std::string modLogFilePath = _gw->getLogFolder() + THIS_MODULE_NAME ".log";
    try{
        _log = NULL;
        _log = new ZQ::common::FileLog(
            modLogFilePath.c_str(),
            gConfig.logLevel,
            gConfig.logCount,
            gConfig.logFileSize
            );
		ZQ::common::setGlogger(_log);
    }catch (...) {
        _gw->superLogger()(ZQ::common::Log::L_ERROR, "Caught unknown exception during create FileLog [%s]", modLogFilePath.c_str());
        return false;
    }

    // init the SOAP client
    _client = new EventGateway::CMESOAPHelper::ClientImpl(*_log, gConfig.client);
    {
        Ice::ObjectPtr obj = new EventGateway::CMESOAPHelper::StreamEventSinkI(*_log, *_client);
        _streamEventSink = gateway->getAdapter()->addWithUUID(obj);
    }
    {
        Ice::ObjectPtr obj = new EventGateway::CMESOAPHelper::PlaylistEventSinkI(*_log, *_client);
        _playlistEventSink = gateway->getAdapter()->addWithUUID(obj);
    }
     _gw->subscribe(_streamEventSink, TianShanIce::Streamer::TopicOfStream);
     _gw->subscribe(_playlistEventSink, TianShanIce::Streamer::TopicOfPlaylist);

    return true;
}
EGH_SOAP_CME_API void EventGw_Module_Entry_uninit()
{
    // unsubscribe
    try{
     _gw->unsubscribe(_streamEventSink, TianShanIce::Streamer::TopicOfStream);
     _gw->unsubscribe(_playlistEventSink, TianShanIce::Streamer::TopicOfPlaylist);
     _gw->getAdapter()->remove(_streamEventSink->ice_getIdentity());
     _gw->getAdapter()->remove(_playlistEventSink->ice_getIdentity());
    } catch (...) {
        _gw->superLogger()(ZQ::common::Log::L_ERROR, "Caught unknown exception when unsubscribe");
    }
    _gw = NULL;
    clear();
}
} // extern "C"
