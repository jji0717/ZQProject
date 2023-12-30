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
// Ident : $Id: STVPlaylist.h$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : declaration of STV timer thread
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/PlaylistMod/STVPMTimerMan.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 10    05-06-27 18:04 Bernie.zhao
// June.27/2005.  Fixed interface error with SM when creating new STV list
// 
// 9     05-06-09 10:16 Bernie.zhao
// 
// 8     05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 7     05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 6     04-10-21 15:29 Bernie.zhao
// 
// 5     04-10-11 15:20 Bernie.zhao
// mem problem?
// 
// 4     04-09-30 14:32 Bernie.zhao
// before National Season
// 
// 3     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 2     04-09-17 17:09 Bernie.zhao
// ===========================================================================
// STVPMTimerMan.h: interface for the STVPMTimerMan class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STVPMTIMERMAN_H__6EF7B8AF_C43F_4842_BFB9_B5FB36B4E5B3__INCLUDED_)
#define AFX_STVPMTIMERMAN_H__6EF7B8AF_C43F_4842_BFB9_B5FB36B4E5B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable: 4786)
#include <vector>
#include <map>

#include "ZqThread.h"
#include "STVPlaylistManager.h"

#define		STV_TIMECRIT_ASSOCIATE		300
#define		STV_TIMECRIT_PLAY		1

#define		DEFAULT_SLEEPTIME		3000

#define		PENALTY_NORMAL			10
#define		PENALTY_DEFAULT_INC		5
#define		PENALTY_DEFAULT_DEC		1

class STVPlaylistManager;

/// class STVPMTimerMan functions as a timer to meet each time point of playlists
//##ModelId=4147B84B00CB
class STVPMTimerMan : 
			public ZQ::comextra::Thread  
{
private:
	/// play manager which holds control of this timer thread
	//##ModelId=4147B84B00EB
	STVPlaylistManager*		_thePM;

	/// indicate how long the timer loop will sleep
	//##ModelId=4147B84B00EF
	DWORD	_nextsleeptime;

	/// event handles for wake up thread and terminate thread
	/// _hWakeupOrDie[0] is wake up event
	/// _hWakeupOrDie[1] is termination event
	HANDLE	_hWakeupOrDie[2];

	/// approximate penalty value for CM
	DWORD	_dwPenalty;
	
//	/// event handle for termination
//	HANDLE	_hSTVTimerEnd;
//	/// event handle for wake up
//	HANDLE	_hSTVTimerWakeup;
	
public:
	//##ModelId=4147B84B00FA
	STVPMTimerMan(STVPlaylistManager* paramPM);
	//##ModelId=4147B84B0109
	virtual ~STVPMTimerMan();

	/// overrided main thread function
	virtual int run();

public:
	/// test if penalty value in within normal level
	bool penaltyTest() { return (_dwPenalty<PENALTY_NORMAL?true:false); }

	/// increase penalty value
	void penaltyIncr() { _dwPenalty+=PENALTY_DEFAULT_INC; }

	/// decrease penalty value
	void penaltyDecr() { _dwPenalty-=PENALTY_DEFAULT_DEC; }

public:
	/// state change method

	/// signal that thread should be terminated
	void OnTerminate() { ::SetEvent(_hWakeupOrDie[1]); }

	/// signal that thread should be waken up
	void OnWakeup() { ::SetEvent(_hWakeupOrDie[0]); }
	
};

#endif // !defined(AFX_STVPMTIMERMAN_H__6EF7B8AF_C43F_4842_BFB9_B5FB36B4E5B3__INCLUDED_)
