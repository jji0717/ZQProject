#include "TsEdgeResource.h"
#pragma warning (disable:4273)

class AllocationOwnerImpl : public TianShanIce::EdgeResource::AllocationOwner
{
public:
	AllocationOwnerImpl();
	~AllocationOwnerImpl();

public:
	virtual ::Ice::Int OnAllocationExpiring(const ::std::string&, const ::TianShanIce::EdgeResource::AllocationPrx&, const ::Ice::Current&);
};
