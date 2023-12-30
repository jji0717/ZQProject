#include "ContentStoreReplicaImpl.h"
#include "ContentReplicaImpl.h"
#include "MetaVolumeImpl.h"

ContentStoreReplicaImpl::ContentStoreReplicaImpl(::ZQTianShan::MetaLib::MetaLibImpl& lib) : _lib(lib)
{
}

ContentStoreReplicaImpl::~ContentStoreReplicaImpl()
{
}

Ice::Identity ContentStoreReplicaImpl::getObjId(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return LibMetaObject::ident;
}

std::string ContentStoreReplicaImpl::getType(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return type;
}

Ice::Identity ContentStoreReplicaImpl::getNetId(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return LibMetaObject::ident;
}

::TianShanIce::Storage::ContentStorePrx ContentStoreReplicaImpl::theStore(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return contentstore;
}

::TianShanIce::ObjectInfo ContentStoreReplicaImpl::getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const
{
	RLock rLock(*this);
	TianShanIce::ObjectInfo objectInfo;
	objectInfo.ident = LibMetaObject::ident;
	objectInfo.props.clear();
/*
	TianShanIce::Properties searchForMetaData;
	::TianShanIce::Repository::MetaDataValue metaDataValue;
	metaDataValue.hintedType = ::TianShanIce::vtStrings;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, netId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, CONTENTSTOREREPLICA));
	TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(CONTENTSTOREREPLICA, searchForMetaData, expectedMetaData);
	if(ret.size() > 0) //exist in db
	{
		for ( ::TianShanIce::Repository::MetaDataMap::const_iterator it = ret[0].metaDatas.begin(); 
			it != ret[0].metaDatas.end(); it++)
		{
			objectInfo.props.insert(std::make_pair(it->first, it->second.value));
		}
	}
*/
	for ( ::TianShanIce::StrValues::const_iterator it = expectedMetaData.begin(); 
		it != expectedMetaData.end(); it++)
	{
		::TianShanIce::Repository::MetaDataMap::const_iterator map_it = metaDataMap.find(*it);
		if(map_it != metaDataMap.end())
			objectInfo.props.insert(std::make_pair(map_it->first, map_it->second.value));
	}
	return objectInfo;
}

void ContentStoreReplicaImpl::OnEvent(const std::string& eventName, const ::TianShanIce::Properties& params, const ::Ice::Current& c) // process the volume event
{
	WLock wLock(*this);
	return;
}

void ContentStoreReplicaImpl:: OnTimer(const ::Ice::Current& c)
{
	WLock wLock(*this);
	return;
}

::std::string ContentStoreReplicaImpl::getCreatedTime(const ::Ice::Current& c) const
{
	char buf[64];
	RLock rLock(*this);
	return ::ZQTianShan::TimeToUTC(stampCreated, buf, sizeof(buf)-2);
}

::TianShanIce::Repository::MetaDataValue ContentStoreReplicaImpl::get(const ::std::string& key, const ::Ice::Current& c) const
{
	::TianShanIce::Repository::MetaDataValue ret;
	ret.hintedType = ::TianShanIce::vtUnknown;
	RLock rLock(*this);
	return _lib.getMetaData(ident, key);
}

void ContentStoreReplicaImpl::set(const ::std::string& key, const ::TianShanIce::Repository::MetaDataValue& value, const ::Ice::Current& c)
{
	WLock wLock(*this);
	_lib.setMetaData(ident.name, key, value, false, c);
}

void ContentStoreReplicaImpl::setLastSync(::Ice::Long time, const ::Ice::Current& c)
{
	WLock wLock(*this);
	lastSync = time;
}

Ice::Long ContentStoreReplicaImpl::getLastSync(const Ice::Current &) const
{
	RLock rLock(*this);
	return lastSync;
}	

TianShanIce::Repository::MetaObjectInfos ContentStoreReplicaImpl::listVolume(const Ice::Current &) const
{
	RLock rLock(*this);
	TianShanIce::Properties searchForMetaData;
	TianShanIce::StrValues expectedMetaData;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, netId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, METAVOLUME));
	return _lib.proxy()->lookup(METAVOLUME, searchForMetaData, expectedMetaData);
}

::TianShanIce::Repository::MetaDataMap ContentStoreReplicaImpl::getMetaDataMap(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return metaDataMap;
}

void ContentStoreReplicaImpl::setMetaDataMap(const ::TianShanIce::Repository::MetaDataMap& valueMap, const Ice::Current &)
{
	WLock wLock(*this);
	metaDataMap = valueMap;
}

