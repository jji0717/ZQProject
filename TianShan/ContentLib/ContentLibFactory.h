#ifndef __ContentLibFactory_H__
#define __ContentLibFactory_H__

#include "MetaLibFactory.h"

namespace ZQTianShan {
	namespace MetaLib {

class ContentLibFactory : virtual public MetaLibFactory
{
	friend class MetaLibImpl;

public:
	ContentLibFactory(MetaLibImpl& lib);
	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();

	typedef IceUtil::Handle<ContentLibFactory> Ptr;
};

	}
}

#endif //__ContentLibFactory_H__

