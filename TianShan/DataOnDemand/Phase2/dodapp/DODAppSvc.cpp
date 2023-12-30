// DBSyncServ.cpp: implementation of the DBSyncServ class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DODAppSvc.h"
#include "DODAppImpl.h"
#include "DODAppMain.h"
#include "TianShanDefines.h"
#include "global.h"
#include "DODAppCfg.h"
using ZQ::common::BaseZQServiceApplication;
using ZQ::common::Log;

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

DODAppSVC g_server;

BaseZQServiceApplication* Application = &g_server;

DODAppMainThread* mainThread = NULL;


ZQ::common::Config::Loader<DODAppCfg> gDODAppServiceConfig("DODApp.xml");
ZQ::common::Config::ILoader *configLoader = &gDODAppServiceConfig;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DODAppSVC::DODAppSVC(): 
	BaseZQServiceApplication()
{
   m_pIceLog = NULL;
}

DODAppSVC::~DODAppSVC()
{
	
}

HRESULT DODAppSVC::OnInit(void)
{
	HRESULT hr;
	std::string			_strLogFolder;

	sourceCachePath = gDODAppServiceConfig.szCachepath;

	if(sourceCachePath.empty())
	{	
		char path[MAX_PATH];
		if (::GetModuleFileNameA(NULL, path, MAX_PATH-1)>0)
		{
			char* p = strrchr(path, FNSEPC);
			if (NULL !=p)
			{
				*p='\0';
			}
		}
		strcat(path, FNSEPS "DODAppCachedir" FNSEPS);
		sourceCachePath = path;		
	}

	// open dictionary
   	::CreateDirectoryA((sourceCachePath + FNSEPS).c_str(), NULL);
	int length = sourceCachePath.size();

	if(sourceCachePath[length - 1] != '\\') {
		sourceCachePath += FNSEPS;
	}

    _strLogFolder = m_wsLogFolder;
	
	int size = _strLogFolder.size();
	if(size > 0 && _strLogFolder[size -1] != '\\' && _strLogFolder[size -1] != '/')
		_strLogFolder += "\\";
	m_pIceLog=new ZQ::common::FileLog((_strLogFolder+"DODApplicationIce.log").c_str(),
							gDODAppServiceConfig.lIceTraceLogLevel,
							ZQLOG_DEFAULT_FILENUM,
							gDODAppServiceConfig.lIceTraceLogSize);


	glog(Log::L_INFO, "Enter DODAppService::OnInit()");

	hr = BaseZQServiceApplication::OnInit();

    g_bServiceStarted = TRUE;
	
    TCHAR dumpPath[MAX_PATH];
	GetModuleFileName(NULL, dumpPath, sizeof(dumpPath));
	 
	TCHAR* P = dumpPath + strlen(dumpPath) - 1;
	while (*P) {
		 if (*P == _T('\\')) {
			 *P = 0;
			 break;
		 }		 
		 P--;
	 }
 
	g_minidump.setDumpPath(dumpPath);	
	g_minidump.enableFullMemoryDump(true);
	g_minidump.setExceptionCB(MiniDumpCallback);
	
	assert(!mainThread);
	mainThread = new DODAppMainThread;
	mainThread->m_pIceLog = m_pIceLog;
    
	glog(Log::L_INFO, L"Leave DODAppService::OnInit()");
	
	return S_OK;
}

HRESULT DODAppSVC::OnUnInit(void)
{	
	HRESULT hr;	
	glog(Log::L_INFO, L"Enter DODAppService::OnUnInit()");

	mainThread->destory();
	if(mainThread)
	{
		delete mainThread;
		mainThread = NULL;
	}
	if(m_pIceLog)
	{
		try
		{
			delete m_pIceLog;
			m_pIceLog = NULL;
		}
		catch(...){
		}
	}

	hr = BaseZQServiceApplication::OnUnInit();

	glog(Log::L_INFO, L"Leave DODAppService::OnUnInit()");

	return S_OK;
}

HRESULT DODAppSVC::OnStart(void)
{
	HRESULT hr;
	hr = BaseZQServiceApplication::OnStart();
	
	glog(Log::L_INFO, L"Enter DODAppService::OnStart()");
	
	mainThread->start();	

	glog(Log::L_INFO, L"Leave DODAppService::OnStart()");

    return S_OK;	//	success
}

HRESULT DODAppSVC::OnStop(void)
{
	glog(Log::L_INFO, L"Enter DODAppService::OnStop()");

	g_bServiceStarted = FALSE;

	mainThread->stop();

	HRESULT hr;
	hr = BaseZQServiceApplication::OnStop();

	glog(Log::L_INFO, L"Leave DODAppServiceServ::OnStop()");
	
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
