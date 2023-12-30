// DBSyncServ.cpp: implementation of the DBSyncServ class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DODAppSvc.h"
#include "DODAppImpl.h"
#include "DODAppMain.h"
#include "TianShanDefines.h"
#include "global.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using ZQ::common::BaseSchangeServiceApplication;
using ZQ::common::Log;
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

DODAppSVC g_server;

BaseSchangeServiceApplication* Application = &g_server;

DODAppMainThread* mainThread = NULL;

DODAppSVC::DODAppSVC(): 
	BaseSchangeServiceApplication()
{

}

DODAppSVC::~DODAppSVC()
{
	
}

HRESULT DODAppSVC::OnInit(void)
{
	HRESULT hr;

	glog(Log::L_DEBUG, L"Entering DODAppService::OnInit()");

	hr = BaseSchangeServiceApplication::OnInit();

    g_bServiceStarted = TRUE;

    WCHAR dumpPath[MAX_PATH];
	GetModuleFileNameW(NULL, dumpPath, sizeof(dumpPath));
	 
	TCHAR* c = dumpPath + wcslen(dumpPath) - 1;
	while (*c) {
		 if (*c == _T('\\')) {
			 *c = 0;
			 break;
		 }		 
		 c --;
	 }
 
	g_minidump.setDumpPath(dumpPath);	
	g_minidump.enableFullMemoryDump(true);
	g_minidump.setExceptionCB(MiniDumpCallback);
	
	assert(!mainThread);
	mainThread = new DODAppMainThread;

	glog(Log::L_DEBUG, L"Leaving DODAppService::OnInit()");
	
	return S_OK;
}

HRESULT DODAppSVC::OnUnInit(void)
{	
	HRESULT hr;	
	glog(Log::L_DEBUG, L"Entering DODAppService::OnUnInit()");

	mainThread->destory();
	if(mainThread)
	{
		delete mainThread;
		mainThread = NULL;
	}

	hr = BaseSchangeServiceApplication::OnUnInit();

	glog(Log::L_DEBUG, L"Leaving DODAppService::OnUnInit()");

	return S_OK;
}

HRESULT DODAppSVC::OnStart(void)
{
	HRESULT hr;
	 hr = BaseSchangeServiceApplication::OnStart();
	
	glog(Log::L_DEBUG, L"Entering DODAppService::OnStart()");
	
	mainThread->start();	

	glog(Log::L_DEBUG, L"leaving DODAppService::OnStart()");

    return S_OK;	//	success
}

HRESULT DODAppSVC::OnStop(void)
{
	glog(Log::L_DEBUG, L"Entering DODAppService::OnStop()");

	g_bServiceStarted = FALSE;
	mainThread->stop();

	HRESULT hr;
	hr = BaseSchangeServiceApplication::OnStop();

	glog(Log::L_DEBUG, L"Leaving DODAppServiceServ::OnStop()");
	
	return hr;
}

//////////////////////////////////////////////////////////////////////////

/*#include "log.h"

ZQ::common::DebugMsg dodapp_log(Log::L_DEBUG);

int main(int argc, char* argv[])
{
	HANDLE handles[2];
	HANDLE stopEvent = CreateEventW(NULL, TRUE, FALSE, L"DODAppStop");
	mainThread = new DODAppMainThread;
	mainThread->start();
	HANDLE threadHandle = mainThread->getThreadHandle();

	handles[0] = stopEvent;
	handles[1] = threadHandle;
	
	DWORD r = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
	DWORD handleIndex;
	if (r >= WAIT_OBJECT_0) {
		handleIndex = r - WAIT_OBJECT_0;
	}
	
	if (r == 0) {
		mainThread->stop();
	}

	return 0;
}*/
