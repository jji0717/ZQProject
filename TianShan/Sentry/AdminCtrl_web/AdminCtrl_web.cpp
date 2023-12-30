// AdminCtrl_web.cpp : Defines the entry point for the DLL application.
//

//#include "stdafx.h"
#include <Log.h>
#include "../httpdInterface.h"
#include "SiteController.h"
#include "PathController.h"

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

    // site admin pages
     __EXPORT bool SitePage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;

        SiteController ctrler(pHttpRequestCtx);
        return ctrler.SitePage();
    }
    __EXPORT bool AppPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;

        SiteController ctrler(pHttpRequestCtx);
        return ctrler.AppPage();
    }
    __EXPORT bool MountPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;

        SiteController ctrler(pHttpRequestCtx);
        return ctrler.MountPage();
    }
     __EXPORT bool TxnPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        SiteController ctrler(pHttpRequestCtx);
        return ctrler.TxnPage();
    }
    // path admin pages
    __EXPORT bool ServiceGroupPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        PathController ctrler(pHttpRequestCtx);
        return ctrler.ServiceGroupPage();
    }
    __EXPORT bool StoragePage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        PathController ctrler(pHttpRequestCtx);
        return ctrler.StoragePage();
    }
    __EXPORT bool StreamerPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        PathController ctrler(pHttpRequestCtx);
        return ctrler.StreamerPage();
    }
     __EXPORT bool StorageLinkPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        PathController ctrler(pHttpRequestCtx);
        return ctrler.StorageLinkPage();
    }
    __EXPORT bool StreamLinkPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        PathController ctrler(pHttpRequestCtx);
        return ctrler.StreamLinkPage();
    }
	__EXPORT bool StreamLinkBySGIdPage(IHttpRequestCtx *pHttpRequestCtx)
	{
		if(NULL == pHttpRequestCtx)
			return false;

		PathController ctrler(pHttpRequestCtx);
		return ctrler.StreamLinkBySGIdPage();
	}
    __EXPORT bool TransportMapPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        PathController ctrler(pHttpRequestCtx);
        return ctrler.TransportMapPage();
    }
    
    __EXPORT bool TransportConfPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
            return false;
        
        PathController ctrler(pHttpRequestCtx);
        return ctrler.TransportConfPage();
    }
}
