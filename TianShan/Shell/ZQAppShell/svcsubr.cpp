/***************************************************************************
**
** Copyright (c) 2006 by
** zq Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
** 
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of zq  Technology  Inc.   Possession, use,
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
** zq  assumes  no  responsibility  for the use or reliability of its
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
**		Service Utilities
**
**
** VERSION:
**
**		X2.2-0 001
**
**
** FACILITY
**
**		CDCI Services
**
**
** ABSTRACT:
**
**		This module contains utility routines used by the Service Shell and
**		the Application Shell.
**
**
** REVISION HISTORY: (Most recent at top)
**
**	 Revision	   Date			Modified By		Reviewed By
**	----------	----------- 	-----------		-----------
**
**		Added the ReportHook function to trap debug asserts and raise
**		and SEH exception to appshell's handler.  The result is to stop
**		the process and get a Watson log and/or dump depending on how
**		Watson's configured and assuming Watson's the default "debugger"
**		configured into AeDebug in the registry.  We'll also put an event
**		in the EventLog with the module name and line number assuming the
**		Assert's done with ASSERT, _ASSERT, _ASSERTE.  The ansi "assert"
**		just stupidly prints "abnormal program termination".
**
**
**		Modified the CopyExceptionInformation() routine so conditionally
**		return EXCEPTION_EXECUTE_HANDLER or EXCEPTION_CONTINUE_SEARCH.
**		When debugging, the symbol DEBUG_EXCEPTION can be defined on
**		the compiler command line to force this routine to continue
**		the exception search.  This will make the Application Shell
**		ignore the exception and will let the debugger pick it up.
**
**
**
**		The following modifications were made to this module as
**		part of this edit:
**
**			o Added routines used to copy exception information, free
**			  copied exception information, and log exception information.
**
**			o Cleaned up the copyright, header, etc.
**
**
**	
**		
**
***************************************************************************/

