
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
// Ident : $Id: FtpConnMgr.h,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : interface the Connection manager class FtpConnMgr
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpConnMgr.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 3     05-03-14 10:25 Kaliven.lee
// 0.0.6
// 
// 2     05-02-04 11:38 Kaliven.lee
// add head
// 
//
//////////////////////////////////////////////////////////////////////


#pragma once

#include "FtpMoverConf.h"
#include "FtpConn.h"

class FtpConnMgr : public ZQ::common::NativeThread
{
public:
	friend class FtpMover;
	FtpConnMgr(DWORD MaxConns,DWORD ScanInterval);
	virtual ~FtpConnMgr();
private:
	FTPCONNLIST ConnectionList;
	ZQ::common::Mutex m_ConnLock;
	DWORD m_dwMaxConnects;
	DWORD m_dwInterval;
	HANDLE m_event;
	bool m_bQuit;

public:
	int run();
	void Stop(void);	
	void ScanWorkQueue(void);
	void Initialize(void);	
};


