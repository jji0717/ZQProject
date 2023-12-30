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
// Ident : $Id: TrickImportUser.h,v 1.7 2004/08/03 02:19:18 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/TrickModule/TrickImportUser.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 2     04-12-08 15:50 Jie.zhang
// 
// 2     04-11-22 19:53 Jie.zhang
// seperate initialize()/uninitialize() steps from import()
// Revision 1.7  2004/08/03 02:19:18  jshen
// no message
//
// Revision 1.6  2004/07/29 06:21:00  jshen
// before release
//
// Revision 1.5  2004/07/22 01:45:58  jshen
// remove manual stop
//
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


#ifndef TRICK_IMPORT_USER_H
#define TRICK_IMPORT_USER_H

#include "../stdafx.h"

#include "Subfile.h"
#include "BufferPool.h"

#include <string>

using namespace std;

#define VOD_MAX_SUBFILE_COUNT		10

#ifdef V7VVX_CURRENT_MINORVERSION
typedef enum splice_type_enum {SPLICE_UNSPECIFIED=-1, SPLICE_DISABLE, SPLICE_ENABLE} SpliceType;
#endif

#ifndef STATUS_EVENT_DONE
#define STATUS_EVENT_DONE                ((NTSTATUS)0x40000012L)
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Video import thread parameter block
//
/////////////////////////////////////////////////////////////////////////////

class TrickFileXfer;
struct ImportThreadParams
{
	PVOID				context;
	int					bufferSize;
	CBufferPool			*srcBufferPool;
	CBufferPool			*outBufferPool;
	TCHAR				targetFilename[256];
	DWORD				finalStatus;
	void				(*logTextRoutine)(_TCHAR *text);
	DWORD				maxMpegCodingErrors;
};

/////////////////////////////////////////////////////////////////////////////
//
// Import object
//
/////////////////////////////////////////////////////////////////////////////

class CTrickImportUser
{
	friend class CSubfileContext;
public:
	CTrickImportUser(ImportThreadParams *param, DWORD dwAUID);
	~CTrickImportUser();


	enum
	{
		E_OK,
		E_MAYNOT_MPEG2,
		E_CREATEFILE,
		E_UNKNOWN
	};
	//
	// public interfaces
	//
	HRESULT Initialize();
	

	HRESULT Import();
	//
	// diagnostics
	//

	HRESULT UnInitialize();


	DWORD				GetIoQueueDepth(SubfileType type);
	LONGLONG			GetSubfileBytesWritten(SubfileType type);
	bool				GetSubfileState(SubfileType type, PULONG state, PULONG subState);
	void				DumpSubfileQueue(SubfileType j);
	//
	// interesting video characteristics
	//
	TRICK_CHARACTERISTICS	m_trickCharacteristics;
	//
	// counters
	//
	TRICK_COUNTERS			m_trickCounters;
	ULONG					m_flushCount;
	DWORD					m_pictureCount;
	ULONGLONG				m_sourceBytesRead;			// bytes read
	ULONGLONG				m_targetBytesWritten;		// bytes written

protected:
	inline void LogText(_TCHAR *buffer)
	{
		if (m_params->logTextRoutine)
			m_params->logTextRoutine(buffer);
	}

private:
	void GetTimes(PULONGLONG cpuTime, PULONGLONG clockTime);
	void CopySourceFile();
	bool CopyVvxHeaderInfo();
	HRESULT CreateSubfile(int index, SubfileType type, LONG speed, PSUBFILE_CONTEXT *);

	void CloseSubfiles();
	void CloseSubfile(PSUBFILE_CONTEXT subFile);

	//
	// these routines are called by the trick file library to acquire or release buffers
	//
	static PMPEG_BUFFER AcquireSourceMpegBuffer(PVOID context);
	static void			ReleaseSourceMpegBuffer(PVOID context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed);
	static PMPEG_BUFFER AcquireOutputMpegBuffer(PVOID context);
	static void			ReleaseOutputMpegBuffer(PVOID context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed);
	static void			FlushOutputMpegBuffer(PVOID	context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed);
	static void			LogMsgFromTrickLibrary(PVOID context, const char *format, ...);
	static PVOID		MemoryAllocate(PVOID context, ULONG size);
	static void			MemoryFree(PVOID context, PVOID buffer);

	//
	// flags
	//
	ULONG					m_enableSetFilePointer	: 1;
	ULONG					m_copyNormalFile		: 1;
	ULONG					m_atEndOfFile			: 1;
	ULONG					m_vvxFlushNeeded		: 1;

	TCHAR					m_targetFilename[256];			// import file name

	PTRICK_CONTEXT			m_trickContext;				// trick file library context
	ULONG					m_providerReadSize;			// buffer size

	ULONG					m_ffFileCount;				// count of ff files 
	ULONG					m_frFileCount;				// count of fr files
	//
	// subfile contexts
	//
	CSubfileContext			*m_normalFile;				// context for normal file
	CSubfileContext			*m_ffFile[VOD_MAX_SUBFILE_COUNT]; // context for ff files
	CSubfileContext			*m_frFile[VOD_MAX_SUBFILE_COUNT]; // context for fr files
	CSubfileContext			*m_vvxFile;					// context for vvx file

#ifdef V7VVX_CURRENT_MINORVERSION
	CSubfileContext		   *m_spliceFile;				// context for splice file
	SpliceType				m_spliceType;				// splice flags
	BOOLEAN					m_gradualTiming : 1;		// ...
	LONG					m_spliceMinGopSize;			// ...
	LONG					m_holdPictureCount;			// ...
#endif

    PVOID					(*m_mallocRoutine)(UINT size);
	void					(*m_freeRoutine)(PVOID buffer);
	long					m_refCount;
	ImportThreadParams		*m_params;
// helper

	DWORD					_dwAssetUID;		// current job's asset uid
};

/////////////////////////////////////////////////////////////////////////////
//
// Debug diagnostics
//
/////////////////////////////////////////////////////////////////////////////


inline void DbgString(_TCHAR *format, ...)
{
	_TCHAR msg[4096];
    va_list	ap;
    va_start(ap, format);
	_vstprintf(msg, format, ap);

	OutputDebugString(msg);
}


#endif