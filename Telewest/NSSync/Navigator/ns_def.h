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
// Name  : nsOptimizer.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : macro and defines
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/ns_def.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 4     08-09-19 12:26 Ken.qian
// add EntryType in the function
// 
// 3     08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================

#pragma once

//////////////////////////////////////////////////////////////////////////
// default timeout for ODBC
#define DEFAULT_SQL_TIMEOUT		60

//////////////////////////////////////////////////////////////////////////
// worker type

#define WORKER_NONE				-1
#define WORKER_REBUILD			1
#define WORKER_SAMPLEUPDATE		2
#define WORKER_REMOVE			3
#define WORKER_RENAME			4

//////////////////////////////////////////////////////////////////////////
// error code

#define NS_SUCCESS		0

#define NS_ERROR		100
#define NS_NOENTRY		(NS_ERROR+1)
#define NS_BADWORKER	(NS_ERROR+2)
#define NS_RECURLOOP	(NS_ERROR+3)

//////////////////////////////////////////////////////////////////////////
// DBSync operation type
#define WQ_DBSYNC_ADD           1
#define WQ_DBSYNC_DELETE        2
#define WQ_DBSYNC_LINK          3
#define WQ_DBSYNC_UNLINK        4
#define WQ_DBSYNC_RENAME        5
#define WQ_DBSYNC_UPDATE        6

// DBSync Entry Type
#define DBSYNC_ENTRY_TYPE_ASSET		1
#define DBSYNC_ENTRY_TYPE_PACKAGE	11

//////////////////////////////////////////////////////////////////////////
// Navigation source type
#define FOLDER_SRC_TYPE_LAM        0
#define FOLDER_SRC_TYPE_NAV        1

//////////////////////////////////////////////////////////////////////////
// Navigation folderUpdate type
#define SP_FOLDER_UPDATE_TYPE_SAMPLE_SYNC     0
#define SP_FOLDER_UPDATE_TYPE_TARGET_SYNC     1