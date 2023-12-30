/* File Name: Storage_web.cpp
   Date     : 26th Nov
   Purpose  : Defines the entry point of the DLL application for Storage Contents.
**/

#include "Log.h"
#include "httpdInterface.h"
#include "Controller.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

#ifdef _MANAGED
#pragma managed(pop)
#endif


extern "C"
{
    // lib init func
    __EXPORT bool LibInit(ZQ::common::Log* pLog)
    {
        if(NULL == pLog)
			return false;

		ZQ::common::setGlogger(pLog);
        return true;
    }
    // lib uninit func
    __EXPORT void LibUninit( )
    {
		ZQ::common::setGlogger();
    }

	__EXPORT bool ListVolumes(IHttpRequestCtx * pHttpRequestCtx)
	{
		if (NULL == pHttpRequestCtx)
		{
			return false;
		}
		StorageWeb::Controller controller(pHttpRequestCtx);
		return controller.listVolumes();
	}
	__EXPORT bool ListContents(IHttpRequestCtx * pHttpRequestCtx)
	{
		if (NULL == pHttpRequestCtx)
		{
			return false;
		}
		StorageWeb::Controller controller(pHttpRequestCtx);
		return controller.listContents();
	}
}



