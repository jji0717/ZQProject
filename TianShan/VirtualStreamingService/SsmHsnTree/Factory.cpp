#include "./Factory.h"
#include "./Environment.h"
#include "./SessionContextImpl.h"

namespace HSNTree
{	
	Factory::Factory(Environment& env) : _env(env)
	{
	}
	
	Factory::~Factory()
	{
	}
	
	Ice::ObjectPtr Factory::create(const std::string& classid)
	{
		if (classid == SessionContext::ice_staticId())
			return new SessionContextImpl(_env);
		return NULL;
	}
	
	void Factory::destroy()
	{
	}

} // end namespace HSNTree

