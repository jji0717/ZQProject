#include "ContentLibUtil.h"

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

TianShanIce::Storage::ContentState convertState(std::string contentState)
{
	if(PENDING == contentState)
		return TianShanIce::Storage::csNotProvisioned;
	else if(TRANSFER == contentState)
		return TianShanIce::Storage::csProvisioning;
	else if(STREAMABLE == contentState)
		return TianShanIce::Storage::csProvisioningStreamable;
	else if(COMPLETE == contentState)
		return TianShanIce::Storage::csInService;
	else if(CANCELED == contentState)
		return TianShanIce::Storage::csCleaning;
	else if(FAILED == contentState)
		return TianShanIce::Storage::csOutService;
	else 
		return TianShanIce::Storage::csOutService;
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