/*
*  Includes
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>		//X2.2-0 001
#include <winnt.h>
#include <excpt.h>		//X2.2-0 001
#include <tchar.h>
#include <crtdbg.h>

#include "shell.h"
#include "zqappshell.h"
#include "zqcfgpkg.h"
//#include "manpkg.h"
#include "ZqMessages.h"


//X2.2-0 001 start
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
#ifdef __cplusplus
extern "C" {
#endif
extern DWORD   fac_mask;
//extern TCHAR    servname[ MAX_APPSHELL_NAME_LENGTH +1 ];
extern TCHAR    sysname[ MAX_COMPUTERNAME_LENGTH + 3 ];
extern TCHAR    eventsink[ MAX_COMPUTERNAME_LENGTH + 3 ];
//X2.2-0 001 end
#ifdef __cplusplus
}
#endif

LPTSTR  	msg[3];


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
LogServiceEvent(DWORD eventID, WORD strcnt, TCHAR **strarray, DWORD bdatacnt,
	LPVOID bdata, TCHAR *servname, DWORD fac_mask, TCHAR *eventsink, TCHAR *sysname)
{
HANDLE  hEventSource;
WORD		type;
TCHAR		buf[MAXOBJNAMELEN];

	/* Register ourselves as the app's shell. Always log locally. */
		_stprintf(buf, _T("%s_Shell"), servname);
	  hEventSource = RegisterEventSource(NULL, buf);
	
	/* 
	 * Switch facility codes to "impersonate" the service we're monitoring.
	 * Moved out of hEventSource conditional for V2.0.
	 */
		eventID &= 0xf000ffff;
		eventID |= fac_mask;

	/* If we don't have an eventlog handle, we can't log locally. */  		
		if(hEventSource != NULL) {

		/* Calculate type based on eventid */
			type = (WORD)(eventID >> 30) & 0x3;
			switch (type) {

				case 1:				 				/* Map 01 to INFORMATION (04)*/
					type = EVENTLOG_INFORMATION_TYPE;
					break;

				case 2:								/* Map 10 to Warning (02)*/
					type = EVENTLOG_WARNING_TYPE;
					break;

				default:							/* Map anything else to ERROR */
					type = EVENTLOG_ERROR_TYPE;
					break;
			}

		/* Log the event. */
			ReportEvent(hEventSource, // handle of event source
				    type,									// event type
				    0,                    // event category
				    eventID,              // event ID
				    NULL,                 // current user's SID
				    strcnt,               // strings in lpszStrings
				    bdatacnt,             // no. of bytes of raw data
                    (LPCTSTR*) strarray,  // array of error strings
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
TCHAR                msgbuf[ 2 ][ 128 ];
TCHAR                *msglist[ 2 ];


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
			memset( &XBuf.Context, _T('?'), sizeof( XBuf.Context ) );
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
		memset( &XBuf, _T('?'), sizeof( XBuf ) );
	}


	/*
	*  Log the Application Shell exception event.
	*/
	LogServiceEvent(	Event,
						2,
						msglist,
						sizeof( XBuf ),
						&XBuf,
						servname,
						fac_mask,
						eventsink,
						sysname );
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
		eto = (EXCEPTION_RECORD*)malloc( sizeof( EXCEPTION_RECORD ) );
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
		cto = (CONTEXT*)malloc( sizeof( CONTEXT ) );
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
		eto = (EXCEPTION_RECORD*)malloc( sizeof( EXCEPTION_RECORD ) );
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

/*
** pbm 09-Jan-02
**
** This function is called whenever _CrtDbgReport is called and it's passed
** most of the arguments that would be passed to _CrtDbgReport.  The intent
** of the function is to trap asserts (denoted by reportType == _CRT_ASSERT).
** However, if the programmer used the lowercase ansi "assert", the reportType
** is passed to this function as _CRT_ERROR.  To trap that style of assert we
** also have to trap the ERRORS.  Trapping the errors is not a bad idea anyway
** so we only let the _CRT_WARNING reportTypes pass through unscathed.
**
** Differing styles of asserts will write different information into the event
** log as follows:
**
**	ASSERT(condition) - MFC Style:  Writes module name and Line number
**  _ASSERT(condition) - C runtime Style: Same as ASSERT
**  _ASSERTE(condition) - C runtime Style: adds assert condition to module name 
**		and Line number
**  assert(condition) - Ansi style:  Just writes "Abnormal Program Termination", 
**		but writes ModuleName, Line number, and assert condition to STDOUT - 
**		wherever that might be pointing....
**
** Assert trapping can be disabled altogether by defining an AppShell DWORD 
** registry value named "EnableAsserts" and setting the value to non-zero.  
** If the registry value doesn't exist or is set to zero, then this function 
** will be invoked when an assert's encountered.
*/
//modify by dony 2006 change change seachange's unicode  WCHAR to  TCHAR 
int ReportHook( int reportType, char *userMessage, int *retVal ){

	const int temp_buf_length = 1000;
//	WCHAR temp[temp_buf_length];
	TCHAR temp[temp_buf_length];

	if((reportType == _CRT_ASSERT) || (reportType == _CRT_ERROR)){
		msg[0] = _T("continue");
		msg[1] = _T("SCASSERT_EXCEPTION");
		if(userMessage > 0){
			if(strlen(userMessage) < temp_buf_length){
//				wsprintf(temp, _T("%hs"), userMessage);
				_stprintf(temp, _T("%hs"), userMessage);
				msg[2] = temp;
			}else{
				msg[2] = _T("Assert Message too Long");
			}
		}else{
			msg[2] = _T("Null Assert Message Pointer");
		}
		LogServiceEvent(APPSHELL_EVENT_ERROR, 3, msg, 0, NULL,
                        servname, fac_mask, eventsink, sysname);

		RaiseException(SCASSERT_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}
	// *retVal = 1 tells _CrtDbgReport to start the debugger
	// *retVal = 0 tells _CrtDbgReport to continue with normal execution
	*retVal = 0;

	// returning true means call _CrtDbgReport
	// returning false means don't call _CrtDbgReport
	return false;
}
//X2.2-0 001 end
