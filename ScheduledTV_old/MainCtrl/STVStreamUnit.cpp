// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: STVStreamUnit.h$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : implementation of STV stream bring up thread collection unit
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/MainCtrl/STVStreamUnit.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 12    05-04-19 21:18 Bernie.zhao
// autobuild modification
// 
// 11    05-03-24 14:53 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 10    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 9     05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 8     04-12-16 14:55 Bernie.zhao
// 
// 7     04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 6     04-11-23 10:01 Bernie.zhao
// 
// 5     04-10-19 17:21 Bernie.zhao
// for QA release
// 
// 4     04-10-15 16:36 Bernie.zhao
// 
// 3     04-10-14 14:56 Bernie.zhao
// 
// 2     04-10-11 15:19 Bernie.zhao
// 
// 1     04-10-07 9:09 Bernie.zhao
// ===========================================================================

#include "STVStreamUnit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STVStreamUnit::STVStreamUnit(const char* RtspHostIP, const char* RtspURL, DWORD RtspPort, DWORD RtspNsec)
{
	strcpy(_szRtspHostIP, RtspHostIP);
	strcpy(_szRtspURL, RtspURL);
	_dwRtspHostPort = RtspPort;
	_dwRtspNsec = RtspNsec;
}

STVStreamUnit::~STVStreamUnit()
{

}

HRESULT STVStreamUnit::OnStartStream(STVPlaylist* pPL, DWORD chnlID, bool isStart)
{
	bool bDupWorker = FALSE;

	// kiss dummy worker thread goodbye first
	for(size_t i=0; i<_WorkerPool.size(); i++) 
	{
		if( !_WorkerPool[i]->isWorking()) 
		{	
			// this worker had done his job, so kill him :)
			STVStreamWorker* exWorker = _WorkerPool[i];
			glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVStreamUnit::OnStartStream()  Worker thread %x was killed", exWorker);
			_WorkerPool.erase(_WorkerPool.begin()+i);
			if(exWorker)
				delete exWorker;
		}
		else
		{
//			// this worker is working
//			if(_WorkerPool[i]->getChnlID()==chnlID)
//			{
//				// there is a worker working for this channel
//				bDupWorker = TRUE;
//				glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVStreamUnit::OnStartStream()  There is an existed worker for this channel.");
//				return STVSUCCESS;
//			}
		}
	}

	// give birth to new worker thread and set it work
	STVStreamWorker*	newWorker = createWorker(this, pPL, chnlID, isStart);
	if(!newWorker) {
		glog(ZQ::common::Log::L_ERROR, "FAILURE  STVStreamUnit::OnStartStream()  Can not create new worker thread");
		return STVRTSPFAIL;
	}
	
	newWorker->start();
	
	return STVSUCCESS;
}

STVStreamWorker* STVStreamUnit::createWorker(STVStreamUnit* boss, STVPlaylist* pPL, DWORD ChnlID, bool isStart)
{
	STVStreamWorker*	newWorker = new STVStreamWorker(boss, pPL, ChnlID, _szRtspHostIP, _szRtspURL, _dwRtspHostPort, _dwRtspNsec, isStart );
	_WorkerPool.push_back(newWorker);
	glog(ZQ::common::Log::L_DEBUG, "NOTIFY   STVStreamUnit::createWorker()  Worker thread %x was created. Altogether %d workers now.", newWorker, _WorkerPool.size());
	return newWorker;
}

bool STVStreamUnit::removeWorker(STVStreamWorker* pWorker)
{
	bool retval = FALSE;
	for(size_t i=0; i<_WorkerPool.size(); i++) {
		if(pWorker== _WorkerPool[i]) {
			_WorkerPool.erase(_WorkerPool.begin()+i);	//found, remove it
			retval = TRUE;
		}
	}
	// can not find, return
	if(!retval)
		return FALSE;

	// found, delete it
	delete pWorker;
	return retval;
}