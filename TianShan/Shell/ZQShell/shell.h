/***************************************************************************
**
** Copyright (c) 1994, 1995 by
** SeaChange Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
** 
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of SeaChange  Technology  Inc.   Possession, use,
** duplication or dissemination of the software and media is authorized only
** pursuant to a valid written license from SeaChange Technology Inc.
** 
** This software is furnished under a  license  and  may  be used and copied
** only in accordance with the terms of  such license and with the inclusion
** of the above copyright notice.  This software or any other copies thereof
** may not be provided or otherwise made available to  any other person.  No
** title to and ownership of the software is hereby transferred.
**
** The information in this software is subject to change without  notice and
** should not be construed as a commitment by SeaChange Technology Inc.
** 
** SeaChange  assumes  no  responsibility  for the use or reliability of its
** software on equipment which is not supplied by SeaChange.
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
**      Shell Definitions (shell.h)
**
**
** VERSION:
**
**     	X3.0
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
**   Revision      Date     	Modified By     Description
**  ----------  -----------     -----------     -----------
**  1.00		25-10-2006		DYH				Created
*/


#ifndef SHELL_H
#define SHELL_H

#define MAX_APPSHELL_NAME_LENGTH    64

//////////////////////////////////////////////
// Service Shell Registry default timer values
//
// "StartTimeout" Maximum number of milliseconds for which the service shell 
// waits for the application to set the 1st ALIVE event
#ifdef _TESTALIVE // for test the heartbeat,so the test time is 1/10 of the default time
	#define START_TIMEOUT    4500          
#else
	#define START_TIMEOUT    45000          
#endif

// "AliveTimeout"  Maximum number of milliseconds for which the service shell 
// waits for the application to set each subsequent ALIVE event
#ifdef _TESTALIVE // for test the heartbeat,so the test time is 1/10 of the default time
	#define ALIVE_TIMEOUT    4500          
#else
	#define ALIVE_TIMEOUT    45000          
#endif

// "StopTimeout"  Number of millisecons for which the service shell waits
// for the application to respond to a STOP event
#ifdef _TESTALIVE // for test the heartbeat,so the test time is 1/10 of the default time
	#define STOP_TIMEOUT     4500
#else
    #define STOP_TIMEOUT     45000          
#endif

#define	MIN_DIFFERENCE	 10000			// Minimum value of (ServiceShell timeout - AppShell wait)

//////////////////////////////////////////
// AppShell Registry default timer values
//
// "AppStartupWait"  Number of milliseconds which the application shell
// waits for the app_main thread to set the 1st ALIVE event.
#define STARTUP_WAIT	 30000			


// "AppAliveWait" Interval in milliseconds in which the application sets the ALIVE event
#ifdef _TESTALIVE // for test the heartbeat,so the test time is 1/10 of the default time
	#define ALIVE_WAIT		 3000			
#else
	#define ALIVE_WAIT		 30000			
#endif

// "AppShutDownWait" Interval in milliseconds in which the application shell waits
// after seeing the SCMgrSTOP event before it calls ExitProcess.  During this interval
// the application control thread is performing its application specific shutdown operations.
#ifdef _TESTALIVE // for test the heartbeat,so the test time is 1/10 of the default time
	#define	SHUTDOWN_WAIT	 3500			
#else
	#define	SHUTDOWN_WAIT	 35000			
#endif

#endif
