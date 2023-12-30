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
// Ident : $Id: TrickImportUser.cpp,v 1.8 2004/08/12 09:40:42 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/common/TrickImportUser.cpp $
// 
// 2     12/12/13 1:45p Hui.shao
// %lld
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 11    08-12-16 15:27 Jie.zhang
// 
// 10    08-03-28 16:13 Build
// 
// 9     08-03-27 16:18 Jie.zhang
// 
// 10    08-03-07 18:50 Jie.zhang
// 
// 9     08-03-07 18:37 Jie.zhang
// 
// 8     08-03-04 18:13 Jie.zhang
// 
// 8     08-03-04 17:30 Jie.zhang
// 
// 7     08-02-28 16:35 Jie.zhang
// 
// 6     08-02-22 19:02 Jie.zhang
// 
// 5     08-02-21 18:31 Jie.zhang
// in ReleaseSourceMpeg, we could not use byteconsumed, because the last
// time is 0, so the filesize smaller than original one.
// 
// 4     08-02-21 14:20 Jie.zhang
// 
// 3     08-02-20 16:39 Jie.zhang
// 
// 2     08-02-18 18:25 Jie.zhang
// changes check in
// 
// 1     08-02-15 12:45 Jie.zhang
// 
// ===========================================================================

#include "StdAfx.h"
#include "Log.h"

#include "TrickImportUser.h"
#include "trickfilesmessages.h"
#include "BufferPool.h"
#include <set>

#pragma comment (lib, "TrickFilesLibraryUser.lib")
#pragma comment (lib, "MpegLibraryUser.lib")



#define TrickLib			"TrickLib"

using namespace ZQ::common;

#define MOLOG (*_pLog)
#define SMOLOG (*(This->_pLog))


namespace ZQTianShan {
	namespace ContentProvision {

extern std::string getErrMsg(DWORD);


/////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
/////////////////////////////////////////////////////////////////////////////

CTrickImportUser::CTrickImportUser()
	: m_sourceBytesRead(0), 
	m_atEndOfFile(0), m_targetBytesWritten(0), 
	m_flushCount(0),
	m_bufferDescriptors(sizeof(SDataBuffer)),
#ifdef V7VVX_CURRENT_MINORVERSION
	#ifdef CreateSplicingFile
	m_spliceType(SPLICE_ENABLE)
	#else
	m_spliceType(SPLICE_DISABLE)
	#endif
	, m_spliceMinGopSize(3), m_holdPictureCount(6), m_gradualTiming(1) 

#endif
{
	m_ffFileCount = 1;
	m_frFileCount = 1;

	_nOutputCount = 4;
	_nInputCount = 1;

	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}
	for(i=0;i<_nOutputCount;i++)
	{
		OutputPin pin;
		pin.nNextPin = 0;
		pin.pNextFilter = 0;		
		_outputPin.push_back(pin);
	}
	_bDriverModule = true;

	m_mallocRoutine = malloc;
	m_freeRoutine	= free;

	memset(&m_normalFile, 0, sizeof(m_normalFile));
	memset(&m_vvxFile, 0, sizeof(m_vvxFile));
	memset(&m_spliceFile, 0, sizeof(m_spliceFile));
	
	memset(m_ffFile, 0, sizeof(m_ffFile));
	memset(m_frFile, 0, sizeof(m_frFile));
	memset(&m_trickCharacteristics, 0, sizeof(m_trickCharacteristics));
	memset(&m_trickCounters, 0, sizeof(m_trickCounters));

