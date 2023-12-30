
#include "srminfo.h"
#include "MetaTask.h"
#include "metaresource.h"

SRM_BEGIN

SRMInfo::SRMInfo(ZQ::rpc::RpcServer& server, const char* strInterface)
		:ZQ::MPF::ListInfoMethod(server, strInterface,NODE_TYPE_MANAGERNODE)
{
}

void SRMInfo::OnGetInfo(const char* strInfoType, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result)
{
	MPFLog(MPFLogHandler::L_INFO, "[SRMInfo::OnGetInfo]\treceived get info method");

	result.clear();

	if (0==strcmp(INFOTYPE_WORKNODE, strInfoType))
	{
		ResourceManager worknodemgr(DB_RESOURCE_NODE, 0);
		
		size_t count;
		char** pstrEntry = worknodemgr.listChildren(count);
		for (int i = 0; i < count; ++i)
		{
			rpc::RpcValue rvtemp;
			MetaRecord worknode(pstrEntry[i], PM_PROP_READ_ONLY);

			char strIp[MAX_IP_STR_LEN]				= {0};
			char strMPFVersion[MPF_VERSION_LEN]		= {0};
			char strCpu[MAX_CPU_STR_LEN]			= {0};
			char strMemory[MAX_INT32_STR_LEN]		= {0};
			char strOs[MAX_OS_STR_LEN]				= {0};
			char strHardware[MAX_HARDWARE_STR_LEN]	= {0};
			worknode.get(WORKNODE_IP_KEY, strIp, MAX_IP_STR_LEN);
			worknode.get(MPF_VERSION_KEY, strMPFVersion, MPF_VERSION_LEN);
			worknode.get(CPU_KEY, strCpu, MAX_CPU_STR_LEN);
			worknode.get(OS_KEY, strOs, MAX_OS_STR_LEN);
			size_t zPort		= worknode.get(INFO_PORT_KEY);
			size_t zLastUpdate	= worknode.get(LAST_UPDATE);
			size_t zLeaseTerm	= worknode.get(LEASE_TERM_KEY);
			size_t zTotalMemory	= worknode.get(TOTAL_MEMORY_KEY);
			_snprintf(strHardware, MAX_HARDWARE_STR_LEN-1, "CPU:%s Memory:%ld OS:%s",
				strCpu, strMemory, strOs);

			rvtemp.setStruct(INFO_NODEID_KEY, rpc::RpcValue(utils::NodePath::getPureName(pstrEntry[i]).c_str()));
			rvtemp.setStruct(INFO_IP_KEY, rpc::RpcValue(strIp));
			rvtemp.setStruct(INFO_PORT_KEY, rpc::RpcValue((int)zPort));
			rvtemp.setStruct(INFO_MPFVERSION_KEY, rpc::RpcValue(strMPFVersion));
			rvtemp.setStruct(INFO_LASTHB_KEY, rpc::RpcValue(getTimeStr(zLastUpdate).c_str()));
			rvtemp.setStruct(INFO_LEASETERM_KEY, rpc::RpcValue((int)zLeaseTerm));
			rvtemp.setStruct(INFO_HARDWARE_KEY, rpc::RpcValue(strHardware));

			result.setArray(i, rvtemp);
		}
	
		if (pstrEntry)
			worknodemgr.deleteList(pstrEntry, count);			
	}
	else if (0==strcmp(INFOTYPE_TASK, strInfoType))
	{
		TaskManager taskmgr(0);

		size_t count;
		char** pstrEntry = taskmgr.listChildren(count);
		for (int i = 0; i < count; ++i)
		{
			rpc::RpcValue rvtemp;
			MetaRecord task(pstrEntry[i], PM_PROP_READ_ONLY);

			char strTypeName[MPF_TASKTYPE_LEN]	= {0};
			char strTaskURL[MAX_URL_LEN]		= {0};
			char strSessionURL[MAX_URL_LEN]		= {0};
			char strStatus[MAX_TASK_STATUS_LEN]	= {0};
			task.get(INFO_TYPENAME_KEY, strTypeName, MPF_TASKTYPE_LEN);
			task.get(INFO_TASKURL_KEY, strTaskURL, MAX_URL_LEN);
			task.get(INFO_SESSIONURL_KEY, strSessionURL, MAX_URL_LEN);
			task.get(TASK_STATUS_KEY, strStatus, MAX_TASK_STATUS_LEN);
			size_t zStartTime	= task.get(CREATE_TIME);
			size_t zLastUpdate	= task.get(LAST_UPDATE);

			rvtemp.setStruct(INFO_TYPENAME_KEY, rpc::RpcValue(strTypeName));
			rvtemp.setStruct(INFO_TASKURL_KEY, rpc::RpcValue(strTaskURL));
			rvtemp.setStruct(INFO_SESSIONURL_KEY, rpc::RpcValue(strSessionURL));
			rvtemp.setStruct(INFO_STATUS_KEY, rpc::RpcValue(strStatus));
			rvtemp.setStruct(INFO_STARTTIME_KEY, rpc::RpcValue(getTimeStr(zStartTime).c_str()));
			rvtemp.setStruct(INFO_LASTUPDATE_KEY, rpc::RpcValue(getTimeStr(zLastUpdate).c_str()));

			result.setArray(i, rvtemp);
		}
		
		if (pstrEntry)
			taskmgr.deleteList(pstrEntry, count);
	}
	else if (0==strcmp(INFOTYPE_SESSIONS, strInfoType))
	{
		SessionManager sessionmgr(0);
		
		size_t count;
		char** pstrEntry = sessionmgr.listChildren(count);
		for (int i = 0; i < count; ++i)
		{
			rpc::RpcValue rvtemp;
			MetaRecord session(pstrEntry[i], PM_PROP_READ_ONLY);

			rvtemp.setStruct(INFO_SESSIONID_KEY, rpc::RpcValue(utils::NodePath::getPureName(pstrEntry[i]).c_str()));
			rvtemp.setStruct(INFO_STARTTIME_KEY, rpc::RpcValue(getTimeStr(session.get(CREATE_TIME)).c_str()));
			rvtemp.setStruct(INFO_LASTUPDATE_KEY, rpc::RpcValue(getTimeStr(session.get(LAST_UPDATE)).c_str()));
			rvtemp.setStruct(INFO_LEASETERM_KEY, rpc::RpcValue((int)session.get(SESS_PARAM_LEASETERM)));

			result.setArray(i, rvtemp);
		}

		if (pstrEntry)
			sessionmgr.deleteList(pstrEntry, count);
	}
	else if (0==strcmp(INFOTYPE_SESSIONDETAIL, strInfoType))
	{
		char strSessionId[MPF_SESSID_LEN] = {0};
		if (NULL == params.toString(strSessionId, MPF_SESSID_LEN))
		{
			result = ZQ::rpc::RpcValue("session id is empty");
			return;
		}

		rpc::RpcValue dbvalue;
		MetaSession session(strSessionId, PM_PROP_READ_ONLY);
		DbEntry2RpcValue(session, dbvalue);

		if (ZQ::rpc::RpcValue::TypeStruct == dbvalue.getType())
			result = dbvalue;
		else
			result.setStruct(NON_STRUCTURE, dbvalue);
	}
	else
	{
		result = ZQ::rpc::RpcValue("unknown information type");
	}
}

SRM_END
