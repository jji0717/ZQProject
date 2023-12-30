/***************************************************************************
**
** Copyright (c) 1994, 1995 by
** ZQ Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
** 
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of SeaChange  Technology  Inc.   Possession, use,
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
***************************************************************************/

/***************************************************************************
**
** TITLE:
**
**      Application Shell Definitions
**
**
** VERSION:
**
**      V1.0-0 001
**
**
** FACILITY
**
**      CDCI Services
**
**
** ABSTRACT:
**
**      This module contains definitions and declarations used by CDCI
**      service applications.
**
**
** REVISION HISTORY: (Most recent at top)
**
**   Revision      Date     	Modified By     Reviewed By
**  ----------  -----------   -----------     -----------
**
**  T1.0-0 001    10-21-2006         DWH
**
**      Initial fieldtest release.
**
**
***************************************************************************/

#ifndef _APPSHELL_H
#define _APPSHELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "shell.h"	// Default registry timer values for Service and App shell

/*
*  Defines.  ***  NOTE!!! APPALIVE Must Be The Last Event ***
*/
#define SCMgrSTOP           (0)         /* Service Exit Event */
#define SCMgrPAUSE          (1)         /* Service Pause Event */
#define SCMgrCONTINUE       (2)         /* Service Continue Event */
#define APPSHUTDOWN         (3)         /* App shutting down Event, added 5/3/95, V2.1-7, MMS */
#define APPTHREAD           (4)         /* Application Thread Exit Event */
#define APPALIVE            (5)         /* Application Alive Event (!!MUST BE LAST!!) */
#define APPSHLNUMHANDLES    (6)         /* Total Number of Handles */

#define SCASSERT_EXCEPTION 0xEF00E001
#define STATUS_APPSHELL_SHUTDOWN_TIMEOUT  0xEF00E002  // Exception codes for SPR# 2716
#define STATUS_SRVSHELL_SHUTDOWN_TIMEOUT  0xEF00E003  // Exception codes for SPR# 2716

/*
*  Module Wide Data
*/



#ifdef _APPSHELL_C
HANDLE  handle_array[ APPSHLNUMHANDLES ]; 
BOOL    gbAppShellSetKeepAlive = TRUE; 		// Flag used to indicate the application will set the keepalive event
DWORD   gdwAppShellAliveWait = ALIVE_WAIT;   // AppShell alive wait timeout value
#endif


/////////////////////////////////////////////////////////////
// The user probably cares about the following 
/*
*  Externals
*/
#ifndef _APPSHELL_C
#ifdef __cplusplus
extern "C" {
#endif
extern HANDLE  handle_array[ APPSHLNUMHANDLES ]; 
extern HANDLE  HAppPaused, HAppContinued;
extern BOOL    gbAppShellSetKeepAlive; // Flag used to indicate the application will set the keepalive event
extern DWORD   gdwAppShellAliveWait;

//modify by dony 20061021 the remarked code is the seachange code
extern TCHAR   servname[MAX_APPSHELL_NAME_LENGTH]; // 10/10/95 CJH - extern this so everyone can use it
extern TCHAR   prodname[MAX_APPSHELL_NAME_LENGTH]; // product name to which this service belongs (set on input in argv)

///////////////////////////////////////////////////
// app_main
// -- The main entry point to your service.  This is called by 
// appshell once it initializes the service control events
//
// argc and argv are the command line parameters passed into the appshell by
// SrvShell or a user.
// The cmd line is: <image> -s <service-name> <product-name>
// For example, L:\Debug\ids_d.exe -s "ICM" "ITV"
// 
// When app_main begins the globals referred to above are filled in with useful 
// information.  servname hols <service-name> in wide-char format.
// prodname holds <product-name> in wide-char format.
//

// Yes, this is narrow char... even for UNICODE programs it is the same as main
void app_main(int argc, char *argv[]);

//////////////////////////////////////////////////
// app_service_control
// -- The entry point for service control of your service.
// This is called by appshell when SrvShell reports a Service
// Control Manager Command.
//
// iSCCode -- one of the #define codes described above
// usually these are SCMgrStop, SCMgrPause, and SCMgrContinue
// commands can be ignored, it's entirely up to your app
// However, the SCMgrStop command should not be ignored or 
// abnormal and very nasty shutdown (i.e. ExitProcess()) will occur.
//
void  app_service_control( int iSCCode);

#ifdef __cplusplus
}
#endif
#endif // !_APPSHELL_C

/*
*  Function prototypes
*/
HANDLE StartThread( void*,
                    unsigned,
                    int (*)(void*),
                    void*,
                    unsigned,
                    unsigned*,
                    int (*)(void*, EXCEPTION_POINTERS* ),
                    int (*)(void*, EXCEPTION_POINTERS* ),
                    int (*)(void*) );

#ifdef __cplusplus
}
#endif



#endif /* _APPSHELL_H */
