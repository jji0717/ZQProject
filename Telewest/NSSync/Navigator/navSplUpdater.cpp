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
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navSplUpdater.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 31    08-12-31 11:09 Ken.qian
// change MAX_SP_PARAMETER_LENGTH to 3200
// 
// 30    08-12-30 13:28 Ken.qian
// filter for duplicated source HUID
// 
// 29    08-12-30 11:01 Ken.qian
// add detail logs, and fix a problem of finding ancestor folder in NAV
// 
// 28    08-12-29 19:28 Build
// check in by Ken Qian for performance tuning
// 
// 27    08-12-29 11:06 Ken.qian
// 
// 26    08-12-26 18:57 Ken.qian
// add codes to get asset/bundle NAV parent HUID befefore upgrading
// 
// 25    08-12-24 14:40 Ken.qian
// 
// 23    08-12-23 12:08 Ken.qian
// add codes to calculate entrycount and handle the manual mantained
// asset/bundle
// 
// 22    08-12-11 13:45 Li.huang
// add codes to avoid the duplicated source HUID to improve the
// performance
// 
// 21    08-12-10 19:30 Li.huang
// add codes to remove duplicated target HUID
// 
// 20    08-09-26 11:08 Li.huang
// 
// 19    08-09-19 15:18 Li.huang
// 
// 18    08-09-19 14:33 Li.huang
// 
// 17    08-09-19 14:03 Li.huang
// 
// 16    08-09-19 12:26 Ken.qian
// add EntryType in the function
// 
// 15    08-09-19 12:03 Li.huang
// 
// 14    08-09-17 21:42 Ken.qian
// fix the dead loop of bad condition
// 
// 13    08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 12    07-02-12 21:02 Bernie.zhao
// fixed bug when in telewest environment getQAFlag() meets error
// 
// 11    12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 10    06-05-23 16:04 Ken.qian
// 
// 9     5/16/06 8:34a Bernie.zhao
// fixed entrycount calc bug, added auto-detect of 'ns_folderupdate'
// parameter count
// 
// 8     06-04-20 16:18 Bernie.zhao
// moved exception catch from nsBuild to every single 'exec'
// 
// 7     06-01-12 21:11 Bernie.zhao
// added logic to support PM
// 
// 6     05-10-14 14:15 Bernie.zhao
// quick sp
// 
// 5     05-10-13 6:26 Bernie.zhao
// quick sp to telewest, with same version number as 1.1.2
// 
// 3     05-07-22 11:57 Bernie.zhao
// ver 1.1.1,
// 1. modified rebuild strategy
// 2. update view/viewfolders when update space
// 
// 2     05-04-22 17:28 Bernie.zhao
// added macro to enable NVARCHAR
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================

#include "navsplupdater.h"
#include "NavigationService.h"

#define MAX_NVARCHAR_LENGTH        4000
#define MAX_SP_PARAMETER_LENGTH    MAX_NVARCHAR_LENGTH - 800

navSplUpdater::navSplUpdater(WQ_Entry& wqentry, CString connectStr): navWorker(wqentry, connectStr)
{
}

navSplUpdater::~navSplUpdater()
{
	if(_db.IsOpen())
		_db.Close();
	_callStack.clear();
}

void navSplUpdater::free()
{
	delete this;
}

int navSplUpdater::work()
{
	_callStack.clear();
	
	CString SampleUID;
	// determine sample space node
	if(_wq_Entry.Source_type==0)	// DBSync
	{
		if(SP_FOLDER_UPDATE_TYPE_TARGET_SYNC == NavigationService::m_folderUpdateSPType)
		{
			// Add by KenQ to entry the new handling logic
			if(DBSYNC_ENTRY_TYPE_ASSET == _wq_Entry.Entry_type && WQ_DBSYNC_UPDATE == _wq_Entry.Operation_type)
			{
				// asset update
				// set sampleUID to "", means it is object updating, coz same asset can have multiple parent. 
				SampleUID = L"";
			}
			else
			{
				// folder(package)/asset link/unlink, folder(package) update
				// DBSync did not specify the parent HUID for bundle
				SampleUID = _wq_Entry.Parent_HUID;
				if(L"(null)" == SampleUID)
				{
					SampleUID=L"";
				}
			}
			// invoke the new function
			CStringArray affectedHUIDs;
			updateAffected(_wq_Entry.local_entry_UID, _wq_Entry.Entry_type, _wq_Entry.Operation_type, SampleUID, FOLDER_SRC_TYPE_LAM, affectedHUIDs);
		}
		else
		{
			if(_wq_Entry.Operation_type==3 || _wq_Entry.Operation_type==4) 
			{	// LINK or UNLINK
			  SampleUID = _wq_Entry.Parent_HUID;
			}
			else 
			{
			}
			return updateAffected(SampleUID, _wq_Entry.Source_type);
		}
	}
	else if(_wq_Entry.Source_type==1)	// Navigation
	{
		if(_wq_Entry.Operation_type==1 || _wq_Entry.Operation_type==3) 
		{	// add/link or delete/unlink
			if(_wq_Entry.Entry_type==5) 
			{	// bundle
				SampleUID = _wq_Entry.local_entry_UID;
			}
			else if(_wq_Entry.Entry_type==6) 
			{	// asset
				SampleUID = _wq_Entry.Parent_HUID;
			}
		}
		else if(_wq_Entry.Operation_type==4 || _wq_Entry.Operation_type==5) 
		{	// pre-search condition or target update
			SampleUID = _wq_Entry.local_entry_UID;
		}
		return updateAffected(SampleUID, _wq_Entry.Source_type);
	}
	else if(_wq_Entry.Source_type==2)	// Program Manager
	{
		if(_wq_Entry.Operation_type==1 || _wq_Entry.Operation_type==2 || _wq_Entry.Operation_type==3)
		{	// add/delete/update schedule
			SampleUID = _wq_Entry.Parent_HUID;	// actually this is the channel id of this schedule
		}
		else
		{
		}
		return updateAffected(SampleUID, _wq_Entry.Source_type);
	}
	return  NS_SUCCESS;
}


