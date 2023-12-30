#include "ContentReplicaImpl.h"
#include "ContentLibUtil.h"
#include "TsStorage.h"
#include "ContentStoreReplicaImpl.h"
#include "ContentLibConfig.h"

ContentReplicaImpl::ContentReplicaImpl(::ZQTianShan::MetaLib::MetaLibImpl& lib) : _lib(lib)
{
}

ContentReplicaImpl::~ContentReplicaImpl()
{
}

Ice::Identity ContentReplicaImpl::getObjId(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return LibMetaObject::ident;
}

std::string ContentReplicaImpl::getType(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return type;
}

Ice::Identity ContentReplicaImpl::getReplicatId(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return LibMetaObject::ident;
}

void ContentReplicaImpl::getAssetId(std::string& providerId, std::string& providerAssetId, const ::Ice::Current& c) const
{
	RLock rLock(*this);
	TianShanIce::Properties searchForMetaData;
	TianShanIce::StrValues expectedMetaDataNames;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, netId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(VOLUMENAME, volumeName));
	searchForMetaData.insert(TianShanIce::Properties::value_type(CONTENTNAME, contentName));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, CONTENTREPLICA));
	TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(CONTENTREPLICA, searchForMetaData, expectedMetaDataNames);
	if(ret.size() > 0)
	{		 
		providerAssetId = ret[0].metaDatas[METADATA_ProviderAssetId].value;
		providerId = ret[0].metaDatas[METADATA_ProviderId].value;
	}
}

void ContentReplicaImpl::getReplicaLocation(std::string& contentStoreNetId, std::string& volumeName, const ::Ice::Current& c) const
{
	RLock rLock(*this);
	contentStoreNetId = LibMetaObject::ident.name.substr(LibMetaObject::ident.name.find("@") + 1, LibMetaObject::ident.name.find("$"));
	volumeName = LibMetaObject::ident.name.substr(LibMetaObject::ident.name.find("$") + 1);
}

::TianShanIce::Properties ContentReplicaImpl::getMetaData(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	::TianShanIce::Properties metaData;

	for(::TianShanIce::Repository::MetaDataMap::const_iterator it = this->metaDataMap.begin(); it != this->metaDataMap.end(); it++)
	{
		metaData.insert(std::make_pair(it->first, it->second.value));
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "get MetaData [%s]=[%s]"), it->first.c_str(), it->second.value.c_str());
	}
	return metaData;
}

::TianShanIce::Storage::ContentState ContentReplicaImpl::getState(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return state;
}

::TianShanIce::Properties ContentReplicaImpl::getUpdateMetaData(const ::Ice::Current& c)
{
	::TianShanIce::Properties metaData = content->getMetaData();
	try
	{
		::TianShanIce::Repository::MetaDataValue metaDataValue;
		metaDataValue.hintedType = ::TianShanIce::vtStrings;

		WLock wLock(*this);
		::TianShanIce::Repository::MetaDataMap metaDataMap, indexMetaDataMap;
		for(::TianShanIce::Properties::iterator it = metaData.begin(); it != metaData.end(); it++)
		{
			metaDataValue.value = it->second;
			metaDataMap.insert(make_pair(it->first, metaDataValue));
			MAP_SET(::TianShanIce::Repository::MetaDataMap, this->metaDataMap, it->first, metaDataValue);
		}

		for (ContentLibConfig::MetadataCategories::iterator iter = _config.categories.begin(); iter != _config.categories.end(); ++iter) 
		{
			if(iter->indexFlag == 1)
			{
				if(metaDataMap.find(iter->category) != metaDataMap.end())
					indexMetaDataMap.insert(make_pair(iter->category, metaDataMap[iter->category]));
			}
		}

//		this->metaDataMap = metaDataMap;

		_lib.setMetaDatas(LibMetaObject::ident.name, indexMetaDataMap, false);

		if(metaData.find(METADATA_StampLastUpdated) != metaData.end())
		{
			lastModified = metaData[METADATA_StampLastUpdated];
		}
		
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentReplicaImpl, "getMetaData() : catch an exception [%s]"), ex.ice_name().c_str());
	}
	return metaData;
}

