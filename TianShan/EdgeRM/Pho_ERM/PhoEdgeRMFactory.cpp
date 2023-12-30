#include "PhoEdgeRMFactory.h"
#include "PhoEdgeRMEnv.h"
#include "Log.h"

namespace ZQTianShan {
namespace EdgeRM {

#define envlog (_env._log)

PhoEdgeRMFactory::PhoEdgeRMFactory(PhoEdgeRMEnv& env)
:_env(env)
{
	if ((Ice::ObjectAdapterPtr&)_env._adapter)
	{
		Ice::CommunicatorPtr ic = _env._adapter->getCommunicator();
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PhoEdgeRMFactory, "add factory onto communicator"));
		
		ic->addObjectFactory(this, TianShanIce::EdgeResource::PhoAllocation::ice_staticId());
	}
}

Ice::ObjectPtr PhoEdgeRMFactory::create(const std::string& type)
{

	if (TianShanIce::EdgeResource::PhoAllocation::ice_staticId() == type)
		return new PhoAllocationImpl(_env);

	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(PhoEdgeRMFactory, "create(%s) type unknown"), type.c_str());
    return NULL;
}

void PhoEdgeRMFactory::destroy()
{
}

}} // namespace
