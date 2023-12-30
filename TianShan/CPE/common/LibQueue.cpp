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
// Ident : $Id: LibQueue.cpp,v 1.4 2004/07/06 07:20:43 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/common/LibQueue.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 4     09-06-23 18:20 Yixin.tian
// merge for Linux OS
// 
// 3     08-03-27 16:18 Jie.zhang
// 
// 2     08-03-04 17:30 Jie.zhang
// 
// 1     08-02-15 12:45 Jie.zhang
// 
// 1     06-08-30 12:33 Jie.zhang
// 
// 1     05-09-06 13:58 Jie.zhang
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
#pragma warning(disable:4786)
#pragma warning(disable:4018)

#include "LibQueue.h"
#include <stdio.h>
#include <iostream>
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/time.h>
#include <errno.h>
}
#endif

LIBQUEUE CreateQueue()
{
	CLibQueue *pQueue = new CLibQueue;
	return (LIBQUEUE) pQueue;
}

int GetNextOnQueue(LIBQUEUE pQueueParam, int fBlock, void **data)
{
	CLibQueue *pQueue = (CLibQueue *)pQueueParam;
	return pQueue->RemoveFront(fBlock, data);
}

void PutOnQueue(LIBQUEUE pQueueParam, void *pElement)
{
	CLibQueue *pQueue = (CLibQueue *)pQueueParam;
	pQueue->InsertEnd(pElement);
	return;
}

void DeleteQueue(LIBQUEUE pQueueParam)
{
	CLibQueue *pQueue = (CLibQueue *)pQueueParam;
	delete pQueue;
	return;
}

#ifdef ZQ_OS_MSWIN
CLibQueue::CLibQueue() : 
		m_ElementBufferMgr(sizeof(SElementDsc)), 
		m_hEvent(INVALID_HANDLE_VALUE),
		m_Entries(0), m_maxEntries(0)
{
	m_Link = (SElementDsc *)&m_Link;
	m_hEvent = CreateEvent(0,0,0,0);
}

CLibQueue::~CLibQueue()
{
	CloseHandle(m_hEvent);
}
#else
CLibQueue::CLibQueue() : 
		m_ElementBufferMgr(sizeof(SElementDsc)), 
		m_Entries(0), m_maxEntries(0)
{
	m_Link = (SElementDsc *)&m_Link;
	sem_init(&m_sem,0,0);
}

CLibQueue::~CLibQueue()
{
	try{
		sem_destroy(&m_sem);
	}
	catch(...){}
}

#endif

// this Queue loops through the second element so the 
// order is back->front->next->...->back
//
void CLibQueue::InsertEnd(void *E)
{
	SElementDsc *pElemDsc = (SElementDsc *)m_ElementBufferMgr.Get();

	ZQ::common::MutexGuard gd(m_QueueLock);
	
	if (m_Link == (SElementDsc *)&m_Link)	// if this is the first element
	{
		pElemDsc->Link = pElemDsc;			// then set last == first
	}
	else
		pElemDsc->Link = m_Link->Link;		// point new last to first

	m_Link->Link = pElemDsc;				// prior last point to new last
	m_Link = pElemDsc;						// and queue head points to last
	pElemDsc->pElement = E;

	m_Entries++;
	if (m_Entries > m_maxEntries)
		m_maxEntries = m_Entries;
	//
	// signal new entry if Queue was empty
	//
	if (m_Link->Link == pElemDsc)			// if last == first
	{
#ifdef ZQ_OS_MSWIN
		SetEvent(m_hEvent);
#else
		sem_post(&m_sem);
#endif
	}
}
//
// Remove the first element on the Queue
//
int CLibQueue::RemoveFront(int Block, void** pData, uint32 timeOut)
{
	*pData = 0;
	m_QueueLock.enter();	
	// get the first element
	//
	CLibQueue::SElementDsc *dsc = m_Link->Link;
	if (dsc == (CLibQueue::SElementDsc *)&m_Link)			// if none
	{
#ifdef ZQ_OS_MSWIN
		ResetEvent(m_hEvent);				// make sure event is clear on empty list
		m_QueueLock.leave();
		if (Block)
		{
			//
			// wait for an insertion if there's a cond variable
			//
			int s = WaitForSingleObject(m_hEvent, timeOut);
			if (s == WAIT_OBJECT_0)
			{
				m_QueueLock.enter();
				//
				// pop this element
				//
				dsc = m_Link->Link;				// get first element
			}
			else
			{
				if (s != WAIT_TIMEOUT)
				{
#ifdef _DEBUG
					printf("CLibQueue::RemoveFront: WaitForSingleObject error %d\n", s);
#endif
				}
				return -1;
			}
		}
#else
		int nval = 0;
		sem_getvalue(&m_sem,&nval);
		for(;nval>0 ;nval--)
			sem_trywait(&m_sem);
		m_QueueLock.leave();
		if (Block)
		{
			struct timespec tsp;
			struct timeval tval;
			gettimeofday(&tval,NULL);
			int64 micros = timeOut*1000ll + tval.tv_usec;
			tsp.tv_sec = tval.tv_sec + micros/1000000l;
			tsp.tv_nsec = (micros%1000000l)*1000;
				
			int ret = sem_timedwait(&m_sem, &tsp);
			if (ret == 0)
			{
				m_QueueLock.enter();
				//
				// pop this element
				//
				dsc = m_Link->Link;				// get first element
			}
			else
			{
				if (errno != ETIMEDOUT)
				{
#ifdef _DEBUG
					printf("CLibQueue::RemoveFront: sem_timedwait error %d\n", errno);
#endif
				}
				return -1;
			}
		}
#endif
		else
		{
			return 0;
		}

	}
	if (dsc == m_Link)						// if removing final item
		m_Link = (SElementDsc *)&m_Link;	// remove back pointer too
	else
		m_Link->Link = dsc->Link;			// last now points to new first

	m_Entries--;

	m_QueueLock.leave();
	void * E = dsc->pElement;
	m_ElementBufferMgr.Free(dsc);
	*pData = E;
	return 1;
}

int CLibQueue::Dump()
{
	//printf("Dump of queue %x, %d elements\n", this, m_Entries);
	std::cout << "Dump of queue " << this << ", " << m_Entries << " elements\n";
	SElementDsc *dsc = (SElementDsc *)m_Link;
	if (dsc == (SElementDsc *)&m_Link)
	{
		printf("    queue is empty\n");
	}
	else
	{
		int i = 1;
		do
		{
//			printf("    element %d = 0x%x\n", i++, dsc->pElement);
			std::cout << "    element " << i++ << " = 0x" << dsc->pElement << "\n";
			dsc = dsc->Link;
		} while (dsc != m_Link);

		printf("\n    total of %d items in queue\n", i-1);
	}

	return m_Entries;
}

void CLibQueue::Stats(QueueStats *sb)
{
	sb->entries = m_Entries;
	sb->maxEntries = m_maxEntries;
}
