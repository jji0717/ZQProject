#include "AllocationOwnerImpl.h"
#include <Ice/Connection.h>

AllocationOwnerImpl::AllocationOwnerImpl()
{
}

AllocationOwnerImpl::~AllocationOwnerImpl()
{
}

Ice::Int AllocationOwnerImpl::OnAllocationExpiring(const ::std::string& ownerContextKey, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::Ice::Current& c)
{
//	printf("%s call OnAllocationExpiring\n", ownerContextKey.c_str());
//	printf("Connection [%s]\n", (c.con)->toString().c_str());
//	std::string conn = (c.con)->toString();
//	conn = conn.substr(conn.find_last_of("=")+1, conn.size());
//	printf("Connection [%s]\n", conn.c_str());
//	alloc->renew(1000*60*10);
	return 1000*60*10;
}