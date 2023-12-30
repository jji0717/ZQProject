#include "crm_dsmcc.h"

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
DSMCC_Environment env ;

ProcessResult dsmcc_FixupRequest( RequestPtr request )
{
	return env.doFixupRequest(request);
}

ProcessResult dsmcc_ContentHandle( RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess )
{
	return env.doContentHandler(request,sess) ;
}

ProcessResult dsmcc_FixupResponse( RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess )
{
	return env.doFixupResponse(request,sess) ;
}

/*
ProcessResult dsmcc_doRestore(TianShanIce::ClientRequest::SessionPrx sess )
{
	return env.doRestore(sess) ;
}
*/

void dsmcc_doExpired(TianShanIce::ClientRequest::SessionPrx sess )
{
	return env.doExpired(sess);
}

extern "C"
{
	__EXPORT bool ModuleInit( Gateway& gw, ZQADAPTER_DECLTYPE objAdapter, const char* configpath, const char* loggerpath )
	{
		if (false == bModuled)
		{
			bModuled = true;
			if (true == env.doInit(gw,objAdapter, configpath,loggerpath))
			{
				gw.registerFixupRequestStage(dsmcc_FixupRequest);
				gw.registerContentHandlerStage(dsmcc_ContentHandle,"DSMCC");
				gw.registerFixupResponseStage(dsmcc_FixupResponse);				
				gw.registerOntimerProc(dsmcc_doExpired);
			}
			else
				return false;
		}
		return true;
	}
	__EXPORT bool ModuleUninit(Gateway& gw)
	{
		if (true == bModuled)
		{
			gw.unregisterFixupRequestStage(dsmcc_FixupRequest);
			gw.unregisterContentHandlerStage(dsmcc_ContentHandle,"DSMCC");
			gw.unregisterFixupResponseStage(dsmcc_FixupResponse);				
			bModuled = false;
			env.doUninit();
		}
		return true;
	}
}

