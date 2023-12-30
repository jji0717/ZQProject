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
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/TrickModule/TrickImportUser.cpp $
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
// 3     04-11-22 19:53 Jie.zhang
// seperate initialize()/uninitialize() steps from import()
// Revision 1.8  2004/08/12 09:40:42  jshen
// remove output to screen
//
// Revision 1.7  2004/07/29 06:21:04  jshen
// before release
//
// Revision 1.6  2004/07/22 01:46:03  jshen
// remove manual stop
//
// Revision 1.5  2004/07/12 06:44:47  jshen
// because of the IP change of the CVS server
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


#include "Log.h"
#include "tchar.h"
#include "TrickImportUser.h"

using namespace ZQ::common;
#include "BufferPool.h"
//#include "Resource.h"
#include "NtFileIo.h"
#ifdef _DEBUG
#include <crtdbg.h>
#endif

using namespace ZQ::common;

#pragma comment (lib, "TrickFilesLibraryUser.lib")
#pragma comment (lib, "MpegLibraryUser.lib")

/////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
/////////////////////////////////////////////////////////////////////////////

CTrickImportUser::CTrickImportUser(ImportThreadParams *params, DWORD dwAUID)
	: m_ffFileCount(1), m_frFileCount(1), m_sourceBytesRead(0),
	m_atEndOfFile(0), m_targetBytesWritten(0), 
	m_providerReadSize(params->bufferSize), m_refCount(0), m_flushCount(0)

#ifdef V7VVX_CURRENT_MINORVERSION
	#ifdef CreateSplicingFile
	,m_spliceType(SPLICE_ENABLE)
	#else
	,m_spliceType(SPLICE_DISABLE)
	#endif
	, m_spliceMinGopSize(3), m_holdPictureCount(6), m_gradualTiming(1) 

