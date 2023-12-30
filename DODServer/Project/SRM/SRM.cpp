// SRM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SRM.h"
#include <Windows.h>
#include <Winsvc.h>
#include <stdio.h>
#include <TCHAR.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 //Clog( LOG_DEBUG, _T("catch Error in CDSAResource::RestartConnec") );
/////////////////////////////////////////////////////////////////////////////////////
// The one and only application object
#define SZSERVICENAME    "SRM Service"     //service name.
#define SZAPPNAME        "SRM Service"	 //service application name.
char gszConfigurationFile[256];

SERVICE_STATUS          gServiceStatus;				//service status
SERVICE_STATUS_HANDLE   gServiceStatusHandle;		//Handler' handle

//void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
VOID WINAPI ServiceMainEntry (DWORD argc, LPTSTR *argv);

//void WINAPI Handler(DWORD fdwControl)
VOID WINAPI ServiceHandler (DWORD opcode); 

//install service
int InstallService();

//uninstall service
void RemoveService();

///////////////////////////////////////////////////////////////////////////////////////////////////
#include"SRManager.h"


CSRManager gSRManager;

CWinApp theApp;
using namespace std;

void InitiServer(void)
{
   ClogEstablishSettings(LOGFILENAME, LOGMAXLEVEL, LOGMAXSIZE);
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
   // InitiServer();
    //////////////////////////////////// /////
    int nIndex;
    char   sModuleName[1025];
    CString   szsModuleName;
    CString   szsConfigurationFileName;
    DWORD dwSize = GetModuleFileName(NULL, sModuleName, 1024);
    sModuleName[dwSize] = '\0';
    szsModuleName = sModuleName;
    nIndex = szsModuleName.ReverseFind('.');
    szsConfigurationFileName = szsModuleName.Left(nIndex)+".Log";
    ClogEstablishSettings(szsConfigurationFileName, LOGMAXLEVEL, LOGMAXSIZE);


/////////////////////////////////////////////
    Clog( LOG_DEBUG, _T("Main") );
	int nRetCode = 0;
   //gSRManager.create();
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
//////////////////////////////////////////////////////////////
    if(argc==2 && strcmp(argv[1],"install")==0 )
	{
		printf("[SRM]start to install service...................................\n");
		if(InstallService()==0)
		{
			printf("[SRM]successful to install service.\n");
		}
		else
		{
			printf("[SRM]failed to install service.\n");
		}
		printf("[SRM]finished to install service.\n");
		return 1;
	}

	//(2)uninstall
	else if( argc==2 && strcmp(argv[1],"uninstall")==0 )  
	{
		printf("[SRM]start to uninstall service............................\n");
		RemoveService();
		printf("[SRM]finished to uninstall service.\n");
		return 2;
	}

	else if( argc==2 && strcmp(argv[1],"application")==0 )//
	{
		
		printf("[SRM]start to check MOG pay license.\n");
		if( !MOGRegistry() )
		{
			printf("[SRM]MOG pay license can not be found.Please registry MOG Solutions pay license first.\n");
			return 1;
		}
		printf("[SRM]successful to check MOG pay license.\n");
		printf("start to run as application................\n");
		/*if(!gSvrManager.Create(gszConfigurationFile))*/
		if(!gSRManager.create())
		{
			//UninitSocket();
			printf("Failed to create service manager.\n");
			printf("process any key and enter to exit.\n");
			int n = getchar();
			return 3;
		}
		else
		{
			printf("SRM is successful to start.\n\n");		
			while( 1 )
			{
				printf("If you want to exit, process q and enter:\n");
				int n = getchar();
				if( n == 'q' )
				{
					printf("star to exit SRM..............\n");
					gSRManager.Destroy();
				    //	UninitSocket();
					return 0;
				}
			}
		}
        //UninitSocket();
		::CoUninitialize();
		return 0;
	}
	//(3)start service.

	//InitSocket();
		
	printf("[SRM]Start service...................................................\n");
	
	HRESULT hr = ::CoInitialize(NULL);
	if( FAILED(hr) )
	{
		printf("[SRM]Failed to call CoInitialize. hr=%d.\n", hr);
        //UninitSocket();
		return FALSE;
	}
	DWORD dwError;
	SERVICE_TABLE_ENTRY   DispatchTable[] = 
    { 
        { TEXT(SZSERVICENAME),ServiceMainEntry}, 
        { NULL,	NULL          } 
    }; 
	printf("[SRM]call StartServiceCtrlDispatcher.\n");
    if (!StartServiceCtrlDispatcher( DispatchTable)) 
    { 
      
		dwError=GetLastError();
        /* The service process could not connect to the service controller.*/
		printf("[SRM]failed to call StartServiceCtrlDispatcher, error code is %d.\n",dwError);
		switch(dwError)
		{
		case ERROR_INVALID_DATA:
			OutputDebugStringA("error: invalid data.\n");
			break;
		case ERROR_SERVICE_ALREADY_RUNNING:
			OutputDebugStringA("error: server already running.\n");
			break;
		default:
			OutputDebugStringA("unknown error.\n");
			break;
		}
    } 
	printf("[SRM]service is exited.\n");
	printf(" ");
	::CoUninitialize();
    //UninitSocket();
	return 0;

/////////////////////////////////////////////////////////////////
/*	gSRManager.create();
    cout << "SRM server Start!" << endl;
	Sleep(INFINITE);
	//Sleep(10);
	return nRetCode;*/
}
//////////////////////////////////////////////////////////////////////
//service main function entry.
void WINAPI ServiceMainEntry (DWORD argc, LPTSTR *argv) 
{ 

    Clog( LOG_DEBUG, _T("ServiceMainEntry") );
	//initialize server's status.
	HRESULT hr = ::CoInitialize(NULL);
    DWORD status; 
	gServiceStatus.dwServiceType             = SERVICE_WIN32;//SERVICE_WIN32_OWN_PROCESS;
	gServiceStatus.dwCurrentState            = SERVICE_START_PENDING; 
	gServiceStatus.dwControlsAccepted		 = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE; 
	gServiceStatus.dwWin32ExitCode           = 0;
	gServiceStatus.dwServiceSpecificExitCode = 0; 
    gServiceStatus.dwCheckPoint              = 0; 
    gServiceStatus.dwWaitHint                = 0; //300;
	
	//register service handler.
	Clog(0,"[SRM]start to call RegisterServiceCtrlHandler.");
    gServiceStatusHandle = RegisterServiceCtrlHandler( 
        TEXT(SZSERVICENAME), 
		ServiceHandler); 
	if(gServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    { 
		Clog(0,"[SRM]failed to call RegisterServiceCtrlHandler.");
		gServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gServiceStatusHandle,&gServiceStatus);
		::CoUninitialize();
        return; 
    }
	Clog(0,"[SRM]successful to call RegisterServiceCtrlHandler.");

	//report service that service is pending running.
	Clog(0,"[SRM]start to report service manager that service is pending running.");
	if(!SetServiceStatus (gServiceStatusHandle, &gServiceStatus)) 
    { 
		Clog(0,"[SRM]failed to report service status to service manager that service is pending running.");
		gSRManager.Destroy();
        status = GetLastError(); 
        gServiceStatus.dwCurrentState			  = SERVICE_STOPPED; 
        gServiceStatus.dwCheckPoint				  = 0; 
        gServiceStatus.dwWaitHint				  = 0; 
        gServiceStatus.dwWin32ExitCode			  = status; 
        gServiceStatus.dwServiceSpecificExitCode  = 300; 
		SetServiceStatus (gServiceStatusHandle, &gServiceStatus); 
		::CoUninitialize();
        return;
    }
	Clog(0,"[SRM]successful to report service manager that service is pending running.");

	Clog(0, "[SRM]start to check user.");
	/*if( !CheckUser() )
	{
		Clog(0, "[SRM]Failed to check user.");
		gServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gServiceStatusHandle,&gServiceStatus);
		::CoUninitialize();
		return ;
	}
	Clog(0, "[SRM]successful to check user.");*/

	Clog(0, "[SRM]Start to check MOG pay license.");
	if(!MOGRegistry())
	{
		Clog(0,"[SRM]MOG pay license can not be found.Please registry MOG Solutions pay license first.");
		gServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gServiceStatusHandle,&gServiceStatus);
		::CoUninitialize();
		return ;
	}
	Clog(0, "[SRM]successful to check MOG pay license.");

	//create server.
	//if failed to create, report stopped and return.
	Clog(0,"[SRM]start to create gSvrManager.");
	if(!gSRManager.create())
	//if(!gSvrManager.Create(gszConfigurationFile))
	{
		Clog(0,"[SRM]failed to create gSvrManager.");
		gServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gServiceStatusHandle,&gServiceStatus);
		::CoUninitialize();
		return ;
	}
	Clog(0,"[SRM]successful to create gSvrManager.");

	//service is created, report server is running.
	Clog(0,"[SRM]start to report service manager that service is now running.");
	gServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
	gServiceStatus.dwCheckPoint         = 0;
	gServiceStatus.dwWaitHint           = 300; 
	if(!SetServiceStatus (gServiceStatusHandle, &gServiceStatus)) 
    { 
		Clog(0,"[SRM]failed to report service manager that service is now running.");
		gSRManager.Destroy();
        status = GetLastError(); 
        gServiceStatus.dwCurrentState			  = SERVICE_STOPPED; 
        gServiceStatus.dwCheckPoint				  = 0; 
        gServiceStatus.dwWaitHint				  = 0; 
        gServiceStatus.dwWin32ExitCode			  = status; 
        gServiceStatus.dwServiceSpecificExitCode  = 300; 
		SetServiceStatus (gServiceStatusHandle, &gServiceStatus); 
		::CoUninitialize();
        return;
    }
	Clog(0,"[SRM]successful to report service manager that service is now running.");

	::CoUninitialize();
	return; 
} 

