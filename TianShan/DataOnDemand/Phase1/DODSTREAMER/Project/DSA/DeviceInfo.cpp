
/*
**	FILENAME			DeviceInfo.cpp
**
**	PURPOSE				The file is a .cpp file, used along with DeviceInfo.h.
**						this file is the primar file of the service, in function ServiceMain(), 
**						this is a class CSCDeviceInfo newed for detail work.
**						
**						
**
**	CREATION DATE		19-07-2004
**	LAST MODIFICATION	21-07-2004
**
**	AUTHOR				Leon.li (Interactive ZQ)
**
**
*/
/*
#define WINVER 0x0400
#include <afxwin.h>

#include <stdio.h>
#include <wtypes.h>
#include <winnt.h>
#include <winsvc.h>
#include <winuser.h>
*/

#include "DeviceInfo.h"
#include "clog.h"
//#include "CSCDeviceInfo.h"
//#include "CFileWatcher.h"
#include "DeviceAgent.h"
#include "DeviceInfoParser.h"

void application();
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv); 
VOID WINAPI ServiceCtrlHandler(DWORD dwControl);    // Handler Function

VOID WriteInLogFile(char *text);

// DWORD ServiceInitialization(DWORD argc, LPTSTR *argv, DWORD *specificError); 

FILE *g_LogFile;
char g_Msg[255];
BOOL g_isRunning = false;

SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   ServiceStatusHandle; 

//////////////////////////////////////////////////////////////////////////
// support crash dump
#include "MiniDump.h"
ZQ::common::MiniDump			crash_dumper;

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);

void WINAPI crashHandler(DWORD excepCode, PVOID excepAddr)
{
	DWORD threadId = GetCurrentThreadId();

	glog( ISvcLog::L_ERROR,  "crashHandler()\t"
		"excepCode 0x%08x, excepAddr 0x%08x, threadId: 0x%04x",
		excepCode, excepAddr, threadId);
	service_log.flush();
}

void initMiniDump()
{
	TCHAR dumpPath[MAX_PATH];
	GetModuleFileName(NULL, dumpPath, sizeof(dumpPath));

	TCHAR* c = dumpPath + strlen(dumpPath) - 1;
	while (*c) {
		if (*c == _T('\\')) {
			*c = 0;
			break;
		}

		c --;
	}
    
	crash_dumper.setDumpPath(dumpPath);
	crash_dumper.enableFullMemoryDump(TRUE);
	crash_dumper.setExceptionCB(crashHandler);	
}

// support crash dump
//////////////////////////////////////////////////////////////////////////

// First Implement main function
int main( int argc, char* argv[] )
{
	// init crash dumper first.
	initMiniDump();

//	The SERVICE_TABLE_ENTRY structure is used by the StartServiceCtrlDispatcher function 
//	to specify the ServiceMain function for a service that can run in the calling process. 
    SERVICE_TABLE_ENTRY DispatchTable[] = { { "DSA", ServiceMain }, { 0, 0 } };

    sprintf( g_Msg, "Log opened\n" );
    WriteInLogFile( g_Msg );

	// Install a Service if -i switch used
    if ( argc > 1 && !( stricmp(argv[1], "install") ) )
	{
        char szBuffer[255];
        char szPath[MAX_PATH];

        sprintf( g_Msg, "First Calling OpenSCManager() \n");
        WriteInLogFile( g_Msg );

//		The OpenSCManager function establishes a connection to the Service Control Manager(SCM) 
//		on the specified computer and opens the specified service control manager database. 

        SC_HANDLE scmHandle = OpenSCManager ( NULL, NULL, SC_MANAGER_ALL_ACCESS );

        if (scmHandle == NULL) // Perform error handling.
        {
            sprintf( g_Msg, "DSA! OpenSCManager error = %d\n", GetLastError() ); 
            WriteInLogFile(g_Msg);
        }

        GetModuleFileName( GetModuleHandle(NULL), szPath, MAX_PATH );
        
        strcpy( szBuffer, "\"" );
        strcat( szBuffer, szPath );
        strcat( szBuffer, "\"" );
 
		printf( "\n CreateService()! Installing Service %s", szPath );

        SC_HANDLE scHandle;
		scHandle = CreateService (
			scmHandle, 
			"DSA", 
            "DSA", 
			SERVICE_ALL_ACCESS, 
            SERVICE_WIN32_OWN_PROCESS, 
            SERVICE_AUTO_START, 
            SERVICE_ERROR_NORMAL, 
            szBuffer, NULL, NULL, NULL, NULL, NULL );

        if ( scHandle == NULL ) // Process error
        {
            sprintf(g_Msg, " DSA! CreateService error = %d\n", GetLastError() ); 
            WriteInLogFile(g_Msg);
        }

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);
    }
    else if ( argc > 1 && !( stricmp(argv[1], "uninstall" ) ) ) // Uninstall the Service
    {
        SC_HANDLE scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if (scmHandle == NULL) // Perform error handling.
        {
            sprintf(g_Msg, "DSA! OpenSCManager error = %d\n", GetLastError() ); 
            WriteInLogFile(g_Msg);
        }
   
		SC_HANDLE scHandle;
        scHandle = OpenService( scmHandle, "DSA", SERVICE_ALL_ACCESS );
        DeleteService( scHandle );

        printf("\nDeviceInfo uninstalled...");
    }
	else if(argc>1 && stricmp(argv[1], "application")==0 )
	{
		application();
	}
    else
    {
        sprintf( g_Msg, "StartServiceCtrlDispatcher()...\n" );
        
		WriteInLogFile(g_Msg);
        
		if ( !StartServiceCtrlDispatcher(DispatchTable) ) 
        { 
            sprintf( g_Msg,"DSA StartServiceCtrlDispatcher error = %d\n", GetLastError() ); 
            WriteInLogFile(g_Msg);
        }
    } 

    return 0;
}

