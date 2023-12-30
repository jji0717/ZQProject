
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
// Ident : $Id: FtpMover.h,v 1.5 2004/07/29 05:13:27 wliExp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : interface the main service  class FtpMover
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpMover.h $
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
// 2     05-02-04 11:39 Kaliven.lee
// add head
//


#pragma once
#include "BaseSchangeServiceApplication.h"
#include "FtpMoverScanner.h"
#include "FtpConnMgr.h"

using ZQ::common::BaseSchangeServiceApplication;

class FtpMover: public BaseSchangeServiceApplication  
{
public:
	FtpMover();
	virtual ~FtpMover();
protected:
	HRESULT OnInit(void);
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnUnInit(void);
	bool isHealth(void);

	HRESULT ReadConfigFile(void);

private:
	static FtpConnMgr *m_pFtpConMgr;
	FtpMoverScanner *m_pScanner;
	static wchar_t m_wcsGuardDir[MAX_PATH];
	static wchar_t m_wcsTempDir[MAX_PATH];
	static DWORD m_dwScanDirInterval;
	static DWORD m_dwScanWQInterval;
	static DWORD m_dwMaxConnCount;
	static wchar_t m_wcsConfigFile[MAX_PATH];
private:	
	static UINT MgmtWorkQueue(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength);
	static UINT MgmtFtpConn(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength);
};


