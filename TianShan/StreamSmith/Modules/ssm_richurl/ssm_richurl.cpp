// ssm_richurl.cpp : Defines the entry point for the DLL application.
//

#include "./Environment.h"

#ifdef ZQ_OS_MSWIN
#define _WINSOCK2API_
#include "Windows.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif
bool bModuled = false;
ZQTianShan::Plugin::SsmRichURL::Environment env;

RequestProcessResult s1FixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return env.doFixupRequest(pSite,pReq);
}

extern "C"
{
	__EXPORT	void ModuleInitEx(IStreamSmithSite* pSite, const char* cfgPath)
	{
		if (false == bModuled)
		{
			bModuled = true;
			if (true == env.doInit(pSite, cfgPath))
			{
				pSite->RegisterFixupRequest((SSMH_FixupRequest) s1FixupRequest);
			}
		}
	}
	__EXPORT void ModuleUninit(IStreamSmithSite* pSite)
	{
		if (true == bModuled)
		{
			bModuled = false;
			env.doUninit(pSite);
		}
	}
}

