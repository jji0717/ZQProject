/***************************************************************************
**
** Copyright (c) 2006 by
** ZQ Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
**
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of ZQ  Technology  Inc.   Possession, use,
** duplication or dissemination of the software and media is authorized only
** pursuant to a valid written license from ZQ Technology Inc.
**
** This software is furnished under a  license  and  may  be used and copied
** only in accordance with the terms of  such license and with the inclusion
** of the above copyright notice.  This software or any other copies thereof
** may not be provided or otherwise made available to  any other person.  No
** title to and ownership of the software is hereby transferred.
**
** The information in this software is subject to change without  notice and
** should not be construed as a commitment by ZQ Technology Inc.
**
** ZQ  assumes  no  responsibility  for the use or reliability of its
** software on equipment which is not supplied by ZQ.
**
** RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
** Government is subject  to  restrictions  as  set  forth  in  Subparagraph
** (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
***************************************************************************/

/***************************************************************************
**
** TITLE:
**
**		ZQ Service Shell
**
**
** VERSION:
**
**		1.0 001
**
**
** FACILITY
**
**		ITV Services
**
**
** ABSTRACT:
**
**		This module contains the Service Shell.  This shell provides a
**		framework for launching, controlling, and monitoring ITV services.
**		This shell launches ITV service applications by creating another
**		process to execute the service application.  The ITV service applications
**		must be built using the Application Shell.  The Application Shell
**		and Service Shell communicate with each other using NT events.
**
**  
** Issues/bugs
**  o Share-service:
**      Manpkg is designed to support a single client/process.
**          > single journal file (name = ?) per process
**          > single event source registered per process
**          > single management port (port #?) per process
**      Solution:  
**          Either change manpkg to support multiple clients/process
**          or better support single client/process.
**
** o Flow-control problems with ServiceControl Manager.  deadlocking etc.
**
**
** REVISION HISTORY: (Most recent at top)
**
**	 Revision	   Date			Modified By		Reviewed By
**	----------	----------- 	-----------		-----------
**  ITV V1.5    10/24/2006      TONY
**  Add the Install and Unstall Service cmd
**  Install Service cmd: -i servicename
**  Unstall Service cmd: -u servicename
**  
**  Shell prints process ids both in hex and decimal
**	
**	  SCMgrStop event was not being reset after a timeout and was shutting down
**		after restarting service. Added a new event AppStop to stop the app instead of
**		using SCMgrSTOP for double duty which does not work.
**  
**      WindowsNT 3.51 SP5 cannot reboot within Daylight Savings End.
**      Wait until that critical hour ends.       
**	
**		Support Share_process & own_process
**	
**		Added Clog support
**
**  
**		1. Process Application process exit status request:  reboot, restart, stop service
**		2. Runtime determination of SERVICE | NOSERVICE mode to simplify debugging
**  
**		Verify that registry value of LastReboot time precedes current time
**		Support new system types for Greyhound
**		Bullet-proofing efforts
**	
**    Put the actual name of the service in 'pSV->szServiceName' and pass to appshell
**
**
**		Added AppShutdown event which is set by the app prior to calling ExitProcess
**		in order to notify the service shell that the app it terminating. In cases
**		where the ExitProcess hangds (e.g. a DLL detach routine hangs), this event will
**		allow the service shell to detect the app's desire to exit, and will cause the
**		system to be rebooted when the service shell wants to restart the app but finds
**		the app still alive.
**
**	
**
**		The following modifications were made to the Service Shell as
**		part of this edit:
**
**			o Increased some of the service control manager wait hints.
**
**			o Added exception handling and exception logging code.  The
**			  service application is now torn down when an exception occurs,
**			  and the service control manager is informed that the service
**			  has stopped.
**
**			o Added a check to make sure that the stop timeout is greater
**			  than the shutdown wait by a moderate amount.
**
**			o Added code to support PAUSE and CONTINUE of ITV services in
**			  conjunction with changes made to the Application Shell.
**
**			o Cleaned up the copyright, header, etc.  Some of the revision
**			  numbers / dates don't make any sense but were copied anyway.
***
**		Change defaults so restart always happens by default.
**
**		If there is no upstream system in the registry, only restart the
**		service if the system is the master library, IE or SS. Otherwise,
**		fail.
**
**		Added code to interrupt the reboot cycle if a unit is constantly
**		rebooting.
**
**		Nuked startup cmd line param check.  Added event to not allow
**		creation of multiple app processes.
**
**		Fixed bug in call to CFG_GET_PARENT_VALUES that broke remote event
**		logging.
**
**		Initial Fieldtest release .
**
**
***************************************************************************/
#include "ZQ_common_conf.h"

// System Header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

//the delete lib add by dony
#include <iostream>
#include <string>
using std::string;
using std::cin;
//add by dony

#include <vector>
using std::vector ;
//add by romalin

#include <winnt.h>
#include <winsvc.h>
#include <time.h>
#include <excpt.h>
#include <crtdbg.h>  // for ASSERT macro
#include <tchar.h>


// ITV Header files
#include "ZqMessages.h"
#include "zqcfgpkg.h"
#include "ShellCfgPkg.h"
#include <snmp/ZQSnmp.h>
#include <FileLog.h>
#include "strHelper.h"


//#include "LibDefine.h"

#include "shell.h"		// Define default values for start/stop/alive waits and timeouts 
#include "mclog.h"
#include "zq.h"
#include "MDump.h"
#include "DumpState.h"

// Service Vector
	enum {				// Define Array indeces for synchronization handles
		SCMgrSTOP
		, SCMgrPAUSED
		, SCMgrCONTINUED
		, ChildExit
		, ChildAlive
		, AppShutdown
		, SRVSHLNUMHANDLES
		};

// Service State Vector
typedef struct SV_s 
{
	DWORD   dwServiceIndex;					        // Service Index
	BOOL    bIsActive;                       // Is Service Vector currently active/InUse?
	TCHAR   szServiceName[MAXOBJNAMELEN+1];	// Service Name
    TCHAR   szProductName[MAX_PATH+1];
	MCB_t * hMClog;                        // Handle for mclog
	HANDLE  phHandleArray[SRVSHLNUMHANDLES]; // Array of NT synchronization handles
	HANDLE  HSCMgrPAUSE;                   
	HANDLE  HSCMgrContinue;
	HANDLE  AppStop;

	// This mutex shared by the shell and the app
	// is released when the shell process is terminates
	// signalling the possible orphaned service_app to terminate
	HANDLE hMutexShellAlive;

	// Registry parameters
    TCHAR szImagePath[MAX_PATH+1];	
    DWORD dwStartTimeout;
	DWORD dwStopTimeout; 
 	DWORD dwAliveTimeout;
	BOOL bRebootOnExit;
	DWORD dwFacilityMask;
	DWORD dwRestartTries;
	DWORD dwRestartInterval;
	DWORD dwRestartDelay;
	DWORD dwAppShutdownWait;
	DWORD dwPauseTimeout;
	DWORD dwContinueTimeout;
	DWORD dwLastReboot;
	DWORD dwRebootCount;
	TCHAR szLogfilePath[MAX_PATH+1];
    //WCHAR wszLogfilePath[MAX_PATH+1];
	DWORD dwLogfileSize;
	DWORD dwLoggingLevel;
	DWORD dwLoggingMask;

    //////////////////////////////////////////////////////////////////////////
    //added by xiaohui.chai
    DWORD dwSnmpLoggingMask;//for snmp
    TCHAR szArgument[MAX_PATH]; // the addition argument
    DWORD dwKillCheckTime; // the check interval after kill the child process
    DWORD dwKillCheckCount; // the max count of kill check
    //////////////////////////////////////////////////////////////////////////
    
    DWORD dwTotalStarts;    // Number of times this service shell has started the application.
    TCHAR szLastStart[26];   // Time when application last started.
  //WCHAR wszLastStart[26];

	// Service Control Manager data structures
	SERVICE_STATUS          ssStatus;
	SERVICE_STATUS_HANDLE   sshStatusHandle;
	DWORD                   dwGlobalErr;
	DWORD                   dwCheckPoint;

    DWORD					dwServiceOid;
	DWORD					dwInstanceId;

//  Application Process exit code
#define EVENT_M_CODE	0xFFFF		// Code mask for Event
	DWORD	dwAppExitCode;			// Application Process Exit Code

	// When program is requested to stop/shutdown by ServiceControlManager, the Flag is set TRUE.
	// If the flag is FALSE prior to exitting, then the program logs an NT event
	BOOL	bIsNormalTermination;

} SV_t;

//////////////////////////////////
// Global data
#define MAX_SERVICES 32    // Maximum number of services supported
SV_t tbSV[MAX_SERVICES];		// Table of Service State Vectors


// Forward routine Declarations of ServiceControlHandlers for each Service thread
VOID ServiceCtrl_0(DWORD dwCtrlCode);
VOID ServiceCtrl_1(DWORD dwCtrlCode);
VOID ServiceCtrl_2(DWORD dwCtrlCode);
VOID ServiceCtrl_3(DWORD dwCtrlCode);
VOID ServiceCtrl_4(DWORD dwCtrlCode);
VOID ServiceCtrl_5(DWORD dwCtrlCode);
VOID ServiceCtrl_6(DWORD dwCtrlCode);
VOID ServiceCtrl_7(DWORD dwCtrlCode);
VOID ServiceCtrl_8(DWORD dwCtrlCode);
VOID ServiceCtrl_9(DWORD dwCtrlCode);
VOID ServiceCtrl_10(DWORD dwCtrlCode);
VOID ServiceCtrl_11(DWORD dwCtrlCode);
VOID ServiceCtrl_12(DWORD dwCtrlCode);
VOID ServiceCtrl_13(DWORD dwCtrlCode);
VOID ServiceCtrl_14(DWORD dwCtrlCode);
VOID ServiceCtrl_15(DWORD dwCtrlCode);
VOID ServiceCtrl_16(DWORD dwCtrlCode);
VOID ServiceCtrl_17(DWORD dwCtrlCode);
VOID ServiceCtrl_18(DWORD dwCtrlCode);
VOID ServiceCtrl_19(DWORD dwCtrlCode);
VOID ServiceCtrl_20(DWORD dwCtrlCode);
VOID ServiceCtrl_21(DWORD dwCtrlCode);
VOID ServiceCtrl_22(DWORD dwCtrlCode);
VOID ServiceCtrl_23(DWORD dwCtrlCode);
VOID ServiceCtrl_24(DWORD dwCtrlCode);
VOID ServiceCtrl_25(DWORD dwCtrlCode);
VOID ServiceCtrl_26(DWORD dwCtrlCode);
VOID ServiceCtrl_27(DWORD dwCtrlCode);
VOID ServiceCtrl_28(DWORD dwCtrlCode);
VOID ServiceCtrl_29(DWORD dwCtrlCode);
VOID ServiceCtrl_30(DWORD dwCtrlCode);
VOID ServiceCtrl_31(DWORD dwCtrlCode);


// ServiceControl Table
struct SVC_s 
{
	VOID (*SVCHandler)(DWORD);		// Pointer to ServiceControlHandler
	SV_t *SVC_SV;				    // Pointer to Service Vector
}
tbSVC[MAX_SERVICES] = 
{
	{ServiceCtrl_0, tbSV+0}
	, {ServiceCtrl_1, tbSV+1}
	, {ServiceCtrl_2, tbSV+2}
	, {ServiceCtrl_3, tbSV+3}
	, {ServiceCtrl_4, tbSV+4}
	, {ServiceCtrl_5, tbSV+5}
	, {ServiceCtrl_6, tbSV+6}
	, {ServiceCtrl_7, tbSV+7}
	, {ServiceCtrl_8, tbSV+8}
	, {ServiceCtrl_9, tbSV+9}
	, {ServiceCtrl_10, tbSV+10}
	, {ServiceCtrl_11, tbSV+11}
	, {ServiceCtrl_12, tbSV+12}
	, {ServiceCtrl_13, tbSV+13}
	, {ServiceCtrl_14, tbSV+14}
	, {ServiceCtrl_15, tbSV+15}
	, {ServiceCtrl_16, tbSV+16}
	, {ServiceCtrl_17, tbSV+17}
	, {ServiceCtrl_18, tbSV+18}
	, {ServiceCtrl_19, tbSV+19}
	, {ServiceCtrl_20, tbSV+20}
	, {ServiceCtrl_21, tbSV+21}
	, {ServiceCtrl_22, tbSV+22}
	, {ServiceCtrl_23, tbSV+23}
	, {ServiceCtrl_24, tbSV+24}
	, {ServiceCtrl_25, tbSV+25}
	, {ServiceCtrl_26, tbSV+26}
	, {ServiceCtrl_27, tbSV+27}
	, {ServiceCtrl_28, tbSV+28}
	, {ServiceCtrl_29, tbSV+29}
	, {ServiceCtrl_30, tbSV+30}
	, {ServiceCtrl_31, tbSV+31}
};

// SVC Stack variables
CRITICAL_SECTION csSV;			// Serializes access to these critical variables
int SVCStk[MAX_SERVICES];		// Stack of indeces of Unallocated ServiceControl Entries 
int SVC_SP = MAX_SERVICES;	// Stack pointer(index)

ZQ::common::FileLog* logger;
ZQ::SNMP::ModuleMIB* mmib;
ZQ::SNMP::SubAgent* snmpSubAgent;

BOOL	bIsServiceMode;			  // Service Mode = {SERVICE | NOSERVICE}
BOOL	bIsServiceTypeShare;	// ServiceType = {SHARE | OWN}
DWORD   dwTlspSV;             // Thread local storage index

TCHAR  tceventsink[MAX_COMPUTERNAME_LENGTH + 3] =_T("");
TCHAR  tcsysname[MAX_COMPUTERNAME_LENGTH + 3] = _T("\\\\");
TCHAR *pszEmptyString = _T("");

//////////////////////////////////////////////////////////////////////////
//added by xiaohui.chai
//there should be only one service be managed in the process
static SV_t *f_pActiveSvc = NULL;// for snmp
static void SnmpCallback(const char *varName)
{
    if(NULL == varName)
        return;

    if(NULL == f_pActiveSvc)
        return;

    if(0 == strcmp(varName, "LoggingLevel"))
    {
        //assume that hMClog is valid, so must close session before MCLogTerm
        MClogSetTraceLevel(f_pActiveSvc->hMClog, f_pActiveSvc->dwLoggingLevel);
    }
}