int navSplUpdater::updateAffected(CString nav_HUID, int nav_sourceType, CString levelChar/* =L"|" */)
{
	int retVal = NS_SUCCESS;

	// if is root, stop recursive
	if(nav_HUID==_T("H0") || nav_HUID==_T("0"))
		return NS_SUCCESS;

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

	glog(Log::L_DEBUG, L"[%s] (%s) %s Begin updating entry HUID = '%s'", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)levelChar,(LPCTSTR)nav_HUID);
	
	CDatabase tmpDb;
	tmpDb.OpenEx(_connRCStr, CDatabase::noOdbcDialog);
	CNAVTable record_set(&tmpDb, _connRCStr);

	wchar_t typebuff[8];
	_itow(nav_sourceType, typebuff, 10);
	CString typeStr = typebuff;
	CString selectStr;
	//////////////////////////////////////////////////////////////////////////
	// deal with myself first
	
//	if(nav_sourceType==1) {	// this folder is in NAV
//		if(record_set.IsOpen())
//			record_set.Close();
//		selectStr = _T("SELECT Source_type,Source_HUID FROM NAV_Folder_Condition WHERE Hierarchy_UID='")+nav_HUID+_T("'");
//#ifdef _DEBUG
//		wprintf(L"[SQL] navSplUpdater::updateAffected() %s\n",selectStr);
//#endif
//		glog(Log::L_DEBUG, L"navSplUpdater::updateAffected()");
//		glog(Log::L_DEBUG, L"[SQL] %s", selectStr);
//		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);
//
//		if(!record_set.IsBOF()) {	
//			// has source, recursive to the source folder first
//			CStringX sourceHUID, sourceType;
//			short nIndex;
//			nIndex=0;
//			record_set.GetFieldValue(nIndex,sourceType);
//			nIndex=1;
//			record_set.GetFieldValue(nIndex,sourceHUID);
//			record_set.Close();
//			// convert into TCHAR
//			CX2T atl_sourceHUID(sourceHUID);
//			CX2T atl_sourceType(sourceType);
//			// source is NAV, update source first
//			if(wcscmp(LPCTSTR(atl_sourceType),_T("1"))==0)
//				updateAffected(LPCTSTR(atl_sourceHUID),1);
//
//			// update asset to me
//			folderUpdate(nav_HUID);
//		}
//		else {
//			record_set.Close();
//		}
//	}
	
	if(nav_sourceType==1)
	{
		// update asset to me
		retVal |= folderUpdate(nav_HUID);
	}
	
    //////////////////////////////////////////////////////////////////////////
	// if i am a bundle from DBSync, must be assets under bundle number change
	if(_wq_Entry.Source_type==0 || _wq_Entry.Entry_type==1) 
	{
		if(record_set.IsOpen())
			record_set.Close();

		selectStr = _T("SELECT LocalEntryUID FROM Hierarchy WHERE LocalHierarchyUID='")+nav_HUID
			+_T("' AND EntryType=3 AND EXISTS (SELECT 1 FROM Folder WHERE Hierarchy.LocalEntryUID=Folder.LocalFolderUID AND Folder.IsPackage=1)");
		
		traceSQL(selectStr, __WFUNC__);
		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);
		if(!record_set.IsBOF()) 
		{	// is bundle
			CStringX FolderUID;
			int addcount=0;
			short nIndex=0;
			record_set.GetFieldValue(nIndex, FolderUID);
			CX2T atl_FolderUID(FolderUID);
			if(_wq_Entry.Operation_type==3)  // Link
				addcount = 1;
			else if(_wq_Entry.Operation_type==4)  // UnLink
				addcount = -1;

			// update all bundles' count and return
			record_set.Close();
			retVal |= bundleCountUpdate(LPCTSTR(atl_FolderUID));
			return NS_SUCCESS;
		}
		else 
		{	// not bundle
			record_set.Close();
		}
	}
	

	//////////////////////////////////////////////////////////////////////////
	// deal with any folder that has source pointed to me
	if(record_set.IsOpen())
		record_set.Close();
	
	selectStr = _T("SELECT Hierarchy_UID FROM")+ tableName(L"NAV_Folder_Condition") +_T("WHERE Source_HUID='")+nav_HUID+_T("' AND Source_type=")+typeStr;
	
	traceSQL(selectStr, __WFUNC__);
	record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

	while(!record_set.IsEOF()) 
	{
		// fetch each, and recursive to it
		CStringX HUID;
		short nIndex=0;
		record_set.GetFieldValue(nIndex, HUID);
		CX2T atl_HUID(HUID);
		retVal |= updateAffected(LPCTSTR(atl_HUID),1, levelChar+L"-");
		record_set.MoveNext();
	}

	// mark me as Done
	_callStack[nav_HUID]=TRUE;

	record_set.Close();

	//////////////////////////////////////////////////////////////////////////
	// deal with my parent
	if(nav_sourceType==1) 
	{ // this folder is in NAV
		if(record_set.IsOpen())
			record_set.Close();
		selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE Hierarchy_UID='")+nav_HUID+_T("'");
		
		traceSQL(selectStr, __WFUNC__);
		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);
		
		if(!record_set.IsBOF()) 
		{
			CStringX parent_HUID;
			short nIndex=0;
			record_set.GetFieldValue(nIndex,parent_HUID);
			CX2T atl_parent_HUID(parent_HUID);
			record_set.Close();

			retVal |= updateAffected(LPCTSTR(atl_parent_HUID),1, levelChar+L"-");
		}
		else 
		{
			record_set.Close();
		}
	}
	else if(nav_sourceType==0)
	{ // this folder is in LAM
		if(record_set.IsOpen())
			record_set.Close();
		selectStr = _T("SELECT LocalParentHUID FROM Hierarchy WHERE LocalHierarchyUID='")+nav_HUID+_T("'");
		
		traceSQL(selectStr, __WFUNC__);
		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);
		
		if(!record_set.IsBOF()) 
		{
			CStringX parent_HUID;
			short nIndex=0;
			record_set.GetFieldValue(nIndex,parent_HUID);
			CX2T atl_parent_HUID(parent_HUID);
			record_set.Close();

			retVal |= updateAffected(LPCTSTR(atl_parent_HUID),0,levelChar+L"-");
		}
		else 
		{
			record_set.Close();
		}
	}
	else if(nav_sourceType==2)
	{	// this is not a folder, this is a channel-id in program manager
		// right now seems we are all good with PM, since channel-id does not have a tree style structure
	}

	tmpDb.Close();

	glog(Log::L_DEBUG, L"[%s] (%s) %s End updating entry HUID = '%s'", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)levelChar,(LPCTSTR)nav_HUID);

	return retVal;
}

