// FileName : A3Ccommon.cpp
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : common functions and constants

#include "TimeUtil.h"
#include "A3Common.h"

std::string convertState(TianShanIce::Storage::ContentState contentState)
{
	switch (contentState)
	{
	case TianShanIce::Storage::csNotProvisioned:
		return PENDING;
	case TianShanIce::Storage::csProvisioning:
		return TRANSFER;
	case TianShanIce::Storage::csProvisioningStreamable:
		return STREAMABLE;
	case TianShanIce::Storage::csInService:
		return COMPLETE;
	case TianShanIce::Storage::csCleaning:
		return CANCELED;
	case TianShanIce::Storage::csOutService:
		return FAILED;
	default:
		return UNKNOWN;
	}
}

std::string eventStateToA3State(std::string strEventContentState)
{
	if ("OutService(4)" == strEventContentState)
	{
		return FAILED;
	}
	if ("InService(3)" == strEventContentState)
	{
		return COMPLETE;
	}
	if ("Cleaning(5)" == strEventContentState)
	{
		return CANCELED;
	}
	if ("NotProvisioned(0)" == strEventContentState)
	{
		return PENDING;
	}
	if ("Provisioning(1)" == strEventContentState)
	{
		return TRANSFER;
	}
	if ("ProvisioningStreamable(2)" == strEventContentState)
	{
		return STREAMABLE;
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

