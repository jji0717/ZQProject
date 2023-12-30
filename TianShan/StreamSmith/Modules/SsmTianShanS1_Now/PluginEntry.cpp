#include "./Environment.h"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

bool bModuled = false;
TianShanS1::Environment env;

RequestProcessResult s1FixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return env.doFixupRequest(pSite,pReq);
}

RequestProcessResult s1ContentHandle(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return env.doContentHandler(pSite,pReq);
}

RequestProcessResult s1FixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	return env.doFixupResponse(pSite,pReq);
}

extern "C"
{
	__EXPORT void ModuleInitEx(IStreamSmithSite* pSite,const char* cfgPath)
	{
		if (false == bModuled)
		{
			bModuled = true;
			env.setLogDir(pSite->getApplicationLogFolder());
			if (true == env.doInit(cfgPath, pSite))
			{
				pSite->RegisterContentHandle("tianshan_s1",(SSMH_ContentHandle) s1ContentHandle);
				pSite->RegisterFixupRequest((SSMH_FixupRequest) s1FixupRequest);
				pSite->RegisterFixupResponse((SSMH_FixupResponse) s1FixupResponse);
			}
		}
	}
    __EXPORT void ModuleUninit(IStreamSmithSite* pSite)
	{
		if (true == bModuled)
		{
			bModuled = false;
			env.doUninit();
		}
	}
}