	m_vvxFlushNeeded = 0;
	m_trickFilesDebugLevel = 0;
	m_maxMpegCodingErrors = 0;
	m_trickContext = 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
/////////////////////////////////////////////////////////////////////////////

CTrickImportUser::~CTrickImportUser()
{
	UnInitialize();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Start an import
//
/////////////////////////////////////////////////////////////////////////////

bool CTrickImportUser::Import()
{
	DWORD status = STATUS_SUCCESS;	

	MOLOG(Log::L_INFO, CLOGFMT(TrickLib, "[%s] Import entered"), _strLogHint.c_str());

	//
	// process every GOP in the file
	//
	for (m_pictureCount = 0 ; NT_SUCCESS(status) && !_bStop ; m_pictureCount++)
	{
		//
		// call library to process 1 I Frame
		//
		status = TrickFilesProcessNextPicture(m_trickContext);
	
		//
		// check for completion code
		//
		if (status == STATUS_EVENT_DONE)
		{
			status = STATUS_SUCCESS;
			break;
		}
		else if (!NT_SUCCESS(status))
			break;

		//
		// see if a vvx update has been requested
		//
		// A VVX update is needed to keep the index and the source file
		// and speed files written in synchronization in order for a
		// video to be played while trick files are being created.
		//
		// See also ReleaseOutputMpegBuffer routine
		//
		if (m_vvxFlushNeeded)
		{
			//
			// update index records and re-write the header
			//
			TrickFilesUpdateVvxIndexHeader(m_trickContext);
			CopyVvxHeaderInfo();

			m_vvxFlushNeeded = 0;
			m_flushCount++;
  		}
	}

	//
	// verify success status
	//
	//even if exceed max coding error, let it to finish it
//	if (NT_SUCCESS(status))
	{
		//
		// re-write the header
		//
		TrickFilesUpdateVvxIndexHeader(m_trickContext);
		TrickFilesReleaseIndexMpegBuffers(m_trickContext);
		CopyVvxHeaderInfo();
	}
	//
	// get a copy of the counters
	//
	memcpy(&m_trickCounters,
		TrickFilesGetCounters(m_trickContext),
		sizeof(m_trickCounters));

	{
		MOLOG(Log::L_INFO, CLOGFMT(TrickLib, "[%s] %lld bytes processed,"
			"%d pictures decoded,"
			"%d pictures encoded,"
			"%d pictures deinterlaced,"
			"%d pictures with coding errors detected"),
			_strLogHint.c_str(),
			m_trickCounters.bytesProcessed,
			m_trickCounters.picturesDecoded,
			m_trickCounters.picturesEncoded,
			m_trickCounters.picturesDeinterlaced,
			m_trickCounters.picturesWithCodingErrors);
	}

	if (!NT_SUCCESS(status))
	{	
		char tmp[256];			
		if (status == TFS_EXCEEDED_MAX_CODING_ERRORS)
		{		
			sprintf(tmp, "Current coding error count [%d], exceeded max coding error %d, trick files generation stopped", 
				m_trickCounters.picturesWithCodingErrors, m_maxMpegCodingErrors);
			MOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] %s"), _strLogHint.c_str(), tmp);
		}
		else
		{
			sprintf(tmp, "Current coding error count [%d], TrickFilesProcessNextPicture failed with errorcode[0x%08x], trick files generation stopped", 
				m_trickCounters.picturesWithCodingErrors, m_maxMpegCodingErrors);
			MOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] %s"), _strLogHint.c_str(), tmp);
		}

		SetLastError(tmp);
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(TrickLib, "[%s] Import success"), _strLogHint.c_str());
	return true;	
}

/////////////////////////////////////////////////////////////////////////////
//
//  AcquireSourceMpegBuffer
//  This routine is called by the trick library when more mpeg data is 
//	needed.
//
//	Returns
//
//		== 0	End of file or error happened
//		!= 0	new MpegBuffer containing mpeg data
//
/////////////////////////////////////////////////////////////////////////////

PMPEG_BUFFER CTrickImportUser::AcquireSourceMpegBuffer(PVOID context)
{
	CTrickImportUser	*This		= (CTrickImportUser *)context;
	SDataBuffer			*buf		= 0;
	PMPEG_BUFFER		mpegBuffer	= 0;
	PSUBFILE_CONTEXT	subFile		= &This->m_normalFile;

	//
	// Allocate a data buffer
	//

	if (This->_bStop)
	{
		//
#pragma message(__MSGLOC__"TODO: log")
		return NULL;
	}

	InputPin&  pin = This->_inputPin[0];	
	MediaSample* pSample = pin.pPrevFilter->GetData(pin.nPrevPin);
	if (!pSample)
	{
		//
#pragma message(__MSGLOC__"TODO: log")
		return NULL;
	}

	buf	= (SDataBuffer*)This->m_bufferDescriptors.Get();
	buf->mpegBuffer.pointer		= (unsigned char*)pSample->getPointer();
	buf->len					= pSample->getDataLength();
	buf->mpegBuffer.length		= pSample->getDataLength();
	buf->offset					= 0;
	buf->reserve				= pSample;
	mpegBuffer					= &buf->mpegBuffer;
	//
	// format MPEG_BUFFER object with
	// the data buffer pointer, length
	// and file byte offset for this buffer
	//
	MpegLibraryInitializeBuffer(
			&buf->mpegBuffer,  This, This->m_sourceBytesRead,
			buf->len, buf->mpegBuffer.pointer);

	//
	// prepare file byte offset for next data buffer
	//
	This->m_sourceBytesRead += buf->len;

//  	static int aa=0;
//  	aa++;
//  	SMOLOG(Log::L_DEBUG, CLOGFMT(TrickLib, "[%s] 1 time[%d] pointer[%08x]"), This->_strLogHint.c_str(), aa, mpegBuffer);
	return mpegBuffer;	
}

/////////////////////////////////////////////////////////////////////////////
//
//  AcquireOutputMpegBuffer
//	This routine is called by the trick file library when it needs more buffers for
//	output data to the trick files and/or vvx file.
//
/////////////////////////////////////////////////////////////////////////////

PMPEG_BUFFER CTrickImportUser::AcquireOutputMpegBuffer(PVOID context)
{
	PMPEG_BUFFER		mpegBuffer	= 0;
	PSUBFILE_CONTEXT	subFile		= (PSUBFILE_CONTEXT)context;
	

	{
		CTrickImportUser *This	= (CTrickImportUser *)subFile->consumerContext;
		//
		// Allocate a data buffer
		//
		MediaSample* pSample = This->GetGraph()->allocMediaSample();
		if (!pSample)
		{
			//
#pragma message(__MSGLOC__"TODO: log")
			return NULL;
		}
		
		SDataBuffer* buf	= (SDataBuffer*)This->m_bufferDescriptors.Get();
		buf->mpegBuffer.pointer		= (unsigned char*)pSample->getPointer();
		buf->mpegBuffer.length		= pSample->getBufSize();
		buf->len					= pSample->getBufSize();
		buf->offset					= 0;
		buf->reserve				= pSample;
		mpegBuffer					= &buf->mpegBuffer;
		
		//
		// format MPEG_BUFFER object with
		// the data buffer pointer, length
		// and file byte offset for this buffer
		//
		MpegLibraryInitializeBuffer(
				&buf->mpegBuffer,  subFile, subFile->runningByteOffset,
				pSample->getBufSize(), buf->mpegBuffer.pointer);
			//
			// prepare file byte offset for next data buffer
			//
		subFile->runningByteOffset += mpegBuffer->length;		
	}

	return (mpegBuffer);
}

/////////////////////////////////////////////////////////////////////////////
//
//  ReleaseSourceMpegBuffer
//
//	This routine is called by the trick file library when it has completed processing
//	of the indicated mpeg data buffer.
//
//	The buffer will likely have been altered so it needs to be written to the output
//	mpeg target file
//
//	Note: bytesConsumed indicates the amount of video bytes contained in the buffer 
//	and should not be used as a byte count of the data.
//
/////////////////////////////////////////////////////////////////////////////

VOID CTrickImportUser::ReleaseSourceMpegBuffer(
			PVOID			context,
			PMPEG_BUFFER	mpegBuffer,
			ULONG			bytesConsumed)
{
	CTrickImportUser	*This = (CTrickImportUser *)context;

// 	static int aa=0;
// 	aa++;
// 	SMOLOG(Log::L_DEBUG, CLOGFMT(TrickLib, "[%s] 2 time[%d] pointer[%08x]"), This->_strLogHint.c_str(), aa, mpegBuffer);
	
	//
	// get pointer to sub file structure
	//
	ULONG				amountWritten = 0;
	int					status = 1;

	if (!mpegBuffer)
	{
		//
		// invalid mpeg buffer pointer
		//
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "ReleaseSourceMpegBuffer: released mpeg buffer pointer is 0"));
		return;
	}


	//
	// compute the data buffer pointer which is relative to the address of
	// the mpeg buffer structure
	//
	SDataBuffer *buf = (SDataBuffer *)((PUCHAR)mpegBuffer - offsetof(SDataBuffer, mpegBuffer));
	MediaSample* pSample = (MediaSample*)buf->reserve;

	if (buf->len)
	{
		pSample->setDataLength(buf->len);
		pSample->setOffset(This->m_normalFile.fileOffset.LowPart, This->m_normalFile.fileOffset.HighPart);
		This->m_normalFile.fileOffset.QuadPart += buf->len;
	
		OutputPin& pin = This->_outputPin[0];
// 		static int aa=0;
// 		aa++;
// 		SMOLOG(Log::L_DEBUG, CLOGFMT(TrickLib, "[%s] Receive time[%d] pointer[%08x], %d"), This->_strLogHint.c_str(), aa, mpegBuffer,This->m_normalFile.fileOffset.LowPart);

		if (!pin.pNextFilter->Receive(pSample, pin.nNextPin))
		{
			SMOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] Next filter[%s] failed to Receive data"), This->_strLogHint.c_str(), pin.pNextFilter->GetName());
		
			//
			//release the sample
			This->GetGraph()->freeMediaSample(pSample);
		}		
	}
	else
	{
		This->GetGraph()->freeMediaSample(pSample);
		SMOLOG(Log::L_WARNING, CLOGFMT(TrickLib, "[%s] ReleaseSourceMpegBuffer: released source buffer with length 0"), This->_strLogHint.c_str());
	}

	This->m_bufferDescriptors.Free(buf);		
}

