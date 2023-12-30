#include "ContentLibFactory.h"
#include "ContentReplicaImpl.h"
#include "ContentStoreReplicaImpl.h"
#include "MetaVolumeImpl.h"
#include "MetaLibImpl.h"
#include "Log.h"

namespace ZQTianShan {
	namespace MetaLib {

ContentLibFactory::ContentLibFactory(MetaLibImpl& lib) : MetaLibFactory(lib)
{
	if (_lib._adapter)
	{
		Ice::CommunicatorPtr ic = _lib._adapter->getCommunicator();

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibFactory, "add factory onto communicator"));

		ic->addObjectFactory(this, TianShanIce::Repository::ContentReplica::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Repository::ContentReplicaEx::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Repository::ContentStoreReplica::ice_staticId());
		ic->addObjectFactory(this, TianShanIce::Repository::ContentStoreReplicaEx::ice_staticId());
		ic->addObjectFactory(this, ::TianShanIce::Repository::MetaVolumeEx::ice_staticId());
		ic->addObjectFactory(this, ::TianShanIce::Repository::MetaVolume::ice_staticId());
	}
}

Ice::ObjectPtr ContentLibFactory::create(const std::string& type)
{
	//	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibFactory, "create obj of %s"), type.c_str());

	if (TianShanIce::Repository::LibMetaObject::ice_staticId() == type)
		return new LibMetaObjectImpl(_lib);

	if (TianShanIce::Repository::LibMetaValue::ice_staticId() == type)
		return new LibMetaValueImpl(_lib);

	if (TianShanIce::Repository::ContentReplicaEx::ice_staticId() == type)
		return new ContentReplicaImpl(_lib);

	if (TianShanIce::Repository::ContentStoreReplicaEx::ice_staticId()  == type)
		return new ContentStoreReplicaImpl(_lib);

	if (::TianShanIce::Repository::MetaVolumeEx::ice_staticId() == type)
		return new MetaVolumeImpl(_lib);

	glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibFactory, "create(%s) type unknown"), type.c_str());
	return NULL;
}

void ContentLibFactory::destroy()
{
	//	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibFactory, "destroy()"));
}

	}
}