// register snmp
static void registerSnmp(const char* logFilename, int nLogFileSize, SV_t *pSV)
{
    //create FileLog object
    if(logFilename)
    {
        DWORD loglevel = pSV->dwSnmpLoggingMask & 0x0f;
        logger = new ZQ::common::FileLog(logFilename, loglevel, 3/* filenum */, nLogFileSize);
    }

    mmib = new ZQ::SNMP::ModuleMIB(*logger, pSV->dwServiceOid, ZQ::SNMP::ModuleMIB::ModuleOid_SvcShell, pSV->dwInstanceId);
    snmpSubAgent = new ZQ::SNMP::SubAgent(*logger, *mmib, 5000); // timeout 5000ms

    mmib->addObject(new ZQ::SNMP::SNMPObject("LoggingMask", 	    (uint32&)pSV->dwLoggingMask), 	     ".1");
    mmib->addObject(new ZQ::SNMP::SNMPObject("SnmpLoggingMask", 	(uint32&)pSV->dwSnmpLoggingMask), 	     ".2");
    mmib->addObject(new ZQ::SNMP::SNMPObject("LogDir", 	            (uint32&)pSV->szLogfilePath), 	     ".3");
    mmib->addObject(new ZQ::SNMP::SNMPObject("LogfileSize", 	    (uint32&)pSV->dwLogfileSize), 	     ".4");
    mmib->addObject(new ZQ::SNMP::SNMPObject("LoggingLevel", 	    (uint32&)pSV->dwLoggingLevel, false), ".5");
    mmib->addObject(new ZQ::SNMP::SNMPObject("ImagePath", 		    pSV->szImagePath), 	     ".6");
    mmib->addObject(new ZQ::SNMP::SNMPObject("AliveTimeout", 	    (uint32&)pSV->dwAliveTimeout), 	     ".7");
    mmib->addObject(new ZQ::SNMP::SNMPObject("Restart Threshold", 	(uint32&)pSV->dwRestartTries, false), ".8");
    mmib->addObject(new ZQ::SNMP::SNMPObject("RestartInterval", 	(uint32&)pSV->dwRestartInterval, false), ".9");
    mmib->addObject(new ZQ::SNMP::SNMPObject("RestartDelay", 		(uint32&)pSV->dwRestartDelay), 	     ".10");
    mmib->addObject(new ZQ::SNMP::SNMPObject("Last Restart", 		pSV->szLastStart), 	     ".11");
    mmib->addObject(new ZQ::SNMP::SNMPObject("StartTimeout", 	    (int32&)pSV->dwStartTimeout, false), ".12");

    mmib->addObject(new ZQ::SNMP::SNMPObject("IsServiceTypeShare", 	bIsServiceTypeShare), 	     ".13");
    mmib->addObject(new ZQ::SNMP::SNMPObject("Total Starts", 		(uint32&)pSV->dwTotalStarts), 	     ".14");
    mmib->addObject(new ZQ::SNMP::SNMPObject("StopTimeout", 	    (uint32&)pSV->dwStopTimeout, false), ".15");
    mmib->addObject(new ZQ::SNMP::SNMPObject("RebootOnExit", 		pSV->bRebootOnExit, false), ".16");  
    mmib->addObject(new ZQ::SNMP::SNMPObject("KillCheckTime", 	    (uint32&)pSV->dwKillCheckTime), 	     ".17");
    mmib->addObject(new ZQ::SNMP::SNMPObject("KillCheckCount", 	    (uint32&)pSV->dwKillCheckCount), 	     ".18");
    snmpSubAgent->start();
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Default register values
#define	PAUSE_TIMEOUT	 30000			// Time to wait for app to respond to Pause request  X2.2-0 001
#define	CONTINUE_TIMEOUT 30000		// Time to wait for app to respond to Continue request  X2.2-0 001
#define RESTART_DELAY    200			// Time to delay between restarting application process

/* Default values for reboot cycle checks - T1.0-04 */
#define	DEF_REBOOTTIMETHRESH 1			// 1-second reboot window		T1-0.6
#define DEF_REBOOTCOUNTLIMIT 1000		// 1000 per second (always reboot)	T1-0.6
#define SERVICE_NAME     _T("SNMP")     
#define ALIVE1_TIMEOUT   5000           

//  Forward routine declarations
//VOID    service_main(DWORD argc, char *argv[]);
VOID WINAPI service_main(DWORD dwArgc, LPTSTR lpszArgv[]);
VOID WINAPI service_ctrl(SV_t* pSV, DWORD dwCtrlCode);
int     service_init(SV_t* pSV);
int     ReportStatusToSCMgr(SV_t *pSV, DWORD dwCurrentState, DWORD dwWin32ExitCode,DWORD dwCheckPoint, DWORD dwWaitHint);
int	    StopService(SV_t *pSV, BOOL restart, PROCESS_INFORMATION *proc_info);
VOID    LogServiceEvent(DWORD eventID,
					WORD strcnt,
					LPCTSTR *strarray,
					DWORD bdatacnt,
					LPVOID bdata,
					TCHAR *szServiceName,
					DWORD dwFacilityMask,
					TCHAR *eventsink,
					TCHAR *sysname);
VOID    cleanup(SV_t *pSV, TCHAR *app, PROCESS_INFORMATION *proc_info);
BOOL    reboot(SV_t *pSV);

BOOL    CheckDSTEnd(DWORD* pdwMin);
BOOL    CheckDefaultDSTEnd(DWORD* pdwMin);
WORD    GetInstanceOfDay(struct tm *currenttime);

BOOL  BuildDispatchTable (TCHAR *pszImagePath, SERVICE_TABLE_ENTRY *pServiceTable, BOOL *bShareService);
// unsigned int MgmtServiceTableCallBack(char* pszCmd, char** ppRespBuf, int* piRespSize);
void  LogServiceException( DWORD, EXCEPTION_POINTERS* );
int   CopyExceptionInformation( EXCEPTION_POINTERS*, EXCEPTION_POINTERS* );
void  FreeExceptionInformation( EXCEPTION_POINTERS* );

// add by dony for ZQShell.exe of Install and Unstall service
void  InstallService(TCHAR  *pstrServiceName);
void  UnstallService(TCHAR  *pstrServiceName);
void  InitRegeditData(TCHAR *pstrServiceName,TCHAR *pstrProdName,TCHAR *szImageFile,TCHAR *szServiceOID,TCHAR *szEvnetDllName,BOOL bCmdType = TRUE);
//void  ResetServieCurrentOID(TCHAR *pstrServiceName,TCHAR *pstrProdName);
void  AddService(TCHAR *pstrServiceName,TCHAR *pszProduceName,TCHAR *pszImageFileName,TCHAR *szServiceOID,TCHAR *pszAccount,TCHAR *pszPassword,TCHAR *szEventDllName,TCHAR *szStartType=_T("manual"),BOOL bCmdType = TRUE);
//BOOL  ReStartSNMPService(MCB_t *hHandle);
BOOL  MyStopService(MCB_t *hHandle,SC_HANDLE hManager, TCHAR * szServiceName,BOOL bStopDependants = TRUE);
BOOL  IsInstalled(TCHAR *pstrServiceName);
BOOL  bNotAlive;
// add by dony for ZQShell.exe of Install and Unstall service

/////////////////////////////////////////////////////////////////////////////////////
// main()
//  Perform minimal process initialization
//  Invoke the the StartServiceCtrlDispatcher to register the services
//  
#if defined _UNICODE || defined UNICODE
	VOID wmain(int argc, TCHAR *argv[])
#else
	VOID main(int argc, TCHAR* argv[])
#endif
{
	SERVICE_TABLE_ENTRY stDispatchTable[MAX_SERVICES+1];
	BOOL bCmdType = TRUE;

	_try 
	{
		// Allocate Thread local storage index
		if ((dwTlspSV = TlsAlloc()) == 0XFFFFFFFF) _leave;

		// Initialize stack of unallocated ServiceControl entries
		InitializeCriticalSection(&csSV);
		for (SVC_SP=MAX_SERVICES; SVC_SP>0; )
		{
			SVCStk[--SVC_SP] = SVC_SP;
		}

		// If invoked from cmd shell with cmd line: <image> -s <service-name>
		// Then, by ITV convention, run interactively as a process -- not an NT service.
		if ((argc >=3) && (_tcsicmp(argv[1], _T("-s") )== 0))
		{
			bIsServiceMode = FALSE;         // Not NT service environment
			stDispatchTable[0].lpServiceName = pszEmptyString;
			stDispatchTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)service_main;
			stDispatchTable[1].lpServiceName = NULL;							
			stDispatchTable[1].lpServiceProc = NULL;
			service_main(argc-2, &argv[2]);
		}
		// If invoked from cmd shell with cmd line: <image> -i <service-name> <product-name> <serviceprograme'simagefile-name> <szOIDValue>  < szAccount > < szPassword > <szEventDllName> <szStartType>
		else if ((argc >=3) && (_tcsicmp(argv[1], _T("-i") )== 0)) // add by dony for install service
		{                       
			if ( argc >= 3  && argc <= 4 )  // If invoked from cmd shell with cmd line: <image> -i <service-name>, add by dony
			{
				InstallService(argv[2]);
			}
			else if ( argc == 10 ) // argv[4] ="Shell's image filename
			{

				AddService(argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8],argv[9],bCmdType);
			}
			else
			{
				_tprintf(_T("        Command Mode Setup:\n"));
				_tprintf(_T("              ZQShell -i szServiceName szProduceName, szImageFileName szOid \n"));
				_tprintf(_T("       	      szAccount,szPassword,szEventDllName,szStartType(auto/manuatl) \n"));
				_tprintf(_T("\n"));

				_tprintf(_T("        Guide Mode Setup:\n"));
				_tprintf(_T("              ZQShell -i szServiceName\n"));
				_tprintf(_T("\n"));

				_tprintf(_T("        Unstall Mode:\n"));
				_tprintf(_T("              ZQShell -u szServiceName\n"));
				_tprintf(_T("\n"));
				_leave;
			}
		}
		// If invoked from cmd shell with cmd line: <image> -u <service-name>
		else if ((argc >=3) && (_tcsicmp(argv[1], _T("-u") )== 0)) // add by dony for unstall service
		{
			UnstallService(argv[2]);
		}
		else if ( argc == 1 )
		{
			
			bIsServiceMode = TRUE;          // NT Service environment
		
			if (BuildDispatchTable(argv[0], stDispatchTable, &bIsServiceTypeShare))			
			{
				
				StartServiceCtrlDispatcher(stDispatchTable);
			}
			else
			{
				SC_UASSERT(FALSE);  // we got problems, we need to know about 'em
			}
		}
		else
		{
			_tprintf(_T("        Command Mode Setup:\n"));
			_tprintf(_T("              ZQShell -i szServiceName szProduceName, szImageFileName szOid \n"));
			_tprintf(_T("       	      szAccount,szPassword,szEventDllName,szStartType(auto/manuatl) \n"));
			_tprintf(_T("\n"));

			_tprintf(_T("        Guide Mode Setup:\n"));
			_tprintf(_T("              ZQShell -i szServiceName\n"));
			_tprintf(_T("\n"));

			_tprintf(_T("        Unstall Mode:\n"));
			_tprintf(_T("              ZQShell -u szServiceName\n"));
			_tprintf(_T("\n"));
			_leave;
		}
	}
	_finally 
	{
	}
	DeleteCriticalSection(&csSV);
}
//////////////////////////////////////////////////////////////////////////////
// Build Service Dispatch Table from SCMgr's repository of service definitions
//
BOOL BuildDispatchTable (TCHAR *pszImagePath, SERVICE_TABLE_ENTRY *pServiceTable
                        , BOOL *pbShareService) 
{

	int iSts;
	int j= 0;

	TCHAR *pStr;

	SC_HANDLE schSCMgr;
	SC_HANDLE schService;

	ENUM_SERVICE_STATUS tbEnumServicesStatus[64];
	unsigned long iBytesNeeded;
	unsigned long iNEntriesRead;
	DWORD dwResumeHandle;


	// Try to build Service Dispatch Table

	// Obtain handle to ServiceControlManager
	if ((schSCMgr = OpenSCManager(NULL, NULL, GENERIC_READ)) == NULL)
	{
		iSts = GetLastError();
		_tprintf(_T("OpenSCManager failed -- %d"), iSts); // TBS 
		return (FALSE);
	}
			
	// Retrieve 1st group of ServiceStatus blocks from SCMgr
	dwResumeHandle = 0;
	if (!EnumServicesStatus(
		schSCMgr
		, SERVICE_WIN32
		, SERVICE_ACTIVE + SERVICE_INACTIVE
		, tbEnumServicesStatus
		, sizeof(tbEnumServicesStatus)
		, &iBytesNeeded
		, &iNEntriesRead
		, &dwResumeHandle
	)) 
	{
		iSts = GetLastError();
		if (iSts != ERROR_MORE_DATA) 
		{
			_tprintf(_T("EnumServicesStatus failed -- %d"), iSts);  // tbs
			return (FALSE);
		}
	} 
	else
	{
		iSts = 0;
	}
						
	// Process groups of ServiceStatus blocks
	while (iNEntriesRead > 0) 
	{

		// Process body of ServiceStatus blocks within current group
		unsigned long dwIndex;
		for (dwIndex = 0; dwIndex < iNEntriesRead; dwIndex++) 
		{

			// If Service Status is either stopped or starting (?), then retrieve its binary path
			if ((tbEnumServicesStatus[dwIndex].ServiceStatus.dwCurrentState == SERVICE_STOPPED)
				|| (tbEnumServicesStatus[dwIndex].ServiceStatus.dwCurrentState == SERVICE_START_PENDING)) 
			{

				// Obtain handle to Service
				schService = OpenService(schSCMgr, tbEnumServicesStatus[dwIndex].lpServiceName, SERVICE_QUERY_CONFIG);

				if (schService == NULL)
				{
				    _tprintf(_T("Failure: OpenService %s -- %d"), 
				    	tbEnumServicesStatus[dwIndex].lpServiceName, GetLastError());
					return (FALSE);
				}

				// Retrieve Service Configuration
				DWORD cbBytesNeeded;
				LPQUERY_SERVICE_CONFIG pQSCBuf = NULL;

				// determine the buffer size needed
				QueryServiceConfig(schService, NULL, 0, &cbBytesNeeded);
				pQSCBuf = (LPQUERY_SERVICE_CONFIG)malloc(cbBytesNeeded);
				
				if (pQSCBuf == NULL) 
				{
					_tprintf(_T("Malloc failure") );
					return(FALSE);
				}
 
				if (!QueryServiceConfig(schService,
									pQSCBuf,
									cbBytesNeeded,
									&cbBytesNeeded)
					) 
				{
					_tprintf(_T("QueryServiceConfig %s failed -- %d")
								,tbEnumServicesStatus[dwIndex].lpServiceName, GetLastError());
					
					if(pQSCBuf)
					{
						free(pQSCBuf);
						pQSCBuf = NULL;
					}
					return(FALSE);

				} 
				else
				{
					// If path of service image == path of self, then MATCH
					// If the current image is the BinaryPathName of the service, then 
					// Construct Service Table entry(s)
					if (_tcsicmp(pQSCBuf->lpBinaryPathName, pszImagePath) == 0) 
					{
						// If Service Type == OWN, then Construct complete OWN table and return success
						if (tbEnumServicesStatus[dwIndex].ServiceStatus.dwServiceType == SERVICE_WIN32_OWN_PROCESS) 
						{
							pServiceTable[0].lpServiceName = pszEmptyString;
							pServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)service_main;
							pServiceTable[1].lpServiceName = NULL;							
							pServiceTable[1].lpServiceProc = NULL;
							*pbShareService = FALSE;

							if(pQSCBuf)
							{
								free(pQSCBuf);
								pQSCBuf = NULL;
							}
							
							return(TRUE);
							
						// Else ServiceType = SHARE, append entry to Service Dispatch Table														
						} else 
						{						

							if (j >= MAX_SERVICES) 
							{
									_tprintf(_T("Service Entry Table overflow (> %d)"), MAX_SERVICES);
									if(pQSCBuf)
									{
										free(pQSCBuf);
										pQSCBuf = NULL;
									}
									return(FALSE);

							} 
							else 
							{
								pStr = (TCHAR *)malloc(_tcsclen(tbEnumServicesStatus[dwIndex].lpServiceName) +2);
								if (pStr == NULL) 
								{
									_tprintf(_T("Malloc failure") );
									if(pQSCBuf)
									{
										free(pQSCBuf);
										pQSCBuf = NULL;
									}
									return(FALSE);
								}
								_tcscpy(pStr, tbEnumServicesStatus[dwIndex].lpServiceName);
								pServiceTable[j].lpServiceName = pStr;              
								pServiceTable[j].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)service_main;
								j++;
							}
						}
					}
				}
				CloseServiceHandle(schService);
			}
		}
		// Read next possible group of entries
		if (iSts == ERROR_MORE_DATA) 
		{
			if (!EnumServicesStatus(
				schSCMgr
				, SERVICE_WIN32
				, SERVICE_ACTIVE + SERVICE_INACTIVE
				, tbEnumServicesStatus
				, sizeof(tbEnumServicesStatus)
				, &iBytesNeeded
				, &iNEntriesRead
				, &dwResumeHandle
			)) 
			{
				iSts = GetLastError();
				if (iSts != ERROR_MORE_DATA) 
				{
					iNEntriesRead = 0;
					_tprintf(_T("EnumServicesStatus failed -- %d"), iSts);
					return (FALSE);
				}
			} 
			else 
			{
				iSts = 0;
			}
		} 
		else
		{
			iNEntriesRead = 0;
		}
	}

	CloseServiceHandle(schSCMgr);
	pServiceTable[j].lpServiceName = NULL;
	pServiceTable[j].lpServiceProc = NULL;

	if (j == 0) 
	{
		_tprintf(_T("No Services to manage") );
		return(FALSE);
	}

	*pbShareService = TRUE;
	return (TRUE);
}

void GetInstanceId( const TCHAR* serviceName, DWORD& dwInstanceId )
{
	dwInstanceId = 0;
	TCHAR szKey[MAX_PATH+1];
	_stprintf (szKey,_T("SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\%s_shell"),serviceName);
	LONG lRet;
	HKEY hKey;
	DWORD dwType;
	DWORD dwSize = MAX_PATH;

	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lRet)
	{
		dwSize = sizeof(dwInstanceId); 
		lRet = RegQueryValueEx(hKey, _T("InstanceId"), NULL, &dwType, (LPBYTE)&dwInstanceId, &dwSize);
		RegCloseKey(hKey);	
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//  GetProductName -- fetch from the registry the product name for which this service 
//              is installed.
//              It is stored in the following key
//      HKEY_LOCAL_MACHINE
//          System
//            CurrentControlSet
//              Services
//                pSV->szServiceName
//                  ProductName REG_SZ ""
//
// Note: We can not use CFGPKG here because we require the product name to know where
//       to look things up in the registry.  Here we know we are a service and 
//       we know how to find our service key in the registry.
// 
void GetProductName(const TCHAR* lpszServiceName, TCHAR lpszProductName[MAX_PATH+1])
{
    LONG lRet;
    HKEY hKey;
    TCHAR szKey[MAX_PATH+1] = _T("SYSTEM\\CurrentControlSet\\Services\\");
    TCHAR szValue[] = _T("ProductName");
    DWORD dwType;
    DWORD dwSize = MAX_PATH;

    _tcscat(szKey, lpszServiceName);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == lRet)
    {
        dwSize = MAX_PATH*(sizeof(TCHAR)); 
		lRet = RegQueryValueEx(hKey, szValue, NULL, &dwType, (LPBYTE)lpszProductName, &dwSize);
        RegCloseKey(hKey);

        if (ERROR_SUCCESS == lRet && REG_EXPAND_SZ == dwType)
        {
            TCHAR szProductName[MAX_PATH + 1];
            ExpandEnvironmentStrings(lpszProductName, szProductName, MAX_PATH);
            _tcsncpy(lpszProductName, szProductName, MAX_PATH+1);
        }
    }

    if (ERROR_SUCCESS != lRet)
    {
        /////////////
        /// TODO: NT Log this error!!
        LPCTSTR msg[10];
        TCHAR buf1[64];
        _stprintf(buf1, _T("%ld"), lRet);
        msg[0] = lpszServiceName;
        msg[1] = buf1;
        LogServiceEvent(SHELL_NO_PRODUCT, 2, msg, 0, NULL, const_cast<TCHAR*>(lpszServiceName), (ZQShell << 16), tceventsink, tcsysname);

        
        //Assume ITV
        _tcscpy(lpszProductName, _T("ITV"));
    }

}

