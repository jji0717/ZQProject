
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
// Name  : TaskUpdateSubscriber.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-8
// Desc  : subscriber
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/SRMSubscriber.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 59    05-07-29 18:26 Jie.zhang
// 
// 58    05-07-15 14:15 Daniel.wang
// 
// 57    05-07-15 14:12 Daniel.wang
// 
// 56    05-07-15 11:51 Daniel.wang
// 
// 55    05-06-28 5:08p Daniel.wang
// 
// 54    05-06-24 5:11p Daniel.wang
// 
// 53    05-06-21 12:19p Daniel.wang
// 
// 52    05-06-17 1:57p Daniel.wang
// 
// 51    05-06-14 4:58p Daniel.wang
// 9     05-04-15 11:31 Daniel.wang
// change the arraies to structures
// ===========================================================================

#include "SRMSubscriber.h"
#include "SRMDaemon.h"
#include "MetaNode.h"
#include "MetaSession.h"
#include "../MPFException.h"


SRM_BEGIN

long TaskUpdateSubscriber::OnTaskInit(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result)
{
	MPFLog(MPFLogHandler::L_WARNING, "[TaskUpdateSubscriber::OnTaskInit]\tuse default task init process");
	return HANDLED_NEXT;
}

long TaskUpdateSubscriber::OnTaskProgress(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result)
{
	MPFLog(MPFLogHandler::L_WARNING, "[TaskUpdateSubscriber::OnTaskProgress]\tuse default task progres process");
	return HANDLED_NEXT;
}

long TaskUpdateSubscriber::OnTaskFinal(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result)
{
	MPFLog(MPFLogHandler::L_WARNING, "[TaskUpdateSubscriber::OnTaskFinal]\tuse default task final process");
	return HANDLED_NEXT;
}

long TaskUpdateSubscriber::OnUpdateSession(const char* strTaskType, const char* strActionId, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result)
{
	long userState = OnTaskUser(strTaskType, strActionId, strWorkNodeId, strTaskId, params, result);
	if (userState >= SUCC)
	{
		return userState;
	}

	if (0 == stricmp(TASK_INIT_ACTION, strActionId))
	{
		MPFLog(MPFLogHandler::L_INFO, "[TaskUpdateSubscriber::OnUpdateSession]\tinitlize task: %s.%s", strWorkNodeId, strTaskId);
		if (SUCC == OnTaskInit(strTaskType, strWorkNodeId, strTaskId, params, result))
			m_state = TS_INIT;
	}
	else if (0 == stricmp(TASK_PROGRESS_ACTION, strActionId))
	{
		MPFLog(MPFLogHandler::L_INFO, "[TaskUpdateSubscriber::OnUpdateSession]\tin process task: %s.%s", strWorkNodeId, strTaskId);
		if (SUCC == OnTaskProgress(strTaskType, strWorkNodeId, strTaskId, params, result))
			m_state = TS_INPROGRESS;
	}
	else if (0 == stricmp(TASK_FINAL_ACTION, strActionId))
	{
		MPFLog(MPFLogHandler::L_INFO, "[TaskUpdateSubscriber::OnUpdateSession]\tend task: %s.%s", strWorkNodeId, strTaskId);
		if (SUCC == OnTaskFinal(strTaskType, strWorkNodeId, strTaskId, params, result))
			m_state = TS_FINAL;
	}

	return NOT_HANDLED;
}

long TaskUpdateSubscriber::OnTaskUser(const char* strTaskType, const char* strActionId, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result)
{
	return NOT_HANDLED;
}

TaskUpdateSubscriber::TaskUpdateSubscriber(TaskSubScriberStack& stack, type_e type)
:m_state(TS_CREATED), m_refStack(stack), _type(type)
{
}

TaskUpdateSubscriber::~TaskUpdateSubscriber()
{
}

TaskUpdateSubscriber::TaskState TaskUpdateSubscriber::getState()
{
	return m_state;
}

bool TaskUpdateSubscriber::isFinal()
{
	return m_state == TS_FINAL;
}

bool TaskUpdateSubscriber::open()
{
	return m_refStack.reg(this);
}

bool TaskUpdateSubscriber::close()
{	
	return m_refStack.unreg(this);
}


ZQ::common::Mutex TaskSubScriberStack::s_mutex;

TaskSubScriberStack::TaskSubScriberStack(rpc::RpcServer& server)
:rpc::RpcServerMethod(UPDATESESSION_METHOD, 0)
{
	MPFLog(MPFLogHandler::L_NOTICE, "[TaskSubScriberStack::TaskSubScriberStack]\tcreate subscriber stack");
	m_arrSubscriber.clear();

	server.addMethod(this);
}

TaskSubScriberStack::~TaskSubScriberStack()
{
	MPFLog(MPFLogHandler::L_NOTICE, "[TaskSubScriberStack::~TaskSubScriberStack]\tdelete subscriber stack");
}

