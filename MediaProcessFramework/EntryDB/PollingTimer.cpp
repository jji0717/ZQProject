// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : impl a timer for polling
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/PollingTimer.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/PollingTimer.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 2     4/12/05 5:20p Hui.shao
// ============================================================================================

#include "PollingTimer.h"

#ifndef WIN32

PollingTimer::PollingTimer()
{
	active = false;
	gettimeofday(&timer, NULL);
}

void PollingTimer::setTimer(timeout_t timeout)
{
	gettimeofday(&timer, NULL);
	active = false;
	if(timeout)
		incTimer(timeout);
}

void PollingTimer::incTimer(timeout_t timeout)
{
	int secs = timeout / 1000;
	int usecs = (timeout % 1000) * 1000;

	timer.tv_usec += usecs;
	if(timer.tv_usec > 1000000l)
	{
		++timer.tv_sec;
		timer.tv_usec %= 1000000l;
	}
	timer.tv_sec += secs;
	active = true;
}

void PollingTimer::endTimer(void)
{
	active = false;
}

timeout_t PollingTimer::getTimer(void)
{
	struct timeval now;
	long diff;

	if(!active)
		return TIMEOUT_INF;

	gettimeofday(&now, NULL);
	diff = (timer.tv_sec - now.tv_sec) * 1000l;
	diff += (timer.tv_usec - now.tv_usec) / 1000l;

	if(diff < 0)
		return 0l;

	return diff;
}

timeout_t PollingTimer::getElapsed(void)
{
	struct timeval now;
	long diff;

	if(!active)
		return TIMEOUT_INF;

	gettimeofday(&now, NULL);
	diff = (now.tv_sec -timer.tv_sec) * 1000l;
	diff += (now.tv_usec - timer.tv_usec) / 1000l;
	if(diff < 0)
		return 0;
	return diff;
}

#else // WIN32

PollingTimer::PollingTimer()
{
	active = false;
  timer = GetTickCount();
}

void PollingTimer::setTimer(timeout_t timeout)
{
  timer = GetTickCount();
	active = false;
	if(timeout)
		incTimer(timeout);
}

void PollingTimer::incTimer(timeout_t timeout)
{
  timer += timeout;
	active = true;
}

void PollingTimer::endTimer(void)
{
	active = false;
}

timeout_t PollingTimer::getTimer(void)
{
	DWORD now;
	long diff;

	if(!active)
		return TIMEOUT_INF;

	now = GetTickCount();
  diff = timer - now;

	if(diff < 0)
		return 0l;

	return diff;
}

timeout_t PollingTimer::getElapsed(void)
{
  return getTimer();
}

#endif // WIN32

/*
-------------------------------------------------------------
 Revision history since 2003/03/20:
 $Log: /ZQProjs/MediaProcessFramework/EntryDB/PollingTimer.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 2     4/12/05 5:20p Hui.shao
// 
// 1     3/02/05 5:53p Hui.shao
// Entry Database Architecture
 Revision 1.2  2003/06/03 22:49:34  shao
 no message

 Revision 1.1.1.1  2003/05/30 23:00:21  shao
 Enterprise Domain Objects by Hui Shao

 Revision 1.2  2003/03/20 04:18:51  shao
 Copy to Infraworks VSS

-------------------------------------------------------------
*/

