#ifndef __ZQTianShan_AllocationOwnerImpl_H__
#define __ZQTianShan_AllocationOwnerImpl_H__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "Locks.h"
#include "EdgeRM.h"
namespace ZQTianShan{
namespace EdgeRM{

class AllocationOwnerImpl;
typedef IceUtil::Handle<AllocationOwnerImpl> AllocationOwnerImplPtr;

class AllocationOwnerImpl : public ::TianShanIce::EdgeResource::AllocationOwner
{
	friend class EdgeRMEnv;
public:
	AllocationOwnerImpl(int time2Expiring, EdgeRMEnv &env):_time2Expiring(time2Expiring),_env(env),_sequence(0){};
	~AllocationOwnerImpl(){};

	virtual ::Ice::Int OnAllocationExpiring(const ::std::string& ownerContextKey, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::Ice::Current& c);

private:
	EdgeRMEnv& _env;
	::Ice::Int _time2Expiring;
	Ice::Long  _sequence;
	ZQ::common::Mutex		_seqMutex;
};

}//namespace EdgeRM
}//namespace ZQTianShan
#endif //__ZQTianShan_AllocationOwnerImpl_H__
