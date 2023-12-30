
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
// Name  : MetaSession.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : session manager for database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaSession.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 31    05-07-19 14:46 Daniel.wang
// 
// 30    05-07-18 17:22 Daniel.wang
// 
// 29    05-07-18 16:03 Daniel.wang
// 
// 28    05-06-28 3:05p Daniel.wang
// 
// 27    05-06-24 9:12p Daniel.wang
// 
// 26    05-06-24 5:11p Daniel.wang
// 
// 25    05-06-17 1:57p Daniel.wang
// 
// 24    05-06-16 11:05a Daniel.wang
// 
// 23    05-06-14 6:59p Daniel.wang
// 
// 22    05-06-14 4:58p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#ifndef _ZQ_DBSESSION_H_
#define _ZQ_DBSESSION_H_

#include "MetaRecord.h"
#include <Timestamp.h>

SRM_BEGIN

#define SESS_PARAM_ATLEAST_TRAFFIC	"LeastTraffic"
#define SESS_PARAM_TASKTYPE			"TaskType"
#define SESS_PARAM_LEASETERM		"LeaseTerm"

#define SESSION_STATE				"SessionState"
#define SS_INIT						0
#define SS_RUNING					1
#define SS_STOP						2

// -----------------------------
//SessionManager
// -----------------------------
/// session manager for database
class DLL_PORT SessionManager : public RecordManager
{
private:
	static std::string		m_strRootEntry;

	bool OnLeaseTerm(const char* entry, size_t leaseterm);

public:
	///constructor
	///@param leaseterm - session leaseterm time
	///@param entry - session manager entry
	SessionManager(size_t leaseterm, const char* entry);

	///constructor
	///@param leaseterm - session leaseterm time
	SessionManager(size_t leaseterm);

	///get session manager root entry
	static const char* getRootEntry();
};

// -----------------------------
//MetaSession
// -----------------------------
/// comment: session in database
class DLL_PORT MetaSession : public MetaRecord
{
public:
	///constructor
	///@param sessionid - session id string
	///@param property - session property
	MetaSession(const char* sessionid = NULL, unsigned int property = PM_PROP_LAST_SAVE_TIME);
};

DLL_PORT extern SessionManager* g_mpf_session_manager_instance;


SRM_END

#endif//_ZQ_DBSESSION_H_