/////////////////////////////////////////////////////////////////////////////
//
//  FlushOutputMpegBuffer
//
//	This routine is called by the trick file library when new records are being added
//	to the vvx file.  the write of accumulated Vvx records is triggered by ReleaseOutputMpegBuffer
//	when it detects that a FR speed file is being released.  FF and FR files are updated at the 
//	same time so that's the only time that a vvx file and the speed files are synchronized with
//	respect to each other (i.e. all vvx records point to actual speed file data).
//
//	This routine is unlike ReleaseOutputMpegBuffer in that the data buffer being writting is still
//	in use by the trickfile library.
//
//	bytesConsumed is the actual byte count of the data buffer that is being written and may be
//	less than the total byte count of the buffer.
//
/////////////////////////////////////////////////////////////////////////////

void CTrickImportUser::FlushOutputMpegBuffer(
			  PVOID	context,
			  PMPEG_BUFFER			mpegBuffer,
			  ULONG					bytesConsumed)
{
	PSUBFILE_CONTEXT		subFile = (PSUBFILE_CONTEXT) context;
	CTrickImportUser		*This = subFile->consumerContext;	

	if (!mpegBuffer)
	{
		//
		// invalid mpeg buffer pointer
		//
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "FlushOutputMpegBuffer: released mpeg buffer pointer is 0"));
		return;
	}

	SDataBuffer *buf = (SDataBuffer *)((PUCHAR)mpegBuffer - offsetof(SDataBuffer, mpegBuffer));
	buf->len = bytesConsumed;
	if (bytesConsumed)
	{
#if 1
		MediaSample* pSample = (MediaSample*)buf->reserve;
		pSample->setDataLength(bytesConsumed);
		pSample->setOffset(subFile->fileOffset.LowPart,subFile->fileOffset.HighPart);	//don't change the offset for Flush operation		
		MediaSample* pSample1 = This->GetGraph()->allocMediaSample();
		memcpy(pSample1->getPointer(), pSample->getPointer(), bytesConsumed);
		pSample1->setDataLength(bytesConsumed);
		pSample1->setOffset(subFile->fileOffset.LowPart,subFile->fileOffset.HighPart);	//don't change the offset for Flush operation		
		
		OutputPin& pin = This->_outputPin[3];
		if (!pin.pNextFilter->Receive(pSample1, pin.nNextPin))
		{
#pragma message(__MSGLOC__"TODO: log")
			
			//
			//release the sample
			This->GetGraph()->freeMediaSample(pSample1);
		}		
#else
		MediaSample* pSample = (MediaSample*)buf->reserve;
		pSample->setDataLength(bytesConsumed);
		pSample->setOffset(subFile->fileOffset.LowPart,subFile->fileOffset.HighPart);	//don't change the offset for Flush operation		
		
		OutputPin& pin = This->_outputPin[3];
		if (!pin.pNextFilter->Receive(pSample, pin.nNextPin))
		{
#pragma message(__MSGLOC__"TODO: log")
			
			//
			//release the sample
			This->GetGraph()->freeMediaSample(pSample);
		}		

#endif
	}
	else
	{
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "FlushOutputMpegBuffer: flush buffer with 0 data length"));
	}
	
