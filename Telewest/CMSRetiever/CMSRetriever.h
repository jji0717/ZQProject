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
// Ident : $Id: RetrieveWorker.cpp,v 1.8 2004/08/09 10:08:56 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : impl SrvLoad service class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/CMSRetiever/CMSRetriever.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     06-11-08 15:20 Ken.qian
// 
// 2     05-03-24 17:36 Kaliven.lee
// 
// 1     05-03-10 11:12 Kaliven.lee
// file create
#pragma  once
#include "afxdb.h"
#include "baseSchangeServiceApplication.h"
#include "xmlpreference.h"
#include "RetrieveWorker.h"
#define MAXDNSLENGTH		32
#define MAXUSERNAMELENGTH	64
#define MAXPASSWORDLENGTH	64
#define MAXDBNAMELENGTH		64



using ZQ::common::BaseSchangeServiceApplication;
class CMSRetriever: public BaseSchangeServiceApplication
{
public:
	CMSRetriever();
	~CMSRetriever();
public:
	HRESULT OnInit(void);
	HRESULT OnStop(void);
	HRESULT OnStart(void);
	HRESULT OnUnInit(void);
	bool isHealth(void);
	void Stop(void);
	
	bool GetFirstConnString(CString& strSiteName, CString& strconn);
	bool GetNextConnString(CString& strSiteName, CString& strconn);

private:
	wchar_t m_LoadFileName[MAX_PATH];
	wchar_t m_LAMDBDSN[MAXDNSLENGTH];
	wchar_t m_LAMDBUSERNAME[MAXUSERNAMELENGTH];
	wchar_t m_LAMDBPASSWORD[MAXPASSWORDLENGTH];
	wchar_t m_LAMDBNAME[MAXDBNAMELENGTH];
	wchar_t m_szAppSitesCfg[MAX_PATH];
	
private :

	RetrieveWorker* m_pWorker;

	DWORD m_dwWorkTimeOut;
	bool m_bStarted;

	int m_nIndex;
	CStringArray m_arrSites;
	CStringArray m_arrConnStrs;

private:
	bool LoadConfigFile();
};

