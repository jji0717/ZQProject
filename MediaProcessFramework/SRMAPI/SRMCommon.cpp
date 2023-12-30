
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
// Name  : SRMCommon.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : common utility for SRM
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/SRMCommon.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 24    05-08-23 15:13 Jie.zhang
// 
// 23    05-08-10 10:55 Jie.zhang
// 
// 22    05-08-01 13:48 Jie.zhang
// 
// 21    05-06-28 8:30p Daniel.wang
// 
// 20    05-06-28 11:39a Daniel.wang
// 
// 19    05-06-24 5:11p Daniel.wang
// 
// 18    05-06-21 12:19p Daniel.wang
// 
// 17    05-06-16 11:05a Daniel.wang
// 
// 16    05-06-14 4:57p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#include "../MPFCommon.h"
#include "SRMCommon.h"

#include "MetaRecord.h"

#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>

#include "../MPFException.h"

SRM_BEGIN

void DbEntry2RpcValue(MetaRecord& ent, rpc::RpcValue& val)
{
	rpc::RpcValue propertyval;
	for (size_t i = 0; true; ++i)
	{
		char key[MAX_DB_ENTRY_LEN] = {0};
		if (NULL == ent.getKey(i, key, MAX_DB_ENTRY_LEN))
			break;

		char value[MAX_DB_VALUE_LEN] = {0};
		propertyval.setStruct(key, rpc::RpcValue(ent.get(key, value, MAX_DB_VALUE_LEN)));
	}

	val.setStruct(PROP_VALUE, propertyval);

	size_t count;
	char** pstrEntry = ent.listChildren(count);
	for (i = 0; i < count; ++i)
	{
		MetaRecord subrecord(pstrEntry[i], PM_PROP_READ_ONLY);
		rpc::RpcValue subval;
		DbEntry2RpcValue(subrecord, subval);

		val.setStruct(utils::NodePath::getPureName(pstrEntry[i]).c_str(), subval);
	}
	
	if (pstrEntry)
		ent.deleteList(pstrEntry, count);	
}


SRM_END