int navSplUpdater::folderUpdate(CString nav_HUID)
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
		glog(Log::L_ERROR, L"[%s] (%s) navSplUpdater::folderUpdate()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)pDBexcep->m_strError);

		pDBexcep->Delete();

		retVal = NS_ERROR;
	}

	haveTea();

	return retVal;
}

int navSplUpdater::isInStackOrDone(CString nav_HUID)
{
	std::map<CString, bool>::const_iterator iter = _callStack.find(nav_HUID);
	
	if(iter==_callStack.end()) {
		// not in stack, add it
		_callStack.insert(std::pair<CString,bool>(nav_HUID, FALSE));
		return 0;
	}

	if(iter->second==TRUE) 
	{
		// in stack, and is done
		return 1;
	}

	if(iter->second==FALSE) 
	{
		// in stack, and not done, some recursive loop occurs
		return -1;
	}

	return 1;
}


int navSplUpdater::getTargetNAVFolders(int sourceType, const CString& localEntryUID, const CString& HUID, CStringArray& targetNavHUIDs, CStringArray& parentHUIDs, CString levelChar)
{
	int retVal = NS_SUCCESS;

	CString selectStr;
	CDatabase tmpDb;
	tmpDb.OpenEx(_connRCStr, CDatabase::noOdbcDialog);
	CNAVTable record_set(&tmpDb, _connRCStr);
	CNAVTable record_set_manual(&tmpDb, _connRCStr);

	CString key; 
	std::map<CString, CString> srcHUIDs;
	CString tmpSrcHUID;

	CString srcLog, tagLog, tmpLog;
	int i = 0;
	/////////////////////////////////////////////
	// find all source LAM/NAV HUID, and save to parentHUIDs
	if(FOLDER_SRC_TYPE_LAM == sourceType)
	{	
		if(L"" == HUID)
		{	
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find entry direct parent HUIDs in LAM ", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);


			// the invoker set nav_HUID to empty if it is an asset update
			// if is an asset update, all its parent Hierarchy target must be synced. (One Asset may link in different folders)
			if(record_set.IsOpen())
				record_set.Close();

			selectStr = _T("SELECT LocalParentHUID FROM Hierarchy WHERE LocalEntryUID='")+localEntryUID+_T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			while(!record_set.IsEOF()) 
			{
				// fetch each, and recursive to it
				CStringX HUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex, HUID);
				CX2T atl_HUID(HUID);

				parentHUIDs.Add((LPCTSTR)atl_HUID);

				record_set.MoveNext();
			}
			record_set.Close();
		} // end of if("" == nav_HUID)
		else
		{
			parentHUIDs.Add(HUID);
		}

		CString lowestHUID;
		tmpLog = L"";
		for(i=0; i<parentHUIDs.GetSize(); i++)
		{
			tmpLog += parentHUIDs[i] + L" ";
		}

		glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Entry direct parent HUID in LAM are: %s ", 
			__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)tmpLog);

		// get Asset/Bundle parents HUID
		for(i=0; i<parentHUIDs.GetSize(); i++)
		{
			tmpLog = L"";
			tmpSrcHUID = parentHUIDs[i];
			lowestHUID = tmpSrcHUID;

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s List all ancestor folder in LAM for HUID=%s ", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, lowestHUID);

			// add direct parent HUID firstly
			key.Format(L"%s_%d", (LPCTSTR)tmpSrcHUID, FOLDER_SRC_TYPE_LAM);
			srcHUIDs[key] = tmpSrcHUID;

			// find all all ancestor HUID
			while(tmpSrcHUID != "H0" && tmpSrcHUID != "0" && tmpSrcHUID != "" )
			{
				if(record_set.IsOpen())
					record_set.Close();

				selectStr = _T("SELECT LocalParentHUID FROM Hierarchy WHERE LocalHierarchyUID='")+tmpSrcHUID+_T("'");

				traceSQL(selectStr, __WFUNC__);
				record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

				if(!record_set.IsBOF()) 
				{
					CStringX parent_HUID;
					short nIndex=0;
					record_set.GetFieldValue(nIndex,parent_HUID);
					CX2T atl_parent_HUID(parent_HUID);
					record_set.Close();

					tmpSrcHUID = atl_parent_HUID;

					// save the source HUID to map
					key.Format(L"%s_%d", (LPCTSTR)tmpSrcHUID, FOLDER_SRC_TYPE_LAM);

					if(srcHUIDs.find(key) != srcHUIDs.end())
					{
						glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s In LAM HUID %s Parent HUID %s has been found previously, skip all its ancestor folder", 
							__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, lowestHUID, tmpSrcHUID);
						break;
					}

					tmpLog += tmpSrcHUID + L" ";

					srcHUIDs[key] = tmpSrcHUID;
				}
				else
				{
					tmpSrcHUID = "";
					record_set.Close();
				}
			} // end of while(tmpSrcHUID != ...)

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Folder HUID=%s has following parent HUIDs: %s", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, lowestHUID, tmpLog);

		} // end of for(int i=0; i<parentHUIDs.GetSize(); i++)

		// figure out all parent folder in NAV_Hierarchy if I am a manual maintained asset/bundle
		glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find all NAV folder that had source of this entry", 
			__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

		if(record_set_manual.IsOpen())
			record_set_manual.Close();

		selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE Local_entry_UID='")+localEntryUID+_T("' AND Is_manual=1");

		traceSQL(selectStr, __WFUNC__);
		record_set_manual.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

		while(!record_set_manual.IsEOF()) 
		{
			tmpLog = L"";

			// fetch each, and recursive to it
			CStringX HUID;
			short nIndex=0;
			record_set_manual.GetFieldValue(nIndex, HUID);
			CX2T atl_HUID(HUID);

			tmpSrcHUID = atl_HUID;
			lowestHUID = tmpSrcHUID;

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s This entry is manually maintained in NAV HUID %s, find other NAV folder had source on this NAV folder", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, lowestHUID);

			// add the direct parent HUID first
			key.Format(L"%s_%d", (LPCTSTR)tmpSrcHUID, FOLDER_SRC_TYPE_NAV);
			srcHUIDs[key] = tmpSrcHUID;

			while(tmpSrcHUID != "H0" && tmpSrcHUID != "0" && tmpSrcHUID != "" )
			{
				if(record_set.IsOpen())
					record_set.Close();
				selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE Hierarchy_UID='")+tmpSrcHUID+_T("'");

				traceSQL(selectStr, __WFUNC__);
				record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

				if(!record_set.IsBOF()) 
				{
					CStringX parent_HUID;
					short nIndex=0;
					record_set.GetFieldValue(nIndex,parent_HUID);
					CX2T atl_parent_HUID(parent_HUID);

					tmpSrcHUID = atl_parent_HUID;

					tmpLog += tmpSrcHUID + L" ";
					// save the source HUID to map
					key.Format(L"%s_%d", (LPCTSTR)tmpSrcHUID, FOLDER_SRC_TYPE_NAV);

					if(srcHUIDs.find(key) != srcHUIDs.end())
					{
						glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s In NAV HUID %s Parent HUID %s has been found previously, skip all its ancestor folder", 
							__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, lowestHUID, tmpSrcHUID);
						break;
					}

					srcHUIDs[key] = tmpSrcHUID;
				}
				else
				{
					tmpSrcHUID = "";
					record_set.Close();
				}
			}
			record_set_manual.MoveNext();

			if(L"" == tmpLog )
			{
				glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s There is no other NAV folder had source on %s", 
					__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, lowestHUID);
			}
			else
			{
				glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s NAV HUID %s has following NAV folder having source on it: %s", 
					__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, lowestHUID, tmpLog);
			}
		}

	} // end of if(0 == nav_sourceType)
	else if(FOLDER_SRC_TYPE_NAV == sourceType)
	{
		// if sourceType is Navigation, get Navigation HUID all parent HUID

		// add asset direct parent HUID
		key.Format(L"%s_%d", (LPCTSTR)HUID, FOLDER_SRC_TYPE_NAV);
		srcHUIDs[key] = HUID;
		tmpLog = L"";

		tmpSrcHUID = HUID;

		glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find ancestor folders for NAV HUID %s", 
			__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, HUID);

		while(tmpSrcHUID != L"H0" && tmpSrcHUID != L"0" && tmpSrcHUID != L"")
		{
			if(record_set.IsOpen())
				record_set.Close();
			selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE Hierarchy_UID='")+tmpSrcHUID+_T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			if(!record_set.IsBOF()) 
			{
				CStringX parent_HUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex,parent_HUID);
				CX2T atl_parent_HUID(parent_HUID);

				tmpSrcHUID = atl_parent_HUID;

				// save the source HUID to map
				key.Format(L"%s_%d", (LPCTSTR)tmpSrcHUID, FOLDER_SRC_TYPE_NAV);

				if(srcHUIDs.find(key) != srcHUIDs.end())
				{
					glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s In NAV HUID %s Parent HUID %s has been found previously, skip all its ancestor folder", 
						__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, HUID, tmpSrcHUID);
					break;
				}

				tmpLog += tmpSrcHUID + L" ";
				srcHUIDs[key] = tmpSrcHUID;
			}
			else
			{
				tmpSrcHUID = "";
				record_set.Close();
			}
		}
		if(L"" == tmpLog)
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s NAV HUID %s does not have ancestor folder", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, HUID);
		}
		else
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s NAV HUID %s has following ancestor HUIDs %s", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, HUID, tmpLog);
		}
	}
	////////////////////////////////////////////////////////////
	// find all the target NAV HUID

	glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find all target NAV folders in NAV_Folder_Condition which had source on this entry", 
		__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

	int nTargetCount = 0;
	CString typeStr;
	CString srcHUID;
	CString targetHUID;
	std::map<CString, CString>::iterator it = srcHUIDs.begin();
	for(; it!=srcHUIDs.end(); it++)
	{
		CString key = it->first;
		srcHUID = it->second;

		int slashPos = key.ReverseFind(L'_');
		typeStr = key.Right(key.GetLength()-slashPos-1);

		srcLog = srcLog + _T("[SourceHUID=") + srcHUID + _T(" SourceType=")+typeStr+_T("] ");

		// deal with any folder that has source pointed to me
		if(record_set.IsOpen())
			record_set.Close();

		selectStr = _T("SELECT Hierarchy_UID FROM")+ tableName(L"NAV_Folder_Condition") +_T("WHERE Source_HUID='")+srcHUID+_T("' AND Source_type=")+typeStr;

		traceSQL(selectStr, __WFUNC__);
		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

		while(!record_set.IsEOF()) 
		{
			// fetch each, and recursive to it
			CStringX HUID;
			short nIndex=0;
			record_set.GetFieldValue(nIndex, HUID);
			CX2T atl_HUID(HUID);

			// check whether this target HUID has been already processed
			switch(isInStackOrDone(LPCTSTR(atl_HUID))) 
			{
			case 0:   // not in stack
				targetNavHUIDs.Add(LPCTSTR(atl_HUID));
				tagLog = tagLog + " " + LPCTSTR(atl_HUID);
				nTargetCount++;
				break;
			case 1:   // in the stack, and already processed
			case -1:  // in the stack, but not processed
				break;
			}

			record_set.MoveNext();
		}
		record_set.Close();
	}

	////////////////////////////////////////////////////////////
	// build the target folder in NAV_Hierarch
	glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Source HUIDs which include this entry are: %s", 
		__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)srcLog);

	if(0 == nTargetCount)
	{
		glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s No un-processed target HUID found which have source on above folders", 
			__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

	}
	else 
	{

		glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Target HUIDs which have source on above source folders are: %s", 
			__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)tagLog);
	}

	return nTargetCount;
}

