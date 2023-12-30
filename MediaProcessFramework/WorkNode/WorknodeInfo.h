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
// Name  : WorknodeInfo.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-6-3
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/WorkNode/WorknodeInfo.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 2     05-06-07 18:08 Bernie.zhao
// added sysTray support
// ===========================================================================

#ifndef __WORKNODEINFO_H__
#define __WORKNODEINFO_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "listinfo.h"

MPF_WORKNODE_NAMESPACE_BEGIN
USE_MPF_WORKNODE_NAMESPACE

class TaskAcceptor;
///@brief this is an RpcMethod that list worknode information
class WorknodeInfo : public ZQ::MPF::ListInfoMethod  
{
public:
	/// constructor
	///@param[in]	acpt			-the reference of task acceptor instance this belongs to
	///@param[in]	server			-rpc server instance to bind
	///@param[in]	strInterface	-the network interface MPF currently uses
	WorknodeInfo(TaskAcceptor& acpt, ZQ::rpc::RpcServer& server, const char* strInterface);

	/// destructor
	virtual ~WorknodeInfo();

protected:
	/// this function will be called when information request come
	///@param[in]	strInfoType		-info request type
	///@param[in]	params			-the input parameters
	///@param[out]	result			-the output result
	virtual void OnGetInfo(const char* strInfoType, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result);

protected:

	/// reference of task acceptor instance this belongs to
	TaskAcceptor&	_acceptor;
};
MPF_WORKNODE_NAMESPACE_END

#endif // #ifndef __WORKNODEINFO_H__
