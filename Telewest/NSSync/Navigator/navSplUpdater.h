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
// Name  : navRebuilder.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : worker for update sample space
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navSplUpdater.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 8     08-12-29 19:28 Build
// check in by Ken Qian for performance tuning
// 
// 7     08-12-23 12:08 Ken.qian
// add codes to calculate entrycount and handle the manual mantained
// asset/bundle
// 
// 6     08-12-11 13:45 Li.huang
// add codes to avoid the duplicated source HUID to improve the
// performance
// 
// 5     08-09-19 12:26 Ken.qian
// add EntryType in the function
// 
// 4     08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 3     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================
#pragma once

#include <map>

#include "navworker.h"

class navSplUpdater :
	public navWorker
{
protected:
	friend class navWorkerProvider;

	/// protected constructor, use navWorkerProvider to create instance
	navSplUpdater(WQ_Entry& wqentry, CString connectStr);

	/// protected destructor, call free() to delete self
	virtual ~navSplUpdater();

public:
	/// free method
	virtual void free();

	/// overrided function
	///@return	work result, NS_SUCCESS if successfully
	virtual int work();

public:
	/// recursively update specified folder, all folder which have source pointed to it, and all its parent
	///@param[in]	nav_HUID	the hierarchyUID of this folder in NAV_Hierarchy table
	///@param[in]	nav_sourceType		the type of this folder, 0 indicating from local AM, and 1 indicating from NAV Hierarchy
	///@return		NS_SUCCESS if successfully
	int updateAffected(CString nav_HUID, int nav_sourceType, CString levelChar=L"|");

	/// call stored procedure to list all qualified assets under this folder
	///@param[in]	nav_HUID	the hierarchyUID of this folder in NAV_Hierarchy table
	///@return		NS_SUCCESS if successfully
	int folderUpdate(CString nav_HUID);

	/// check if this folder is in call stack or not to avoid recursive loop
	/// if folder not in call stack, add this folder
	///@param[in]	nav_HUID	the hierarchyUID of this folder in NAV_Hierarchy table
	///@return		0 if it is the first time enter call stack
	///             1 if this folder had finished update job and marked as done
	///            -1 if this folder is already in stack, but not marked as done. some sort of recursive loop occurs
	///@remarks		all folder in stack would be marked as done when this folder finished its update job
	int isInStackOrDone(CString nav_HUID);

	/// call store procedure to build target Navigation folder
	///@param[in]	localEntryUID	the entry which trigger the target updating
	///@param[in]	operationType   to distinguish the source operation, such as Asset Deletion or Metadata update
	///@param[in]   targetHUIDs     the target HUID which point current LAM or NAV folder
	///@param[out]  affectedHUIDs   after the updating, return the affected target HUID (sub set of targetHUIDs)
	///@return		0 if it is the first time enter call stack
	int targetUpdate(CString localEntryUID, int entryType, int operationType, const CStringArray& targetHUIDs, CStringArray& affectedHUIDs);

	/// recursively update specified object in Navigation, all folder which have source pointed to it, and all its parent
	///@param[in]	localEntryUID	Entry Local UID which trigger the 
	///@param[in]	operationType   to distinguish the source operation, such as Asset Deletion or Metadata update
	///@param[in]	nav_HUID	    the hierarchyUID of this folder in NAV_Hierarchy table
	///@param[in]	nav_sourceType	the type of this folder, 0 indicating from local AM, and 1 indicating from NAV Hierarchy
	///@return		NS_SUCCESS if successfully
	int updateAffected(CString localEntryUID, int entryType, int operationType, CString HUID, int nav_sourceType, CStringArray& affectedHUIDs, CString levelChar=L"|");

	int getTargetNAVFolders(int sourceType, const CString& localEntryUID, const CString& HUID, CStringArray& targetNavHUIDs, CStringArray& parentHUIDs, CString levelChar=L"|");
private:
	/// call stack, indicating if folder is in recursive stack and if is marked as done
	std::map<CString, bool> _callStack;
};
