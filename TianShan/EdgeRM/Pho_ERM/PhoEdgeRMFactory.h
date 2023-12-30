#ifndef __ZQTianShan_PhoEdgeRMFactory_H__
#define __ZQTianShan_PhoEdgeRMFactory_H__

#include "TianShanDefines.h"

#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace EdgeRM {

#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class PhoEdgeRMEnv;
class PhoEdgeRMFactory : public Ice::ObjectFactory
{
	friend class PhoEdgeRMEnv;

public:

    PhoEdgeRMFactory(PhoEdgeRMEnv& env);

    // Operations from ObjectFactory
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

	typedef IceUtil::Handle<PhoEdgeRMFactory> Ptr;

protected:
	PhoEdgeRMEnv& _env;
};

}} // namespace

#endif // __ZQTianShan_PhoEdgeRMFactory_H__
