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
// $Log: /ZQProjs/TianShan/CPE/common/TrickImportUser.h $
// 
// 2     12/19/12 3:58p Hui.shao
//  PRINTFLIKE
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 7     08-03-28 16:13 Build
// 
// 6     08-03-27 16:18 Jie.zhang
// 
// 5     08-03-04 17:30 Jie.zhang
// 
// 4     08-02-28 16:35 Jie.zhang
// 
// 3     08-02-22 19:02 Jie.zhang
// 
// 2     08-02-20 16:39 Jie.zhang
// 
// 1     08-02-15 12:45 Jie.zhang
// 
// ===========================================================================

#ifndef TRICK_IMPORT_USER_H
#define TRICK_IMPORT_USER_H

#include "LibBuffer.h"
#include "BufferPool.h"
#include "NativeThread.h"
#include <string>
#include <deque>
#include "BaseClass.h"

#define PROCESS_TYPE_TRICKGEN	"TrickGen"


using namespace std;


#define V7VVX_CURRENT_MINORVERSION

namespace ZQTianShan {
	namespace ContentProvision {


//
// Subfile types
//
enum SubfileType
{
	SUBFILE_NORMAL=0,
		SUBFILE_FF,
		SUBFILE_FR,
		SUBFILE_VVX,
		SUBFILE_SPLICETRANSITION,
		
		// add new subfile types before here
		SUBFILE_MAXFILETYPES
};

class CTrickImportUser;

/////////////////////////////////////////////////////////////////////////////
//
// Base subfile objects
//
/////////////////////////////////////////////////////////////////////////////

struct SubfileContext
{
	CTrickImportUser	*consumerContext;
	SubfileType			type;				// type of file (ff, fr vvx)
	ULONG				index;				// create table index
	LONG				relativeSpeed;		// 1 = ff, -1 = fr, 0 = normal
	LARGE_INTEGER		fileOffset;
	ULONGLONG			runningByteOffset;
};

typedef struct SubfileContext SUBFILE_CONTEXT, *PSUBFILE_CONTEXT;


#define VOD_MAX_SUBFILE_COUNT		10

#ifdef V7VVX_CURRENT_MINORVERSION
typedef enum splice_type_enum {SPLICE_UNSPECIFIED=-1, SPLICE_DISABLE, SPLICE_ENABLE} SpliceType;
#endif

#ifndef STATUS_EVENT_DONE
#define STATUS_EVENT_DONE                ((NTSTATUS)0x40000012L)
#endif


class CTrickImportUser : public BaseProcess
{
	friend class CSubfileContext;
public:

	virtual bool Init();
	
	virtual void Close();
	
	virtual void endOfStream(){}

	virtual const char* GetName() {return PROCESS_TYPE_TRICKGEN;}
	
	virtual bool Receive(MediaSample* pSample, int nInputIndex = 0){return false;}
		
	virtual LONGLONG getProcessBytes(){return m_sourceBytesRead;}

	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}

	virtual void setMaxCodingError(int nMaxMpegCodingErr)
	{
		m_maxMpegCodingErrors = nMaxMpegCodingErr;
	}

	CTrickImportUser();
	virtual ~CTrickImportUser();

	//
	// public interfaces
	//
	bool Initialize();
	
	bool Import();

	void UnInitialize();
	
	LONGLONG GetSubfileBytesWritten(SubfileType type);
	bool GetSubfileState(SubfileType type, PULONG state, PULONG subState);
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

	virtual bool Run();
protected:
	
	
private:
	void GetTimes(PULONGLONG cpuTime, PULONGLONG clockTime);
	void CopySourceFile();
	bool CopyVvxHeaderInfo();

	bool CreateSubfiles();
	bool CreateSubfile(int index, SubfileType type, LONG speed);

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
	static void			LogMsgFromTrickLibrary(PVOID context, const char *format, ...)  PRINTFLIKE(2, 3);
	static PVOID		MemoryAllocate(PVOID context, ULONG size);
	static void			MemoryFree(PVOID context, PVOID buffer);

	CLibBuffer				m_bufferDescriptors;
	//
	// flags
	//
	ULONG					m_enableSetFilePointer	: 1;
	ULONG					m_copyNormalFile		: 1;
	ULONG					m_atEndOfFile			: 1;
	ULONG					m_vvxFlushNeeded		: 1;

	PTRICK_CONTEXT			m_trickContext;				// trick file library context
	ULONG					m_providerReadSize;			// buffer size
	
	ULONG					m_ffFileCount;				// count of ff files 
	ULONG					m_frFileCount;				// count of fr files
	//
	// subfile contexts
	//
	SUBFILE_CONTEXT			m_normalFile;				// context for normal file
	SUBFILE_CONTEXT			m_ffFile[VOD_MAX_SUBFILE_COUNT]; // context for ff files
	SUBFILE_CONTEXT			m_frFile[VOD_MAX_SUBFILE_COUNT]; // context for fr files
	SUBFILE_CONTEXT			m_vvxFile;					// context for vvx file

//	LONGLONG				m_outputOffset[VOD_MAX_SUBFILE_COUNT*2+2];			

#ifdef V7VVX_CURRENT_MINORVERSION
	SUBFILE_CONTEXT		    m_spliceFile;				// context for splice file
	SpliceType				m_spliceType;				// splice flags
	BOOLEAN					m_gradualTiming : 1;		// ...
	LONG					m_spliceMinGopSize;			// ...
	LONG					m_holdPictureCount;			// ...
#endif

    PVOID					(*m_mallocRoutine)(size_t size);
	void					(*m_freeRoutine)(PVOID buffer);
	
	std::string				m_strLastLog;				// some time, a same log will be output N times(N is a very big number)
	DWORD					m_trickFilesDebugLevel;	// 1 means enable, 0 disable debug, 0 is default
	DWORD					m_maxMpegCodingErrors;
};

}}

#endif