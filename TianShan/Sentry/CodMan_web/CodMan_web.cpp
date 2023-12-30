// CodMan_web.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <log.h>
#include <Ice/Ice.h>
#include "../httpdInterface.h"
#include "./CodMain.h"
#include "./AddChannel.h"
#include "./EditChannel.h"
#include "./RemoveChannel.h"
#include "./PushItem.h"
#include "./InsertItem.h"
#include "./EditItem.h"
#include "./RemoveItem.h"
#include "./ShowChannel.h"
#include "./ShowItem.h"

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

extern "C"
{
    __declspec(dllexport) bool LibInit(ZQ::common::Log* pLog)
    {
        if(NULL == pLog)
            return false;
		
		ZQ::common::setGlogger(pLog);
        return true;
    }

    __declspec(dllexport) void LibUninit( )
    {
		ZQ::common::setGlogger();
	}

	// call this function to show channels page
	// this page can show all the channels could be demand now
	// you can add a new channel, remove an existing channel and edit a channel properties
	// by click the appropriate button shown on this page, you can also click some button to
	// enter into a channel to see which are contained in the channel.
	__declspec(dllexport) bool CodMain(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::CodMain page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool ShowChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::ShowChannel page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool AddChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::AddChannel page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool RemoveChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::RemoveChannel page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool EditChannel(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::EditChannel page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool ShowItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::ShowItem page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool InsertItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::InsertItem page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool PushItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::PushItem page( pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool RemoveItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::RemoveItem page(pHttpRequestCtx);
		return page.process();
	}

	__declspec(dllexport) bool EditItem(IHttpRequestCtx* pHttpRequestCtx)
	{
		CodWebPage::EditItem page(pHttpRequestCtx);
		return page.process();
	}
}

