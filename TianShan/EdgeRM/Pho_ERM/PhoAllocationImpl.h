#ifndef __ZQTianShan_PhoAllocationImpl_H__
#define __ZQTianShan_PhoAllocationImpl_H__

#include "PhoEdgeRMEnv.h"
#include "PhoAllocation.h"


namespace ZQTianShan{
namespace EdgeRM{

class PhoAllocationImpl;
typedef IceUtil::Handle<PhoAllocationImpl> PhoAllocationImplPtr;

class PhoAllocationImpl : public ::TianShanIce::EdgeResource::PhoAllocation, 
						  //public ::IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
						  public ICEAbstractMutexRLock
{
public:
	PhoAllocationImpl(PhoEdgeRMEnv &env);
	~PhoAllocationImpl();

	//PhoAllocation impl
	virtual ::Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::TianShanIce::EdgeResource::AllocationPrx getAllocation(const ::Ice::Current& ) const;
	//virtual ::TianShanIce::EdgeResource::AllocationOwnerPrx getAllocationOwer(const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::std::string getSessionGroup(const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::std::string getSessKey(const ::Ice::Current& = ::Ice::Current()) const;

private:
	PhoEdgeRMEnv &_env;
};

}//namespace EdgeRM
}//namespace ZQTianShan
#endif //__ZQTianShan_PhoAllocationImpl_H__

