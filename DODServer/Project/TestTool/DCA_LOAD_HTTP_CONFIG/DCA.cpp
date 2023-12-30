// This is the main project file for VC++ application project 
// generated using an Application Wizard.

#include "stdafx.h"
#include <Windows.h>
#include <Winsvc.h>
#include <stdio.h>
#include <TCHAR.H>
//#using <mscorlib.dll>

//using namespace System;
/*
int _tmain()
{
    // TODO: Please replace the sample code below with your own.
    Console::WriteLine(S"Hello World");
	return 0;
}*/

#define SZSERVICENAME    "DCA Service"     //service name.
#define SZAPPNAME        "DCA Service"	 //service application name.
#define SZAPPDESCRIPTION        "DOD client agent service"	 //service description.

#include "DODClientAgent.h"

char gszConfigurationFile[256];

SERVICE_STATUS          gServiceStatus;				//service status
SERVICE_STATUS_HANDLE   gServiceStatusHandle;		//Handler' handle

//void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
VOID WINAPI ServiceMainEntry (DWORD argc, LPTSTR *argv);

//void WINAPI Handler(DWORD fdwControl)
VOID WINAPI ServiceHandler (DWORD opcode); 

DWORD WINAPI DODRunhread(LPVOID lpParam); 

BOOL	g_bStop=FALSE;
BOOL	g_bRealExit=FALSE;

int RunThread(void)
{
	DWORD IDThread;
	g_bStop=FALSE;
	g_bRealExit=FALSE;
	HANDLE hSendThread=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)DODRunhread,NULL,0 , &IDThread); 
	return 1;
}
int StopThread(void)
{	
	g_bStop=TRUE;
	Sleep(3000);
	for(;g_bRealExit==FALSE;)
	{
		Sleep(2);
	}
//	Clog(LOG_DEBUG,"[DCA] StopThread. exit");
	return 1;
}

//install service
int InstallService();

//uninstall service
void RemoveService();

///////////////////////////////////////////////////////////////////////////////////////////////////

CWinApp theApp;
using namespace std;

void InitiServer(void)
{
	CString        strCurDir;
	char           sModuleName[MAX_PATH];
	DWORD dSize = GetModuleFileName(NULL,sModuleName,MAX_PATH);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.ReverseFind('\\');
	strCurDir = strCurDir.Left(nIndex); //end with "\\"
	strCurDir=strCurDir+"\\"+"DODClientAgent.log";	
	ClogEstablishSettings(strCurDir, LOGMAXLEVEL, LOGMAXSIZE);
}

