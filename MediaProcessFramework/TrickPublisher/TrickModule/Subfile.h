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
// Ident : $Id: Subfile.h,v 1.4 2004/07/06 07:20:43 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/TrickModule/Subfile.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 1     04-12-03 13:56 Jie.zhang
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:19:41  jshen
// add comments
//
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:19:43  jshen
// copied to production tree
//
// ===========================================================================

#ifndef SUBFILE_H
#define SUBFILE_H
#include "LibQueue.h"


#define V7VVX_CURRENT_MINORVERSION
//
// Subfile types
//
enum SubfileType
{
	SUBFILE_NORMAL,
	SUBFILE_FF,
	SUBFILE_FR,
	SUBFILE_VVX
#ifdef V7VVX_CURRENT_MINORVERSION
	,SUBFILE_SPLICETRANSITION
#endif
};
//
// subfile states
//
enum ESubfileState	{SUBFILE_INITIALIZING, SUBFILE_RUNNING, SUBFILE_STOPPED};
enum ESubState		{SUBSTATE_QUEUE, SUBSTATE_WRITE};

class CTrickImportUser;

/////////////////////////////////////////////////////////////////////////////
//
// Base subfile objects
//
/////////////////////////////////////////////////////////////////////////////

class CSubfileContext
{
public:
	CSubfileContext(
			ULONG				consumerIndex, 
			SubfileType			sfType, 
			LONG				speed, 
			CTrickImportUser	*context);

	LONGLONG BytesWritten() { return fileOffset.QuadPart; };
	virtual ~CSubfileContext();
	//
	// virtual functions supplied by file system specific objects
	//
	virtual bool CloseSubfile() = 0;
	virtual bool CreateSubfile() = 0;
	virtual bool FlushSubfile(DWORD count, PBYTE buf) = 0;
	virtual	bool WriteVvxHeader(DWORD count, PBYTE buf) = 0;

	DWORD				state;
	DWORD				subState;
	DWORD				finalStatus;		// == ERROR_SUCCESS
	ULONGLONG			runningByteOffset;	// 
	TCHAR				filename[512];
	TCHAR				strMsg[512];
	long				refCount;
	CLibQueue			ioQueue;
	CTrickImportUser	*consumerContext;
	HANDLE				hStartEvent;
	HANDLE				hStopEvent;
	long				queueDepth;

protected:
	virtual DWORD WriteSubfile(DWORD count, PBYTE buf) = 0;

	static unsigned int __stdcall StartIoThread(PVOID context);
	void ProcessWorkQueue();

	HANDLE				hFile;
	SubfileType			type;				// type of file (ff, fr vvx)
	ULONG				index;				// create table index
	LONG				relativeSpeed;		// 1 = ff, -1 = fr, 0 = normal
	LARGE_INTEGER		fileOffset;

private:
    HANDLE              m_hThread;
    unsigned int        m_uiThreadId;
};

typedef class CSubfileContext SUBFILE_CONTEXT, *PSUBFILE_CONTEXT;

#endif