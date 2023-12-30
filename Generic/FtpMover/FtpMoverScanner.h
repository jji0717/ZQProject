
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
// Ident : $Id: FtpMoverScanner.h,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : interface the directory guard  class FtpMoverScanner.h
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpMoverScanner.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 3     05-04-07 17:07 Kaliven.lee
// because of moving project
// 
// 2     05-02-04 11:41 Kaliven.lee
// add head
//
//////////////////////////////////////////////////////////////////////


#pragma once
#include "FtpMoverConf.h"
#include "thread.h"


class FtpMoverScanner: public ZQ::common::NativeThread 
{
public:
	FtpMoverScanner(DWORD dwInterval);
	virtual ~FtpMoverScanner();
public:
	
	int run(void);
	void Stop(void);
	
public:
	void ScanDirectory(void);
private:
	void AddNewTask(wchar_t* transferFile,wchar_t* removeFile,SERVERLIST FtpServerList);
	void InitScan(void);
private:
	DWORD m_dwScanInterval;
	// variable to control the exit of the thread.
	HANDLE m_event;
	bool m_bQuit ;
};

