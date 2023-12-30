/***************************************************************************
**
** Copyright (c) 2006 by
** ZQ Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
** 
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of zq  Technology  Inc.   Possession, use,
** duplication or dissemination of the software and media is authorized only
** pursuant to a valid written license from zq Technology Inc.
** 
** This software is furnished under a  license  and  may  be used and copied
** only in accordance with the terms of  such license and with the inclusion
** of the above copyright notice.  This software or any other copies thereof
** may not be provided or otherwise made available to  any other person.  No
** title to and ownership of the software is hereby transferred.
**
** The information in this software is subject to change without  notice and
** should not be construed as a commitment by zq Technology Inc.
** 
** ZQ  assumes  no  responsibility  for the use or reliability of its
** software on equipment which is not supplied by zq.
** 
***************************************************************************/

/***************************************************************************
**
** TITLE:
**
**      Application Shell
**
**
** VERSION:
**
**      1.0-0 001
**
**
** FACILITY
**
**      CDCI Services
**
**
** ABSTRACT:
**
**      This module contains the Application Shell.  This shell provides an
**      execution environment for CDCI service applications.  This shell
**      communicates with the Service Shell process on behalf of the
**      application. The Application Shell receives and processes service
**      start/stop requests and maintains an alive protocol with the Service
**      Shell.
**
**      There are three events/handles of interest to this shell:  the
**      SCMgrSTOP handle, the APPALIVE handle, and the APPTHREAD handle.
**      The SCMgrSTOP event is set when the service manager or some other
**      service control program issues a stop request to the Service Shell.
**      (The application may also set this event to force the service to
**      exit without restarting).  The APPALIVE event is initially set by
**      the application's main logic to inform the Service Shell that the
**      application is alive.  The Application Shell assumes the task of
**      periodically setting this event thereafter.  The APPTHREAD event
**      is set when the application's main thread terminates.  Normally
**      this event should never be set.  However, it may be set if the
**      application's main thread causes an exception or voluntarily
**      exits.  In most cases, the signaling of this event should cause
**      the logging of an event to the specified event sink and a restart
**      of the service application or a reboot of the system.
**
**
** REVISION HISTORY: (Most recent at top)
**
**   Revision      Date     Modified By     Reviewed By
**  ----------  ---------   -----------     -----------
**   V1.0.0      10-21-2006   dony            
**  
**  Initial Fieldtest release 
**
***************************************************************************/
/*
*  Includes
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winnt.h>
#include <process.h>    //X2.2-0 001
#include <excpt.h>
#include <tchar.h>
#include <crtdbg.h>
#include <time.h>


#include "ZQ.h"
#include "zqcfgpkg.h"
//#include "manpkg.h" //modify by dony remark all about the manpkg.lib's api 
#include "ZqMessages.h"

#define  _APPSHELL_C     //X2.2-0 001
#include "zqappshell.h"   //X2.2-0 001
#include <getopt.h>

static DWORD   gdwDisableExceptionOnExit = FALSE;   // Enables or disables the Raise Expection SPR# 2716


#pragma comment(lib, "advapi32.lib")

/*
*  Type Definitions (X2.2-0 001)
*/
typedef struct _THREAD_BINDING
{
    int   (*thread_main)( void* );
    int   (*thread_filter)( void*, EXCEPTION_POINTERS* );
    int   (*thread_except)( void*, EXCEPTION_POINTERS* );
    int   (*thread_finally)( void* );
    void  *arglist;
} THREAD_BINDING;

//////////////////////////////////////////////////////////////
// Globally-available names... 
//
#ifdef __cplusplus
extern "C"
{
#endif

//modify by dony 20061022 the remarked code is the seachange code

TCHAR        servname[MAX_APPSHELL_NAME_LENGTH + 1];
TCHAR        prodname[MAX_APPSHELL_NAME_LENGTH + 1];
TCHAR        sysname[MAX_COMPUTERNAME_LENGTH + 3] = _T("\\\\");
// eventsink is unused in the new appshell.
TCHAR        eventsink[MAX_COMPUTERNAME_LENGTH + 3] = _T("");

DWORD		backup_warning_time;								// Added 02/01/00 RPD. SPR #947

HANDLE      HAppPaused, HAppContinued;  //X2.2-0 001
////////////////////////////////////////////////////////////////

DWORD       fac_mask = ZQShell << 16;  // used by svcsubr.cpp

#ifdef __cplusplus
}
#endif

//struct _cmd_params {
//    DWORD   argc;
//    LPSTR  *argv;
//} cmd_params;

int app_argc = 0;
char* app_argv[] = {NULL, NULL, NULL, NULL};

/* Buffer for building message strings */

TCHAR        buf[256];

BOOL halted = FALSE;
BOOL bIsServiceMode = FALSE;	// T - Service mode; F - Interactive

/*  Forward declarations */

static int	 backup_warning_thread(void*);
static int   app_thread( void* );
static int   monitor_thread(void *);
static int   app_svc_ctrl( void *);
static void  ThreadEntry( THREAD_BINDING* );
static int   AppFilter( THREAD_BINDING*, EXCEPTION_POINTERS*, EXCEPTION_POINTERS* );
static void  CrashProcess( EXCEPTION_POINTERS* );
static void  usage();
//int         get_exception_data(EXCEPTION_POINTERS   *ptr, char *buffer);
//X2.2-0 001 end
extern VOID  LogServiceEvent(DWORD eventID,
                    WORD strcnt,
                    TCHAR **strarray,
                    DWORD bdatacnt,
                    LPVOID bdata,
                    TCHAR *servname,
                    DWORD fac_mask,
                    TCHAR *eventsink,
                    TCHAR *sysname);

// These app_* functions are defined by the user's of appshell
#ifdef __cplusplus
extern "C" {
#endif
extern void  app_main( int, char** );
extern void  app_service_control( int );
#ifdef __cplusplus
}
#endif

extern void  LogServiceException( DWORD, EXCEPTION_POINTERS* );
extern int   CopyExceptionInformation( EXCEPTION_POINTERS*, EXCEPTION_POINTERS* );
extern void  FreeExceptionInformation( EXCEPTION_POINTERS* );
extern int	 ReportHook( int reportType, char *userMessage, int *retVal );
// X2.2-0 001 end