//	This->m_bufferDescriptors.Free(buf);		
}

/////////////////////////////////////////////////////////////////////////////
//
//  ReleaseOutputMpegBuffer
//
//	This routine is called by the trick file library when it is releasing a vvx or trick
//	file (ff,fr) buffer to be written to disk.
//
//	bytesConsumed is the actual byte count of the data buffer.
//
/////////////////////////////////////////////////////////////////////////////

void CTrickImportUser::ReleaseOutputMpegBuffer(
			PVOID			context,
			PMPEG_BUFFER	mpegBuffer,
			ULONG			bytesConsumed)
{
	SUBFILE_CONTEXT*		subFile = (SUBFILE_CONTEXT*) context;
	ULONG					amountWritten = 0;
	int						status = 1;
	CTrickImportUser*		This=subFile->consumerContext;
	
	if (!mpegBuffer)
	{
		//
		// invalid mpeg buffer pointer
		//
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "ReleaseOutputMpegBuffer: released mpeg buffer pointer is 0"));
		return;
	}

	SDataBuffer *buf = (SDataBuffer *)((PUCHAR)mpegBuffer - offsetof(SDataBuffer, mpegBuffer));
	buf->len = bytesConsumed;
	if (bytesConsumed)
	{
		MediaSample* pSample = (MediaSample*)buf->reserve;
		pSample->setDataLength(bytesConsumed);		
		pSample->setOffset(subFile->fileOffset.LowPart, subFile->fileOffset.HighPart);
		subFile->fileOffset.QuadPart += bytesConsumed;
		
		int nIndex;
		if (subFile->type==SUBFILE_FF)
			nIndex = 1;
		else if (subFile->type==SUBFILE_FR)
			nIndex = 2;
		else
			nIndex = 3;

		OutputPin& pin = This->_outputPin[nIndex];
		if (!pin.pNextFilter->Receive(pSample, pin.nNextPin))
		{
#pragma message(__MSGLOC__"TODO: log")
			
			//
			//release the sample
			This->GetGraph()->freeMediaSample(pSample);
		}		

		if (subFile->type == SUBFILE_FR)
		{
			This->m_vvxFlushNeeded = 1;
		}
	}
	else
	{
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "FlushOutputMpegBuffer: flush buffer with 0 data length"));
	}
	
	This->m_bufferDescriptors.Free(buf);
}

