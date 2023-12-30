 
//
// Copyright (c) 1997, 2002 by
// SeaChange International, Inc., Maynard, Mass.
// All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of SeaChange International Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from SeaChange International Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without  notice and
// should not be construed as a commitment by SeaChange International Inc.
// 
// SeaChange  assumes  no  responsibility  for the use or reliability of its
// software on equipment which is not supplied by SeaChange.
// 
// RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
// Government is subject  to  restrictions  as  set  forth  in  Subparagraph
// (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
//

// To use this, make certain that your STDAFX.H has the following:
//
//				#include <afxmt.h>
//				#include <io.h>

//#include "stdafx.h"

#include "clog.h"

#ifndef _NO_FIX_LOG

//////////////////////////////////////////////////////////////////////////
// added by CARY
#include <afx.h>
#include <atlbase.h>
#include "common.h"

CSvcLog service_log;

void glog(ISvcLog::LogLevel level, const char* fmt, ...)
{
	if (service_log.getLevel() < level)
		return;

	char outbuf[2048];
	va_list vlist;

	static const char prefix[] = "[DataStream]";
	static const int prefixLen = sizeof(prefix) - 1;
	strcpy(outbuf, prefix);
	va_start(vlist, fmt);
	vsprintf(&outbuf[prefixLen], fmt, vlist);
	service_log.log0(level, outbuf);
	return;
}

void Clog( int level, const char* fmt, ...)
{
	char outbuf[2048];
	va_list vlist;

	ISvcLog::LogLevel logLevel;

	switch (level) {
	case LOG_NORECORD:
		logLevel = ISvcLog::L_DEBUG_DETAIL;
		break;

	case LOG_ERR:
		logLevel = ISvcLog::L_ERROR;
		break;

	case LOG_RELEASE:
		logLevel = ISvcLog::L_INFO;
		break;

	case LOG_DEBUG:	
		logLevel = ISvcLog::L_DEBUG;
		break;

	default:
		logLevel = ISvcLog::L_DEBUG_DETAIL;
		break;
	}

	if (service_log.getLevel() < level)
		return;

	static const char prefix[] = "[DataStream]";
	static int prefixLen = sizeof(prefix) - 1; // exclude the zero

	strcpy(outbuf, prefix);
	va_start(vlist, fmt);
	vsprintf(&outbuf[prefixLen], fmt, vlist);
	service_log.log0(logLevel, outbuf);
}

void ClogEstablishSettings(const TCHAR* strLogfileName, 
						   int dwClogTraceLevel, 
						   unsigned int dwLogFileSize)
{
	TCHAR logPath[MAX_PATH];
	BOOL r = GetModuleFileName(NULL, logPath, sizeof(logPath) / sizeof(TCHAR));
	if (!r) {
		ASSERT(r);
		return;
	}

	int len = lstrlen(logPath);
	TCHAR* c = logPath + len;
	while (*c != '\\') {
        c --;
	}
    lstrcpy(c + 1, strLogfileName);
	r = service_log.init(logPath, (CSvcLog::LogLevel )dwClogTraceLevel, 
		dwLogFileSize);
	if (!r) {
		ASSERT(FALSE);
		printf("Log init failed.\n");
		service_log.beep(CSvcLog::BEEP_INIT_FAILED);
	}
}

#else // #ifndef _NO_FIX_LOG
// added by CARY
//////////////////////////////////////////////////////////////////////////

#include "Afxdisp.h"
#include "Afxmt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

