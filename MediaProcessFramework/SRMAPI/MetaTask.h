
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
// Dev  : Microsoft Developer Studio
// Name  : MetaTask.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : task manager in database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaTask.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 25    05-07-18 17:22 Daniel.wang
// 
// 24    05-07-15 10:05 Daniel.wang
// 
// 23    05-06-28 3:05p Daniel.wang
// 
// 22    05-06-24 5:11p Daniel.wang
// 
// 21    05-06-20 3:11p Daniel.wang
// 
// 20    05-06-14 4:58p Daniel.wang
// 15    05-06-03 5:24p Daniel.wang
// add task status key
// 14    05-06-03 5:14p Daniel.wang
// add comment
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#ifndef _ZQ_DBTASK_H_
#define _ZQ_DBTASK_H_

#include "MetaSession.h"


#define TASK_STATUS_KEY		"TaskStatus"

#define TASK_STATUS_INIT	"TaskInit"
#define TASK_STATUS_SETUP	"TaskSetup"
#define TASK_STATUS_PROCESS	"TaskProcess"
#define TASK_STATUS_FINAL	"TaskFinal"

SRM_BEGIN


// -----------------------------
//DBTaskManager
// -----------------------------
/// task manager for database

class DLL_PORT TaskManager : public RecordManager
{
private:
	static std::string		m_strRootEntry;
	
	bool OnLeaseTerm(const char* entry, size_t leaseterm);
public:
	///constructor
	///@param leaseterm - leaseterm time
	///@param entry - task manager entry
	TaskManager(size_t leaseterm, const char* entry);

	///constructor
	///@param leaseterm - leaseterm time
	TaskManager(size_t leaseterm);
	
	///get root entry
	static const char* getRootEntry();
};

// -----------------------------
//MetaTask
// -----------------------------
/// task in database
class DLL_PORT MetaTask : public MetaRecord
{
public:
	///constructor
	///@param taskid - task id string(worknode id +"." + task id)
	///@param property - record property
	MetaTask(const char* taskid, unsigned int property = PM_PROP_LAST_SAVE_TIME);
};

extern DLL_PORT TaskManager* g_mpf_task_manager_instance;

SRM_END

#endif//_ZQ_DBTASK_H_
