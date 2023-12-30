#ifndef __ContentReplicaImpl_H__
#define __ContentReplicaImpl_H__

#define _WINSOCK2API_
#include <ContentReplicaEx.h>
#include "MetaLibImpl.h"
#include "ContentUser.h"
#include "ContentSysMD.h"

#define CONTENTREPLICA	"ContentReplica"
#define NETID			"netId"
#define VOLUMENAME		"volumeName"
#define CONTENTNAME		"contentName"
#define CONTENTSTATE	"contentState"
#define LASTUPDATED		"stampLastUpdated"
//#define OBJECTTYPE		"objectType"

#define MAP_SET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL

//class ContentReplicaImpl : public ::TianShanIce::Repository::ContentReplicaEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class ContentReplicaImpl : public ::TianShanIce::Repository::ContentReplicaEx, public ICEAbstractMutexRLock
{
public:
	ContentReplicaImpl(::ZQTianShan::MetaLib::MetaLibImpl& _lib);
	~ContentReplicaImpl();

public:
	virtual Ice::Identity getObjId(const ::Ice::Current& c) const;

	virtual std::string getType(const ::Ice::Current& c) const;

	virtual Ice::Identity getReplicatId(const ::Ice::Current& c) const;

	virtual void getAssetId(std::string& providerId, std::string& providerAssetId, const ::Ice::Current& c) const;

	virtual void getReplicaLocation(std::string& contentStoreNetId, std::string& volumeName, const ::Ice::Current& c) const;

	virtual ::TianShanIce::Properties getMetaData(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Storage::ContentState getState(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Storage::ContentPrx theContent(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Properties getUpdateMetaData(const ::Ice::Current& c);

	virtual ::TianShanIce::Storage::ContentState getUpdateState(const ::Ice::Current& c);

	virtual ::TianShanIce::StatedObjInfo getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const;

	virtual void OnEvent(const std::string& eventName, const ::TianShanIce::Properties& params, const ::Ice::Current& c); // process the service event

	virtual void destroy(const std::string& reason, const ::Ice::Current& c);

	virtual void OnTimer(const ::Ice::Current& c);

	virtual std::string getCreatedTime(const Ice::Current &) const;

	virtual TianShanIce::Repository::MetaDataValue get(const std::string &,const Ice::Current &) const;

	virtual void set(const std::string &,const TianShanIce::Repository::MetaDataValue &,const Ice::Current &);

	virtual void setLastModified(const ::std::string&, const Ice::Current &);

	virtual ::std::string getLastModified(const Ice::Current &) const;

	virtual ::TianShanIce::Repository::MetaDataMap getMetaDataMap(const ::Ice::Current& c) const;

	virtual void setMetaDataMap(const ::TianShanIce::Repository::MetaDataMap& valueMap, const Ice::Current &);


private:
	::ZQTianShan::MetaLib::MetaLibImpl& _lib;
	Ice::CommunicatorPtr _ic;
};

typedef ::IceInternal::Handle< ContentReplicaImpl> ContentReplicaImplPtr;

#endif //__ContentReplicaImpl_H__