static DATE LocalGetNowAsDATE()
{
	// The static function 'COleDateTime::GetCurrentTime()' is brain dead,
	// in that it has a granularity of 1 second.  This routine is intended
	// to correct that situation

	SYSTEMTIME	st;
	FILETIME	ftNow;
	double msecPerDay = 60.0 * 60.0 * 24.0 * 1000.0;

	// NOTE : In the NVPak, this should be the ONLY place where we directly get the system
	//	 time, rather than getting it with a fixed offset through the special routine 
	//   'NVoD_GetSystemTimeAsFileTime'

	GetSystemTimeAsFileTime(&ftNow);
	VERIFY(FileTimeToSystemTime(&ftNow,&st));

	COleDateTime cDt(st);
    
	DATE dt = (DATE) cDt;

	// Now add in the milliseconds, which is the part that the constructor fails
	// to take into account

	dt += ((double) st.wMilliseconds) / msecPerDay;

	return(dt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum	ClogStyle_
	{
	eClog_InternalOnly,
	eClog_ExternalOnly,
	eClog_Both
	} ClogStyle;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static CCriticalSection			critClog;
static CStdioFile				g_cStdioFile;
static BOOL						g_logfile_initialized = FALSE;
static DWORD					g_dwClogTraceLevel;
static DWORD					g_dwLogSize;
static CString					g_stLogFileName;
static int						g_iClogIndent = 0;
static CClogThreadTxfrInterface	*g_ThrdTxfrInterface = NULL;
static ClogStyle				g_eCurrentClogStyle = eClog_Both;
static BOOL						g_bInsertBlankLinesOnDelays = TRUE;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static void Clog_wrapped( CString& stWrkBuf )
{
	SYSTEMTIME		St;
	BOOL			bLeadWithBlankLine = FALSE;

	// Adding a method of inserting eye-candy; blank lines whenever it has been longer
	// than 5 seconds since the last message was written

	static COleDateTime	dtLastWriteTimestamp((DATE)0.0);

	COleDateTime dtNowUTC = COleDateTime(LocalGetNowAsDATE());
	if (dtLastWriteTimestamp.m_dt != 0.0)
		{
		COleDateTimeSpan dtSpan = dtNowUTC - dtLastWriteTimestamp;
		if (dtSpan.GetTotalSeconds() > 5.0)
			bLeadWithBlankLine = TRUE;
		}

	dtLastWriteTimestamp = dtNowUTC;

	// First, create the output string.  The call to 'TrimRight()' insures that it does
	// NOT end with a newline.  Use the 'g_iClogIndent' value to form an indent string
	// that goes between the timestamp and the text

	CString	stFormattedOutput;
	CString	stTimestamp;
	CString	stIndentStr(' ',g_iClogIndent);

	GetLocalTime(&St);
	stTimestamp.Format(_T("%02d/%02d %02d:%02d:%02d.%03d"),St.wMonth,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds);
	stFormattedOutput.Format(_T("%s%s"),stIndentStr,stWrkBuf);
	stFormattedOutput.TrimRight();

	critClog.Lock();
		{
		// First lets try to write it to the file

		switch (g_eCurrentClogStyle)
			{
			case eClog_ExternalOnly :
			case eClog_Both :
				{
				if (g_logfile_initialized)
					if (g_cStdioFile.m_pStream != NULL)
						try
							{
							CString stWrkStr;

							// Read the first line, which is always the current position

							g_cStdioFile.SeekToBegin();

							if (g_cStdioFile.ReadString(stWrkStr))
								if (stWrkStr.GetLength() == 10)
									{
									DWORD dwPosition = _tcstoul((LPCTSTR)stWrkStr,NULL,10);

									if ((dwPosition + (DWORD)stFormattedOutput.GetLength()) > g_dwLogSize)
										{
										g_cStdioFile.SetLength(dwPosition);
										dwPosition = 12L;							// Set position to top message
										}
					
									// Move to the current position

									if (g_cStdioFile.Seek((LONG)dwPosition,CFile::begin))
										{
										CString stOutput;
										stOutput.Format(_T("%s %s\n"),(LPCTSTR)stTimestamp,(LPCTSTR)stFormattedOutput);
										if (bLeadWithBlankLine && g_bInsertBlankLinesOnDelays)
											stOutput = "\n" + stOutput;
										g_cStdioFile.WriteString(stOutput);
										}
									
									dwPosition = (DWORD)g_cStdioFile.Seek(0,CFile::current);  // New position

									g_cStdioFile.SeekToBegin();
									stWrkStr.Format(_T("%10lu\n"),dwPosition);
									g_cStdioFile.WriteString(stWrkStr);

									g_cStdioFile.Flush();
									}
							}
						catch (CFileException *cF)
							{
							cF->Delete();
							}
				break;
				}

			default :
				break;
			}

		// Now again, but with an internal focus.  Internal messages are meant for display in a window,
		// so they do not have the time stamps...

		switch (g_eCurrentClogStyle)
			{
			case eClog_InternalOnly :
			case eClog_Both :
				{
				// I have a special recognition of the string '*' as a blank
				// line, but only in 'eClog_InternalOnly' mode

				if (stFormattedOutput.Compare(_T("*")) == 0)
					if (g_eCurrentClogStyle == eClog_InternalOnly)
						stFormattedOutput = " ";		// One blank character...

				if (g_ThrdTxfrInterface != NULL)
					g_ThrdTxfrInterface->SubmitNextClogLine(stFormattedOutput);
				break;
				}
			default :
				break;
			}
		}
	critClog.Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void Clog( int iLevel, LPCTSTR pStrFmt, ...)
{
	va_list		vaArgList;
	BOOL		bErrorWasInFormatting = TRUE;
	CString		stWrkBuf;

	try
		{
		// Everything with a trace level of 0 is guaranteed to go through

		if ((DWORD)iLevel > g_dwClogTraceLevel)
			return;

		va_start(vaArgList,pStrFmt);
		stWrkBuf.FormatV(pStrFmt,vaArgList);
		va_end(vaArgList);

		// If no exception so far, any exception raised will be in the body of the next subproc

		bErrorWasInFormatting = FALSE;
		Clog_wrapped(stWrkBuf);
		}
	catch (...)
		{
		if (bErrorWasInFormatting)
			Clog( 0,_T("CLOG EXCEPTION, FORMATTING : Format Was '%s'"),pStrFmt);
		else
			Clog( 0,_T("CLOG EXCEPTION, POST-FORMATTING : Format Was '%s'"),pStrFmt);
		}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void ClogGroup( BOOL bTakeTheLock )
{
	// This routine allows for the reliable grouping of clog messages into 
	// consecutive lines of the log, independent of the number of threads attempting
	// to access the log.  It should be used with caution.
	//
	// NOTE : This MUST be called in pairs, with a TRUE then a FALSE, or it will lock
	//		  up the environment tighter than a drum.  Don't take any appreciable time
	//		  between the two calls

	ASSERT(g_logfile_initialized);
	if (!g_logfile_initialized)
		return;								// Caller must explicitly init

	if (bTakeTheLock)
		critClog.Lock();
	else
		critClog.Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static BOOL FileExists( CString stFileName )
{
	return(GetFileAttributes(stFileName) != 0xFFFFFFFF);
}
 
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static void	CreatePrecursorDirectory( CString stTargetDir )
{
	// I am trying to write a log file, but I first have to make certain that its 
	// directory structure exists

	if (stTargetDir.IsEmpty())
		return;

	// Lets see if this directory exists

	DWORD dwAttr = GetFileAttributes(stTargetDir);
	if (dwAttr == 0xFFFFFFFF)
		{
		DWORD dwStat = GetLastError();

		switch (dwStat)
			{
			case ERROR_PATH_NOT_FOUND :				// Recursion needed
				{
				TCHAR	tcDrive[_MAX_DRIVE];
				TCHAR	tcDir[_MAX_DIR];

				_tsplitpath(
						   stTargetDir,
						   tcDrive,
						   tcDir,
						   NULL,
						   NULL
						   );

				// Create the directory name that this directory resides in

				CString stDrive(tcDrive);
				CString	stDir(tcDir);
				if (!stDir.IsEmpty())
					stDir.Delete(stDir.GetLength() - 1);

				CString stFullDirName = stDrive + stDir;

				CreatePrecursorDirectory(stFullDirName);
				break;
				}

			case ERROR_FILE_NOT_FOUND :
				// The path exists, but the target directory doesn't.  This is the
				// proper end to the recursion...

				break;

			default :
				ASSERT(FALSE);
				break;
			}

		// Now create this one

		if (!CreateDirectory(stTargetDir,NULL))
			{
			dwStat = GetLastError();
			ASSERT(FALSE);
			}
		}
	else
		ASSERT((FILE_ATTRIBUTE_DIRECTORY & dwAttr) != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static void	InitClog()
{
	// Initialize the circular logging environment

	critClog.Lock();
		{
		ASSERT(!g_logfile_initialized);

		if (!g_logfile_initialized)			// Do it a second time in case two threads try to init
			{
			// There is significant risk that the directory in which I want to log does
			// not exist - this comes from the fact that we abandoned C:\cdci\log as a standard
			// log file location (along with any of the other choices make - c:\, c:\temp, etc).
			// This leaves me with no choice but to got to the trouble of creating the target
			// directory if it doesn't exist

			TCHAR	tcDrive[_MAX_DRIVE];
			TCHAR	tcDir[_MAX_DIR];

			_tsplitpath(
					   g_stLogFileName,
					   tcDrive,
					   tcDir,
					   NULL,
					   NULL
					   );

			// Create the directory...

			CString stDrive(tcDrive);
			CString	stDir(tcDir);
			if (!stDir.IsEmpty())
				stDir.Delete(stDir.GetLength() - 1);

			CString stFullDirName = stDrive + stDir;

			CreatePrecursorDirectory(stFullDirName);

			// Create the file, or open it without truncation.  If I end up with a zero
			// length file, then this was a create, so I need to write the first line

			UINT uiOpenFlags = CFile::modeCreate
							 | CFile::modeNoTruncate
				             | CFile::modeReadWrite
							 | CFile::shareDenyWrite
							 | CFile::typeText;

			if (g_cStdioFile.Open(g_stLogFileName,uiOpenFlags))
				{
				DWORD dwLength = g_cStdioFile.GetLength();
				if (dwLength == 0)
					{
					CString stPositionTag;
					stPositionTag.Format(_T("%10lu\n"),12L);
					g_cStdioFile.WriteString(stPositionTag);
					g_cStdioFile.Flush();
					}
				}
			
			g_logfile_initialized = TRUE;
			}
		}
	critClog.Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void ClogEstablishSettings(
						  CString	stLogfileName,
						  DWORD		dwClogTraceLevel,
						  DWORD		dwLogFileSize
						  )
{
	ASSERT(!g_logfile_initialized);
	if (g_logfile_initialized)
		return;								// Cannot alter on the fly

	g_stLogFileName		= stLogfileName;
	g_dwClogTraceLevel	= dwClogTraceLevel;
	g_dwLogSize			= dwLogFileSize;
	
	InitClog();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL	ClogSwitchLogFile( CString	stNewLogFileName )
{
	ASSERT(g_logfile_initialized);

	// Close out the prior log file, and attempt to open the new one.
	// If the new one fails, go back and open the old one (should work...)
	// and log the problem

	BOOL	bItWorked = FALSE;

	critClog.Lock();
		{
		Clog(0,_T("Attempting On-the-fly Log File Switch from here to '%s'..."),stNewLogFileName);

		CString	stPriorLogFile = g_stLogFileName;
		if (g_cStdioFile.m_pStream != NULL)
			g_cStdioFile.Close();
		g_logfile_initialized = FALSE;

		// Set up the new name and re-call the init function

		g_stLogFileName = stNewLogFileName;
		InitClog();

		// I will know that it worked if I now have a non-null 'g_logfile'

		if (g_cStdioFile.m_pStream != NULL)
			{
			Clog(0,_T("================================================================"));
			Clog(0,_T("Logging was started in file '%s'; switched on the fly to here..."),stPriorLogFile);
			bItWorked = TRUE;
			}
		else
			{
			// It failed for some reason - try to go back

			ClogSwitchLogFile(stPriorLogFile);
			Clog(0,_T("UNABLE TO SWITCH; logging will continue here..."));
			}
		}
	critClog.Unlock();
		
	return(bItWorked);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void	ClogAlterLogFileSize( DWORD dwNewSize )
{
	critClog.Lock();
		{
		// No smaller than 256K

		dwNewSize = max(dwNewSize,0x40000);
		g_dwLogSize	= dwNewSize;
		}
	critClog.Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void	ClogAlterLogLevel( DWORD dwNewLevel )
{
	critClog.Lock();
		{
		g_dwClogTraceLevel = dwNewLevel;
		}
	critClog.Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void	ClogAlterIndent( int iDelta )
{
	g_iClogIndent += iDelta;
	if (g_iClogIndent < 0)
		g_iClogIndent = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void	ClogSetThreadTxfrInterface( CClogThreadTxfrInterface *pInterface )
{
	// Call this function with the address of an instance of a class derived from 
	// CClogThreadTxfrInterface to get Clog to transfer all its messages internally
	// in addition to writing them to the log.  Call it with NULL to turn it off

	critClog.Lock();
		{
		g_ThrdTxfrInterface = pInterface;
		}
	critClog.Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void ClogSetForInternalLoggingOnly() 
{
	g_eCurrentClogStyle = eClog_InternalOnly;
}

void ClogSetForExternalLoggingOnly() 
{
	g_eCurrentClogStyle = eClog_ExternalOnly;
}

void ClogSetForStandardLogging() 
{
	 g_eCurrentClogStyle = eClog_Both;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

CClogThreadTxfrInterface::CClogThreadTxfrInterface()
{
	InitializeCriticalSection(&m_critArbitrator);
	m_slstClogLines.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

CClogThreadTxfrInterface::~CClogThreadTxfrInterface()
{
	DeleteCriticalSection(&m_critArbitrator);

	while (!m_slstClogLines.IsEmpty())
		{
		CString	*pSt = m_slstClogLines.RemoveHead();
		if (pSt != NULL)
			delete pSt;
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void	CClogThreadTxfrInterface::SubmitNextClogLine( CString stLine )
{
	// A thread (in practice, the Clog function called from a thread) calls this to 
	// give a line of output to another waiting thread

	EnterCriticalSection(&m_critArbitrator);
		{
		// Make certain the string does not end with trailing white space, including
		// carriage returns

		stLine.TrimRight();

		// Testing something.  Lets do this on the heap....

		int iLength = stLine.GetLength();

		CString *pSt = new CString("");

		if (iLength > 0)
			{
			LPTSTR pDest = pSt->GetBuffer(iLength);
			CopyMemory(pDest,(LPCTSTR)stLine,iLength);
			pSt->ReleaseBuffer(iLength);
			}

		m_slstClogLines.AddTail(pSt);

		// Now call the virtual notification function, but only call it if there is one and
		// only one line here.  If there are more, I HAVE ALREADY CALLED IT, and it has yet
		// to respond...

		if (m_slstClogLines.GetCount() == 1)
			OnDataAvailable();
		}
	LeaveCriticalSection(&m_critArbitrator);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL	CClogThreadTxfrInterface::RetrieveNextClogLine( CString& stLine )
{
	// Pull off the next string, or return FALSE if there is nothing there...

	BOOL	bGotALine = FALSE;

	EnterCriticalSection(&m_critArbitrator);
		{
		if (!m_slstClogLines.IsEmpty())
			{
			CString *pSt = m_slstClogLines.RemoveHead();
			stLine = *pSt;
			delete pSt;
			bGotALine = TRUE;
			}
		else
			stLine.Empty();
		}
	LeaveCriticalSection(&m_critArbitrator);

	return(bGotALine);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void	ClogBlankLineGeneration( BOOL bInsertThem )
{
	g_bInsertBlankLinesOnDelays = bInsertThem;
}

#endif // #ifndef _NO_FIX_LOG
