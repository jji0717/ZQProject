
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
// Name  : MetaTask.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : task in database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaTask.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 21    05-06-28 11:39a Daniel.wang
// 
// 20    05-06-24 9:12p Daniel.wang
// 
// 19    05-06-24 5:11p Daniel.wang
// 
// 18    05-06-20 3:11p Daniel.wang
// 
// 17    05-06-14 4:58p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#include "MetaTask.h"
#include "SRMDaemon.h"
#include "MPFException.h"


SRM_BEGIN

std::string TaskManager::m_strRootEntry;

TaskManager::TaskManager(size_t leaseterm, const char* entry)
:RecordManager(entry, RM_PROP_IMMEDIATELY_FLUSH|PM_PROP_LAST_UPDATE_TIME, leaseterm)
{
	m_strRootEntry = entry;
}

TaskManager::TaskManager(size_t leaseterm)
:RecordManager(m_strRootEntry.c_str(), RM_PROP_IMMEDIATELY_FLUSH|PM_PROP_LAST_UPDATE_TIME, leaseterm)
{
}

bool TaskManager::OnLeaseTerm(const char* entry, size_t leaseterm)
{
	MetaRecord mr(entry, PM_PROP_READ_ONLY);
	size_t accesstime = mr.get(LAST_ACCESS);
	time_t curtime;
	time(&curtime);

	if ((curtime > accesstime) && ((curtime-accesstime)*1000 > leaseterm))
	{
		return false;
	}
	return true;
}

const char* TaskManager::getRootEntry()
{
	return m_strRootEntry.c_str();
}

MetaTask::MetaTask(const char* taskid, unsigned int property)
		: MetaRecord(utils::NodePath::getSubPath(TaskManager::getRootEntry(), taskid).c_str(),
		property|PM_PROP_LAST_ACCESS_TIME)
{
}

TaskManager* g_mpf_task_manager_instance = NULL;


SRM_END

