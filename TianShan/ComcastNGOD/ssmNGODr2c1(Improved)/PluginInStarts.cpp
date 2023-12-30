// ssm_NGOD_r2c1.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "streamsmithmodule.h"
#include "ssmNGODr2c1.h"

HANDLE	g_hModule;
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	g_hModule=hModule;
    return TRUE;
}
ssmNGODr2c1	ssmNGOD;
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
 __declspec(dllexport)	void ModuleInitEx(IStreamSmithSite* pSite,const char* cfgPath)
 {
	if (!bModuled)
	{
		pSite->RegisterContentHandle("NGOD_r2c1",(SSMH_ContentHandle) s1ContentHandle);
		pSite->RegisterFixupRequest((SSMH_FixupRequest) s1FixupRequest);
		bModuled = true;
		ssmNGOD.setConfigPath(cfgPath);
		ssmNGOD.doInit(pSite);
	}
 }
 __declspec(dllexport) void ModuleUninit(IStreamSmithSite* pSite)
 {
	 bModuled = false;
	 ssmNGOD.doUninit();
 }
}
