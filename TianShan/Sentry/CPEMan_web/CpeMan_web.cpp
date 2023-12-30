// CpeMan_web.cpp : Defines the entry point for the DLL application.
//
#include "SessionController.h"
#include "MethodController.h"
#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

extern "C"
{
    __EXPORT bool LibInit(ZQ::common::Log* pLog)
    {
        if(NULL == pLog)
            return false;

		ZQ::common::setGlogger(pLog);
        return true;
    }

    __EXPORT void LibUninit( )
    {
		ZQ::common::setGlogger();
    }
   
    __EXPORT bool session(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;

        SessionController ctrler(pHttpRequestCtx);
        return ctrler.process();
    }

	__EXPORT bool method(IHttpRequestCtx *pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		MethodController mctl(pHttpRequestCtx);
		return mctl.process();
	}
  
}
