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
// Name  : STVPMTimerMan.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-14
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/PLAYLISTMOD/STVPMTimerMan.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     06-04-30 17:46 Bernie.zhao
// 
// 2     05-12-27 15:19 Bernie.zhao
// 
// 1     05-08-30 18:30 Bernie.zhao
// ===========================================================================

#include "STVPMTimerMan.h"
#include "STVPlaylistManager.h"
#include "../MainCtrl/ScheduleTV.h"

extern ScheduleTV gSTV;

ZQ::common::ComInitializer	STVPMTimerMan::_coInit;

STVPMTimerMan::STVPMTimerMan(STVPlaylistManager& paramPM)
:_thePM(paramPM)
{
	_hWakeupOrDie[0]	= ::CreateEvent(0,0,0,0);
	_hWakeupOrDie[1]	= ::CreateEvent(0,0,0,0);
	_dwPenalty = 0;
}

STVPMTimerMan::~STVPMTimerMan()
{
	::CloseHandle(_hWakeupOrDie[0]);
	::CloseHandle(_hWakeupOrDie[1]);
}

void	STVPMTimerMan::penaltyIncr() 
{ 
	_dwPenalty+=gSTV.m_dwPenaltyIncrRate; 
}

int STVPMTimerMan::run()
{
	bool bStartup = true;
	DWORD waitstatus;

	// sleep for 30 seconds for ISS init to complete
	glog(ZQ::common::Log::L_INFO, "STVPMTimerMan::run()  playlist timer init...");
	
	if(WAIT_OBJECT_0 == ::WaitForSingleObject(_hWakeupOrDie[1], DEFAULT_SLEEPTIME*10))
	{
		glog(ZQ::common::Log::L_INFO, "STVPMTimerMan::run()  playlist timer terminated");
		return 0;
	}

	
	glog(ZQ::common::Log::L_INFO, "STVPMTimerMan::run()  playlist timer started");
	
	
		
	for(;;) 
	{
		try 
		{
			DWORD nextsleeptime = DEFAULT_SLEEPTIME*5;

			//////////////////////////////////////////////////////////////////////////
			// test penalty, if high, do not care any stream
			if(!penaltyTest())
			{
				glog(ZQ::common::Log::L_DEBUG, "STVPMTimerMan::run()  Penalty too high: %ld, waiting...", _dwPenalty);
				Sleep(nextsleeptime);
				penaltyDecr();
				continue;
			}
			
			//////////////////////////////////////////////////////////////////////////
			// scan channel to activate newly added filler/barker
			nextsleeptime = _thePM.scan(bStartup);
			bStartup = false;

			// sleep until time reached or wakeup/terminate signal invoked
			waitstatus = ::WaitForMultipleObjects(2, _hWakeupOrDie, false, nextsleeptime);

			if(waitstatus == WAIT_OBJECT_0) 
			{
				// wake up signal, wake up and re-scan the list
				continue;
			}
			else if(waitstatus == WAIT_OBJECT_0+1) 
			{
				// PM ended, should terminate this thread
				glog(ZQ::common::Log::L_INFO, "STVPMTimerMan::run()  playlist timer terminated");
				return 0;
			}
			else if(waitstatus == WAIT_TIMEOUT) {
				// timeout, wake up and re-scan the list
				continue;
			}
			else 
			{
				DWORD err = ::GetLastError();
				glog(ZQ::common::Log::L_ERROR, "STVPMTimerMan::run()  ::WaitForSingleObject() Error - %d", err);
			}

		}
		catch(std::exception &e) {
			glog(ZQ::common::Log::L_CRIT, "STVPMTimerMan::run()  Got std exception: %s", e.what());
		}
		catch(...) {
			glog(ZQ::common::Log::L_CRIT, "STVPMTimerMan::run()  Got unknown exception");
		}
	
		
	}	// end for(;;)
	
	return 0;
}
