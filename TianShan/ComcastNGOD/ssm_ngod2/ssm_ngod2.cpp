
#include "stdafx.h"
#include "streamsmithmodule.h"
#include "NGODEnv.h"

HANDLE	g_hModule;
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	g_hModule=hModule;
    return TRUE;
}
NGODEnv	ssmNGOD;
bool bModuled = false;


RequestProcessResult	s1FixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return ssmNGOD.doFixupRequest(pSite,pReq);
}

RequestProcessResult	s1ContentHandle(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return ssmNGOD.doContentHandler(pSite,pReq);
}

RequestProcessResult	s1FixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	return ssmNGOD.doFixupResponse(pSite,pReq);
}

extern "C"
{
 __declspec(dllexport)	void ModuleInitEx(
	 IStreamSmithSite* pSite, 
	 const char* cfgPath)
 {
	if (!bModuled)
	{
		bModuled = true;
		ssmNGOD.setConfigPath(cfgPath);
		ssmNGOD.setLogDir(pSite->getApplicationLogFolder());
		if (1 == ssmNGOD.doInit(pSite))
		{
			throw "ssm_NGOD2.dll initialization failed";
		}
		pSite->RegisterContentHandle("NGOD2",(SSMH_ContentHandle) s1ContentHandle);
		pSite->RegisterFixupRequest((SSMH_FixupRequest) s1FixupRequest);
	}
 }
 __declspec(dllexport) void ModuleUninit(IStreamSmithSite* pSite)
 {
	 bModuled = false;
	 ssmNGOD.doUninit();
 }
}