void TaskSubScriberStack::execute(rpc::RpcValue& params, rpc::RpcValue& result)
{
	MPFLog(MPFLogHandler::L_INFO, "[TaskSubScriberStack::execute]\treceived update session");
	rpc::RpcValue uwResult;
	try
	{
		char strActionId[MPF_UDSESS_ACTION_LEN] = {0};
		char strWorkNodeId[MPF_NODEID_LEN] = {0};
		char strTaskId[MPF_TASKID_LEN] = {0};
		char strTaskType[MAX_TASK_TYPE_LEN] = {0};

		params[0][ACTION_ID_KEY].toString(strActionId, MPF_UDSESS_ACTION_LEN);
		params[0][WORKNODE_ID_KEY].toString(strWorkNodeId, MPF_NODEID_LEN);
		params[0][TASK_ID_KEY].toString(strTaskId, MPF_TASKID_LEN);
		params[0][TASK_TYPE_KEY].toString(strTaskType, MPF_TASKTYPE_LEN);

		char strFullTaskId[MPF_FULL_TASKID_LEN] = {0};
		_snprintf(strFullTaskId, MPF_FULL_TASKID_LEN-1, "%s.%s", strWorkNodeId, strTaskId);
		MetaTask mt(strFullTaskId, PM_PROP_LAST_UPDATE_TIME);

		/*
		if (NULL == mt.get(INFO_TYPENAME_KEY, strTaskType, MAX_TASK_TYPE_LEN))
		{
			throw SRMException("[TaskSubScriberStack::execute] can not get task type");
		}
		*/

		if (0 == stricmp(TASK_INIT_ACTION, strActionId))
		{
			mt.set(TASK_STATUS_KEY, TASK_STATUS_SETUP);
		}
		else if (0 == stricmp(TASK_PROGRESS_ACTION, strActionId))
			mt.set(TASK_STATUS_KEY, TASK_STATUS_PROCESS);
		else if (0 == stricmp(TASK_FINAL_ACTION, strActionId))
			mt.set(TASK_STATUS_KEY, TASK_STATUS_FINAL);

		long ret = (long)TaskUpdateSubscriber::NOT_HANDLED;

		ZQ::common::Guard<ZQ::common::Mutex> guard(s_mutex);
		for (std::vector<TaskUpdateSubscriber*>::iterator it = m_arrSubscriber.begin();
			it < m_arrSubscriber.end(); ++it)
		{
			int lastret = ret;
			if (NULL == *it)
			{
				continue;
			}

			ret	= (*it)->OnUpdateSession(strTaskType, strActionId, strWorkNodeId, strTaskId, params, result);

			if (ret>=0&&(*it)->_type < TaskUpdateSubscriber::TST_EXTENSION)
			{
				//todo: log here
				MPFLog(MPFLogHandler::L_WARNING, "[TaskSubScriberStack::execute]\treceived wrong code from filter subscriber: %d, adjust it", ret);
				ret = TaskUpdateSubscriber::HANDLED_NEXT;
			}

			//if ((*it)->_type >= TaskUpdateSubscriber::TST_EXTENSION)
			//	*it = NULL;

			ret = max(ret, lastret);

			if (ret >= TaskUpdateSubscriber::SUCC)
			{
				// TODO: log here
				MPFLog(MPFLogHandler::L_NOTICE, "[TaskSubScriberStack::execute]\tleave subscriber process cycle");
				break;
			}
		}
	}
	catch(SRMException& e)
	{
		MPFLog(MPFLogHandler::L_ERROR, "[TaskSubScriberStack::execute]\tUpdateSessionMethod::execute : Handled exception with %s", e.what());

		uwResult.SetStruct(ERROR_CODE_KEY, rpc::RpcValue(RPC_ERROR_EXCEPTION));
		uwResult.SetStruct(COMMENT_KEY, rpc::RpcValue("get an SRM exception on update session"));
		
		result[0] = uwResult;
		return;
	}
	catch(std::exception& e)
	{
		MPFLog(MPFLogHandler::L_ERROR, "[TaskSubScriberStack::execute]\tUpdateSessionMethod::execute : Handled exception with %s", e.what());

		uwResult.SetStruct(ERROR_CODE_KEY, rpc::RpcValue(RPC_ERROR_EXCEPTION));
		uwResult.SetStruct(COMMENT_KEY, rpc::RpcValue("get an standard exception on update session"));
		
		result[0] = uwResult;
		return;
	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_ERROR, "[TaskSubScriberStack::execute]\tUpdateSessionMethod::execute : Unhandled exception");

		uwResult.SetStruct(ERROR_CODE_KEY, rpc::RpcValue(RPC_ERROR_EXCEPTION));
		uwResult.SetStruct(COMMENT_KEY, rpc::RpcValue("get an standard exception on update session"));
		
		result[0] = uwResult;
		return;
	}
}

char* TaskSubScriberStack::help(char* strBuffer, int nMax)
{
	assert(strBuffer);

	_snprintf(strBuffer, nMax-1, "receive update session method from work node");
	return strBuffer;
}

bool TaskSubScriberStack::reg(TaskUpdateSubscriber* pSubscriber)
{
	assert(pSubscriber);

	MPFLog(MPFLogHandler::L_INFO, "[TaskSubScriberStack::reg]\tregister subscriber");

	ZQ::common::Guard<ZQ::common::Mutex> guard(s_mutex);

	for (std::vector<TaskUpdateSubscriber*>::iterator itor = m_arrSubscriber.begin(); itor< m_arrSubscriber.end(); ++itor)
	{
		if (*itor == pSubscriber)
			return true;

		if (*itor ==NULL)
			continue;

		if ((*itor)->_type > pSubscriber->_type)
			break;
	}

	if (itor == m_arrSubscriber.end())
		m_arrSubscriber.insert(itor, pSubscriber);
	else if (*itor != pSubscriber)
		m_arrSubscriber.insert(itor, pSubscriber);

	return true;
}

bool TaskSubScriberStack::unreg(TaskUpdateSubscriber* pSubscriber)
{		   	
	assert(pSubscriber);

	MPFLog(MPFLogHandler::L_INFO, "[TaskSubScriberStack::unreg]\tun-register subscriber");

	ZQ::common::Guard<ZQ::common::Mutex> guard(s_mutex);

	std::vector<TaskUpdateSubscriber*>::iterator itor = std::find(m_arrSubscriber.begin(), m_arrSubscriber.end(), pSubscriber);
	if (m_arrSubscriber.end() == itor)
		return false;
	m_arrSubscriber.erase(itor);
	return true;
}

SRM_END


