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
// Ident : $Id: JndiClient.java, hui.shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/sdk/AquaClientImpl.h $
// 
// 3     11/19/14 8:05p Hui.shao
// userDomain
// 
// 2     2/20/14 10:03a Hui.shao
// nonCdmi_CreateObject
// 
// 1     2/19/14 7:12p Hui.shao
// 
// 2     1/02/14 5:47p Hui.shao
// 
// 1     11/19/13 3:15p Hui.shao
// AquaClient draft
// ===========================================================================
#ifndef __CDMIFUSE_AQUA_CLIENT_IMPL_H__
#define __CDMIFUSE_AQUA_CLIENT_IMPL_H__

#include "ZQ_common_conf.h"
#include "Locks.h"
#include "AquaClient.h"
#include "FileLog.h"
#include "../CdmiFuseOps.h"

#include <json/json.h>
#include <vector>

namespace XOR_Media {
namespace AquaClient {

class AquaClientImpl : public AquaClient
{
public:
	AquaClientImpl(const std::string& rootUrl, const std::string& userDomain, const std::string& homeContainer, const uint32 flags, const FuseOpsConf& conf);
	virtual ~AquaClientImpl();

	virtual void release();

	CdmiFuseOps* _ctx;
	int exec_Cdmi(const std::string& cmd, const std::string& uri, Json::Value& jsonArgs, Json::Value& jsonResult);
	int exec_nonCdmi(const std::string& cmd, const std::string& uri, Json::Value& jsonArgs, Json::Value& result, char* buf, uint32& len);
};

extern ZQ::common::FileLog* pLogger;
extern ZQ::common::NativeThreadPool* pThrdPool;

}}

#define NTAG(_T)  ("NTAG_" #_T)

#endif // __CDMIFUSE_AQUA_CLIENT_IMPL_H__