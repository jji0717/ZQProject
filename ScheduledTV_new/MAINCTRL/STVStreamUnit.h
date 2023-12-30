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
// Name  : STVStreamUnit.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-14
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/MAINCTRL/STVStreamUnit.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// ===========================================================================


#if !defined(AFX_STVSTREAMUNIT_H__CFA19992_2F34_4F71_A5B3_B3F01479F3E0__INCLUDED_)
#define AFX_STVSTREAMUNIT_H__CFA19992_2F34_4F71_A5B3_B3F01479F3E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786)
#include <vector>

#include "Locks.h"
#include "STVStreamWorker.h"

class STVStreamUnit  
{
public:
	STVStreamUnit();
	virtual ~STVStreamUnit();

	///////////////////////////////////// stream operation /////////////////////////////////////
public:
	/// create a new Stream Worker thread to start a stream via RTSP
	///@param[in]	chnlD	the channel ID
	///@return				error code
	HRESULT				OnStartStream(DWORD ChnlID, const char* schNO, int ieIndex);

	/// create a new Stream Worker thread to stop a stream via RTSP
	///@param[in]	chnlD	the channel ID
	///@return				error code
	HRESULT				OnStopStream(DWORD chnlID);

public:
	void				resetDelay();
	void				enableDelay();
	void				disableDelay();

	///////////////////////////////////// worker thread methods /////////////////////////////////////
protected:
	/// create a new worker thread and register it into pool
	///@param[in]	boss	the STVStreamUnit object pointer which controls the worker
	///@param[in]	pPL		the playlist this worker would deal with
	///@param[in]	isStart	indicating whether start or end stream
	///@return				new worker pointer
	STVStreamWorker*	createWorker(DWORD ChnlID);

	/// remove workers that have finished their jobs
	///@return				ture if remove successfully
	void				clearWorkers();
	
	///////////////////////////////////// thread related attributes /////////////////////////////////////
private:
	ZQ::common::Mutex				_workerMutex;
	
	std::vector<STVStreamWorker*>	_workerPool;

	int								_workerDelayTime;
	bool							_workerShouldDelay;

public:
	/// RTSP related attributes
	static std::string		_strRtspHostIP;
	static DWORD			_dwRtspHostPort;
	static DWORD			_dwRtspNsec;
	static std::string		_strRtspURL;

};

#endif // !defined(AFX_STVSTREAMUNIT_H__CFA19992_2F34_4F71_A5B3_B3F01479F3E0__INCLUDED_)
