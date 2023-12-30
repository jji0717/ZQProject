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
// Ident : $Id: LibBuffer.cpp,v 1.3 2004/07/06 07:20:43 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/IngestSim/LibBuffer.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 2     05-08-31 16:42 Mei.zhang
// 
// 1     05-08-26 17:04 Lin.ouyang
// init version
// 
// 1     04-12-03 13:56 Jie.zhang
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

#include <windows.h>
#include <stdio.h>

#include "LibBuffer.h"
#include "LibQueue.h"

extern "C"
{
	LIBBUFFER LibInitializeBufferPool(int size)
	{
		return new CLibBuffer(size);
	}

	BOOL LibGetBuffer(LIBBUFFER *pBufDsc, void **pBuf)
	{
		CLibBuffer *pHeader = (CLibBuffer *)pBufDsc;
		*pBuf = pHeader->Get();
		return pBuf != 0;
	}
	

	void LibFreeBuffer(LIBBUFFER *pBufDsc, void *pBuf)
	{
		CLibBuffer *pHeader = (CLibBuffer *)pBufDsc;
		pHeader->Free(pBuf);
	}

	void LibDeleteBufferPool(LIBBUFFER pBufDsc)
	{
		CLibBuffer *pHeader = (CLibBuffer *)pBufDsc;
		delete pHeader;
	}
}

CLibBuffer::CLibBuffer(int size) : m_Link(0), m_nAllocated(0), m_Size(size)
{
	InitializeCriticalSection(&m_BufLock);
}

CLibBuffer::~CLibBuffer()
{
	EnterCriticalSection(&m_BufLock);

	SBufferDsc *buf;
	while(buf = m_Link)
	{
		m_Link = buf->Link;
		m_nAllocated--;
		free(buf);
	}

	LeaveCriticalSection(&m_BufLock);
	DeleteCriticalSection(&m_BufLock);
}

void *CLibBuffer::Get()
{
	EnterCriticalSection(&m_BufLock);
	SBufferDsc *buf = m_Link;
	if (buf)
		m_Link = buf->Link;
	else
	{
		buf = (SBufferDsc *)malloc(m_Size);
		if (buf)
		{
//			printf("allocating buffer %x for pool %x\n", buf, this);
			m_nAllocated++;
		}
	}
	LeaveCriticalSection(&m_BufLock);
	return (void *)buf;
}

void CLibBuffer::Free(void *buf)
{
	if (buf)
	{
		SBufferDsc *dsc = (SBufferDsc *)buf;
		EnterCriticalSection(&m_BufLock);
		dsc->Link = m_Link;
		m_Link = dsc;
		LeaveCriticalSection(&m_BufLock);
	}
}
