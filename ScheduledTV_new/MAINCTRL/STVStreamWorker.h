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
// Name  : STVStreamWorker.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-14
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/MAINCTRL/STVStreamWorker.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// ===========================================================================


#if !defined(AFX_STVSTREAMWORKER_H__5A5E9195_12AF_4298_8E8A_D54C8C8687A0__INCLUDED_)
#define AFX_STVSTREAMWORKER_H__5A5E9195_12AF_4298_8E8A_D54C8C8687A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include "nativethread.h"

class ScheduleTV;
class STVStreamUnit;

class STVStreamWorker  
	:public ZQ::common::NativeThread
{
	enum workerType
	{
		WORKER_NONE =0,		// no work to do
		WORKER_START,		// start a stream
		WORKER_STOP			// stop a stream
	};

public:
	STVStreamWorker(DWORD ChnlID);
	virtual ~STVStreamWorker();

protected:
	virtual int run();

public:
	/// test if assigned a work
	bool	isWorking() { return _bWorking; }

	/// get purchase id (channel id)
	DWORD	getChnlID() { return _chnlID; }
	
public:
	bool	setStartParam(const char* scheNo, int ieIndex);

	bool	setStopParam();

	void	setDelayTime(DWORD delay) {_delayTime = delay;}

private:
	/// Tear down last stream of this channel and start new stream
	bool	StartNewStream();

	/// Tear down last stream
	bool	ShutdownStream();

private:
	/// Related Purchase ID
	DWORD			_chnlID;
	
	/// start stream of end stream
	workerType		_type;

	/// if is working or not
	bool			_bWorking;

	/// schedule no, for starting only
	std::string		_scheNO;

	/// schedule ie index, for starting only
	int				_ieIndex;

	/// delay time for starting stream, in msec
	DWORD			_delayTime;
};

#endif // !defined(AFX_STVSTREAMWORKER_H__5A5E9195_12AF_4298_8E8A_D54C8C8687A0__INCLUDED_)
