#ifndef __NGOD_FACTORY__
#define __NGOD_FACTORY__

#include <Freeze/Freeze.h>

class ssmNGODr2c1;

class NGODFactory : public Ice::ObjectFactory
{
public:
	NGODFactory(ssmNGODr2c1& env);
	virtual ~NGODFactory();
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();
	typedef IceUtil::Handle<NGODFactory> Ptr;

protected:
	ssmNGODr2c1& _env;
};

#endif // __NGOD_FACTORY__