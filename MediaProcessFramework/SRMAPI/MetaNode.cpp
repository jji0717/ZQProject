
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
// Name  : MetaNode.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-8
// Desc  : node in database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaNode.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 77    05-08-23 15:13 Jie.zhang
// 
// 76    05-08-10 10:55 Jie.zhang
// 
// 75    05-07-27 17:33 Daniel.wang
// 
// 74    05-07-21 17:58 Daniel.wang
// 
// 73    05-07-21 17:47 Daniel.wang
// 
// 72    05-07-07 15:13 Jie.zhang
// 
// 71    05-07-01 7:44p Daniel.wang
// 
// 70    05-06-29 3:38p Daniel.wang
// 
// 69    05-06-28 11:39a Daniel.wang
// 
// 68    05-06-27 10:10a Daniel.wang
// 
// 67    05-06-24 5:11p Daniel.wang
// 
// 66    05-06-21 10:00p Daniel.wang
// 
// 65    05-06-21 12:19p Daniel.wang
// 
// 64    05-06-20 16:37 Yan.zheng
// 
// 63    05-06-14 7:45p Daniel.wang
// 
// 62    05-06-14 6:59p Daniel.wang
// 
// 61    05-06-14 4:58p Daniel.wang
// 10    05-04-15 11:31 Daniel.wang
// change the arraies to structures
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#include "MetaNode.h"
#include "../MPFException.h"
#include "MPFCommon.h"

using namespace ZQ::MPF::utils;

SRM_BEGIN

NodeQuery::NodeQuery(NodeManager& nm)
:m_nm(nm), RpcServerMethod(QUERY_METHOD)
{
}

void NodeQuery::execute(rpc::RpcValue& params, rpc::RpcValue& result)
{
	m_nm.OnQuery(params, result);
}

void NodeManager::execute(rpc::RpcValue& params, rpc::RpcValue& result)
{
	MPFLog(MPFLogHandler::L_INFO, "[NodeManager::execute]\treceived heartbeat");

	//params.print(print_screen);

	/*
	char strTemp[2048] = {0};
	params.toXml(strTemp, 2048);
	printf(":::%s\n", strTemp);
	*/

	HeartbeatInfo hi;
	char worknodeURL[MAX_URL_LEN] = {0};
	if (!params[0][WORKNODE_ID_KEY].toString(worknodeURL, MAX_URL_LEN))
	{
		result.SetStruct(ERROR_CODE_KEY, rpc::RpcValue(1));
		result.SetStruct(COMMENT_KEY, rpc::RpcValue("can not find worknode URL in parameters"));
	}
	utils::URLStr nodeurl(worknodeURL);
	hi.worknode	= nodeurl.getVar(URL_VARNAME_WORKNODE_ID);
	hi.value	= params[0];

	m_buffer.push(hi);

	MetaNode curnode(nodeurl.getVar(URL_VARNAME_WORKNODE_ID), 0);
	size_t zLeaseTerm = max(curnode.get(LEASE_TERM_KEY), 5000);

	result.SetStruct(ERROR_CODE_KEY, rpc::RpcValue(RPC_ERROR_SUCCESS));
	result.SetStruct(COMMENT_KEY, rpc::RpcValue(RPC_ERROR_COMMENT_SUCCESS));
	result.SetStruct(LEASE_TERM_KEY, rpc::RpcValue((int)zLeaseTerm));
}

char* NodeManager::help(char* strBuffer, int nMax)
{
	assert(strBuffer);

	sfstrncpy(strBuffer, "receive heartbeat from work node", nMax);
	return strBuffer;
}

