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
// Desc  : worker for rebuild the whole NE-Tree
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/navRebuilder.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 1     05-03-25 12:47 Bernie.zhao
// ===========================================================================


#if !defined(AFX_NAVREBUILDER_H__B8C01628_B276_4A74_AB4B_155885AB07B9__INCLUDED_)
#define AFX_NAVREBUILDER_H__B8C01628_B276_4A74_AB4B_155885AB07B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>

// local include
#include "navWorker.h"

class navRebuilder : public navWorker  
{
protected:
	friend class navWorkerProvider;

	/// protected constructor, use navWorkerProvider to create instance
	navRebuilder(WQ_Entry& wqentry, CString connectStr);

	/// protected destructor, call free() to delete self
	virtual ~navRebuilder();

public:
	/// free method
	virtual void free();

	/// overrided function
	///@return	work result, NS_SUCCESS if successfully
	virtual int work();

	/// recursively update specified folder and all children in NE tree
	///@param[in]	nav_HUID	the hierarchyUID of this folder in NAV_Hierarchy table
	///@return		NS_SUCCESS if successfully
	int purifyFolder(CString nav_HUID, CString levelChar = L"|");

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

private:
	/// call stack, indicating if folder is in recursive stack and if is marked as done
	std::map<CString, bool> _callStack;

};

#endif // !defined(AFX_NAVREBUILDER_H__B8C01628_B276_4A74_AB4B_155885AB07B9__INCLUDED_)
