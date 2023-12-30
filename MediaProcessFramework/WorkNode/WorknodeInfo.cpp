// WorknodeInfo.cpp: implementation of the WorknodeInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WorknodeInfo.h"
#include "TaskAcceptor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
MPF_WORKNODE_NAMESPACE_BEGIN
WorknodeInfo::WorknodeInfo(TaskAcceptor& acpt, ZQ::rpc::RpcServer& server, const char* strInterface)
	:ListInfoMethod(server, strInterface,NODE_TYPE_WORKNODE), _acceptor(acpt)
{

}

WorknodeInfo::~WorknodeInfo()
{

}

void WorknodeInfo::OnGetInfo(const char* strInfoType, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result)
{
	if(0 == strcmp(strInfoType, INFOTYPE_TASK_TYPE))
	{
		_acceptor.listWorkTypes(result);
	}
	else if(0 == strcmp(strInfoType, INFOTYPE_TASK))
	{
		_acceptor.listTaskInstances(result);
	}
	else if(0 == strcmp(strInfoType, INFOTYPE_TASKDETAIL))
	{
		char buff[32]={0};
		params[INFO_TASKID_KEY].toString(buff, 32);

		_acceptor.listTaskDetails(buff, result);
		if(result.getType()!=RpcValue::TypeStruct)
		{
			RpcValue in_ret = result;
			result.clear();
			result.SetStruct(NON_STRUCTURE, in_ret);
		}
	}
}
MPF_WORKNODE_NAMESPACE_END