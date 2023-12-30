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
// Name  : navWorker.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : classe for abstract NAV database builder woker
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navWorker.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 6     08-12-23 20:33 Ken.qian
// add addtional codes if ParentHUID was null or empety
// 
// 5     08-12-23 12:08 Ken.qian
// add codes to calculate entrycount and handle the manual mantained
// asset/bundle
// 
// 4     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================


#if !defined(AFX_NAVWORKER_H__CCB0C786_E021_49B3_A0D5_83C71B91FE43__INCLUDED_)
#define AFX_NAVWORKER_H__CCB0C786_E021_49B3_A0D5_83C71B91FE43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// external include
//#include <string>
#include <afxdb.h>

// common include
#include "Log.h"
#include "ScLog.h"

// local include
#include "stdafx.h"
#include "ns_def.h"
#include "CNAVTable.h"

class navWorkerProvider;

enum WQStatus
{
	wq_waiting = 0,
	wq_building,
	wq_completed,
	wq_skipped,
	wq_failed
};

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFUNC__ WIDEN(__FILE__) WIDEN("(") WIDEN(__STR1__(__LINE__)) WIDEN(")")

#define		__WQAID__	((_isQA)? L"QANAV":L"  NAV")

typedef struct _WQ_Entry
{
	// --------------------------------- from AM DB ---------------------------------
	/// queue_UID in db
	CString	Queue_UID;		
	/// specified DBSync or Navigation
	int		Source_type;		
	/// hierarchy UID of operation object
	CString	local_entry_UID;	
	/// parent hierarchy UID of operation object
	CString	Parent_HUID;		
	/// object name (maybe for 'AM rename' use)
	CString	Entry_Name;			
	/// object type
	int		Entry_type;			
	/// operation category
	int		Operation_type;		
	/// operation time
	CTime	Operation_time;
	/// meta-data name
	CString	MD_name;
	// --------------------------------- Local use ---------------------------------
	/// status of the entry
	WQStatus	Status;
	/// task start time
	CTime	Start_time;
	/// task end time
	CTime	End_time;
} WQ_Entry;

// string format functions
const WCHAR*	WQSourceTypeToString(WQ_Entry& entry);
const WCHAR*	WQEntryTypeToString(WQ_Entry& entry);
const WCHAR*	WQOperationTypeToString(WQ_Entry& entry);
const WCHAR*	WQStatusToString(WQ_Entry& entry);

#define CTIMETOSTRING(_BUF, _VALUE)   { if(_VALUE.GetYear()==1970) { _BUF += wsprintf(_BUF, L"N/A\n"); } \
	else { CString s = _VALUE.Format( "%m-%d %H:%M:%S" ); \
	_BUF += wsprintf(_BUF, L"%s\n", s); } }

#define NASTRING(_VALUE) ((_VALUE!="")?_VALUE:L"N/A")

CTime NullCTime();

using namespace ZQ::common;

/// this class is base class for all navWorker class
/// it uses factory pattern, all specific workers are born by navWorkerProvider class object
class navWorker  
{
protected:
    friend class navWorkerProvider;

	/// protected constructor, use navWorkerProvider to create instance
	navWorker(WQ_Entry& wqentry, CString connectStr);

	/// protected destructor, call free() to delete self
	virtual ~navWorker();

public:
	/// set timeout attribute
	void setTimeout(DWORD timeout) { _dbTimeout = timeout; _db.SetQueryTimeout(_dbTimeout); }

	/// get timeout attribute
	DWORD getTimeout() { return _dbTimeout; }

	/// sleep in tea time
	void haveTea();

	/// get QA attribute
	bool getIsQA() { return _isQA; }

	/// set QA attribute
	void setIsQA(bool isQA) { _isQA = isQA; }

	/// update entry count for a bundle in NAV_Hierarchy table
	///@param[in]	entryHUID the LocalEntryUID of this bundle in Folder table
	///@return		NS_SUCCESS if successfully
	int bundleCountUpdate(CString entryUID);

public:
	/// get to work
	virtual int work();

	/// self destructor method
	virtual void free();

protected:
	/// trace SQL statements
	void traceSQL(CString statement, CString funcName);


	CString tableName(CString name) 
		{	CString retVal = (_isQA)? (L" "+name+L"_QA ") : (L" "+name+L" "); 
			return retVal;
		}

protected:
	/// work queue entry associated with
	WQ_Entry _wq_Entry;

	/// database object
	CDatabase _db;

	/// database connect string, eg: "ODBC;DSN=LAM;UID=am;PWD=am;DATABASE=am"
	CString _connDBStr;

	/// recordset connect string, eg: "DSN=LAM;UID=am;PWD=am;DATABASE=am"
	CString _connRCStr;

	/// query timeout attribute
	DWORD	_dbTimeout;

	/// flag indicate if this worker is working for QA navigation
	bool	_isQA;

};

#endif // !defined(AFX_NAVWORKER_H__CCB0C786_E021_49B3_A0D5_83C71B91FE43__INCLUDED_)
