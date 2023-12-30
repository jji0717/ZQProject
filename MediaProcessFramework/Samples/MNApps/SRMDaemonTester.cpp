
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
// Name  : SRMDaemonTester.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-13
// Desc  : daemon tester
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/Samples/MNApps/SRMDaemonTester.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 23    05-05-10 15:57 Daniel.wang
// 
// 22    05-04-29 20:11 Daniel.wang
// 
// 21    05-04-28 22:40 Daniel.wang
// 
// 20    05-04-28 18:04 Daniel.wang
// 
// 19    05-04-28 15:33 Daniel.wang
// 
// 18    05-04-28 11:49 Daniel.wang
// 
// 17    05-04-27 21:15 Daniel.wang
// 
// 16    05-04-27 17:21 Daniel.wang
// 
// 15    05-04-27 16:54 Daniel.wang
// 
// 14    05-04-22 15:52 Daniel.wang
// 
// 13    05-04-22 15:47 Daniel.wang
// 
// 12    05-04-22 11:38 Daniel.wang
// 
// 11    05-04-21 20:29 Daniel.wang
// 
// 10    05-04-21 11:39 Daniel.wang
// 
// 9     05-04-20 16:40 Daniel.wang
// 
// 8     05-04-20 11:44 Daniel.wang
// 
// 6     05-04-15 22:15 Daniel.wang
// 
// 5     05-04-15 20:41 Daniel.wang
// 
// 4     05-04-15 17:56 Daniel.wang
// 
// 3     05-04-14 21:51 Daniel.wang
// 
// 2     05-04-13 17:19 Daniel.wang
// 
// 1     05-04-13 11:36 Daniel.wang
// ===========================================================================

#include "../../srmapi/SRMDaemon.h"
#include "../../mpfException.h"
#include "../../srmapi/SRMSubscriber.h"
#include "../../srmapi/SRMSetting.h"
#include "../../../common/Log.h"

using namespace ZQ::MPF::SRM;
using namespace ZQ::rpc;

//define local address
#define LOCAL_ADDR_IP "192.168.80.74"
#define LOCAL_ADDR_PORT 12000

//there are two task type for work
#define TASK_NET_COPY_FILE "netCopyFile"
#define TASK_LOCAL_COPY_FILE "localCopyFile"

int g_nInstance = 0;

//create task state subscriber for netCopyFile task
class NetCopyFileSubscriber : public TaskStateSubscriber
{
public:
	NetCopyFileSubscriber(MetaResource& pNode, const char* strSessionId)
		:TaskStateSubscriber(strSessionId, pNode, TASK_NET_COPY_FILE, LOCAL_ADDR_IP, LOCAL_ADDR_PORT)
	{
	}

	//while the task is setup in work node application
	long OnTaskSetup(const char* strRequestId, RpcValue& params, RpcValue& expAttr, RpcValue& result)
	{
		//printf("NetCopyFileSubscriber::OnTaskSetup		received task setup method for request id: %s\n", strRequestId);

		char strTemp[256] = {0};
		if (NULL == getRequestId(strTemp, 256))
		{
			printf("NetCopyFileSubscriber::OnTaskSetup		request id is empty in netCopyFile task\n");
			return TaskStateSubscriber::ERR;
		}
		//printf("NetCopyFileSubscriber::OnTaskSetup		netCopyFile request id: %s\n", strTemp);

		//compare request ids from local and work node application
		if (0 != stricmp(strTemp, strRequestId))
		{
			//skip 
			return TaskStateSubscriber::HANDLE_NEXT;
		}

		//printf("NetCopyFileSubscriber::OnTaskSetup		netCopyFile task setup\n");

		result.SetStruct("netCopyFile.SourceFile", "d:\\test\\king.exe");
		result.SetStruct("netCopyFile.DestFile", "d:\\test\\dest\\king.exe");

		//set subscriber state
		setState(TaskStateSubscriber::TS_TASKSETUP);

		return TaskStateSubscriber::SUCC;
	}

