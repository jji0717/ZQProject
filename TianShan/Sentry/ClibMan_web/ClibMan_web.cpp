// ClibMan_web.cpp : Defines the entry point for the DLL application.
//

#include "../httpdInterface.h"
#include <Log.h>
#include <Ice/Ice.h>
#include "ClibMain.h"
#include "ShowContent.h"
#include "ShowVolume.h"
#include "ContentDetail.h"

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
	__EXPORT bool LibInit(ZQ::common::Log* pLog)
	{
		ZQ::common::setGlogger(pLog);
		return true;
	}

	__EXPORT void LibUninit( )
	{
		ZQ::common::setGlogger();
	}

	__EXPORT bool ClibMain(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ClibWebPage::ClibMain page(pHttpRequestCtx);
		return page.process();
	}
	__EXPORT bool ShowContent(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ClibWebPage::ShowContent page(pHttpRequestCtx);
		return page.process();
	}
	__EXPORT bool ShowVolume(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ClibWebPage::ShowVolume page(pHttpRequestCtx);
		return page.process();
	}
	__EXPORT bool ContentDetail(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ClibWebPage::ContentDetail page(pHttpRequestCtx);
		return page.process();
	}
}