VOID WriteInLogFile( char *text )
{/*
    g_LogFile = fopen( "DeviceInfoService.log", "a+t" );
    
	if ( g_LogFile )
    {
        fprintf( g_LogFile, text );
        fclose( g_LogFile );
    }
   */ 
	return;
}

// Second function to implement
VOID WINAPI ServiceMain( DWORD argc, LPTSTR *argv ) 
{ 
//    DWORD status; 
//    DWORD specificError; 

    ServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS; 
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP; 
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    ServiceStatusHandle = RegisterServiceCtrlHandler( "DSA", ServiceCtrlHandler ); 
 
    if ( ServiceStatusHandle == (SERVICE_STATUS_HANDLE) 0 ) 
    { 
        sprintf(g_Msg, "DSA! RegisterServiceCtrlHandler() failed %d\n", GetLastError() ); 
        WriteInLogFile( g_Msg );
        return; 
    } 
 
//    status = ServiceInitialization( argc, argv, &specificError ); 
//    if ( status != NO_ERROR ) 
//    { 
//        ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
//        ServiceStatus.dwCheckPoint         = 0; 
//        ServiceStatus.dwWaitHint           = 0; 
//        ServiceStatus.dwWin32ExitCode      = status; 
//        ServiceStatus.dwServiceSpecificExitCode = specificError; 
// 
//        SetServiceStatus( ServiceStatusHandle, &ServiceStatus ); 
//        return; 
//    } 

    ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    if ( !SetServiceStatus( ServiceStatusHandle, &ServiceStatus ) ) 
    { 
        sprintf( g_Msg,"DSA! SetServiceStatus() error %ld\n", GetLastError() ); 
        WriteInLogFile(g_Msg);
    } 

    g_isRunning = true;

    sprintf( g_Msg, "Just before processing Loop...\n" );
    WriteInLogFile(g_Msg);

//	This is where the service does its work. 
//	First we display a message on the desktop
    /*
	MessageBox(NULL, "Hello Service World", "DSA", MB_OK | MB_SERVICE_NOTIFICATION | MB_ICONINFORMATION);
    
	while (1)
    {
        Beep(1000, 200);
        
		Sleep(1000);
        
		if ( !g_isRunning )
        {
            break;
        }
    }*/

	// set current directory
	TCHAR szPath[MAX_PATH];
	GetModuleFileName( NULL, szPath, MAX_PATH );
	TCHAR drive[MAX_PATH],dir[MAX_PATH],fname[MAX_PATH],ext[MAX_PATH];
	_tsplitpath( szPath,drive,dir,fname,ext );
	strcpy( szPath, drive );
	strcat( szPath, dir );
	SetCurrentDirectory( szPath );

	//Clog( LOG_DEBUG, "SetCurrentDirectory: %s", szPath );

	// DeviceInfo
	//g_pDeviceInfo = new CSCDeviceInfo(szPath);

	// DSA
	//g_pFileWatcher = new CFileWatcher(szPath);
	g_pDeviceAgent = new CDeviceAgent();

	//CDeviceInfoParser * tmpParser = new CDeviceInfoParser;
	//tmpParser->test( 1 );
	//Clog( 3, "Call Parser test 1 success" );
	//tmpParser->test( 2 );
	//Clog( 3, "Call Parser test 2 success" );
	//tmpParser->test( 3 );
	//Clog( 3, "Call Parser test 3 success" );
	//tmpParser->test( 4 );
	//Clog( 3, "Call Parser test 4 success" );

	while (1)
    {        
		Sleep(2000);
        
		if ( !g_isRunning )
        {
            break;
        }
    }
	
    //MSG msg;
    //while (GetMessage(&msg, 0, 0, 0))
        //DispatchMessage(&msg);
	/*
	if( g_pDeviceInfo )
	{
		delete g_pDeviceInfo;
		g_pDeviceInfo = NULL;
	}*/
	if( g_pDeviceAgent )
	{
		delete g_pDeviceAgent;
		g_pDeviceAgent = NULL;
	}

	/*
	if( tmpParser )
	{
		delete tmpParser;
		tmpParser = NULL;
	}*/
	
    ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
    
	// Send current status. 
    if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)) 
    { 
        sprintf(g_Msg,"DSA! SetServiceStatus error %ld\n", GetLastError() ); 
        WriteInLogFile( g_Msg );
    } 
 
    return; 
} 
 