/*  main() -- main routine 
**
**	The main thread is responsible for providing a management 
**	interfaces for the process.
**	The thread creates the application service thread, and
**	services manangement requests to STOP, CONTINUE, and PAUSE
**  the application.
*/
//add by dony  add to hold out the char style seachange hold out the unicode style
int main(int argc, char *argv[])
{
// for the test
#ifdef _TESTALIVE
	int iCount = 0;
#endif
// for the test

	unsigned int       TID = 0;
	unsigned int       iMTId;
	HANDLE		HandleMonitorThread;
	HANDLE		HandleWarningThread;		// Added 02/01/99 RPD
	LPTSTR  	msg[3];
	DWORD   	WaitReason = 0;
	// BOOL        halted = FALSE;
	HANDLE  	hThread;
	HANDLE  	hcfg;
	EXCEPTION_POINTERS  XPtr; //X2.2-0 001

	DWORD       cfg_value, cfg_size, cfg_type;
	TCHAR		cfg_string[256];
	DWORD       dwErr = ERROR_SUCCESS;
//	MANSTATUS   manRet;  //modify by dony remark all about the manpkg.lib's api 
//	HANDLE      hManSession=NULL; //modify by dony remark all about the manpkg.lib's api 

	DWORD       startup_wait = STARTUP_WAIT;        //DHW 7-25-94
	DWORD       shutdown_wait = SHUTDOWN_WAIT;
	int         iRet;

	
	time_t     start_time, end_time; // add by dony for the test
	double     elapsed_time;

	  _try                              //X2.2-0 001
	  {                                 //X2.2-0 001
		XPtr.ExceptionRecord = NULL;    //X2.2-0 001
		XPtr.ContextRecord = NULL;      //X2.2-0 001

		// If invoke from cmd shell, then cmd line: <image> -s <service-name> <product-name>
		// For example, ids.exe -s "Interactive Directory Service" "ITV"
        app_argc = 1;
        app_argv[0]  ="0";

        int ch;
        while((ch = getopt(argc, argv, "hvms:p:i:n:")) != EOF)
        {
            switch (ch)
            {
            case '?':
            case 'h':
                usage();
                return (0);

            case 'v':
                printf("%s\n", ITV_VERSION_FILEVERSION);
                return (0);

            case 'm':
                bIsServiceMode = true;
                break;

            case 's':
                if (!optarg)
                {
                    usage();
                    return 0;
                }else{
                    memcpy(servname,optarg,strlen(optarg));
                    servname[strlen(optarg)]='\0';
                }
                break;

            case 'p':
                if (!optarg)
                {
                   usage();
                    return 0;
                }else{
                    memcpy(prodname,optarg,strlen(optarg));
                    prodname[strlen(optarg)]='\0';
                }
                break; 

            case 'i':
                app_argv[0] = optarg ? optarg : (char*)"0";
                break;

            case 'n':
                if (optarg)
                {
                    app_argv[1]  = optarg;
                    app_argc =2;
                }
                break;
            }
        }

        if (strlen(servname) == 0 || strlen(prodname) == 0)
        {
            usage();
            return 0;
        }


	//	if ((argc >=4) && (stricmp(argv[1], "-s" ) == 0)) {
	//		bIsServiceMode = FALSE;
	//		
	//#if defined _UNICODE || defined UNICODE
	//		MultiByteToWideChar(
	//			CP_ACP,         // code page
	//			0,              // character-type options
	//			argv[2],        // address of string to map
	//			strlen (argv[2]),      // number of bytes in string
	//			servname,       // address of wide-character buffer
	//			MAX_APPSHELL_NAME_LENGTH);             // size of buffer);
	//		MultiByteToWideChar(
	//			CP_ACP,         // code page
	//			0,              // character-type options
	//			argv[3],        // address of string to map
	//			strlen (argv[3]),      // number of bytes in string
	//			prodname,       // address of wide-character buffer
	//			MAX_APPSHELL_NAME_LENGTH);             // size of buffer);
	//#else
	//		memcpy(servname,argv[2],strlen(argv[2]));
	//		servname[strlen(argv[2])]='\0';
	//	
	//		memcpy(prodname,argv[3],strlen(argv[3]));
	//		prodname[strlen(argv[3])]='\0';
	//#endif

 //           // hide the base argument
 //           cmd_params.argc -= 4;
 //           cmd_params.argv += 4;

	//	// Else invoked from Service Control Manager cmdline: <service-name>
	//	}
	//	else if ((argc >= 2) && (stricmp(argv[1], "-s") != 0) )
	//	{
	//		bIsServiceMode = TRUE;
	//		//
	//		// Note: Should check argc to be sure prodname included.
	//		//
	//#if defined _UNICODE || defined UNICODE
	//		MultiByteToWideChar(
	//			CP_ACP,         // code page
	//			0,              // character-type options
	//			argv[0],        // address of string to map
	//			strlen (argv[0]),      // number of bytes in string
	//			servname,       // address of wide-character buffer
	//			MAX_APPSHELL_NAME_LENGTH);             // size of buffer);
	//		MultiByteToWideChar(
	//			CP_ACP,         // code page
	//			0,              // character-type options
	//			argv[1],        // address of string to map
	//			strlen (argv[1]),      // number of bytes in string
	//			prodname,       // address of wide-character buffer
	//			MAX_APPSHELL_NAME_LENGTH);             // size of buffer);
	//#else
	//		memcpy(servname,argv[0],strlen(argv[0]));
	//		servname[strlen(argv[0])]='\0';
	//	
	//		memcpy(prodname,argv[1],strlen(argv[1]));
	//		prodname[strlen(argv[1])]='\0';
	//#endif

 //           // hide the base argument
 //           cmd_params.argc -= 2;
 //           cmd_params.argv += 2;
	//	}
	//	else
	//	{
	//		printf("\nInvalid number of parameters on command line!");
	//		printf("\nYou must use -s <svc_name> <product_name>!\nExiting...");
	//		OutputDebugString(_T("\nInvalid number of parameters on command line!") );
	//		OutputDebugString(_T("\nYou must use -s <svc_name> <product_name>!\nExiting...") );

	//		goto exit_main;
	//	}

	/*
	 * Create the event objects.
	 */

	// Added 10/5/94, T1.0-03, MMS
	// Create a named event that will be used by the srvshell to determine whether this
	// image is still running. We don't need a handle to this event once created, so
	// simply create the event using the first handle_array entry to temporarily store
	// the handle. We never close or reference this event after creation, so the handle
	// is not needed within this image.

		_stprintf(buf, _T("%s_ALREADY_UP"), servname);
		handle_array[0] = CreateEvent(NULL,TRUE,FALSE, buf);
	  if (handle_array[0] == (HANDLE)NULL) {
			_stprintf((TCHAR *)&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
			msg[0] = _T("create");
			msg[1] = buf;
			msg[2] = (TCHAR *)&buf[MAXOBJNAMELEN + 8];
			LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
											servname, fac_mask, eventsink, sysname);
		goto exit_main;
		}
	// End 10/5 mod


		_stprintf(buf, _T("%s_Stop"), servname);
		if (!bIsServiceMode)
    		handle_array[ SCMgrSTOP ] = CreateEvent( NULL, TRUE, FALSE, buf );
		else 
			handle_array[SCMgrSTOP] = OpenEvent(
						EVENT_ALL_ACCESS, 
						FALSE,                      // not inheritable
						buf);                       // name

	  if (handle_array[SCMgrSTOP] == (HANDLE)NULL) 
	  {
			_stprintf((TCHAR *)&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
			msg[0] = (bIsServiceMode) ? _T("open") : _T("create");
			msg[1] = buf;
			msg[2] = (TCHAR *)&buf[MAXOBJNAMELEN + 8];
			LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
											servname, fac_mask, eventsink, sysname);
			goto exit_main;
		}
    
		_stprintf(buf, _T("%s_Alive"), servname);
		if (!bIsServiceMode)
    		handle_array[ APPALIVE ] = CreateEvent( NULL, TRUE, FALSE, buf );
		else
			handle_array[APPALIVE] = OpenEvent(
						EVENT_ALL_ACCESS, 
						FALSE,                      // not inheritable
						buf);                       // name

	  if (handle_array[APPALIVE] == (HANDLE)NULL) {
			_stprintf((TCHAR *)&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
			msg[0] = (bIsServiceMode) ? _T("open") : _T("create");
			msg[1] = buf;
			msg[2] = (TCHAR *)&buf[MAXOBJNAMELEN + 8];
			LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
											servname, fac_mask, eventsink, sysname);
    		goto exit_main;
		}

		// Create an event used by the app to signal to srvshell that its about to die.

		_stprintf(buf, _T("%s_Shutdown"), servname);
		if (!bIsServiceMode)
    		handle_array[ APPSHUTDOWN ] = CreateEvent( NULL, TRUE, FALSE, buf );
		else
  			handle_array[APPSHUTDOWN] = OpenEvent(
						EVENT_ALL_ACCESS, 
						FALSE,                      // not inheritable
						buf);                       // name

	  if (handle_array[APPSHUTDOWN] == (HANDLE)NULL) {
			_stprintf((TCHAR *)&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
			msg[0] = (bIsServiceMode) ? _T("open") : _T("create");
			msg[1] = buf;
			msg[2] = (TCHAR *)&buf[MAXOBJNAMELEN + 8];
			LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
											servname, fac_mask, eventsink, sysname);
			goto exit_main;
		}
	// End X2.2-0 002 Addition


	// X2.2-0 001 start
		_stprintf( buf, _T("%s_Pause"), servname );
		if (!bIsServiceMode)
    		handle_array[ SCMgrPAUSE ] = CreateEvent( NULL, FALSE, FALSE, buf );
		else
    		handle_array[ SCMgrPAUSE ] = OpenEvent( EVENT_ALL_ACCESS, FALSE, buf );

		if (handle_array[ SCMgrPAUSE ] == (HANDLE)NULL)
		{
			/*
			*  Error creating/opening the event.  Log an error message and exit.
			*/
			_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
			msg[0] = (bIsServiceMode) ? _T("open") : _T("create");
			msg[1] = buf;
			msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
			LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
								servname, fac_mask, eventsink, sysname );
			goto exit_main;
		}

    
		_stprintf( buf, _T("%s_Continue"), servname );
		if (!bIsServiceMode)
    		handle_array[ SCMgrCONTINUE ] = CreateEvent( NULL, FALSE, FALSE, buf );
		else
    		handle_array[ SCMgrCONTINUE ] = OpenEvent( EVENT_ALL_ACCESS, FALSE, buf );

		if (handle_array[ SCMgrCONTINUE ] == (HANDLE)NULL)
		{
			/*
			*  Error creating/opening the event.  Log an error message and exit.
			*/
			_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
			msg[0] = (bIsServiceMode) ? _T("open") : _T("create");
			msg[1] = buf;
			msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
			LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
								servname, fac_mask, eventsink, sysname );
			goto exit_main;
		}


		_stprintf( buf, _T("%s_Paused"), servname );
		if (!bIsServiceMode)
    		HAppPaused = CreateEvent( NULL, FALSE, FALSE, buf );
		else
    		HAppPaused = OpenEvent( EVENT_ALL_ACCESS, FALSE, buf );

		if (HAppPaused == (HANDLE)NULL)
		{
			/*
			*  Error creating/opening the event.  Log an error message and exit.
			*/
			_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
			msg[0] = (bIsServiceMode) ? _T("open") : _T("create");
			msg[1] = buf;
			msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
			LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
								servname, fac_mask, eventsink, sysname );
			goto exit_main;
		}
    

		_stprintf( buf, _T("%s_Continued"), servname );
		if (!bIsServiceMode)
    		HAppContinued = CreateEvent( NULL, FALSE, FALSE, buf );
		else
			HAppContinued = OpenEvent( EVENT_ALL_ACCESS, FALSE, buf );

		if (HAppContinued == (HANDLE)NULL)
		{
			/*
			*  Error creating/opening the event.  Log an error message and exit.
			*/
			_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
			msg[0] = (bIsServiceMode) ? _T("open") : _T("create");
			msg[1] = buf;
			msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
			LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
								servname, fac_mask, eventsink, sysname );
			goto exit_main;
		}
	// X2.2-0 001 end
	/*
	 * Get configuration information. If it's not there, let the defaults suffice. 
     */        

		cfg_size = MAX_COMPUTERNAME_LENGTH + 1;         //X2.2-0 001
		GetComputerName((TCHAR *)&sysname[2], &cfg_size);
		_stprintf(buf, _T("%s_Shell"), servname);

		// initialize with config package, this is based on the product name and service_Shell name
		hcfg = CFG_INITEx(buf, &cfg_value, prodname);

		if (hcfg != NULL) {

			iRet = CFG_GET_VALUE(hcfg,
								 _T("EnableAsserts"),
								 (BYTE *)&cfg_value,
								 &cfg_size,
								 &cfg_type);

			// pbm 01/07/02 - If there's a non-zero value in the registry, we want
			// asserts.  Otherwise trap them and raise an exception.
			//
			if ((iRet != CFG_SUCCESS) || (cfg_type !=   REG_DWORD) || (cfg_value == 0)){
				_CrtSetReportHook(ReportHook);
			}
			
			//SPR# 2716 The expection on shutdown failures can be turned off
			gdwDisableExceptionOnExit = FALSE; //default value;
			cfg_size = sizeof(gdwDisableExceptionOnExit);
			iRet = CFG_GET_VALUE(hcfg,
								_T("DisableExceptionOnExit"),
								(BYTE *)&cfg_value,
								&cfg_size,
								&cfg_type);
			if ((iRet == CFG_SUCCESS) && (cfg_type ==   REG_DWORD) && (cfg_value != 0))
			{
				gdwDisableExceptionOnExit = TRUE;
			}
			// end SPR 2716
			

			cfg_size = sizeof(gdwAppShellAliveWait);
			iRet = CFG_GET_VALUE(hcfg,
								_T("AppAliveWait"),
								(BYTE *)&cfg_value,
								&cfg_size,
								&cfg_type);
			if ((iRet == CFG_SUCCESS) && (cfg_type ==   REG_DWORD) && (cfg_value > 9999))
				gdwAppShellAliveWait = cfg_value;

			// Added 02/01/00 RPD. SPR #947.
			// Produce NT events if service is running on its backup node.
			// -----------------------------------------------------------
			cfg_size = sizeof(backup_warning_time);
			iRet = CFG_GET_VALUE(hcfg,
								 _T("BackupWarningTime"),
								 (BYTE *)&cfg_value,
								 &cfg_size,
								 &cfg_type);
			if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
				backup_warning_time = cfg_value;
			else
				backup_warning_time = 0xFFFFFFFF;

		/*
		 * Added the following 3 statements to fetch the startup timeout value.
		 * Formerly, the AppAliveWait value was used both for startup and the alive
		 * protocol, which did not map with the service shell, which uses separate
		 * values for those two stages of app monitoring.
		 *
		 *  7-25-94 DWH
		 */
			cfg_size = sizeof(startup_wait);
			iRet = CFG_GET_VALUE(hcfg,
								_T("AppStartupWait"),
								(BYTE *)&cfg_value,
								&cfg_size,
								&cfg_type);
			if ((iRet == CFG_SUCCESS) && (cfg_type ==   REG_DWORD) && (cfg_value > 9999))
				startup_wait = cfg_value;
		/*
		 * End AppAliveWait modifications.  7-25-94 DWH
		 */

			cfg_size = sizeof(shutdown_wait);
			iRet = CFG_GET_VALUE(hcfg,
								 _T("AppShutdownWait"),
								 (BYTE *)&cfg_value,
								 &cfg_size,
								 &cfg_type);
			if ((iRet == CFG_SUCCESS) && (cfg_type ==   REG_DWORD) && (cfg_value > 2999))
				shutdown_wait = cfg_value;

			cfg_size = sizeof(DWORD);
			iRet = CFG_GET_VALUE(hcfg,
								_T("FacilityMask"),
								(BYTE *)&cfg_value,
								&cfg_size,
								&cfg_type);
			if ((iRet == CFG_SUCCESS) && (cfg_type ==   REG_DWORD))
				fac_mask = cfg_value;

			cfg_size = sizeof(eventsink) - 3;   // buffer is last n-2 characters
			if (CFG_GET_PARENT_VALUES(hcfg,
									 _T("EventSink"),
									 (BYTE*)cfg_string,
									 &cfg_size,
									 &cfg_type) == CFG_SUCCESS) {
				if ((_tcslen(cfg_string)) && (_istalnum(cfg_string[0])))
					_stprintf(eventsink, _T("\\\\%s"), cfg_string);
			}

			CFG_TERM(hcfg);

		}

	/* modify by dony remark all about the manpkg.lib's api 
	 * Set up management variables for the timeout values...
	 */
	 

	
	// for the test 20061127 for check the shunxun of varialbe 
