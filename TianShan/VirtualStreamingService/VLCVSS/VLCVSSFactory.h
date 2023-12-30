#ifndef __ZQTianShan_VLCVSSFactory_H__
#define __ZQTianShan_VLCVSSFactory_H__

#include "ZQThreadPool.h"
#include "FileLog.h"

#include <Freeze/Freeze.h>
#include <VLCVSS.h>

namespace ZQTianShan{
namespace VSS{
namespace VLC{

#define FACTOBJNAME(_CLASS) "::ZQTianShan::" #_CLASS

class VLCVSSFactory : public Ice::ObjectFactory
{
	friend class VLCVSSEnv;

public:

	VLCVSSFactory(VLCVSSEnv& env);

	typedef IceUtil::Handle<VLCVSSFactory> Ptr;
	
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string& type);
	virtual void destroy();

protected:
	VLCVSSEnv& _env;
};
}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan

#endif __ZQTianShan_VLCVSSFactory_H__
