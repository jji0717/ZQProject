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
// Ident : $Id: NtFileIo.h,v 1.4 2004/07/29 06:21:13 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/TrickModule/NtFileIo.h $
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
// 
// 2     04-11-22 19:47 Jie.zhang
// Revision 1.4  2004/07/29 06:21:13  jshen
// before release
//
// Revision 1.3  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.2  2004/07/05 02:19:41  jshen
// add comments
//
// Revision 1.1  2004/06/07 09:19:43  jshen
// copied to production tree
//
// ===========================================================================

#ifndef NTFILEIO_H
#define NTFILEIO_H
#include "Subfile.h"


/////////////////////////////////////////////////////////////////////////////
//
//  Object to write to Ntfs
//
/////////////////////////////////////////////////////////////////////////////

class CNtFileIo : public CSubfileContext
{
public:
	/////////////////////////////////////////////////////////////////////////////
	//
	// Constructor and Destructor
	//
	/////////////////////////////////////////////////////////////////////////////

	CNtFileIo(ULONG index, SubfileType type, LONG speed, CTrickImportUser *context)
		: CSubfileContext(index, type, speed, context)
	{
	}

	~CNtFileIo() 
	{
#ifdef _DEBUG
		DbgString(_T("~CNtFileIo %s\n"), filename);
#endif
		
		CloseSubfile(); 
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Create a new file
	//
	/////////////////////////////////////////////////////////////////////////////

	bool CreateSubfile()
	{
//		DWORD flags = (type == SUBFILE_VVX) ? 0 : FILE_FLAG_NO_BUFFERING;
		hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			finalStatus = GetLastError();
			_stprintf(strMsg, _T("Error %d creating file %s"), finalStatus, filename);
			return 0;
		}

		return 1;
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Update the vvx header
	//
	/////////////////////////////////////////////////////////////////////////////

	bool WriteVvxHeader(DWORD count, PBYTE buf)
	{
		int status;
		LARGE_INTEGER off1, off2;
		off1.QuadPart = off2.QuadPart = 0;

		off1.LowPart = SetFilePointer(hFile, off2.LowPart, &off1.HighPart,FILE_CURRENT);
		SetFilePointer(hFile, off2.LowPart, &off2.HighPart, FILE_BEGIN);
		
		status = WriteSubfile(count, buf);
		
		SetFilePointer(hFile, off1.LowPart, &off1.HighPart, FILE_BEGIN);

		return status != 0;
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	//  close a file
	//
	/////////////////////////////////////////////////////////////////////////////

	bool CloseSubfile() 
	{ 
		if (hFile != INVALID_HANDLE_VALUE)
			return CloseHandle(hFile) != 0; 

		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	//  FlushSubFile -  Flush operations write at the current byte offset and keep the 
	//					same byte offset. Flushes are used for the VVX file to
	//					allow multiple writes of the same byte offset with more and
	//					more data
	//
	/////////////////////////////////////////////////////////////////////////////

	bool FlushSubfile(DWORD count, PBYTE buf)
	{
		DWORD amountWritten;
		if (!WriteFile(hFile, buf, count, &amountWritten, 0))
		{
			finalStatus = GetLastError();
			_stprintf(strMsg, _T("Flush write error %d to file %s"), finalStatus, filename);
			DbgString(strMsg);
			return 0;
		}
		SetFilePointer(hFile, fileOffset.LowPart, &fileOffset.HighPart, FILE_BEGIN);
		return 1;
	}

protected:

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Write at the current file byte offset
	//
	/////////////////////////////////////////////////////////////////////////////

	DWORD WriteSubfile(DWORD count, PBYTE buf)
	{
		DWORD amountWritten;

		if (!WriteFile(hFile, buf, count, &amountWritten, 0))
		{
			finalStatus = GetLastError();
			_stprintf(strMsg, _T("Error %d writing file %s"), finalStatus, filename);
			DbgString(_T("%s\n"), strMsg);
			return 0;
		}
		return amountWritten;
	}
};

#endif