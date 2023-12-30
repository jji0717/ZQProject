// File Name : ssm_openVBO.cpp

#include "StreamSmithModule.h"
#include "Environment.h"
#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
#endif
EventISVODI5::Environment ssmEventISVODI5Env;
bool bModuled = false;

RequestProcessResult s1FixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return ssmEventISVODI5Env.doFixupRequest(pSite, pReq);
}

RequestProcessResult s1ContentHandle(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return ssmEventISVODI5Env.doContentHandler(pSite, pReq);
}

RequestProcessResult s1FixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	return ssmEventISVODI5Env.doFixupResponse(pSite, pReq);
}

extern "C"
{
	__EXPORT void ModuleInitEx(IStreamSmithSite* pSite, const char* cfgPath)
	{
		if (!bModuled)
		{
			// 1 - failed to initial enviroment
			if (1 == ssmEventISVODI5Env.doInit(cfgPath, pSite->getApplicationLogFolder(), pSite))
			{
				ssmEventISVODI5Env.doUninit();
				return;
			}
			bModuled = true;
			pSite->RegisterFixupRequest((SSMH_FixupRequest) s1FixupRequest);
			pSite->RegisterContentHandle("OpenVBO", s1ContentHandle);
			pSite->RegisterFixupResponse((SSMH_FixupResponse) s1FixupResponse);
		}
	}
	__EXPORT void ModuleUninit(IStreamSmithSite* pSite)
	{
		if (bModuled)
		{
			bModuled = false;
			ssmEventISVODI5Env.doUninit();
		}
	}
} // end extern "C"
