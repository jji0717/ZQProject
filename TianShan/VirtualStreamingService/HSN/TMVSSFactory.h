#ifndef __ZQTianShan_TMVSSFactory_H__
#define __ZQTianShan_TMVSSFactory_H__

//#include "TMVSSEnv.h"
#include "ZQThreadPool.h"
#include "FileLog.h"

#include <Freeze/Freeze.h>
#include <TMVSSIce.h>

namespace ZQTianShan{
namespace VSS{
namespace TM{

#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class TMVSSFactory : public Ice::ObjectFactory
{
	friend class TMVSSEnv;

public:

	TMVSSFactory(TMVSSEnv& env);

	typedef IceUtil::Handle<TMVSSFactory> Ptr;
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string& type);
	virtual void destroy();

protected:
	TMVSSEnv& _env;
};
}//namespace TM
}//namespace NSS
}//namespace ZQTianShan

#endif __ZQTianShan_TMVSSFactory_H__