#ifdef _TEST
	FILE *pFile;
	pFile = _tfopen(_T("C:\\ZQAppShell.log"),_T("wta+"));
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR tszFileName[128];
	_stprintf(tszFileName, _T("%s%d%02d%02d%02d:%02d:%02d:%02d\n"),_T("ZQAppShell.exe's main to set the variable curtime="),
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,st.wMilliseconds);



	_fputts(tszFileName,pFile);
	fclose(pFile);
#endif

	// for the test 20061127 for check the shunxun of varialbe 


	 /*
		// We'll close this managment session just before we go away
		_stprintf(buf, _T("%s"), servname); // not the shell, but the real service
											// the app will define the port later.
		manRet = ManOpenSession(buf, MAN_NO_MGMT_PORT, &hManSession, &dwErr);

		if (MAN_SUCCESS == manRet)
			manRet = ManManageVar(hManSession, _T("Mgmt Alive Wait"), MAN_THRESH, (DWORD)&gdwAppShellAliveWait, FALSE, &dwErr);

		
//		 Added code to make the startup wait timer management-visible.
//		  7-25-94  DWH
//		 /
		// These variables are read/write
		if(manRet == MAN_SUCCESS)
			manRet = ManManageVar(hManSession, _T("Mgmt Startup Wait"), MAN_THRESH, (DWORD)&startup_wait, FALSE, &dwErr);

		if(manRet == MAN_SUCCESS)
			manRet = ManManageVar(hManSession, _T("Mgmt Shutdown Wait"),  MAN_THRESH, (DWORD)&shutdown_wait, FALSE, &dwErr);

		if(manRet == MAN_SUCCESS)
			manRet = ManManageVar(hManSession, _T("Mgmt Backup Warning"),  MAN_THRESH, (DWORD)&backup_warning_time, FALSE, &dwErr);

		if (manRet != MAN_SUCCESS) {
			_stprintf(buf, _T("%d"), iRet);
			msg[0] = buf;
			_stprintf(&buf[16], _T("%d"), GetLastError());
			msg[1] = &buf[16];
			LogServiceEvent(APPSHELL_MANINIT_FAIL, 2, msg, 0, NULL,
											servname, fac_mask, eventsink, sysname);
		}
*/  //modify by dony remark all about the manpkg.lib's api 

	// Create thread to monitor service shell.
	// If the ShellAlive Mutex is ever acquired by the monitor thread
	// (for instance, if the shell process exits)
	// then the monitor thread will terminate the service_application
	// 
		HandleMonitorThread = StartThread (NULL, 16*1024, &monitor_thread, NULL, 0, &iMTId, NULL, NULL, NULL);
		if (HandleMonitorThread == NULL) {
			_stprintf(buf, _T("%d"), GetLastError());
			LogServiceEvent(APPSHELL_NO_MONITOR_THREAD, 1, msg, 0, NULL, servname, fac_mask, eventsink, sysname);
			goto exit_main;
		}
    
	// Added 02/01/00 RPD. Start a thread to produce NT events if service is running on backup node.
	// ---------------------------------------------------------------------------------------------
		if (backup_warning_time != 0xFFFFFFFF) {
			HandleWarningThread = StartThread(
								NULL,           // Security attributes
								0,              // Stack size
								&backup_warning_thread,    // Thread entry point
								NULL,           // Argument list
								0,              // Thread creation flags
								&TID,           // Pointer to thread ID
								NULL,           // Exception filter routine
								NULL,           // Exception routine
								NULL );         // Finally routine
			if (HandleWarningThread == NULL)
			{
				_stprintf( buf, _T("%d"), GetLastError() );
				msg[ 0 ] = buf;     
				LogServiceEvent(APPSHELL_NO_WARNING_THREAD, 1, msg, 0, NULL,
	                        		servname, fac_mask, eventsink, sysname);
				goto exit_main;
			}
		}

	/* start the thread that performs the work of the service. */
		handle_array[ APPTHREAD ] = StartThread(
								NULL,           /* Security attributes */
								0,              /* Stack size */
								&app_thread,    /* Thread entry point */
								NULL,           /* Argument list */
								0,              /* Thread creation flags */
								&TID,           /* Pointer to thread ID */
								NULL,           /* Exception filter routine */
								NULL,           /* Exception routine */
								NULL );         /* Finally routine */
		if (handle_array[ APPTHREAD ] == NULL)
		{
			_stprintf( buf, _T("%d"), GetLastError() );
			msg[ 0 ] = buf;     
			LogServiceEvent(APPSHELL_NO_MAIN_THREAD, 1, msg, 0, NULL,
                        		servname, fac_mask, eventsink, sysname);
			goto exit_main;
		}
	// X2.2-0 001 end

	/*
	 * Wait 30 seconds until handle_array[SCMgrSTOP] is signaled or the real app
	 * thread exits sets ALIVE. If we time out, assume the main thread hosed up
	 * before it had a chance either to set ALIVE or to exit cleanly.
	 */

	//++  16-JUN-1995
	//  added code to not to include the keepalive event (last event)
	//  when the application wants to set the keepalive event.
	//--
	  WaitReason = WaitForMultipleObjectsEx(
					APPSHLNUMHANDLES,         // Number of handles to wait on
					handle_array,             // Array of objects to wait on
					FALSE,                    // Only wait for one handle
					startup_wait,             // wait startup_wait sec - 7-25-94 DWH
					FALSE);                   // Not alertable

		// Dispatch on Wait result
		switch (WaitReason) 
		{

		/*
		 * If we see STOP here, either the app decided to kill us all before even
		 * initing, or the SCMgr did.
		 */

			// X2.2-0 001 start
			case WAIT_OBJECT_0 + SCMgrSTOP:
			case WAIT_OBJECT_0 + SCMgrPAUSE:
			case WAIT_OBJECT_0 + SCMgrCONTINUE:
			{
				// LogNTEvent to trace service requests
				if (WaitReason == WAIT_OBJECT_0 + SCMgrSTOP) msg[0] = _T("Stop");
				else if (WaitReason == WAIT_OBJECT_0 + SCMgrPAUSE) msg[0] = _T("Pause");
				else  msg[0] = _T("Continue");
				LogServiceEvent(APPSHELL_SERVICE_REQUEST, 1, msg, 0, NULL,
									servname, fac_mask, eventsink, sysname);

				/*
				*  Call the application's service control routine with the
				*  service control code.  Use a separate thread to prevent
				*  the application from hosing this thread.  We don't care
				*  about this thread after we create it (other than making
				*  sure it was created).
				*/
				hThread = StartThread(
							NULL,           /* Security attributes */
							0,              /* Stack size */
							&app_svc_ctrl,
											/* Thread entry point */
							(void *)(WaitReason - WAIT_OBJECT_0),
											/* Argument list */
							0,              /* Thread creation flags */
							&TID,           /* Pointer to thread ID */
							NULL,           /* Exception filter routine */
							NULL,           /* Exception routine */
							NULL );         /* Finally routine */
				if (hThread == NULL) {
					_stprintf( buf, _T("%d"), GetLastError() );
					msg[ 0 ] = buf;
					LogServiceEvent(APPSHELL_NO_CONTROL_THREAD, 1, msg, 0, NULL,
                            			servname, fac_mask, eventsink, sysname);
				} else 
				  CloseHandle(hThread);

            
					/*
					*  If this was a stop request, then
					*  Wait up to "shutdown_wait" milliseconds for main thread to terminate
					*/
					if (WaitReason == (WAIT_OBJECT_0 + SCMgrSTOP)) 
					{
						DWORD dwAppThreadWait = WaitForSingleObject(handle_array[APPTHREAD], shutdown_wait);
						//SPR# 2716 call raise exception if the main thread does not shutdown in time.
						if((dwAppThreadWait == WAIT_TIMEOUT) && (gdwDisableExceptionOnExit == FALSE))
						{
							// Check weather AppAlive is set.
							DWORD dwAppAliveCheck = WaitForSingleObject(handle_array[APPALIVE], 1);
							if((WAIT_TIMEOUT == dwAppAliveCheck)&& (!gbAppShellSetKeepAlive))/// The AppAlive is not set.
							{
								RaiseException( STATUS_APPSHELL_SHUTDOWN_TIMEOUT, 0, 0, 0);
							}
									
						}
						CloseHandle(handle_array[APPTHREAD]);
					  halted = TRUE;
					}

				break;
			}
			// X2.2-0 001 end

	// Added X2.2-0 002, 5/3/95, MMS

		/*
		 * If APPSHUTDOWN is set, then the application is going to exit.
		 */

			case WAIT_OBJECT_0 + APPSHUTDOWN:
					  WaitForSingleObject(handle_array[APPTHREAD], shutdown_wait);
					  CloseHandle(handle_array[APPTHREAD]);
				halted = TRUE;
				break;
	// End X2.2-0 002 add


		/*
		 * This is the expected case - the app is inited and we go into the main loop.
		 */

			case WAIT_OBJECT_0 + APPALIVE:
					break;

		// X2.2-0 001 start
		/*
		*  The main application thread terminated.  log a message and give up.
		*/
			case WAIT_OBJECT_0 + APPTHREAD:
			{
				/*
				*  Log a service event.
				*/
				LogServiceEvent(APPSHELL_DEAD_APPTHREAD, 0, NULL, 0, NULL,
								servname, fac_mask, eventsink, sysname);
				halted = TRUE;
				break;
			}
		// X2.2-0 001 end

		/*
		 * The app never set ALIVE - log a message and give up.
		 */
			case WAIT_TIMEOUT:
					_stprintf(buf, _T("%ld"), startup_wait);
					msg[0] = buf;
					LogServiceEvent(APPSHELL_ALIVE_TIMEOUT, 1, msg, 0, NULL,
													servname, fac_mask, eventsink, sysname);
					halted = TRUE;
					break;

		/*
		 * Some funky completion code came in. Bomb out.
		 */
			default:
					//++    16-JUN-1995
					//  Added GetLastError() in the log message
					//--
					_stprintf(buf, _T("%d, %d"), WaitReason, GetLastError());
					msg[0] = buf;                                   
					LogServiceEvent(APPSHELL_UNKNOWN_WAIT_RESULT, 1, msg, 0, NULL,
													servname, fac_mask, eventsink, sysname);
					halted = TRUE;
					break;
		}

	// #endif  X2.2-0 001

		// Now loop until we're told to stop.

		 	

		while(!halted) 
		{

	// X2.2-0 001 start
		/*
		*  Wait for objects in handle array.  NOTE: This code assumes that the
		*  APPALIVE event is the last handle in the handle array.
		*/

		time(&start_time); // for the test

		WaitReason = WaitForMultipleObjectsEx(
					(APPSHLNUMHANDLES - 1), /* Number of handles to wait on */
					handle_array,           /* Array of objects to wait on */
					FALSE,                  /* Only wait for one handle */
					gdwAppShellAliveWait,             /* Wait for alive timeout seconds */
					FALSE);                 /* Not alertable */
	// X2.2-0 001 end
		time(&end_time);
		elapsed_time = difftime(end_time, start_time); // for the test

		// Figure out which kind of completion happened..

			switch (WaitReason)
			{

				// X2.2-0 001 start
				/*
				*  STOP, PAUSE, or CONTINUE signaled.
				*/
				case WAIT_OBJECT_0 + SCMgrSTOP:
				case WAIT_OBJECT_0 + SCMgrPAUSE:
				case WAIT_OBJECT_0 + SCMgrCONTINUE:
				{

					// LogNTEvent to trace service requests
					if (WaitReason == WAIT_OBJECT_0 + SCMgrSTOP) msg[0] = _T("Stop");
					else if (WaitReason == WAIT_OBJECT_0 + SCMgrPAUSE) msg[0] = _T("Pause");
					else  msg[0] = _T("Continue");
					LogServiceEvent(APPSHELL_SERVICE_REQUEST, 1, msg, 0, NULL,
													servname, fac_mask, eventsink, sysname);

					/*
					*  Call the application's service control routine with the
					*  service control code.  Use a separate thread to prevent
					*  the application from hosing this thread.  We don't care
					*  about this thread after we create it (other than making
					*  sure it was created).
					*/
					hThread = StartThread(
								NULL,           /* Security attributes */
								0,              /* Stack size */
								&app_svc_ctrl,
												/* Thread entry point */
								(void *)(WaitReason - WAIT_OBJECT_0),
												/* Argument list */
								0,              /* Thread creation flags */
								&TID,           /* Pointer to thread ID */
								NULL,           /* Exception filter routine */
								NULL,           /* Exception routine */
								NULL );         /* Finally routine */
					if (hThread == NULL) {
						_stprintf( buf, _T("%d"), GetLastError() );
						msg[ 0 ] = buf;
						LogServiceEvent(APPSHELL_NO_CONTROL_THREAD, 1, msg, 0, NULL,
	                            			servname, fac_mask, eventsink, sysname);
					} else
								CloseHandle(hThread); 

            
					/*
					*  If this was a stop request, then
					*  Wait up to "shutdown_wait" milliseconds for main thread to terminate
					*/
					if (WaitReason == (WAIT_OBJECT_0 + SCMgrSTOP)) 
					{
						DWORD dwAppThreadWait = WaitForSingleObject(handle_array[APPTHREAD], shutdown_wait);
						//SPR# 2716 call raise exception if the main thread does not shutdown in time.
						if((dwAppThreadWait == WAIT_TIMEOUT) && (gdwDisableExceptionOnExit == FALSE))
						{
						
					  		// Check weather AppAlive is set. SPR# 2790
							DWORD dwAppAliveCheck = WaitForSingleObject(handle_array[APPALIVE], 1);
							if((WAIT_TIMEOUT == dwAppAliveCheck) && (!gbAppShellSetKeepAlive))/// The AppAlive is not set.
							{
								RaiseException( STATUS_APPSHELL_SHUTDOWN_TIMEOUT, 0, 0, 0);
							}
							
									
						}
        					
        				CloseHandle(handle_array[APPTHREAD]);
						halted = TRUE;
					}

					break;
				}

	// Added X2.2-00 002, 5/3/95, MMS

			/*
			*  The app intends to ExitProcess
			*/
				case WAIT_OBJECT_0 + APPSHUTDOWN:
        					WaitForSingleObject(handle_array[APPTHREAD], shutdown_wait);
        					CloseHandle(handle_array[APPTHREAD]);
					halted = TRUE;
					break;
	// End X2.2-00 002 add


			/*
			*  The main application thread terminated. log a message and give up.
			*/
				case WAIT_OBJECT_0 + APPTHREAD:
				{
					LogServiceEvent(APPSHELL_DEAD_APPTHREAD, 0, NULL, 0, NULL,
									servname, fac_mask, eventsink, sysname);
					halted = TRUE;
					break;
				}
			// X2.2-0 001 end

			/* Time to toggle the ALIVE event for the shell. */

				case WAIT_TIMEOUT:


	//++  16-JUN-1995
	//  added code to check if the appshell thread should set the keepalive event
	//--

					if (gbAppShellSetKeepAlive)     // if the application shell wants to set event
					{
						// for the test
						#ifdef _TESTALIVE
							iCount ++;
							if ( iCount < 5) 
							{
								SetEvent(handle_array[APPALIVE]);
							}
						#else
						   SetEvent(handle_array[APPALIVE]); // for the test there are not heart 
						#endif
						// for the test
						   //SetEvent(handle_array[APPALIVE]); // for the test there are not heart,this is the old code
					}
					break;

				default:
					//++    16-JUN-1995
					//  Added GetLastError() in the log message
					//--
					_stprintf(buf, _T("%d, %d"), WaitReason, GetLastError());
					msg[0] = buf;                                   
					LogServiceEvent(APPSHELL_UNKNOWN_WAIT_RESULT, 1, msg, 0, NULL,
													servname, fac_mask, eventsink, sysname);
					halted = TRUE;
					break;

			}   
		}


	exit_main:

		//modify by dony remark all about the manpkg.lib's api 
		// close the Management Package Handle
