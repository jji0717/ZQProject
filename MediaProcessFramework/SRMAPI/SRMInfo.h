
#ifndef _ZQ_SRMINFO_H_
#define _ZQ_SRMINFO_H_

#include "listinfo.h"
#include "metarecord.h"


SRM_BEGIN

///SRMInfo
///manager system information list method
class DLL_PORT SRMInfo : public ZQ::MPF::ListInfoMethod
{
private:
	void OnGetInfo(const char* strInfoType, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result);

public:
	///constructor
	///@param server - rpc server
	///@param strInterface - network interface ip string
	SRMInfo(ZQ::rpc::RpcServer& server, const char* strInterface);
};

SRM_END

#endif//_ZQ_SRMINFO_H_
