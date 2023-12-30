// This is the main DLL file.

#include "Stdafx.h"
#include <Log.h>
#include <Ice/Ice.h>
#include "../httpdInterface.h"
#include "BcastMain.h"
#include "AddChannel.h"
#include "ShowChannel.h"
#include "RemoveChannel.h"
#include "EditChannel.h"
#include "ShowItem.h"
#include "PushItem.h"
#include "EditItem.h"
#include "RemoveItem.h"
#include "InsertItem.h"
#include "AddFilterItem.h"
#include "RemoveFilterItem.h"

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
		if(NULL == pLog)
			return false;

		ZQ::common::setGlogger(pLog);
		return true;
	}

	__EXPORT void LibUninit( )
	{
		ZQ::common::setGlogger();
	}

	__EXPORT bool BcastMain(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::BcastMain page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ShowChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::ShowChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool AddChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::AddChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool RemoveChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::RemoveChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool EditChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::EditChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ShowItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::ShowItem page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool InsertItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::InsertItem page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool RemoveItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::RemoveItem page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool PushItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::PushItem page( pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool EditItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::EditItem page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool AddFilterItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::AddFilterItem page( pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool RemoveFilterItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		BcastWebPage::RemoveFilterItem page(pHttpRequestCtx);
		return page.process();
	}

}

