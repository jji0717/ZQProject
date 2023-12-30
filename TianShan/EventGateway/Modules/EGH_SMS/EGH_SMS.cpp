// EGH_SMS.cpp : Defines the entry point for the DLL application.
//
#include "EghSmsConfig.h"
#include "EventHandle.h"
#include <FileLog.h>

/*
#ifdef EGH_SMS_EXPORTS
#define EGH_SMS_API __declspec(dllexport)
#else
#define EGH_SMS_API __declspec(dllimport)
#endif
*/

#define GENERIC_EVENT_TOPIC "TianShan/Event/Generic"

static EventHandle* gEventHandle = NULL;
static EventGateway::IEventGateway* gGW = NULL;
static ZQ::common::FileLog* gLog = NULL;

#define SMS_MODULE_NAME "EGH_SMS"
ZQ::common::Config::Loader <EGH_SMS::eghSms> gConfig(SMS_MODULE_NAME ".xml");

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

extern "C"
{
__EXPORT bool EventGw_Module_Entry_init(EventGateway::IEventGateway* gateway)
{
	gGW = gateway;

	//load config item
	gConfig.setLogger(&gGW->superLogger());
    if(!gConfig.loadInFolder(gGW->getConfigFolder().c_str()))
    {
		gGW->superLogger()(ZQ::common::Log::L_ERROR,"EGH_SMS module config loadInFolder '%s' failed",gGW->getConfigFolder().c_str());
        return false;
    }

	//init log
	std::string strLogPath = gGW->getLogFolder() + SMS_MODULE_NAME ".log";
    try
	{
        if(gLog == NULL)
			gLog = new ZQ::common::FileLog(strLogPath.c_str(), gConfig.logLevel, ZQLOG_DEFAULT_FILENUM, gConfig.logSize);
    }
	catch (...) 
	{
        gGW->superLogger()(ZQ::common::Log::L_ERROR, "Caught unknown exception during create FileLog [%s]", strLogPath.c_str());
        return false;
    }

	if(gEventHandle == NULL)
	{
		gEventHandle = new EventHandle();
		if(gEventHandle == NULL)
		{
			gGW->superLogger()(ZQ::common::Log::L_ERROR, "Can not new a EventHandle class");
			return false;
		}
	}

	gEventHandle->SetLog(gLog);
	if(!gEventHandle->Init())
	{
		gEventHandle->UnInit();
		delete gEventHandle;
		gEventHandle = NULL;
		gGW->superLogger()(ZQ::common::Log::L_ERROR, "init EventHandle failed");
		return false;
	}
  
	gateway->subscribe(gEventHandle, GENERIC_EVENT_TOPIC);

    return true;
}

__EXPORT void EventGw_Module_Entry_uninit()
{
	gGW->unsubscribe(gEventHandle, GENERIC_EVENT_TOPIC);
	if(gEventHandle)
	{
		gEventHandle->UnInit();
		delete gEventHandle;
		gEventHandle = NULL;
	}

	if(gLog)
    {
        try
        {
            delete gLog;
			gLog = NULL;
        }
        catch(...)
        {	
        }
    }
}
} // extern "C"
