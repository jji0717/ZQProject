#include "MetaVolumeImpl.h"
#include "ContentReplicaImpl.h"

MetaVolumeImpl::MetaVolumeImpl(::ZQTianShan::MetaLib::MetaLibImpl& lib) : _lib(lib)
{
}

MetaVolumeImpl::~MetaVolumeImpl()
{
}

Ice::Identity MetaVolumeImpl::getObjId(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return LibMetaObject::ident;
}

std::string MetaVolumeImpl::getType(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return type;
}

Ice::Identity MetaVolumeImpl::getNetId(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return LibMetaObject::ident;
}

::TianShanIce::Storage::VolumePrx MetaVolumeImpl::theVolume(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return volume;
}

::TianShanIce::StatedObjInfo MetaVolumeImpl::getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const
{
	RLock rLock(*this);
	TianShanIce::StatedObjInfo stateInfo;
	stateInfo.ident = LibMetaObject::ident;
	stateInfo.props.clear();
/*
	TianShanIce::Properties searchForMetaData;
	::TianShanIce::Repository::MetaDataValue metaDataValue;
	metaDataValue.hintedType = ::TianShanIce::vtStrings;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, netId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(VOLUMENAME, volumeName));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, METAVOLUME));
	TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(METAVOLUME, searchForMetaData, expectedMetaData);
	if(ret.size() > 0) //exist in db
	{
		for ( ::TianShanIce::Repository::MetaDataMap::const_iterator it = ret[0].metaDatas.begin(); 
			it != ret[0].metaDatas.end(); it++)
		{
			stateInfo.props.insert(std::make_pair(it->first, it->second.value));
		}
	}
*/
	for ( ::TianShanIce::StrValues::const_iterator it = expectedMetaData.begin(); 
		it != expectedMetaData.end(); it++)
	{
		::TianShanIce::Repository::MetaDataMap::const_iterator map_it = metaDataMap.find(*it);
		if(map_it != metaDataMap.end())
			stateInfo.props.insert(std::make_pair(map_it->first, map_it->second.value));
	}
	return stateInfo;
}

void MetaVolumeImpl::OnEvent(const std::string& eventName, const ::TianShanIce::Properties& params, const ::Ice::Current& c) // process the content event
{
	WLock wLock(*this);
	return;
}

void MetaVolumeImpl:: OnTimer(const ::Ice::Current& c)
{
	WLock wLock(*this);
	return;
}

::std::string MetaVolumeImpl::getCreatedTime(const ::Ice::Current& c) const
{
	char buf[64];
	RLock rLock(*this);
	return ::ZQTianShan::TimeToUTC(stampCreated, buf, sizeof(buf)-2);
}

::TianShanIce::Repository::MetaDataValue MetaVolumeImpl::get(const ::std::string& key, const ::Ice::Current& c) const
{
	::TianShanIce::Repository::MetaDataValue ret;
	ret.hintedType = ::TianShanIce::vtUnknown;
	RLock rLock(*this);
	return _lib.getMetaData(ident, key);
}

void MetaVolumeImpl::set(const ::std::string& key, const ::TianShanIce::Repository::MetaDataValue& value, const ::Ice::Current& c)
{
	WLock wLock(*this);
	_lib.setMetaData(ident.name, key, value, false, c);
}

void MetaVolumeImpl::setLastModified(const ::std::string& timeStamp, const Ice::Current &)
{
	WLock wLock(*this);
	lastModified = timeStamp;
}

::std::string MetaVolumeImpl::getLastModified(const Ice::Current &) const
{
	RLock rLock(*this);
	return lastModified;
}	

::TianShanIce::Repository::MetaDataMap MetaVolumeImpl::getMetaDataMap(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return metaDataMap;
}

void MetaVolumeImpl::setMetaDataMap(const ::TianShanIce::Repository::MetaDataMap& valueMap, const Ice::Current &)
{
	WLock wLock(*this);
	metaDataMap = valueMap;
}