/////////////////////////////////////////////////////////////////////////////
//
//  CopyVvxHeaderInfo
//
/////////////////////////////////////////////////////////////////////////////

bool CTrickImportUser::CopyVvxHeaderInfo()
{
	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (pSample)
	{
		
		int len = TrickFilesGetVvxIndexHeaderLength(m_trickContext);
		memcpy(pSample->getPointer(), TrickFilesGetVvxIndexHeader(m_trickContext), len);
		pSample->setDataLength(len);
		pSample->setOffset(0, 0);

		OutputPin& pin = _outputPin[3];
		if (!pin.pNextFilter->Receive(pSample, pin.nNextPin))
		{
#pragma message(__MSGLOC__"TODO: log")
			
			//
			//release the sample
			GetGraph()->freeMediaSample(pSample);
			return false;
		}		

		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CreateSubfile
//
/////////////////////////////////////////////////////////////////////////////

bool CTrickImportUser::CreateSubfile(int index, SubfileType type, LONG speed)
{
	DWORD status = STATUS_SUCCESS;

	switch(type)
	{
	case SUBFILE_FF:
		m_ffFile[index].consumerContext = this;
		m_ffFile[index].type = SUBFILE_FF;				// type of file (ff, fr vvx)
		m_ffFile[index].index = index;				// create table index
		m_ffFile[index].relativeSpeed = index+1;		// 1 = ff, -1 = fr, 0 = normal
		m_ffFile[index].fileOffset.QuadPart = 0;
		m_ffFile[index].runningByteOffset = 0;


		status = TrickFilesCreateOutputContext(
				m_trickContext, &m_ffFile[index], speed, m_providerReadSize,
				AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer);
		break;

	case SUBFILE_FR:
		m_frFile[index].consumerContext = this;
		m_frFile[index].type = SUBFILE_FR;				// type of file (ff, fr vvx)
		m_frFile[index].index = index;				// create table index
		m_frFile[index].relativeSpeed = -(index+1);		// 1 = ff, -1 = fr, 0 = normal
		m_frFile[index].fileOffset.QuadPart = 0;
		m_frFile[index].runningByteOffset = 0;

		status = TrickFilesCreateOutputContext(
				m_trickContext, &m_frFile[index], speed, m_providerReadSize,
				AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer);
		break;

	case SUBFILE_VVX:
		m_vvxFile.consumerContext = this;
		m_vvxFile.type = SUBFILE_VVX;				// type of file (ff, fr vvx)
		m_vvxFile.index = index;				// create table index
		m_vvxFile.relativeSpeed = 0;		// 1 = ff, -1 = fr, 0 = normal
		m_vvxFile.fileOffset.QuadPart = 0;
		m_vvxFile.runningByteOffset = 0;

		status = TrickFilesCreateVvxContext(
				m_trickContext, &m_vvxFile, m_providerReadSize,
				AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer, FlushOutputMpegBuffer);
		break;

	case SUBFILE_NORMAL:
		m_normalFile.consumerContext = this;
		m_normalFile.type = SUBFILE_NORMAL;				// type of file (ff, fr vvx)
		m_normalFile.index = index;				// create table index
		m_normalFile.relativeSpeed = 0;		// 1 = ff, -1 = fr, 0 = normal
		m_normalFile.fileOffset.QuadPart = 0;
		m_normalFile.runningByteOffset = 0;

#ifdef V7VVX_CURRENT_MINORVERSION
		
		status = TrickFilesInitializeSourceStream(
				m_trickContext, this, m_providerReadSize,
				m_spliceType == SPLICE_ENABLE,
				AcquireSourceMpegBuffer, ReleaseSourceMpegBuffer);
#else
		status = TrickFilesInitializeSourceStream(
				m_trickContext, this, m_providerReadSize,
				AcquireSourceMpegBuffer, ReleaseSourceMpegBuffer);
#endif
		//
		// set maximum mpeg coding errors permitted
		//
		TrickFilesSetMaximumMpegCodingErrors(m_trickContext, m_maxMpegCodingErrors);

		break;

#ifdef V7VVX_CURRENT_MINORVERSION
	case SUBFILE_SPLICETRANSITION:
		m_spliceFile.consumerContext = this;
		m_spliceFile.type = SUBFILE_SPLICETRANSITION;				// type of file (ff, fr vvx)
		m_spliceFile.index = index;				// create table index
		m_spliceFile.relativeSpeed = 0;		// 1 = ff, -1 = fr, 0 = normal
		m_spliceFile.fileOffset.QuadPart = 0;
		m_spliceFile.runningByteOffset = 0;
		
		if (m_spliceType == SPLICE_ENABLE)
		{
			status = TrickFilesCreateSpliceContext(
						m_trickContext, &m_spliceFile, m_providerReadSize,
						m_spliceMinGopSize, m_gradualTiming, m_holdPictureCount,
						AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer);			
		}
		break;
#endif

	default:;
	}

	if (!NT_SUCCESS(status))
	{		
#pragma message(__MSGLOC__"to do")
		char tmp[300];
		if (!m_sourceBytesRead)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] Fail to init trick context for index [%d], cannot read source data"),
				_strLogHint.c_str(), index, status, m_sourceBytesRead);
			sprintf(tmp, "Fail to init trick context for index [%d], cannot read data", index);	
		}
		else if (status == 0xc0000024)
		{
			
			MOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] Fail to TrickFilesInitializeSourceStream with status[%08x], source data read: [%lld], bad mpeg file"),
				_strLogHint.c_str(), status, m_sourceBytesRead);
			sprintf(tmp, "Fail to TrickFilesInitializeSourceStream with status[%08x], source data read: [%lld], bad mpeg file", status, m_sourceBytesRead);
		}
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] Fail to init trick context for index [%d], error status[0x%08x], source file read: [%lld]"),
				_strLogHint.c_str(), index, status, m_sourceBytesRead);
			
			sprintf(tmp, "Fail to create trick context for index [%d], error status[0x%08x], source file read: [%lld]",
				index, status, m_sourceBytesRead);	
		}
		SetLastError(tmp);
		
		return false;
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////
//
//  GetSubfileBytesWritten
//
/////////////////////////////////////////////////////////////////////////////

