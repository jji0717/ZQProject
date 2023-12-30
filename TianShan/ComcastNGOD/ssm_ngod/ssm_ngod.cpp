// ssm_ngod.cpp : Defines the entry point for the DLL application.
//
#include <ZQ_common_conf.h>
#include "NgodService.h"
#include "ClientRequest.h"


#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif//ZQ_OS_MSWIN

NGOD::NgodService ngodService;


void throwerror(const char* msg )
{
	throw msg;
}

RequestProcessResult	s1ContentHandle(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return ngodService.processRequest( pReq);
}

static bool bInited = false;

extern "C"
{

__EXPORT void ModuleInitEx( IStreamSmithSite* pSite, const char* cfgPath )
{
	if(bInited )
		return;

	bInited = true;

	if( !ngodService.start( pSite , cfgPath , pSite->getApplicationLogFolder() ) )
	{
		throwerror( ngodService.getErrMsg().c_str() );
	}

	pSite->RegisterContentHandle("NGOD2",(SSMH_ContentHandle) s1ContentHandle);
	//pSite->RegisterContentHandle("NGOD",(SSMH_ContentHandle) s1ContentHandle);	
}

__EXPORT void ModuleUninit(IStreamSmithSite* pSite)
{
	bInited = false;
	ngodService.stop();
}

};

