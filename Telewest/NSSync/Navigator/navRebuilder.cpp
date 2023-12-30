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
// Desc  : implementation of the navRebuilder class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navRebuilder.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 14    08-12-16 17:36 Ken.qian
// clean duty asset/bundle from NAV_Hierarchy at the end of rebuild.
// 
// 13    07-02-12 21:02 Bernie.zhao
// fixed bug when in telewest environment getQAFlag() meets error
// 
// 12    12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 11    06-05-23 16:04 Ken.qian
// 
// 10    5/16/06 8:32a Bernie.zhao
// fixed entrycount calc bug, added auto-detect of 'ns_folderupdate'
// parameter count
// 
// 9     06-04-20 16:18 Bernie.zhao
// moved exception catch from nsBuild to every single 'exec'
// 
// 8     06-01-12 21:11 Bernie.zhao
// added logic to support PM
// 
// 7     05-10-24 15:10 Bernie.zhao
// move to lap
// 
// 6     05-10-13 6:26 Bernie.zhao
// quick sp to telewest, with same version number as 1.1.2
// 
// 4     05-07-29 16:16 Bernie.zhao
// 
// 3     05-07-22 11:57 Bernie.zhao
// ver 1.1.1,
// 1. modified rebuild strategy
// 2. update view/viewfolders when update space
// 
// 2     05-04-22 17:28 Bernie.zhao
// added macro to enable NVARCHAR
// 
// 1     05-03-25 12:47 Bernie.zhao
// ===========================================================================

#include "navRebuilder.h"
#include "NavigationService.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

navRebuilder::navRebuilder(WQ_Entry& wqentry, CString connectStr): navWorker(wqentry, connectStr)
{

}

navRebuilder::~navRebuilder()
{
	if(_db.IsOpen())
		_db.Close();
	_callStack.clear();
}

void navRebuilder::free()
{
	delete this;
}

int navRebuilder::work()
{
	int retVal = NS_SUCCESS;
	CNAVTable record_set(&_db, _connDBStr);
	//////////////////////////////////////////////////////////////////////////
	// get all condition folders into a table
	if(record_set.IsOpen())
		record_set.Close();
	
	// modified by Bernie.Zhao 2005-7-20.
	// just list the root, and begin the rebuild from the root
	// this is because the entry-count of manual-created folder(no source) should be maintained
	CString selectStr = _T("SELECT Hierarchy_UID FROM") +  tableName(L"NAV_Hierarchy") + _T("WHERE Parent_HUID='0'");

	traceSQL(selectStr, __WFUNC__);
	record_set.Open(CRecordset::dynaset, selectStr, CRecordset::readOnly);

	while(!record_set.IsEOF()) {
		// fetch each folder, and rebuild it
		CStringX HUID;
		short nIndex=0;
		record_set.GetFieldValue(nIndex, HUID);
		CX2T atl_HUID(HUID);
		retVal |= purifyFolder(LPCTSTR(atl_HUID));
		record_set.MoveNext();
	}
	record_set.Close();
	
	// modified by Ken.Qian @ 2008-12-16
	// to remove the duty data if there is in rebuild
	
	glog(Log::L_DEBUG, L"[%s] Clean duty asset/bundle from NAV_Hierarchy if there are", __WQAID__);

  // delete duty asset
	CString execStr=_T("DELETE FROM NAV_HIERARCHY WHERE Entry_type = 6 AND Local_entry_UID NOT IN (SELECT LocalAssetUID FROM Asset)");
	
  if(!_db.IsOpen()) {
		_db.Open(NULL, FALSE, FALSE, _connDBStr);
	}

	try
	{
		traceSQL(execStr, __WFUNC__);
		_db.ExecuteSQL( execStr );
	}
	catch (CDBException* pDBexcep)
	{
		glog(Log::L_ERROR, L"[%s] navRebuilder::work() A database exception occurs, with error string: %s", __WQAID__, pDBexcep->m_strError);

		pDBexcep->Delete();

		retVal = NS_ERROR;
	}

  // delete duty bundle
	execStr=_T("DELETE FROM NAV_HIERARCHY WHERE Entry_type = 5 AND Local_entry_UID NOT IN (SELECT LocalFolderUID FROM Folder)");
	
  if(!_db.IsOpen()) {
		_db.Open(NULL, FALSE, FALSE, _connDBStr);
	}

	try
	{
		traceSQL(execStr, __WFUNC__);
		_db.ExecuteSQL( execStr );
	}
	catch (CDBException* pDBexcep)
	{
		glog(Log::L_ERROR, L"[%s] navRebuilder::work() A database exception occurs, with error string: %s", __WQAID__, pDBexcep->m_strError);

		pDBexcep->Delete();

		retVal = NS_ERROR;
	}
	
	return retVal;
}

