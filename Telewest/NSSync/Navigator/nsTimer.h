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
// Name  : nsTimer.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : class for NS service timer
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsTimer.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     05-07-05 10:22 Bernie.zhao
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================

#if !defined(AFX_NSTIMER_H__CB5CB4E8_808E_4612_9B8B_046FEA5E24E8__INCLUDED_)
#define AFX_NSTIMER_H__CB5CB4E8_808E_4612_9B8B_046FEA5E24E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// common include
#include "NativeThread.h"

// local include
#include "ns_def.h"
#include "nsBuilder.h"

#define DEFAULT_INTERVAL	600

#define DEFAULT_TIMEOUT		15

using namespace ZQ::common;
class nsTimer : public NativeThread 
{
//////////////////////////////////////////////////////////////////////////
public:
	nsTimer(nsBuilder* theBuilder);
	virtual ~nsTimer();

//////////////////////////////////////////////////////////////////////////
// attribute method
public:
	/// get timer supervise interval
	///@return the supervise interval, in seconds
	DWORD getInterval() { return _interval; }

	/// set timer supervise interval
	///@param[in] interval	the supervise interval, in seconds
	void setInterval(DWORD interval) { _interval = interval; }

	/// signal event that terminate the thread
	void signalTerm() { ::SetEvent(_hTerminate); }

//////////////////////////////////////////////////////////////////////////
// overrided functions
	virtual int run();
	
	//////////////////////////////////////////////////////////////////////////
// attributes
public:
	/// pointer to NE database builder
	nsBuilder*	_pTheBuilder;	

	/// timer interval for supervising WorkQueue
	DWORD	_interval;

	/// signal for stop thread
	HANDLE	_hTerminate;
};

#endif // !defined(AFX_NSTIMER_H__CB5CB4E8_808E_4612_9B8B_046FEA5E24E8__INCLUDED_)
