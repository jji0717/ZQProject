/*
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
** title to and ownership of the software is hereby transferred.7
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
*/
/*
**
** title:
**
** version:     V1.00
**
** facility:    Digital Ad Insertion
**
** abstract:
**          This file contains functions for the multiple circular log module
**
**
** 
**/

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <time.h>
#include <io.h>

#include "mclog.h"


// Globals, hidden 						   
DWORD       dwClogMutexTimout = 5000; // mutex timeout, usec

/////////////////////////////////////////////////////////////////////////////////////////
// ClogInit()
//
//  Initialize things required for circular logging
/////////////////////////////////////////////////////////////////////////////////////////

MCB_t* MClogInit(TCHAR *wszlogfile, int iTraceLevel, int iFileSize)
{
	BOOL bIsSuccess = FALSE;
	MCB_t *hHandle = NULL;
//	 char * pBuf = NULL;

	_try {

	/*	//convert the wchars to tchar
	    DWORD dwLen = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszlogfile, -1, NULL, 0, NULL, NULL);
        if(dwLen != 0)
		{
			pBuf = new char[dwLen+1];
        
			if (0 == WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszlogfile, -1, pBuf, dwLen, NULL, NULL) )
			{
              memset(pBuf, 0, dwLen+1);
			}
		}
       */


		// Allocate MClog block
		if ((hHandle = (MCB_t*)calloc(1, sizeof(MCB_t))) == NULL) _leave;
	  	
		// Try to create mutex for multithreaded logging
	  	if ((hHandle->hClogMutex = CreateMutex(NULL, FALSE, NULL)) == NULL) _leave;

		hHandle->iLogTraceLevel = iTraceLevel;  // Set initial logging level  
		hHandle->iLogFileSize = iFileSize; // Set logfile size

		// Create circular log
	  	if (_tcscmp(wszlogfile, _T("") ) != 0)
		{
		    if (_taccess(wszlogfile, 0) == -1) 
			{   // logging requested but file ain't there
		      if ((hHandle->fLogfp = _tfopen(wszlogfile, _T("w+")) ) == NULL) _leave;
		      setvbuf(hHandle->fLogfp, NULL, _IONBF, 0);     // Set no buffering on logfile
		      _ftprintf(hHandle->fLogfp,_T("%10lu\n"), 12L);     // put circular file position at top of file
		    } 
			else 
			{                    // logging requested and file is there
		      if ((hHandle->fLogfp = _tfopen(wszlogfile, _T("r+")) ) == NULL) _leave;
		      setvbuf(hHandle->fLogfp, NULL, _IONBF, 0);     // Set no buffering on logfile
		    }
		} else _leave;

		// Success
		bIsSuccess = TRUE;
													   
	} _finally 
	{
		if (!bIsSuccess)
		{
			if (hHandle)
			{
				if (hHandle->fLogfp) fclose(hHandle->fLogfp);
				if (hHandle->hClogMutex) CloseHandle(hHandle->hClogMutex);
				free(hHandle);
				hHandle = NULL;
			}
		}
	}

   return (hHandle);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MClogTerm()
//
/////////////////////////////////////////////////////////////////////////////////////////

BOOL MClogTerm(MCB_t *hHandle)
{
  if (hHandle)
  {
	  fclose(hHandle->fLogfp);
	  CloseHandle(hHandle->hClogMutex);
	  free(hHandle);
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MClogSetTraceLevel()
//
//  10/10/95 davidr - created 
//  Change logging trace level
/////////////////////////////////////////////////////////////////////////////////////////

BOOL MClogSetTraceLevel(MCB_t *hHandle, int iTraceLevel)
{
  if (hHandle) hHandle->iLogTraceLevel = iTraceLevel;  // Only change logging level if there is a log
  return TRUE;
}
 
/////////////////////////////////////////////////////////////////////////////////////////
// MClog()
//
//  5/22/95 CJH - stole this simplified logging routine from iusetup
//  This routine logs a text message if the file passed to it is open.
//  The message is logged in the 'standard' SeaChange circular file format and can 
//  be read with ctail.exe.
//  The function returns ERROR_SUCCESS if it succeeds, ERROR_FILE_NOT_FOUND if the log
//  file has not been opened, and the error code on error.
//  6/20/95 CJH - added trace level parameter to allow user to adjust the ammount of logging
//              - returns ERROR_INVALID_LEVEL if message trace level > level we're configured to log
//  10/12/95 DJR - added variable argument support
/////////////////////////////////////////////////////////////////////////////////////////
int MClog(MCB_t *hHandle, int iTraceLevel, TCHAR* wszMessage, ...) 
{

  int           iStatus;
  TCHAR          timbuf[32];
  unsigned long lPosition;
  DWORD         dwStatus; //5/22/95 CJH
  SYSTEMTIME    systime;
  va_list       vamarker;  
  TCHAR          szExpandedMsg[MAXLINE]; 
//  char*  pBuf   = NULL;

  if (hHandle == NULL)
    return ERROR_FILE_NOT_FOUND;

  if (iTraceLevel > hHandle->iLogTraceLevel)
    return ERROR_INVALID_LEVEL;

  //
       

  va_start (vamarker, wszMessage); // Handle variable format strings
  _vsntprintf(szExpandedMsg, (size_t) MAXLINE - 1, wszMessage, vamarker); // Expand user string

  GetLocalTime(&systime);                               // Current time
  
  //_stprintf(timbuf,_T("%02d/%02d %d:%02d:%02d"),systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond); // This is the old code
  _stprintf(timbuf,_T("%02d/%02d %d:%02d:%02d:%03d"),systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond,systime.wMilliseconds); // This is the old code

  
  // EnterCriticalSection(&csClog);
  dwStatus = WaitForSingleObject(hHandle->hClogMutex, dwClogMutexTimout);
  switch (dwStatus) 
  {
    case WAIT_FAILED:
      iStatus = GetLastError();
      return iStatus;
    case WAIT_TIMEOUT:
      return ERROR_SEM_TIMEOUT;
    case WAIT_ABANDONED:
      return ERROR_SEM_OWNER_DIED;
    default:
      break;
  }

  iStatus = fseek(hHandle->fLogfp,0L,SEEK_SET);           // go to beginning of file
  if (iStatus != 0) 
  {                           // error moving cursor
	return ERROR_SEEK_ON_DEVICE;
  }
  
  if (_filelength(_fileno(hHandle->fLogfp)) <= 12L)
  {
    _ftprintf(hHandle->fLogfp,_T("%10lu\n"),12L);       // put circular file position at top of file
    iStatus = fseek(hHandle->fLogfp,0L,SEEK_SET);         // go back to beginning of file
    if (iStatus != 0) 
	{                         // error moving cursor
      return ERROR_SEEK_ON_DEVICE;
    }
  }
  iStatus = _ftscanf(hHandle->fLogfp,_T("%lu"),&lPosition);     // get position for new message
  if (iStatus == EOF) 
  {                         // error reading file
	return ERROR_FILE_CORRUPT;
  }
  // don't want to make file too big or overwrite the position if position gets corrupted
  if ((lPosition < 12L) || (lPosition > (unsigned long)hHandle->iLogFileSize))
  {
    lPosition = 12L;
  }
  else if ((lPosition + sizeof(timbuf) + sizeof(wszMessage)) > (unsigned long)hHandle->iLogFileSize)
  {  // message wont fit at end
    _chsize(_fileno(hHandle->fLogfp),lPosition);          // move EOF to end of last message
    lPosition = 12L;                            // set position to top message
  }
  iStatus = fseek(hHandle->fLogfp,lPosition,SEEK_SET);    // go to position for new message
  if (iStatus != 0) 
  {                           // error moving cursor
  	return ERROR_SEEK_ON_DEVICE;
  }
  _ftprintf(hHandle->fLogfp,_T("%s %s\n"), timbuf, szExpandedMsg);  ////// write message in logfile //////
  lPosition = ftell(hHandle->fLogfp);                     // get new cursor position
  if (lPosition == -1)
  {                        // error getting cursor position
  	return ERROR_FILE_CORRUPT;
  }
  iStatus = fseek(hHandle->fLogfp,0L,SEEK_SET);           // go to beginning of file
  if (iStatus != 0) 
  {                           // error moving cursor
  	return ERROR_SEEK_ON_DEVICE;
  }
  _ftprintf(hHandle->fLogfp,_T("%10lu\n"),lPosition);           // write new cursor position
  // 5/22/95 CJH - change critical section to mutex for timeout capabilities
  // LeaveCriticalSection(&csClog);
  ReleaseMutex(hHandle->hClogMutex);
  return ERROR_SUCCESS;
}
