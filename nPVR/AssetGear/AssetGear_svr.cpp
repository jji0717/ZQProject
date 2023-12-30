

#include <iostream>
#include "isa.h"
#include "ItvMessages.h"
#include "appshell.h"
#include "manpkg.h"
#include "cfgpkg.h"
#include "AssetGear_soap.h"
#include "ItvMessages.h"
#include "MiniDump.h"
#include "ContentRoutine.h"

using namespace std;


// because in the itvmessages.h, it define ZQ to a value, cause the namespace ZQ error
#ifdef ZQ
#undef ZQ
#endif


ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);

ULONG __stdcall shellKeepAliveProc(LPVOID lpParam);


DWORD			_dwShellAliveTimeOut = 15000;		// in ms
bool serviceInit();
void serviceClose();

HANDLE			_hExit = NULL;

#define		DEF_VODAPP_NAME		L"SeaChange Movies On Demand"

WCHAR			_wszAssetGearUrl[256];
WCHAR			_wszApplicationName[256];
char			_szApplicationName[256];
DWORD			_dwSoapThreadPool;
WCHAR			_wszMetadataGatewayUrl[256];
char			_szMetadataGatewayUrl[256];

DWORD			_dwTimeWindowSecs = 600;		// for start time & end time check

DWORD			_dwFailCAWhenMdFail = 1;				// set to 1, update metadata fail, will cause the createAsset fail, 0, will not

DWORD			_dwMaxMetaDataCount = 50;


