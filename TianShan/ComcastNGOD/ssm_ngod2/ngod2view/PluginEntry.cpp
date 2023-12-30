#include <Log.h>
#include <httpdInterface.h>
#include "NgodPage.h"

#ifdef ZQ_OS_MSWIN
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

extern "C"
{
	DLLEXPORT bool LibInit(ZQ::common::Log* pLog)
	{
		if(NULL == pLog)
			return false;

		ZQ::common::setGlogger(pLog);
		return true;
	}

	DLLEXPORT void LibUninit( )
	{
		ZQ::common::setGlogger(NULL);
	}

	DLLEXPORT bool ngod2main(IHttpRequestCtx* pHttpRequestCtx)
	{
		ngod2view::NgodPage page(pHttpRequestCtx);
		return page.process();
	}
}

