#include "NGODFactory.h"
#include "./ContextImpl.h"

NGODFactory::NGODFactory(ssmNGODr2c1& env) : _env(env)
{
}

NGODFactory::~NGODFactory()
{
}

Ice::ObjectPtr NGODFactory::create(const std::string& id)
{
	if (id == NGODr2c1::Context::ice_staticId())
		return new NGODr2c1::ContextImpl(_env);

	return NULL;
}

void NGODFactory::destroy()
{
}