//// Stub initialization function. 
//DWORD ServiceInitialization( DWORD argc, LPTSTR *argv, DWORD *specificError ) 
//{ 
//    return(NO_ERROR); 
//} 

VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{ 
    sprintf(g_Msg, "In to service control...\n");
    WriteInLogFile(g_Msg);

    switch(dwControl) 
    { 
        case SERVICE_CONTROL_PAUSE: 
            ServiceStatus.dwCurrentState = SERVICE_PAUSED; 
            break; 
 
        case SERVICE_CONTROL_CONTINUE: 
            ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
            break; 
 
        case SERVICE_CONTROL_STOP: 
            sprintf(g_Msg, "Stopping...\n");
            WriteInLogFile(g_Msg);
            
			ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
            ServiceStatus.dwCheckPoint    = 0; 
            ServiceStatus.dwWaitHint      = 0; 
 
            g_isRunning = false;

            if ( !SetServiceStatus( ServiceStatusHandle, &ServiceStatus) )
            { 
                sprintf(g_Msg,"DSA! SetServiceStatus() error %ld\n",GetLastError() ); 
                WriteInLogFile(g_Msg);
            } 

            sprintf(g_Msg, "DSA! leaving handler \n", 0 ); 
            WriteInLogFile(g_Msg);
            
			return; 
 
        case SERVICE_CONTROL_INTERROGATE: 
            break; 
 
        default: 
            sprintf(g_Msg, "DSA unrecognized control code %ld\n", dwControl ); 
            WriteInLogFile(g_Msg);
    } 
 
    // Send current status. 
    if ( !SetServiceStatus(ServiceStatusHandle, &ServiceStatus) ) 
    { 
        sprintf(g_Msg,"DSA! SetServiceStatus() error %ld\n", GetLastError() ); 
        WriteInLogFile( g_Msg );
    } 

    return; 
} 

#include <conio.h>

void application()
{
    g_isRunning = true;

    // sprintf( g_Msg, "Just before processing Loop...\n" );
    // WriteInLogFile(g_Msg);

	printf("starting DSA service...\n");

	// set current directory
	TCHAR szPath[MAX_PATH];
	GetModuleFileName( NULL, szPath, MAX_PATH );
	TCHAR drive[MAX_PATH],dir[MAX_PATH],fname[MAX_PATH],ext[MAX_PATH];
	_tsplitpath( szPath,drive,dir,fname,ext );
	strcpy( szPath, drive );
	strcat( szPath, dir );
	SetCurrentDirectory( szPath );

	g_pDeviceAgent = new CDeviceAgent();
	if (g_pDeviceAgent == NULL) {
		printf("out of memory\n");
		return;
	}
	
	printf("press q to exit...\n");
	while (1) {   
		int n = getche();
		if( n == 'q' || n == 'Q')
			break;
    }
	
	delete g_pDeviceAgent;
	g_pDeviceAgent = NULL;
}