bool  GetReg_SpecificVale(const TCHAR* lpszServiceName,
						  TCHAR lpszValeName[MAX_PATH+1],
						  TCHAR  lpszValeData[MAX_PATH+1] )
{	

	LONG lRet;
	HKEY hKey;
	TCHAR szKey[MAX_PATH+1] = _T("SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\");
	DWORD dwType;
	DWORD dwSize = MAX_PATH;

	_tcscat(szKey, lpszServiceName);

	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == lRet)
	{
		dwSize = MAX_PATH*(sizeof(TCHAR)); 
		lRet = RegQueryValueEx(hKey,lpszValeName, NULL, &dwType, (LPBYTE)(lpszValeData), &dwSize);
		if (ERROR_SUCCESS == lRet)
			goto RET ;
		else
			return false ;
	}
	else
		return false ;
RET:
	RegCloseKey(hKey);
	return true ;
}
//
static bool _CopyFile(SV_t *pSv,string _pFrom, string _pTo,WORD flags)
{
	MClog(pSv->hMClog, 1,"_CopyFile fun Entry .... ");
	TCHAR pTo[MAX_PATH]={0}; 
	_tcscpy(pTo,_pTo.c_str());
	TCHAR pFrom[MAX_PATH]={0}; 
	_tcscpy(pFrom,_pFrom.c_str());

	SHFILEOPSTRUCT FileOp={0}; 
	FileOp.fFlags= FOF_SILENT |FOF_NOCONFIRMATION| FOF_NOCONFIRMMKDIR  ;
	FileOp.pFrom =pFrom;
	FileOp.pTo = pTo; 
	FileOp.wFunc = FO_COPY; 
	MClog(pSv->hMClog, 1, "       beginning copying : %s ",_pFrom.c_str());
	if (SHFileOperation(&FileOp) == 0)
		MClog(pSv->hMClog, 1, "       copied : %s  Success",_pFrom.c_str());
	else
		MClog(pSv->hMClog, 1, "       copied : %s  Failed",_pFrom.c_str());

    MClog(pSv->hMClog, 1,"_CopyFile fun Exit .... ");
	return 0;
}
//copy a uncertain file to a specific  directory 
void DatafileToLogdir(SV_t *pSv)
{
	TCHAR  ServiceName_Shell[64+1]={0} ;
	TCHAR   LogDir[MAX_PATH]={0},CopiedData[MAX_PATH]=_T("");
	bool  bFlag=false ;
	string  strCopiedData ;
	
	wsprintf(ServiceName_Shell,_T("%s_shell"),pSv->szServiceName);

	bFlag = GetReg_SpecificVale(ServiceName_Shell,_T("logDir"),LogDir) ;
	if(false == bFlag)
	{
		MClog(pSv->hMClog, 1, _T("Open the reg SubKey: %s failed"), ServiceName_Shell);
		return ;
	}
	lstrcat(LogDir, _T("\\")); 
	bFlag =  GetReg_SpecificVale(ServiceName_Shell,_T("backupFiles"),CopiedData) ;
	if(false ==  bFlag )
	{
		MClog(pSv->hMClog, 1, _T("Open the reg ValueName: %s failed"), ServiceName_Shell);
		return ;
	}
	strCopiedData=CopiedData ;
	if (strCopiedData.empty())
	{
		  MClog(pSv->hMClog, 1, _T("Files or Directory For Copying: %s null value!"), CopiedData);
		  return ;
	}
    MClog(pSv->hMClog, 1, _T("Files or Directory For Copying: %s "), CopiedData);

	std::vector<std::string> result_vecstr;
	ZQ::common::stringHelper::SplitString(CopiedData,result_vecstr,";") ;
	vector<string>::iterator itr ;
	for(itr=result_vecstr.begin();itr != result_vecstr.end();itr++)
	{
		_CopyFile(pSv,*itr, LogDir, 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//  service_main() --	This is the service thread started by StartServiceCtrlDispatcher
//
//      This function takes care of actually starting the service,
//      informing the service controller at each step along the way.
//      After launching the service_app process, it waits on the service_app process'
//      handle, which will signal at its termination.
//
//      The service command line is constructed by ServiceControlManager
//		Contents:
//             argv[0]    - service name
//             argv[1...] - strings supplied by StartService caller
//
VOID WINAPI service_main(DWORD argc, LPTSTR lpszArgv[])
{
	#define CMD_LINE_LEN 512

	SV_t *pSV = NULL;			// Pointer to Service Vector

	DWORD                   WaitReason;
	time_t                  start_time, end_time, last_start_time;
	DWORD                   restart_count = 0;
	int						          i;
	double                  elapsed_time;
	BOOL                    SCStop, Alive;
	STARTUPINFO             startup;
	PROCESS_INFORMATION     proc_info;
	int                     wait_time, status;
	LPCTSTR                 msg[10];
	EXCEPTION_POINTERS		  XPtr;		
	TCHAR                    buf[512];
	TCHAR                   cmd_line[CMD_LINE_LEN];
	TCHAR                   sm_buf[32];
	BOOL					bClean = FALSE; // indicates we haven't been cleaned up
	int                     iAliveNum; // add by dony for judge the first to start service 
	iAliveNum     = 0; // add by dony for judge the first to start service 

	proc_info.hProcess = NULL;
	proc_info.hThread = NULL;
	proc_info.dwProcessId = 0;
	proc_info.dwThreadId = 0;

	bNotAlive  = FALSE; // add by dony for save process log

	_try {	// Try-except
		_try 
		{   // Try-finally

			// Initialize exception handling
			XPtr.ExceptionRecord = NULL;				//X2.2-0 001
			XPtr.ContextRecord = NULL;					//X2.2-0 001

			// Allocate ServiceVector and its SCHandler
			_try 
			{
				EnterCriticalSection(&csSV); 

				// Pop next available index off stack
				if (SVC_SP >= MAX_SERVICES) 
				{
					
					LogServiceEvent(SHELL_INITFAIL, 2, msg, 0, NULL, lpszArgv[0], (ZQShell << 16), tceventsink, tcsysname);
					_leave;
				}
				else 
				{
					i = SVCStk[SVC_SP++];
				}

				pSV = tbSVC[i].SVC_SV;
				memset(pSV, 0, sizeof(SV_t));
				pSV->dwServiceIndex = i;	// Save index
				pSV->bIsActive = TRUE;
				if (!(TlsSetValue(dwTlspSV, pSV))) 
				{
					_leave; // Store pSV value in thread local storage
				}
			} 
			_finally 
			{
				LeaveCriticalSection(&csSV);
			}

			// Initialize service
			{
				_tcscpy(pSV->szServiceName,lpszArgv[0]); 			// 10/10/95 CJH - put real service name in 'pSV->szServiceName'
			}
			GetInstanceId( pSV->szServiceName, pSV->dwInstanceId );
			GetProductName( pSV->szServiceName, pSV->szProductName );

			pSV->phHandleArray[ChildExit] = NULL;  // we've never been created.
   			if ((status = service_init(pSV)) != NO_ERROR) 
			{
				msg[0] = pSV->szServiceName;
				_stprintf(buf, _T("%d"), status);
				msg[1] = buf;
				LogServiceEvent(SHELL_INITFAIL, 2, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
				ReportStatusToSCMgr(pSV, SERVICE_STOPPED,0,0,0);	// Tell service manager we're done.
				_stprintf(buf, _T("%s_shell"), pSV->szServiceName);
				cleanup(pSV, buf, &proc_info);
				bClean = TRUE;
				_leave;		// Quit the service without rebooting... Allow user to reconfigure system.
			}

		// Finish initialization by getting the command line for the child process
		// and filling in the STARTUPINFO structure. Set up bogus start and end
		// times for first time through.
			TCHAR szInstanceId[32];
			_stprintf(szInstanceId,_T("%u"),pSV->dwInstanceId);

			/*_tcscpy(cmd_line, _T("\"-s ") );
			_tcscat(cmd_line, pSV->szServiceName);
			_tcscat(cmd_line, _T("\" \"-p ") );
			_tcscat(cmd_line, pSV->szProductName);
			_tcscat(cmd_line, _T("\" \"-i ") );			
			_tcscat(cmd_line, szInstanceId);
			_tcscat(cmd_line, _T("\"") );*/

            _tcscpy(cmd_line, pSV->szImagePath );
            _tcscpy(cmd_line, _T(" -s ") );
            _tcscat(cmd_line, pSV->szServiceName);
            _tcscat(cmd_line, _T(" -p ") );
            _tcscat(cmd_line, pSV->szProductName);
            _tcscat(cmd_line, _T(" -i ") );			
            _tcscat(cmd_line, szInstanceId);
            //_tcscat(cmd_line, _T(" ") );

            // append the addition argument
            if((_tcslen(cmd_line) + _tcslen(pSV->szArgument) + 4) < sizeof(cmd_line))
            {
                if (strlen(pSV->szArgument) != 0)
                {
                    _tcscat(cmd_line, _T(" -n "));
                    _tcscat(cmd_line, pSV->szArgument);
                }
            }
            else
            {
                if(pSV->dwLoggingMask)
                    MClog(pSV->hMClog, 1, _T("Failed to append application argument [%s] to [%s] because of the command line buffer size [%u].")
                    , pSV->szArgument, cmd_line, sizeof(cmd_line) );
            }

            // append the runtime argument
			for (i = 1; i < (int)argc; i++)
			{
				if (_tcslen(cmd_line) + _tcslen(lpszArgv[i]) + 1 < sizeof(cmd_line)) 
				{
					_tcscat(cmd_line, _T(" ") );
					_tcscat(cmd_line, lpszArgv[i]);
					
				}
                else
                {
                    if(pSV->dwLoggingMask)
                        MClog(pSV->hMClog, 1, _T("Failed to append application argument [%s] to [%s] because of the command line buffer size [%u].")
                        , lpszArgv[i], cmd_line, sizeof(cmd_line) );
                }
			}

            _tcscat(cmd_line, _T(" -m") ); // service mode flag

			startup.cb = sizeof(STARTUPINFO);
			startup.lpReserved = NULL;
			startup.lpDesktop = NULL;
			startup.lpTitle = NULL;
			startup.dwFlags = 0;
			startup.cbReserved2 = 0;
			startup.lpReserved = NULL;

			time(&start_time);
			time(&end_time);

			if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Initialization Completed") );
            MClog(pSV->hMClog, 1, _T("command line[%s]"), cmd_line);

	/*
	 * Continue executing this loop until the Service Control manager says
	 * stop, or something unrecoverable happens.
	 */
			SCStop = FALSE;

			while (!SCStop) 
			{

			// Compare service_app restart counters with configuration parameters.
			// If elapsed run time <= RestartTimeInterval
			//   If restart_counter >  RestartLimit, then
			//		Log Event "Too Many Restarts"
			//		Reboot | Quit process
				elapsed_time = difftime(end_time, start_time);

				if (elapsed_time <= pSV->dwRestartInterval) 
				{
					if (++restart_count > pSV->dwRestartTries) 
					{  // If too many restarts within interval, then abort

						// Log NT event
						msg[0] = pSV->szServiceName;
						_stprintf(buf, _T("%d"), restart_count - 1);
						msg[1] = buf;
						_stprintf(&buf[100], _T("%.0f"), elapsed_time);
						msg[2] = &buf[100];
						_tcscpy(&buf[200], pSV->bRebootOnExit ? _T("Rebooting...") :	_T("Exiting...") );
						msg[3] = &buf[200];
						LogServiceEvent(SHELL_TOO_MANY_RESTARTS, 4, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

						// add 20070419 for detail message for restart service
						if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T("Service application %s experiencing too many restarts:%s  starts within %s seconds. %s..."),msg[0],msg[1],msg[2],msg[3]);
						

						// If configured to reboot, then reboot system else quit process,如果注册表配置参数为1,表示重启，否则退出
						if (pSV->bRebootOnExit)
						{
							reboot(pSV);
						}
						else
						{
							ReportStatusToSCMgr(pSV, SERVICE_STOPPED,0,0,0);	// Tell service manager we're done.
							_stprintf(buf, _T("%s_shell"), pSV->szServiceName);
							cleanup(pSV, buf, &proc_info);
							bClean = TRUE;
							_leave;			// Quit the service without rebooting... Allow user to repair system
						}
					}
				} 
				else 
				{	   				// Else begin new restart sequence interval
 					restart_count = 1;		//   reset count
					time(&start_time);		//   reset start time
				}

				// If "start pending state" then Report status to SCMgr
				if (pSV->ssStatus.dwCurrentState == SERVICE_START_PENDING)
					ReportStatusToSCMgr(pSV, 
								SERVICE_START_PENDING, // service state
								NO_ERROR,              // exit code
								pSV->dwCheckPoint++,          // pSV->dwCheckPoint
								pSV->dwStartTimeout);         // wait hint


				// Added 10/5/94, T1.0-03, MMS
				// An independent sanity check to be sure that the service app is not already running.
				// The appshell will create an event named <service name>_ALREADY_UP. This event will
				// stay in existance until the app image terminates. Try to open this event, and if
				// successful twice (five seconds apart), then the app is still up when the servshell
				// thinks it needs to be recreated. This should not happen. Log and event announcing
				// what's going on, then reboot if configured to do so, else exit.
				{
				  TCHAR         tcevent_name[MAXOBJNAMELEN+16];
				  HANDLE        event_hand;
				  BOOL          app_up;

				  _stprintf(tcevent_name,_T("%s_ALREADY_UP"), pSV->szServiceName);     		// Build event name <service-name>_ALREADY_UP
				  event_hand = OpenEvent(SYNCHRONIZE, TRUE, tcevent_name);  	// Try to open the event
				  app_up = ((event_hand != NULL) || (GetLastError() != ERROR_FILE_NOT_FOUND));
													// Success or unexp err, assume app still up
				  CloseHandle(event_hand);          // Close in case open
				  if (app_up)
				  {                     // If app still up
					Sleep(5000);                    // Wait 5 sec
					event_hand = OpenEvent(SYNCHRONIZE, TRUE, tcevent_name);  // Try again
					app_up = ((event_hand != NULL) || (GetLastError() != ERROR_FILE_NOT_FOUND));
					CloseHandle(event_hand);        // Close in case open
					if (app_up)
					{                   // Still up on 2nd try?
					  _tcscpy(tcevent_name, pSV->bRebootOnExit ? _T("Rebooting...") : _T("Exiting...") );
					  msg[0] = pSV->szServiceName;
					  msg[1] = tcevent_name;
					  LogServiceEvent(SHELL_APP_ALREADY_UP, 2, msg, 0, NULL,// Log reboot required
								pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

					  // add 20070419 for add detail message for log
					  if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T("Service  %s  wants to create application process, but application is already running %s."),msg[0],msg[1]);

					  if (pSV->bRebootOnExit)
					  {
						  reboot(pSV);
					  }
					  else
					  {
						ReportStatusToSCMgr(pSV, SERVICE_STOPPED,0,0,0);	// Tell service manager we're done.
						_stprintf(buf, _T("%s_shell"), pSV->szServiceName);
						cleanup(pSV, buf, &proc_info);
						bClean = TRUE;
						_leave;		// QUIT service thread
					  }
					}
				  }
				 }
				  // add by dony if the heartbeat is down namely the service app is not  running.
			
				// End 10/5 mod


		  		// Create service_app process
				if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Creating Process %s..."), pSV->szImagePath);
				if (!CreateProcess(
					pSV->szImagePath,	// service's image name
					cmd_line,   // recontructed command line
						NULL,   	// null security descriptor for process...
						NULL,   	//  and thread
					TRUE,       // inherit parent's handles
					DETACHED_PROCESS,// no console
						NULL,       // inherit parent's environment
					NULL,       // inherit drive and directory
					&startup,   // pointer to startup parameters
						&proc_info)	// Get proc info back here
				) 
				{
					msg[0] = pSV->szServiceName;
					msg[1] = cmd_line;
					msg[2] = pSV->szImagePath;
					status = GetCurrentDirectory(sizeof(buf)-1, buf);
					msg[3] = buf;

					 _stprintf(sm_buf, _T("%d"), GetLastError());

					msg[4] = sm_buf;
					msg[5] = pSV->bRebootOnExit ? _T("Rebooting...") : _T("Exiting...");
					LogServiceEvent(SHELL_START_PROC_FAIL, 6, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

					// add 20070419 for add detail message for log
					if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T("Service  %s  failed to create application process, Command line is %s, Image is %s,Current directory is %s,Error is %s"),msg[0],msg[1],msg[2],msg[3],msg[4],msg[5]);
	
					if (pSV->bRebootOnExit)
					{
						reboot(pSV);				// If RebootOnExit, then try to reboot system
					}
					else
					{
						ReportStatusToSCMgr(pSV, SERVICE_STOPPED,0,0,0);	// Tell service manager we're done.
						_stprintf(buf, _T("%s_shell"), pSV->szServiceName);
						cleanup(pSV, buf, &proc_info);
						bClean = TRUE;
						_leave;		// QUIT service thread
					}
				}

				CloseHandle(proc_info.hThread);
				pSV->phHandleArray[ChildExit] = proc_info.hProcess;	// Save process handle to waitfor child exit signal
				Alive = TRUE;           						// Set service_app alive flag
				pSV->dwAppExitCode = 0;								// Clear App Proc Exit code

				// Log NT Event "Service shell starting service %1 with ProcessId %2."
				msg[0] = pSV->szServiceName;
				_stprintf(buf, _T("%d (0x%x)"), proc_info.dwProcessId,proc_info.dwProcessId);
				msg[1] = buf;
				LogServiceEvent(SHELL_STARTING_SVC, 2, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
				if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Created service app: image %s, pid=%u...,Restart Count = %d"), pSV->szImagePath, proc_info.dwProcessId,restart_count);
			/*
			 * Keep a count of starts and set latest time. Report the status to the
			 * service control manager.
			 */
				pSV->dwTotalStarts += 1;
				time(&last_start_time);
				_tcscpy(pSV->szLastStart, _tctime(&last_start_time));
				pSV->szLastStart[24] = _T('\0');
				//MultiByteToWideChar(CP_ACP, 0, pSV->szLastStart, -1, pSV->wszLastStart, 26);

				wait_time = pSV->dwStartTimeout;  // Give the service "Start Timeout" (default 30) secs to start
				if (pSV->ssStatus.dwCurrentState == SERVICE_START_PENDING)
			  		status = ReportStatusToSCMgr(pSV, 
							SERVICE_START_PENDING,// service state
							NO_ERROR,                               // exit code
							pSV->dwCheckPoint++,         // pSV->dwCheckPoint
							wait_time + 10000);   // wait hint		//X2.2-0 001
		
		//     While service_app process is running, wait for one of the following events:
		//      - the service is stopped by the SC manager
		//      - the service is notified that NT is shutting down
		//		- the service is notified that the service_app has PAUSED or CONTINUED
		//      - the service_app shut itself down
		//      - the alive event is set, indicating the child is still up
		//      - the child handle is set indicating the child process has unexpectedly terminated
		//		- a time out occurs, indicating the child is NOT alive
		//

            if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("The app process is ready. Start the monitoring (timeout=%d)."), wait_time);
			while (Alive) 
			{
				
				 DWORD        dwAlive;
				
				  // Wait for one of multiple synchronization objects to signal
	//			  if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, "WaitForMultipleObjectsEx");
				   

				  WaitReason = WaitForMultipleObjectsEx(
								SRVSHLNUMHANDLES,   // Number of handles to wait on
								pSV->phHandleArray,       // Array of objects to wait on
								FALSE,              // Only wait for one handle
								wait_time,          // wait
								FALSE);             // Not alertable - no I/O happening
				  
				  
				  
					  

					// Dispatch on Wait result
					switch (WaitReason) 
					{

						// The SCMgrSTOP event is signalled when processing these conditions
						// within the Service Shell:
						//	1. When the SCMgr calls the service control handler with a STOP | SHUTDOWN request
						//		then the handler sets this event
						//  2. When an unexpected failure or exception occurs, then the event is set
						//
						case WAIT_OBJECT_0 + SCMgrSTOP:
							SetEvent(pSV->AppStop);
							if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("SCMgrStop Event occurred") );

							SCStop = TRUE;
							if (!StopService(pSV, !SCStop, &proc_info)) // Wait for service_app to terminate
								pSV->bIsNormalTermination = FALSE;        
							// Note if the service_app fails to terminate,
							// then the user needs to reboot the system
							Alive = FALSE;
							msg[0] = pSV->szServiceName;
							LogServiceEvent(SHELL_NORMAL_EXIT, 1, msg, 0, NULL,
											pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

							
							//ResetServieCurrentOID(pSV->szServiceName,pSV->szProductName);  // add by dony 20061201 for reset the current's oid
							

							break;

					// X2.2-0 002 Add, 5/3/95, MMS

					/*
					**	The AppShutdown event is signalled, when the app is going to call ExitProcess.
					**	Clear "Alive" so that a restart gets attempted. Don't set SCStop, because
					**	we want to restart. StopService("TRUE",..) to enable restarting
					*/

						case WAIT_OBJECT_0 + AppShutdown:
							if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("AppShutdown Event occurred") );
							ResetEvent(pSV->phHandleArray[AppShutdown]);	// Reset prior to app restart
							// Do not log an unexpected shut down here.  It's not unexpected and
							// the StopService call below logs everything very nicely.
							if (!(StopService(pSV, !SCStop, &proc_info))) // Wait for app to exit, reboot if not
								pSV->bIsNormalTermination = FALSE;        
                        				time(&end_time);                    // Set the app exit time
							Alive = FALSE;				// Flag app no longer alive

							if (1 == pSV->dwRestartTries)
							{
								// we don't want to log restart, this is basically a service
								// that wants to be able to shut itself down.
								// MSCS integration does this.
								// the Error event is misleading and annoying
								if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("ZQShell configured not to restart.  Exiting normally.") );
								SCStop = TRUE;

								//ResetServieCurrentOID(pSV->szServiceName,pSV->szProductName);  // add by dony 20061201 for reset the current's oid
							}
							break;
					// End X2.2-0 002 add

					//X2.2-0 001 start
						/*
						*  The PAUSED event was signalled.
						*/
						case WAIT_OBJECT_0 + SCMgrPAUSED:
						{
							if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("SCMgrPAUSED Event occurred") );
							/*
							*  The application has informed us that it is paused.
							*  Tell the service control manager.
							*/
							ReportStatusToSCMgr(pSV, SERVICE_PAUSED, NO_ERROR, 0, 0);

							/*
							*  Log a message stating that we're paused.
							*/
							msg[0] = pSV->szServiceName;
							LogServiceEvent(SHELL_PAUSED, 1, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);


							break;
						}


						/*
						*  The CONTINUED event was signalled.
						*/
						case WAIT_OBJECT_0 + SCMgrCONTINUED:
						{
							if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("SCMgrCONTINUED Event occurred") );
							/*
							*  The application has informed us that it has
							*  continued.  Tell the service control manager.
							*/
							ReportStatusToSCMgr(pSV, SERVICE_RUNNING, NO_ERROR, 0, 0);

							/*
							*  Log a message stating that we're paused.
							*/
							msg[ 0 ] = pSV->szServiceName;
							LogServiceEvent(SHELL_CONTINUED, 1, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
							break;
						}
					//X2.2-0 001 end

					/*
					**      We should only reach here when something unexpected has happened
					**      in the app, or the app has called ExitProcess.
					*/
						case WAIT_OBJECT_0 + ChildExit:

							if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("ChildExit Event occurred") );

							// Retrieve Application process exit code
							GetExitCodeProcess(pSV->phHandleArray[ChildExit], &pSV->dwAppExitCode);
							pSV->dwAppExitCode &= EVENT_M_CODE;
							CloseHandle(pSV->phHandleArray[ChildExit]);	// Close process handle
							pSV->phHandleArray[ChildExit] = NULL;
							Alive = FALSE;
							time(&end_time);
							msg[0] = pSV->szServiceName;
							_stprintf(buf, _T("%d (0x%x)"), proc_info.dwProcessId,proc_info.dwProcessId); 
							msg[1] = buf;
							_stprintf(&buf[16], _T("%.04x"), pSV->dwAppExitCode); 
							msg[2] = &buf[16];

							LogServiceEvent(SHELL_UNEXP_EXIT, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
						
							//backup the specific data
							DatafileToLogdir(pSV);
							//ResetServieCurrentOID(pSV->szServiceName,pSV->szProductName);  // add by dony 20061201 for reset the current's oid
							
							break;

					/*
					** We got an Alive Event strobe from the service_app. The first time through,
					** report to the SC mgr that we are running. Otherwise, the event is
					** manually reset. If the report to SCMgr fails,
					** let it ride.  We'll probably get shut down anyway.
					**
					** NOTE - if eventually, we muck with reporting other states to the
					**                      SCMgr during restarts, change the logic here to be
					**                      pSV->ssStatus.dwCurrentState != SERVICE_RUNNING
					*/
						
						case WAIT_OBJECT_0 + ChildAlive:
							
							// for the test 20061127 for check the shunxun of varialbe 
							iAliveNum ++;

							if (pSV->ssStatus.dwCurrentState == SERVICE_START_PENDING) 
							{
								if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("ChildAlive Event occurred") );
									ReportStatusToSCMgr(pSV, SERVICE_RUNNING, NO_ERROR, 0, 0);
							}
							ResetEvent(pSV->phHandleArray[ChildAlive]);
                            if(wait_time != pSV->dwAliveTimeout)
                            {
                                wait_time = pSV->dwAliveTimeout;	// Update wait time to "AliveTimeout" (default=45 seconds)
                                if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("App report ChildAlive Event. Update the timeout to %d"), wait_time);
                            }

							break;
						
					/*
					 * The child hasn't kept the alive protocol going. Go kill him,
					 * initially by pretending the SCMgr sent us a stop.  Loop back
					 * to re-start after waiting for the child in StopService.
					 */
						case WAIT_TIMEOUT:
					
							
							// add by dony below  if the heartbeat is down namely the service app is not  running.
							// An independent sanity check to be sure that the service app is not already running.
							// The appshell will create an event named <service name>_Alive. This event will
							// stay in existance until the app image terminates. Try to open this event, and if
							// successful twice (five seconds apart), then the app is still up when the servshell
							// thinks it needs to be recreated. This should not happen. Log and event announcing
							// what's going on, then reboot if configured to do so, else exit.

							// add by dony for check 心跳
                            if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Child Alive WAIT_TIMEOUT Event occurred. Check again to confirm (timeout=%d)."), wait_time);
							dwAlive =WaitForSingleObject(pSV->phHandleArray[ChildAlive],wait_time);
							// add by dony above if the heartbeat is down namely the service app is not  running.
							if ( dwAlive == WAIT_TIMEOUT )
							{
                                if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Child Alive Event not occurred in the second waiting cycle (timeout=%d). Need restart the app."), wait_time);
                                // check the dumping state
                                if(DumpState::isDumping(pSV->szServiceName))
                                { // dumping now, just ignore this timeout
                                    if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("The application process is dumping now. Perform no action until the dumping is complete."));
                                }
                                else // isn't dumping, kill it.
                                {
                                    bNotAlive  = TRUE; // add  for save process log
                                    // save the process log
                                    //								dumper.SaveProcessLog(DumpPath,pSV->szServiceName,&proc_info);
                                    //								dumper.CallProceAPI();
                                    // save the process log
                                    msg[0] = pSV->szServiceName;
                                    //sprintf(buf, "%x", proc_info.dwProcessId);
                                    _stprintf(buf, _T("%d (0x%x)"), proc_info.dwProcessId,proc_info.dwProcessId);
                                    msg[1] = buf;
                                    LogServiceEvent(SHELL_ALIVE_TIMEOUT, 2, msg, 0, NULL,
                                                    pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

                                    // Signal StopEvent, triggering service_app to terminate.
                                    SetEvent(pSV->AppStop);
                                    if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Signal the child process to quit."));

                                    Alive = FALSE;
                                    if (!(StopService(pSV, !SCStop, &proc_info))) // Wait for app to exit, reboot if not
                                        pSV->bIsNormalTermination = FALSE;        
                                    time(&end_time);
                                    ResetEvent(pSV->AppStop);
                                } // isn't dumping
							}
                            else
                            {
                                if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Child Alive Event occurred in the second waiting cycle (timeout=%d)."), wait_time);
                            }
                            break;
						default:           // Unexpected wait condition...terminate process
							if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T( "default Event occurred") );
							// Signal StopEvent, triggering service_app to terminate.
							SetEvent(pSV->AppStop);
							_stprintf(buf,_T("%d"), WaitReason);
							msg[0] = buf;
							msg[1] = pSV->szServiceName;
							LogServiceEvent(SHELL_UNKNOWN_WAIT_RESULT, 2, msg, 0, NULL,
															pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
							Alive = FALSE;
							SCStop = TRUE;
							StopService(pSV, !SCStop, &proc_info);	// Wait for service_app to terminate
							time(&end_time);
							pSV->bIsNormalTermination = FALSE;        

							//ResetServieCurrentOID(pSV->szServiceName,pSV->szProductName);  // add by dony 20061201 for reset the current's oid

							break;

						} // End wakeup reason switch
					}  // End ALIVE loop

					// Process Application process exit code
					switch (pSV->dwAppExitCode) 
					{

						// Application needs a system reboot in order to recover
						case SHELL_EXITTING_NEED_REBOOT & EVENT_M_CODE :
							if (pSV->bRebootOnExit) reboot(pSV);
							break;
						
						// Application knows that user intervention is required to repair system
						// so do not attempt to reboot or to restart the application																		
						case SHELL_EXITTING_NEED_INTERVENTION & EVENT_M_CODE :
							SCStop = TRUE;
							break;
						
						// General case:  If number of restarts has not exceeded the limit,
						//				  then restart application process
						//			      else 
						//					if RebootOnExit	then reboot system
						//					else terminate the service
						case SHELL_EXITTING_NEED_RESTART & EVENT_M_CODE :
						default :	 ;
					}

					if (!SCStop) 
					{
						// Reset Manual events
						// ResetEvent(pSV->phHandleArray[SCMgrSTOP]);  NEVER do this here, do it above when we were start pending only in service_init
						ResetEvent(pSV->phHandleArray[ChildAlive]);
						ResetEvent(pSV->phHandleArray[AppShutdown]);

						// Sleep for "RestartDelay" time
						Sleep(pSV->dwRestartDelay);
					}

				} // End NOT Stopped loop

				// Successful termination
				_stprintf(buf, _T("%s_shell"), pSV->szServiceName);
				cleanup(pSV, buf, &proc_info);
				bClean = TRUE; // on Exception we don't want to cleanup again.
			} 
			_finally 
			{
                //added by xiaohui.chai
                //must close session before MClogTerm, see SnmpCallback for detail(this file).
                f_pActiveSvc = NULL;

                if (mmib){
                    delete mmib;
                }

                if (snmpSubAgent)
                {
                    snmpSubAgent->stop();
                }

				if (pSV) 
				{
					MClogTerm(pSV->hMClog);			// Terminate MClog, releasing resources.
					// Deallocate Service Vector and its SCHandler
					_try
					{
						EnterCriticalSection(&csSV); 

						// Deallocate
						pSV->bIsActive = FALSE;

						if (SVC_SP <= 0)
						{
							_leave;
						} 
						else 
						{
							SVCStk[--SVC_SP] = pSV->dwServiceIndex;
						}
					} 
					_finally
					{
						LeaveCriticalSection(&csSV);
					}
				}

				if (pSV->bIsNormalTermination == FALSE) 
				{
					msg[0] = pSV->szServiceName;
					LogServiceEvent(SHELL_ABEND, 1, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
				}
			}

	// Functionality no longer works with svcsubr.c  eg logging source > "_shell"
	// Ditto bogus externs
		} _except( CopyExceptionInformation( &XPtr, GetExceptionInformation() ) ) {
		 DWORD  XCode;

			/*
			*  Log the exception information.
			*/
			XCode = GetExceptionCode();
			LogServiceException( SHELL_EXCEPTION, &XPtr );

			/*
			*  Set StopEvent, requesting service_app to terminate
			*/
			if (pSV->AppStop != (HANDLE)NULL)
				SetEvent(pSV->AppStop);

			/*
			*  Wait for the service_app to terminate|reboot|Log Fatal NT Event
			*  and then tell the service manager we stopped.  Cleanup any
			*  open handles, etc.
			*/
			StopService(pSV, FALSE, &proc_info );
			pSV->ssStatus.dwServiceSpecificExitCode = XCode;
			ReportStatusToSCMgr(pSV, SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0, 0);

			if (FALSE == bClean)  // only if we haven't cleaned up already
			{
				_stprintf( buf, _T("%s_shell"), pSV->szServiceName );
				cleanup(pSV,  buf, &proc_info );
			}
		}

		// we get here; we're stopping
		ReportStatusToSCMgr(pSV, SERVICE_STOPPED,0,0,0);	// Tell service manager we're done.

		return;
}

