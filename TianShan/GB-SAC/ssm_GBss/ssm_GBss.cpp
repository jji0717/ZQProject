// ssm_GBss.cpp : Defines the entry point for the DLL application.
//

#include "StreamSmithModule.h"
#include "Environment.h"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
#endif//ZQ_OS_MSWIN

GBss::Environment ssmGBssEnv;
bool bModuled = false;

RequestProcessResult s1FixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return ssmGBssEnv.doFixupRequest(pSite, pReq);
}

RequestProcessResult s1ContentHandle(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return ssmGBssEnv.doContentHandler(pSite, pReq);
}

RequestProcessResult s1FixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	return ssmGBssEnv.doFixupResponse(pSite, pReq);
}

extern "C"
{
	__EXPORT void ModuleInitEx(IStreamSmithSite* pSite, const char* cfgPath)
	{
		if (!bModuled)
		{
			// 1 - failed to initial enviroment
			if (1 == ssmGBssEnv.doInit(cfgPath, pSite->getApplicationLogFolder(), pSite))
			{
				ssmGBssEnv.doUninit();
				return;
			}
			bModuled = true;
			pSite->RegisterContentHandle("GBss", (SSMH_ContentHandle)s1ContentHandle);
			pSite->RegisterFixupRequest((SSMH_FixupRequest) s1FixupRequest);
			pSite->RegisterFixupResponse((SSMH_FixupResponse) s1FixupResponse);
		}
	}
	__EXPORT void ModuleUninit(IStreamSmithSite* pSite)
	{
		if (bModuled)
		{
			bModuled = false;
			ssmGBssEnv.doUninit();
		}
	}
} // end extern "C"