//		if (NULL != hManSession)
//			ManCloseSession(hManSession, &dwErr);

		if (handle_array[SCMgrSTOP] != NULL)
			if(!CloseHandle(handle_array[SCMgrSTOP])) {
				_stprintf(buf, _T("%s_Stop"), servname);
				_stprintf((TCHAR *)&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
				msg[0] = _T("close");
				msg[1] = buf;
				msg[2] = (TCHAR *)&buf[MAXOBJNAMELEN + 8];
				LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
												servname, fac_mask, eventsink, sysname);
			}

		// X2.2-0 001 start
		if (handle_array[ SCMgrPAUSE ] != NULL)
			if (!CloseHandle( handle_array[ SCMgrPAUSE ] ))
			{
				/*
				*  Couldn't close the handle.  Log an error.
				*/
				_stprintf( buf, _T("%s_Pause"), servname );
				_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
				msg[0] = _T("close");
				msg[1] = buf;
				msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
				LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
									servname, fac_mask, eventsink, sysname );
			}

		if (handle_array[ SCMgrCONTINUE ] != NULL)
			if (!CloseHandle( handle_array[ SCMgrCONTINUE ] ))
			{
				/*
				*  Couldn't close the handle.  Log an error.
				*/
				_stprintf( buf, _T("%s_Continue"), servname );
				_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
				msg[0] = _T("close");
				msg[1] = buf;
				msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
				LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
									servname, fac_mask, eventsink, sysname);
			}

		if (HAppPaused != NULL)
			if (!CloseHandle( HAppPaused ))
			{
				/*
				*  Couldn't close the handle.  Log an error.
				*/
				_stprintf( buf, _T("%s_Paused"), servname );
				_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
				msg[0] = _T("close");
				msg[1] = buf;
				msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
				LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
									servname, fac_mask, eventsink, sysname);
			}

		if (HAppContinued != NULL)
			if (!CloseHandle( HAppContinued ))
			{
				/*
				*  Couldn't close the handle.  Log an error.
				*/
				_stprintf( buf, _T("%s_Continued"), servname );
				_stprintf( &buf[ MAXOBJNAMELEN + 8 ], _T("%d"), GetLastError() );
				msg[0] = _T("close");
				msg[1] = buf;
				msg[2] = &buf[ MAXOBJNAMELEN + 8 ];
				LogServiceEvent( APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
									servname, fac_mask, eventsink, sysname);
			}
		// X2.2-0 001 end

	// Added X2.2-00 002, 5/3/95, MMS
		if (handle_array[APPSHUTDOWN] != NULL)
			if(!CloseHandle(handle_array[APPSHUTDOWN])) {
				_stprintf(buf, _T("%s_Shutdown"), servname);
				_stprintf((TCHAR *)&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
				msg[0] = _T("close");
				msg[1] = buf;
				msg[2] = (TCHAR *)&buf[MAXOBJNAMELEN + 8];
				LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
									servname, fac_mask, eventsink, sysname);
			}
	// End X2.2-00 002 add

		if (handle_array[APPALIVE] != NULL)
			if(!CloseHandle(handle_array[APPALIVE])) {
				_stprintf(buf, _T("%s_Alive"), servname);
				_stprintf((TCHAR *)&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
				msg[0] = _T("close");
				msg[1] = buf;
				msg[2] = (TCHAR *)&buf[MAXOBJNAMELEN + 8];
				LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
												servname, fac_mask, eventsink, sysname);
			}

		
		return (SHELL_EXITTING_NEED_RESTART);
	  }
	  _except( CopyExceptionInformation( &XPtr, GetExceptionInformation() ) ) //X2.2-0 001
	  {
		/*
		*  An exception occurred in the Application Shell's main thread.
		*  Crash the process.
		*/
		CrashProcess( &XPtr );
	  }
}