//////////////////////////////////////////////////////////////////
// Service Control Handlers.
//	General design rule: Callbacks should pass context for identification purposes.
//  
VOID ServiceCtrl_0(DWORD dwCtrlCode) { service_ctrl(tbSV+0, dwCtrlCode);}
VOID ServiceCtrl_1(DWORD dwCtrlCode) { service_ctrl(tbSV+1, dwCtrlCode);}
VOID ServiceCtrl_2(DWORD dwCtrlCode) { service_ctrl(tbSV+2, dwCtrlCode);}
VOID ServiceCtrl_3(DWORD dwCtrlCode) { service_ctrl(tbSV+3, dwCtrlCode);}
VOID ServiceCtrl_4(DWORD dwCtrlCode) { service_ctrl(tbSV+4, dwCtrlCode);}
VOID ServiceCtrl_5(DWORD dwCtrlCode) { service_ctrl(tbSV+5, dwCtrlCode);}
VOID ServiceCtrl_6(DWORD dwCtrlCode) { service_ctrl(tbSV+6, dwCtrlCode);}
VOID ServiceCtrl_7(DWORD dwCtrlCode) { service_ctrl(tbSV+7, dwCtrlCode);}
VOID ServiceCtrl_8(DWORD dwCtrlCode) { service_ctrl(tbSV+8, dwCtrlCode);}
VOID ServiceCtrl_9(DWORD dwCtrlCode) { service_ctrl(tbSV+9, dwCtrlCode);}
VOID ServiceCtrl_10(DWORD dwCtrlCode) { service_ctrl(tbSV+10, dwCtrlCode);}
VOID ServiceCtrl_11(DWORD dwCtrlCode) { service_ctrl(tbSV+11, dwCtrlCode);}
VOID ServiceCtrl_12(DWORD dwCtrlCode) { service_ctrl(tbSV+12, dwCtrlCode);}
VOID ServiceCtrl_13(DWORD dwCtrlCode) { service_ctrl(tbSV+13, dwCtrlCode);}
VOID ServiceCtrl_14(DWORD dwCtrlCode) { service_ctrl(tbSV+14, dwCtrlCode);}
VOID ServiceCtrl_15(DWORD dwCtrlCode) { service_ctrl(tbSV+15, dwCtrlCode);}
VOID ServiceCtrl_16(DWORD dwCtrlCode) { service_ctrl(tbSV+16, dwCtrlCode);}
VOID ServiceCtrl_17(DWORD dwCtrlCode) { service_ctrl(tbSV+17, dwCtrlCode);}
VOID ServiceCtrl_18(DWORD dwCtrlCode) { service_ctrl(tbSV+18, dwCtrlCode);}
VOID ServiceCtrl_19(DWORD dwCtrlCode) { service_ctrl(tbSV+19, dwCtrlCode);}
VOID ServiceCtrl_20(DWORD dwCtrlCode) { service_ctrl(tbSV+20, dwCtrlCode);}
VOID ServiceCtrl_21(DWORD dwCtrlCode) { service_ctrl(tbSV+21, dwCtrlCode);}
VOID ServiceCtrl_22(DWORD dwCtrlCode) { service_ctrl(tbSV+22, dwCtrlCode);}
VOID ServiceCtrl_23(DWORD dwCtrlCode) { service_ctrl(tbSV+23, dwCtrlCode);}
VOID ServiceCtrl_24(DWORD dwCtrlCode) { service_ctrl(tbSV+24, dwCtrlCode);}
VOID ServiceCtrl_25(DWORD dwCtrlCode) { service_ctrl(tbSV+25, dwCtrlCode);}
VOID ServiceCtrl_26(DWORD dwCtrlCode) { service_ctrl(tbSV+26, dwCtrlCode);}
VOID ServiceCtrl_27(DWORD dwCtrlCode) { service_ctrl(tbSV+27, dwCtrlCode);}
VOID ServiceCtrl_28(DWORD dwCtrlCode) { service_ctrl(tbSV+28, dwCtrlCode);}
VOID ServiceCtrl_29(DWORD dwCtrlCode) { service_ctrl(tbSV+29, dwCtrlCode);}
VOID ServiceCtrl_30(DWORD dwCtrlCode) { service_ctrl(tbSV+30, dwCtrlCode);}
VOID ServiceCtrl_31(DWORD dwCtrlCode) { service_ctrl(tbSV+31, dwCtrlCode);}