int main(int argc, TCHAR* argv[], TCHAR* envp[])
{
	printf("[DCA] InitiServer before....\n");
	
	InitiServer();
	Clog( LOG_DEBUG, _T("DOD Client Agent will create ! ---------------------------------------------------------------------------------------------------------") );
	int nRetCode = 0;
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
//		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		printf("[DCA] Fatal Error: MFC initialization failed\n");

		nRetCode = 1;
	}
	//////////////////////////////////////////////////////////////
	if(argc==2 && strcmp(argv[1],"install")==0 )
	{
		printf("[DCA]start to install service...................................\n");
		if(InstallService()==0)
		{
			printf("[DCA]successful to install service.\n");
		}
		else
		{
			printf("[DCA]failed to install service.\n");
		}
		printf("[DCA]finished to install service.\n");
		return 1;
	}

	//(2)uninstall
	else if( argc==2 && strcmp(argv[1],"uninstall")==0 )  
	{
		printf("[DCA]start to uninstall service............................\n");
		RemoveService();
		printf("[DCA]finished to uninstall service.\n");
		return 2;
	}

	else if( argc==2 && strcmp(argv[1],"debug")==0 )//
	{		
		
		if(!RunThread())
		{
			//UninitSocket();
			printf("Failed to create service manager.\n");
			printf("process any key and enter to exit.\n");
			int n = getchar();
			return 3;
		}
		else
		{
			printf("DCA is successful to start.\n\n");		
			while( 1 )
			{
				printf("If you want to exit, process q and enter:\n");
				int n = getchar();
				if( n == 'q' )
				{
					printf("star to exit DCA..............\n");
					StopThread();
					//	UninitSocket();
					return 0;
				}
			}
		}
		return 0;
	}
	else
		if( argc==2)
		{
			printf("[DCA] service  command parameter can not be parsed........\n");
			printf("[DCA] service  install........\n");
			printf("[DCA] service  uninstall.......\n");
			return 1;
		}	

	printf("[DCA]Start service...................................................\n");

	//HRESULT hr = ::CoInitialize(NULL);
	//if( FAILED(hr) )
	//{
	//	printf("[DCA]Failed to call CoInitialize. hr=%d.\n", hr);
	//	//UninitSocket();
	//	return FALSE;
	//}
	DWORD dwError;
	SERVICE_TABLE_ENTRY   DispatchTable[] = 
	{ 
		{ TEXT(SZSERVICENAME),ServiceMainEntry}, 
		{ NULL,	NULL          } 
	}; 
	printf("[DCA]call StartServiceCtrlDispatcher.\n");
	if (!StartServiceCtrlDispatcher( DispatchTable)) 
	{ 

		dwError=GetLastError();
		/* The service process could not connect to the service controller.*/
		printf("[DCA]failed to call StartServiceCtrlDispatcher, error code is %d.\n",dwError);
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
	printf("[DCA]service is exited.\n");
	//::CoUninitialize();
	//UninitSocket();
	return 0;
}
//////////////////////////////////////////////////////////////////////
//service main function entry.
void WINAPI ServiceMainEntry (DWORD argc, LPTSTR *argv) 
{ 

	//Clog( LOG_DEBUG, _T("ServiceMainEntry") );
	//initialize server's status.
	//HRESULT hr = ::CoInitialize(NULL);
	DWORD status; 
	gServiceStatus.dwServiceType             = SERVICE_WIN32;//SERVICE_WIN32_OWN_PROCESS;
	gServiceStatus.dwCurrentState            = SERVICE_START_PENDING; 
	gServiceStatus.dwControlsAccepted		 = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE; 
	gServiceStatus.dwWin32ExitCode           = 0;
	gServiceStatus.dwServiceSpecificExitCode = 0; 
	gServiceStatus.dwCheckPoint              = 0; 
	gServiceStatus.dwWaitHint                = 0; //300;

	//register service handler.
	//Clog(LOG_DEBUG,"[DCA]start to call RegisterServiceCtrlHandler.");
	gServiceStatusHandle = RegisterServiceCtrlHandler( 
		TEXT(SZSERVICENAME), 
		ServiceHandler); 
	if(gServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
	{ 
		Clog(LOG_DEBUG,"[DCA]failed to call RegisterServiceCtrlHandler.");
		gServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gServiceStatusHandle,&gServiceStatus);
	//	::CoUninitialize();
		return; 
	}
	//Clog(LOG_DEBUG,"[DCA]successful to call RegisterServiceCtrlHandler.");

	//report service that service is pending running.
	//Clog(LOG_DEBUG,"[DCA]start to report service manager that service is pending running.");
	if(!SetServiceStatus (gServiceStatusHandle, &gServiceStatus)) 
	{ 
		Clog(LOG_DEBUG,"[DCA]failed to report service status to service manager that service is pending running.");
		
		StopThread();

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
	//Clog(LOG_DEBUG,"[DCA]successful to report service manager that service is pending running.");

	//create server.
	//if failed to create, report stopped and return.
	//Clog(LOG_DEBUG,"[DCA]start to create gSvrManager.");
	if(!RunThread())
		//if(!gSvrManager.Create(gszConfigurationFile))
	{
		Clog(LOG_DEBUG,"[DCA]failed to create gSvrManager.");
		gServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gServiceStatusHandle,&gServiceStatus);
		::CoUninitialize();
		return ;
	}
	Clog(LOG_DEBUG,"[DCA]successful to create gSvrManager.");

	//service is created, report server is running.
	//Clog(LOG_DEBUG,"[DCA]start to report service manager that service is now running.");
	gServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
	gServiceStatus.dwCheckPoint         = 0;
	gServiceStatus.dwWaitHint           = 300; 
	if(!SetServiceStatus (gServiceStatusHandle, &gServiceStatus)) 
	{ 
		Clog(LOG_DEBUG,"[DCA]failed to report service manager that service is now running.");
		StopThread();
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
	//Clog(LOG_DEBUG,"[DCA]successful to report service manager that service is now running.");

	//::CoUninitialize();
	return; 
} 

//service handler function.
VOID WINAPI ServiceHandler (DWORD Opcode) 
{ 
//	HRESULT hr = ::CoInitialize(NULL);
	DWORD status; 
	switch(Opcode) 
	{ 
	case SERVICE_CONTROL_PAUSE:
		Clog(LOG_DEBUG,"[DCA]pause command is gotten from service manager.");
		//service is pending pausing.
		gServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
		SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
		//start to pause service. 
		gServiceStatus.dwCurrentState = SERVICE_PAUSED; 
		break; 
	case SERVICE_CONTROL_CONTINUE:
		Clog(LOG_DEBUG,"[DCA]continue command is gotten from service manager.");
		//service is pending continue.
		gServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
		SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
		//start to resume service. 
		gServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
	case SERVICE_CONTROL_STOP: 
		Clog(LOG_DEBUG,"[DCA]stop command is gotten from service manager.");
		//service is pending stopping.
		gServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(gServiceStatusHandle, &gServiceStatus);
		//start to stop service. 
		StopThread();
		//service is stopped.
		gServiceStatus.dwWin32ExitCode = 0; 
		gServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
		gServiceStatus.dwCheckPoint    = 0; 
		gServiceStatus.dwWaitHint      = 0; 
		if (!SetServiceStatus (gServiceStatusHandle, &gServiceStatus))
		{ 
			status = GetLastError(); 
		} 
	//	::CoUninitialize();
		return; 
	default:
		Clog(LOG_DEBUG,"[DCA]unknown command is gotten from service manager.Opcode=%d",Opcode);
		;
	}
	// Send current status. 
	if (!SetServiceStatus (gServiceStatusHandle,  &gServiceStatus)) 
	{ 
		status = GetLastError(); 
	} 
	//::CoUninitialize();
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
// ------------------------------------------------------ Modified by zhenan_ji at 2005年10月21日 10:20:58
			SERVICE_DEMAND_START,		    //启动类型
			SERVICE_ERROR_NORMAL,			//错误控制类型
			szPath,							//服务程序磁盘文件的路径
			NULL,							//服务不属于任何组
			NULL,							//没有tag标识符
			NULL,			                //启动服务所依赖的服务或服务组,这里仅仅是一个空字符串
			NULL,							//LocalSystem 帐号
			NULL);
		if(schService)
		{
			SERVICE_DESCRIPTION sdBuf;
			sdBuf.lpDescription = TEXT(SZAPPDESCRIPTION);
			if (ChangeServiceConfig2 (
				schService, SERVICE_CONFIG_DESCRIPTION, &sdBuf))
			{
				//MessageBox(NULL,"Change SUCCESS","",MB_SERVICE_NOTIFICATION); 
			}

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

DWORD WINAPI DODRunhread(LPVOID lpParam)
{
	CDODClientAgent g_pDCAagent;

	g_pDCAagent.create();	

	for(;g_bStop==FALSE;)
	{
		Sleep(2);
	}
	g_pDCAagent.Destroy();

	g_bRealExit=TRUE;
//	Clog(LOG_DEBUG,"[DCA] DODRunhread. exit");
	return 1;

}