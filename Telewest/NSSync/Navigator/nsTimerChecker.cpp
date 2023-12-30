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
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsTimerChecker.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 5     06-12-22 15:59 Bernie.zhao
// fixed connection reset bug
// 
// 3     05-07-05 10:22 Bernie.zhao
// 
// 2     05-04-19 10:30 Bernie.zhao
// autobuild modification
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================

#include "stdafx.h"
#include ".\nstimerchecker.h"
#include "NavigationService.h"
extern NavigationService gNavigator;

nsTimerChecker::nsTimerChecker(void)
{
	_hTerminate = ::CreateEvent(0, FALSE, FALSE, NULL);
}

nsTimerChecker::~nsTimerChecker(void)
{
	::CloseHandle(_hTerminate);
}

int nsTimerChecker::run()
{
	::Sleep(10000);	// sleep for nsTimer start
	while(1)
	{
		DWORD status=::WaitForSingleObject(_hTerminate, DEFAULT_INTERVAL/20*1000);
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
			glog(Log::L_ERROR, L"nsTimerChecker::run()");
			glog(Log::L_DEBUG, L"::WaitForSingleObject() Error - %d", err);
			return 1;
		}

		if( (!gNavigator._theTimer->isRunning()) && (gNavigator._theBuilder->getStatus()!=nsBuilder::DISABLE) )	// timer is idle, restart it
		{
			glog(Log::L_DEBUG, L"nsTimerChecker::run()");
			glog(Log::L_DEBUG, L"WorkQueue supervise timer is not running, restarting it...");
			delete gNavigator._theTimer;
			
			gNavigator._theTimer = new nsTimer(gNavigator._theBuilder);
			gNavigator._theTimer->setInterval(gNavigator.m_dwTimerInterval);
			gNavigator._theTimer->start();
		}


	}
	return 0;
}


