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
// Ident : $Id: TriggerWorkerMgr.h,v 1.7 2004/07/29 06:26:30 wli Exp $
// Branch: $Name:  $
// Author: Kaliven Lee
// Desc  : define the TriggerWorkerMgr
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/TriggerWorkerMgr.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 3     05-04-13 19:17 Kaliven.lee
// thread class change
// 
// 2     05-03-30 17:09 Kaliven.lee
// 
// 1     05-03-07 15:29 Kaliven.lee
// file create
// TriggerWorkerMgr.h: interface for the TriggerWorkerMgr class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "TriggerWorker.h"



class TriggerWorkerMgr : public ZQ::common::NativeThread
{
public:
	TriggerWorkerMgr(DWORD dwWorkerCount = DEFAULTWORKERCOUNT);
	virtual ~TriggerWorkerMgr();
	int stop(void);
	virtual int run(void);
private:
		
	
	bool m_bThreadExit;
	bool m_bQuit;
	HANDLE m_hStop;
	std::vector<TriggerWorker*> m_workerList;
	DWORD m_dwWorkerCount;
};