int NodeManager::run()
{
	while(ZQ::common::NativeThread::RUNNING == getStatus())
	{
		try
		{
			HeartbeatInfo hi;
			while (m_buffer.pop(hi))
			{
				MetaNode worknode(hi.worknode.c_str());
				
				if (rpc::RpcValue::TypeStruct != hi.value.getType())
					continue;

				char strKey[MAX_RPC_KEY_STR_LEN] = {0};
				char strValue[MAX_RPC_VALUE_STR_LEN] = {0};
				if (!hi.value.listStruct(strKey, MAX_RPC_KEY_STR_LEN, true))
					continue;

				do 
				{
					rpc::RpcValue temp = hi.value[strKey];
					if (rpc::RpcValue::TypeString == temp.getType())
					{
						hi.value[strKey].toString(strValue, MAX_RPC_VALUE_STR_LEN);
						worknode.set(strKey, strValue);
					}
					else if (rpc::RpcValue::TypeStruct == temp.getType())
					{
					}
					else
					{
						//skip;
					}
				} while(hi.value.listStruct(strKey, MAX_RPC_KEY_STR_LEN));

				///* heartbeat 
				char strNodeURL[MAX_URL_LEN]		= {0};
				hi.value[WORKNODE_ID_KEY].toString(strNodeURL, MAX_URL_LEN);

				utils::URLStr urlNode(strNodeURL);

				rpc::RpcValue vlWorkNodeInfo		= hi.value[WORKNODE_INFO_KEY];
				
				int nTotalMemory					= vlWorkNodeInfo[TOTAL_MEMORY_KEY];
				int nCPUUsage						= __min((int)vlWorkNodeInfo[CPU_USAGE_KEY], 100);
				int nMemoryUsage					= __min((int)vlWorkNodeInfo[MEMORY_USAGE_KEY], nTotalMemory);
				double lfCurrentTraffic				= vlWorkNodeInfo[CURRENT_TRAFFIC_KEY];
				int nBandWidth						= vlWorkNodeInfo[BAND_WIDTH_KEY];

				char strCPU[MAX_CPU_STR_LEN]		= {0};
				char strMpfVersion[MPF_VERSION_LEN]	= {0};
				char strOSVersion[MPF_OS_LEN]		= {0};
				vlWorkNodeInfo[CPU_KEY].toString(strCPU, MAX_CPU_STR_LEN);
				vlWorkNodeInfo[MPF_VERSION_KEY].toString(strMpfVersion, MPF_VERSION_LEN);
				vlWorkNodeInfo[OS_KEY].toString(strOSVersion, MPF_OS_LEN);

				const char* strNodeId				= urlNode.getVar(URL_VARNAME_WORKNODE_ID);
				const char* strNodeIP				= urlNode.getHost();
				int nNodePort						= urlNode.getPort();

				//MetaNode worknode(strNodeId);
				worknode.set(CPU_KEY, strCPU);
				worknode.set(OS_KEY, strOSVersion);
				worknode.set(MPF_VERSION_KEY, strMpfVersion);
				worknode.set(WORKNODE_IP_KEY, strNodeIP);
				worknode.set(WORKNODE_PORT_KEY, nNodePort);
				worknode.set(TOTAL_MEMORY_KEY, nTotalMemory);
				worknode.set(CPU_USAGE_KEY, nCPUUsage);
				worknode.set(MEMORY_USAGE_KEY, nMemoryUsage);
				worknode.set(CURRENT_TRAFFIC_KEY, (size_t)lfCurrentTraffic);
				worknode.set(BAND_WIDTH_KEY, nBandWidth);

				rpc::RpcValue vlTaskTypes			= hi.value[TASK_TYPES_KEY];
				if (vlTaskTypes.valid())
				{
					size_t zTaskTypeCount = vlTaskTypes.size();
					for (size_t i = 0; (common::NativeThread::RUNNING==getStatus())&&i<zTaskTypeCount; ++i)
					{
						char strTaskTypeName[MPF_TASKTYPE_LEN]	= {0};
						if (NULL == vlTaskTypes[i][TYPE_NAME_KEY].toString(strTaskTypeName, MPF_TASKTYPE_LEN))
							continue;

						int nRunningInstanceCount				= vlTaskTypes[i][RUN_INSTANCE_KEY];
						int nAvailableInstanceCount				= vlTaskTypes[i][AVBLE_INSTANCE_KEY];

						MetaRecord tasktype(utils::NodePath::getSubPath(worknode.getEntry(), strTaskTypeName).c_str(),
							RM_PROP_IMMEDIATELY_FLUSH);

						tasktype.set(RUN_INSTANCE_KEY, nRunningInstanceCount);
						tasktype.set(AVBLE_INSTANCE_KEY, nAvailableInstanceCount);
					}
				}
			}
		}
		catch(...)
		{
		}
		Sleep(DEF_HEARTBEAT_SLEEP_TIME);
	}
	return 0;
}

