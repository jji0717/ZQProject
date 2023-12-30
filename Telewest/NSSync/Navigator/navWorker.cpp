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
// Name  : navWorker.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navWorker.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 8     08-12-23 20:33 Ken.qian
// add addtional codes if ParentHUID was null or empety
// 
// 7     08-12-23 12:08 Ken.qian
// add codes to calculate entrycount and handle the manual mantained
// asset/bundle
// 
// 6     06-12-15 9:55 Ken.qian
// 
// 5     06-12-14 18:16 Ken.qian
// 
// 4     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 3     05-10-13 6:26 Bernie.zhao
// quick sp to telewest, with same version number as 1.1.2
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================


#include "stdafx.h"
#include "navWorker.h"
#include "NavigationService.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const WCHAR*	WQSourceTypeToString(WQ_Entry& entry)
{
	switch(entry.Source_type) {
	case 0:
		return L"0 (DBSync Service)";
		break;
	case 1:
		return L"1 (Navigation Engine)";
		break;
	case 2:
		return L"2 (Program Manager)";
		break;
	default:
		return L"Unknown";
		break;
	}
}

const WCHAR*	WQEntryTypeToString(WQ_Entry& entry)
{
	if(entry.Source_type==0)
	{
		switch(entry.Entry_type) {
		case 1:
			return L"1 (Asset)";
			break;
		case 3:
			return L"3 (Folder)";
			break;
		case 11:
			return L"11 (Bundle)";
			break;
		default:
			return L"Unknown";
			break;
		}
	}
	else if(entry.Source_type==1)
	{
		switch(entry.Entry_type) {
		case 1:
			return L"1 (Folder)";
			break;
		case 2:
			return L"1 (PO)";
			break;
		case 5:
			return L"5 (Bundle)";
			break;
		case 6:
			return L"6 (Asset)";
			break;
		case 7:
			return L"7 (Symbol Link)";
			break;
		default:
			return L"Unknown";
			break;
		}
	}
	else if(entry.Source_type==2)
	{
		switch(entry.Entry_type) {
		case 1:
			return L"1 (Folder)";
			break;
		case 2:
			return L"2 (Program)";
			break;
		default:
			return L"Unknown";
			break;
		}
	}
	else
	{
		return L"Unknown";
	}
}

const WCHAR*	WQOperationTypeToString(WQ_Entry& entry)
{
	if(entry.Operation_type==99)
		return L"99 (Rebuild)";
	if(entry.Operation_type==98)
		return L"98 (QA Rebuild)";

	if(entry.Source_type==0)
	{
		switch(entry.Operation_type) {
		case 1:
			return L"1 (Add)";
			break;
		case 2:
			return L"2 (Delete)";
			break;
		case 3:
			return L"3 (Link)";
			break;
		case 4:
			return L"4 (Unlink)";
			break;
		case 5:
			return L"5 (Rename)";
			break;
		case 6:
			return L"6 (Update)";
			break;
		default:
			return L"Unknown";
			break;
		}
	}
	else if(entry.Source_type==1)
	{
		switch(entry.Operation_type) {
		case 1:
			return L"1 (Add or Link)";
			break;
		case 2:
			return L"2 (Update)";
			break;
		case 3:
			return L"3 (Delete or Unlink)";
			break;
		case 4:
			return L"4 (Folder Condition Update)";
			break;
		case 5:
			return L"5 (Folder Link Target Update)";
			break;
		default:
			return L"Unknown";
			break;
		}
	}
	else if(entry.Source_type==2)
	{
		switch(entry.Operation_type) {
		case 1:
			return L"1 (Add)";
			break;
		case 2:
			return L"2 (Delete)";
			break;
		default:
			return L"Unknown";
			break;
		}
	}
	else
	{
		return L"Unknown";
	}
}

const WCHAR*	WQStatusToString(WQ_Entry& entry)
{
	switch(entry.Status) {
	case wq_waiting:
		return L"Waiting";
		break;
	case wq_building:
		return L"Building";
		break;
	case wq_completed:
		return L"Completed";
		break;
	case wq_skipped:
		return L"Skipped";
		break;
	case wq_failed:
		return L"Failed";
		break;
	default:
		return L"Unknown";
		break;
	}
}

CTime NullCTime()
{
	CTime t(0);
	return t;
}

//////////////////////////////////////////////////////////////////////
// navWorker Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

navWorker::navWorker(WQ_Entry& wqentry, CString connectStr)
{
	_wq_Entry = wqentry;

	if(!connectStr.IsEmpty())
		_connRCStr = connectStr.Right(connectStr.GetLength()-wcslen(L"ODBC;"));
	else
		_connRCStr = _T("DSN=LAM;UID=am;PWD=am;DATABASE=am");

	_connDBStr = _T("ODBC;")+_connRCStr;

	//_db.SetQueryTimeout(DEFAULT_SQL_TIMEOUT);
	_db.OpenEx(_connRCStr, CDatabase::noOdbcDialog);

	_dbTimeout = DEFAULT_SQL_TIMEOUT;
	_isQA = false;
}

navWorker::~navWorker()
{
	if(_db.IsOpen())
		_db.Close();
}

int navWorker::work()
{
	return NS_SUCCESS;
}

void navWorker::free()
{
	delete (navWorker*)this;
}

void navWorker::haveTea()
{
	::Sleep(NavigationService::m_dwTeaTime);
}

void navWorker::traceSQL(CString statement, CString funcName)
{
	if(NavigationService::m_dwSQLTraceEnabled!=0)
		glog(Log::L_DEBUG, L"[%s] (%s)     << SQL << \"%s\"\t%s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)statement, (LPCTSTR)funcName);
}

int navWorker::bundleCountUpdate(CString bundle_LUID)
{

	CString execStr;
	execStr.Format(L"UPDATE %s SET entry_count = \
( \
SELECT COUNT(1) FROM Hierarchy \
WHERE LocalParentHUID=(SELECT TOP 1 LocalHierarchyUID FROM Hierarchy WHERE LocalEntryUID='%s') \
AND EntryType=1 \
AND (EXISTS (SELECT 1 FROM Asset WHERE LocalAssetUID=LocalEntryUID AND State=1) ) \
) \
WHERE Local_entry_UID='%s' AND Entry_type=5", 
					tableName(L"NAV_Hierarchy"), bundle_LUID, bundle_LUID);

	try
	{
		if(!_db.IsOpen()) 
			_db.Open(NULL, FALSE, FALSE, _connDBStr);

		traceSQL(execStr, __WFUNC__);
		_db.ExecuteSQL( execStr );
	}
	catch (CDBException* pDBexcep)
	{
		glog(Log::L_ERROR, L"[%s] (%s) navWorker::bundleCountUpdate()  A database exception occurs, with error string: %s",  __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)pDBexcep->m_strError);

		pDBexcep->Delete();

		return NS_ERROR;
	}
	return NS_SUCCESS;
}