void  usage()
{
    printf("Invalid parameters!\n\n");
    printf("SERVICE [-s <service>] [-p product] [-i instance_id] [-n net_id] [-m]\n\n");
    printf("-s <service>  service name\n");
    printf("-p <product>  product name\n");
    printf("-i <instance> instance id\n");
    printf("-n <netid>    net id\n");
    printf("-m            service mode\n\n");

    // for debug
    OutputDebugString(_T("Invalid parameters!\n\n"));
    OutputDebugString(_T("SERVICE [-s <service>] [-p product] [-i instance_id] [-n net_id] [-m]\n\n"));
    OutputDebugString(_T("-s <service>  service name\n"));
    OutputDebugString(_T("-p <product>  product name\n"));
    OutputDebugString(_T("-i <instance> instance id\n"));
    OutputDebugString(_T("-n <netid>    net id\n"));
    OutputDebugString(_T("-m            service mode\n\n"));
}

// X2.2-0 001 start
//  app_thread() --
//              this function is the base of the thread which will
//              do the real work of the service. This thread simply
//              unpacks the command line params from the structure
//              pointed by the thread argument, and calls the app_main
//              function as if it were a standard main() entry point.
//

static int app_thread( void *arg )
{
    app_main( app_argc, app_argv );
    return( ERROR );
}