	//while task is ready in work node application
	long OnTaskReady(const char* strWorkNodeId, const char* strTaskId, RpcValue& params, RpcValue& result)
	{
		//printf("NetCopyFileSubscriber::OnTaskReady		received task setup method from %s : %s\n", strWorkNodeId, strTaskId);

		char strCurrentWorkNodeId[256] = {0};
		if (NULL == getWorkNodeId(strCurrentWorkNodeId, 256))
		{
			printf("NetCopyFileSubscriber::OnTaskReady		netCopyFile task WorkNode Id is empty\n");
			return TaskStateSubscriber::ERR;
		}
		char strCurrentTaskId[256] = {0};
		if (NULL == getTaskId(strCurrentTaskId, 256))
		{
			printf("NetCopyFileSubscriber::OnTaskReady		netCopyFile task task Id is empty\n");
			return TaskStateSubscriber::ERR;
		}
		//printf("NetCopyFileSubscriber::OnTaskReady		wait for netCopyFile task by work node : %s : %s\n", strCurrentWorkNodeId, strCurrentTaskId);
		
		//compare work node id and work node id from local and work node application
		if (0 != stricmp(strCurrentWorkNodeId, strWorkNodeId) || 0 != stricmp(strCurrentTaskId, strTaskId))
		{
			//skip
			return TaskStateSubscriber::HANDLE_NEXT;
		}

		//printf("NetCopyFileSubscriber::OnTaskReady		netCopyFile task ready\n");

		//set subscriber state
		setState(TaskStateSubscriber::TS_TASKREADY);

		return TaskStateSubscriber::SUCC;
	}

	//task in process
	long OnTaskProgress(const char* strWorkNodeId, const char* strTaskId, RpcValue& params, RpcValue& result)
	{
		//printf("NetCopyFileSubscriber::OnTaskProgress		received task progress method from %s : %s\n", strWorkNodeId, strTaskId);

		char strCurrentWorkNodeId[256] = {0};
		if (NULL == getWorkNodeId(strCurrentWorkNodeId, 256))
		{
			printf("NetCopyFileSubscriber::OnTaskProgress		netCopyFile task WorkNode Id is empty\n");
			return TaskStateSubscriber::ERR;
		}
		char strCurrentTaskId[256] = {0};
		if (NULL == getTaskId(strCurrentTaskId, 256))
		{
			printf("NetCopyFileSubscriber::OnTaskProgress		netCopyFile task task Id is empty\n");
			return TaskStateSubscriber::ERR;
		}
		//printf("NetCopyFileSubscriber::OnTaskProgress		wait for netCopyFile task by work node : %s : %s\n", strCurrentWorkNodeId, strCurrentTaskId);
		
		//compare work node id and work node id from local and work node application
		if (0 != stricmp(strCurrentWorkNodeId, strWorkNodeId) || 0 != stricmp(strCurrentTaskId, strTaskId))
		{
			return TaskStateSubscriber::HANDLE_NEXT;
		}

		//printf("NetCopyFileSubscriber::OnTaskSetup		netCopyFile task progress\n");

		int nProg = params;

		//set subscriber state
		setState(TaskStateSubscriber::TS_ONPROCESS);

		return TaskStateSubscriber::SUCC;
	}
	
	//while the task is end
	long OnTaskFinal(const char* strWorkNodeId, const char* strTaskId, RpcValue& params, RpcValue& result)
	{
		--g_nInstance;

		printf("NetCopyFileSubscriber::OnTaskFinal		received task final method from %s : %s\n", strWorkNodeId, strTaskId);

		char strCurrentWorkNodeId[256] = {0};
		if (NULL == getWorkNodeId(strCurrentWorkNodeId, 256))
		{
			printf("NetCopyFileSubscriber::OnTaskFinal		netCopyFile task WorkNode Id is empty\n");
			return TaskStateSubscriber::ERR;
		}
		char strCurrentTaskId[256] = {0};
		if (NULL == getTaskId(strCurrentTaskId, 256))
		{
			printf("NetCopyFileSubscriber::OnTaskFinal		netCopyFile task task Id is empty\n");
			return TaskStateSubscriber::ERR;
		}
		printf("NetCopyFileSubscriber::OnTaskFinal		wait for netCopyFile task by work node : %s : %s\n", strCurrentWorkNodeId, strCurrentTaskId);
		
		//compare work node id and work node id from local and work node application
		if (0 != stricmp(strCurrentWorkNodeId, strWorkNodeId) || 0 != stricmp(strCurrentTaskId, strTaskId))
		{
			return TaskStateSubscriber::HANDLE_NEXT;
		}

		printf("NetCopyFileSubscriber::OnTaskSetup		netCopyFile task final\n");
		
		//set subscriber state
		setState(TaskStateSubscriber::TS_FINAL);

		return TaskStateSubscriber::SUCC;
	}
};

