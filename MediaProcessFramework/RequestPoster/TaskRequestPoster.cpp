
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
// Name  : TaskRequestPoster.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-5-11
// Desc  : task request poster
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/RequestPoster/TaskRequestPoster.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 10    05-06-20 2:40p Daniel.wang
// ===========================================================================

#include "TaskRequestPoster.h"
#include "mpfloghandler.h"

#include <assert.h>

REQPOST_BEGIN

TaskRequestPoster::TaskRequestPoster(const char* worknodeurl, const char* mgmnodeurl, 
	int responsTimeout)
:m_client(m_worknode.getHost(), m_worknode.getPort()), m_worknode(worknodeurl),
m_strMgmnode(mgmnodeurl)
{
	assert(worknodeurl);

	MPFLog(MPFLogHandler::L_NOTICE, "[TaskRequestPoster::TaskRequestPoster]\tget work node for task request: %s", worknodeurl);
	m_client.setResponseTimeout(responsTimeout);

	m_strWorkNodeId = m_worknode.getVar(URL_VARNAME_WORKNODE_ID);
}

TaskRequestPoster::~TaskRequestPoster()
{
}

int TaskRequestPoster::postSetup(const char* tasktype, rpc::RpcValue& result)
{
	rpc::RpcValue setParam;
	setParam.SetStruct(ACTION_ID_KEY, rpc::RpcValue(REQUEST_SETUP));
		rpc::RpcValue setupAttr;
		setupAttr.SetStruct(TASK_TYPE_KEY, rpc::RpcValue(tasktype));
		setupAttr.SetStruct(MGM_SESSION_URL_KEY, rpc::RpcValue(m_strMgmnode.c_str()));
	setParam.SetStruct(REQUEST_ATTR_KEY, setupAttr);

	if (!m_client.execute(TASKREQUEST_METHOD, setParam, result))
	{
		MPFLog(MPFLogHandler::L_ERROR, "[TaskRequestPoster::postSetup]\tcan not send task setup request");
		return POST_ERR_NETWORK;
	}
	MPFLog(MPFLogHandler::L_DEBUG, "[TaskRequestPoster::postSetup]\tsend task setup request");

	/*
	char temp[512] = {0};
	result.toXml(temp, 512);

	printf("task setup: %s\n", temp);
	*/

	char strTaskId[256] = {0};
	result[TASK_ID_KEY].toString(strTaskId, 256);
	m_strTaskId = strTaskId;

	rpc::RpcValue playParam,playResult;
	playParam.SetStruct(ACTION_ID_KEY, rpc::RpcValue(REQUEST_PLAY));
	rpc::RpcValue palyAttr;

	palyAttr.SetStruct(TASK_ID_KEY, rpc::RpcValue(m_strTaskId.c_str()));
	playParam.SetStruct(REQUEST_ATTR_KEY, palyAttr);

	if (!m_client.execute(TASKREQUEST_METHOD, playParam, playResult))
	{
		MPFLog(MPFLogHandler::L_ERROR, "[TaskRequestPoster::postSetup]\tcan not send task ready request");
		return POST_ERR_NETWORK;
	}
	MPFLog(MPFLogHandler::L_DEBUG, "[TaskRequestPoster::postSetup]\tsend task play request");

	/*
	char temp2[512] = {0};
	result.toXml(temp2, 512);
	printf("task play: %s\n", temp2);
	*/

	return POST_ERR_OK;
}

const char* TaskRequestPoster::getTaskId() const
{
	return m_strTaskId.c_str();
}


int TaskRequestPoster::postUser(const char* useractionid, const rpc::RpcValue& param, rpc::RpcValue& result)
{
	rpc::RpcValue usrParam;
	usrParam.setStruct(ACTION_ID_KEY, rpc::RpcValue(REQUEST_USER));
		rpc::RpcValue attr;
		attr.setStruct(TASK_ID_KEY, rpc::RpcValue(m_strTaskId.c_str()));
		attr.setStruct(USER_ACTION_ID_KEY, rpc::RpcValue(useractionid));
		attr.setStruct(USER_ATTR_KEY, param);
	usrParam.setStruct(REQUEST_ATTR_KEY, attr);
		
	if (!m_client.execute(TASKREQUEST_METHOD, attr, result))
		return POST_ERR_NETWORK;

	return POST_ERR_OK;
}

const char* TaskRequestPoster::getWorkNodeId() const
{
	return m_strWorkNodeId.c_str();
}


REQPOST_END
