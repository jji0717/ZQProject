#ifndef __HSNTree_Factory_H__
#define __HSNTree_Factory_H__

#include <Ice/Ice.h>
#include <Freeze/Freeze.h>

namespace HSNTree
{
	class Environment;
	class Factory : public ::Ice::ObjectFactory
	{
	public: // constructor and destructor		
		Factory(Environment& env);
		virtual ~Factory();

		virtual Ice::ObjectPtr create(const std::string&);
		virtual void destroy();

		typedef IceUtil::Handle<Factory> Ptr;

	protected:
		Environment& _env;

	}; // end class Factory

	typedef Factory::Ptr FactoryPtr;

} // end namespace HSNTree

#endif // __HSNTree_Factory_H__

