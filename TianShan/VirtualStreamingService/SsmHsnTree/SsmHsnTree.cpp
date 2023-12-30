// This is the main DLL file.

#include "stdafx.h"

#include "SsmHsnTree.h"

#include "Environment.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	return TRUE;
}

bool bModuled = false;
HSNTree::Environment env;

RequestProcessResult hsnFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return env.doFixupRequest(pSite,pReq);
}

RequestProcessResult hsnContentHandle(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return env.doContentHandler(pSite,pReq);
}

RequestProcessResult hsnFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	return env.doFixupResponse(pSite,pReq);
}

extern "C"
{
	__declspec(dllexport)	void ModuleInitEx(IStreamSmithSite* pSite,const char* cfgPath)
	{
		if (false == bModuled)
		{
			bModuled = true;
			env.setLogDir(pSite->getApplicationLogFolder());
			if (true == env.doInit(cfgPath, pSite))
			{
				pSite->RegisterContentHandle("hsntree",(SSMH_ContentHandle) hsnContentHandle);
				pSite->RegisterFixupRequest((SSMH_FixupRequest) hsnFixupRequest);
				pSite->RegisterFixupResponse((SSMH_FixupResponse) hsnFixupResponse);
			}
		}
	}
	__declspec(dllexport) void ModuleUninit(IStreamSmithSite* pSite)
	{
		if (true == bModuled)
		{
			bModuled = false;
			env.doUninit();
		}
	}
}