#endif
{
	_dwAssetUID = dwAUID;

	m_params = params;
	m_mallocRoutine = malloc;
	m_freeRoutine	= free;
	
#ifdef V7VVX_CURRENT_MINORVERSION
	TrickFilesLibraryInitialize(MemoryAllocate, MemoryFree, (PTRICK_DBGPRINT_ROUTINE)LogMsgFromTrickLibrary);
#else
	TrickFilesLibraryInitialize(m_mallocRoutine, m_freeRoutine);
#endif
	//
	// Get a trick files handle
	//
	m_trickContext = TrickFilesAllocateContext((PVOID)this);
	m_normalFile = 0;
	m_vvxFile = 0;
	memset(m_ffFile, 0, sizeof(m_ffFile));
	memset(m_frFile, 0, sizeof(m_frFile));
	memset(&m_trickCharacteristics, 0, sizeof(m_trickCharacteristics));
	memset(&m_trickCounters, 0, sizeof(m_trickCounters));

#ifdef V7VVX_CURRENT_MINORVERSION
	m_spliceFile = 0;
#endif
	m_refCount++;
	m_vvxFlushNeeded = 0;
	
	_tcscpy(m_targetFilename, m_params->targetFilename);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
/////////////////////////////////////////////////////////////////////////////

CTrickImportUser::~CTrickImportUser()
{
	if (m_trickContext)
		TrickFilesFreeContext(m_trickContext);

	TrickFilesLibraryShutdown();

	m_refCount--;
	if (0 != m_refCount)
	{
		glog(Log::L_DEBUG, _T("%08x: CTrickImportUser: m_refCount is not 0!"), _dwAssetUID);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Start an import
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CTrickImportUser::Import()
{
	DWORD status = STATUS_SUCCESS;	

	try
	{
		for (m_pictureCount = 0 ; NT_SUCCESS(status) ; m_pictureCount++)
		{
			// call library to process 1 I Frame
			//
			try{
			status = TrickFilesProcessNextPicture(m_trickContext);
			}
			catch(...)
			{
				glog(Log::L_ERROR, "%08x: CTrickImportUser: exception in Import loop at TrickFilesProcessNextPicture", _dwAssetUID);	

			}
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
			if (m_vvxFlushNeeded)
			{
				//
				// update index records and re-write the header
				//
				try
				{
				TrickFilesUpdateVvxIndexHeader(m_trickContext);
				CopyVvxHeaderInfo();
				m_vvxFlushNeeded = 0;
				m_flushCount++;
				}
				

				catch(...)
				{
					glog(Log::L_ERROR, "%08x: CTrickImportUser: exception in Import TrickFilesUpdateVvxIndexHeader. ", _dwAssetUID);	
				}
			}		
		}
	}
	catch(...)
	{
		glog(Log::L_ERROR, "%08x: CTrickImportUser: exception in Import loop. and flushCount=%d", _dwAssetUID,m_flushCount);	
	}


	glog(Log::L_DEBUG, "%08x: CTrickImportUser: end of TrickFilesProcessNextPicture.\n", _dwAssetUID);

	//
	// get a copy of the counters
	//
	memcpy(&m_trickCounters, TrickFilesGetCounters(m_trickContext),	sizeof(m_trickCounters));

	try
	{
		//
		// Update VVX index header and data
		// re-write the header
		//
		TrickFilesUpdateVvxIndexHeader(m_trickContext);
		TrickFilesReleaseIndexMpegBuffers(m_trickContext);
		CopyVvxHeaderInfo();
	}
	catch(...)
	{
		glog(Log::L_DEBUG, "%08x: CTrickImportUser: exception in Import VVX index header.", _dwAssetUID);
	}
	
	if (!NT_SUCCESS(status))
	{
		glog(Log::L_ERROR, _T("%08x: CTrickImportUser: Import complete failed, Final status %08X"), _dwAssetUID, status);
		return E_MAYNOT_MPEG2;
	}
	else
	{
		glog(Log::L_DEBUG, "%08x: CTrickImportUser: Import complete success.", _dwAssetUID);
		return E_OK;
	}
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
	PSUBFILE_CONTEXT	subFile		= This->m_normalFile;
	
	//
	// Allocate a data buffer
	//
	if (!This->m_atEndOfFile)
	{
		CBufferPool	*bp = This->m_params->srcBufferPool;
		buf	= bp->Alloc();	// this call TrickFileXfer::Alloc() which allocate
							// a network receive buffer
		if(buf)
		{
			if (buf->mpegBuffer.pointer)
			{
				
				buf->offset					= 0;
				buf->freeList				= bp;
				mpegBuffer					= &buf->mpegBuffer;
				
				MpegLibraryInitializeBuffer(
					mpegBuffer,  This, This->m_sourceBytesRead,
					buf->len, buf->mpegBuffer.pointer);
				
				This->m_sourceBytesRead += buf->len;
			}
			else
			{
				This->m_atEndOfFile = 1;
			}
		}
		else
			This->m_atEndOfFile = 1;
	}
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

	if (subFile->state != SUBFILE_STOPPED)
	{
		CTrickImportUser *This	= (CTrickImportUser *)subFile->consumerContext;
		//
		// Allocate a buffer
		//
		CBufferPool *bp			= (CBufferPool *)This->m_params->outBufferPool;
		SDataBuffer *buf		= bp->Alloc();

		if (buf)
		{
			buf->len			= This->m_providerReadSize;
			buf->freeList		= bp;
			mpegBuffer			= &buf->mpegBuffer;

			MpegLibraryInitializeBuffer(
					mpegBuffer,  This, subFile->runningByteOffset,
					This->m_providerReadSize, buf->mpegBuffer.pointer);

			subFile->runningByteOffset += mpegBuffer->length;
		}
		else
		{
			glog(Log::L_DEBUG, _T("%08x: CTrickImportUser: AcquireOutputMpegBuffer() no SOURCE buffers."), This->_dwAssetUID);
#ifdef _DEBUG
			subFile->consumerContext->LogText(_T("AcquireOutputMpegBuffer() no OUTPUT buffers"));
#endif 
		}
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
	
	PSUBFILE_CONTEXT	subFile = This->m_normalFile;
	ULONG				amountWritten = 0;
	int					status = 1;
	
	if (mpegBuffer)
	{
		SDataBuffer *buf = (SDataBuffer *)((PUCHAR)mpegBuffer - offsetof(SDataBuffer, mpegBuffer));
		if (subFile)
		{
			if(buf)
			{
				if (buf->len)
				{
					if (subFile->state != SUBFILE_STOPPED)
					{
						subFile->ioQueue.InsertEnd((PVOID)buf);
						This->m_targetBytesWritten += buf->len;
						InterlockedIncrement(&subFile->queueDepth);
						while(subFile->queueDepth > 2)
							Sleep(10);
					}
					else
					{
						if(buf->freeList)
							buf->freeList->Free(buf);
						This->m_atEndOfFile = 1;
					}
				}
				else
				{
					if(buf->freeList)
						buf->freeList->Free(buf);
					This->m_atEndOfFile = 1;//added by salien
				}
			}
			else
				This->m_atEndOfFile = 1;//added by salien
		}
		else
		{
			if(buf->freeList)
				buf->freeList->Free(buf);

			This->m_atEndOfFile = 1;//added by salien
		}
	}
	else
		This->m_atEndOfFile = 1;//added by salien
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

	if (mpegBuffer)
	{
		SDataBuffer *buf = (SDataBuffer *)((PUCHAR)mpegBuffer - offsetof(SDataBuffer, mpegBuffer));
		if (subFile)
		{
			if (bytesConsumed)
			{
				if (!subFile->FlushSubfile(buf->len, buf->mpegBuffer.pointer))
				{					
					glog(Log::L_DEBUG, _T("CTrickImportUser: subfile context(%08X) Flushing OUTPUT buffer %08X failed"), subFile->consumerContext, (PUCHAR)mpegBuffer);
				}
			}
		}
	}
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
	PSUBFILE_CONTEXT		subFile = (PSUBFILE_CONTEXT) context;
	ULONG					amountWritten = 0;
	int						status = 1;
	
	if (mpegBuffer)
	{
		CTrickImportUser *This = subFile->consumerContext;
		SDataBuffer *buf = (SDataBuffer *)((PUCHAR)mpegBuffer - offsetof(SDataBuffer, mpegBuffer));
		if (subFile)
		{
			if (buf)
			{
			
				if (bytesConsumed)
				{
					if (subFile->state != SUBFILE_STOPPED)
					{
						buf->len = bytesConsumed;
						subFile->ioQueue.InsertEnd((PVOID)buf);
						InterlockedIncrement(&subFile->queueDepth);
						while(subFile->queueDepth > 2)
							Sleep(10);


						This->m_targetBytesWritten += amountWritten;
#if 1
						//	enable this code when playback while record is supported
						//

						//
						// key updates of the index file on writes of the fr file
						// to keep index records and file data synchronized
						//
						if (subFile == This->m_frFile[This->m_frFileCount-1])
						{
							This->m_vvxFlushNeeded = 1;
						}
#endif 
					}
					else
					{
						buf->freeList->Free(buf);
					}
				}
				else
					buf->freeList->Free(buf);
			}			
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  CopyVvxHeaderInfo
//
/////////////////////////////////////////////////////////////////////////////

bool CTrickImportUser::CopyVvxHeaderInfo()
{
	PUCHAR		buffer			= TrickFilesGetVvxIndexHeader(m_trickContext);
	ULONG		headerLength	= TrickFilesGetVvxIndexHeaderLength(m_trickContext);
	//SDataBuffer *buf			= m_params->outBufferPool->Alloc();
	//if ( buf )
	//{
	//	memcpy(buf->mpegBuffer.pointer, buffer, headerLength);
	//	m_vvxFile->WriteVvxHeader(headerLength, buf->mpegBuffer.pointer);
	//	buf->freeList->Free(buf);
	//	return true;
	//}
	//return false;
	return m_vvxFile->WriteVvxHeader(headerLength, buffer);
}

/////////////////////////////////////////////////////////////////////////////
//
//  CreateSubfile
//
/////////////////////////////////////////////////////////////////////////////

HRESULT CTrickImportUser::CreateSubfile(int index, SubfileType type, LONG speed, PSUBFILE_CONTEXT *pSubfile)
{
	DWORD status = STATUS_SUCCESS;

	*pSubfile = new CNtFileIo(index, type, speed, this);

	PSUBFILE_CONTEXT subFile = *pSubfile;

	InterlockedIncrement(&subFile->refCount);
	SetEvent(subFile->hStartEvent);

	switch(type)
	{
	case SUBFILE_FF:
		if (index == 0)
			_stprintf(subFile->filename, _T("%s.ff"), m_targetFilename);
		else
			_stprintf(subFile->filename, _T("%s.f%d"), m_targetFilename, index);
		status = TrickFilesCreateOutputContext(
				m_trickContext, subFile, speed, m_providerReadSize,
				AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer);
		break;

	case SUBFILE_FR:
		if (index == 0)
			_stprintf(subFile->filename, _T("%s.fr"), m_targetFilename);
		else
			_stprintf(subFile->filename, _T("%s.r%d"), m_targetFilename, index);

		status = TrickFilesCreateOutputContext(
				m_trickContext, subFile, speed, m_providerReadSize,
				AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer);
		break;

	case SUBFILE_VVX:
		_stprintf(subFile->filename, _T("%s.vvx"), m_targetFilename);
		status = TrickFilesCreateVvxContext(
				m_trickContext, subFile, m_providerReadSize,
				AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer, FlushOutputMpegBuffer);
		break;

	case SUBFILE_NORMAL:
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
		TrickFilesSetMaximumMpegCodingErrors(m_trickContext, m_params->maxMpegCodingErrors);
		_tcscpy(subFile->filename, m_targetFilename);
		break;

#ifdef V7VVX_CURRENT_MINORVERSION
	case SUBFILE_SPLICETRANSITION:
		if (m_spliceType == SPLICE_ENABLE)
		{
			_stprintf(subFile->filename, _T("%s.vvt"), m_targetFilename);

			status = TrickFilesCreateSpliceContext(
						m_trickContext, subFile, m_providerReadSize,
						m_spliceMinGopSize, m_gradualTiming, m_holdPictureCount,
						AcquireOutputMpegBuffer, ReleaseOutputMpegBuffer);			
		}
		break;
#endif

	default:;
	}

//	DbgString("subFile %x created for file %s\n", subFile, subFile->filename);

	if (NT_SUCCESS(status))
	{		
		if (subFile->CreateSubfile())
			return E_OK;
		else
		{
			CloseSubfile(*pSubfile);
			*pSubfile = NULL;
			return E_CREATEFILE;
		}
	}
	else
	{	
		CloseSubfile(*pSubfile);
		*pSubfile = NULL;
		
		if (status == 0xC0000024)
		{
			return E_MAYNOT_MPEG2;
		}
		else
		{
			return E_UNKNOWN;
		}		
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Close all subfiles
//
/////////////////////////////////////////////////////////////////////////////

void CTrickImportUser::CloseSubfiles()
{
	DWORD i;

	// Shutdown subfile threads
	//
	if (m_normalFile)
	{
		CloseSubfile(m_normalFile);
		m_normalFile = NULL;
	}

	if (m_vvxFile)
	{
		CloseSubfile(m_vvxFile);
		m_vvxFile = NULL;
	}

	for (i = 0 ; i < m_ffFileCount ; i++)
	{
		if (m_ffFile[i])
		{
			CloseSubfile(m_ffFile[i]);
			m_ffFile[i] = NULL;
		}
	}

	for (i = 0 ; i < m_frFileCount ; i++)
	{
		if (m_frFile[i])
		{
			CloseSubfile(m_frFile[i]);
			m_frFile[i] = NULL;
		}
	}

#ifdef V7VVX_CURRENT_MINORVERSION
#ifdef CreateSplicingFile
	if (m_spliceFile)
	{
		CloseSubfile(m_spliceFile);
		m_spliceFile = NULL;
	}
#endif 
#endif
	
	while(m_refCount > 1)
		Sleep(100);	
}

/////////////////////////////////////////////////////////////////////////////
//
//  Shutdown subfile threads
//
/////////////////////////////////////////////////////////////////////////////

void CTrickImportUser::CloseSubfile(PSUBFILE_CONTEXT subFile)
{
	//
	// clean up subfiles
	//
	if (subFile)
	{
		long count=0;
		if (subFile->refCount>0)
			count =InterlockedDecrement(&subFile->refCount);
		
		subFile->ioQueue.InsertEnd(0);

		// before return, wait for the subfile io thread to terminate
		WaitForSingleObject(subFile->hStopEvent, 600000);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  GetQueueDepth
//
/////////////////////////////////////////////////////////////////////////////

DWORD CTrickImportUser::GetIoQueueDepth(SubfileType type)
{
	switch(type)
	{
	case SUBFILE_NORMAL:
		if (m_normalFile)
			return m_normalFile->queueDepth;
		break;

	case SUBFILE_VVX:
		if (m_vvxFile)
			return m_vvxFile->queueDepth;
		break;

	case SUBFILE_FF:
		if (m_ffFile[0])
			return m_ffFile[0]->queueDepth;
		break;

	case SUBFILE_FR:
		if (m_frFile[0])
			return m_frFile[0]->queueDepth;
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//  GetSubfileBytesWritten
//
/////////////////////////////////////////////////////////////////////////////

LONGLONG CTrickImportUser::GetSubfileBytesWritten(SubfileType type)
{
	switch(type)
	{
	case SUBFILE_NORMAL:
		if (m_normalFile)
			return m_normalFile->BytesWritten();
		break;

	case SUBFILE_VVX:
		if (m_vvxFile)
			return m_vvxFile->BytesWritten();
		break;

	case SUBFILE_FF:
		if (m_ffFile[0])
			return m_ffFile[0]->BytesWritten();
		break;

	case SUBFILE_FR:
		if (m_frFile[0])
			return m_frFile[0]->BytesWritten();
		break;
	}

	return 0;
}

bool CTrickImportUser::GetSubfileState(SubfileType type, PULONG state, PULONG subState)
{
	switch(type)
	{
	case SUBFILE_NORMAL:
		if (m_normalFile)
		{
			*state = m_normalFile->state;
			*subState = m_normalFile->subState;
			return 1;
		}
		break;

	case SUBFILE_VVX:
		if (m_vvxFile)
		{
			*state = m_vvxFile->state;
			*subState = m_vvxFile->subState;
			return 1;
		}
		break;

	case SUBFILE_FF:
		if (m_ffFile[0])
		{
			*state = m_ffFile[0]->state;
			*subState = m_ffFile[0]->subState;
			return 1;
		}
		break;

	case SUBFILE_FR:
		if (m_frFile[0])
		{
			*state = m_frFile[0]->state;
			*subState = m_frFile[0]->subState;
			return 1;
		}
		break;
	}

	return 0;
}

void CTrickImportUser::DumpSubfileQueue(SubfileType type)
{
	switch(type)
	{
	case SUBFILE_NORMAL:
		if (m_normalFile)
		{
			m_normalFile->ioQueue.Dump();
		}
		break;

	case SUBFILE_VVX:
		if (m_vvxFile)
		{
			m_vvxFile->ioQueue.Dump();
		}
		break;

	case SUBFILE_FF:
		if (m_ffFile[0])
		{
			m_ffFile[0]->ioQueue.Dump();
		}
		break;

	case SUBFILE_FR:
		if (m_frFile[0])
		{
			m_frFile[0]->ioQueue.Dump();
		}
		break;
	}
}


PVOID CTrickImportUser::MemoryAllocate(PVOID context, ULONG size)
{
	if (context)
	{
		CTrickImportUser	*This = (CTrickImportUser *)context;
		return This->m_mallocRoutine(size);
	}

	return malloc(size);
}

void CTrickImportUser::MemoryFree(PVOID context, PVOID buffer)
{
	if (context)
	{
		CTrickImportUser	*This = (CTrickImportUser *)context;
		This->m_freeRoutine(buffer);
	}
	else
		free(buffer);
}


void CTrickImportUser::LogMsgFromTrickLibrary(PVOID context, const char *format, ...)
{
}


HRESULT CTrickImportUser::Initialize()
{
	DWORD i;
	
	HRESULT hRet = E_OK;

	do
	{
#ifndef _UNICODE
		wchar_t fn[256];
		MultiByteToWideChar(CP_ACP, 0, m_targetFilename, -1, fn, sizeof(fn));
		TrickFilesSetFileName(m_trickContext, (PUCHAR)fn);
#else
		TrickFilesSetFileName(m_trickContext, (PUCHAR)m_targetFilename);
#endif		
		
		hRet = CreateSubfile(0, SUBFILE_NORMAL, 0, &m_normalFile);
		if (hRet != E_OK)
			break;

		memcpy(&m_trickCharacteristics, TrickFilesStreamCharacteristics(m_trickContext), sizeof(m_trickCharacteristics));
		
		//
		// Set up a fast forward trick files output context
		//
		for ( i = 0; i < m_ffFileCount ; i++ )
		{
			hRet = CreateSubfile(i, SUBFILE_FF, (LONG)(i+1), &m_ffFile[i]);
			if (hRet != E_OK)
				break;
		}

		if (hRet != E_OK)
			break;

		//
		// Set up a fast reverse trick files output context
		//
		for ( i = 0; i < m_frFileCount; i++ )
		{
			hRet = CreateSubfile(i, SUBFILE_FR, -((LONG)(i+1)), &m_frFile[i]);
			if (hRet != E_OK)
				break;
		}

		if (hRet != E_OK)
			break;

		//
		// Set up index file output context
		//
		hRet = CreateSubfile(0, SUBFILE_VVX, 0, &m_vvxFile);
		if (hRet != E_OK)
			break;

#ifdef V7VVX_CURRENT_MINORVERSION
		//
		// Set up splicing segment file output context
		//
#ifdef CreateSplicingFile
		hRet = CreateSubfile(0, SUBFILE_SPLICETRANSITION, 0, &m_spliceFile);
#endif 

#endif
	}while(0);

	
	if (hRet != E_OK)
	{
		if (m_trickContext)
		{
			TrickFilesFreeContext(m_trickContext);
			m_trickContext = 0;
		}

		CloseSubfiles();
		glog(Log::L_ERROR, "%08x: CTrickImportUser: Initialize error with code %d.", _dwAssetUID, hRet);

		return hRet;
	}

	glog(Log::L_DEBUG, "%08x: CTrickImportUser: Initialize OK.", _dwAssetUID);

	return E_OK;
}

HRESULT CTrickImportUser::UnInitialize()
{
	if (!m_trickContext)
		return 0;

#ifdef _DEBUG
	DbgString(_T("%08x: CTrickImportUser UnInitialize enter.\n"), _dwAssetUID);
#endif

	glog(Log::L_DEBUG, "%08x: CTrickImportUser: UnInitialize enter.", _dwAssetUID);

	try
	{
		TrickFilesReleaseSourceMpegBuffers (m_trickContext);
	}
	catch(...)
	{
		glog(Log::L_ERROR, "%08x: CTrickImportUser: exception in call TrickFilesReleaseSourceMpegBuffers.", _dwAssetUID);
	}

	try
	{
		//
		// Release the trick files processing context
		//
		TrickFilesFreeContext(m_trickContext);
	}
	catch(...)
	{
		glog(Log::L_ERROR, "%08x: CTrickImportUser: exception in call TrickFilesFreeContext.", _dwAssetUID);
	}

	m_trickContext = 0;

#if 0
	//
	// Delete splice file if nothing was written to it
	//
	if (m_spliceFile.bytesWritten == 0)
	{
		TFLODeleteSpliceTransitionFile(m_trickHandle);
	}
#endif
	

	//
	// request shutdown of all file threads
	//
	try
	{
		CloseSubfiles();
	}
	catch(...)
	{
		glog(Log::L_ERROR, "%08x: CTrickImportUser: UnInitialize execpton in CloseSubfiles.", _dwAssetUID);
	}
	
	
	glog(Log::L_DEBUG, "%08x: CTrickImportUser: UnInitialize leave.", _dwAssetUID);

#ifdef _DEBUG
	DbgString(_T("%08x: CTrickImportUser UnInitialize leave.\n"), _dwAssetUID);
#endif	

	return 0;
}