PVOID CTrickImportUser::MemoryAllocate(PVOID context, ULONG size)
{
// 	if (context)
// 	{
// 		CTrickImportUser	*This = (CTrickImportUser *)context;
// 		return This->GetGraph()->GetMemAllocator()->alloc(size);
// 	}

	return malloc(size);
}

void CTrickImportUser::MemoryFree(PVOID context, PVOID buffer)
{
// 	if (context)
// 	{
// 		CTrickImportUser	*This = (CTrickImportUser *)context;
// 		This->GetGraph()->GetMemAllocator()->free((char*)buffer);
// 	}
// 	else
	{
		try{free(buffer);}catch(...){}
	}
}

void CTrickImportUser::LogMsgFromTrickLibrary(PVOID context, const char *format, ...)
{
	CTrickImportUser	*This = (CTrickImportUser *)context;

	//
	// skip info messages if not enabled
	//
	switch(*format)
	{
	case 'E':
	case 'W':
	default:			// display by default
		break;

	case 'I':
		if (!This->m_trickFilesDebugLevel)
			return;
		break;
	}

    va_list    marker;
    char		szMsg[4096];

    // Initialize access to variable arglist
    va_start(marker, format);

    // Expand message
    vsnprintf(szMsg, 4095 * sizeof(char), format, marker);
    szMsg[4095] = 0;

	//delete the last \r\n from the string
	{
		char* pPtr = szMsg;
		while(*pPtr)pPtr++;

		pPtr--;

		while(pPtr >=szMsg)
		{
			if (*pPtr == '\n' ||*pPtr == '\r')
				pPtr--;
			else
				break;
		}

		*(pPtr+1) = '\0';

		if (!szMsg[0])
			return;
	}

	if (!This->m_strLastLog.compare(szMsg))
	{
		return;
	}
	
	if (szMsg[0]=='E')
	{
		This->SetLastError(szMsg);
	}
	
	This->m_strLastLog = szMsg;

	SMOLOG(Log::L_INFO, CLOGFMT(TrickLib, "[%s] %s"), This->_strLogHint.c_str() ,szMsg);
}


