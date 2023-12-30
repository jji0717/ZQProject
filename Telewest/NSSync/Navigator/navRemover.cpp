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
// Name  : navRebuilder.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : implementation of the navRemover class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navRemover.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 7     08-12-24 14:38 Ken.qian
// remove codes to get parentHUID since in deletion case, can not find in
// database anymore
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
// 3     06-04-20 16:18 Bernie.zhao
// moved exception catch from nsBuild to every single 'exec'
// 
// 1     05-03-25 12:47 Bernie.zhao
// ===========================================================================
#include ".\navremover.h"

navRemover::navRemover(WQ_Entry& wqentry, CString connectStr): navWorker(wqentry, connectStr)
{
}

navRemover::~navRemover(void)
{
	if(_db.IsOpen())
		_db.Close();
}

void navRemover::free()
{
	delete this;
}

int navRemover::work()
{
	CString entryUID=_wq_Entry.local_entry_UID;
	CString parentHUID = _wq_Entry.Parent_HUID;

	return deleteEntry(entryUID, parentHUID);
}

int navRemover::deleteEntry(const CString& Entry_UID, const CString& parentHUID)
{
	int retVal = NS_SUCCESS;
	CString execStr;
	CNAVTable record_set(&_db, _connRCStr);

	if(!_db.IsOpen()) {
		_db.Open(NULL, FALSE, FALSE, _connDBStr);
	}

	// Update BundleEntry Count and NAV_Hierarchy(Folder) EntryCount before deletion
	// 1. Update Bundle EntryCount if the deleted entry is asset and whose parent in Hierarchy is bundle
	if(DBSYNC_ENTRY_TYPE_ASSET == _wq_Entry.Entry_type)
	{
		// fetch the localEntryUID
		if(record_set.IsOpen())
			record_set.Close();

		CString selectStr;
		selectStr.Format(_T("SELECT LocalEntryUID From Hierarchy WHERE LocalHierarchyUID='%s' AND ( EXISTS(SELECT 1 FROM Folder WHERE LocalFolderUID=LocalEntryUID AND isPackage=1) )"),
			parentHUID);

		traceSQL(selectStr, __WFUNC__);
		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

		if(!record_set.IsEOF()) 
		{
			// fetch each, and recursive to it
			CStringX bundleLocalUID;
			short nIndex=0;
			record_set.GetFieldValue(nIndex, bundleLocalUID);
			CX2T atl_bundleUID(bundleLocalUID);

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Update Bundle EntryCount in NAV_Hierarchy, Bundle HUID= %s, Bundle LocalEntryUID=%s", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)Entry_UID, L"|", (LPCTSTR)parentHUID,(LPCTSTR)atl_bundleUID);

			retVal |= bundleCountUpdate(LPCTSTR(atl_bundleUID));
		}
		record_set.Close();
	}

	// 2. Update NAV_Hierarchy(Folder) EntryCount whatever it is asset or bundle
	try
	{
		if(record_set.IsOpen())
			record_set.Close();

		CString selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") + _T("WHERE Local_entry_UID='") +  Entry_UID + _T("'");

		traceSQL(selectStr, __WFUNC__);
		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

		while(!record_set.IsEOF()) 
		{
			// fetch each, and recursive to it
			CStringX navHUID;
			short nIndex=0;
			record_set.GetFieldValue(nIndex, navHUID);
			CX2T atl_navHUID(navHUID);

			execStr.Format(_T("Exec ns_UpdateEntryCount '%s'"), LPCTSTR(atl_navHUID));
			traceSQL(execStr, __WFUNC__);
			_db.ExecuteSQL( execStr );		

			record_set.MoveNext();
		}
		record_set.Close();
	}
	catch (CDBException* pDBexcep)
	{
		glog(Log::L_ERROR, L"[%s] (%s) navRemover::deleteEntry()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, pDBexcep->m_strError);

		pDBexcep->Delete();

		retVal = NS_ERROR;
	}	

	glog(Log::L_DEBUG, L"[%s] (%s) %s Begin deleting entry HUID = '%s'", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, L"|",(LPCTSTR)Entry_UID);

	//CString execStr=_T("UPDATE")+ tableName(L"NAV_Hierarchy") + _T("SET entry_count=entry_count-1	WHERE Hierarchy_UID IN (SELECT Parent_HUID FROM") + tableName(L"NAV_Hierarchy") + _T("WHERE Local_entry_UID='")+Entry_UID+_T("')")+_T("  DELETE FROM") + tableName(L"NAV_Hierarchy") +_T("WHERE Local_entry_UID='")+Entry_UID+_T("'");;
	execStr=_T("DELETE FROM") + tableName(L"NAV_Hierarchy") +_T("WHERE Local_entry_UID='")+Entry_UID+_T("'");;
	
	try
	{
		traceSQL(execStr, __WFUNC__);
		_db.ExecuteSQL( execStr );
	}
	catch (CDBException* pDBexcep)
	{
		glog(Log::L_ERROR, L"[%s] (%s) navRemover::deleteEntry()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, pDBexcep->m_strError);

		pDBexcep->Delete();

		retVal = NS_ERROR;
	}

	glog(Log::L_DEBUG, L"[%s] (%s) %s End deleting entry HUID = '%s'", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, L"|",(LPCTSTR)Entry_UID);	

	return retVal;
}