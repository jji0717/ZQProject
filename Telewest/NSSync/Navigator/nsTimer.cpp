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
// Name  : nsTimer.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsTimer.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 4     06-12-22 15:59 Bernie.zhao
// fixed connection reset bug
// 
// 2     05-07-05 10:22 Bernie.zhao
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================


#include "stdafx.h"
#include "nsTimer.h"
#include "Log.h"
#include "ScLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

nsTimer::nsTimer(nsBuilder* theBuilder)
{
	_pTheBuilder = theBuilder;
	_interval = DEFAULT_INTERVAL;

	_hTerminate = ::CreateEvent(0, FALSE, FALSE, NULL);
}

nsTimer::~nsTimer()
{
	::CloseHandle(_hTerminate);
}

int nsTimer::run()
{
	glog(Log::L_DEBUG, "nsTimer::run()");
	glog(Log::L_DEBUG, L"Begin WorkQueue supervise, every %ld seconds", _interval);

	while(1)
	{
		DWORD status=::WaitForSingleObject(_hTerminate, _interval*1000);
		if(status==WAIT_OBJECT_0) {
			// should terminate thread
			break;
		}
		else if(status==WAIT_TIMEOUT) {
			// no signal
		}
		else {
			// error happened
			int err = ::GetLastError();
#ifdef _DEBUG
			printf("::WaitForSingleObject() Error - %d \n", err);
#endif
			glog(Log::L_ERROR, L"nsTimer::run()");
			glog(Log::L_DEBUG, L"::WaitForSingleObject() Error - %d", err);
			return 1;
		}

		if(_pTheBuilder->getStatus()==nsBuilder::IDLE) {
			// builder is not working currently, get it work
			_pTheBuilder->build();
		}
		else {
#ifdef _DEBUG
			printf("NS is busy, wait for next time\n");
#endif
		}
	}
	return 0;
}
