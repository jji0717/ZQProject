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
// $Log: /ZQProjs/TianShan/CPE/common/LibBuffer.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 4     09-06-23 18:20 Yixin.tian
// 
// 3     08-03-27 16:18 Jie.zhang
// 
// 2     08-03-04 17:30 Jie.zhang
// 
// 1     08-02-15 12:45 Jie.zhang
// 
// 1     06-08-30 12:33 Jie.zhang
// 
// 3     06-07-03 14:47 Jie.zhang
// 
// 2     06-06-12 15:42 Jie.zhang
// resolve memoery leak when trick generation failure
// 
// 1     05-09-06 13:58 Jie.zhang
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

#pragma warning(disable:4786)
#pragma warning(disable:4018)

#include <stdio.h>

#include "LibBuffer.h"
#include "LibQueue.h"

extern "C"
{
	LIBBUFFER LibInitializeBufferPool(int size)
	{
		return new CLibBuffer(size);
	}

	bool LibGetBuffer(LIBBUFFER *pBufDsc, void **pBuf)
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

CLibBuffer::CLibBuffer(int size) : m_Link(0), m_Size(size), m_nAllocated(0)
{
}

CLibBuffer::~CLibBuffer()
{
	ZQ::common::MutexGuard gd(m_BufLock);

	for(size_t i=0;i<m_pLinkBak.size();i++)
	{
		m_nAllocated--;
		free(m_pLinkBak[i]);
	}

}

void *CLibBuffer::Get()
{
	ZQ::common::MutexGuard gd(m_BufLock);
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
			m_pLinkBak.push_back(buf);
		}
	}
	return (void *)buf;
}

void CLibBuffer::Free(void *buf)
{
	if (buf)
	{
		SBufferDsc *dsc = (SBufferDsc *)buf;
		ZQ::common::MutexGuard gd(m_BufLock);
		dsc->Link = m_Link;
		m_Link = dsc;
	}
}
