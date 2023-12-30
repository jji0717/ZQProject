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
// Name  : nsBuilder.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : class for navigation service
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/NavigationService.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 10    08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 9     07-03-16 15:25 Ken.qian
// 
// 8     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 7     06-10-19 13:57 Ken.qian
// 
// 1     05-03-25 12:47 Bernie.zhao
// ===========================================================================

///@mainpage NS Sync
///@section Background
/// Navigation service (NS), cooperating with Work Queue (WQ), is a kind of cache mechanism.  
/// It provide synchronized database information between local AM DB and NE DB.  
/// The work queue save the object information that updates or refresh, such as the information about modifying one asset, or delete one element, or update/delete metadata, etc. 
/// These update or refresh operations could be from Asset Manager (AM) of ITV system, or from Navigation Engine (NE).  
/// NSSync is a background service that gets these update or refresh information from Work Queue, and keeps an up-to-date local database for NE.
///@section Purpose
/// The primary responsibility of NS Sync falls into the following categories:
/// * NS Sync watches work queue database, and keep track of every entry, including deleting.
/// * NS Sync periodically update NE tree to reflect correct and up-to-date date views from AM database.

#pragma once
#include "baseschangeserviceapplication.h"

#include "stdafx.h"
#include "ns_def.h"
#include "nsBuilder.h"
#include "nsTimer.h"
#include "nsTimerChecker.h"

#define MAXLEN_DSN		256
#define MAXLEN_UID		256
#define MAXLEN_PWD		256
#define MAXLEN_DB		256
#define MAXLEN_DBTYPE	256

#define DEFAULT_TEATIME		200		// in msec
#define MAX_TEATIME			5000	// in msec

#define DEFAULT_DBCONN_INTERVAL       10   // in second

#define MAX_WQ_OPT_REBUILD		100

#define DB_SP_NS_FOLDERUPDATE						"ns_FolderUpdate"
#define DB_TABLE_NAV_HIERARCHY_QA					"NAV_Hierarchy_QA"
#define DB_TABLE_NAV_FOLDER_CONDITION_QA			"NAV_Folder_Condition_QA"
#define DB_SP_NAV_GETQAENABLEFLAG					"nav_getQAEnableFlag"
#define DB_SP_NAV_GETQAGENERATINGSTATUS				"nav_getQAGeneratingStatus"
#define	DB_SP_NAV_GETOFFERINGWINDOWENABLEFLAG		"nav_getOfferingWindowEnableFlag"
#define	DB_SP_NAV_GETOFFERINGWINDOWENDOFFSETDAYS	"nav_getOfferingWindowEndOffsetDays"

using namespace ZQ::common;

class NavigationService :
	public BaseSchangeServiceApplication
{
public:
	NavigationService(void);
	~NavigationService(void);

	//////////////////////////////////////////////////////////////////////////
	// service control operations	
public:
	// reserve for SeaChange Service
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	HRESULT	OnStart(void);
	HRESULT OnStop(void);

	//////////////////////////////////////////////////////////////////////////
	// attribute methods

	void updateTeaTime() 
		{
			DWORD backTime = m_dwTeaTime;
			getConfigValue(L"TeaTime", &m_dwTeaTime, m_dwTeaTime, false, false);
			if(m_dwTeaTime>MAX_TEATIME || m_dwTeaTime<0)
				m_dwTeaTime = backTime;

			if(m_dwTeaTime!=backTime)
			{
				glog(Log::L_DEBUG, "[TeaTime] has been updated to %ld msec", m_dwTeaTime);
			}
		}

	void exceptionCallback(DWORD ExceptionCode, PVOID ExceptionAddress);

public:
	
	/// pointer to the builder object
	nsBuilder*	_theBuilder;

	/// pointer to the supervise timer object
	nsTimer*	_theTimer;

	/// pointer to the checker for supervise timer
	nsTimerChecker*	_theTimerChecker;

	/// string containing version information
	static wchar_t m_wszVerInfo[64];

	///////////////////////////////////// statistic attributes for manutil /////////////////////////////////////
	// all the member variable which is started with "m_" must be static variable, so that the manutil can monitor the process
	
	//----------- for service
	
	/// idle time between work of folders, in msec.  Added by Bernie, 20051013 03:12:02
	/// this time interval is for NSSync to have a cup of tea, and lease SQL server a bit
	static DWORD m_dwTeaTime;

	/// region id string in database, for NAV_hierarchyuid use
	static wchar_t m_wszRegionID[16];

	/// When the reaches the max value, NSSync will do rebuild directly and ignore all WQ data
	static DWORD m_maxWQOptRebuild;

	/// whether logging of SQL statements is enabled
	static DWORD m_dwSQLTraceEnabled;

	//----------- internal flags

	/// whether this database supports Program Manager, 0 for unknown
	static int m_folderUpdateParamCount;

	/// whether QA Navigation logic is enabled
	static bool m_bQANavigationEnabled;

	//----------- for nsTimer

	/// WorkQueue supervise interval, in seconds
	static DWORD m_dwTimerInterval;

	//----------- for nsBuilder

	/// query timeout value for DB operation
	static DWORD m_dwDBTimeout;

	/// retry interval when DBSync connection failed
	static DWORD m_dwDBConnRetryInterval;

	/// Local AM db ODBC name
	static wchar_t m_wszDSN[MAXLEN_DSN];

	/// Local AM db user id
	static wchar_t m_wszUID[MAXLEN_UID];

	/// Local AM db password
	static wchar_t m_wszPWD[MAXLEN_PWD];

	/// Local AM db database name
	static wchar_t m_wszDATABASE[MAXLEN_DB];

	/// Local AM ODBC type, SQL Server or Oracle
	static wchar_t m_wszDBTYPE[MAXLEN_DBTYPE];

	/// the SP type for folder update
	static DWORD m_folderUpdateSPType;

	/// wait time for fetching Asset/Bundle update in seconds
	static DWORD m_dwUpdateWaitTime;
};
