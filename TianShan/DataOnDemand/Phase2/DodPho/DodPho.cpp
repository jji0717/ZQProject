// DodPho.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "DodStoragePho.h"
#include "DodStreamPho.h"
#include <Log.h>
#include <filelog.h>
ZQLIB::FileLog* phoLog;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {

		char fileName[MAX_PATH];
		char* c;
		GetModuleFileName((HMODULE )hModule, fileName, sizeof(fileName) - 1);
		c = fileName + strlen(fileName) - 1;

		while(c >= fileName) {

			if (*c == '\\') {
				*c = '\0';
				break;
			}

			c --;
		}
		std::string LogfilePath = fileName;

		int npos = LogfilePath.find_last_of('\\');
		if(npos > 0)
		{
			LogfilePath = LogfilePath.substr(0, npos +1)+ "Logs\\pho_DOD.log";
			memset(fileName, 0, MAX_PATH);
			strcpy(fileName, LogfilePath.c_str());
		}
		else
		{
		    strcat(fileName, "\\pho_DOD.log");
		}

        phoLog = new ZQLIB::FileLog(fileName,ZQLIB::Log::L_DEBUG, 1024 * 1024 * 100);

//		if (svclog.init(fileName, ZQLIB::Log::L_DEBUG, 1024 * 1024 * 100))
//			ZQLIB::pGlog = &svclog;
	}

	_TRACE("DllMain(): reason = %d\n", ul_reason_for_call);

    return TRUE;
}


DodStoragePho* dodStoragePho = NULL;
DodStreamPho* dodStreamPho = NULL;
ZQTsAcPath::IPHOManager* phoMgr = NULL;

void InitPHO(ZQTsAcPath::IPHOManager& mgr, void* pCtx, 
			 const char* configFile, const char* logFolder)
{
	ZQ::common::setGlogger(phoLog);
	_TRACE("InitPHO()\n");

	if (phoMgr)
		return;

	phoMgr = &mgr;

	dodStoragePho = new DodStoragePho(mgr);
	dodStreamPho = new DodStreamPho(mgr);

	mgr.registerStorageLinkHelper(DOD_STORAGEPHO_NAME, *dodStoragePho, pCtx);
	mgr.registerStreamLinkHelper(DOD_STREAMPHO_NAME, *dodStreamPho, pCtx);
}

void UninitPHO(void)
{
	_TRACE("UninitPHO()\n");

	if (phoMgr) {
		phoMgr->unregisterStorageLinkHelper(DOD_STORAGEPHO_NAME);
		phoMgr->unregisterStreamLinkHelper(DOD_STREAMPHO_NAME);
		delete dodStoragePho;
		delete dodStreamPho;
	}
	ZQ::common::setGlogger(NULL);
	if(phoLog)
		delete phoLog;
}

inline void _cdecl logTrace(LPCSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	char szBuffer[512];

	nBuf = vsprintf(szBuffer, lpszFormat, args);
	if (phoLog) {
		(*phoLog)(ZQLIB::Log::L_DEBUG, szBuffer);
	} else {
		OutputDebugStringA(szBuffer);
	}

	va_end(args);
}
