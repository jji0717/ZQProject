// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: CallBackProcessThread.h,v 1.9 2006/05/19 07:24:54 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Define the encapsulated class for IDS callback data
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/CallBackProcessThread.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 2     07-04-12 17:35 Ken.qian
// Change DBSync auto-restart logic
// 
//
// Revision 1.0  2006/05/19 16:20:20  Ken Qian
// Initial codes, it is avaliable since DBSync 3.6.0
//
// ===========================================================================

#if !defined(AFX_CALLBACKPROCESSTHREAD_H__CEA4156C_0034_4177_9A40_13B92608AAFC__INCLUDED_)
#define AFX_CALLBACKPROCESSTHREAD_H__CEA4156C_0034_4177_9A40_13B92608AAFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <queue>

#include "Locks.h"
#include "NativeThread.h"
#include "DSCallBack.h"

class CallBackProcessThread : public ZQ::common::NativeThread
{
public:
	CallBackProcessThread();
	virtual ~CallBackProcessThread();

protected:
	/// virtual function defined in NativeThread, it must be implement in sub class
	bool init(void);
	int run(void);
	void final(void);

public:
	void StopProcessing(void);
	bool AddCallback(DSCallBackBase* pCallback);
	void RemoveAllCallback();

	int  GetQueueSize(void);
private:
	ZQ::common::Mutex m_mutex;
	std::queue<DSCallBackBase*> m_cbQueue;
	
	HANDLE m_hStop;
	HANDLE m_hNotify;

	bool   m_bContinue;
};

#endif // !defined(AFX_CALLBACKPROCESSTHREAD_H__CEA4156C_0034_4177_9A40_13B92608AAFC__INCLUDED_)
