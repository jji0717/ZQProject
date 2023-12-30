#include "PhoAllocationOwnerImpl.h"
#include "PhoEdgeRMEnv.h"
namespace ZQTianShan{
namespace EdgeRM{

::Ice::Int PhoAllocationOwnerImpl::OnAllocationExpiring(const ::std::string& ownerContextKey, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::Ice::Current& c)
{
	if(!_env.hasOnDemandSession(ownerContextKey))
		return 0;

	return _time2Expiring; 
}

}//namespace EdgeRM
}//namespace ZQTianShan

