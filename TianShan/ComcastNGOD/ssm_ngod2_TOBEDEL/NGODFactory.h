#ifndef __NGOD_FACTORY__
#define __NGOD_FACTORY__

#include <Freeze/Freeze.h>

class NGODEnv;

class NGODFactory : public Ice::ObjectFactory
{
public:
	NGODFactory(NGODEnv& env);
	virtual ~NGODFactory();
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();
	typedef IceUtil::Handle<NGODFactory> Ptr;

protected:
	NGODEnv& _env;
};

#endif // __NGOD_FACTORY__