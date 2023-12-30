
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


#pragma once

//////////////////////////////////////////////////////////////////////////
// added by Cary
#ifndef _NO_FIX_LOG
#include <afx.h>
#include <tchar.h>
#include "svclog.h"

extern CSvcLog service_log;

void glog(ISvcLog::LogLevel level, const char* fmt, ...);
void Clog( int level, LPCTSTR fmt, ...);
void ClogEstablishSettings(const TCHAR* strLogfileName, 
						   int dwClogTraceLevel, 
						   unsigned int dwLogFileSize);

#else // #ifndef _NO_FIX_LOG
#define glog
// added by Cary
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//#include "DeviceInfo.h"

#include <afxtempl.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef CTypedPtrList<CPtrList,CString *>	CStringPtrList;

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// This class is used if you want to pass Clog messages (a line at a time) between
// two threads.  One has to override this class, since the 'OnDataAvailable()' member
// function is pure virtual

class CClogThreadTxfrInterface
{
public:
	CClogThreadTxfrInterface();
	~CClogThreadTxfrInterface();

private:
	CRITICAL_SECTION	m_critArbitrator;
//	CCriticalSection	m_critArbitrator;
	CStringPtrList		m_slstClogLines;

public:
	void	SubmitNextClogLine( CString stLine );
	BOOL	RetrieveNextClogLine( CString& stLine );

public:
	virtual void	OnDataAvailable() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void Clog( int iLevel, LPCTSTR pStrFmt, ...);
extern "C" void ClogGroup( BOOL bTakeTheLock );
extern "C" void ClogEstablishSettings(
									 CString	stLogfileName,
									 DWORD		dwClogTraceLevel,
									 DWORD		dwLogFileSize
									 );
extern "C" BOOL	ClogSwitchLogFile( CString	stNewLogFileName );
extern "C" void	ClogAlterLogFileSize( DWORD dwNewSize );
extern "C" void	ClogAlterLogLevel( DWORD dwNewLevel );
extern "C" void	ClogAlterIndent( int iDelta );
extern "C" void ClogSetThreadTxfrInterface( CClogThreadTxfrInterface *pInterface );
extern "C" void ClogSetForInternalLoggingOnly();
extern "C" void ClogSetForExternalLoggingOnly();
extern "C" void ClogSetForStandardLogging();
extern "C" void	ClogBlankLineGeneration( BOOL bInsertThem );

#endif // #ifndef _NO_FIX_LOG
