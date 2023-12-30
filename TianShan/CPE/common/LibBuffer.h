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
// Ident : $Id: LibBuffer.h,v 1.3 2004/07/06 07:20:43 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/common/LibBuffer.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 2     09-06-23 18:20 Yixin.tian
// 
// 1     08-02-15 12:45 Jie.zhang
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


#ifndef LIB_BUFFER_H
#define LIB_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif
//
// C functions
//
typedef void *LIBBUFFER;

LIBBUFFER LibInitializeBufferPool(int size);
bool LibGetBuffer(LIBBUFFER *pBufDsc, void **pBuf);
void LibFreeBuffer(LIBBUFFER *pBufDsc, void *pBuf);
void LibDeleteBufferPool(LIBBUFFER pBufDsc);
#ifdef __cplusplus
}
#include "Locks.h"
#include <vector>
//
// a buffer manager class
//
class CLibBuffer
{
protected:
	struct SBufferDsc
	{
		struct SBufferDsc	*Link;
		void				*UserBuffer;
	} *m_Link;
		
	std::vector<struct SBufferDsc*>	m_pLinkBak;
	int					m_Size;
	int					m_nAllocated;
	ZQ::common::Mutex	m_BufLock;

public:
	CLibBuffer(int size);
	~CLibBuffer();
	void	*Get();
	void	Free(void *buf);

	inline int Size() { return m_Size; };
	inline int GetAllocatedBufferCount() { return m_nAllocated;}
};
#endif
#endif		// ifndef LIB_BUFFER_H
