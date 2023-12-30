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
// Name  : STVPMTimerMan.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-14
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/PLAYLISTMOD/STVPMTimerMan.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// ===========================================================================


#if !defined(AFX_STVPMTIMERMAN_H__6EF7B8AF_C43F_4842_BFB9_B5FB36B4E5B3__INCLUDED_)
#define AFX_STVPMTIMERMAN_H__6EF7B8AF_C43F_4842_BFB9_B5FB36B4E5B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

#include "nativeThread.h"
#include "XMLPreference.h"

#define		PENALTY_NORMAL			10
#define		PENALTY_DEFAULT_INC		5
#define		PENALTY_DEFAULT_DEC		1

class STVPlaylistManager;

/// class STVPMTimerMan functions as a timer to meet each time point of playlists
class STVPMTimerMan : 
			public ZQ::common::NativeThread
{
public:
	STVPMTimerMan(STVPlaylistManager& paramPM);

	virtual ~STVPMTimerMan();

	/// overrided main thread function
	virtual int	run();

public:
	// penalty operations
	
	/// test if penalty value in within normal level
	bool		penaltyTest() { return (_dwPenalty<PENALTY_NORMAL?true:false); }

	/// increase penalty value
	void		penaltyIncr();

	/// decrease penalty value
	void		penaltyDecr() { _dwPenalty-=PENALTY_DEFAULT_DEC; }

public:
	// state change method

	/// signal that thread should be terminated
	void		OnTerminate() { ::SetEvent(_hWakeupOrDie[1]); }

	/// signal that thread should be waken up
	void		OnWakeup() { ::SetEvent(_hWakeupOrDie[0]); }

public:
	/// static com init class
	static	ZQ::common::ComInitializer	_coInit;

private:
	/// play manager which holds control of this timer thread
	STVPlaylistManager&		_thePM;

	/// event handles for wake up thread and terminate thread
	/// _hWakeupOrDie[0] is wake up event
	/// _hWakeupOrDie[1] is termination event
	HANDLE					_hWakeupOrDie[2];

	/// approximate penalty value for CM
	DWORD					_dwPenalty;
};

#endif // !defined(AFX_STVPMTIMERMAN_H__6EF7B8AF_C43F_4842_BFB9_B5FB36B4E5B3__INCLUDED_)
