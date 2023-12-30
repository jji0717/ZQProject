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
// Ident : $Id: TriggerWorker.h,v 1.7 2004/07/29 06:26:30 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : define the TriggerWorker
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/TriggerWorker.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 6     05-04-26 15:08 Kaliven.lee
// friend class
// 
// 5     05-04-13 19:17 Kaliven.lee
// thread class change
// 
// 4     05-04-01 16:23 Kaliven.lee
// 3.3.0 patch
// modify the borderline to decide if a worker is busy.
// 
// 3     05-03-30 17:09 Kaliven.lee
// 
// 2     05-03-17 19:42 Kaliven.lee
// 3.2.11
// 
// 1     05-03-07 15:29 Kaliven.lee
// file create
// TriggerWorker.h: interface for the TriggerWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "DBSyncConf.h"
#pragma once

class TriggerWorker : public ZQ::common::NativeThread  
{
	friend class TriggerWorkerMgr;
public:
	TriggerWorker();
	virtual ~TriggerWorker();
	void SetProcTask(DETAILTASK* task);
	void Stop(void);
	int run(void);
	void StartProc(DETAILTASK* task);
	bool IsProccessing(void);
private:
	
	DETAILTASK m_task;
	HANDLE m_hStop;
	HANDLE m_hEvent;
	bool m_bQuit;
	bool m_bProccessing;
	bool m_bThreadExit;
	DWORD m_dwID;
	static DWORD m_ID;
//	static std::map<DWORD,DWORD> m_AssetsLock;
};