int navRebuilder::purifyFolder(CString nav_HUID, CString levelChar /* = L"|" */)
{
	int retVal = NS_SUCCESS;

	// check if in stack?
	switch(isInStackOrDone(nav_HUID)) {
	case 0:
		break;
	case 1:
		return NS_SUCCESS;
		break;
	case -1:
		return NS_SUCCESS;
		break;
	}

	glog(Log::L_DEBUG, L"[%s] (%s) %s Begin rebuilding entry HUID = '%s'", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)levelChar,(LPCTSTR)nav_HUID);

	CNAVTable record_set(&_db, _connRCStr);
	//////////////////////////////////////////////////////////////////////////
	// deal with my Source first

	if(record_set.IsOpen())
		record_set.Close();
	CString selectStr = _T("SELECT Source_type,Source_HUID FROM")+ tableName(L"NAV_Folder_Condition") + _T("WHERE Hierarchy_UID='")+nav_HUID+_T("'");
	
	traceSQL(selectStr, __WFUNC__);	
	record_set.Open(CRecordset::dynaset, selectStr, CRecordset::readOnly);

	if(!record_set.IsBOF()) 
	{	
		// has source, recursive to the source folder first
		CStringX sourceHUID, sourceType;
		short nIndex;
		nIndex=0;
		record_set.GetFieldValue(nIndex,sourceType);
		nIndex=1;
		record_set.GetFieldValue(nIndex,sourceHUID);
		record_set.Close();
		
		// convert into TCHAR
		CX2T atl_sourceHUID(sourceHUID);
		CX2T atl_sourceType(sourceType);

		// source is NAV, update source first
		if(wcscmp(LPCTSTR(atl_sourceType),_T("1"))==0)
		{
			retVal |= purifyFolder(LPCTSTR(atl_sourceHUID), levelChar+L"-");
		}

		// update asset or schedule to me
		retVal |= folderUpdate(nav_HUID);
	}
	else 
	{
		// normal folder

		// update asset to me
		retVal |= folderUpdate(nav_HUID);

		record_set.Close();
	}
	
	//////////////////////////////////////////////////////////////////////////
	// deal with all children
	if(record_set.IsOpen())
		record_set.Close();
	selectStr = _T("SELECT Hierarchy_UID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE (Entry_type=1 OR Entry_type=2) AND Parent_HUID='")+nav_HUID+_T("'");

	traceSQL(selectStr, __WFUNC__);
	record_set.Open(CRecordset::dynaset, selectStr, CRecordset::readOnly);
	while(!record_set.IsEOF()) {
		CStringX child_HUID;
		short nIndex=0;
		record_set.GetFieldValue(nIndex,child_HUID);
		CX2T atl_child_HUID(child_HUID);
		retVal |= purifyFolder(LPCTSTR(atl_child_HUID), levelChar+L"-");
		record_set.MoveNext();
	}
	record_set.Close();

	// mark me as Done
	_callStack[nav_HUID]=TRUE;

	glog(Log::L_DEBUG, L"[%s] (%s) %s End rebuilding entry HUID = '%s'", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)levelChar,(LPCTSTR)nav_HUID);

	return retVal;
}

int navRebuilder::folderUpdate(CString nav_HUID)
{
	int retVal = NS_SUCCESS;
	CString execStr = L"";

	if(NavigationService::m_folderUpdateParamCount==1)	// 1 param version
	{
		execStr=L"EXEC ns_FolderUpdate '"+nav_HUID+L"'";
	}
	else if(NavigationService::m_folderUpdateParamCount==2)	// 2 params version
	{
		execStr=L"EXEC ns_FolderUpdate '"+nav_HUID+L"','"+ NavigationService::m_wszRegionID + L"'";
	}
	else if(_isQA)	// 3 params version, and for QANAV
	{
		// to support QANAV, we need to pass extra parameter to SP
		execStr= L"EXEC ns_FolderUpdate '"+nav_HUID+L"','"+ NavigationService::m_wszRegionID + L"',1";
	}
	else	// 3 params version, standard NAV
	{
		execStr= L"EXEC ns_FolderUpdate '"+nav_HUID+L"','"+ NavigationService::m_wszRegionID + L"',0";
	}

	try
	{
		if(!_db.IsOpen()) {
			_db.Open(NULL, FALSE, FALSE, _connDBStr);
		}

		traceSQL(execStr, __WFUNC__);
		_db.ExecuteSQL( execStr );
	}
	catch (CDBException* pDBexcep)
	{
		glog(Log::L_ERROR, L"[%s] (%s) navRebuilder::folderUpdate()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)pDBexcep->m_strError);
		pDBexcep->Delete();
		retVal = NS_ERROR;
	}

	haveTea();

	return retVal;
}

int navRebuilder::isInStackOrDone(CString nav_HUID)
{
	std::map<CString, bool>::const_iterator iter = _callStack.find(nav_HUID);
	if(iter==_callStack.end()) {
		// not in stack, add it
		_callStack.insert(std::pair<CString,bool>(nav_HUID, FALSE));
		return 0;
	}

	if(iter->second==TRUE) {
		// in stack, and is done
		return 1;
	}

	if(iter->second==FALSE) {
		// in stack, and not done, some recursive loop occurs
		return -1;
	}
	return 1;
}