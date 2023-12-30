#ifndef __ContentStoreReplicaImpl_H__
#define __ContentStoreReplicaImpl_H__

#define _WINSOCK2API_
#include <ContentReplicaEx.h>
#include "MetaLibImpl.h"

#define CONTENTSTOREREPLICA "ContentStoreReplica"
#define ENDPOINT "endpoint"

//class ContentStoreReplicaImpl : virtual public ::TianShanIce::Repository::ContentStoreReplicaEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class ContentStoreReplicaImpl : virtual public ::TianShanIce::Repository::ContentStoreReplicaEx, public ICEAbstractMutexRLock
{
public:
	ContentStoreReplicaImpl(::ZQTianShan::MetaLib::MetaLibImpl& _lib);
	~ContentStoreReplicaImpl();

public:
	virtual Ice::Identity getObjId(const ::Ice::Current& c) const;

	virtual std::string getType(const ::Ice::Current& c) const;

	virtual Ice::Identity getNetId(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Storage::ContentStorePrx theStore(const ::Ice::Current& c) const;

	virtual ::TianShanIce::ObjectInfo getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const;

	virtual void OnEvent(const std::string& eventName, const ::TianShanIce::Properties& params, const ::Ice::Current& c); // process the volume event

	virtual void OnTimer(const ::Ice::Current& c);

	virtual std::string getCreatedTime(const Ice::Current &) const;

	virtual TianShanIce::Repository::MetaDataValue get(const std::string &,const Ice::Current &) const;

	virtual void set(const std::string &,const TianShanIce::Repository::MetaDataValue &,const Ice::Current &);

	virtual void setLastSync(::Ice::Long, const ::Ice::Current& = ::Ice::Current());

	virtual Ice::Long getLastSync(const Ice::Current &) const;

	virtual TianShanIce::Repository::MetaObjectInfos listVolume(const Ice::Current &) const;

	virtual ::TianShanIce::Repository::MetaDataMap getMetaDataMap(const ::Ice::Current& c) const;

	virtual void setMetaDataMap(const ::TianShanIce::Repository::MetaDataMap& valueMap, const Ice::Current &);

private:
	::ZQTianShan::MetaLib::MetaLibImpl& _lib;
};

typedef ::IceInternal::Handle< ContentStoreReplicaImpl> ContentStoreReplicaImplPtr;

#endif //__ContentStoreReplicaImpl_H__

