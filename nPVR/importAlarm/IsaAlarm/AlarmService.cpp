// AlarmService.cpp: implementation of the AlarmService class.
//
//////////////////////////////////////////////////////////////////////

#include "AlarmService.h"
#include "InfoCollector.h"
#include "MiniDump.h"
#include "Log.h"

using namespace ZQ::common;


#define SERVICE_NAME	L"EventCollector"
#define PRODUCT_NAME	L"General"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);

bool validatePath    ( const WCHAR *     wszPath )
{
    if (-1 != ::GetFileAttributes(wszPath))
        return true;

    DWORD dwErr = ::GetLastError();
    if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
    {
        if (!::CreateDirectory(wszPath, NULL))
        {
            dwErr = ::GetLastError();
            if ( dwErr != ERROR_ALREADY_EXISTS)
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

AlarmService::AlarmService()
{
//	wcscpy(servname, SERVICE_NAME);
//	wcscpy(prodname, PRODUCT_NAME);

	_pInfoCollector = NULL;
}

AlarmService::~AlarmService()
{
	if (_pInfoCollector)
	{
		_pInfoCollector->close();
		delete _pInfoCollector;
	}
}

HRESULT AlarmService::OnInit()
{
	if (_pInfoCollector)
	{
		_pInfoCollector->close();
		delete _pInfoCollector;
	}

	pGlog = m_pReporter;

	glog(Log::L_DEBUG, L"===================== Loading isaAlarm interface ======================");

	DWORD dwSize;
	// add by jie 2005/09/09
	{
		// get the crash dump path
		static wchar_t wszDumpPath[256];
		dwSize = sizeof(wszDumpPath);
		getConfigValue(_T("CrashDumpPath"), wszDumpPath, _T("c:\\isa\\crashdump"), &dwSize, true, true);
		
		if (!validatePath(wszDumpPath))
		{
			glog(Log::L_ERROR, L"CrashDumpPath %s error", wszDumpPath);
			return false;
		}

		static DWORD dwDumpFullMemory;
		getConfigValue(L"CrashDumpFullMemory", &dwDumpFullMemory, true, true, true);

		_crashDump.setDumpPath(wszDumpPath);
		_crashDump.enableFullMemoryDump(dwDumpFullMemory);
		_crashDump.setExceptionCB(CrashExceptionCallBack);		
	}

	wchar_t  wszFilename[256];	
	dwSize = sizeof(wszFilename);
	getConfigValue(_T("ConfigFileName"), wszFilename, L"", &dwSize, true, true);
	char  szFilename[256];
	WideCharToMultiByte(CP_ACP, NULL, wszFilename, -1, szFilename, sizeof(szFilename), NULL, NULL);

	_pInfoCollector = new InfoCollector();
	if(!_pInfoCollector->init(szFilename))
	{
		glog(Log::L_ERROR, L"InfoCollector init failed, config filename %s", wszFilename);
		return S_FALSE;
	}
	
	BaseSchangeServiceApplication::OnInit();

	glog(Log::L_DEBUG, L"isaAlarm service ready");
	
	return S_OK;
}

HRESULT AlarmService::OnStart()
{
	if (_pInfoCollector)
	{
		 if (_pInfoCollector->start())
		 {
			 return BaseSchangeServiceApplication::OnStart();
		 }
	}
	
	return S_FALSE;	
}

HRESULT AlarmService::OnStop()
{
	if (_pInfoCollector)
	{
		_pInfoCollector->close();
		delete _pInfoCollector;
		_pInfoCollector = NULL;
	}

	pGlog = &NullLogger;

	return BaseSchangeServiceApplication::OnStop();
}

HRESULT AlarmService::OnUnInit()
{
	if (_pInfoCollector)
	{
		_pInfoCollector->close();
		delete _pInfoCollector;
		_pInfoCollector = NULL;
	}

	pGlog = &NullLogger;

	return BaseSchangeServiceApplication::OnUnInit();
}

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	
	glog( Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);

	glog.flush();
}

