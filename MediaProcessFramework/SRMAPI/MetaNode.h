
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
// Name  : MetaNode.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : node manager for database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaNode.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 41    05-07-27 17:33 Daniel.wang
// 
// 40    05-07-21 17:58 Daniel.wang
// 
// 39    05-07-21 17:47 Daniel.wang
// 
// 38    05-06-28 3:05p Daniel.wang
// 
// 37    05-06-28 11:39a Daniel.wang
// 
// 36    05-06-24 9:12p Daniel.wang
// 
// 35    05-06-24 5:11p Daniel.wang
// 
// 34    05-06-14 6:59p Daniel.wang
// 
// 33    05-06-14 4:58p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#ifndef _ZQ_DBNODE_H_
#define _ZQ_DBNODE_H_

#include "MetaResource.h"
#include "NativeThread.h"
#include "comextra\ZqBuffer.h"

SRM_BEGIN

class DLL_PORT NodeManager;
class DLL_PORT MetaNode;

struct HeartbeatInfo
{
	std::string		worknode;
	rpc::RpcValue	value;

	bool operator==(const HeartbeatInfo& hi)
	{ return worknode==hi.worknode; }
};

class NodeQuery : public ZQ::rpc::RpcServerMethod
{
private:
	NodeManager&	m_nm;

public:
	NodeQuery(NodeManager& nm);

	///call back function which called as heartbeat method
	///@param params - heartbeat method parameters
	///@param result - heartbeat method result
	void execute(rpc::RpcValue& params, rpc::RpcValue& result);
};

// -----------------------------\n
//NodeManager\n
// -----------------------------\n
/// the manager for resource node in database
class NodeManager : public ZQ::common::NativeThread, public ZQ::rpc::RpcServerMethod, public ResourceManager
{
private:
	friend class NodeQuery;
	rpc::RpcServer&					m_server;
	comextra::Buffer<HeartbeatInfo>	m_buffer;
	NodeQuery						m_nq;

protected:
	///call back function which called as heartbeat method
	///@param params - heartbeat method parameters
	///@param result - heartbeat method result
	void execute(rpc::RpcValue& params, rpc::RpcValue& result);

	virtual void OnQuery(rpc::RpcValue& params, rpc::RpcValue& result)
	{}

	///help method of heartbeat
	///@param strBuffer - help string buffer
	///@param nMax - max size of string buffer
	char* help(char* strBuffer, int nMax);

	///get the score of special resource
	///@param resourceentry - resource entry
	///@param sessionentry - session entry
	///@return score number
	BYTE score(const char* resourceentry, const char* sessionentry);
	
	///run\n
	///run heartbeat receive thread
	///@return - return thread code
	int run();
	
public:
	///constructor
	///@param server - rpc server end
	///@param database - database instance
	///@param leaseTerm - lease term time
	NodeManager(rpc::RpcServer& server, bool book = false, int leaseTerm = DEF_RESOURCE_CLEAR_TIME, unsigned int property = 0);

	///begin node manager processor
	bool begin();
};


// -----------------------------\n
//MetaNode\n
// -----------------------------\n
/// work node record in database
class MetaNode : public MetaResource
{
public:
	///constructor
	///@param nm - node manager instance
	///@param nodeid - node id
	///@param property - record property
	///@param leaseterm - lease term time
	MetaNode(const NodeManager& nm, const char* nodeid, unsigned int property = 0, size_t leaseterm = DEF_RESOURCE_CLEAR_TIME);

	///constuctor
	///@param nodeid - node id
	///@param property - record property
	///@param leaseterm - lease term time
	///@param rootentry - root entry
	MetaNode(const char* nodeid, unsigned int property = 0, size_t leaseterm = DEF_RESOURCE_CLEAR_TIME, const char* rootentry = DB_NODE_ROOT);
};



SRM_END

#endif//_ZQ_DBNODE_H_