//service handler function.
VOID WINAPI ServiceHandler (DWORD Opcode) 
{ 
	HRESULT hr = ::CoInitialize(NULL);
    DWORD status; 
    switch(Opcode) 
    { 
	case SERVICE_CONTROL_PAUSE:
		Clog(0,"[SRM]pause command is gotten from service manager.");
		//service is pending pausing.
		gServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
		SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
		//start to pause service. 
		gSRManager.Pause();
		gServiceStatus.dwCurrentState = SERVICE_PAUSED; 
		break; 
	case SERVICE_CONTROL_CONTINUE:
		Clog(0,"[SRM]continue command is gotten from service manager.");
		//service is pending continue.
		gServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
		SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
		//start to resume service. 
		gSRManager.Resume();
		gServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_STOP: 
		Clog(0,"[SRM]stop command is gotten from service manager.");
		//service is pending stopping.
		gServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
		//start to stop service. 
		gSRManager.Destroy();
		//service is stopped.
		gServiceStatus.dwWin32ExitCode = 0; 
		gServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
		gServiceStatus.dwCheckPoint    = 0; 
		gServiceStatus.dwWaitHint      = 0; 
		if (!SetServiceStatus (gServiceStatusHandle, &gServiceStatus))
		{ 
			status = GetLastError(); 
		} 
		::CoUninitialize();
		return; 
	default:
		Clog(0,"[SRM]unknown command is gotten from service manager.");
		;
    }
	// Send current status. 
    if (!SetServiceStatus (gServiceStatusHandle,  &gServiceStatus)) 
    { 
		status = GetLastError(); 
    } 
	::CoUninitialize();
    return; 
}


