// PT_FtpServer.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "PushSessI.h"
#include "PT_FtpServer.h"
#include "FtpSite.h"
#include "NativeThread.h"
#include "FtpsXferExtension.h"
#include "Ftp_Svr.h"



BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

Ftp_Svr*	 _svr = NULL;

/// read the configuration xml, and start ftp server
PT_FTPSERVER_API bool PT_ModuleInit(const char* cfgPath, PushSessIMgr* pMgr)
{
	_svr = new Ftp_Svr();
	if (_svr == NULL)
		return false;
	
	if (!_svr->PT_Init(cfgPath,pMgr))
		return false;

	if (!_svr->PT_Start())
		return false;

//	FtpXferClass *xfer = new FtpXferClass(*this, _site, _pasv_sock, _port_sock, _Pool);

 	return true;
}


/// do the uninitialize
PT_FTPSERVER_API void PT_ModuleUninit()
{
	if (!_svr)
		return;

	_svr->PT_Stop();
	_svr->PT_Uninit();
}
 