::TianShanIce::Storage::ContentState ContentReplicaImpl::getUpdateState(const ::Ice::Current& c)
{
	try
	{
		state = content->getState();

		::TianShanIce::Repository::MetaDataValue metaDataValue;
		metaDataValue.hintedType = ::TianShanIce::vtStrings;

		metaDataValue.value = convertState(state);
		try
		{
			WLock wLock(*this);
			MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, CONTENTSTATE, metaDataValue);
			_lib.proxy()->setMetaData(LibMetaObject::ident.name, CONTENTSTATE, metaDataValue, false);
		}
		catch (const Ice::Exception&)
		{
			return state;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "set MetaData [%s]=[%s]"), CONTENTSTATE, metaDataValue.value.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentReplicaImpl, "getState() : catch an exception [%s]"), ex.ice_name().c_str());
	}
	return state;
}

::TianShanIce::Storage::ContentPrx ContentReplicaImpl::theContent(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return content;
}

::TianShanIce::StatedObjInfo ContentReplicaImpl::getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const
{
	RLock rLock(*this);
	TianShanIce::StatedObjInfo stateInfo;
	stateInfo.state = ContentStateToState(state);
	stateInfo.ident = LibMetaObject::ident;
	stateInfo.props.clear();
	try
	{
/*
		TianShanIce::Properties searchForMetaData;
		::TianShanIce::Repository::MetaDataValue metaDataValue;
		metaDataValue.hintedType = ::TianShanIce::vtStrings;
		searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, netId));
		searchForMetaData.insert(TianShanIce::Properties::value_type(VOLUMENAME, volumeName));
		searchForMetaData.insert(TianShanIce::Properties::value_type(CONTENTNAME, contentName));
		searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, CONTENTREPLICA));
		TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(CONTENTREPLICA, searchForMetaData, expectedMetaData);
		if(ret.size() > 0) //exist in db, then modify
		{
			for(::TianShanIce::Repository::MetaDataMap::iterator it = ret[0].metaDatas.begin(); it != ret[0].metaDatas.end(); it++)
			{
				stateInfo.props.insert(std::make_pair(it->first, it->second.value));
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "get MetaData [%s]=[%s]"), it->first.c_str(), it->second.value.c_str());
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
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentReplicaImpl, "getInfo() : catch an exception [%s]"), ex.ice_name().c_str());
	}
	return stateInfo;
}

void ContentReplicaImpl::OnEvent(const std::string& eventName, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	WLock wLock(*this);
	return;
}

void ContentReplicaImpl::destroy(const std::string& reason, const ::Ice::Current& c)
{
	WLock wLock(*this);
	try
	{
		_lib.proxy()->removeObject(LibMetaObject::ident.name);
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentReplicaImpl, "Content [%s] removed with reason [%s]"), 
			contentName.c_str(), reason.c_str());
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentReplicaImpl, "remove ContentReplica caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}
}

void ContentReplicaImpl:: OnTimer(const ::Ice::Current& c)
{
	WLock wLock(*this);
	return;
}

::std::string ContentReplicaImpl::getCreatedTime(const ::Ice::Current& c) const
{
	char buf[64];
	RLock rLock(*this);
	return ::ZQTianShan::TimeToUTC(stampCreated, buf, sizeof(buf)-2);
}

::TianShanIce::Repository::MetaDataValue ContentReplicaImpl::get(const ::std::string& key, const ::Ice::Current& c) const
{
	::TianShanIce::Repository::MetaDataValue ret;
	ret.hintedType = ::TianShanIce::vtUnknown;
	RLock rLock(*this);
	return _lib.getMetaData(ident, key);
}

void ContentReplicaImpl::set(const ::std::string& key, const ::TianShanIce::Repository::MetaDataValue& value, const ::Ice::Current& c)
{
	WLock wLock(*this);
	_lib.setMetaData(ident.name, key, value, false, c);
}

void ContentReplicaImpl::setLastModified(const ::std::string& timeStamp, const Ice::Current &)
{
	WLock wLock(*this);
	lastModified = timeStamp;
}

::std::string ContentReplicaImpl::getLastModified(const Ice::Current &) const
{
	RLock rLock(*this);
	return lastModified;
}	

::TianShanIce::Repository::MetaDataMap ContentReplicaImpl::getMetaDataMap(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return metaDataMap;
}

void ContentReplicaImpl::setMetaDataMap(const ::TianShanIce::Repository::MetaDataMap& valueMap, const Ice::Current &)
{
	WLock wLock(*this);
//	metaDataMap = valueMap;
	for (::TianShanIce::Repository::MetaDataMap::const_iterator iter = valueMap.begin(); iter != valueMap.end(); iter++)
	{
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, iter->first, iter->second);
	}
}