static int app_svc_ctrl( void* arg)
{
    app_service_control((int)arg);
    return ERROR;
}

///////////////////////////////////////////////////////////////////////
//	backup_warning_thread
//
//	This service is running on its backup node. Post an NT warning event
//	every "backup_warning_time" minutes.
//
int backup_warning_thread(void*) 
{
	LPTSTR	msg[1];
	msg[0] = servname;

	LogServiceEvent(APPSHELL_TO_BACKUP, 1, msg, 0, NULL, servname, fac_mask, eventsink, sysname );
	while (TRUE)
	{
		if (backup_warning_time !=0)
		{
			Sleep(backup_warning_time*60*1000);
			LogServiceEvent(APPSHELL_ON_BACKUP, 1, msg, 0, NULL, servname, fac_mask, eventsink, sysname );
		}
		else
			Sleep(5000);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////
//  monitor_thread
//
//		This thread provides a backup (direct and indirect) method 
//		for the shell to request the application to terminate immediately.
//
//		The srvshell creates and acquires a "<service>_ShellAlive" mutex.
//		This thread waits indefinitely to acquire the mutex, and then
//			releases the mutex, and
//			immediately exits the process.
//
//		If the srvshell process "unexpectedly vanishes", 
//		then the system will release the mutex, and the app will now terminate.
//
//		It is important that whenever the srvshell process terminates
//		that the application process also terminate, because the application
//		process can not be terminated by the systems tools (pview, killproc,..).
//		The only recourse would be to reboot the box!
//
//		The srvshell may also directly release the mutex to signal the
//		appshell to terminate immediately.  This may happen when the srvshell
//		has requested the application to terminate by means of setting the
//		SCMgrSTOP event, but application has failed to terminate within the 
//		alloted time interval.
//
int monitor_thread(void *arg)
{
	TCHAR buf[256];
	HANDLE hShellAlive;
	int iSts;
	TCHAR *msg[2];

	if (bIsServiceMode) 
	{
		// Open handle to <service-name>_ShellAlive mutex
	    _stprintf( buf, _T("%s_ShellAlive"), servname );
	    hShellAlive = OpenMutex(MUTEX_ALL_ACCESS, FALSE, buf);
	    if (hShellAlive == (HANDLE)NULL) {
	        msg[0] = buf;
	        _stprintf(&buf[MAXOBJNAMELEN + 8], _T("%d"), GetLastError());
	        msg[1] = &buf[ MAXOBJNAMELEN + 8 ];
	        LogServiceEvent(APPSHELL_MUTEX_ERROR, 2, msg, 0, NULL, servname, fac_mask, eventsink, sysname );
	        ExitProcess(SHELL_EXITTING_NEED_RESTART);
		}

		// Wait for Shell to release mutex, signalling that the Shell is NO LONGER ALIVE
		iSts = WaitForSingleObject(hShellAlive, INFINITE);
		ReleaseMutex(hShellAlive);

	// Keep exit path simple...don't attempt to log event.
	//	// Log NT Event
	//  msg[0] = servname;
	//	sprintf(buf, "%d", iSts);
	//	msg[1] = buf;
	//	LogServiceEvent(APPSHELL_SHELL_NOTALIVE, 2, msg, 0, NULL, servname, fac_mask, eventsink, sysname);

		
		// ExitProcess
		ExitProcess(SHELL_EXITTING_NEED_RESTART);
	}
    return( ERROR );
}
/***************************************************************************
*
* PROCEDURE:
*
*   StartThread
*
*
* DESCRIPTION:
*
*   This routine creates a new thread using the VC++ V1.0 _beginthread()
*   procedure.  This routine binds the caller's parameters to the thread
*   and then starts the thread's execution at the ThreadEntry() routine.
*
*   Note that this routine ignores the security attributes, thread creation
*   flags, and thread ID pointer because the VC++ V1.0 RTL does not use
*   these parameters.  The VC++ V2.0 RTL provides an additional thread
*   creation routine called _beginthreadex() which will accept these
*   parameters.  This routine should be recoded to call _beginthreadex()
*   when VC++ V2.0 is available.
*
*
* ARGUMENTS:
*
*   security
*
*       The thread's security attributes.  Currently ignored.
*
*   stack_size
*
*       The thread's stack size.  A value of 0 causes the thread to
*       inherit it's parent's stack size.
*
*   thread_main
*
*       The thread's entry point.  The thread actually starts execution in
*       the ThreadEntry() routine which then calls the (*thread_main)()
*       routine.
*
*   arglist
*
*       A pointer to the argument list passed to the (*thread_main)(),
*       (*thread_filter)(), (*thread_except)(), and (*thread_finally)()
*       routines.  This pointer is added to the THREAD_BINDING structure
*       which is passed to the ThreadEntry() routine.
*
*   initflag
*
*       The thread creation flags.  Currently ignored.
*
*   thread_id
*
*       The new thread's ID.  Currently a value of 0 is always returned.
*
*   thread_filter
*
*       The address of the exception filter routine to call if and when the
*       new thread incurs an exception.  If this exception filter routine
*       continues the exception search (or if no routine is specified), the
*       Application Shell's exception filter routine will be called.
*
*   thread_except
*
*       The address of the exception routine to call if the new thread's
*       (*thread_filter)() routine indicates that the exception handler
*       should be executed.  If no routine is specified and the filter
*       indicates that the handler should be executed, the Application
*       Shell's exception handler will be invoked instead.
*
*   thread_finally
*
*       The address of the termination routine to call when the thread
*       normally (returns) or abnormally (exception) terminates.  Note
*       that this routine is NOT invoked if the _endthread() or ExitThread()
*       routines are called.  The Application Shell's termination routine
*       is also called when a thread terminates.
*
*
* RETURNS:
*
*   A handle to the new thread or NULL if the thread creation failed.
*
*
* SIDE EFFECTS:
*
*   none
*
*
***************************************************************************/

HANDLE StartThread( void      *security,
                    unsigned  stack_size,
                    int       (*thread_main)( void* ),
                    void      *arglist,
                    unsigned  initflag,
                    unsigned *thread_id,
                    int       (*thread_filter)( void*, EXCEPTION_POINTERS* ),
                    int       (*thread_except)( void*, EXCEPTION_POINTERS* ),
                    int       (*thread_finally)( void* ) )
{
	THREAD_BINDING  *thread_bind;
	HANDLE          THandle;



    /*
    *  Zero out the thread_id.  We don't provide a thread_id on return.
    */
    *thread_id = 0;


    /*
    *  Allocate a thread binding structure for the new thread.
    */
    thread_bind = (THREAD_BINDING*)malloc( sizeof( *thread_bind ) );
    if (thread_bind == NULL) return( (HANDLE)NULL );


    /*
    *  Bind the caller's parameters to the thread.
    */
    thread_bind->thread_main = thread_main;
    thread_bind->thread_filter = thread_filter;
    thread_bind->thread_except = thread_except;
    thread_bind->thread_finally = thread_finally;
    thread_bind->arglist = arglist;


    /*
    *  Create a new thread and make it start executing the ThreadEntry()
    *  routine.  Pass the thread binding structure to the new thread.
    */
    THandle = (void *)_beginthreadex(
    	security
		, stack_size
		, (unsigned (__stdcall *) (void *))ThreadEntry
		, thread_bind
		, initflag
		, thread_id
		);
	  return (THandle);
}


/***************************************************************************
*
* PROCEDURE:
*
*   ThreadEntry
*
*
* DESCRIPTION:
*
*   This routine is (or should be) the main entry point for all newly
*   created service application threads.  It sets up two levels of
*   exception handling:  one for the thread creator which is embedded
*   inside one for the Application Shell.  This layering of the exception
*   handling allows the Application Shell to catch any exceptions that
*   the creator's routines don't handle.  It also allows the creator to
*   avoid writing any exception handling code if desired.  In this case,
*   the Application Shell's exception handling routines will always be
*   invoked when an exception occurs.
*
*   Each layer of exception handling has a _try/_except embedded in a
*   _try/_finally.  When an exception occurs, the _except routine for that
*   layer will be called first followed by a call to the _finally routine
*   for that layer.
*
*
* ARGUMENTS:
*
*   TBinding
*
*       Address of the structure containing the thread information.  This
*       structure is populated with information supplied by the creator
*       (caller of the StartThread() routine).
*
*
* RETURNS:
*
*   none
*
*
* SIDE EFFECTS:
*
*   Exceptions handled by the thread creator's handler may cause side effects.
*   Exceptions handled by the Application Shell will cause the entire
*   process to exit.
*
*
***************************************************************************/

//static void ThreadEntry( THREAD_BINDING *TBinding )
//{
//	EXCEPTION_POINTERS  TXPtr, AXPtr;
//
//
//    /*
//    *  Initialize.
//    */
//    TXPtr.ExceptionRecord = NULL;
//    TXPtr.ContextRecord = NULL;
//    AXPtr.ExceptionRecord = NULL;
//    AXPtr.ContextRecord = NULL;
//
//
//    /*
//    *  Create the Application Shell's termination handler to catch the
//    *  termination of this new thread.  This handler will be invoked when
//    *  the thread terminates normally (return statement) or abnormally
//    *  (exception).  The handler will NOT be invoked if the thread is
//    *  terminated with an _endthread() or ExitThread() call.  When the
//    *  thread exits, the creator's termination handler is called before
//    *  this termination handler.
//    */
//    _try
//    {
//        /*
//        *  Create the Application Shell's exception handler to catch
//        *  exceptions generated by this thread.  The handler will only be
//        *  invoked if the creator's exception handler (embedded inside this
//        *  one) continues the exception search or if the creator didn't
//        *  provide an exception handling routine.
//        */
//        _try
//        {
//            /*
//            *  Create the thread creator's termination handler to catch the
//            *  termination of this new thread.  This handler is invoked
//            *  when the thread terminates normally (return statement) or
//            *  abnormally (exception).  The handler will NOT be invoked if
//            *  the thread is terminated with an _endthread() or ExitThread()
//            *  call.  When the thread exits, this termination handler is
//            *  called before the Application Shell's termmination handler
//            */
//            _try
//            {
//                /*
//                *  Create the thread creator's exception handler to catch
//                *  exceptions generated by this thread.  This handler can
//                *  catch the thread's exceptions or pass them on to the
//                *  Application Shell's handler based on the creator's filter
//                *  routine.
//                */
//                _try
//                {
//                    /*
//                    *  Call the thread's main entry point passing it the
//                    *  argument specified by the creator.  When an exception
//                    *  occurs, the exception handler will be invoked if the
//                    *  creator's filter/exception routines are valid (exist)
//                    *  and the filter routine indicates that the handler
//                    *  should be executed.  Otherwise, the search is passed
//                    *  on to the Application Shell's handler.
//                    */
//                    (*(TBinding->thread_main))( TBinding->arglist );
//                }
//                _except( AppFilter( TBinding, GetExceptionInformation(), &TXPtr ) )
//                {
//                    /*
//                    *  An exception occurred.  We know the caller has
//                    *  provided an exception handling routine because the
//                    *  AppFilter() routine won't let this handler execute
//                    *  if TBinding->thread_except is NULL.  Call the handler
//                    *  passing the exception information pointer.
//                    */
//                    (*(TBinding->thread_except))( TBinding->arglist, &TXPtr );
//                }
//            }
//            _finally
//            {
//                /*
//                *  The thread has terminated.  Free the exception chain
//                *  that we created if we terminated because of an exception.
//                *  If we didn't terminate because of an exception, the free
//                *  routine will just return.
//                */
//                FreeExceptionInformation( &TXPtr );
//
//
//                /*
//                *  If the caller has provided a termination handling routine,
//                *  call it.
//                */
//                if (TBinding->thread_finally != NULL)
//                    (*(TBinding->thread_finally))( TBinding->arglist );
//            }
//        }
//        _except( CopyExceptionInformation( &AXPtr, GetExceptionInformation() ) )
//        {
//            /*
//            *  An exception has occurred.  Crash the process.
//            */
//            CrashProcess( &AXPtr );
//        }
//    }
//    _finally
//    {
//        /*
//        *  The thread has terminated.  Free the exception chain
//        *  that we created if we terminated because of an exception.
//        *  If we didn't terminate because of an exception, the free
//        *  routine will just return.
//        */
//        FreeExceptionInformation( &AXPtr );
//
//
//        /*
//        *  Free the thread binding and return.
//        */
//        free( TBinding );
//    }
//}

//////////////////////////////////////////////////////////////////////////
//
//  because we want to use MiniDump to create memory dump when exception 
//  happens, so we disable the try..catch and let the MiniDump to deal with
//  it.
//
//////////////////////////////////////////////////////////////////////////

static void ThreadEntry( THREAD_BINDING *TBinding )
{
    (*(TBinding->thread_main))( TBinding->arglist );
	free( TBinding );
}



/***************************************************************************
*
* PROCEDURE:
*
*   AppFilter
*
*
* DESCRIPTION:
*
*   The Application Shell's main exception filtering procedure. It calls the
*   thread's exception filtering routine if it exists and returns the value
*   returned by the thread filtering routine.  If the thread has an exception
*   filtering routine but no exception handling routine, this procedure will
*   call the thread's filtering routine, but it will always return the value
*   EXCEPTION_CONTINUE_SEARCH to invoke the next highest layer of exception
*   handling.  Finally, if the thread has an exception handler routine but
*   does not have an exception filter routine, this procedure will always
*   return the value EXCEPTION_EXECUTE_HANDLER to invoke the thread's
*   exception handling routine.
*
*   NOTE!!!  A bug (in MSVC++ V1.0? or NT?) will cause any exception
*   filtering routine that generates an exception itself to be called
*   endlessly.  This procedure uses a _try/_except frame around the
*   call to the thread's filtering routine to guard against this bug.
*   If the thread's filtering routine generates an exception, this
*   procedure's handler will be invoked.  The handler will always return
*   the value EXCEPTION_CONTINUE_SEARCH to cause the next highest layer
*   of exception handling to be invoked instead of the same filtering
*   routine that just crashed.
*
*   The exception handler in this procedure MUST be kept simple (one single
*   return statement).  If additional code or function calls were added to
*   the handler, we would run the risk of generating an exception and having
*   THIS procedure called endlessly.  Therefore, no events are logged in
*   this procedure's exception handler.
*
*
* ARGUMENTS:
*
*   TBinding
*
*       Address of the structure containing the thread information.  This
*       structure is populated with information supplied by the creator
*       (caller of the StartThread() routine).
*
*   XPtr
*
*       A pointer to the exception information for the exception that caused
*       this routine to be called.
*
*   RetXPtr
*
*       A pointer to the location that receives a copy of the exception
*       information referenced by XPtr.
*
*
* RETURNS:
*
*   EXCEPTION_CONTINUE_SEARCH
*   EXCEPTION_EXECUTE_HANDLER
*
*
* SIDE EFFECTS:
*
*   If the thread's exception filtering routine generates an exception
*   itself, the thread's handler will NOT be invoked.  Instead, the next
*   highest layer of exception handling is invoked.
*
*
***************************************************************************/

static int AppFilter( THREAD_BINDING      *TBinding,
                      EXCEPTION_POINTERS  *XPtr,
                      EXCEPTION_POINTERS  *RetXPtr )
{
	EXCEPTION_POINTERS  LPtr;
	int                 status;


    /*
    *  Initialize.  The initial return status should be set up to assume
    *  that we're going to continue the exception search.
    */
    LPtr.ExceptionRecord = NULL;
    LPtr.ContextRecord = NULL;

    status = EXCEPTION_CONTINUE_SEARCH;


    /*
    *  Encase this procedure in an exception handling frame to protect
    *  against the bug mentioned in this procedure's description.
    */
    _try
    {
        /*
        *  Copy the exception information so we can pass it to the exception
        *  handler code.
        */
        CopyExceptionInformation( RetXPtr, XPtr );


        /*
        *  Attempt to call the thread's filter routine if the thread's
        *  _except is calling us.  When the Application Shell's _except
        *  calls us, TBinding is NULL.
        */
        if (TBinding != NULL)
        {
            /*
            *  Did the thread creator provide us with a filter?  If so, call
            *  it.  If not, we assume that the creator always wants it's
            *  exception handler invoked.
            */
            if (TBinding->thread_filter != NULL)
            {
                /*
                *  The thread creator provided an exception filter routine,
                *  so let's call it.  If the filter routine tells us to
                *  execute the creator's exception handler and the creator
                *  did not provide us with a handler, continue the exception
                *  search.  The Application Shell's exception handler will
                *  take care of this exception.
                */
                status = (*(TBinding->thread_filter))( TBinding->arglist, XPtr );
                switch( status )
                {
                    /*
                    *  The thread creator's filter routine told us to execute
                    *  the thread's exception handler.  If the thread doesn't
                    *  have an exception handler, change the status so the
                    *  Application Shell's exception handling will be invoked.
                    */
                    case EXCEPTION_EXECUTE_HANDLER:
                    {
                        if (TBinding->thread_except == NULL)
                            status = EXCEPTION_CONTINUE_SEARCH;
                        break;
                    }


                    /*
                    *  The thread creator's filter routine returned search or
                    *  execute.  Just pass this value on (return it).
                    */
                    case EXCEPTION_CONTINUE_SEARCH:
                    case EXCEPTION_CONTINUE_EXECUTION:
                        break;


                    /*
                    *  The thread creator's filter routine returned garbage.
                    *  Since we can't trust the thread's filter/handler,
                    *  change the status so the Application Shell's exception
                    *  handling will be invoked.
                    */
                    default:
                        status = EXCEPTION_CONTINUE_SEARCH;
                        break;
                }
            }
            else if (TBinding->thread_except != NULL)
            {
                /*
                *  The thread creator didn't provide a filter function but did
                *  provide an exception handling function.  We interpret this
                *  to mean "always call my exception handler", so return the
                *  "execute handler" code.
                */
                status = EXCEPTION_EXECUTE_HANDLER;
            }
            else
            {
                /*
                *  The thread creator didn't provide us with a filter routine
                *  or an exception handling routine.  Continue the search.
                */
                status = EXCEPTION_CONTINUE_SEARCH;
            }
        }
        else
        {
            /*
            *  This is the Application Shell's _except calling.  Execute
            *  the exception handler.
            */
            status = EXCEPTION_EXECUTE_HANDLER;
        }
    }
    _except( CopyExceptionInformation( &LPtr, GetExceptionInformation() ) )
    {
        /*
        *  Some routine called by this procedure (possibly the thread
        *  creator's exception filtering routine) generated an exception!
        *  Log the exception information in the application log.
        */
        LogServiceException( APPSHELL_EXCEPTION, &LPtr );


        /*
        *  If the Application Shell's _except called us, execute it's
        *  handler anyway.  Otherwise continue the search so we invoke
        *  the Application Shell's handler.
        */
        if (TBinding != NULL)
            status = EXCEPTION_CONTINUE_SEARCH;
        else
            status = EXCEPTION_EXECUTE_HANDLER;
    }


    /*
    *  Return the exception disposition.
    */
    return( status );
}


/***************************************************************************
*
* PROCEDURE:
*
*   CrashProcess
*
*
* DESCRIPTION:
*
*   This routine logs the contents of an exception record in the application
*   log and then crashes (exits) the process.
*
*
* ARGUMENTS:
*
*   XPtr
*
*       The address of the exception pointer structure that contains the
*       exception handling chain.
*
*
* RETURNS:
*
*   none
*
*
* SIDE EFFECTS:
*
*   Logs exception information in the application log.
*   Exits the process.
*
*
***************************************************************************/

static void CrashProcess( EXCEPTION_POINTERS *XPtr )
{
    /*
    *  Log the Application Shell exception event.
    */
    if (!halted)
    {
        LogServiceException( APPSHELL_EXCEPTION, XPtr );
    }

    /*
    *  Kill the process.
    */
    ExitProcess(SHELL_EXITTING_NEED_RESTART);
}
