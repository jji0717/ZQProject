// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: TriggerWorker.h,v 1.7 2004/07/29 06:26:30 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : DBSync configuration define
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/DBSyncConf.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 4     05-04-21 11:17 Kaliven.lee
// 
// 3     05-03-17 19:42 Kaliven.lee
// 3.2.11
// 
// 2     05-03-14 17:48 Kaliven.lee
// 3.14
// 
// 1     05-03-07 15:29 Kaliven.lee
// file create
#pragma  once
#include <queue>
#include "NativeThread.h"
#include "locks.h"
#include <map>
#define DEFAULTWORKERCOUNT		5
#define ITV_RETRY_TIMEOUT		30000

typedef struct tag_DetailTask{
		DWORD dwEntryUID;
		DWORD dwEntryType;
}DETAILTASK;
typedef std::queue<DETAILTASK*> WORKQUEUE;