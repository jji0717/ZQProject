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
// Desc  : declaration of STV stream bring up thread collection unit
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/MainCtrl/STVStreamUnit.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 4     05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 3     04-10-14 14:56 Bernie.zhao
// 
// 2     04-10-11 15:19 Bernie.zhao
// 
// 1     04-10-07 9:09 Bernie.zhao
// ===========================================================================

#if !defined(AFX_STVSTREAMUNIT_H__CFA19992_2F34_4F71_A5B3_B3F01479F3E0__INCLUDED_)
#define AFX_STVSTREAMUNIT_H__CFA19992_2F34_4F71_A5B3_B3F01479F3E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "STVStreamWorker.h"

class STVStreamUnit  
{
public:
	///@param[in]	RtspHostIP	the ip addr of RTSP server
	///@param[in]	RtspURL		the dummy asset URL
	///@param[in]	RtspHostPort	the port of RTSP server
	///@param[in]	RtspNsec		the time out of RTSP
	STVStreamUnit(const char* RtspHostIP, const char* RtspURL, DWORD RtspPort, DWORD RtspNsec);
	virtual ~STVStreamUnit();

	///////////////////////////////////// stream operation /////////////////////////////////////
public:
	/// create a new Stream Worker thread to fill AElist or start RTSP request
	///@param[in]	pPL		the pointer to the playlist object that works on
	///@param[in]	chnlD	the channel ID
	///@param[in]	isStart	start stream or end stream
	///@return				error code
	HRESULT OnStartStream(STVPlaylist* pPL, DWORD chnlID, bool isStart);

	///////////////////////////////////// worker thread methods /////////////////////////////////////
protected:
	/// create a new worker thread and register it into pool
	///@param[in]	boss	the STVStreamUnit object pointer which controls the worker
	///@param[in]	pPL		the playlist this worker would deal with
	///@param[in]	isStart	indicating whether start or end stream
	///@return				new worker pointer
	STVStreamWorker* createWorker(STVStreamUnit* boss, STVPlaylist* pPL, DWORD ChnlID, bool isStart);

	/// remove specific worker out of pool and delete it
	///@param[in]	pWorker	the worker to delete
	///@return				ture if remove successfully
	bool	removeWorker(STVStreamWorker* pWorker);
	
	///////////////////////////////////// thread related attributes /////////////////////////////////////
private:
	std::vector<STVStreamWorker*>	_WorkerPool;

	/// RTSP related attributes
	char		_szRtspHostIP[32];
	DWORD		_dwRtspHostPort;
	DWORD		_dwRtspNsec;
	char		_szRtspURL[MAX_URL_LEN];

};

#endif // !defined(AFX_STVSTREAMUNIT_H__CFA19992_2F34_4F71_A5B3_B3F01479F3E0__INCLUDED_)
