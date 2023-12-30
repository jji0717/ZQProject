#ifndef __ZQTianShan_AllocationOwnerImpl_H__
#define __ZQTianShan_AllocationOwnerImpl_H__

#include "Locks.h"
#include "TianShanDefines.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "TsEdgeResource.h"
namespace ZQTianShan{
namespace EdgeRM{
class PhoEdgeRMEnv;
class PhoAllocationOwnerImpl : public ::TianShanIce::EdgeResource::AllocationOwner,
								/*public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>*/ 
							   public ICEAbstractMutexRLock
{
public:
	PhoAllocationOwnerImpl(::Ice::Int heartBeatTime, PhoEdgeRMEnv& env):
	  _time2Expiring(heartBeatTime), _env(env){};
	~PhoAllocationOwnerImpl(){};

	virtual ::Ice::Int OnAllocationExpiring(const ::std::string& ownerContextKey, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::Ice::Current& c);
private:
	::Ice::Int _time2Expiring;
	PhoEdgeRMEnv& _env;
public:
	typedef IceUtil::Handle<PhoAllocationOwnerImpl> Ptr;
};

}//namespace EdgeRM
}//namespace ZQTianShan
#endif //__ZQTianShan_AllocationOwnerImpl_H__

