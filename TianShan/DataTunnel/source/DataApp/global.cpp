// global.cpp: implementation of the global class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "global.h"

using ZQ::common::Log;

// cache path of data source
std::string			sourceCachePath;

// space name of current DODApp
std::string			configSpaceName;

// active channel manager
ActiveDataMgr	ActiveDataManager;

bool				g_bServiceStarted;

bool initGlobal()
{
	char hostName[MAX_PATH];
	configSpaceName = gDODAppServiceConfig.szSpaceName;
	
	if(configSpaceName.size() < 1)
	{
		gethostname(hostName, MAX_PATH * sizeof(char));
		configSpaceName = hostName;
	}
	if(configSpaceName.size() < 1)
	{			
		glog(ZQ::common::Log::L_ERROR, "Parser space name error, please set spacename in DataTunnelApp.xml");
		return false;
	}
	return true;
}

ZQ::common::MiniDump g_minidump;

void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	
	glog(ZQ::common::Log::L_ERROR,
		"Crash exception callback called,ExceptonCode 0x%08x, "
		"ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);
}