//  service_ctrl() -- Service Control Handler
// 		This function is called by the Service Control Management facility
//		to service ControlService requests --
//		(pause, continue, stop, shutdown, interrogate)
//
VOID WINAPI service_ctrl(SV_t *pSV, DWORD dwCtrlCode)
{
	LPCTSTR  msg[1] = {pSV->szServiceName};

  	// Service the request.
	if (!pSV->bIsActive) 
	{
		MClog(pSV->hMClog, 1,_T( "%d: Service Control request = %d for %s...but Service not active!")
			, GetCurrentThreadId, dwCtrlCode, pSV->szServiceName);
		return;
	}


  	switch(dwCtrlCode)
	{

		//X2.2-0 001 start
		case SERVICE_CONTROL_PAUSE:	
		{
				if (pSV->dwLoggingMask) 
					MClog(pSV->hMClog, 1, _T("%d: Service Control request = %s"), GetCurrentThreadId, "SERVICE_CONTROL_PAUSE");
				LogServiceEvent(SHELL_PAUSING_SVC, 1, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

				//  Set the pause event object and then inform the service control manager.
				if (!SetEvent( pSV->HSCMgrPAUSE )) 
					MClog(pSV->hMClog, 1, _T("SetEvent failure, GetLastError = %d"), GetLastError());
				 
				ReportStatusToSCMgr(pSV, SERVICE_PAUSE_PENDING, NO_ERROR, 0,	pSV->dwPauseTimeout);
			break;
		}

		case SERVICE_CONTROL_CONTINUE:
		{
		    // Log a message stating that we're attempting the continue...
			LogServiceEvent(SHELL_CONTINUING_SVC, 1, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

			//  Set the continue event and then inform the service control manager.
		    if (!SetEvent( pSV->HSCMgrContinue ))
				MClog(pSV->hMClog, 1, _T("SetEvent failure, GetLastError = %d"), GetLastError());

		    ReportStatusToSCMgr(pSV, SERVICE_CONTINUE_PENDING, NO_ERROR, 0, pSV->dwContinueTimeout );
			break;
		}
		//X2.2-0 001 end

		// Stop the service.
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:

            // added log message for distinguish the system shutdown from user
            if(SERVICE_CONTROL_STOP == dwCtrlCode)
            {
                MClog(pSV->hMClog, 1, _T("Requested to stop the service."));
            }
            else
            {
                MClog(pSV->hMClog, 1, _T("System is shutting down..."));
            }
		    // Report the status, specifying the pSV->dwCheckPoint and wait hint,
		    //  before setting the termination event.
			pSV->dwCheckPoint = 0;
		    ReportStatusToSCMgr(pSV, SERVICE_STOP_PENDING, NO_ERROR, pSV->dwCheckPoint++, 30000);
			MClog(pSV->hMClog, 1, _T("ReportStatusToSCMgr(pSV, SERVICE_STOP_PENDING..)"));

			LogServiceEvent(SHELL_STOPPING_SVC, 1, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
			MClog(pSV->hMClog, 1, _T("LogServiceEvent(SHELL_STOPPING_SVC...)"));

		    SetEvent(pSV->phHandleArray[SCMgrSTOP]);           // Signal event.
			MClog(pSV->hMClog, 1,_T( "SetEvent(pSV->phHandleArray[SCMgrSTOP])"));

			pSV->bIsNormalTermination = TRUE;
			break;

		// Interrogate the service status.
		case SERVICE_CONTROL_INTERROGATE:
		    ReportStatusToSCMgr(pSV, pSV->ssStatus.dwCurrentState, NO_ERROR, pSV->dwCheckPoint, 0);
	    	break;

		// invalid control code
		default:
	    	break;

  	}
}


// utility functions...

//////////////////////////////////////////////////////////////////////////////
// ReportStatusToSCMgr(pSV, ) --
//      This function is called by the ServMainFunc() and
//      ServCtrlHandler() functions to report the service's status
//      to the service control manager.
// Returns
//	NO_ERROR	-- success
//	other		-- error
//
int ReportStatusToSCMgr(SV_t *pSV, DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwCheckPoint, DWORD dwWaitHint)
{
    TCHAR    buf[256];
	int fResult = NO_ERROR;  // Assume success
    int i;

    // Disable control requests until the service is started.
	switch (dwCurrentState)
	{
		case SERVICE_START_PENDING:
		case SERVICE_STOP_PENDING:
			pSV->ssStatus.dwControlsAccepted = 0;
			pSV->ssStatus.dwCheckPoint = dwCheckPoint;
			break;

		default:
//X2.2-0 001 start
//				pSV->ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
//												SERVICE_ACCEPT_SHUTDOWN;
			pSV->ssStatus.dwControlsAccepted =	SERVICE_ACCEPT_STOP |
											SERVICE_ACCEPT_SHUTDOWN |
											SERVICE_ACCEPT_PAUSE_CONTINUE;
//X2.2-0 001 end
			pSV->ssStatus.dwCheckPoint = 0;
			break;
	}

    // These SERVICE_STATUS members are set from parameters.
    pSV->ssStatus.dwCurrentState = dwCurrentState;
    pSV->ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    pSV->ssStatus.dwWaitHint = dwWaitHint;

	if (bIsServiceMode) 
	{
        // Report the status of the service to the service control manager.
		LPCTSTR msg[2];	//X2.2-0 001

        // if we get an RPC_S_SERVER_TOO_BUSY error, we sleep for 
        // two seconds and try again, we try this 5 times, then bail.
        // the sleep time and # of tries are both pretty arbitrary
        // you can tinker with it later if you like.
        i=0;
        do
        {
            if (!SetServiceStatus(
				    pSV->sshStatusHandle,   // service reference handle
				    &pSV->ssStatus))        // SERVICE_STATUS structure
            {         
			    fResult = GetLastError();
			    _stprintf(buf, _T("%d"), fResult);
			    _stprintf(&buf[16], _T("%d"), dwCurrentState);
			    msg[0] = buf;
			    msg[1] = &buf[16];
			    LogServiceEvent(SHELL_LOST_SCMGR_CNXN, 2, msg, 0, NULL,
											    pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
		        if (RPC_S_SERVER_TOO_BUSY == fResult)
                {
                    Sleep(2000);  // two seconds == two thousand milliseconds
                    i++;
                    if (i > 5)
                    {
                        // we've exceeded retries, log this and break out
                        // (ZQShell)Unable to report status to Service Control Manager.
                        // After trying several times, the RPC Server is still too busy.
                        // Service %1 is terminating.
                        msg[0] = pSV->szServiceName;
                        LogServiceEvent(SHELL_SCMGR_RPC_BUSY, 1, msg, 0, NULL,
                                            pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
                        break;  // out of the while loop, fResult is RPC_S_SERVER_TOO_BUSY
                    }
                }

            }
            else
                fResult = ERROR_SUCCESS; // SetServiceStatus was successful!

        } while(fResult == RPC_S_SERVER_TOO_BUSY);

  }					//X2.2-0 001

  return fResult;
}
/*
 ******************************************************************************
 *  StopService()
 *
 *  Description:
 *  	This function is called to stop the child process. If restart is TRUE,
 *              don't tell the service control manager about this instance of the service
 *              stopping. Otherwise, go into a STOP_PENDING mode.
 *
 *              If the service process refuses to stop, try up to max_killtries to stop
 *              it. If it persists, shut the system down with the reboot flag enabled.
 *
 *  Input:
 *              restart -
 *
 *  Output:
 *			TRUE - if success, else FALSE if failure
 *
 *  Returns:
 *
 *  Side Effects:
 *
 *  Assumptions:
 ******************************************************************************
 */
int StopService(SV_t *pSV, BOOL restart, PROCESS_INFORMATION *proc_info)
{
	LPCTSTR msg[3];
	DWORD   WaitReason;
	BOOL    ItsAlive = TRUE;
	BOOL	MutexIsReleased = FALSE;
	int		i;
	int 	iSts;
	BOOL	IsSuccess = TRUE;
	BOOL	iRebootSts = FALSE;
    TCHAR   buf[256];

	// add by dony for the save process log
	MiniDump      dumper;
	TCHAR        *DumpPath=_T("C:\\Temp");

//	double       elapsed_time;
	// add by dony for the save process log

#define NTRIES 3

    // only attempt to stop the service if  we have ever created the process
	if (NULL == pSV->phHandleArray[ChildExit] )
		return IsSuccess;

	// Application has been requested to terminate.  Mechanism is setevent
	// Wait for application process to terminate.
	// 
	// If application doesn't respond, then release ShellAlive mutex 
	// to signal request for immediate termination.
	//
	for (i=0; i<NTRIES; i++) 
	{

		// If NOT Restarting service_app, then
		//	Report "Stop_Pending" status to SCMmgr
		if (!restart)
		{
			ReportStatusToSCMgr(pSV, 
				SERVICE_STOP_PENDING, // current state
				NO_ERROR,             // exit code
				pSV->dwCheckPoint++,         // pSV->dwCheckPoint
				(pSV->dwStopTimeout + 10000)); // waithint	//X2.2-0 001
		}

		// Wait for service application process to terminate
		// tmo = Registry value of <service>_shell\StopTimeout

		if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Waiting the child process to quit (%d time, timeout=%d)..."), i + 1, pSV->dwStopTimeout);

		WaitReason = WaitForSingleObjectEx(pSV->phHandleArray[ChildExit], pSV->dwStopTimeout, FALSE);

		// save the process log add by dony
		if ( bNotAlive )
		{
			//removed by xiaohui.chai
			//disable minidump to keep away from the bug in these calls
			//need to be improved
			//				dumper.SaveProcessLog(DumpPath,pSV->szServiceName,proc_info);
			//				dumper.CallProceAPI();
			bNotAlive = FALSE;
		}
		// save the process log add by dony

		// If ServiceApp process has terminated, then
		// LogEvent "successful termination"
		if (WaitReason == WAIT_OBJECT_0)
		{ 	// If Service_app process has terminated
			// Retrieve Application Process Exit Code

			GetExitCodeProcess(pSV->phHandleArray[ChildExit], &pSV->dwAppExitCode);
			pSV->dwAppExitCode &= EVENT_M_CODE;

			if (!CloseHandle(pSV->phHandleArray[ChildExit])) 
			{
				_stprintf(buf, _T("child process (PId=%u )"), proc_info->dwProcessId);
				msg[0] = buf;
				_stprintf(&buf[64], _T("%d"), GetLastError());
				msg[1] = &buf[64];
				LogServiceEvent(SHELL_CLOSE_ERR, 2, msg, 0, NULL,
					pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
			} 
			else 
			{
				msg[0]=pSV->szServiceName;
				//sprintf(buf, "%x", proc_info->dwProcessId);
				_stprintf(buf, _T("%d (0x%x)"), proc_info->dwProcessId,proc_info->dwProcessId);
				msg[1]=buf;
				_stprintf(&buf[16], _T("%.04x"), pSV->dwAppExitCode);
				msg[2]=&buf[16];
				LogServiceEvent(SHELL_APP_TERM_OK, 3, msg, 0, NULL, 
					pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
				pSV->phHandleArray[ChildExit] = NULL;
			}

			if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T( "Child process has terminated in %d tries"), i + 1);
			ItsAlive = FALSE;
			break;
		} 

		// Else Service App is NOT Responding to stop request !!!
		// If held, then release ShellAlive mutex to request immediate abort
		if (!MutexIsReleased)
		{
			if (ReleaseMutex(pSV->hMutexShellAlive)) MutexIsReleased = TRUE;
			msg[0]=pSV->szServiceName;
			//sprintf(buf, "%x", proc_info->dwProcessId);
			_stprintf(buf, _T("%d (0x%x)"), proc_info->dwProcessId,proc_info->dwProcessId);
			msg[1]=buf;
			LogServiceEvent(SHELL_RELEASING_SHELLALIVE_MUTEX, 2, msg, 0, NULL, 
				pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
		}

		//SPR 2665 The Service shell must try to kill the process 
		// using TerminateProcess before reboot.
		if( (NTRIES-1) == i)  // This is our last try
		{

			//SPR 2728 The srv shell must try to terminate in all cases. 
			// So the if condition is removed (commented) by Anand Bosco on 09/01/02
			/*
			if( (!restart) && (pSV->bRebootOnExit)) // if the stop request is not restarting 
			{										//then kill the process*/
			msg[0] = pSV->szServiceName;
			_stprintf(buf, _T("%d (0x%x)"), proc_info->dwProcessId,proc_info->dwProcessId);
			msg[1] = buf;
			msg[2] = _T("Attempting to kill the process...");
			LogServiceEvent(SHELL_NOKILL, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
			// DLLs will not be notified that process is exiting
			// Clean up will not be done!
			if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T( "Child process is still alive after %d*%d msec. Terminate it immediately."), NTRIES, pSV->dwStopTimeout);
			if(0 == TerminateProcess(proc_info->hProcess,0))
			{	
				//Terminate process faild!
				LPVOID lpMsgBuf;
				FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);

				msg[2] = (LPCTSTR)lpMsgBuf;
				LogServiceEvent(SHELL_KILL_FAILED, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
				LocalFree( lpMsgBuf );
			}

			// make sure the process is killed
			int KillCheckTime = pSV->dwKillCheckTime;
			if(KillCheckTime < 1000)
			{ // minimal 1sec
				KillCheckTime = 1000;
			}
			int KillCheckCount = pSV->dwKillCheckCount;
			if(KillCheckCount < 1)
			{
				KillCheckCount = 1;
			}

			int iChk = 0;
			for(; iChk < KillCheckCount; ++iChk)
			{
				// the SCMgrSTOP event need manual reset, so the wait here is ok
				if (WAIT_OBJECT_0 == WaitForSingleObject(pSV->phHandleArray[SCMgrSTOP], KillCheckTime))
				{
					if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T( "Service is stopped by ServiceManager, stop checking the child process[%d]"), proc_info->dwProcessId);
					break;
				}

				DWORD exitCode = 0;
				if (!::GetExitCodeProcess(proc_info->hProcess, &exitCode))
				{
					if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T( "failed to get child process[%d] exit code with errno(%u)"), proc_info->dwProcessId, GetLastError());
					continue;
				}

				bool bExited = true;

				switch (exitCode)
				{
				case STILL_ACTIVE:
				case STATUS_WAIT_0:    
				case STATUS_ABANDONED_WAIT_0:   
				case STATUS_TIMEOUT:    
					// case STATUS_PENDING: == STILL_ACTIVE   
				case DBG_EXCEPTION_HANDLED:    
				case DBG_CONTINUE:    
					bExited = false;

				default: // leave bExited = true;
					break;
				}

				if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T( "terminate child process[%d] %s: exit[%d]"), proc_info->dwProcessId, (bExited?"succ":"fail"), exitCode);
				if (bExited)
					break;
			}

			if (KillCheckCount == iChk)
			{
				if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1,_T( "Child process didn't quit after %d*%d msec, give up checking."), KillCheckCount, KillCheckTime);
			}

		} 
		//end SPR 2665

	}//for (i=0; i<NTRIES; i++) 

	// If ServiceApp process still exists, then Log NT Reboot Needed Error Event
	if (ItsAlive)
	{
		msg[0] = pSV->szServiceName;
		_stprintf(buf, _T("%d (0x%x)"), proc_info->dwProcessId,proc_info->dwProcessId);
		msg[1] = buf;
		msg[2] = pSV->bRebootOnExit ? _T("Rebooting...") :	_T("Exiting...");
		LogServiceEvent(SHELL_NOKILL, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

		// If RebootOnExit is true, then reboot machine
		// Else ... the system is left in a bad state...
		// 	with the service_app running and the service_shell terminated
		if (pSV->bRebootOnExit) iRebootSts = reboot(pSV);
		IsSuccess = FALSE;

	}

	// If restarting the service_app, then
	if (restart) 
	{
		// If necessary, then ReAcquire ShellAlive Mutex
		if (MutexIsReleased)
		{
			for (i=0; i<NTRIES; i++) 
			{
				iSts = WaitForSingleObject(pSV->hMutexShellAlive, pSV->dwStopTimeout);
				if ((iSts == WAIT_OBJECT_0) || (iSts == WAIT_ABANDONED)) break;
				else Sleep (10*1000);
			}

			if ((iSts != WAIT_OBJECT_0) && (iSts != WAIT_ABANDONED))
			{
				msg[0] = pSV->szServiceName;
				_stprintf(buf, _T("%d (0x%x), Status = %d"), proc_info->dwProcessId,proc_info->dwProcessId, GetLastError());
				msg[1] = buf;
				msg[2] = pSV->bRebootOnExit ? _T("Rebooting...") : _T("Exiting...");
				LogServiceEvent(SHELL_REACQUIRE_SHELLALIVE_MUTEX, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
				if (pSV->bRebootOnExit) iRebootSts = reboot(pSV);
				IsSuccess = FALSE;
			}
		}
	}

	// If child process failed to terminate and reboot will not occur,
	// then log event - user intervention (REBOOT) required.
	if ((!IsSuccess) && (!iRebootSts))
	{
		msg[0] = pSV->szServiceName;
		LogServiceEvent(SHELL_HELPLESS_STATE, 1, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
	}

	return (IsSuccess);
}

/*
 ******************************************************************************
 *  service_init()
 *
 *  Description:
 *              Initialize the service shell.
 *
 *  Input:
 *              SV_t *pSV  -- ptr to ServiceVector
 *  Output:
 *
 *  Returns:
 *				NO_ERROR	-- success
 *				<other>		-- failure
 *
 *  Side Effects:
 *
 *  Assumptions:
 ******************************************************************************
 */
//modify by dony change all seachang WCHAR to TCHAR
int     service_init(SV_t *pSV)
{
	
	int iRet;
//	ZQSNMPSTATUS  manRet;
	DWORD   dwError = 0;
	HANDLE  hcfg;
	int     trial_limit, trial_interval;
	DWORD   cfg_value, cfg_size, cfg_type;
	TCHAR   cfg_string[256];
	LPCTSTR	msg[2];
	/* Following declarations added for T1.3-04 */
	TCHAR	streampath[] = _T("Software\\ZQ Interactive\\CDCI\\StreamTopology\\Downstream");
	time_t delta, reboot_thresh;
	DWORD  reboot_limit;
	struct	vstream_obj streamobj[1];
	HKEY	hremkey = NULL, htargkey = NULL;
	TCHAR	remotsys[MAX_COMPUTERNAME_LENGTH + 3];
	/* Added for T1.3-05 */
	struct	vstream_obj_info	stream_info;
	int 	i;
	TCHAR    buf[2*MAX_PATH+1]; // buffer room for WCHAR's too
	
	BOOL  bGetThresholdsFromUpstream = FALSE;
	TCHAR wszSysName[MAX_COMPUTERNAME_LENGTH + 3];
	
		_try
		{			// try-finally

			/////////////////////////////////////////////////////////////////////////////////////
			// Registry Service Control handler
			//
			// Set non-volatile SERVICE_STATUS members
			if (bIsServiceTypeShare)
				pSV->ssStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
			else 	
				pSV->ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

			pSV->ssStatus.dwServiceSpecificExitCode = 0;

			if (bIsServiceMode)
			{
				// Register the service control handler for our service (pSV->szServiceName)
				pSV->sshStatusHandle = RegisterServiceCtrlHandler(
						    		pSV->szServiceName,							// Service Name
						    		(LPHANDLER_FUNCTION)tbSVC[pSV->dwServiceIndex].SVCHandler); // Service Control Handler
				if (pSV->sshStatusHandle == (SERVICE_STATUS_HANDLE) NULL)
				{	    // If failure, then	Quit
					iRet =(CFGSTATUS) GetLastError();
					_leave;
				}
			}


			///////////////////////////////////////////////////////////////////////////
			// Retrieve possible registry parms for reboot_thresh & reboot_limit from upstream node
 			//  T1.3-04 Check the upstream node for the reboot count and time threshold
 			//  registry parameter values. If registry access failure, then use default values.
 			//  Start by reporting status to the service control manager.

  			if ((iRet =(CFGSTATUS) ReportStatusToSCMgr(pSV, SERVICE_START_PENDING, NO_ERROR,
  												pSV->dwCheckPoint++, 30000)) != NO_ERROR) _leave;

			// Retrieve system name	(global)
			
			DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;				// T1.3-05
			GetComputerName(wszSysName, &dwSize);	// T1.3-05

			// Set defaults values
			reboot_thresh = DEF_REBOOTTIMETHRESH;
			reboot_limit = DEF_REBOOTCOUNTLIMIT;
	/*
			// Determine Configuration SystemType, defaulting to CFG_UNKNOWN
	#ifndef _UNICODE
	   // MultiByteToWideChar(CP_ACP, 0, (char*)&sysname[2], -1, wszSysName, MAX_COMPUTERNAME_LENGTH + 3);
	#else
		#error We're not implemented for this!!!!
	#endif
	 */     
			//modify by dony change seachang WCHAR to TCHAR
			//wcscpy(stream_info.obj.obj_name, wszSysName); 
			_tcscpy(stream_info.obj.obj_name, wszSysName);
			stream_info.obj.obj_type = VSTREAM_SYSTEM;
			stream_info.sys_obj.no_netadapt = 0;
	//    Disabled 1/9/97 - davidr
	//		iRet = CFG_GET_STREAM_OBJ_INFO(&stream_info);
	//		if (iRet != CFG_SUCCESS) stream_info.sys_obj.systype = CFG_UNKNOWN;
	//  Always be unknown type, don't use system type to toggle behavior - 1/9/97 davidr
			stream_info.sys_obj.systype = CFG_UNKNOWN; 

			// Set default registry values
			pSV->dwStartTimeout= START_TIMEOUT;
			pSV->dwStopTimeout = STOP_TIMEOUT; 
 			pSV->dwAliveTimeout= ALIVE_TIMEOUT;
			pSV->dwFacilityMask = ZQShell << 16;
			pSV->dwAppShutdownWait = SHUTDOWN_WAIT;
			pSV->dwPauseTimeout = PAUSE_TIMEOUT;
			pSV->dwContinueTimeout = CONTINUE_TIMEOUT;
			pSV->bRebootOnExit = FALSE;
			pSV->dwRestartTries = 3;
			pSV->dwRestartInterval = 120;
			pSV->dwRestartDelay = RESTART_DELAY;
			//_stprintf(pSV->szLogfilePath, _T("c:\\ITV\\log\\%s_shell.log"), pSV->szServiceName);	
			_stprintf(pSV->szLogfilePath, _T("c:\\"));	
			pSV->dwLogfileSize = 500000;
			pSV->dwLoggingLevel = 10;
			pSV->dwLoggingMask = 0;					// "LoggingMask"
													// 0 - no logging
													// 1 - logging enabled  ...possible future expansion...
            pSV->hMClog = NULL;
            pSV->dwSnmpLoggingMask = 0;//for snmp
            _stprintf(pSV->szArgument, _T(""));	
            pSV->dwKillCheckTime = 5*1000; // 5s
            pSV->dwKillCheckCount = 600;

            //removed by xiaohui.chai
			// add by dony 20061201 for reset the current's oid
			//ResetServieCurrentOID(pSV->szServiceName,pSV->szProductName);
			// add by dony 20061201 for reset the current's oid

            // get service oid from register, SOFTWARE\\ZQ Interactive\\SNMPOID\\CurrentVersion\\Services\\
            
            hcfg = ShellCfgInitEx(pSV->szServiceName, &cfg_value, "SNMPOID");
            if (hcfg)
            {
                cfg_size = sizeof(int);

                iRet = ShellCfgGetValue(hcfg, _T("ServiceOID"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
                if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 0))
                    pSV->dwServiceOid = cfg_value;
            }

			// Initialize configuration package using Registry key "<service-name>_shell"
			_stprintf(buf, _T("%s_shell"), pSV->szServiceName);
			hcfg = ShellCfgInitEx(buf, &cfg_value, pSV->szProductName);
			
			if (hcfg)
			{
				  // Get the value GetThresholdsFromUpstream from local <servicename>_shell registry key
				  cfg_size = sizeof(int);
				  //modify by dony change seachang WCHAR to TCHAR
				  iRet = ShellCfgGetValue(hcfg, _T("GetThresholdsFromUpstream"), (BYTE *)&cfg_value, &cfg_size, &cfg_type);
				  if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 0))
					bGetThresholdsFromUpstream = TRUE;
			}
  			// If our bGetThresholdsFromUpstream set, try to retrieve configuration parameters from upstream registry.
			if (bGetThresholdsFromUpstream) 
			{
				_try
				{
					// Retrieve Upstream Topology objects from Registry
					// If failure, then Quit
					cfg_size = 1;
					iRet = CFG_GET_STREAM_TOPOLOGY(NULL, CFG_UPSTREAM, streamobj, &cfg_size);
					if ((iRet != CFG_SUCCESS) || (cfg_size == 0))
					{
						// Log NT Event: "Service shell %1 failed to get configuration values for RebootTimeThreshold/RebootCountLimit for SystemType %2, quitting"
						msg[0] = pSV->szServiceName;
						_stprintf(buf, _T("%d"), stream_info.sys_obj.systype);
						msg[1] = buf;
						_stprintf(&buf[16], _T("because failed to retrieve name of upstream node from registry -- cfg_status = %d"), iRet);
						msg[2] = &buf[16];
						LogServiceEvent(SHELL_ERROR_REBOOTPARMS, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
						iRet = ERROR_GEN_FAILURE;
						_leave;
					}

					// If there a configured upstream node,
					// Then try to open the remote registry (with 2 retries if RPC svr busy)
					_stprintf(remotsys, _T("\\\\%s"), streamobj[0].obj_name);
					for (i=0; i<10; i++) 
					{
						iRet = RegConnectRegistry(remotsys, HKEY_LOCAL_MACHINE, &hremkey);
						if (iRet == NO_ERROR) break;
						else if (iRet == RPC_S_SERVER_TOO_BUSY)
						{
							Sleep(2000);
					  		if ((iRet = ReportStatusToSCMgr(pSV, SERVICE_START_PENDING, NO_ERROR,
	  											pSV->dwCheckPoint++, 20000)) != NO_ERROR) _leave;
						} else break;
					}
					if (iRet != NO_ERROR)
					{
						// Log NT Event: "Service shell %1 failed to get configuration values for RebootTimeThreshold/RebootCountLimit for SystemType %2, quitting"
						msg[0] = pSV->szServiceName;
						_stprintf(buf, _T("%d"), stream_info.sys_obj.systype);
						msg[1] = buf;
						_stprintf(&buf[16], _T("because failed to connect to upstream registry (%s) -- NTStatus = %d"), remotsys, iRet);
						msg[2] = &buf[16];
						LogServiceEvent(SHELL_ERROR_REBOOTPARMS, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
						_leave;
					}

					// Open the target key, retrieving registry values
					if ((iRet = RegOpenKeyEx(hremkey, streampath, 0, KEY_READ, &htargkey)) != NO_ERROR) 
					{
						// Failed to open upstream registry key (\\remotesys\streampath) -- status = %s
						// Log NT Event: "Service shell %1 failed to get configuration values for RebootTimeThreshold/RebootCountLimit for SystemType %2, quitting"
						msg[0] = pSV->szServiceName;
						_stprintf(buf, _T("%d"), stream_info.sys_obj.systype);
						msg[1] = buf;
						_stprintf(&buf[16], _T("because failed to open upstream registry key (%s\\%s) -- NTStatus = %d"), remotsys, streampath, iRet);
						msg[2] = &buf[16];
						LogServiceEvent(SHELL_ERROR_REBOOTPARMS, 3, msg, 0, NULL, pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
						if (hremkey) RegCloseKey(hremkey);			// Close Registry handle
						_leave;
					}

					cfg_size = sizeof(reboot_thresh);
		  			if ((iRet = RegQueryValueEx(htargkey, _T("RebootTimeThreshold"),
		  						NULL, &cfg_type, (LPBYTE)&cfg_value, &cfg_size)) == NO_ERROR)
		  						reboot_thresh = cfg_value;

					cfg_size = sizeof(reboot_limit);
		  			if ((iRet = RegQueryValueEx(htargkey, _T("RebootCountLimit"),
		  						NULL, &cfg_type, (LPBYTE)&cfg_value, &cfg_size)) == NO_ERROR)
								reboot_limit = cfg_value;

					if (hremkey) RegCloseKey(hremkey);			// Close Registry handles
					if (htargkey) RegCloseKey(htargkey);
				}
				_finally 
				{
				}

			} // end if bGetThresholdsFromUpstream

			/* Set defaults for local values. */
			pSV->dwRebootCount = 1;
			time((time_t *)&pSV->dwLastReboot);

			/////////////////////////////////////////////////////////////////////////////////////////
			// Get configuration information

			// Report the status to the service control manager.
			  if ((iRet = ReportStatusToSCMgr(pSV, SERVICE_START_PENDING, NO_ERROR,
		  										pSV->dwCheckPoint++, 30000)) != NO_ERROR) _leave;
			
			if (hcfg != NULL) 
			{

				trial_limit = pSV->dwRestartTries;
				trial_interval = pSV->dwRestartInterval;

				cfg_size = sizeof(int);
				
				iRet = ShellCfgGetValue(hcfg, _T("RestartTries"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 0))
					trial_limit = cfg_value;

				cfg_size = sizeof(int);
				
				iRet = ShellCfgGetValue(hcfg, _T("RestartInterval"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 0))
					trial_interval = cfg_value;

				cfg_size = sizeof(cfg_value);
				
				iRet = ShellCfgGetValue(hcfg, _T("RestartDelay"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 0))
					pSV->dwRestartDelay = cfg_value;

				//X2.2-0 001 start
				cfg_size = sizeof( cfg_value );
				
				iRet = ShellCfgGetValue(hcfg, _T("AppShutdownWait"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 2999))
					pSV->dwAppShutdownWait = cfg_value;
				//X2.2-0 001 end

				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("StopTimeout"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 2999))
					pSV->dwStopTimeout = cfg_value;

				//X2.2-0 001 start
				/*
				*  Make sure the service shell waits at least 10 seconds longer to
				*  stop than the app shell waits to shutdown.
				*/
				if ((pSV->dwStopTimeout - pSV->dwAppShutdownWait) < MIN_DIFFERENCE)
					pSV->dwStopTimeout = pSV->dwAppShutdownWait + MIN_DIFFERENCE;
				//X2.2-0 001 end

				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("StartTimeout"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 2999))
					pSV->dwStartTimeout = cfg_value;

				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("AliveTimeout"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && (cfg_value > 9999))
					pSV->dwAliveTimeout = cfg_value;


				cfg_size = sizeof(pSV->szImagePath);
				BYTE  cTempBuf[(2*MAX_PATH)+2];
				iRet = ShellCfgGetValue(hcfg,_T("ImagePath"), cTempBuf, &cfg_size, &cfg_type);
				cTempBuf[(2*MAX_PATH)] = 0;
				cTempBuf[(2*MAX_PATH)+1] =0;
				if ( ( iRet == CFG_SUCCESS ) &&  ( cfg_type == REG_SZ  ))
					_tcscpy(pSV->szImagePath, reinterpret_cast<TCHAR *>(cTempBuf) );
				else
					_stprintf(pSV->szImagePath, _T("%s_app.exe"), pSV->szServiceName);

				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg, _T("RebootOnExit"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					pSV->bRebootOnExit = (cfg_value != 0);
				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg, _T("FacilityMask"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					pSV->dwFacilityMask = cfg_value;

				cfg_size = sizeof(tceventsink);    // 7-25-94 DWH - added to fix remote log bug
				if(ShellCfgGetParentValues(hcfg, _T("EventSink"), (BYTE*)cfg_string, &cfg_size, &cfg_type) == CFG_SUCCESS)
					if ((_tcslen(cfg_string)) && (_istalnum(cfg_string[0])))
						_stprintf(tceventsink, _T("\\\\%s"), cfg_string);

				// pSV->szLogfilePath. REG_SZ
				cfg_size = sizeof(pSV->szLogfilePath);
				iRet = ShellCfgGetValue(hcfg,_T("LogDir"), cTempBuf, &cfg_size, &cfg_type);
				cTempBuf[(MAX_PATH)] = 0;
				cTempBuf[(MAX_PATH)+1] =0;
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_SZ)) _tcscpy(pSV->szLogfilePath, reinterpret_cast<TCHAR *>(cTempBuf) );

				// pSV->dwLogfileSize. REG_DWORD
				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("LogfileSize"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					if ((cfg_value > 10000) && (cfg_value <= 10000000)) pSV->dwLogfileSize = cfg_value;


				// pSV->dwLoggingLevel. REG_DWORD
				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("LoggingLevel"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					pSV->dwLoggingLevel = cfg_value;

				// pSV->dwLoggingMask. REG_DWORD
				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("LoggingMask"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					pSV->dwLoggingMask = cfg_value;

				// pSV->dwSnmpLoggingMask. REG_DWORD
				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("SnmpLoggingMask"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					pSV->dwSnmpLoggingMask = cfg_value;

                // pSV->szArgument. REG_SZ
                pSV->szArgument[0] = 0; // reset to empty
                cfg_size = sizeof(pSV->szArgument);
                iRet = ShellCfgGetValue(hcfg,_T("Argument"), cTempBuf, &cfg_size, &cfg_type);
                if ((iRet == CFG_SUCCESS) && (cfg_type == REG_SZ))
                    _tcscpy(pSV->szArgument, (const TCHAR*)cTempBuf);

				// pSV->dwKillCheckTime. REG_DWORD
				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("KillCheckTime"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					pSV->dwKillCheckTime = cfg_value;

				// pSV->dwKillCheckCount. REG_DWORD
				cfg_size = sizeof(int);
				iRet = ShellCfgGetValue(hcfg,_T("KillCheckCount"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
					pSV->dwKillCheckCount = cfg_value;

				/*
				 * The next two values and associated logic relate to the reboot cycle checks
				 * for T1.3-04.
				 * For V3.0 Verify that registry value of pSV->dwLastReboot precedes current time.
				 *  cfg_value contains registry value of LastReboot
				 *  pSV->dwLastReboot contains Current Time
				 */
		 		cfg_size = sizeof(pSV->dwLastReboot);
				iRet = ShellCfgGetValue(hcfg,_T("LastReboot"), (BYTE*)&cfg_value, &cfg_size,	&cfg_type);
				if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD) && ((DWORD)cfg_value < pSV->dwLastReboot)) {
					pSV->dwLastReboot = cfg_value;

				/* Get the count as well - it should be there too. */
					cfg_size = sizeof(pSV->dwRebootCount);
					iRet = ShellCfgGetValue(hcfg,_T("RebootCount"), (BYTE*)&cfg_value, &cfg_size, &cfg_type);
					if ((iRet == CFG_SUCCESS) && (cfg_type == REG_DWORD))
						pSV->dwRebootCount = cfg_value + 1;
				}

				/*
				 * If we're beyond the reboot time threshold, then
				 *  Reset pSV->dwRebootCount to 1 and pSV->dwLastReboot time to now.
				 */
				time(&delta);
				if ((delta - (int)pSV->dwLastReboot) > (int)reboot_thresh) {
					pSV->dwRebootCount = 1;
					pSV->dwLastReboot = delta;
				}
				delta = delta - pSV->dwLastReboot;

				/* Write new values back in registry. */
				iRet = ShellCfgSetValue(hcfg, _T("LastReboot"), (BYTE*) &pSV->dwLastReboot,
	      										 sizeof(pSV->dwLastReboot), REG_DWORD);

				iRet = ShellCfgSetValue(hcfg, _T("RebootCount"), (BYTE*) &pSV->dwRebootCount,
	      										 sizeof(pSV->dwRebootCount), REG_DWORD);

				/* End of changes for T1.3-04 */
				CFG_TERM(hcfg);

				/*
				 * Sanity check values: make sure we have at least a minute between restarts.
				 */
                //modified by xiaohui.chai
//				if ((trial_interval / trial_limit) >= 60) {
					pSV->dwRestartTries = trial_limit;
					pSV->dwRestartInterval = trial_interval;
//				}

			} 
			else
			{	// Config_ini failed
				iRet = ERROR_GEN_FAILURE;
				_leave;
			}
			/* The following check is for T1.3-04: Test whether or not to re-start. */
			if ((delta <= reboot_thresh) && (pSV->dwRebootCount > reboot_limit))
			{
				// Too many reboots within time threshold
				msg[0] = pSV->szServiceName;
				_stprintf(buf, _T("%d"), pSV->dwRebootCount);
				msg[1] = buf;
				_stprintf(&buf[16], _T("%d"), delta);
				msg[2] = &buf[16];
				LogServiceEvent(SHELL_TOO_MANY_REBOOTS, 3, msg, 0, NULL,
									pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
				iRet = ERROR_GEN_FAILURE;
				_leave;
			}

			if (pSV->dwLoggingMask)
			{
				TCHAR szLogFile[MAX_PATH+1];
				_tcscpy(szLogFile, pSV->szLogfilePath);
				
				// append the log name
				{
					TCHAR* pPtr = szLogFile + _tcslen(szLogFile);
					while(pPtr>szLogFile && *(pPtr - 1)==_T(' ')) pPtr--;
					
					if (pPtr == szLogFile || *(pPtr - 1)==_T('\\')) 
						_stprintf(pPtr, _T("%s_shell.log"), pSV->szServiceName);
					else
						_stprintf(pPtr, _T("\\%s_shell.log"), pSV->szServiceName);
				}

				if ((pSV->hMClog = MClogInit(szLogFile, pSV->dwLoggingLevel, pSV->dwLogfileSize)) == NULL)
					pSV->dwLoggingMask = 0; 
			}
				
			// Initialize Management
			//
			// Report status to Service Control Manager
			if ((iRet = ReportStatusToSCMgr(pSV, SERVICE_START_PENDING, NO_ERROR, pSV->dwCheckPoint++, 10000)) != NO_ERROR) _leave;

			/*
			// for the test 20070419 for check the 写Log是否有延迟时间,经测试发现，写Log时没有时间的延迟问题
			for ( int  m  =0; m < 2000; m ++)
			{
				MClog(pSV->hMClog, 1, _T("LogSaveTest%d"),(m+1));
			}
			// for the test 20070419 for check the 写Log是否有延迟时间
			*/


			_stprintf(buf, _T("%s"), pSV->szServiceName);
			
			//_stprintf(buf, _T("%s_shell"), pSV->szServiceName); // 先不管为什么要加_shell,同一注册到service服务中,modify by dony 20061201
			//////////////////////////////////////////////////////////////////////////
			//snmp
            {
                TCHAR *snmpLogFile = NULL;
				TCHAR snmpLogFileBuf[MAX_PATH + 1];
				// append the log name
				_tcscpy(snmpLogFileBuf, pSV->szLogfilePath);
				TCHAR* pPtr = snmpLogFileBuf + _tcslen(snmpLogFileBuf);
				while(pPtr>snmpLogFileBuf && *(pPtr - 1)==_T(' ')) pPtr--;

				if (pPtr == snmpLogFileBuf || *(pPtr - 1)==_T('\\')) 
					_stprintf(pPtr, _T("%s_shell.log"), pSV->szServiceName);
				else
					_stprintf(pPtr, _T("\\%s_shell.log"), pSV->szServiceName);

				snmpLogFile = snmpLogFileBuf;

				int nLogFileSize = 0xA00000;
				DWORD nProcessInstanceId = pSV->dwInstanceId;

                registerSnmp(snmpLogFile, nLogFileSize, pSV);

            }//snmp
            
			////////////////////////////////////////
			// Initialize logging
			//
			/*
			 * Report Service status to Service Control Manager.
			 */
			if ((iRet = ReportStatusToSCMgr(pSV, 
					SERVICE_START_PENDING, // service state
					NO_ERROR,              // exit code
					pSV->dwCheckPoint++,          // pSV->dwCheckPoint
					10000)) != NO_ERROR) _leave;	// Quit if failure

			/* 
			if (pSV->dwLoggingMask)
				if ((pSV->hMClog = MClogInit(pSV->szLogfilePath, pSV->dwLoggingLevel, pSV->dwLogfileSize)) == NULL)
					pSV->dwLoggingMask = 0; 
			*/

			if (pSV->dwLoggingMask) MClog(pSV->hMClog, 1, _T("Starting Service %s, pid=%u"), pSV->szServiceName, GetCurrentProcessId());

					
			///////////////////////////////////////////////////////////////////////////////////
			// Create the event objects:

			/*
			 * Report Service status to Service Control Manager.
			 */
			if ((iRet = ReportStatusToSCMgr(pSV, 
					SERVICE_START_PENDING, // service state
					NO_ERROR,              // exit code
					pSV->dwCheckPoint++,          // pSV->dwCheckPoint
					10000)) != NO_ERROR) _leave;	// Quit if failure


			// Create <service-name>_Stop Event
			_stprintf(buf, _T("%s_Stop"), pSV->szServiceName);
 			pSV->AppStop = CreateEvent(
						NULL,           // No security attributes
						TRUE,           // manually reset event
						FALSE,          // not-signalled
						buf);           // name so child can find it

  			if (pSV->AppStop == NULL) 
			{
				iRet = GetLastError();
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create Stop Event failed") );
				}
				_leave;					// Quit if failure
			}
			// Create event for SCMgr STOP request
 			pSV->phHandleArray[SCMgrSTOP] = CreateEvent(
						NULL,           // No security attributes
						TRUE,           // manually reset event
						FALSE,          // not-signalled
						NULL);

  			if (pSV->phHandleArray[SCMgrSTOP] == (HANDLE)NULL)
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create SCMgrSTOP Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}

			// we may have only opened it. Reset it just to be sure
			// we only do this once.  Once set, the service must Stop
			// A Shared EXE service may re-use it, so we take care to reset it
			ResetEvent(pSV->phHandleArray[SCMgrSTOP]);

			// Create <service-name>_Alive Event
			_stprintf(buf, _T("%s_Alive"), pSV->szServiceName);
  			pSV->phHandleArray[ChildAlive] = CreateEvent(NULL, TRUE, FALSE, buf);

  			if (pSV->phHandleArray[ChildAlive] == (HANDLE)NULL) 
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create Alive Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}

			// X2.2-0 002 Add, 5/3/95, MMS
			// Create <service-name>_Shutdown Event
			_stprintf(buf, _T("%s_Shutdown"), pSV->szServiceName);
			pSV->phHandleArray[AppShutdown] = CreateEvent(NULL, TRUE, FALSE, buf);

			if (pSV->phHandleArray[AppShutdown] == (HANDLE)NULL) 
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create AppShutdown Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}
			// End X2.2-0 002 add

			//X2.2-0 001 start
			// Create <service-name>_Pause Event
			_stprintf( buf, _T("%s_Pause"), pSV->szServiceName );
			pSV->HSCMgrPAUSE = CreateEvent(NULL, FALSE, FALSE, buf);
			if (pSV->HSCMgrPAUSE == (HANDLE)NULL)
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create Servie_Pause Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}

			// Create <service-name>_Continue Event
			_stprintf( buf, _T("%s_Continue"), pSV->szServiceName );
			pSV->HSCMgrContinue = CreateEvent(NULL, FALSE, FALSE, buf);
			if (pSV->HSCMgrContinue == (HANDLE)NULL)
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create Service_Continue Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}

			// Create <service-name>_Paused Event
			_stprintf( buf, _T("%s_Paused"), pSV->szServiceName );
	 		pSV->phHandleArray[ SCMgrPAUSED ] = CreateEvent(NULL, FALSE, FALSE, buf);
			if (pSV->phHandleArray[ SCMgrPAUSED ] == (HANDLE)NULL) 
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create Service_Paused Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}

			// Create <service-name>_Continued Event
			_stprintf( buf, _T("%s_Continued"), pSV->szServiceName );
	 		pSV->phHandleArray[ SCMgrCONTINUED ] = CreateEvent(NULL, FALSE, FALSE, buf);
			if (pSV->phHandleArray[ SCMgrCONTINUED ] == (HANDLE)NULL)
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create Service_Continued Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}
			// X2.2-0 001 end

			// Create <service-name>_ShellAlive Mutex
			// with service_thread as initial owner.
			// The mutex is released when the shell process is terminated
			_stprintf(buf, _T("%s_ShellAlive"), pSV->szServiceName);
			pSV->hMutexShellAlive = CreateMutex(NULL, TRUE, buf);
			if (pSV->hMutexShellAlive == (HANDLE)NULL) 
			{
				if (pSV->dwLoggingMask) 
				{
					MClog(pSV->hMClog, 1, _T("Create Service_ShellAlive Event failed") );
				}
				iRet = GetLastError();
				_leave;					// Quit if failure
			}

			// Successful initialization
			iRet = NO_ERROR;

		} 
		_finally
		{
			ReportStatusToSCMgr(pSV, SERVICE_START_PENDING, NO_ERROR, pSV->dwCheckPoint++, 10000);
		}
		return (iRet);
}

/*
 ******************************************************************************
 *  cleanup(pSV, )
 *
 *  Description:
 *              Close handles and call ManCloseSession in preparation for process exit.
 *
 *  Input:
 *              app             - name of the service shell instance
 *
 *  Output:
 *
 *  Returns:
 *
 *  Side Effects:
 *
 *  Assumptions:
 ******************************************************************************
 */
VOID cleanup(SV_t *pSV, TCHAR *app, PROCESS_INFORMATION *proc_info)
{
	LPCTSTR msg[2];
	TCHAR buf[2][32];
//	DWORD dwError;

	if (pSV->phHandleArray[SCMgrSTOP] != (HANDLE)NULL)
		if (!CloseHandle(pSV->phHandleArray[SCMgrSTOP]))
		{
				_stprintf(buf[0], _T("stop event. Child PId=%u"), proc_info->dwProcessId);
				msg[0] = buf[0];
				_stprintf(buf[1], _T("%d"), GetLastError());
				msg[1] = buf[1];
				LogServiceEvent(SHELL_CLOSE_ERR, 2, msg, 0, NULL,
												pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
		}

    if (pSV->phHandleArray[ChildAlive] != (HANDLE)NULL)
		if (!CloseHandle(pSV->phHandleArray[ChildAlive])) 
		{
				_stprintf(buf[0], _T("alive event. Child PId=%u"), proc_info->dwProcessId);
				msg[0] = buf[0];
				_stprintf(buf[1], _T("%d"), GetLastError());
				msg[1] = buf[1];
				LogServiceEvent(SHELL_CLOSE_ERR, 2, msg, 0, NULL,
												pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
		}

//X2.2-0 001 start
	if (pSV->HSCMgrPAUSE != (HANDLE)NULL)
		if (!CloseHandle( pSV->HSCMgrPAUSE ))
		{
			_stprintf( buf[0], _T("pause event. Child PId=%u"), proc_info->dwProcessId );
			msg[0] = buf[0];
			_stprintf( buf[1], _T("%d"), GetLastError() );
			msg[1] = buf[1];
			LogServiceEvent( SHELL_CLOSE_ERR, 2, msg, 0, NULL,
							 pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname );
		}


	if (pSV->HSCMgrContinue != (HANDLE)NULL)
		if (!CloseHandle( pSV->HSCMgrContinue ))
		{
			_stprintf( buf[0], _T("continue event. Child PId=%u"), proc_info->dwProcessId );
			msg[0] = buf[0];
			_stprintf( buf[1], _T("%d"), GetLastError() );
			msg[1] = buf[1];
			LogServiceEvent( SHELL_CLOSE_ERR, 2, msg, 0, NULL,
							 pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname );
		}


	if (pSV->phHandleArray[ SCMgrPAUSED ] != (HANDLE)NULL)
		if (!CloseHandle( pSV->phHandleArray[ SCMgrPAUSED ] ))
		{
			_stprintf( buf[0], _T("paused event. Child PId=%u"), proc_info->dwProcessId );
			msg[0] = buf[0];
			_stprintf( buf[1], _T("%d"), GetLastError() );
			msg[1] = buf[1];
			LogServiceEvent( SHELL_CLOSE_ERR, 2, msg, 0, NULL,
							 pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname );
		}


	if (pSV->phHandleArray[ SCMgrCONTINUED ] != (HANDLE)NULL)
		if (!CloseHandle( pSV->phHandleArray[ SCMgrCONTINUED ] ))
		{
			_stprintf( buf[0], _T("continued event. Child PId=%u"), proc_info->dwProcessId );
			msg[0] = buf[0];
			_stprintf( buf[1], _T("%d"), GetLastError() );
			msg[1] = buf[1];
			LogServiceEvent( SHELL_CLOSE_ERR, 2, msg, 0, NULL,
							 pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname );
		}

	if (pSV->phHandleArray[ AppShutdown] != (HANDLE)NULL)
		if (!CloseHandle( pSV->phHandleArray[ AppShutdown ] ))
		{
			_stprintf( buf[0], _T("shutdown event. Child PId=%u"), proc_info->dwProcessId );
			msg[0] = buf[0];
			_stprintf( buf[1], _T("%d"), GetLastError() );
			msg[1] = buf[1];
			LogServiceEvent( SHELL_CLOSE_ERR, 2, msg, 0, NULL,
							 pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname );
		}


    // Release ShellAlive Mutex
 	if (pSV->hMutexShellAlive != (HANDLE)NULL) 
	{
		ReleaseMutex(pSV->hMutexShellAlive);
		CloseHandle(pSV->hMutexShellAlive); 
		}

//X2.2-0 001 end

//   ManCloseSession(pSV->hManSession, &dwError); ////modify by dony remark all about the manpkg.lib's api 

    return;
}
/*
 ******************************************************************************
 *  reboot(pSV)
 *
 *  Description:
 *              Try to initiate a system shutdown.
 *
 *  Input:
 *
 *  Output:
 *
 *  Returns:
 *
 *  Side Effects:
 *
 *  Assumptions:
 ******************************************************************************
 */
BOOL reboot(SV_t *pSV)
{
	HANDLE  hToken;              /* handle to process token */
	TOKEN_PRIVILEGES tkp;        /* ptr. to token structure */
	LPCTSTR msg[2];
	BOOL 	iRet = FALSE;
	TCHAR    buf[256];

	//To workaround Microsoft bug check windows NT version(SP5) and datetime(Daylight saving end
	// last Sunday of October 1am-2am after time switch back)

	DWORD   dwMin = 0;
	OSVERSIONINFO vers;
	vers.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   if (CheckDSTEnd(&dwMin))
   {
       //Yes, check WinNT version
       if (!GetVersionEx(&vers))
       {
            _stprintf(buf, _T("%d"), GetLastError());
	        _tcscpy(&buf[16],_T( "Error in GetVersionEx ") );
		    msg[0] = &buf[16];
		    msg[1] = buf;
            LogServiceEvent(SHELL_NOSHUTDOWN, 2, msg, 0, NULL,
							pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
            return (iRet);
       }
       if (vers.dwMajorVersion == 3 && 
           vers.dwMinorVersion == 51 &&
           (_tcsstr(vers.szCSDVersion,_T("Service Pack 5") )!=NULL))
       {
           _stprintf(buf, _T("Will reboot in %d minutes."), (65-dwMin));
           msg[0] = buf;
           LogServiceEvent(SHELL_REBOOT_DELAY, 1, msg, 0, NULL,
						   pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
           //don't reboot machine within this critical hour
           Sleep((65-dwMin)*60*1000);
           LogServiceEvent(SHELL_REBOOTING, 0, msg, 0, NULL,
						   pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
        }
        else
        {
           LogServiceEvent(SHELL_REBOOTING, 0, msg, 0, NULL,
						   pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
        }
   }
   else
   {
      LogServiceEvent(SHELL_REBOOTING, 0, msg, 0, NULL,
						   pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
   }
//end workaround

/*
 * Get the current process token handle
 * so we can get shutdown privilege.
 */
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
	{
		_stprintf(buf, _T("%d"), GetLastError());
		_tcscpy(&buf[16], _T("OpenProcessToken") );
		msg[0] = &buf[16];
		msg[1] = buf;
		LogServiceEvent(SHELL_NOSHUTDOWN, 2, msg, 0, NULL,
										pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);

	/* Get the LUID for shutdown privilege. */
	} 
	else
	{

		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

		tkp.PrivilegeCount = 1;  /* one privilege to set    */
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		/* Get shutdown privilege for this process. */
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

		/* Cannot test the return value of AdjustTokenPrivileges. */
		if (GetLastError() != ERROR_SUCCESS) 
		{
			_stprintf(buf, _T("%d"), GetLastError());
			_tcscpy(&buf[16], _T("AdjustTokenPrivileges"));
			msg[0] = &buf[16];
			msg[1] = buf;
			LogServiceEvent(SHELL_NOSHUTDOWN, 2, msg, 0, NULL,
											pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
		} 
		else
		{
			// Initialize System Shutdown on local machine
			// Force all applications and services to shutdown immediately
			if (!InitiateSystemShutdown(NULL, NULL, 0, TRUE, TRUE)) 
			{
				_stprintf(buf, _T("%d"), GetLastError());
				_tcscpy(&buf[16], _T("InitiateSystemShutdown"));
				msg[0] = &buf[16];
				msg[1] = buf;
				LogServiceEvent(SHELL_NOSHUTDOWN, 2, msg, 0, NULL,
												pSV->szServiceName, pSV->dwFacilityMask, tceventsink, tcsysname);
			} 
			else
			{
				/* Disable shutdown privilege. */
				tkp.Privileges[0].Attributes = 0;
				AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);
				iRet = TRUE;
			}
		}
	}
	return (iRet);
}


/*
*  Type definitions
*/
typedef struct _EXCEPTION_BUFFER
{
	EXCEPTION_RECORD  Exception;
	CONTEXT           Context;
} EXCEPTION_BUFFER;


/*
*  Externals
*/
//extern DWORD   fac_mask;
//extern char    servname[ 64 ];
//extern char    sysname[ MAX_COMPUTERNAME_LENGTH + 3 ];
//extern char    eventsink[ MAX_COMPUTERNAME_LENGTH + 3 ];
//X2.2-0 001 end


/*
 ******************************************************************************
 *  LogServiceEvent()
 *
 *  Description:
 * 		Try to initiate a system shutdown.
 *
 *  Input:
 *
 *  Output:
 *
 *  Returns:
 *
 *  Side Effects:
 *
 *  Assumptions:
  *
 ******************************************************************************
 */
VOID
LogServiceEvent(DWORD eventID, WORD strcnt, LPCTSTR *strarray, DWORD bdatacnt,
	LPVOID bdata, TCHAR *servname, DWORD fac_mask, TCHAR *eventsink, TCHAR *sysname)
{
	HANDLE      hEventSource;
	WORD		type;
	TCHAR		buf[MAXOBJNAMELEN];

	/* Register ourselves as the app's shell. Always log locally. */
    _stprintf(buf, _T("%s_shell"), servname);
	hEventSource = RegisterEventSource(NULL, buf);
	
	/* 
	 * Switch facility codes to "impersonate" the service we're monitoring.
	 * Moved out of hEventSource conditional for V2.0.
	 */
	eventID &= 0xf000ffff;
    eventID |= fac_mask;

    /* If we don't have an eventlog handle, we can't log locally. */  		
	if(hEventSource != NULL)
	{

		/* Calculate type based on eventid */
	    type = (WORD)(eventID >> 30) & 0x3;
		switch (type) 
		{
		    case 1:				 											/* Map 01 to INFORMATION (04)*/
				type = EVENTLOG_INFORMATION_TYPE;
				break;
			case 2:															/* Map 10 to Warning (02)*/
				type = EVENTLOG_WARNING_TYPE;
				break;
			default:														/* Map anything else to ERROR */
				type = EVENTLOG_ERROR_TYPE;
				break;
		}

		/* Log the event. */
		ReportEvent(hEventSource, // handle of event source
				    type,				  // event type
				    0,                    // event category
				    eventID,              // event ID
				    NULL,                 // current user's SID
				    strcnt,               // strings in lpszStrings
				    bdatacnt,             // no. of bytes of raw data
				    strarray, 		        // array of error strings
				    bdata);               // raw data

		(VOID) DeregisterEventSource(hEventSource);
    }
	return;
}


//X2.2-0 001 start
/***************************************************************************
*
* PROCEDURE:
*
*	LogServiceException
*
*
* DESCRIPTION:
*
*	This routine logs the contents of an exception record in the application
*	log.
*
*
* ARGUMENTS:
*
*	Event
*
*		The event id to use when logging the exception information.
*
*	XPtr
*
*		The address of the exception pointer structure that contains the
*		exception handling chain.
*
*
* RETURNS:
*
*	none
*
*
* SIDE EFFECTS:
*
*	Logs exception information in the application log.
*
*
***************************************************************************/

void LogServiceException( DWORD Event, EXCEPTION_POINTERS *XPtr )
{
    EXCEPTION_BUFFER	XBuf;
    TCHAR               msgbuf[ 2 ][ 128 ];
    LPCTSTR             msglist[ 2 ];
    SV_t *pSV;
    TCHAR *pszSName;
    DWORD dwFacMask;

	/*
	*  Extract the exception code and address from the record.
	*/
	if (XPtr->ExceptionRecord != NULL )
	{
		/*
		*  We have a valid pointer to the first exception record.  Get
		*  the exception code and address (PC).
		*/
		_stprintf( &msgbuf[ 0 ][ 0 ], _T("0x%08X"), XPtr->ExceptionRecord->ExceptionCode );
		_stprintf( &msgbuf[ 1 ][ 0 ], _T("0x%08X"), XPtr->ExceptionRecord->ExceptionAddress );
		msglist[ 0 ] = &msgbuf[ 0 ][ 0 ];
		msglist[ 1 ] = &msgbuf[ 1 ][ 0 ];


		/*
		*  Put the first exception record and the context record into a
		*  buffer.
		*/
		XBuf.Exception = *XPtr->ExceptionRecord;
		if (XPtr->ContextRecord != NULL)
			XBuf.Context = *XPtr->ContextRecord;
		else
			memset( &XBuf.Context, '?', sizeof( XBuf.Context ) );
	}
	else
	{
		/*
		*  We have a NULL pointer to the exception record.  We probably
		*  couldn't allocate enough memory in the CopyExceptionInformation()
		*  routine to copy the exception chain.
		*/
		msglist[ 0 ] = _T("unavailable (NULL Exception Record Pointer)");
		msglist[ 1 ] = _T("unavailable (NULL Exception Record Pointer)");
		memset( &XBuf, '?', sizeof( XBuf ) );
	}


	/*
	*  Log the Application Shell exception event.
	*/
    pSV = (SV_t*)TlsGetValue(dwTlspSV);  // Retrieve ServiceVector pointer
    if (pSV) {
        pszSName = pSV->szServiceName;
        dwFacMask = pSV->dwFacilityMask;
    } else {
        pszSName = _T("Service");
        dwFacMask = ZQShell << 16;
    }
    
	LogServiceEvent(	Event,
						2,
						msglist,
						sizeof( XBuf ),
						&XBuf,
						pszSName,
						dwFacMask,
						tceventsink,
						tcsysname );
}


/***************************************************************************
*
* PROCEDURE:
*
*	CopyExceptionInformation
*
*
* DESCRIPTION:
*
*	This routine copies an entire exception handling chain from one
*	exception pointer to another.
*
*	This routine must be kept as simple as possible to avoid generating
*	exceptions.  Generating exceptions in this routine may force an infinite
*	loop in other parts of the module (via cascading exceptions).  Therefore,
*	this routine does not do any event logging if it can't allocate enough
*	memory for the exception chain.  Instead, when the exception is logged,
*	the logging routine will detect that it doesn't have any exception
*	records to look at and will log an event stating so.
*
*
* ARGUMENTS:
*
*	To
*
*		The address of the exception pointer structure that recevies the
*		copied exception chain.
*
*	From
*
*		The address of the exception pointer containing containing the
*		exception chain to be copied.
*
*
* RETURNS:
*
*	EXCEPTION_EXECUTE_HANDLER
*	EXCEPTION_CONTINUE_SEARCH
*
*
* SIDE EFFECTS:
*
*	none
*
*
***************************************************************************/

int CopyExceptionInformation(	EXCEPTION_POINTERS *To,
								EXCEPTION_POINTERS *From )
{
EXCEPTION_RECORD *eto, *efrom, *eold;
CONTEXT          *cto, *cfrom;


	/*
	*  Initialize
	*/
	efrom = From->ExceptionRecord;
	cfrom = From->ContextRecord;


	/*
	*  Copy the first exception record.
	*/
	if (efrom != NULL)
	{
		/*
		*  The record exists in the original copy, so attempt to allocate
		*  memory and copy it.
		*/
		eto = (EXCEPTION_RECORD *)malloc( sizeof( EXCEPTION_RECORD ) );
		if (eto != NULL)
			*eto = *efrom;
	}
	else
		eto = NULL;


	/*
	*  Copy the context record.
	*/
	if (cfrom != NULL)
	{
		/*
		*  The record exists in the original copy, so attempt to allocate
		*  memory and copy it.
		*/
		cto = (CONTEXT *) malloc( sizeof( CONTEXT ) );
		if (cto != NULL)
			*cto = *cfrom;
	}
	else
		cto = NULL;


	/*
	*  Fill in the record pointers in the pointer structure.
	*/
	To->ExceptionRecord = eto;
	To->ContextRecord = cto;


	/*
	*  Copy the rest of the exception records (if any).
	*/
	eold = eto;
	efrom = efrom->ExceptionRecord;
	while ((efrom != NULL) && (eold != NULL))
	{
		/*
		*  Found another exception record.  Attempt to allocate memory
		*  and copy it.
		*/
		eto = ( EXCEPTION_RECORD *)malloc( sizeof( EXCEPTION_RECORD ) );
		if (eto != NULL)
			*eto = *efrom;


		/*
		*  Add this exception record to the list.
		*/
		eold->ExceptionRecord = eto;


		/*
		*  Move to the next record.
		*/
		eold = eto;
		efrom = efrom->ExceptionRecord;
	}


	/*
	*  Execute the handler.
	*/
#ifdef DEBUG_EXCEPTION
	return( EXCEPTION_CONTINUE_SEARCH );
#else
	return( EXCEPTION_EXECUTE_HANDLER );
#endif /* DEBUG_EXCEPTION */
}


/***************************************************************************
*
* PROCEDURE:
*
*	FreeExceptionInformation
*
*
* DESCRIPTION:
*
*	This routine frees a copied version of an exception handling chain.
*
*
* ARGUMENTS:
*
*	XPtr
*
*		The address of the exception pointer structure that contains the
*		exception handling chain.
*
*
* RETURNS:
*
*	none
*
*
* SIDE EFFECTS:
*
*	none
*
*
***************************************************************************/

void FreeExceptionInformation( EXCEPTION_POINTERS *XPtr )
{
	EXCEPTION_RECORD  *erecord, *enext;


	/*
	*  Initialize
	*/
	erecord = XPtr->ExceptionRecord;


	/*
	*  Free the context record if it exists.
	*/
	if (XPtr->ContextRecord != NULL)
		free( XPtr->ContextRecord );


	/*
	*  Free all of the exception records (if any).
	*/
	while (erecord != NULL)
	{
		/*
		*  Get a pointer to the next exception record if any.
		*/
		enext = erecord->ExceptionRecord;


		/*
		*  Free this exception record.
		*/
		free( erecord );


		/*
		*  Move on to the next record.
		*/
		erecord = enext;
	}
}
//X2.2-0 001 end

/****************************************************************************
CheckDefaultDSTEnd() --
  this function returns TRUE if current time is within "critical" hour - 
  one hour after switch from Daylight Saving time to Standard time.
  By default Daylight Saving time ends at 2:00am last Sunday of October
  (Eastern Standard Time).
  This function is called only for unknown time zone.
*****************************************************************************/
BOOL CheckDefaultDSTEnd(DWORD* pdwMin)
{
    struct  tm *currenttime;
    time_t  long_time;
    int     yr, mdays, lastsun;
    int     begin_epoch = 4; //Jan 1, 1970 Thursday(4) - beginning of the epoch
    int     days_in_month[] = {
                  -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};
    
    *pdwMin = 0;
    time( &long_time );                    // Get time as long integer.
    currenttime = localtime( &long_time ); // Convert to local time.

    //if the month is not October it is not DSTEnd
    if(currenttime->tm_mon != 9) return FALSE;

    //determine the year-day of last Sunday of October for current year
    yr = currenttime->tm_year + 1900;
    mdays = days_in_month[currenttime->tm_mon + 1];
    if (!(yr & 3)) mdays++;  //if this is a leap-year, add an extra day
    
    yr = currenttime->tm_year - 70;
    lastsun = mdays - ((mdays + 365*yr + ((yr+1)/4) + begin_epoch) % 7);
    //is it last Sunday of October and that critical hour?
    if (currenttime->tm_yday == lastsun && currenttime->tm_hour == 1)
    {
        if (currenttime->tm_isdst == 0) 
        {
            *pdwMin = currenttime->tm_min;  //set number of minute
            return TRUE;
        }
    }
    return FALSE;
}

/****************************************************************************
CheckDSTEnd() --
  this function returns TRUE if current time is within "critical" hour - 
  one hour after switch from Daylight Saving time to Standard time.
*****************************************************************************/
BOOL CheckDSTEnd(DWORD* pdwMin)
{
    TIME_ZONE_INFORMATION  TimeZoneInformation;
	DWORD                  dwRet;
    time_t                 long_time;
	struct tm*             currenttime;
 
	*pdwMin = 0;
	dwRet =	GetTimeZoneInformation(&TimeZoneInformation);
	if (dwRet == TIME_ZONE_ID_UNKNOWN)
        return CheckDefaultDSTEnd(pdwMin);
    if (dwRet == TIME_ZONE_ID_DAYLIGHT)	return FALSE; 
 	if (TimeZoneInformation.StandardDate.wMonth == 0) return FALSE; 

    time(&long_time);                     
    currenttime = localtime(&long_time);

	if (TimeZoneInformation.StandardDate.wMonth != currenttime->tm_mon+1) 
		return FALSE; 

	if (TimeZoneInformation.StandardDate.wYear == 0) //day-in-month format
    {
	    if (TimeZoneInformation.StandardDate.wDayOfWeek != currenttime->tm_wday)
		    return FALSE; 
        if (TimeZoneInformation.StandardDate.wDay != GetInstanceOfDay(currenttime))
		    return FALSE; 
	}
	else //absolute format
    {
	   if (TimeZoneInformation.StandardDate.wDay != currenttime->tm_mday)
		   return FALSE; 
    }
    if (TimeZoneInformation.StandardDate.wHour - 1 != currenttime->tm_hour) 
		return FALSE; 

    //Probably this is critical hour
    if (currenttime->tm_isdst == 0)
    {
        //Yes! DST is not in effect anymore!!!
        *pdwMin = currenttime->tm_min;  //set number of minute
        return TRUE;
    }
    //No, DST is still in effect!!!
    return FALSE;
}

/****************************************************************************
GetInstanceOfDay() --
  this function calculates instance of the day in the month.
  If this is last instance in the month set it to 5.
*****************************************************************************/
WORD GetInstanceOfDay(struct tm *currenttime)
{
    int  nweek; 
    int  begin_epoch = 4; //Jan 1, 1970 Thursday(4) - beginning of the epoch
	int  dow_som; //the day of the week of the start of the month.
	long lLeapYearAdjust = 17L; //Leap years 1900 - 1970
    int  days_in_month[] = {
               -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};

	dow_som=((currenttime->tm_yday+1-(currenttime->tm_mday)) + ((currenttime->tm_year - 70) * 365) +
		 ((currenttime->tm_year - 1) >> 2) - lLeapYearAdjust + begin_epoch) % 7;
	nweek=(currenttime->tm_mday - (currenttime->tm_wday - dow_som))/7;
	if (dow_som <= currenttime->tm_wday)
       nweek = nweek + 1;
    //check if this last instance of day in month and set instance to 5
	if ((currenttime->tm_yday + 7) > 
		(((((currenttime->tm_mon+1) > 1)&&(!(currenttime->tm_year & 3))) ? 1 : 0) +
		 days_in_month[currenttime->tm_mon + 1]))
        return 5; //last instance of day in month
    return nweek;
}

BOOL  IsInstalled(TCHAR *pstrServiceName)
{
	BOOL bResult = FALSE;

	//打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
		//打开服务
        SC_HANDLE hService = ::OpenService(hSCM,pstrServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

void  AddService(TCHAR *pszServiceName,TCHAR *pszProduceName,TCHAR *pszImageFileName,TCHAR *szServiceOID,TCHAR *pszAccount,TCHAR *pszPassword,TCHAR *szEventDllName,TCHAR *szStartType,BOOL bCmdType)
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;
	DWORD  dwType;

	schSCManager = ::OpenSCManager(
						    NULL,                   // machine (NULL == local)
							NULL,                   // database (NULL == default)
							SC_MANAGER_ALL_ACCESS   // access required
					    );
	if ( schSCManager )
	{
		TCHAR _moduleFullPath[_MAX_PATH]={0};
		TCHAR szEventFileName[_MAX_PATH]={0};
		TCHAR szServeExeName[_MAX_PATH]={0};
		TCHAR szTemp[_MAX_PATH]={0};

		::GetModuleFileName(NULL, _moduleFullPath, _MAX_PATH);
		if ( _tcsicmp(szStartType,_T("auto")) == 0 )
		{
			dwType = SERVICE_AUTO_START;
		}
		else
		{
			dwType = SERVICE_DEMAND_START;
		}

		if ( bCmdType) // 命令行安装,
		{
			memset(_moduleFullPath,0,sizeof(_moduleFullPath));
			if  ( _tcsstr(pszImageFileName,_T("%") )!= NULL )
			{
				memset(szTemp,0,sizeof(szTemp));
				ExpandEnvironmentStrings(pszImageFileName, szTemp, _MAX_PATH);
				_stprintf(_moduleFullPath,_T("%s"),szTemp);
			}
			else
			{
				_stprintf(_moduleFullPath,_T("%s"),pszImageFileName);
			}
		}
		else
		{
			if  ( _tcsstr(pszImageFileName,_T("%") )!= NULL )
			{
				memset(szServeExeName,0,sizeof(szServeExeName));
				memset(szTemp,0,sizeof(szTemp));
				ExpandEnvironmentStrings(pszImageFileName, szTemp, _MAX_PATH);
				_stprintf(szServeExeName,_T("%s"),szTemp);
			}
			else
			{
				_stprintf(szServeExeName,_T("%s"),pszImageFileName);
			}
		}

		if  ( _tcsstr(szEventDllName,_T("%") )!= NULL )
		{
			memset(szEventFileName,0,sizeof(szEventFileName));
			memset(szTemp,0,sizeof(szTemp));
			ExpandEnvironmentStrings(szEventDllName, szTemp, _MAX_PATH);
			_stprintf(szEventFileName,_T("%s"),szTemp);
		}
		else
		{
			_stprintf(szEventFileName,_T("%s"),szEventDllName);
		}
		
		schService = CreateService(
	           schSCManager,               // SCManager database
	           pszServiceName,            // name of service
               pszServiceName,            // name to display
               SERVICE_ALL_ACCESS,         // desired access
//             SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,  // service type
               SERVICE_WIN32_OWN_PROCESS,  // service type
               dwType,         // start type
	   		   SERVICE_ERROR_NORMAL,       // error control type
			   _moduleFullPath,                     // service's binary
			   NULL,                       // no load ordering group
			   NULL,                       // no tag identifier
			   _T(""),// dependencies
			   pszAccount,                       // LocalSystem account
			   pszPassword);                      // no password
		
		if ( schService )
		{
				OutputDebugString(_T("Service Installed"));
				_tprintf(_T("\n"));
				_tprintf(_T("The installation of the service successful. Exiting the installation process\n"));
				CloseServiceHandle(schService);
				InitRegeditData(pszServiceName,pszProduceName,szServeExeName,szServiceOID,szEventFileName,bCmdType);
		}
		else
		{
			OutputDebugString(TEXT("CreateService failed"));
			_tprintf(_T("\n"));
			_tprintf(_T("The installation of the service failed. Exiting the installation process\n"));
		}
		CloseServiceHandle(schSCManager);
	}
	else
	{
		OutputDebugString(TEXT("OpenSCManager failed"));
		_tprintf(_T("\n"));
		_tprintf(_T("Exection Occur\n"));
	}
	return;
}

BOOL  MyStopService(MCB_t *hHandle,SC_HANDLE hManager, TCHAR * szServiceName,BOOL bStopDependants)
{
	SC_HANDLE  schService;
	BOOL bReturn = TRUE;

	if(bStopDependants)
	{
		schService = OpenService(hManager, szServiceName, SERVICE_STOP + 
			SERVICE_QUERY_STATUS + SERVICE_ENUMERATE_DEPENDENTS);
	}
	else
	{
		schService = OpenService(hManager, szServiceName, SERVICE_STOP + 
			SERVICE_QUERY_STATUS);
	}
	
	if(schService == NULL)
	{
		MClog(hHandle, 1, _T("Failure: OpenService"));
		return FALSE;
	}

	ENUM_SERVICE_STATUS *lpServices;
	ENUM_SERVICE_STATUS *lpService;
	DWORD cbBytesNeeded, cbBytes, ServicesReturned, nIndex;
	SERVICE_STATUS ssStatus;
	static int nPrintDepend = 0;
	int  nCount;

	nCount = 0;
	DWORD dwErrorCode = -1;
	while (!ControlService(schService,	SERVICE_CONTROL_STOP, &ssStatus))
	{
		MClog(hHandle, 1, _T("Failure: ControlService"));
 		if((dwErrorCode == ERROR_DEPENDENT_SERVICES_RUNNING) && (nCount++ < 2))
		{
			if(!bStopDependants)
			{
				CloseServiceHandle(schService);
				return FALSE;
			}
			//Stop all services
			cbBytes = 0;
			lpServices = 0;
			EnumDependentServices(schService, SERVICE_ACTIVE, lpServices,
			cbBytes, &cbBytesNeeded, &ServicesReturned);
			cbBytes = cbBytesNeeded;
			lpServices = (ENUM_SERVICE_STATUS *) malloc(cbBytes);
			EnumDependentServices(schService, SERVICE_ACTIVE, lpServices,
			cbBytes, &cbBytesNeeded, &ServicesReturned);
			for(nIndex = 0; nIndex < ServicesReturned;nIndex++)
			{
				lpService = lpServices + nIndex;
				MyStopService(hHandle,hManager,lpService->lpServiceName);
			}
			free(lpServices);
		}
		else
		{
			CloseServiceHandle(schService);
			return FALSE;
		}
	}
	CloseServiceHandle(schService);
	Sleep(1000);
	return bReturn;
}
//removed by xiaohui.chai
//
//BOOL  ReStartSNMPService(MCB_t *hHandle)
//{
//	BOOL bFind   = FALSE;
//	EnterCriticalSection(&csSV); 
//	try
//	{
//		
//		SC_HANDLE  schSCMgr;
//		SC_HANDLE  schService;
//		ENUM_SERVICE_STATUS tbEnumServicesStatus[64];
//		SERVICE_STATUS ServiceStatus;
//		unsigned long iBytesNeeded;
//		unsigned long iNEntriesRead;
//		DWORD dwState = 0xFFFFFFFF;
//		DWORD dwResumeHandle;
//		BOOL  bQueryService;
//		int iSts;
//		
//		// Obtain handle to ServiceControlManager
//		if (( schSCMgr = OpenSCManager(NULL,NULL,GENERIC_READ)) == NULL )
//		{
//			iSts = GetLastError();
//			MClog(hHandle, 1,_T("OpenSCManager failed --%d"),iSts );
//			CloseServiceHandle(schSCMgr);
//			LeaveCriticalSection(&csSV);
//			return FALSE;
//		}
//		// Retrieve 1st group of ServiceStatus blocks from SCMgr
//	
//		dwResumeHandle = 0;
//		if (!EnumServicesStatus(
//			schSCMgr
//			, SERVICE_WIN32
//			, SERVICE_ACTIVE + SERVICE_INACTIVE
//			, tbEnumServicesStatus
//			, sizeof(tbEnumServicesStatus)
//			, &iBytesNeeded
//			, &iNEntriesRead
//			, &dwResumeHandle)) 
//		{
//			iSts = GetLastError();
//			if (iSts != ERROR_MORE_DATA) 
//			{
//				MClog(hHandle, 1,_T("EnumServicesStatus failed --%d"),iSts );
//				LeaveCriticalSection(&csSV);
//				return FALSE;
//			}
//		} 
//		else
//		{
//			iSts = 0;
//		}
//							
//		// Process groups of ServiceStatus blocks
//		while (iNEntriesRead > 0) 
//		{
//			// Process body of ServiceStatus blocks within current group
//			unsigned long dwIndex;
//			for (dwIndex = 0; dwIndex < iNEntriesRead; dwIndex++) 
//			{
//				// If Service Status is either stopped or starting (?), then retrieve its binary path
//				if ( _tcsicmp(tbEnumServicesStatus[dwIndex].lpServiceName,SERVICE_NAME ) == 0 )
//				{
//					bFind = TRUE;
//					break;
//				}
//			}
//			if ( bFind )
//			{
//				break;
//			}
//
//			// Read next possible group of entries
//			if (iSts == ERROR_MORE_DATA) 
//			{
//				if (!EnumServicesStatus(
//					schSCMgr
//					, SERVICE_WIN32
//					, SERVICE_ACTIVE + SERVICE_INACTIVE
//					, tbEnumServicesStatus
//					, sizeof(tbEnumServicesStatus)
//					, &iBytesNeeded
//					, &iNEntriesRead
//					, &dwResumeHandle
//				)) 
//				{
//					iSts = GetLastError();
//					if (iSts != ERROR_MORE_DATA) 
//					{
//						iNEntriesRead = 0;
//						MClog(hHandle, 1,_T("EnumServicesStatus failed --%d"),iSts );
//						LeaveCriticalSection(&csSV);
//						return FALSE;
//					}
//				} 
//				else 
//				{
//					iSts = 0;
//				}
//			} 
//			else
//			{
//				iNEntriesRead = 0;
//			}
//		}
//		if ( !bFind )
//		{
//			MClog(hHandle, 1,_T("SNMP Service not finded"));
//			CloseServiceHandle(schSCMgr);
//			LeaveCriticalSection(&csSV);
////			return FALSE;
//			return TRUE;// 为了保证没有安装SNMP Service的PC启动ZQShell.exe
//		}
//				
//		// open the service
//        schService = OpenService(schSCMgr, SERVICE_NAME,SERVICE_ALL_ACCESS);
//		if ( schService  == NULL)
//		{
//			iSts = GetLastError();
//			MClog(hHandle, 1,_T("Open SNMP Service failed --%d"),iSts);
//			LeaveCriticalSection(&csSV);
//			CloseServiceHandle(schService);
//			CloseServiceHandle(schSCMgr);
//			return FALSE;
//		}
//		memset(&ServiceStatus,0,sizeof(ServiceStatus));
//		bQueryService= QueryServiceStatus(schService,&ServiceStatus);
//		if ( !bQueryService)
//		{
//			iSts = GetLastError();
//			MClog(hHandle, 1,_T("Query Service Status failed --%d"),iSts);
//			CloseServiceHandle(schService);
//			CloseServiceHandle(schSCMgr);
//			LeaveCriticalSection(&csSV);
//			return FALSE;
//		}
//		dwState = ServiceStatus.dwCurrentState;
//
//		if ( dwState == SERVICE_RUNNING ) 
//		{
//			memset(&ServiceStatus,0,sizeof(ServiceStatus));
//			bQueryService = ControlService(schService,SERVICE_CONTROL_INTERROGATE,&ServiceStatus);
//			if ( !bQueryService)
//			{
//				iSts = GetLastError();
//				MClog(hHandle, 1, _T("Query Service Status  failed -- %d"), iSts);
//				CloseServiceHandle(schService);
//				CloseServiceHandle(schSCMgr);
//				LeaveCriticalSection(&csSV);
//				return FALSE;
//			}
//			dwState = ServiceStatus.dwCurrentState;
//		}
//		if ( dwState != SERVICE_STOPPED ) // SNMP Service has not be stopped
//		{
//
//			/*
//			 memset(&ServiceStatus,0,sizeof(ServiceStatus));
//			 bQueryService = ControlService(schService,SERVICE_CONTROL_STOP,&ServiceStatus);
// 			 if ( !bQueryService)
//			 {
//				iSts = GetLastError();
//				_tprintf(_T("To to Stop SNMP Service Status  failed -- %d"), iSts);
//				CloseServiceHandle(schService);
//				CloseServiceHandle(schSCMgr);
//				LeaveCriticalSection(&csSV);
//				return FALSE;
//			 }
//			 */
//			// try to stop the service
//			memset(&ServiceStatus,0,sizeof(ServiceStatus));
//			if ( ControlService( schService, SERVICE_CONTROL_STOP, &ServiceStatus ) )
//			{
//				Sleep( DEF_REBOOTCOUNTLIMIT );
//				while( QueryServiceStatus( schService, &ServiceStatus ) )
//				{
//					if ( ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING )
//					{
//						Sleep( DEF_REBOOTCOUNTLIMIT );
//					}
//					else
//					{
//						break;
//					}
//				}
//			}
//		}
//    	// SNMP Service has been stopped
//		bQueryService = StartService(schService, 0, NULL);
//		if ( !bQueryService)
//		{
//			iSts = GetLastError();
//			MClog(hHandle, 1, _T("To to Start SNMP Service Status  failed -- %d"), iSts);
//			CloseServiceHandle(schService);
//			CloseServiceHandle(schSCMgr);
//			LeaveCriticalSection(&csSV);
//			return FALSE;
//		}
//		
//        // close the service handle
//        CloseServiceHandle(schService);
//		CloseServiceHandle(schSCMgr);
//	}
//	catch(...)
//	{
//		MClog(hHandle, 1, _T("Enter the Start SNMP Service Catch Status"));
//	}
//	MClog(hHandle, 1, _T(" Start SNMP Service Successful"));
//	LeaveCriticalSection(&csSV); 
//	return TRUE;
//}

void  InstallService(TCHAR *pstrServiceName)
{
	EnterCriticalSection(&csSV); 
    try
	{
		if (IsInstalled(pstrServiceName))
		{
			LeaveCriticalSection(&csSV);
			_tprintf(_T("The Service has installed\n"));
			return;
		}

		_tprintf(_T("\n"));
		_tprintf(_T("Welcome to the ITV Service Installer!\n"));
		_tprintf(_T("\n"));
		_tprintf(_T("\n"));
		_tprintf(_T("Step 1: Configure the ITV service name (as it should appear in the NT Services control panel)\n"));
		_tprintf(_T("        Typical examples are: 'ZQ ITV Streaming Service', or 'ZQ ITV Propagation Service'\n"));
		_tprintf(_T("\n"));
		_tprintf(_T("\n"));
		_tprintf(_T("Please enter service name:"));
		string strServiceName;
		cin >> strServiceName;

		TCHAR szServiceName[MAX_PATH]={0};
#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
            CP_ACP,         // code page
            0,              // character-type options
            (char*)strServiceName.c_str(),        // address of string to map
            strlen((char*)strServiceName.c_str()),      // number of bytes in string
            szServiceName,       // address of wide-character buffer
            MAX_PATH);   // size of buffer);
#else
		memcpy(szServiceName,(char*)strServiceName.c_str(),strlen((char*)strServiceName.c_str()));
#endif

		_tprintf(_T("\n"));
		_tprintf(_T("Step 2: Configure the ITV product name.\n"));
		_tprintf(_T("        Typical examples are: 'ITV', or 'ITV Applications'.\n"));
		_tprintf(_T("\n"));
		_tprintf(_T("Please enter product name:"));
		string strProduceName;
		cin >> strProduceName;

		TCHAR szProductName[MAX_PATH]={0};
#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
            CP_ACP,         // code page
            0,              // character-type options
            (char*)strProduceName.c_str(),        // address of string to map
            strlen ((char*)strProduceName.c_str()),      // number of bytes in string
            szProductName,       // address of wide-character buffer
            MAX_PATH);             // size of buffer);
#else
		memcpy(szProductName,(char*)strProduceName.c_str(),strlen((char*)strProduceName.c_str()));
#endif
		szProductName[strlen((char*)strProduceName.c_str())]='\0';
		
		_tprintf(_T("\n"));
		_tprintf(_T("Step 3: Configure the ITV service binary image, which is the name of the executable that service shell should run to actually spawn an instance of the ITV service.\n"));
		_tprintf(_T("        Typical examples are: 'd:\\tianshan\\bin\\iam_d.exe' \n"));
		_tprintf(_T("        The general format is: '<your ITVROOT>\\bin\\<the executable>'\n"));
		_tprintf(_T("\n"));
		_tprintf(_T("Please enter the ITV service binary image:"));
		string strBinFileName;
		cin >> strBinFileName;

		TCHAR szImageFileName[MAX_PATH]={0};
#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
            CP_ACP,         // code page
            0,              // character-type options
            (char*)strBinFileName.c_str(),        // address of string to map
            strlen ((char*)strBinFileName.c_str()),      // number of bytes in string
            szImageFileName,       // address of wide-character buffer
            MAX_PATH);             // size of buffer);
#else
		memcpy(szImageFileName,(char*)strBinFileName.c_str(),strlen((char*)strBinFileName.c_str()));
#endif
		szImageFileName[strlen((char*)strBinFileName.c_str())]='\0';

		_tprintf(_T("\n"));
		_tprintf(_T("Step 4: Configure the Service's SNMP OID Value,It is a dword value, and it is a uniquely value,so please not to set it  as same as other service's\n"));
		_tprintf(_T("\n"));
		
		TCHAR buf[MAX_PATH] =_T("SOFTWARE\\ZQ Interactive\\SNMPOID\\CurrentVersion\\Services\\");
		HKEY hkey,hKeyService;
		int     iSubKey = 0 ;
        TCHAR   szName[MAX_PATH] ={0};
		DWORD   dwSize = MAX_PATH,dwOID,dwType;
		LONG status = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              buf,
                              0,
                              KEY_READ,
                              &hkey);
							  
		if ( status == ERROR_SUCCESS )
		{
			_tprintf(_T("        The OIDs below  have been allocated by Services,So they are not to allocate again\n"));
			_tprintf(_T("                       ServiceName                        OID\n"));
			
			
			while (ERROR_SUCCESS == (status = RegEnumKeyEx(hkey,      // opened registry key
                                                      iSubKey,          // sub key index
                                                      szName,           // buffer to hold the key name
                                                      &dwSize,          // size of buffer
                                                      NULL,             // reserved
                                                      NULL,             // class, not interested here
                                                      NULL,             // size of buffer to hold class
                                                      NULL)))           // last time written, not interested
			{
				// now read this subkey to see if it is configured to be manageable
				if (ERROR_SUCCESS == RegOpenKeyEx(hkey,
													szName,
													0,
													KEY_READ,
													&hKeyService))
				{
					dwSize = sizeof(DWORD);
					if (ERROR_SUCCESS == RegQueryValueEx(hKeyService,
														   _T("ServiceOID"),
														   NULL,
														   &dwType,
														   (LPBYTE)&dwOID,
														   &dwSize) 
														   && (dwType == REG_DWORD))
					{
						_tprintf(_T("                       %s                        %d\n"),szName,dwOID);
					}
					RegCloseKey(hKeyService);
				}
				++iSubKey;              // increment iSubkey to get next key
				dwSize = MAX_PATH;      // reset size
			}
			_tprintf(_T("\n"));
			RegCloseKey(hkey);
		}
        
		_tprintf(_T("Please enter the SNMP OID Value for this service:"));
		string strOIDValue;
		cin >> strOIDValue;

		TCHAR szOIDValue[MAX_APPSHELL_NAME_LENGTH]={0};
#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
            CP_ACP,         // code page
            0,              // character-type options
            (char*)strOIDValue.c_str(),        // address of string to map
            strlen ((char*)strOIDValue.c_str()),      // number of bytes in string
            szOIDValue,       // address of wide-character buffer
            MAX_APPSHELL_NAME_LENGTH);             // size of buffer);
#else
		memcpy(szOIDValue,(char*)strOIDValue.c_str(),strlen((char*)strOIDValue.c_str()));
#endif
		szOIDValue[strlen((char*)strOIDValue.c_str())]='\0';
		
		_tprintf(_T("\n"));
		_tprintf(_T("Step 5: Configure the installation type for the service. Enter '1' if you wantto install the service as a LocalSystem account, or '2' as a SeaChange account.\n"));
		_tprintf(_T("\n"));
		_tprintf(_T("Please enter the type of installation (1 or 2):"));
		char ch;
		cin >> ch;

		TCHAR *szPassword = _T("ZQ");
		TCHAR *szAccount = (TCHAR*)malloc(MAX_PATH+1);
		if ( ch =='1' ) // LocalSystem account
		{
			szPassword = NULL;
			_tcscpy(szAccount,_T("LocalSystem"));
			szAccount[_tcsclen(_T("LocalSytem"))+1]='\0';
//			szAccount = NULL;

		}
		else
		{
			TCHAR szSysName[MAX_COMPUTERNAME_LENGTH + 3];
			DWORD dwSize = MAX_COMPUTERNAME_LENGTH +1;				// T1.3-05
			GetComputerName(szSysName, &dwSize);	// T1.3-05
			_tcscpy(szAccount,szSysName);
			_tcscat(szAccount,_T("\\ZQ"));
		}

		_tprintf(_T("\n"));

		_tprintf(_T("Step 6: Configure the NT Event Log messages DLL for the ITV service."));
		_tprintf(_T("\n"));
		_tprintf(_T("      Typical examples are: 'd:\\tianshan\\exe\\ZQShellMsgs.dll,%ITVROOT%\\ZQShellMsgs.dll'  \n"));
		_tprintf(_T("\n"));
		_tprintf(_T("Please enter the NT Event Log Message DLL:"));
		

		string strEventDllName;
		cin >> strEventDllName;

		TCHAR szEventDllName[MAX_PATH]={0};
#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
            CP_ACP,         // code page
            0,              // character-type options
            (char*)strEventDllName.c_str(),        // address of string to map
            strlen ((char*)strEventDllName.c_str()),      // number of bytes in string
            szEventDllName,       // address of wide-character buffer
            MAX_PATH);             // size of buffer);
#else
		memcpy(szEventDllName,(char*)strEventDllName.c_str(),strlen((char*)strEventDllName.c_str()));
#endif
		szEventDllName[strlen((char*)strEventDllName.c_str())]='\0';

		TCHAR *szStartType=_T("manual");
		BOOL bCmdType = FALSE;
		AddService(szServiceName,szProductName,szImageFileName,szOIDValue,szAccount,szPassword,szEventDllName,szStartType,bCmdType);
	}
	catch(...)
	{
		OutputDebugString(TEXT("Exection Occur"));
		_tprintf(_T("Exection Occur\n"));
	}
	LeaveCriticalSection(&csSV);
	return;
}

// remove the NT Service
void  UnstallService(TCHAR *pstrServiceName)
{
	EnterCriticalSection(&csSV); 
	_try
	{
		
		if (!IsInstalled(pstrServiceName))
		{
			_tprintf(_T("The Service has not installed \n"));
			LeaveCriticalSection(&csSV);
			return ;
		}

		SC_HANDLE       schService;
		SC_HANDLE       schSCManager;
		SERVICE_STATUS  ssStatus;       // current status of the service

		schSCManager = ::OpenSCManager(
							NULL,                   // machine (NULL == local)
							NULL,                   // database (NULL == default)
							SC_MANAGER_ALL_ACCESS   // access required
							);
		if ( schSCManager )
		{
			schService = OpenService(schSCManager,pstrServiceName, SERVICE_ALL_ACCESS);

			if (schService)
			{
				// try to stop the service
				if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
				{
					Sleep( DEF_REBOOTCOUNTLIMIT );

					while( QueryServiceStatus( schService, &ssStatus ) )
					{
						if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
						{
							Sleep( DEF_REBOOTCOUNTLIMIT );
						}
						else
						{
							break;
						}
					}
				}

				// now remove the service
				if( DeleteService(schService) )
				{
					OutputDebugString(_T("Removed Service Successful"));
					_tprintf(_T("Removed Service Successful\n"));
				}
				else
				{
					OutputDebugString(_T("Removed Service Failed"));
					_tprintf(_T("Removed Service Failed\n"));
				}
				CloseServiceHandle(schService);
			}
			else
			{
			   CloseServiceHandle(schSCManager);
			}
		}
	}
	_finally 
	{
		
	}
	LeaveCriticalSection(&csSV);
	return;
}
//removed by xiaohui.chai
//void  ResetServieCurrentOID(TCHAR *pstrServiceName,TCHAR *pstrProdName)
//{
//	TCHAR   buf[2*MAX_PATH+1]; 
//	memset(buf,0,sizeof(buf));
//	_stprintf(buf, _T("%s"),pstrServiceName);
//	HANDLE  hcfg;
//	DWORD   cfg_value;
//
//	hcfg = ShellCfgInitEx(buf, &cfg_value,_T("SNMPOID"));
//	if ( hcfg )
//	{
//	    cfg_value = 0;
//		ShellCfgSetValue(hcfg, _T("CurrentVarOID"), (BYTE*)&cfg_value,
//	     											 sizeof(cfg_value), REG_DWORD);
//		ShellCfg_Term(hcfg);
//	}
//}

void  InitRegeditData(TCHAR *pstrServiceName,TCHAR *pstrProdName,TCHAR *szImageFile,TCHAR *szServiceOID,TCHAR *szEvnetDllName,BOOL bCmdType)
{
	TCHAR   buf[2*MAX_PATH]={0}; 
	HANDLE  hcfg;
	DWORD   cfg_value;
	int     iRet;
	DWORD   dLen;
	DWORD   dValue;

	try
	{
		if ( !bCmdType ) // 如果是向导安装
		{
			_stprintf(buf, _T("%s_shell"),pstrServiceName);

			hcfg = ShellCfgInitEx(buf, &cfg_value,pstrProdName);
			
			if ( hcfg )
			{

				cfg_value = 45000;
				iRet = ShellCfgSetValue(hcfg, _T("AliveTimeout"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 30000;
				iRet = ShellCfgSetValue(hcfg, _T("AppAliveWait"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 35000;
				iRet = ShellCfgSetValue(hcfg, _T("AppShutdownWait"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);
				cfg_value = 30000;
				iRet = ShellCfgSetValue(hcfg, _T("AppStartupWait"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				dLen = _tcsclen(szImageFile) * 2;
				iRet = ShellCfgSetValue(hcfg, _T("ImagePath"), (BYTE*)szImageFile,
	      												 dLen, REG_SZ);

				memset(buf,0,sizeof(buf));
				_stprintf(buf, _T("C:\\%s_Shell\\"),pstrServiceName);
				dLen = _tcsclen(buf) * 2;
				iRet = ShellCfgSetValue(hcfg, _T("LogDir"), (BYTE*)buf,
	      												 dLen, REG_SZ);
				

				cfg_value = 0;
				iRet = ShellCfgSetValue(hcfg, _T("LastReboot"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);
				cfg_value = _ttol(szServiceOID);
				iRet = ShellCfgSetValue(hcfg, _T("OID"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);
				cfg_value = 1;
				iRet = ShellCfgSetValue(hcfg, _T("LoggingMask"), (BYTE*)&cfg_value,
														sizeof(cfg_value), REG_DWORD);

				cfg_value = 0;
				iRet = ShellCfgSetValue(hcfg, _T("SnmpLoggingMask"), (BYTE*)&cfg_value,
														sizeof(cfg_value), REG_DWORD);
				cfg_value = 5*1000;
				iRet = ShellCfgSetValue(hcfg, _T("KillCheckTime"), (BYTE*)&cfg_value,
														sizeof(cfg_value), REG_DWORD);
				cfg_value = 600;
				iRet = ShellCfgSetValue(hcfg, _T("KillCheckCount"), (BYTE*)&cfg_value,
														sizeof(cfg_value), REG_DWORD);
				cfg_value = 0;
				iRet = ShellCfgSetValue(hcfg, _T("RebootCount"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 0;
				iRet = ShellCfgSetValue(hcfg, _T("RebootOnExit"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 200;
				iRet = ShellCfgSetValue(hcfg, _T("RestartDelay"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 120;
				iRet = ShellCfgSetValue(hcfg, _T("RestartInterval"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 3;
				iRet = ShellCfgSetValue(hcfg, _T("RestartTries"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 45000;
				iRet = ShellCfgSetValue(hcfg, _T("StartTimeout"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 45000;
				iRet = ShellCfgSetValue(hcfg, _T("StopTimeout"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);
				iRet = ShellCfg_Term(hcfg);
			}

			memset(buf,0,sizeof(buf));
			_stprintf(buf, _T("%s"),pstrServiceName);

			hcfg = ShellCfgInitEx(buf, &cfg_value,pstrProdName);
			if ( hcfg )
			{
				cfg_value = 2000;
				iRet = ShellCfgSetValue(hcfg, _T("KeepAliveIntervals"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 204800;
				iRet = ShellCfgSetValue(hcfg, _T("LogBufferSize"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 50000;
				iRet = ShellCfgSetValue(hcfg, _T("LogFileSize"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);

				cfg_value = 0;
				iRet = ShellCfgSetValue(hcfg, _T("SnmpLoggingMask"), (BYTE*)&cfg_value,
														sizeof(cfg_value), REG_DWORD);

				
				cfg_value = 2000;
				iRet = ShellCfgSetValue(hcfg, _T("LogFileTimeOut"), (BYTE*)&cfg_value,
	      												 sizeof(cfg_value), REG_DWORD);
                /*
				memset(buf,0,sizeof(buf));
				_stprintf(buf, _T("C:\\%s.log"),pstrServiceName);
				dLen = _tcsclen(buf) * 2;
				iRet = ShellCfgSetValue(hcfg, _T("LogFileName"), (BYTE*)buf,
	      												 dLen, REG_SZ);
				*/
				memset(buf,0,sizeof(buf));
				_stprintf(buf, _T("C:\\%s\\"),pstrServiceName);
				dLen = _tcsclen(buf) * 2;
				iRet = ShellCfgSetValue(hcfg, _T("LogDir"), (BYTE*)buf,
	      												 dLen, REG_SZ);

				iRet = ShellCfgSetValue(hcfg, _T("configDir"), (BYTE*)buf,
	      												 dLen, REG_SZ);

				iRet = ShellCfg_Term(hcfg);
			}
		}

		memset(buf,0,sizeof(buf));
		_stprintf(buf, _T("%s"),pstrServiceName);

		hcfg = ShellCfgInitEx(buf, &cfg_value,_T("SNMPOID"));
		if ( hcfg )
		{
			cfg_value = _ttol(szServiceOID);
			iRet = ShellCfgSetValue(hcfg, _T("ServiceOID"), (BYTE*)&cfg_value,
	      											 sizeof(cfg_value), REG_DWORD);
			iRet = ShellCfg_Term(hcfg);
		}
		
	
	// Try to Open registry key, set value, close key.
		memset(buf,0,sizeof(buf));
    	_stprintf(buf, _T("SYSTEM\\CurrentControlSet\\Services\\%s"),pstrServiceName);

		LONG status;
		HKEY hkey;
		DWORD           dwDisposition;


        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              buf,
                              0,
                              KEY_ALL_ACCESS,
                              &hkey);
        if (ERROR_SUCCESS == status)
        {
			dLen = _tcsclen(pstrProdName) * 2;
			
			status = RegSetValueEx(hkey,
                                   _T("ProductName"),
                                   0,
                                   REG_SZ,
                                   (BYTE*)pstrProdName,
                                   dLen);
            RegCloseKey(hkey);
        }
		 
		memset(buf,0,sizeof(buf));
		_stprintf(buf, _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\%s"),pstrServiceName);

		status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                buf,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkey,
                                &dwDisposition);

        if (ERROR_SUCCESS == status)
        {
			status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              buf,
                              0,
                              KEY_ALL_ACCESS,
                              &hkey);

			dLen = _tcsclen(szEvnetDllName) * 2;
			
			status = RegSetValueEx(hkey,
                                   _T("EventMessageFile"),
                                   0,
                                   REG_SZ,
                                   (BYTE*)szEvnetDllName,
                                   dLen);
			dValue = 7;
			status = RegSetValueEx(hkey,
				                   _T("TypesSupported"),
								   0,
								   REG_DWORD,
								   (BYTE*)&dValue,
								   sizeof(dValue));
		
            RegCloseKey(hkey);
        }

		memset(buf,0,sizeof(buf));
		_stprintf(buf, _T("SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\%s_shell"),pstrServiceName);

		status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                buf,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkey,
                                &dwDisposition);

        if (ERROR_SUCCESS == status)
        {
			status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              buf,
                              0,
                              KEY_ALL_ACCESS,
                              &hkey);

			dLen = _tcsclen(szEvnetDllName) * 2;
			
			status = RegSetValueEx(hkey,
                                   _T("EventMessageFile"),
                                   0,
                                   REG_SZ,
                                   (BYTE*)szEvnetDllName,
            					   dLen);
			
			dValue = 7;
			status = RegSetValueEx(hkey,
				                   _T("TypesSupported"),
								   0,
								   REG_DWORD,
								   (BYTE*)&dValue,
								   sizeof(dValue));
		
            RegCloseKey(hkey);
        }

    }
	catch(...)
	{
	}
	return;
}