int navSplUpdater::updateAffected(CString localEntryUID, int entryType, int operationType, CString HUID, int nav_sourceType, CStringArray& affectedHUIDs, CString levelChar/* =L */)
{
	int retVal = NS_SUCCESS;

	CString selectStr;
	CDatabase tmpDb;
	tmpDb.OpenEx(_connRCStr, CDatabase::noOdbcDialog);
	CNAVTable record_set(&tmpDb, _connRCStr);
	CNAVTable record_set_manual(&tmpDb, _connRCStr);

	CStringArray parentHUIDs;
	CStringArray newAffectedHUIDs;
	CStringArray targetHUIDs;
	std::map<CString, CString> entryNavParentHUIDs;

	CString srcLog, tagLog;
	int i = 0;
	/////////////////////////////////////////////
	// find all source LAM/NAV HUID, and save to parentHUIDs
	if(FOLDER_SRC_TYPE_LAM == nav_sourceType)
	{	
		//
		// this entry only execute at the first, it does not care about affectedHUIDs parameters.
		// For all recursive calls, they goes to else logic. 
		//

		// get asset/bundle parents from NAV_Hierarchy before updating for later entry count, coz we have to remember them, 
		// otherwise once the asset/bundle was removed from NAV folder, we have no idea about it after nav building
		try
		{
			CString tmpLog;
			CString execStr;

			if(record_set.IsOpen())
				record_set.Close();

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find all NAV folders in Nav_Hierarchy which include this entry before navigation building", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

			selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") + _T("WHERE Local_entry_UID='") +  localEntryUID + _T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			while(!record_set.IsEOF()) 
			{
				// fetch each, and recursive to it
				CStringX navHUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex, navHUID);
				CX2T atl_navHUID(navHUID);

				tmpLog += CString(atl_navHUID) + L" ";

				entryNavParentHUIDs[LPCTSTR(atl_navHUID)]=LPCTSTR(atl_navHUID);

				record_set.MoveNext();
			}
			record_set.Close();

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s There are following NAV folders which include this entry before navigation building: %s", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, tmpLog);

		}
		catch (CDBException* pDBexcep)
		{
			glog(Log::L_ERROR, L"[%s] (%s) navRemover::deleteEntry()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, pDBexcep->m_strError);

			pDBexcep->Delete();

			retVal = NS_ERROR;
		}
		////////////////////////////////////////////////////////////
		// find all the target NAV HUID that had source to the local HUID
		getTargetNAVFolders(FOLDER_SRC_TYPE_LAM, localEntryUID, HUID, targetHUIDs, parentHUIDs, levelChar);
		if(targetHUIDs.GetSize() > 0)
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s do target NAV folders build", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

			retVal |= targetUpdate(localEntryUID, entryType, operationType, targetHUIDs, newAffectedHUIDs);
		}
	} // end of if(0 == nav_sourceType)
	else if(FOLDER_SRC_TYPE_NAV == nav_sourceType)
	{
		//
		// this entry is for the recursive call, in this logic, it don't care about parameters: HUID and parentHUIDs
		//
		for(i=0; i<affectedHUIDs.GetSize(); i++)
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s get all un-processed NAV folders for affected HUID %s", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)affectedHUIDs[i]);

			getTargetNAVFolders(FOLDER_SRC_TYPE_NAV, localEntryUID, affectedHUIDs[i], targetHUIDs, parentHUIDs, levelChar);
		}

		if(targetHUIDs.GetSize() > 0)
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s do target NAV folders build", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

			retVal |= targetUpdate(localEntryUID, entryType, operationType, targetHUIDs, newAffectedHUIDs);
		}
		else
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s did not find any further un-processed NAV folders need to be built, reach the end of recursive", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);
		}
	}

	// mark target HUIDs as processed
	for(i=0; i<targetHUIDs.GetSize(); i++)
	{
		// mark me as Done
		_callStack[targetHUIDs[i]]=TRUE;
	}

	if(targetHUIDs.GetSize() > 0)
	{
		// log the affected HUIDs
		if(newAffectedHUIDs.GetSize() > 0) 
		{
			tagLog= "";
			for(i=0; i<newAffectedHUIDs.GetSize(); i++)
			{
				tagLog = tagLog + " " + newAffectedHUIDs[i];
			}

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s There are %d affected HUIDs found, they are: %s", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, newAffectedHUIDs.GetSize(), (LPCTSTR)tagLog);

			updateAffected(localEntryUID, entryType, operationType, "", FOLDER_SRC_TYPE_NAV, newAffectedHUIDs, levelChar+L"-");
		}
		else
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s No affected HUIDs after building, reach the end of recursive", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);
		}
	}

	// at the end of routine, update entry count
	if(FOLDER_SRC_TYPE_LAM == nav_sourceType)
	{
		//
		// this entry only execute at the first, and parentHUIDs are only used here to calculate bundle entrycount
		//

		// 1. Update bundle entrycount in NAV_Hierarchy
		if(DBSYNC_ENTRY_TYPE_ASSET == _wq_Entry.Entry_type)
		{
			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Asset has %d parent folders, re-calculate bundle EntryCount for each of them if it is bundle", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, parentHUIDs.GetSize());

			for(i=0; i<parentHUIDs.GetSize(); i++)
			{
				glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Re-calculate bundle EntryCount for asset parent HUID %s if it is bundle", 
					__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, parentHUIDs[i]);

				// fetch the localEntryUID
				if(record_set.IsOpen())
					record_set.Close();

				selectStr.Format(_T("SELECT LocalEntryUID From Hierarchy WHERE LocalHierarchyUID='%s' AND ( EXISTS(SELECT 1 FROM Folder WHERE LocalFolderUID=LocalEntryUID AND isPackage=1) )"),
					parentHUIDs[i]);

				traceSQL(selectStr, __WFUNC__);
				record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

				if(!record_set.IsEOF()) 
				{
					// fetch each, and recursive to it
					CStringX bundleLocalUID;
					short nIndex=0;
					record_set.GetFieldValue(nIndex, bundleLocalUID);
					CX2T atl_bundleUID(bundleLocalUID);

					glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Asset parent HUID %s is a bundle, re-calculate its EntryCount in NAV_Hierarchy (Bundle LocalEntryUID=%s)", 
						__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)parentHUIDs[i], (LPCTSTR)atl_bundleUID);

					retVal |= bundleCountUpdate(LPCTSTR(atl_bundleUID));
				}
				record_set.Close();
			}
		}

		// 2. Update NAV_Hierarchy(Folder) EntryCount whatever it is asset or bundle
		try
		{
			CString tmpLog;

			CString execStr;

			if(record_set.IsOpen())
				record_set.Close();

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find NAV folders in NAV_Hierarchy which include this entry after navigation building", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

			selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") + _T("WHERE Local_entry_UID='") +  localEntryUID + _T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			int i=0;
			while(!record_set.IsEOF()) 
			{
				// fetch each, and recursive to it
				CStringX navHUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex, navHUID);
				CX2T atl_navHUID(navHUID);

				tmpLog += CString(atl_navHUID) + L" ";

				entryNavParentHUIDs[LPCTSTR(atl_navHUID)]=LPCTSTR(atl_navHUID);

				record_set.MoveNext();
			}
			record_set.Close();

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s There are following NAV folders which include this entry after navigation building: %s", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, tmpLog);

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Re-calculate NAV Hierarchy folder EntryCount for this entry", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

			std::map<CString, CString>::iterator it = entryNavParentHUIDs.begin();
			for(; it!=entryNavParentHUIDs.end(); it++)
			{
				glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Update Nav_Hierarchy Folder EntryCount for NavHUID=%s", 
					__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, it->second);

				execStr.Format(_T("Exec ns_UpdateEntryCount '%s'"), it->second);
				traceSQL(execStr, __WFUNC__);
				_db.ExecuteSQL( execStr );		
			}
		}
		catch (CDBException* pDBexcep)
		{
			glog(Log::L_ERROR, L"[%s] (%s) navRemover::deleteEntry()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, pDBexcep->m_strError);

			pDBexcep->Delete();

			retVal = NS_ERROR;
		}

	}

	return retVal;

}

