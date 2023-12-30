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
// Name  : STVStreamUnit.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-14
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/MAINCTRL/STVStreamUnit.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 2     05-10-24 15:09 Bernie.zhao
// move to lap
// 
// 1     05-08-30 18:29 Bernie.zhao
// ===========================================================================

#include "../RtspClient/RtspHeaders.h"
#include "../STVMainHeaders.h"
#include "ScLog.h"
#include "STVStreamUnit.h"

#define			WORKER_DELAY_STEP		500

std::string		STVStreamUnit::_strRtspHostIP;
DWORD			STVStreamUnit::_dwRtspHostPort	= RTSP_DEFAULT_PORT;
DWORD			STVStreamUnit::_dwRtspNsec		= RTSP_DEFAULT_NSEC;
std::string		STVStreamUnit::_strRtspURL;

STVStreamUnit::STVStreamUnit()
{
	_workerPool.clear();
	_workerDelayTime = 0;
	_workerShouldDelay = false;
}

STVStreamUnit::~STVStreamUnit()
{
	_workerPool.clear();
}

HRESULT STVStreamUnit::OnStartStream(DWORD ChnlID, const char* schNO, int ieIndex)
{
	// kiss dummy worker threads goodbye first
	clearWorkers();
	
	// give birth to new worker thread and set it work
	STVStreamWorker*	newWorker = createWorker(ChnlID);
	if(!newWorker) {
		glog(ZQ::common::Log::L_ERROR, "STVStreamUnit::OnStartStream()  Can not create new worker thread");
		return STVRTSPFAIL;
	}
	
	newWorker->setStartParam(schNO, ieIndex);
	newWorker->start();
	
	return STVSUCCESS;
}

HRESULT STVStreamUnit::OnStopStream(DWORD chnlID)
{
	// kiss dummy worker threads goodbye first
	clearWorkers();
	
	// give birth to new worker thread and set it work
	STVStreamWorker*	newWorker = createWorker(chnlID);
	if(!newWorker) {
		glog(ZQ::common::Log::L_ERROR, "STVStreamUnit::OnStartStream()  Can not create new worker thread");
		return STVRTSPFAIL;
	}
	
	newWorker->setStopParam();
	newWorker->start();
	
	return STVSUCCESS;
}

STVStreamWorker* STVStreamUnit::createWorker(DWORD ChnlID)
{
	ZQ::common::MutexGuard	tmpGd(_workerMutex);
	
	STVStreamWorker*	newWorker = new STVStreamWorker(ChnlID);
	_workerPool.push_back(newWorker);

	if(_workerShouldDelay && _workerDelayTime>=0)
	{
		newWorker->setDelayTime(_workerDelayTime);
		glog(ZQ::common::Log::L_DEBUG, "STVStreamUnit::createWorker()  Worker thread %x was created with delay %ld. Altogether %d workers now.", newWorker, _workerDelayTime, _workerPool.size());
		_workerDelayTime += WORKER_DELAY_STEP;
	}
	else
	{
		glog(ZQ::common::Log::L_DEBUG, "STVStreamUnit::createWorker()  Worker thread %x was created without delay. Altogether %d workers now.", newWorker, _workerPool.size());
	}
	return newWorker;
}

void STVStreamUnit::clearWorkers()
{
	ZQ::common::MutexGuard	tmpGd(_workerMutex);

	if(_workerPool.empty())
		return;

	std::vector<STVStreamWorker*>::iterator iter=0;
	for(iter=_workerPool.begin(); iter!=_workerPool.end(); )
	{
		if((*iter)==NULL)
		{
			_workerPool.erase(iter);
		}
		else if(!(*iter)->isWorking())
		{	
			// this worker had done his job, so kill him
			STVStreamWorker* exWorker = (*iter);
			_workerPool.erase(iter);
			if(exWorker)
				delete exWorker;
			exWorker = NULL;
		}
		else
		{
			iter++;
		}
	}
}

void STVStreamUnit::enableDelay()
{
	_workerShouldDelay = true;
}

void STVStreamUnit::disableDelay()
{
	_workerShouldDelay = false;
}

void STVStreamUnit::resetDelay()
{
	_workerDelayTime = 0;
	_workerShouldDelay = false;
}