BYTE NodeManager::score(const char* resourceentry, const char* sessionentry)
{
	if (NULL == resourceentry)
	{
		assert(false);
		return SCORE_0;
	}
	
	if (NULL == sessionentry)
	{
		assert(false);
		return SCORE_0;
	}

	try
	{
		MetaRecord node(resourceentry, PM_PROP_READ_ONLY|RM_PROP_IMMEDIATELY_FLUSH);

		size_t zLastUpdate		= node.get(LAST_UPDATE);
		size_t zLeaseTerm		= node.get(LEASE_TERM_KEY);
		size_t zCPUUsage		= node.get(CPU_USAGE_KEY);
		size_t zMemoryTotal		= node.get(TOTAL_MEMORY_KEY);
		size_t zMemoryUsage		= node.get(MEMORY_USAGE_KEY);
		size_t zCurrentTraffic	= node.get(CURRENT_TRAFFIC_KEY);

		time_t curtime;
		time(&curtime);
// remove by Jie 2005-08-01, zLastUpdate is 0 sometime
//		if ((curtime < zLastUpdate)||((curtime-zLastUpdate)*1000>zLeaseTerm))
//			return SCORE_0;
		
		int nCPUPercent = 100 - zCPUUsage;
		int nMemFree = zMemoryTotal - zMemoryUsage;
		int nScore = 1 + __min((nCPUPercent), (nMemFree/1024/1024/32));
		BYTE byScore = (nScore > 0xff) ? 0xff : nScore&0xff;
		if (0 == byScore)
			return SCORE_0;

		MetaRecord session(sessionentry, PM_PROP_READ_ONLY|RM_PROP_IMMEDIATELY_FLUSH);

		char strTaskType[MPF_TASKTYPE_LEN] = {0};
		if (NULL == session.get(SESS_PARAM_TASKTYPE, strTaskType, MPF_TASKTYPE_LEN))
			return SCORE_0;
		size_t zAtleastTraffice	= session.get(SESS_PARAM_ATLEAST_TRAFFIC);
		if (zAtleastTraffice >= zCurrentTraffic)
			return SCORE_0;

		size_t count;
		char** pstrEntry = node.listChildren(count);

		BYTE result = SCORE_0;
		for (int i = 0; i < count; ++i)
		{
			if (utils::NodePath::getPureName(pstrEntry[i]) == strTaskType)
			{
				result = byScore;
				break;
			}
		}

		if (pstrEntry)
			node.deleteList(pstrEntry, count);	

		return result;
	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_ERROR, "[NodeManager::score]\tUnhandled exception");
		return SCORE_0;
	}
}

NodeManager::NodeManager(rpc::RpcServer& server, bool book, int leaseTerm, unsigned int property)
:RpcServerMethod(HEARTBEAT_METHOD),ResourceManager(DB_RESOURCE_NODE, leaseTerm, book), m_server(server), m_nq(*this)
{
	MPFLog(MPFLogHandler::L_NOTICE, "[NodeManager::NodeManager]\tcreate node manager");
}


bool NodeManager::begin()
{	
	MPFLog(MPFLogHandler::L_NOTICE, "[NodeManager::NodeManager]\trun node manager heartbeat receiver");

	m_server.addMethod(this);
	m_server.addMethod(&m_nq);
	return start();
}

MetaNode::MetaNode(const NodeManager& nm, const char* nodeid, unsigned int property, size_t leaseterm)
:MetaResource(nm, nodeid, RM_PROP_IMMEDIATELY_FLUSH|PM_PROP_LAST_UPDATE_TIME|property)
{
	if (!isReadOnly())
	{
		if (-1 == get(LEASE_TERM_KEY))
			set(LEASE_TERM_KEY, leaseterm);
	}
}

MetaNode::MetaNode(const char* nodeid, unsigned int property, size_t leaseterm, const char* rootentry)
:MetaResource(rootentry, nodeid, RM_PROP_IMMEDIATELY_FLUSH|PM_PROP_LAST_UPDATE_TIME|property)
{
	if (!isReadOnly())
	{
		if (-1 == get(LEASE_TERM_KEY) || 0 == get(LEASE_TERM_KEY))
			set(LEASE_TERM_KEY, leaseterm);
	}
}


SRM_END