// add by KenQ routine for new navigation building logic
// these codes are before performance improvement
/*
int navSplUpdater::updateAffected(CString localEntryUID, int entryType, int operationType, CString nav_HUID, int nav_sourceType, CString levelChar)
{
	int retVal = NS_SUCCESS;

	CString selectStr;
	CDatabase tmpDb;
	tmpDb.OpenEx(_connRCStr, CDatabase::noOdbcDialog);
	CNAVTable record_set(&tmpDb, _connRCStr);
	CNAVTable record_set_manual(&tmpDb, _connRCStr);

	CStringArray srcHUIDs;
	CArray<int> srcHUIDsSrcType;
	CStringArray parentHUIDs;
	CString tmpSrcHUID;

	CStringArray entryNavParentHUIDs;

	CString srcLog, tagLog;
	int i = 0;
	/////////////////////////////////////////////
	// find all source LAM/NAV HUID, and save to parentHUIDs
	if(FOLDER_SRC_TYPE_LAM == nav_sourceType)
	{	
		// get asset/bundle parents from NAV_Hierarchy before updating for later entry count
		try
		{
			CString execStr;

			if(record_set.IsOpen())
				record_set.Close();

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find all parent Nav folder in Nav_Hierarchy before folder updating", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

			selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") + _T("WHERE Local_entry_UID='") +  localEntryUID + _T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			while(!record_set.IsEOF()) 
			{
				// fetch each, and recursive to it
				CStringX navHUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex, navHUID);
				CX2T atl_navHUID(navHUID);

				entryNavParentHUIDs.Add(LPCTSTR(atl_navHUID));

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


		if(L"" == nav_HUID)
		{	// the invoker set nav_HUID to empty if it is an asset update
			// if is an asset update, all its parent Hierarchy target must be synced. (One Asset may link in different folders)
			if(record_set.IsOpen())
				record_set.Close();

			selectStr = _T("SELECT LocalParentHUID FROM Hierarchy WHERE LocalEntryUID='")+localEntryUID+_T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			while(!record_set.IsEOF()) 
			{
				// fetch each, and recursive to it
				CStringX HUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex, HUID);
				CX2T atl_HUID(HUID);

				parentHUIDs.Add((LPCTSTR)atl_HUID);

				record_set.MoveNext();
			}
			record_set.Close();
		} // end of if("" == nav_HUID)
		else
		{
			parentHUIDs.Add(nav_HUID);
		}

		CString huidlog;
		for(i=0; i<parentHUIDs.GetSize(); i++)
		{
			huidlog += parentHUIDs[i] + L" ";
		}

		glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Source Parent HUID are: %s ", 
			__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)huidlog);

		// get Asset/Bundle parents HUID
		for(i=0; i<parentHUIDs.GetSize(); i++)
		{
			tmpSrcHUID = parentHUIDs[i];

			// add direct parent HUID firstly
			srcHUIDs.Add(tmpSrcHUID);
			srcHUIDsSrcType.Add(FOLDER_SRC_TYPE_LAM);			

			// find all all ancestor HUID
			while(tmpSrcHUID != "H0" && tmpSrcHUID != "0" && tmpSrcHUID != "" )
			{
				if(record_set.IsOpen())
					record_set.Close();

				selectStr = _T("SELECT LocalParentHUID FROM Hierarchy WHERE LocalHierarchyUID='")+tmpSrcHUID+_T("'");

				traceSQL(selectStr, __WFUNC__);
				record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

				if(!record_set.IsBOF()) 
				{
					CStringX parent_HUID;
					short nIndex=0;
					record_set.GetFieldValue(nIndex,parent_HUID);
					CX2T atl_parent_HUID(parent_HUID);
					record_set.Close();

					tmpSrcHUID = atl_parent_HUID;

					// to avoid the duplicated source id
					int i=0;
					for(i=0; i<srcHUIDs.GetSize(); i++)
					{
						if(tmpSrcHUID == srcHUIDs[i])
							break;
					}
					if(srcHUIDs.GetSize() == i)
					{
						srcHUIDs.Add(tmpSrcHUID);			
						srcHUIDsSrcType.Add(FOLDER_SRC_TYPE_LAM);		
					}
				}
				else
				{
					tmpSrcHUID = "";
					record_set.Close();
				}
			} // end of while(tmpSrcHUID != ...)
		} // end of for(int i=0; i<parentHUIDs.GetSize(); i++)
		
		// figure out all parent folder in NAV_Hierarchy if I am a manual maintained asset/bundle
		if(record_set_manual.IsOpen())
			record_set_manual.Close();
	
		selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE Local_entry_UID='")+localEntryUID+_T("' AND Is_manual=1");
	
		traceSQL(selectStr, __WFUNC__);
		record_set_manual.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);
	
		while(!record_set_manual.IsEOF()) 
		{
			// fetch each, and recursive to it
			CStringX HUID;
			short nIndex=0;
			record_set_manual.GetFieldValue(nIndex, HUID);
			CX2T atl_HUID(HUID);
			
			tmpSrcHUID = atl_HUID;
			
			// add the direct parent HUID first
			srcHUIDs.Add(tmpSrcHUID);
			srcHUIDsSrcType.Add(FOLDER_SRC_TYPE_NAV);

			while(tmpSrcHUID != "H0" && tmpSrcHUID != "0" && tmpSrcHUID != "" )
			{
				if(record_set.IsOpen())
					record_set.Close();
				selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE Hierarchy_UID='")+tmpSrcHUID+_T("'");
	
				traceSQL(selectStr, __WFUNC__);
				record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);
	
				if(!record_set.IsBOF()) 
				{
					CStringX parent_HUID;
					short nIndex=0;
					record_set.GetFieldValue(nIndex,parent_HUID);
					CX2T atl_parent_HUID(parent_HUID);
	
					tmpSrcHUID = atl_parent_HUID;
					
					// to avoid the duplicated source id
					int i=0;
					for(i=0; i<srcHUIDs.GetSize(); i++)
					{
						// avoid the same LAM HUID, so need to check source type either
						if(tmpSrcHUID == srcHUIDs[i] && FOLDER_SRC_TYPE_NAV == srcHUIDsSrcType[i])
							break;
					}
					if(srcHUIDs.GetSize() == i)
					{
						srcHUIDs.Add(tmpSrcHUID);			
						srcHUIDsSrcType.Add(FOLDER_SRC_TYPE_NAV);		
					}	
				}
				else
				{
					tmpSrcHUID = "";
					record_set.Close();
				}
			}
	
			record_set_manual.MoveNext();
		}
		
	} // end of if(0 == nav_sourceType)
	else if(FOLDER_SRC_TYPE_NAV == nav_sourceType)
	{
		// if sourceType is Navigation, get Navigation HUID all parent HUID

		// add asset direct parent HUID
		srcHUIDs.Add(nav_HUID);
		srcHUIDsSrcType.Add(FOLDER_SRC_TYPE_NAV);

		while(tmpSrcHUID != "H0" && tmpSrcHUID != "0" && tmpSrcHUID != "" )
		{
			if(record_set.IsOpen())
				record_set.Close();
			selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") +_T("WHERE Hierarchy_UID='")+nav_HUID+_T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			if(!record_set.IsBOF()) 
			{
				CStringX parent_HUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex,parent_HUID);
				CX2T atl_parent_HUID(parent_HUID);

				tmpSrcHUID = atl_parent_HUID;
				
				int i=0;
				for(i=0; i<srcHUIDs.GetSize(); i++)
				{
					// avoid the same LAM HUID, so need to check source type either
					if(tmpSrcHUID == srcHUIDs[i] && FOLDER_SRC_TYPE_NAV == srcHUIDsSrcType[i])
						break;
				}
				if(srcHUIDs.GetSize() == i)
				{
					srcHUIDs.Add(tmpSrcHUID);			
					srcHUIDsSrcType.Add(FOLDER_SRC_TYPE_NAV);		
				}	
			}
			else
			{
				tmpSrcHUID = "";
				record_set.Close();
			}
		}
	}
	////////////////////////////////////////////////////////////
	// find all the target NAV HUID

	CString typeStr;
	wchar_t typebuff[8];
	
	CString srcHUID;
	CString targetHUID;
	CStringArray targetHUIDs;
	for(i=0; i<srcHUIDs.GetSize(); i++)
	{
		srcHUID = srcHUIDs[i];
		
		_itow(srcHUIDsSrcType[i], typebuff, 10);
		typeStr = typebuff;

		srcLog = srcLog + _T("SourceHUID=") + srcHUID + _T(",SourceType=")+typeStr+_T(";");

		// deal with any folder that has source pointed to me
		if(record_set.IsOpen())
			record_set.Close();

		selectStr = _T("SELECT Hierarchy_UID FROM")+ tableName(L"NAV_Folder_Condition") +_T("WHERE Source_HUID='")+srcHUID+_T("' AND Source_type=")+typeStr;

		traceSQL(selectStr, __WFUNC__);
		record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

		while(!record_set.IsEOF()) 
		{
			// fetch each, and recursive to it
			CStringX HUID;
			short nIndex=0;
			record_set.GetFieldValue(nIndex, HUID);
			CX2T atl_HUID(HUID);

			// check whether this target HUID has been already processed
			switch(isInStackOrDone(LPCTSTR(atl_HUID))) 
			{
			case 0:   // not in stack
				targetHUIDs.Add(LPCTSTR(atl_HUID));
				tagLog = tagLog + " " + LPCTSTR(atl_HUID);
				break;
			case 1:   // in the stack, and already processed
			case -1:  // in the stack, but not processed
				break;
			}

			record_set.MoveNext();
		}
		record_set.Close();
	}
		
	////////////////////////////////////////////////////////////
	// build the target folder in NAV_Hierarch

	if(0 == targetHUIDs.GetSize())
	{
		glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Source HUID: %s No Target HUID, reach the end of recursive", 
			__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)srcLog, (LPCTSTR)tagLog);

		return retVal;
	}

	glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Source HUID: %s, Target HUID: %s", 
		__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)srcLog, (LPCTSTR)tagLog);
	CStringArray affectedHUIDS;
	retVal |= targetUpdate(localEntryUID, entryType, operationType, targetHUIDs, affectedHUIDS);

	
	for(i=0; i<targetHUIDs.GetSize(); i++)
	{
		// mark me as Done
		_callStack[targetHUIDs[i]]=TRUE;
	}
   tagLog= "";
	for(i=0; i<affectedHUIDS.GetSize(); i++)
	{
		tagLog = tagLog + " " + affectedHUIDS[i];
	}

	glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Affected Target HUID: %s", 
		__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)tagLog);

	for(i=0; i<affectedHUIDS.GetSize(); i++)
	{
		retVal |= updateAffected(localEntryUID, entryType, operationType, affectedHUIDS[i], FOLDER_SRC_TYPE_NAV, levelChar+L"-");
	}

	// at the end of routine, update entry count
	if(FOLDER_SRC_TYPE_LAM == nav_sourceType)
	{
		// 1. Update bundle entrycount in NAV_Hierarchy
		if(DBSYNC_ENTRY_TYPE_ASSET == _wq_Entry.Entry_type)
		{
			for(i=0; i<parentHUIDs.GetSize(); i++)
			{
				// fetch the localEntryUID
				if(record_set.IsOpen())
					record_set.Close();

				selectStr.Format(_T("SELECT LocalEntryUID From Hierarchy WHERE LocalHierarchyUID='%s' AND ( EXISTS(SELECT 1 FROM Folder WHERE LocalFolderUID=LocalEntryUID AND isPackage=1) )"),
					parentHUIDs[i]);

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
						__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, (LPCTSTR)parentHUIDs[i], (LPCTSTR)atl_bundleUID);

					retVal |= bundleCountUpdate(LPCTSTR(atl_bundleUID));
				}
				record_set.Close();
			}
		}

		// 2. Update NAV_Hierarchy(Folder) EntryCount whatever it is asset or bundle
		try
		{
			CString execStr;

			if(record_set.IsOpen())
				record_set.Close();

			glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Find Nav folder in NAV_Hierarchy after folder updating", 
				__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar);

			selectStr = _T("SELECT Parent_HUID FROM")+ tableName(L"NAV_Hierarchy") + _T("WHERE Local_entry_UID='") +  localEntryUID + _T("'");

			traceSQL(selectStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, selectStr, CRecordset::none);

			int i=0;
			while(!record_set.IsEOF()) 
			{
				// fetch each, and recursive to it
				CStringX navHUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex, navHUID);
				CX2T atl_navHUID(navHUID);

				for(i=0; i<entryNavParentHUIDs.GetSize(); i++)
				{
					if(entryNavParentHUIDs[i] == LPCTSTR(atl_navHUID))
						break;
				}
				if(entryNavParentHUIDs.GetSize() == i)
				{
					entryNavParentHUIDs.Add(LPCTSTR(atl_navHUID));
				}

				record_set.MoveNext();
			}
			record_set.Close();

			for(i=0; i<entryNavParentHUIDs.GetSize(); i++)
			{
				glog(Log::L_DEBUG, L"[%s] (%s) LocalEntryUID=%s %s Update Nav_Hierarchy Folder EntryCount for NavHUID=%s", 
					__WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)localEntryUID, levelChar, entryNavParentHUIDs[i]);

				execStr.Format(_T("Exec ns_UpdateEntryCount '%s'"), entryNavParentHUIDs[i]);
				traceSQL(execStr, __WFUNC__);
				_db.ExecuteSQL( execStr );		
			}
		}
		catch (CDBException* pDBexcep)
		{
			glog(Log::L_ERROR, L"[%s] (%s) navRemover::deleteEntry()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, pDBexcep->m_strError);

			pDBexcep->Delete();

			retVal = NS_ERROR;
		}

	}

	return retVal;
}
*/
int navSplUpdater::targetUpdate(CString localEntryUID, int entryType, int operationType, const CStringArray& targetHUIDs, CStringArray& affectedHUIDs)
{
	int retVal = NS_SUCCESS;

	CString strEntryType;
	strEntryType.Format(L"%d", entryType);
    CString opType;
	opType.Format(L"%d", operationType);

	// combine the target HUID to a string, slipt with , 
	CString strNavHUID=L"";
	CString secondLastNavHUID=L"";
	int i=0;
	do 
	{
		CString strNavHUID=L"";
		CString secondLastNavHUID=L"";

		for(; i<targetHUIDs.GetSize(); i++)
		{
			if(targetHUIDs[i].GetLength() > MAX_SP_PARAMETER_LENGTH)
				continue;

			if( i == targetHUIDs.GetSize() - 1 )
				strNavHUID += targetHUIDs[i];
			else
				strNavHUID += targetHUIDs[i] +L",";

			if(strNavHUID.GetLength() > MAX_SP_PARAMETER_LENGTH)
			{
				strNavHUID = secondLastNavHUID;
				strNavHUID.SetAt(strNavHUID.GetLength()-1, '\0'); // remove last ,

				break;
			}
			// remember the second last string
			secondLastNavHUID = strNavHUID;
		}

		// execute the sp
		CString execStr = L"";

		if(_isQA)	// 3 params version, and for QANAV
		{

			// to support QANAV, we need to pass extra parameter to SP
			execStr.Format(L"{CALL ns_FolderUpdateEx( '%s',%s,%s,'%s','%s',1)}",localEntryUID,strEntryType, opType,strNavHUID, NavigationService::m_wszRegionID);
		}
		else	// 3 params version, standard NAV
		{
			execStr.Format(L"{CALL ns_FolderUpdateEx( '%s',%s,%s,'%s','%s',0)}",localEntryUID,strEntryType, opType,strNavHUID, NavigationService::m_wszRegionID);
		}

		try
		{
			if(!_db.IsOpen()) {
				_db.Open(NULL, FALSE, FALSE, _connDBStr);
			}

			CNAVTable record_set(&_db, _connRCStr);

			traceSQL(execStr, __WFUNC__);
			record_set.Open(AFX_DB_USE_DEFAULT_TYPE, execStr, CRecordset::none);


			while(!record_set.IsEOF()) 
			{
				// fetch each, and recursive to it
				CStringX HUID;
				short nIndex=0;
				record_set.GetFieldValue(nIndex, HUID);
				CX2T atl_HUID(HUID);

				affectedHUIDs.Add(LPCTSTR(atl_HUID));

				record_set.MoveNext();
			}

			record_set.Close();
		}
		catch (CDBException* pDBexcep)
		{
			glog(Log::L_ERROR, L"[%s] (%s) navSplUpdater::targetUpdate()  A database exception occurs, with error string: %s", __WQAID__, (LPCTSTR)_wq_Entry.Queue_UID, (LPCTSTR)pDBexcep->m_strError);

			pDBexcep->Delete();

			retVal = NS_ERROR;
		}		
	} while(i<targetHUIDs.GetSize());

	haveTea();

	return retVal;
}

