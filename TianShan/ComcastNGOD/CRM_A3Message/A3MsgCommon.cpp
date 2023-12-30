
#include "TimeUtil.h"
#include "A3MsgCommon.h"
namespace CRM
{
	namespace A3Message
	{
std::string convertState(TianShanIce::Storage::ContentState contentState)
{
	switch (contentState)
	{
	case TianShanIce::Storage::csNotProvisioned:
		return CONTENTSTATUS_PENDING;
	case TianShanIce::Storage::csProvisioning:
		return CONTENTSTATUS_TRANSFER;
	case TianShanIce::Storage::csProvisioningStreamable:
		return CONTENTSTATUS_TRANSFER_PLAY;
	case TianShanIce::Storage::csInService:
		return CONTENTSTATUS_COMPLETE;
	case TianShanIce::Storage::csCleaning:
		return CONTENTSTATUS_CANCELED;
	case TianShanIce::Storage::csOutService:
		return CONTENTSTATUS_FAILED;
	default:
		return CONTENTSTATUS_UNKNOWN;
	}
}

std::string eventStateToA3State(std::string strEventContentState)
{
	if ("OutService(4)" == strEventContentState || "OutService" == strEventContentState)
	{
		return CONTENTSTATUS_FAILED;
	}
	if ("InService(3)" == strEventContentState || "InService" == strEventContentState)
	{
		return CONTENTSTATUS_COMPLETE;
	}
	if ("Cleaning(5)" == strEventContentState || "Cleaning" == strEventContentState)
	{
		return CONTENTSTATUS_CANCELED;
	}
	if ("NotProvisioned(0)" == strEventContentState ||"NotProvisioned" == strEventContentState  )
	{
		return CONTENTSTATUS_PENDING;
	}
	if ("Provisioning(1)" == strEventContentState || "Provisioning" == strEventContentState)
	{
		return CONTENTSTATUS_TRANSFER;
	}
	if ("ProvisioningStreamable(2)" == strEventContentState || "ProvisioningStreamable" == strEventContentState)
	{
		return CONTENTSTATUS_TRANSFER_PLAY;
	}
	return UNKNOWN;
}

TianShanIce::State ContentStateToState(TianShanIce::Storage::ContentState contentState)
{
	switch (contentState)
	{
	case TianShanIce::Storage::csNotProvisioned:
		return TianShanIce::stNotProvisioned;
	case TianShanIce::Storage::csInService:
		return TianShanIce::stInService;
	case TianShanIce::Storage::csOutService:
		return TianShanIce::stOutOfService;
	default:
		return TianShanIce::stProvisioned;
	}
}

std::string GenerateUTCTime()
{
	int64 ntime = ZQ::common::now();
	char md5DataTime[64];
	ZQ::common::TimeUtil::TimeToUTC(ntime, md5DataTime, sizeof(md5DataTime));
	std::string strUTCTime = md5DataTime;
	return strUTCTime;
}
std::string  GetDeleteContentReason(int reasonCode)
{
	switch (reasonCode)
	{
	case 200:
		return DCREASON_200;
	case 201:
		return DCREASON_201;
	case 202:
		return DCREASON_202;
	case 403:
		return DCREASON_403;
	case 409:
		return DCREASON_409;
	default:
		return "UNKNOWN";
	}
}
const char* backStoreTypeStr(BackStoreType type)
{
	switch(type)
	{
	case backContentLib:
		return "Contentlib";
	case backCacheStore:
		return "CacheStore";
	case backAuqaServer:
		return "AquaServer";
	default:
		break;
	}
	return "unkonwn";
}

}}

