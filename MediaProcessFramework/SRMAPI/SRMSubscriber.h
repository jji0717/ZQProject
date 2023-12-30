
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
// Name  : TaskStateSubscriber.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-8
// Desc  : subscriber
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/SRMSubscriber.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 35    05-07-15 14:15 Daniel.wang
// 
// 34    05-07-15 11:51 Daniel.wang
// 
// 33    05-06-28 3:05p Daniel.wang
// 
// 32    05-06-17 1:57p Daniel.wang
// 
// 31    05-06-14 7:00p Daniel.wang
// 
// 30    05-06-14 4:58p Daniel.wang
// ===========================================================================


#ifndef _ZQ_SRMSUBSCRIBER_H_
#define _ZQ_SRMSUBSCRIBER_H_

#include "MetaSession.h"
#include "MetaTask.h"

SRM_BEGIN

// -----------------------------
// TaskStateSubscriber
// -----------------------------
/// TaskStateSubscriber is the interface between Manage node application and Work node application.
class DLL_PORT TaskUpdateSubscriber
{
	friend class TaskSubScriberStack;
public:
	enum TaskState
	{
		TS_CREATED,			//created
		TS_INIT,			//initlized
		TS_INPROGRESS,		//in process
		TS_FINAL,			//task final
		TS_USER =0x1000,	//user defined
	};

	enum
	{
		NOT_HANDLED = -10,	//had not processed
		HANDLED_NEXT = -5,	//for next process
		SUCC = 0,			//return success
		ERR  = 1,			//error 
		USER_ = 0x100		//user defined
	};

	typedef enum _type
	{
		TST_FILTER =0,
		TST_EXTENSION = 10
	} type_e;

protected:
	type_e _type;

	TaskState				m_state;
	TaskSubScriberStack&	m_refStack;

	/*
	std::string	m_strWorkNodeId;
	std::string	m_strTaskId;
	*/

	///OnTaskSetup\n
	///usage : inherit\n
	///OnTaskSetup will be called when the work node application need parameters for creating task work.\n
	///The work node application send the values for the work to manage node application in "params" parameter,\n
	///and the manage node application add values into "result" parameter for feed back to work node application.\n
	///it is wrong to return the number more than zero if filter
	///@param strWorkNodeId - work node id string
	///@param strTaskId - task id string
	///@param params - the parameters from work node application, it 's a structure value.
	///@param expAttr - the array of attributes to fill value
	///@param result - add data here feed back to work node application.
	///@return - return true if run ok
	virtual long OnTaskInit(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result);

	///OnTaskProgress\n
	///usage : inherit\n
	///OnTaskProgress will be called when the task being work for cycle.\n
	///Work node application can set progress step into "params" parameter,\n
	///and manager node application can received it. \n
	///it is wrong to return the number more than zero if filter
	///@param strWorkNodeId - work node id string
	///@param strTaskId - task id string
	///@param params - the parameters from work node application, it 's a structure value. 
	///@param result - add data here feed back to work node application.
	///@return - return true if run ok
	virtual long OnTaskProgress(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result);

	///OnTaskFinal\n
	///usage : inherit\n
	///OnTaskFinal will be called when the task is end\n
	///it is wrong to return the number more than zero if filter
	///@param strWorkNodeId - work node id string
	///@param strTaskId - task id string
	///@param params - the parameters from work node application, it 's a structure value. 
	///@param result - add data here feed back to work node application.
	///@return - return true if run ok
	virtual long OnTaskFinal(const char* strTaskType, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result);

	///OnTaskUser\n
	///usage : inherit\n
	///OnTaskUser will be called before other OnTask*** function\n
	///@param strActionId - task action id
	///@param strWorkNodeId - work node id string
	///@param strTaskId - task id string
	///@param params - the parameters from work node application, it 's a structure value. 
	///@param result - add data here feed back to work node application.
	///@return - return true if run ok
	virtual long OnTaskUser(const char* strTaskType, const char* strActionId, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result);
private:
	///OnUpdateSession\n
	///OnUpdateSession will call the [OnTaskInit/OnTaskProgress/OnTaskFinal] to complate mission,\n
	///you can not use OnUpdateSession or OnTaskInit/OnTaskProgress/OnTaskFinal same time
	///this function to get update session for tasks and dispense to other On-function s\n
	///it is wrong to return the number more than zero if filter
	///@param params - the parameters from work node application, it 's a structure value. 
	///@param result - add data here feed back to work node application.
	///@return - return true if run ok
	long OnUpdateSession(const char* strTaskType, const char* strActionId, const char* strWorkNodeId, const char* strTaskId, rpc::RpcValue& params, rpc::RpcValue& result);

public:
	///constructor
	///@param stack - subscribers manager
	///@param type - subscriber type
	TaskUpdateSubscriber(TaskSubScriberStack& stack, type_e type = TST_EXTENSION);
	
	///destructor
	virtual ~TaskUpdateSubscriber();

	///getState\n
	///get subscriber state\n
	///@return - return current state
	TaskState getState();

	///isFinal
	///@return - return true if the task is final
	bool isFinal();

	///begin subscriber manager  
	///@return ture if ok
	bool open();

	///end subscriber manager  
	///@return ture if ok
	bool close();
};



// -----------------------------
//UpdateSessionManager
// -----------------------------
/// update session receiver manager
class DLL_PORT TaskSubScriberStack: public rpc::RpcServerMethod
{
	friend class TaskUpdateSubscriber;

private:
	static ZQ::common::Mutex			s_mutex;
	
	//TaskManager&						m_taskManager;

	//todo: add lock code here
	std::vector<TaskUpdateSubscriber*>	m_arrSubscriber;

	bool reg(TaskUpdateSubscriber* pSubscriber);
	bool unreg(TaskUpdateSubscriber* pSubscriber);

protected:
	///call back function which called as update session method 
	///@param params - update session method parameters
	///@param result - update session method return
	virtual void execute(rpc::RpcValue& params, rpc::RpcValue& result);

	///help method of update session
	///@param strBuffer - help string buffer
	///@param nMax - help string buffer size
	char* help(char* strBuffer, int nMax);

public:
	///constructor
	///@param server - rpc server end 
	TaskSubScriberStack(rpc::RpcServer& server);

	///destructor
	~TaskSubScriberStack();
};

SRM_END

#endif//_ZQ_SRMSUBSCRIBER_H_
