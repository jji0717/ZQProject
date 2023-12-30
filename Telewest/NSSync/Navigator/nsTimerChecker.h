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
// Desc  : class for check nsTimer object status
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsTimerChecker.h $
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

#pragma once

// common include
#include "NativeThread.h"

// local include
#include "ns_def.h"
#include "nsBuilder.h"
#include "nsTimer.h"

using namespace ZQ::common;
class nsTimerChecker :
	public NativeThread
{
public:
	nsTimerChecker(void);
	~nsTimerChecker(void);
	
	//////////////////////////////////////////////////////////////////////////
	// attribute methods

	/// signal event that terminate the thread
	void signalTerm() { ::SetEvent(_hTerminate); }

	//////////////////////////////////////////////////////////////////////////
	// overrided functions
	
	virtual int run();
	

	//////////////////////////////////////////////////////////////////////////
	// attributes

	/// signal for stop thread
	HANDLE	_hTerminate;
};
