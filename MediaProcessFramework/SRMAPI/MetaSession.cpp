
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
// Name  : DBSessin.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : session in database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaSession.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 36    05-07-18 16:03 Daniel.wang
// 
// 35    05-06-28 11:39a Daniel.wang
// 
// 34    05-06-24 9:12p Daniel.wang
// 
// 33    05-06-24 5:11p Daniel.wang
// 
// 32    05-06-21 10:00p Daniel.wang
// 
// 31    05-06-21 12:19p Daniel.wang
// 
// 30    05-06-17 1:57p Daniel.wang
// 
// 29    05-06-16 11:05a Daniel.wang
// 
// 28    05-06-15 3:16p Daniel.wang
// 
// 27    05-06-14 7:45p Daniel.wang
// 
// 26    05-06-14 6:59p Daniel.wang
// 
// 25    05-06-14 4:58p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#include "MetaSession.h"
#include "../MPFException.h"
#include "SRMDaemon.h"

SRM_BEGIN

std::string SessionManager::m_strRootEntry;

SessionManager::SessionManager(size_t leaseterm, const char* entry)
:RecordManager(entry, RM_PROP_IMMEDIATELY_FLUSH|PM_PROP_LAST_UPDATE_TIME, leaseterm)
{
	m_strRootEntry = entry;
}

SessionManager::SessionManager(size_t leaseterm)
:RecordManager(DB_SESSION_ROOT, RM_PROP_IMMEDIATELY_FLUSH|PM_PROP_LAST_UPDATE_TIME, leaseterm)
{
	m_strRootEntry = DB_SESSION_ROOT;
}

bool SessionManager::OnLeaseTerm(const char* entry, size_t leaseterm)
{
	MetaRecord mr(entry, PM_PROP_READ_ONLY);

	size_t sessionstate = mr.get(SESSION_STATE);
	if (SS_RUNING != sessionstate)
		return true;

	size_t accesstime = mr.get(LAST_ACCESS);
	time_t curtime;
	time(&curtime);

	if ((curtime > accesstime) && ((curtime-accesstime)*1000 > leaseterm))
	{
		return false;
	}
	return true;
}

const char* SessionManager::getRootEntry()
{
	return m_strRootEntry.c_str();
}

MetaSession::MetaSession(const char* sessionid, unsigned int property)
: MetaRecord(utils::NodePath::getSubPath(SessionManager::getRootEntry(),
			 (sessionid?sessionid:EntryDB::newID())).c_str(), 
			 property|PM_PROP_CREATE_TIME|PM_PROP_LAST_ACCESS_TIME|RM_PROP_IMMEDIATELY_FLUSH)
{
}

SessionManager* g_mpf_session_manager_instance = NULL;

SRM_END
