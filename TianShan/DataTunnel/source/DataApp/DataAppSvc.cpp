// DBSyncServ.cpp: implementation of the DBSyncServ class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DataAppSVC.h"
#include "DataAppImpl.h"
#include "DataAppMain.h"
#include "TianShanDefines.h"
#include "global.h"
#include "DataAppCfg.h"
using ZQ::common::BaseZQServiceApplication;
using ZQ::common::Log;

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

DataAppSVC g_server;

BaseZQServiceApplication* Application = &g_server;

DataTunnelAppMainThread* mainThread = NULL;
ZQ::common::Config::Loader<DODAppCfg> gDODAppServiceConfig("DataTunnelApp.xml");
ZQ::common::Config::ILoader *configLoader = &gDODAppServiceConfig;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DataAppSVC::DataAppSVC(): 
	BaseZQServiceApplication()
{
   m_pIceLog = NULL;
}

DataAppSVC::~DataAppSVC()
{
	
}

HRESULT DataAppSVC::OnInit(void)
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
		strcat(path, FNSEPS "DataAppCacheDir" FNSEPS);
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
	m_pIceLog=new ZQ::common::FileLog((_strLogFolder+"BcastIce.log").c_str(),
							gDODAppServiceConfig.lIceTraceLogLevel,
							m_dwLogFileCount,
							gDODAppServiceConfig.lIceTraceLogSize);


	glog(Log::L_INFO,  CLOGFMT(DataAppSVC, "enter OnInit()"));

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
	mainThread = new DataTunnelAppMainThread;
	mainThread->m_pIceLog = m_pIceLog;
    
	glog(Log::L_INFO,  CLOGFMT(DataAppSVC, "leave OnInit()"));
	
	return S_OK;
}

HRESULT DataAppSVC::OnUnInit(void)
{	
	HRESULT hr;	
	glog(Log::L_INFO,  CLOGFMT(DataAppSVC, "enter OnUnInit()"));

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

	glog(Log::L_INFO,  CLOGFMT(DataAppSVC, "leave OnUnInit()"));

	return S_OK;
}

HRESULT DataAppSVC::OnStart(void)
{
	HRESULT hr;
	hr = BaseZQServiceApplication::OnStart();
	
	glog(Log::L_INFO,  CLOGFMT(DataAppSVC, "enter OnStart()"));
	
	if(mainThread)
	{
		mainThread->start();	
	}

	glog(Log::L_INFO, CLOGFMT(DataAppSVC,  "leave DataAppSVC::OnStart()"));

    return S_OK;	//	success
}

HRESULT DataAppSVC::OnStop(void)
{
	glog(Log::L_INFO,  CLOGFMT(DataAppSVC, "enter DataAppSVC::OnStop()"));

	g_bServiceStarted = FALSE;

	if(mainThread)
	{
		mainThread->stop();
	}

	HRESULT hr;
	hr = BaseZQServiceApplication::OnStop();

	glog(Log::L_INFO,  CLOGFMT(DataAppSVC, "leave OnStop()"));
	
	return hr;
}

//////////////////////////////////////////////////////////////////////////

/*#include "log.h"

ZQ::common::DebugMsg dodapp_log(Log::L_DEBUG);

int main(int argc, char* argv[])
{
	HANDLE handles[2];
	HANDLE stopEvent = CreateEventW(NULL, TRUE, FALSE, L"DODAppStop");
	mainThread = new DataTunnelAppMainThread;
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