class MyNodeManager : public NodeManager
{
private:
public:
	MyNodeManager(ZQ::rpc::RpcServer& server)
		:NodeManager(server)
	{
	}

protected:
	BYTE score(MetaResource& dbrts, MetaSession& sess)
	{
		unsigned long nDistance = getDistance(dbrts);//second
		if (nDistance > 30)
		{	//more than 30 second
			return 0;
		}
		
		bool bGetEntry = false;

		MetaRecord firstSub, mrTemp;
		if (dbrts.firstChild(firstSub))
		{
			while (firstSub.nextSibling(mrTemp))
			{
				std::string strAcceptor = ZQ::MPF::utils::NodePath::getPureName(mrTemp.getEntry());
				if (0 == stricmp(strAcceptor.c_str(), "netCopyFile"))
				{
					bGetEntry = true;
					break;
				}
			}
		}

		if (!bGetEntry)
			return 0;
		
		std::string bandWidth = dbrts.get(BAND_WIDTH_KEY);
		
		int nCPUPercent = 100 - atoi(dbrts.get(CPU_USAGE_KEY).c_str());
		int nMemTotal = atoi(dbrts.get(TOTAL_MEMORY_KEY).c_str());
		int nMemUsage = atoi(dbrts.get(MEMORY_USAGE_KEY).c_str());
		int nMemFree = nMemTotal - nMemUsage;
		
		int nScore = 1 + __min((nCPUPercent/10), (nMemFree/1024/1024/32));
		
		return (nScore > 0xff) ? 0xff : nScore&0xff;
	}
};

void main()
{
	try
	{
		ZQ::common::DebugMsg dbgMsg;
		ZQ::common::setGlogger(&dbgMsg);

		printf("Daemon Tester\nv0.3\n");

		//set lease term number
		Setting srmsetting;
		RpcValue leaseTerm = 5000;
		srmsetting.set(LEASE_TERM, leaseTerm);

		//Daemon server
		SRMDaemon sd(LOCAL_ADDR_IP, LOCAL_ADDR_PORT);

		//node record manager, also create heartbeat receiver
		MyNodeManager nm(sd);

		//TaskStateManager receive UpdateSession method
		TaskStateManager tsm(sd);

		//run daemon server
		if (!sd.start())
		{
			printf("Can not start daemon server, Quit!\n");
			return;
		}

		char strInput[256] = {0};
		char strTemp[256] = {0};

		for (int i = 0; i < 2000; ++i)
		{
			//std::string strSessionId = ProcuceExclusiveString();
			//Session record
			char strTemp[10] = {0};
			itoa(i, strTemp, 10);
			MetaSession ms((strSessionId+strTemp).c_str(), true);

			if (NULL == nm.allocate(ms, strInput, 256))
			{
				continue;
			}

			MetaNode mr(strInput);
			TaskStateSubscriber* nc = new NetCopyFileSubscriber(mr, strSessionId.c_str());
			tsm.push_back(nc);
			if (!nc->taskRequest(strTemp, 256))
			{
				printf("Can not send task request to work node\n");
			}
			++g_nInstance;
		}

		while(0 != g_nInstance)
			;
	}
	catch(ZQ::MPF::SRMException& e)
	{
		printf("!!!get a srm exception: %s\n", e.what());
	}
	catch(std::exception e)
	{
		printf("!!!get a standard exception: %s\n", e.what());
	}
	catch(...)
	{
		printf("!!!get an unknown exception\n");
	}
}