//install windows service
//0: successful to install
//1: failed to install
int InstallService()
{
	int nRet=1;
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	TCHAR szPath[512];
	//得到程序磁盘文件的路径
	if(GetModuleFileName(NULL,szPath,512)==0)
	{
		_tprintf(TEXT("Unable to install %s - %s \n"),TEXT(SZAPPNAME),GetLastError());//@1获取调用函数返回的最后错误码
		return 1;
	}
	//打开服务管理数据库
	schSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(schSCManager)
	{
		//登记服务程序
		schService=CreateService(
			schSCManager,					//服务管理数据库句柄
			TEXT(SZSERVICENAME),	        //服务名
			TEXT(SZAPPNAME),		        //用于显示服务的标识
			SERVICE_ALL_ACCESS,			    //响应所有的访问请求
			SERVICE_WIN32_OWN_PROCESS,		//服务类型
			SERVICE_AUTO_START,			    //启动类型
			SERVICE_ERROR_NORMAL,			//错误控制类型
			szPath,							//服务程序磁盘文件的路径
			NULL,							//服务不属于任何组
			NULL,							//没有tag标识符
			NULL,			                //启动服务所依赖的服务或服务组,这里仅仅是一个空字符串
			NULL,							//LocalSystem 帐号
			NULL);
		if(schService)
		{
			_tprintf(TEXT("%s installed. \n"),TEXT(SZAPPNAME));
			CloseServiceHandle(schService);
			nRet=0;
		}
		else
		{
			_tprintf(TEXT("CreateService failed - %d \n"),GetLastError());
			nRet=1;
		}
		CloseServiceHandle(schSCManager);
	}
	else
	{
		_tprintf(TEXT("OpenSCManager failed - %d \n"),GetLastError());
		nRet=1;
	}
	return nRet;
}
void RemoveService()
{
	SC_HANDLE	schService;
	SC_HANDLE	schSCManager;
	SERVICE_STATUS ssStatus;
	memset(&ssStatus,0,sizeof(SERVICE_STATUS));
	//打开服务管理数据库
	schSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(schSCManager)
	{
		//获取服务程序句柄
		schService=OpenService(schSCManager,TEXT(SZSERVICENAME),SERVICE_ALL_ACCESS);
		if(schService)
		{
			//试图停止服务
			if(ControlService(schService,SERVICE_CONTROL_STOP,&ssStatus))
			{
				Sleep(1000);				
				//等待服务停止
				while(QueryServiceStatus(schService,&ssStatus))
				{
					if(SERVICE_STOP_PENDING==ssStatus.dwCurrentState)
					{
						Sleep(1000);
					}
					else
					{
						break;
					}
				}
				if(SERVICE_STOPPED==ssStatus.dwCurrentState)
				{
					_tprintf(TEXT("\n %s stopped. \n"),TEXT(SZSERVICENAME));
				}
				else
				{
					_tprintf(TEXT("\n %s failed to stopp. \n"),TEXT(SZSERVICENAME));
				}
			}
			//删除已安装的服务程序安装
			if(DeleteService(schService))
			{
				_tprintf(TEXT("%s removed. \n"),TEXT(SZAPPNAME));
			}
			else
			{
				_tprintf(TEXT("DeleteService failed - %s. \n"), GetLastError());
			}
			CloseServiceHandle(schService);
		}
		else
		{
			_tprintf(TEXT("OpenService failed - %s \n"),GetLastError());
		}
		CloseServiceHandle(schSCManager);
	}
	else
	{
		_tprintf(TEXT("OpenSCManager failed - %s \n"),GetLastError());
	}
}





