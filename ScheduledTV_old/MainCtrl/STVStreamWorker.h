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
// Desc  : declaration of STV stream bring up worker thread
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/MainCtrl/STVStreamWorker.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 7     05-06-09 10:16 Bernie.zhao
// 
// 6     05-03-24 14:53 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 5     05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 4     04-10-19 17:21 Bernie.zhao
// for QA release
// 
// 3     04-10-14 14:56 Bernie.zhao
// 
// 2     04-10-11 15:19 Bernie.zhao
// 
// 1     04-10-07 9:09 Bernie.zhao
// ===========================================================================

#if !defined(AFX_STVSTREAMWORKER_H__5A5E9195_12AF_4298_8E8A_D54C8C8687A0__INCLUDED_)
#define AFX_STVSTREAMWORKER_H__5A5E9195_12AF_4298_8E8A_D54C8C8687A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZqThread.h"

#include "../PlaylistMod/STVPlaylistManager.h"
#include "../RtspClient/RtspConnectionManager.h"

class ScheduleTV;
class STVStreamUnit;

class STVStreamWorker  
	:public ZQ::comextra::Thread
{
public:
	STVStreamWorker(STVStreamUnit* boss, STVPlaylist* pPL, DWORD ChnlID, const char* RtspHostIP, const char* RtspURL, DWORD RtspPort, DWORD RtspNsec, bool isStart);
	virtual ~STVStreamWorker();
public:

	virtual int run();

public:
	/// test if assigned a work
	bool isWorking() { return _bWorking; }

	/// get purchase id (channel id)
	DWORD	getChnlID() { return _PurID; }
	
private:
	/// Tear down last stream of this channel and start new stream
	bool StartNewStream();

	/// Tear down last stream
	bool ShutdownStream();

private:
	/// STVStreamUnit object
	STVStreamUnit*	_Boss;
	/// Playlist works with
	STVPlaylist*	_pPlaylist;
	/// Related Purchase ID
	DWORD			_PurID;
	
	/// start stream of end stream
	bool			_isStart;

	/// RTSP related attributes
	char		_szRtspHostIP[32];
	DWORD		_dwRtspHostPort;
	DWORD		_dwRtspNsec;
	char		_szRtspURL[MAX_URL_LEN];

	/// if is working or not
	bool		_bWorking;
};

#endif // !defined(AFX_STVSTREAMWORKER_H__5A5E9195_12AF_4298_8E8A_D54C8C8687A0__INCLUDED_)
