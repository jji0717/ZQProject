
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
// Name  : SRMCommon.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : common utility for SRM
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/SRMCommon.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 32    05-07-27 17:33 Daniel.wang
// 
// 31    05-07-15 11:51 Daniel.wang
// 
// 30    05-06-28 8:30p Daniel.wang
// 
// 29    05-06-28 20:24 Yan.zheng
// 
// 28    05-06-28 11:39a Daniel.wang
// 
// 27    05-06-24 5:11p Daniel.wang
// 
// 26    05-06-21 10:00p Daniel.wang
// 
// 25    05-06-21 12:19p Daniel.wang
// 
// 24    05-06-20 3:11p Daniel.wang
// 
// 23    05-06-16 11:05a Daniel.wang
// 
// 22    05-06-14 8:58p Daniel.wang
// 
// 21    05-06-14 7:45p Daniel.wang
// 
// 20    05-06-14 6:59p Daniel.wang
// 
// 19    05-06-14 4:58p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#ifndef _ZQ_SRMCOMMON_H_
#define _ZQ_SRMCOMMON_H_

#include <windows.h>

#pragma warning(disable: 4786)
#include <string>

#include "mpfloghandler.h"

#include "xmlrpc.h"
#include "ComExtra/ZqCommon.h"
#include "ComExtra/ZqSafeMem.h"
#include "NativeThread.h"

#include "locks.h"

//all the code mast be inside SRM_BEGIN and SRM_END pair
#define SRM_BEGIN namespace ZQ{namespace MPF{namespace SRM{
#define SRM_END }}}

//naming define
#define HEARTBEAT_EVENT "_WorkNodeHeartbeatEvent"
#define SETTING_DB_FILE "DBFile"
#define LEASE_TERM "LeaseTerm"
#define SESSION_LIFE_CYCLE "LifeCycle"


#define HIDDEN_ATTR ".#$"
//hidden attributes in database
#define LAST_UPDATE HIDDEN_ATTR"LastUpdate" //the time of last write data to database record
#define LAST_ACCESS HIDDEN_ATTR"LastAccess"	//the time of last open or close database record
#define CREATE_TIME HIDDEN_ATTR"CreateTime" //the time of create session record
#define LAST_SAVE HIDDEN_ATTR"LastSave"

//database root and sub entries
#define DB_RESOURCE_ROOT		DB_ROOT DB_SPEC"Resource"
#define DB_SESSION_ROOT			DB_ROOT DB_SPEC"LocalSession"
#define DB_TASK_ROOT			DB_ROOT DB_SPEC"Tasks"
#define DB_RESOURCE_NODE		"Nodes"
#define DB_NODE_ROOT			DB_RESOURCE_ROOT DB_SPEC DB_RESOURCE_NODE

#define MAX_FLOAT_STR_LEN	30
#define MAX_INT8_STR_LEN	4
#define MAX_INT16_STR_LEN	6
#define MAX_INT32_STR_LEN	11
#define MAX_INT64_STR_LEN	21

//default setting
#define DEF_LEASETERM 60000

//data limit
#define MAX_IP_STR_LEN			16
#define MAX_CPU_STR_LEN			256
#define MAX_OS_STR_LEN			256
#define MAX_HARDWARE_STR_LEN	1024
#define MAX_TASK_STATUS_LEN		256
#define MAX_TASK_TYPE_LEN		256

#define HB_IDLE_TIME 500
#define HB_IDLE_NUMBER 5
#define SCORE_0 0

#define MAX_DB_ENTRY_COUNT	256
#define MAX_DB_ENTRY_LEN	256
#define MAX_DB_VALUE_LEN	256

#define DB_FILE_PATH "edbb4://localhost/c:\\test.db"
#define DB_MEMORY_PATH "edbnil://localhost"
#define SESSION_LOG_FILE "c:\\srm_session.log"

#define DB_HEARTBEAT_TIME		MPF_RESERVED_PREFIX "HeartBeat"

#define DEF_SESSION_CLEAR_TIME		100000
#define DEF_TASK_CLEAR_TIME			100000
#define DEF_RESOURCE_CLEAR_TIME		1000000
#define DEF_HEARTBEAT_SLEEP_TIME	5000

#define PROP_VALUE	"property"

#define MAX_RPC_KEY_STR_LEN 256
#define MAX_RPC_VALUE_STR_LEN 256

#define RESOURCE_COUNT "ResourceCount"

SRM_BEGIN

class MetaRecord;
class RecordManager;
//get rpc value from database
void DbEntry2RpcValue(MetaRecord& ent, rpc::RpcValue& val);


SRM_END

#endif//_ZQ_SRMCOMMON_H_
