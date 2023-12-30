
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
// Ident : $Id: FtpMoverConf.h,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : interface the typedef  
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpMoverConf.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 6     05-04-06 20:00 Kaliven.lee
// add version support
// 
// 5     05-04-04 15:39 Kaliven.lee
// version change
// 
// 4     05-04-04 13:33 Kaliven.lee
// 
// 3     05-03-24 17:02 Kaliven.lee
// 
// 2     05-02-04 11:40 Kaliven.lee
// add head
//
#pragma  once
#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <vector>


#include "afxinet.h"
#include "Log.h"
#include "../Common/NativeThread.h"
#include "../Common/locks.h"
#include "XMLPreference.h"

#define FTPCONN_STATE_NOMORAL			0x0000
#define FTPCONN_STATE_TRANSFERRING		0x0001
#define FTPCONN_STATE_READY				0x0002
#define FTPCONN_STATE_TRANSFERRED		0x0003
#define FTPCONN_STATE_READFAILED		0x0004
#define FTPCONN_STATE_WRITEFAILED		0x0005
#define FTPCONN_STATE_CONNECTFAILED		0x0006


#define WRITEBUFFSIZE				10240
#define FTPCONN_TIMEOUT				3000

#define SERVICE_VERSION				_T("0.0.10")

enum TASKSTATE{
	Ready = 0,
	Processing,
	Finished,
	Failed
};

typedef struct tag_FtpServer
{
	wchar_t wcsServer[64];
	wchar_t wcsUserName[64];
	wchar_t wcsPassword[64];
	DWORD dwPort;	
	
}FTPSERVER;

typedef std::vector<FTPSERVER*> SERVERLIST;

typedef struct tag_TransferTask{
	wchar_t wcsFileName[MAX_PATH];
	wchar_t wcsRemoteFileName[MAX_PATH];
	wchar_t wcsDestineServer[64];
	wchar_t wcsUserName[64];
	wchar_t wcsPassword[64];	
	DWORD dwPort;
	double percentage;
	TASKSTATE state;
	DWORD dwFtpConnID;
	// processing show that the file is processing ;
	
}TRANSFERTASK;
class FtpConn;
typedef std::vector<TRANSFERTASK*>	WORKQUEUE;
typedef std::vector<FtpConn*> FTPCONNLIST;

typedef struct tag_DisFtpConnInfo
{
	DWORD dwId;
	wchar_t wcsFileName[MAX_PATH];
	DWORD states;	
}DISFTPCONNINFO;

typedef struct tag_Task
{
	wchar_t wcsGuardDir[MAX_PATH];
	wchar_t wcsTempDir[MAX_PATH];
	SERVERLIST FtpServerList;
}TASK;

typedef std::vector<TASK*>	TASKLIST;

typedef struct tag_FileLock
{
	CString fileName;
	DWORD locks;
}FILELOCK;
typedef std::vector<FILELOCK*> FILELOCKLIST;



