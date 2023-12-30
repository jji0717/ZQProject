// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: ModAuthReporter.cpp,v 1.1 2004/12/13 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Implement the Log writting.
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/MODPlugin/MODAuthorization/ModAuthReporter.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     07-01-26 11:21 Ken.qian
// 
// 2     05-06-20 21:24 Ken.qian
// 
// 1     04-12-13 17:43 Ken.qian
// Revision 1.1  2004/12/13 Ken Qian
//   definition and implemention

#include "stdafx.h"
#include <malloc.h>
#include "ModAuthReporter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModAuthReporter::CModAuthReporter()
{

}

CModAuthReporter::~CModAuthReporter()
{

}

RPTSTATUS CModAuthReporter::ReportLog(REPORTLEVEL dwLevel,WCHAR *pwszFormat, ...)
{
    extern DWORD g_dwLogFileTrace;

    // Is the log level sufficent to continue?
    if (g_dwLogFileTrace < (DWORD)dwLevel)
        return (RPT_SUCCESS);   // No, don't log message

    // Call the CReporter Report function which will write this
    // this message to the log.
    va_list vaMarker;
    WCHAR wszBuf[BUFSIZ*2];
    va_start (vaMarker, pwszFormat);
    _vsnwprintf (wszBuf, BUFSIZ*2 - 1, pwszFormat, vaMarker);

    WCHAR wszTid[30];
    wsprintf(wszTid, L", tid=%x", GetCurrentThreadId());
    wcscat(wszBuf, wszTid);

    return (Report(m_ReporterRegId, dwLevel, wszBuf));
};

RPTSTATUS CModAuthReporter::ReportEvent (REPORTLEVEL dwLevel, DWORD dwEventId, WCHAR *pwszArgTypes, ... )
{
    // Log an event to the NT event log. The first pram is the event
    // category. The next param is the event identifier picked up from
    // the modmsgs.h file. The third param is a string with one char 
    // per argument to be logged, the char being 'S' for a string argument,
    // and 'I' for an int argument. The strlength of this param is the number 
    // of variable arguments. The final n args are the values to be inserted 
    // into the logged event.

    DWORD dwError = 0;

	WCHAR    **wszArgs = NULL;
	va_list   marker;

	// Init the variable argument list
	va_start ( marker, pwszArgTypes );
    // Number of args
    WORD wArgs = wcslen ( pwszArgTypes );

	// Alloc arg ptr array on stack
	wszArgs = (WCHAR **) alloca (sizeof(WCHAR *) * wArgs);

	for ( WORD n = 0; n < wArgs; n++ )
	    {
		 if ( pwszArgTypes[n] == 'S' )
		    {
			 wszArgs[n] = va_arg (marker, WCHAR *);
            }
		else // Else its an int
		    {
			 // Alloc space to sprintf into
			 wszArgs[n] = (WCHAR *) alloca ( 11*sizeof(WCHAR) );
			 wsprintf ( wszArgs[n], L"0x%x", va_arg(marker,DWORD) );
            }
        }

	return (Report(m_ReporterRegId,dwLevel,dwEventId,wArgs,wszArgs,dwError) );
}

RPTSTATUS CModAuthReporter::Register (LPCTSTR lpszRegistrationName, LPCTSTR lpszSourceName/*=NULL*/)
{
	return CReporter::Register(&m_ReporterRegId, lpszRegistrationName, lpszSourceName );
}