//////////////////////////////////////////////////////////////////////////
//
//  backup the recv.log and sent.log when it's size > 10M
//
void backup_soap_log()
{
	const char* szFiles[]= {
		"Recv.log",
		"Sent.log"
	};
	const char* szBakFiles[]= {
		"Recv.bak",
		"Sent.bak"
	};
	int nFileCount = sizeof(szFiles)/sizeof(const char*);

	for(int i=0;i<nFileCount;i++)
	{
		WIN32_FIND_DATAA wfd;
		HANDLE hFind = FindFirstFileA(szFiles[i], &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
			continue;

		FindClose(hFind);

		HANDLE hFile = CreateFileA(szFiles[i], GENERIC_READ,FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile==INVALID_HANDLE_VALUE)
			continue;

		DWORD dwSize = GetFileSize(hFile, 0);
		CloseHandle(hFile);

		if (dwSize < 10485760)		//< 10M
		{
			continue;			
		}

		// check if the bak file exist
		DeleteFileA(szBakFiles[i]);
		MoveFileA(szFiles[i], szBakFiles[i]);
	}
}


// app_main()
//
void app_main( int argc, char *argv[] )
{
	backup_soap_log();

    // Let everyone (service shell, app shell) know we're alive.
    //
    if (!SetEvent(handle_array[ APPALIVE ]))
    {
        return;
    }

    // Initialize basic iTV service requirements 
    //
    if (!serviceInit())
    {
		_log.Flush();
		serviceClose();
        return;
    }

	if (!contentInit())
	{
		_log.Flush();
		serviceClose();
        return;
	}

	//
    // start the service
    //
	char szAssetGearUrl[256];
	WideCharToMultiByte(CP_ACP, 0, _wszAssetGearUrl, -1, szAssetGearUrl, sizeof(szAssetGearUrl), 0, 0);
	char szBindIP[256];
	szBindIP[0] = '\0';
	int nPort = 80;
	{
		const char* p = strstr(szAssetGearUrl, "://");
		if (p!=NULL)
		{
			p += 3;
			const char* p1 = p;
			while(*p && *p!='/' && *p!=':')p++;
			strncpy(szBindIP, p1, p-p1);
			if (*p==':')
				nPort = atoi(p+1);
		}
	}

	LogMsg(L"Soap bind ip %S, listen port %d", szBindIP, nPort);

	AssetGearService agService(szBindIP, nPort, _dwSoapThreadPool);
	agService.start();

	bool bLoop = true;
    while(bLoop)
    {
        DWORD dwRet = WaitForSingleObject(_hExit, _dwShellAliveTimeOut);
		if (dwRet != WAIT_TIMEOUT)
		{
			bLoop = false;
			if (dwRet != WAIT_OBJECT_0)
			{
				LogMsg(REPORT_SEVERE, L"WaitForSingleObject return %08x in shellKeepAliveProc()", dwRet);
			}
		}
        
		SetEvent(handle_array[APPALIVE]); // tell appshell we're alive
	}
}

// app_service_control()
//
void app_service_control( int SCode )
{
    switch( SCode )
    {
        case SCMgrSTOP:
        {
            // Set the stop event to let the service shell know that
            // we are going to stop.
            SetEvent(handle_array[APPSHUTDOWN]);

			LogMsg(L"app_service_control request to stop service");
			serviceClose();

            break;
        }
        case SCMgrCONTINUE:
        {
            // Set the continued event to let the service shell know that
            // we are running.
            SetEvent(HAppContinued);

            // flush log buffer
            _log.Flush();

            break;
        }
        case SCMgrPAUSE:
            break;
        default:
        {
            // Unknown service request code.
            LogEvent( REPORT_SEVERE, _dwEventId, L"Unknown service request code %d", SCode );
            break;
        }
    } // end switch
}


bool serviceInit()
{
    _dwEventId = ISA_CONTENT_OPERATION_ERR;

    // get configuration parameters from registry
    //
    DWORD dwNumber;
    _hCfg = CFG_INITEx( _T("AssetGear"), &dwNumber, _T("ITV"));
    if (NULL == _hCfg)
    {
        wcout << L"Failed in CFG_INITEx()" << endl;
        return false;
    }

    // get management port number
    //
    if (CFG_SUCCESS != CFG_GET_MGMT_PORT(_hCfg, &_dwMgmtPort))
    {
        wcout << L"Failed in CFG_GET_MGMT_PORT()" << endl;
        return false;
    }
    
    // init management subsystem
    //
    DWORD dwError = 0;
    MANSTATUS manStatus = ManOpenSession( _T("AssetGear"), _dwMgmtPort, &_hManPkg, &dwError);
    if (MAN_SUCCESS != manStatus)
    {
        wcout << L"Failed in ManOpenSession(), status =  " << hex << manStatus << endl;
        return false;
    }

    // Get LogFile name
    //
    if (!getConfig( _wszLogFile, _T("LogFile"), sizeof(_wszLogFile), true, true, true, false, 
                    L"%ITVROOT%\\log\\AssetGear.log"))
    {
        return false;
    }

    // convert any environment variables
    if (wcschr(_wszLogFile, '%') != NULL)
    {
        TCHAR szLogName[MAX_PATH];
        if (::ExpandEnvironmentStrings(_wszLogFile, szLogName, sizeof(szLogName)))
        {
            wcscpy(_wszLogFile, szLogName);
        }
    }

    //                                                      MANAGE, READONLY, HAS DEFAUT VALUE
    if ( !getConfig(&_dwLogSize,    _T("LogFileSize"), true, true,  true,     false, 
                    DEFAULT_LOGFILESIZE, ISA_CONTENT_OPERATION_ERR)
      || !getConfig(&_dwLogLevel,    _T("LogLevel"),   true, false, true,     false,
                    TRACE_DEBUG, ISA_CONTENT_OPERATION_ERR)
	)
    {
        return false;
    }

    _log.SetSvcLogMaxFileSize(_dwLogSize);
    //_log.SetSvcLogWriteTimeout(_dwLogDelay);
    //_log.SetSvcLogBufferSize(_dwLogBuffer);
    _log.SetLogLevel(ALL_LOGS, _dwLogLevel);

    // init reporter
    //
    RPTSTATUS rptStatus = _log.Init(_wszLogFile);
    if (RPT_SUCCESS != rptStatus)
    {
        CFG_TERM(_hCfg);
        return false;
    }

    rptStatus = _log.Register(&_dwRegId, L"AssetGear", L"AssetGear");
    if (RPT_SUCCESS != rptStatus)
    {
        CFG_TERM(_hCfg);
        return false;
    }

    LogMsg( L"===================== Loading AssetGear interface ======================" );

    if(!getConfig(_wszORBEndpoint, L"ORBEndpoint", sizeof(_wszORBEndpoint),
                   true, true, true,  false, L"", _dwEventId)
     || !getConfig(_wszNameService, L"NameService", sizeof(_wszNameService),
                   true, true, true,  false,           DEFAULT_NAME_SERVICE, _dwEventId)
      || !getConfig(&_dwShellAliveTimeOut, L"ShellAliveTimeOut",
                    true, true, true,   false,      15000, ISA_CONTENT_OPERATION_ERR) // 15 seconds
      || !getConfig(_wszAssetGearUrl, _T("AssetGearUrl"), sizeof(_wszAssetGearUrl), true, true, true, false,
                    L"http://localhost:1200/services/AssetGearService", ISA_CONTENT_OPERATION_ERR)
       || !getConfig(_wszMetadataGatewayUrl, _T("MetadataGatewayUrl"), sizeof(_wszMetadataGatewayUrl), true, true, true, false,
                    L"", ISA_CONTENT_OPERATION_ERR)
      || !getConfig(&_dwSoapThreadPool, L"SoapThreadPool",
                    true, true, true,   false, DEFAULT_THREAD_POOL_SZ, ISA_CONTENT_OPERATION_ERR)
      || !getConfig(&_dwTimeWindowSecs, L"TimeWindowSecs",
                    true, true, false, false, 600, ISA_CONTENT_OPERATION_ERR)
      || !getConfig(&_dwFailCAWhenMdFail, L"UpdateMdFailCreateAssetFail",
                    true, true, false, false, 1, ISA_CONTENT_OPERATION_ERR)					
      || !getConfig(&_dwMaxMetaDataCount, L"MaxMetaDataNum",
                    true, true, false, false, 50, ISA_CONTENT_OPERATION_ERR)					
      || !getConfig(_wszApplicationName, _T("VODApplicationName"), sizeof(_wszApplicationName), true, true, true, false,
                    DEF_VODAPP_NAME, ISA_CONTENT_OPERATION_ERR)
	)
    {
        return false;
    }

	WideCharToMultiByte(CP_ACP, 0, _wszMetadataGatewayUrl, -1, _szMetadataGatewayUrl, sizeof(_szMetadataGatewayUrl), 0, 0);
	WideCharToMultiByte(CP_ACP, 0, _wszApplicationName, -1, _szApplicationName, sizeof(_szApplicationName), 0, 0);
	
	// add by jie 2005/09/09
	{
		// get the crash dump path
		static wchar_t wszDumpPath[256];
		getConfig(wszDumpPath, _T("CrashDumpPath"),  sizeof(wszDumpPath), true, true, true, false,  
			_T("c:\\isa\\crashdump"), _dwEventId);
		
		if (!validatePath(wszDumpPath))
		{
			return false;
		}

		_crashDump.enableFullMemoryDump(TRUE);
		_crashDump.setDumpPath(wszDumpPath);
		_crashDump.setExceptionCB(CrashExceptionCallBack);		
	}
	
	_hExit = CreateEvent(NULL, FALSE, FALSE, NULL);

	/*
	DWORD dwThreadID;
    HANDLE hHandle = CreateThread(NULL, 0, shellKeepAliveProc, NULL, 0, &dwThreadID);
	CloseHandle(hHandle);*/

    return true;
}

void serviceClose()
{
	if (_hExit)
	{
		LogMsg(L"AssetGear interface is shutting down ...");

		SetEvent(_hExit);
		Sleep(200);
		CloseHandle(_hExit);
		_hExit = NULL;

		contentClose();

		_log.Flush();
		_log.Uninit();
	}
}

/*
ULONG __stdcall shellKeepAliveProc(LPVOID lpParam)
{
	if (_dwShellAliveTimeOut == 0) // Disable this AppShell KeepAlive Feature
    {
        gbAppShellSetKeepAlive = TRUE;
        return 0;
    }
	
	LogMsg(L"shellKeepAliveProc thread enter, thread id [%04x]", GetCurrentThreadId());
	
	bool bLoop = true;
    while(bLoop)
    {
        DWORD dwRet = WaitForSingleObject(_hExit, _dwShellAliveTimeOut);
		if (dwRet != WAIT_TIMEOUT)
		{
			bLoop = false;
			if (dwRet != WAIT_OBJECT_0)
			{
				LogMsg(REPORT_SEVERE, L"WaitForSingleObject return %08x in shellKeepAliveProc()", dwRet);
			}
		}
        
		SetEvent(handle_array[APPALIVE]); // tell appshell we're alive
	}
	
	LogMsg(L"shellKeepAliveProc thread leave");
	return 0;
}
*/

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	
	LogEvent( REPORT_SEVERE, ISA_STREAM_OPERATION_ERR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);

	_log.Flush();
}

