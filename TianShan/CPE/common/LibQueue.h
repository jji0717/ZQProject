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
// Ident : $Id: LibQueue.h,v 1.3 2004/07/06 07:20:43 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/common/LibQueue.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 2     09-06-23 18:20 Yixin.tian
// merge for Linux OS
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

#ifndef LIB_QUEUE_H
#define LIB_QUEUE_H

#include "ZQ_common_conf.h"
#ifdef __cplusplus
extern "C" {
#endif

#define LIBQUEUE	void *

LIBQUEUE CreateQueue();
int GetNextOnQueue(LIBQUEUE Queue, int fBlock, void **data);
bool GetNextOnQueueWithTimeout(LIBQUEUE Queue, int fBlock, void **data, uint32 timeOut);
void PutOnQueue(LIBQUEUE pQueue, void *pElement);
void DeleteQueue(LIBQUEUE pQueue);

typedef struct _QueueStats
{
	int		entries;
	int		maxEntries;
} QueueStats, *PQueueStats;

#ifdef __cplusplus
}

#include "LibBuffer.h"

class CLibQueue
{
protected:
	struct SElementDsc
	{
		SElementDsc	*Link;
		void		*pElement;
	};

	SElementDsc			*m_Link;
	CLibBuffer			m_ElementBufferMgr;

#ifdef ZQ_OS_MSWIN
	HANDLE				m_hEvent;
#else
	sem_t				m_sem;
#endif
	ZQ::common::Mutex	m_QueueLock;
	int					m_Entries;
	int					m_maxEntries;

public:
	CLibQueue();
	~CLibQueue();
	void InsertEnd(void *E);
	int RemoveFront(int fBlock, void **pData, uint32 timeOut=0xFFFFFFFF);
	int Dump();
	void Stats(QueueStats *sb);
};

#endif		// __cplusplus
#endif		// ifndef LIB_Queue_H
