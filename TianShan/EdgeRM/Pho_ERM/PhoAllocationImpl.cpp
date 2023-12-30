#include "PhoAllocationImpl.h"

namespace ZQTianShan{
namespace EdgeRM{

PhoAllocationImpl::PhoAllocationImpl(PhoEdgeRMEnv &env)
:_env(env)
{
}

PhoAllocationImpl::~PhoAllocationImpl()
{

}

::Ice::Identity PhoAllocationImpl::getIdent(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return ident;
}

::TianShanIce::EdgeResource::AllocationPrx PhoAllocationImpl::getAllocation(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return alloc;
}

//::TianShanIce::EdgeResource::AllocationOwnerPrx getAllocationOwer(const ::Ice::Current& = ::Ice::Current()) const;
//{
//	RLock rLock(*this);
//	return allocOwner;
//}

::std::string PhoAllocationImpl::getSessionGroup(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return sessionGroup;
}

::std::string PhoAllocationImpl::getSessKey(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return sessKey;
}

}//namespace EdgeRM
}//namespace ZQTianshan