bool CTrickImportUser::Initialize()
{
	HRESULT hRet;
	DWORD status = STATUS_SUCCESS;

#ifdef V7VVX_CURRENT_MINORVERSION
	status = TrickFilesLibraryInitialize(
				MemoryAllocate, MemoryFree, LogMsgFromTrickLibrary);
#else
	status = TrickFilesLibraryInitialize((PTRICK_MALLOC_ROUTINE) m_mallocRoutine, m_freeRoutine);
#endif
	if (NT_SUCCESS(status))
	{	
		//
		// Get a trick files handle
		//
		m_trickContext = TrickFilesAllocateContext((PVOID)this);
		if (m_trickContext)
		{
			memset(&m_normalFile, 0, sizeof(m_normalFile));
			memset(&m_vvxFile, 0, sizeof(m_vvxFile));
			memset(&m_spliceFile, 0, sizeof(m_spliceFile));
			memset(m_ffFile, 0, sizeof(m_ffFile));
			memset(m_frFile, 0, sizeof(m_frFile));
			memset(&m_trickCharacteristics, 0, sizeof(m_trickCharacteristics));
			memset(&m_trickCounters, 0, sizeof(m_trickCounters));
		}
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] TrickFilesAllocateContext failed"), _strLogHint.c_str());
			SetLastError("TrickFilesAllocateContext failed");

			//
			// close down the library
			//
			TrickFilesLibraryShutdown();
			
			return false;
		}
	}
	else
	{
		char tmp[256];
		sprintf(tmp, "TrickFilesLibraryInitialize failed with status 0x%08x", status);
		SetLastError(tmp);
		MOLOG(Log::L_ERROR, CLOGFMT(TrickLib, "[%s] TrickFilesLibraryInitialize failed with status 0x%08x"),
			_strLogHint.c_str(), status);
		
		return false;
	}

	hRet = S_OK;
	if (!CreateSubfiles())
	{
		return false;
	}

	
