
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
// Ident : $Id: FtpConn.h,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : interface the Connection  class FtpConn
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpConn.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 5     05-03-28 17:50 Kaliven.lee
// 
// 4     05-03-24 17:02 Kaliven.lee
// 
// 2     05-02-04 11:37 Kaliven.lee
// add head
// 
//
//////////////////////////////////////////////////////////////////////
// FtpConn.h: interface for the FtpConn class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "FtpMoverConf.h"



class FtpConn : public ZQ::common::NativeThread 
{
	friend class FtpMover;
public:
	FtpConn(bool isPassive = false);
	virtual ~FtpConn();
private:
	CInternetSession* m_pSession;
	bool m_isPassive;
	FTPSERVER * m_pFtpServer;
	CFtpConnection* m_ftpConn;
//	FtpClient m_FtpClient;
	bool m_isBusy;
	DWORD m_dwState;
	bool m_bQuit;
	TRANSFERTASK* m_pTask;
	HANDLE m_StartEvent;
	HANDLE m_StopEvent;
	DWORD m_id;
	static DWORD m_UnitID;
public:
	bool IsBusy(void);
	DWORD GetCurState(void);	
	

	void StartTransfer(TRANSFERTASK* TransferTask);
	void Stop(void);
	DWORD GetID(void);
private:
	void SetTask(TRANSFERTASK* TransferTask);
	bool DisConnect(void);
	int run();
	void ExecuteTask(void);
	bool Connect(wchar_t* server,wchar_t* username,wchar_t* password,DWORD dwPort = 80 ,bool isPassive = false);
	bool PutFile(wchar_t* fileName,wchar_t* remoteFileName);
};


