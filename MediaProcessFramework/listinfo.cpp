
#include "listinfo.h"

#include <assert.h>
#include <stdio.h>
#include "MPFCommon.h"
#include "MPFLogHandler.h"

using namespace ZQ::MPF::utils;

#define MAX_INFOTYPE_STR_LEN 256

namespace ZQ
{
namespace MPF
{
	
	ListInfoMethod::ListInfoMethod(ZQ::rpc::RpcServer& server, const char* strInterface, int m_nodetype)
		:ZQ::rpc::RpcServerMethod(GETINFO_METHOD, &server), m_nodetype(m_nodetype)
	{
		m_si.Init();

		assert(strInterface);
		rpc::sfstrncpy(m_strInterface, strInterface, 20);
	}
	
	ListInfoMethod::~ListInfoMethod()
	{
	}

	void ListInfoMethod::setInterface(const char* strInterface)
	{
		sfstrncpy(m_strInterface, strInterface, 20);
	}
	
	void ListInfoMethod::execute(ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result)
	{
		if(!OnVerify(params, result))
		{
			// verify failed, quit this execute
			return;
		}

		char strInfoType[MAX_INFOTYPE_STR_LEN] = {0};
		params[0][INFOTYPE_KEY].toString(strInfoType, MAX_INFOTYPE_STR_LEN);
		
		if (0 == strcmp(strInfoType, INFOTYPE_GENERAL))
		{
			getSystemInfo(result);
			return;
		}
		else if (0 == strcmp(strInfoType, INFOTYPE_NODETYPE))
		{
			result = m_nodetype;
			return;
		}
		
		OnGetInfo(strInfoType, params[0][INFOPARAM_KEY], result);
	}

	void ListInfoMethod::getSystemInfo(ZQ::rpc::RpcValue& result)
	{
		DWORD dwStart = GetTickCount();
		result.setStruct(INFO_OS_KEY, ZQ::rpc::RpcValue(m_si.OSGetVersion()));
		result.setStruct(INFO_CPU_KEY, ZQ::rpc::RpcValue(m_si.CPUGetFixInfo().c_str()));
		char strMemory[256] = {0};
		_snprintf(strMemory, 255, "%ld/%ld(Free/Total)", m_si.MemoryGetAvailable(), m_si.MemoryGetTotal());
		result.setStruct(INFO_MEMORY_KEY, ZQ::rpc::RpcValue(strMemory));
		result.setStruct(INFO_PROCESS_KEY, ZQ::rpc::RpcValue(ZQ::MPF::utils::FilePath::getModuleName().c_str()));
		result.setStruct(INFO_MPFVERSION_KEY, ZQ::rpc::RpcValue(ZQ_PRODUCT_VER_STR3));
		result.setStruct(INFO_INTERFACE_KEY, ZQ::rpc::RpcValue(m_strInterface));
		char strNodeID[50] = {0};
		ZQ::MPF::utils::GetNodeGUID(strNodeID, 50);
		result.setStruct(INFO_NODEID_KEY, ZQ::rpc::RpcValue(strNodeID));

		DWORD dwEnd = GetTickCount();
		MPFLog(MPFLogHandler::L_DEBUG, "[system info time spend] %d ms", dwEnd-dwStart);
		
		/*
		char strTemp[512] = {0};
		result.toXml(strTemp, 512);
		printf("%s\n", strTemp);
		*/
	}
}
}