#ifndef V7VVX_CURRENT_MINORVERSION
	//
	// initialize vvx header before doing the first vvx update to
	// work around a bug in V4.0.x libraries
	//
	TrickFilesInitializeVvxIndexHeader(m_trickContext);
#endif
	//
	// create the preliminary index header (VVX) file
	//
	TrickFilesUpdateVvxIndexHeader(m_trickContext);
	
	//
	// fire the OnMediaInfo event
	//
	MediaInfo info;
	info.bitrate = m_trickCharacteristics.bitRate;
	info.videoBitrate = m_trickCharacteristics.videoBitRate;
	info.videoResolutionH = m_trickCharacteristics.horizontalSize;
	info.videoResolutionV = m_trickCharacteristics.verticalSize;

	MOLOG(Log::L_INFO, CLOGFMT(TrickLib, "[%s] MediaInfo: bitrate[%d],video_bitrate[%d],video_height[%d],video_width[%d]"),
		_strLogHint.c_str(), info.bitrate,info.videoBitrate,info.videoResolutionH,info.videoResolutionV);
	GetGraph()->OnMediaInfoParsed(info);
	
	return true;
}

void CTrickImportUser::UnInitialize()
{
	if (!m_trickContext)
		return;

	TrickFilesReleaseSourceMpegBuffers (m_trickContext);
	TrickFilesFreeContext(m_trickContext);
	m_trickContext = NULL;

	//
	// close down the library
	//
	TrickFilesLibraryShutdown();
}

bool CTrickImportUser::Run()
{
	MOLOG(Log::L_INFO, CLOGFMT(TrickLib, "[%s] Run() enter"), _strLogHint.c_str());

	bool hRet;
	do
	{
		hRet = Initialize();
		if (!hRet)
			break;

		hRet = Import();
		
		UnInitialize();
	
	}while(0);

	std::set<BaseFilter*> nextFilters;
	for(int i=0;i<_nOutputCount;i++)
	{
		nextFilters.insert(_outputPin[i].pNextFilter);
	}

	std::set<BaseFilter*>::iterator it=nextFilters.begin();
	while(it!=nextFilters.end())	
	{
		(*it)->endOfStream();
		it++;
	}

	MOLOG(Log::L_INFO, CLOGFMT(TrickLib, "[%s] Run() left"), _strLogHint.c_str());
	return hRet;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CreateSubfiles
//
//		create the various target files.
//
//	Returns
//
//		== 0	allocation error
//		== 1	success
//
//
/////////////////////////////////////////////////////////////////////////////

bool CTrickImportUser::CreateSubfiles()
{
	bool hRet;
	//
	// create the normal play file object
	//
	hRet = CreateSubfile(0, SUBFILE_NORMAL, 0);
	if (hRet)
	{
		int i;
		//
		// zero counters
		//
		memcpy(&m_trickCharacteristics,
			TrickFilesStreamCharacteristics(m_trickContext),
			sizeof(m_trickCharacteristics));
		//
		// Set up a fast forward trick files output context
		//
		for ( i = 0; i < (int) m_ffFileCount ; i++ )
		{
			hRet = CreateSubfile(i, SUBFILE_FF, (LONG)(i+1));
			if (!hRet)
				break;
		}

		if (hRet)
		{
			//
			// Set up a fast reverse trick files output context
			//
			for ( i = 0; i < (int) m_frFileCount; i++ )
			{
				hRet = CreateSubfile(i, SUBFILE_FR, (LONG)-(i+1));
				if (!hRet)
					break;
			}

			if (hRet)
			{
				//
				// Set up index file output context
				//
				hRet = CreateSubfile(0, SUBFILE_VVX, 0);
			}
		}
	}

	return hRet;
}


bool CTrickImportUser::Init()
{
	m_providerReadSize = GetGraph()->GetMediaSampleSize();
	return true;
}

void CTrickImportUser::Close()
{
	UnInitialize();
}

}}