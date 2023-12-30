// This is the main DLL file.
#include "ZQ_common_conf.h"
#include "../httpdInterface.h"
#include <Log.h>
#include <Ice/Ice.h>
#include "ErmMain.h"
#include "EditQAM.h"
#include "ShowChannel.h"
#include "EditChannel.h"
#include "AddChannel.h"
#include "RemoveChannel.h"
#include "ShowDevice.h"
#include "EditDevice.h"
#include "AddDevice.h"
#include "RemoveDevice.h"
#include "ShowAllocation.h"
#include "ShowEdgePort.h"
#include "ChannelDetail.h"
//#include "EditServiceGroup.h"
//#include "ShowServiceGroup.h"
#include "EditRouteNames.h"
#include "ShowRouteNames.h"

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

	__EXPORT bool ErmMain(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::ErmMain page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ShowDevice(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::ShowDevice page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ShowChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::ShowChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ShowAllocation(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::ShowAllocation page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ShowEdgePort(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::ShowEdgePort page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ChannelDetail(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::ChannelDetail page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool ShowRouteNames(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::ShowRouteNames page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool EditRouteNames(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::EditRouteNames page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool AddDevice(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::AddDevice page(pHttpRequestCtx);
		return page.process();
	}

/*
	__EXPORT bool EditChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::EditChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool AddChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::AddChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool RemoveChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::RemoveChannel page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool EditDevice(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::EditDevice page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool RemoveDevice(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::RemoveDevice page(pHttpRequestCtx);
		return page.process();
	}

	__EXPORT bool EditQAM(IHttpRequestCtx* pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		ErmWebPage::EditQAM page(pHttpRequestCtx);
		return page.process();
	}
*/
}

