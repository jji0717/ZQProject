// FileName : APMDllMain.cpp
// Author   : Zheng Junming
// Date     : 2009-06
// Desc     : entry point of asset propagate web page

#include <log.h>
#include <httpdInterface.h>
#include "APMController.h"

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

extern "C"
{
	__declspec(dllexport) bool LibInit(ZQ::common::Log* pLog)
	{
		if(NULL == pLog)
		{
			return false;
		}

		ZQ::common::setGlogger(pLog);
		return true;
	}
	__declspec(dllexport) void LibUninit( )
	{
		ZQ::common::setGlogger(NULL);
	}
	__declspec(dllexport) bool ListAssets(IHttpRequestCtx * pHttpRequestCtx)
	{
		if (NULL == pHttpRequestCtx)
		{
			glog (ZQ::common::Log::L_ERROR, "ListAssets() : IHttpRequestCtx pointer is empty");
			return false;
		}
		APMWeb::Controller controller(pHttpRequestCtx);
		return controller.listAssets();
	}
	__declspec(dllexport) bool ListAssetReplica(IHttpRequestCtx * pHttpRequestCtx)
	{
		if (NULL == pHttpRequestCtx)
		{
			glog (ZQ::common::Log::L_ERROR, "ListAssetReplica() : IHttpRequestCtx pointer is empty");
			return false;
		}
		APMWeb::Controller controller(pHttpRequestCtx);
		return controller.listAssetReplica();
	}
	__declspec(dllexport) bool DeleteReplica(IHttpRequestCtx * pHttpRequestCtx)
	{
		if (NULL == pHttpRequestCtx)
		{
			glog (ZQ::common::Log::L_ERROR, "DeleteReplica() : IHttpRequestCtx pointer is empty");
			return false;
		}
		APMWeb::Controller controller(pHttpRequestCtx);
		return controller.deleteReplica();
	}

}
