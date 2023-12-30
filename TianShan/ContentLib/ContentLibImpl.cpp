// #include "ContentLibImpl.h"
#include "TianShanIceHelper.h"
#include "ContentLibUtil.h"
#include "ContentLibConfig.h"
#include "ContentLibRequest.h"
#include "ContentLibEnv.h"
#include "ContentReplicaImpl.h"
#include "TimeUtil.h"

const std::string ContentCategory = "Content";
const std::string ContentStoreCategory = "ContentStore";
const std::string ContentStoreStarted= "ServiceStarted";
const std::string ContentStoreStopped= "ServiceStopped";
const std::string ContentCreate = "Created";
const std::string ContentDestroy = "Destroyed";
const std::string StateChange = "StateChanged";
const std::string EventField_Content = "content";
const std::string EventField_Volume = "volume";
const std::string EventField_Name = "name";
const std::string EventField_NetId = "netId";
const std::string EventField_PrevState = "oldState";
const std::string EventField_CurState = "newState";
const std::string EventField_EndPoint = "endPoint";
const std::string EventField_ProviderId = "providerId";
const std::string EventField_ProviderAssetId = "providerAssetId";

#define  TimeToSync 3600 //msec level, just for test, maybe it's in config item 

ContentLibImpl::ContentLibImpl(::ZQTianShan::MetaLib::MetaLibImpl& lib, ContentLibEnv& env) :_lib(lib), _env(env), timeToSync((Ice::Long)(_config.timeToSync*1000)), contentCount(0)
{
	_factory = new ZQTianShan::MetaLib::ContentLibFactory(lib);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "Time to Sync [%lld] ms"), timeToSync);
}

ContentLibImpl::~ContentLibImpl()
{
}

::TianShanIce::Repository::ContentStoreReplicaPrx ContentLibImpl::toStoreReplica(const ::std::string& netId, const ::Ice::Current& c)
{
	::TianShanIce::Repository::ContentStoreReplicaPrx proxy;
	Ice::Identity ident;	
	ident.name = netId;
	ident.category = META_OBJECT;
	try
	{	
		proxy = ::TianShanIce::Repository::ContentStoreReplicaPrx::checkedCast(_lib._adapter->createProxy(ident));
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toStoreReplica(%s) caught(%s: %s)"), netId.c_str()
			, ex.ice_name().c_str(), ex.message.c_str());
		return NULL;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toStoreReplica(%s) caught(%s)"), netId.c_str()
			, ex.ice_name().c_str());
		return NULL;
	}
	return proxy;
}

::TianShanIce::Repository::MetaVolumePrx ContentLibImpl::toVolume(const ::std::string& volumeId, const ::Ice::Current& c)
{
	::TianShanIce::Repository::MetaVolumePrx proxy;
	Ice::Identity ident;	
	ident.name = volumeId;
	ident.category = META_OBJECT;
	try
	{
		proxy = ::TianShanIce::Repository::MetaVolumePrx::checkedCast(_lib._adapter->createProxy(ident));
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toVolume(%s) caught(%s: %s)"), volumeId.c_str()
			, ex.ice_name().c_str(), ex.message.c_str());
		return NULL;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toVolume(%s) caught(%s)"), volumeId.c_str()
			, ex.ice_name().c_str());
		return NULL;
	}
	return proxy;
}

::TianShanIce::Repository::ContentReplicaPrx ContentLibImpl::toContentReplica(const ::std::string& contentReplicaId, const ::Ice::Current& c)
{
	std::string strNetId =  contentReplicaId.substr(0, contentReplicaId.find("$"));
	std::string volName = contentReplicaId.substr(contentReplicaId.find("$") + 1, contentReplicaId.find_last_of("/") - contentReplicaId.find("$") - 1);
	std::string contentName = contentReplicaId.substr(contentReplicaId.find_last_of("/") + 1);

	::TianShanIce::Repository::ContentReplicaPrx proxy;
	Ice::Identity ident;	
	ident.name = contentName + "@" + strNetId + "$" + volName;
	ident.category = META_OBJECT;
	try
	{
		proxy = ::TianShanIce::Repository::ContentReplicaPrx::checkedCast(_lib._adapter->createProxy(ident));
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toContentReplica(%s) caught(%s: %s)"), contentReplicaId.c_str()
			, ex.ice_name().c_str(), ex.message.c_str());
		return NULL;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toContentReplica(%s) caught(%s)"), contentReplicaId.c_str()
			, ex.ice_name().c_str());
		return NULL;
	}
	return proxy;
}	

void ContentLibImpl::locateContent_async(const ::TianShanIce::Repository::AMD_ContentLib_locateContentPtr& amdCB, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames, ::Ice::Int maxHop, bool pingOnly, const ::Ice::Current& c)
{
// 	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "locateContent() enter"));
// 	try
// 	{
// 		TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(CONTENTREPLICA, searchForMetaData, expectedMetaDataNames);
// 		amdCB->ice_response(ret);
// 	}
// 	catch (const Ice::Exception& ex)
// 	{
// 		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "locateContent() : catch an ice exception [%s]"), ex.ice_name().c_str());
// 		throw ex;
// 	}
// 	catch (...)
// 	{
// 	}
// 	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "locateContent() leave"));
	try {
		(new LocateCmd(_env, amdCB, searchForMetaData, expectedMetaDataNames, maxHop, pingOnly))->execute();
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"locateContent_async() failed to initialize LocateCmd: exception[%s]"), ex.ice_name().c_str());
		amdCB->ice_exception(::TianShanIce::ServerError("ContentLibImpl", 500, "failed to initialize LocateCmd: exception"));
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"locateContent_async() failed to initialize LocateCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("ContentLibImpl", 500, "failed to initialize LocateCmd"));
	}
}

void ContentLibImpl::locateContentByPIDAndPAID_async(const ::TianShanIce::Repository::AMD_ContentLib_locateContentByPIDAndPAIDPtr& amdCB, const ::std::string& netId, const ::std::string& volumeId, const ::std::string& providerId, const ::std::string& providerAssetId, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current&)
{
	try {
		(new LocateByPidPAidCmd(_env, amdCB, netId, volumeId, providerId, providerAssetId, expectedMetaDataNames))->execute();
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"locateContentByPIDAndPAID_async() failed to initialize LocateByPidPAidCmd: exception[%s]"), ex.ice_name().c_str());
		amdCB->ice_exception(::TianShanIce::ServerError("ContentLibImpl", 501, "failed to initialize LocateByPidPAidCmd: exception"));
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"locateContentByPIDAndPAID_async() failed to initialize LocateByPidPAidCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("ContentLibImpl", 501, "failed to initialize LocateByPidPAidCmd"));
	}
}

void ContentLibImpl::locateContentByNetIDAndVolume_async(const ::TianShanIce::Repository::AMD_ContentLib_locateContentByNetIDAndVolumePtr& amdCB, const ::std::string& netId, const ::std::string& volumeId, const ::TianShanIce::StrValues& expectedMetaDataNames, ::Ice::Int startCount, ::Ice::Int maxCount, const ::Ice::Current&) const
{
	TianShanIce::Repository::MetaObjectInfos result;
	::Ice::Int totalCount;
	result = _env._pContentCacheThread->getContentList(netId, volumeId, startCount, maxCount, totalCount, expectedMetaDataNames);
	amdCB->ice_response(result, totalCount);
}

void ContentLibImpl::locateVolumesByNetID_async(const ::TianShanIce::Repository::AMD_ContentLib_locateVolumesByNetIDPtr& amdCB, const ::std::string& netId, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current&)
{
	try {
		(new LocateVolumesByNidCmd(_env, amdCB, netId, expectedMetaDataNames))->execute();
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"locateVolumesByNetID_async() failed to initialize LocateVolumesByNidCmd: exception[%s]"), ex.ice_name().c_str());
		amdCB->ice_exception(::TianShanIce::ServerError("ContentLibImpl", 501, "failed to initialize LocateVolumesByNidCmd: exception"));
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(MetaLib,"locateVolumesByNetID_async() failed to initialize LocateVolumesByNidCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("ContentLibImpl", 501, "failed to initialize LocateVolumesByNidCmd"));
	}
}


::TianShanIce::Repository::MetaObjectInfo ContentLibImpl::openObject(const ::std::string& id, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current& c)
{
	return _lib.proxy()->openObject(id, expectedMetaDataNames);
}

::std::string ContentLibImpl::createObject(const ::std::string& type, ::Ice::Long timeout, const ::Ice::Current& c)
{
	return _lib.proxy()->createObject(type, timeout);
}

::std::string ContentLibImpl::getAdminUri(const ::Ice::Current& c)
{
	return _strAdminURL;
}

::TianShanIce::State ContentLibImpl::getState(const ::Ice::Current& c)
{
	return _state;
}

void ContentLibImpl::lookup_async(const ::TianShanIce::Repository::AMD_MetaDataLibrary_lookupPtr& amdCB, const ::std::string& type, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current& c)
{
	 lookup_async(amdCB, type, searchForMetaData, expectedMetaDataNames, c);
}

void ContentLibImpl::ping(::Ice::Long timestamp, const ::Ice::Current&)
{
}

void ContentLibImpl::post(
						 const ::std::string& category,
						 ::Ice::Int eventId,
						 const ::std::string& eventName,
						 const ::std::string& stampUTC,
						 const ::std::string& sourceNetId,
						 const ::TianShanIce::Properties& params,
						 const ::Ice::Current&
						 )
{
// 	std::string strNetId = params.find(EventField_NetId) != params.end() ? params.find(EventField_NetId)->second : "";
// 	if ("" == strNetId)
// 	{
// 		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "OnEvent() : Net Id is empty"));
// 	}
	if (ContentStoreCategory == category)
	{
		std::string strEndpoint = params.find(EventField_EndPoint) != params.end() ? params.find(EventField_EndPoint)->second : "";
		if ("" == strEndpoint)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "OnEvent() : Endpoint is empty"));
			return;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "OnEvent() : receive an event (Event name : %s, ContentStore[%s]  from [%s]"), eventName.c_str(), strEndpoint.c_str(), sourceNetId.c_str());
		if (eventName == ContentStoreStarted)
		{
			std::string netId;
			if(connectContentStore(strEndpoint, netId))
				addContentStoreReplica(netId, strEndpoint); // add record to DB or sync ContentStore if in need
			return;
		}
		if (eventName == ContentStoreStopped)
		{
			return;
		}
	}
	else if(ContentCategory == category)
	{
		std::string strName = params.find(EventField_Name) != params.end() ? params.find(EventField_Name)->second : "";
		if ("" == strName)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "OnEvent() : Content Name is empty"));
		}
		std::string strVolume = params.find(EventField_Volume) != params.end() ? params.find(EventField_Volume)->second : "";
		if ("" == strVolume)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "OnEvent() : Volume Name is empty"));
		}
		std::string strFullName = strVolume + "/" + strName;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "OnEvent() : receive an event (Event name : %s, ContentName[%s]  from [%s]"), eventName.c_str(), strFullName.c_str(), sourceNetId.c_str());
		if (eventName == ContentCreate)
		{
			try
			{
				addContentReplica(sourceNetId, strVolume, strName);
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "addContentReplica() : catch an ice exception [%s]"), ex.ice_name().c_str());
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "addContentReplica() : catch an unexpected exception"));
			}
			return;
		}
		if (eventName == ContentDestroy)
		{
			DeleteContentRequest* pRequest = new DeleteContentRequest(_env, sourceNetId, strVolume, strName);
			if (pRequest)
			{
				pRequest->start();
			}
			else 
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create delete content request for [%s@%s$%s] failed"), strName.c_str(), sourceNetId.c_str(), strVolume.c_str());
			}
			return;
		}
		if (eventName == StateChange)
		{
			std::string contentState =  params.find(EventField_CurState) != params.end() ? params.find(EventField_CurState)->second : "";
//			if(contentState == "InService(3)")
			{
				::TianShanIce::Storage::ContentInfo info;
				info.metaData.clear();
				UpdateContentRequest* pRequest = new UpdateContentRequest(_env, sourceNetId, strVolume, strName, info);
				if (pRequest)
				{
					pRequest->start();
				}
				else 
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create update content request for [%s@%s$%s] failed"), strName.c_str(), sourceNetId.c_str(), strVolume.c_str());
				}
			}			
			return;
		}
	}
}

void ContentLibImpl::addContentReplica(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "addContentReplica(%s, %s, %s)"), strNetId.c_str(), strVolume.c_str(), strContentName.c_str());
	std::string objId = strContentName + "@" + strNetId + "$" + strVolume;
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	if (!getContentStoreProxy(strNetId, contentStoreProxy))
	{
		return;
	}
	TianShanIce::Storage::ContentPrx contentProxy;
	std::string strFullName = strVolume + "/" + strContentName;
	try
	{
		contentProxy = contentStoreProxy->openContentByFullname(strFullName);
		if (!contentProxy)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "openContentByFullName(%s) : content proxy is empty"), strFullName.c_str());
			return;
		}
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), strFullName.c_str());
		return;
	}

	::TianShanIce::Repository::MetaDataValue metaData;
	metaData.hintedType = ::TianShanIce::vtStrings;

	::Ice::Identity identObj;
	identObj.category = META_OBJECT;
	identObj.name = objId;
	try {
		ContentReplicaImplPtr pObj = new ContentReplicaImpl(_lib);
		pObj->ident = identObj;
		pObj->type = CONTENTREPLICA;
		pObj->stampCreated = ZQTianShan::now(); ///< timestamp as of created
		pObj->content = contentProxy;
		pObj->lastModified = "";
		pObj->netId = strNetId;
		pObj->volumeName = strVolume;
		pObj->contentName = strContentName;
		::TianShanIce::Repository::MetaDataMap metaDataMap;
		metaData.value = strNetId;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, NETID, metaData);
		metaData.value = strVolume;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, VOLUMENAME, metaData);
		metaData.value = strContentName;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, CONTENTNAME, metaData);
		metaData.value = convertState(TianShanIce::Storage::csNotProvisioned);
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, CONTENTSTATE, metaData);
		pObj->metaDataMap = metaDataMap;
		_lib.addObject(pObj, identObj);	
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "addContentReplica() added new obj[%s:%s]"), identObj.name.c_str(), identObj.category.c_str());
		_env._pContentCacheThread->addContent(strNetId, strVolume, strContentName);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddContentRequest, "addContentReplica content [%s] caught %s: %s"), 
			identObj.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "addContentReplica(%s) ice exception[%s]"), identObj.name.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "addContentReplica(%s) unknown exception"), identObj.name.c_str());
	}

	::TianShanIce::Repository::MetaDataMap metaDataMap;
	metaData.value = strNetId;
	metaDataMap.insert(std::make_pair(NETID, metaData));		

	metaData.value = strVolume;
	metaDataMap.insert(std::make_pair(VOLUMENAME, metaData));

	metaData.value = strContentName;
	metaDataMap.insert(std::make_pair(CONTENTNAME, metaData));

	metaData.value = convertState(TianShanIce::Storage::csNotProvisioned);
	metaDataMap.insert(std::make_pair( CONTENTSTATE, metaData));


	metaData.value = CONTENTREPLICA;
	metaDataMap.insert(std::make_pair(OBJECTTYPE, metaData));
	_lib.setMetaDatas(objId, metaDataMap, false);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "ContentReplica [%s] added"), objId.c_str());
}

void ContentLibImpl::addContentStoreReplica(const std::string& strNetId, const std::string& strEndpoint)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "addContentStoreReplica(%s, %s)"), strNetId.c_str(), strEndpoint.c_str());
	TianShanIce::Properties searchForMetaData;
	TianShanIce::StrValues expectedMetaDataNames;
	::TianShanIce::Repository::MetaDataValue metaData;
	metaData.hintedType = ::TianShanIce::vtStrings;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, strNetId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(ENDPOINT, strEndpoint));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, CONTENTSTOREREPLICA));
	TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(CONTENTSTOREREPLICA, searchForMetaData, expectedMetaDataNames);
	if(ret.size() <= 0) // not exist in db, then add, else check if need sync
	{
		::Ice::Identity identObj;
		identObj.category = META_OBJECT;
		identObj.name = strNetId;
		try {
			ContentStoreReplicaImplPtr pObj = new ContentStoreReplicaImpl(_lib);
			pObj->ident = identObj;
			pObj->type = CONTENTSTOREREPLICA;
			pObj->stampCreated = ZQTianShan::now(); ///< timestamp as of created
			TianShanIce::Storage::ContentStorePrx contentStoreProxy;
			if (!getContentStoreProxy(strNetId, contentStoreProxy))
			{
				return;
			}
			pObj->contentstore = contentStoreProxy;
			pObj->netId = strNetId;
			pObj->endpoint = strEndpoint;
			pObj->lastSync = ZQTianShan::now();
			::TianShanIce::Repository::MetaDataMap metaDataMap;
			metaData.value = strNetId;
			MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, NETID, metaData);
			metaData.value = strEndpoint;
			MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, ENDPOINT, metaData);
			pObj->metaDataMap = metaDataMap;
			_lib.addObject(pObj, identObj);
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "addContentStoreReplica() added new obj[%s:%s]"), identObj.name.c_str(), identObj.category.c_str());
		}
		catch (Ice::Exception& ex) 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "addContentStoreReplica(%s) ice exception[%s]"), identObj.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "addContentStoreReplica(%s) unknown exception"), identObj.name.c_str());
		}

		std::string objId = strNetId;

		::TianShanIce::Repository::MetaDataMap metaDataMap;
		metaData.value = strNetId;
		metaDataMap.insert(std::make_pair(NETID, metaData));

		metaData.value = strEndpoint;
		metaDataMap.insert(std::make_pair(ENDPOINT, metaData));

		metaData.value = CONTENTSTOREREPLICA;
		metaDataMap.insert(std::make_pair(OBJECTTYPE, metaData));
		_lib.setMetaDatas(objId, metaDataMap, false);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "ContentStoreReplica [%s] added"), strNetId.c_str());

//		updateContentStore(strNetId);	// add other info in DB
		try
		{
			FullSyncRequest* pRequest = new FullSyncRequest(_env, strNetId);
			if (pRequest)
			{
				pRequest->start();
			}
			else 
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create sync request for [%s] failed"), strNetId.c_str());
			}
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "sync request caught unexcepted exception for [%s]"), strNetId.c_str());
		}

	}
	else
	{
		// sync if timestamp is out-of-date
		try
		{
			::TianShanIce::Repository::ContentStoreReplicaPrx csPrx = toStoreReplica(strNetId);
			if(csPrx)
			{
				::TianShanIce::Repository::ContentStoreReplicaExPrx cePrx = ::TianShanIce::Repository::ContentStoreReplicaExPrx::checkedCast(csPrx);
				Ice::Long lastModified = cePrx->getLastSync();
				if( ZQTianShan::now() - lastModified > timeToSync)
				{
//					syncContentStore(strNetId);
//					SyncContentStoreRequest* pRequest = new SyncContentStoreRequest(_env, strNetId);
					FullSyncRequest* pRequest = new FullSyncRequest(_env, strNetId);
					if (pRequest)
					{
						pRequest->start();
					}
					else 
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create sync request for [%s] failed"), strNetId.c_str());
						
					}
				}
			}
		}
		catch (TianShanIce::EntityNotFound)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toStoreReplica() error : Entity Not Found"));
		}
		catch (TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toStoreReplica() caught(%s: %s)")
				, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toStoreReplica() caught(%s)")
				, ex.ice_name().c_str());
		}
		catch (...)
		{
		}
	}
}

void ContentLibImpl::addMetaVolume(const std::string& strNetId, const std::string& strVolume)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "addMetaVolume(%s, %s)"), strNetId.c_str(), strVolume.c_str());
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	TianShanIce::Storage::VolumePrx volumeProxy;
	Ice::Long lFreeSpace, lTotalSpace;
	if (!getContentStoreProxy(strNetId, contentStoreProxy))
	{
		return;
	}
	try
	{
		volumeProxy = contentStoreProxy->openVolume(strVolume);
		if (!volumeProxy)
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openVolume() : volume proxy is empty"));
			return;
		}
		else
		{
			volumeProxy->getCapacity(lFreeSpace, lTotalSpace);
		}
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openVolume() : catch an ice exception [%s] when open volume [%s]"), ex.ice_name().c_str(), strVolume.c_str());
		return;
	}

	::TianShanIce::Repository::MetaDataValue metaData;
	metaData.hintedType = ::TianShanIce::vtStrings;

	::Ice::Identity identObj;
	identObj.category = META_OBJECT;
	identObj.name = strNetId + "$" + strVolume;
	try {
		MetaVolumeImplPtr pObj = new MetaVolumeImpl(_lib);
		pObj->ident = identObj;
		pObj->type = METAVOLUME;
		pObj->stampCreated = ZQTianShan::now(); ///< timestamp as of created
		pObj->volume = volumeProxy;
		pObj->netId = strNetId;
		pObj->volumeName = strVolume;
		::TianShanIce::Repository::MetaDataMap metaDataMap;
		metaData.value = strNetId;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, NETID, metaData);
		metaData.value = strVolume;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, VOLUMENAME, metaData);
		char buf[256] = { 0 };
		snprintf(buf, sizeof(buf), FMT64, lFreeSpace);
		metaData.value = buf;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, FREESPACE, metaData);
		snprintf(buf, sizeof(buf), FMT64, lTotalSpace);
		metaData.value = buf;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, TOTALSPACE, metaData);
		pObj->metaDataMap = metaDataMap;
		_lib.addObject(pObj, identObj);	
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "openObject() added new obj[%s:%s]"), identObj.name.c_str(), identObj.category.c_str());
	}
	catch (Ice::AlreadyRegisteredException&)
	{
		return;
	}
	catch (Ice::Exception& ex) 
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(MetaLib, 5003, "openObject() ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, EXPFMT(MetaLib, 5003, "openObject() unknown exception"));
	}

	std::string objId = strNetId + "$" + strVolume;

	::TianShanIce::Repository::MetaDataMap metaDataMap;
	metaData.value = strNetId;
	metaDataMap.insert(std::make_pair(NETID, metaData));

	metaData.value = strVolume;
	metaDataMap.insert(std::make_pair(VOLUMENAME, metaData));

	metaData.value = METAVOLUME;
	metaDataMap.insert(std::make_pair(OBJECTTYPE, metaData));
	_lib.setMetaDatas(objId, metaDataMap, false);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "MetaVolume [%s] added"), objId.c_str());
}

void ContentLibImpl::deleteContentReplica(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName)
{
	std::string objId = strContentName + "@" + strNetId + "$" + strVolume;
	try
	{
		_lib.proxy()->removeObject(objId);
		_env._pContentCacheThread->removeContent(strNetId, strVolume, strContentName);
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "deleteContentReplica(%s) caught(%s: %s)"), objId.c_str()
			, ex.ice_name().c_str(), ex.message.c_str());
		return;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "deleteContentReplica(%s) caught(%s)"), objId.c_str()
			, ex.ice_name().c_str());
		return;
	}
	catch (...)
	{
		return;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "ContentReplica [%s] deleted"), objId.c_str());
}

void ContentLibImpl::deleteContentStoreReplica(const std::string& strNetId)
{
	_lib.proxy()->removeObject(strNetId);
	_env._pContentCacheThread->notify();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "ContentStoreReplica [%s] deleted"), strNetId.c_str());
}

void ContentLibImpl::deleteMetaVolume(const std::string& strNetId, const std::string& strVolume)
{
	std::string objId = strNetId + "$" + strVolume;
	//delete relative contents first
	TianShanIce::Properties searchForMetaData;
	TianShanIce::StrValues expectedMetaDataNames;
	::TianShanIce::Repository::MetaDataValue metaData;
	metaData.hintedType = ::TianShanIce::vtStrings;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, strNetId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(VOLUMENAME, strVolume));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, CONTENTREPLICA));
	TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(CONTENTREPLICA, searchForMetaData, expectedMetaDataNames);
	for(TianShanIce::Repository::MetaObjectInfos::iterator it = ret.begin(); it != ret.end(); it++)
	{
//		if(it->type == CONTENTREPLICA)
		{
			try
			{
				_lib.proxy()->removeObject(it->id);
			}
			catch (TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "[%s] content deletion while deleteMetaVolume(%s) caught(%s: %s)"), it->id.c_str(), objId.c_str()
					, ex.ice_name().c_str(), ex.message.c_str());
				continue;
			}
			catch (Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "[%s] content deletion while deleteMetaVolume(%s) caught(%s)"), it->id.c_str(), objId.c_str()
					, ex.ice_name().c_str());
				continue;
			}
			catch (...)
			{
				continue;
			}
		}
	}
	try
	{
		_lib.proxy()->removeObject(objId);
		_env._pContentCacheThread->notify();
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "deleteMetaVolume(%s) caught(%s: %s)"), objId.c_str()
			, ex.ice_name().c_str(), ex.message.c_str());
		return;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "deleteMetaVolume(%s) caught(%s)"), objId.c_str()
			, ex.ice_name().c_str());
		return;
	}
	catch (...)
	{
		return;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "MetaVolume [%s] deleted"), objId.c_str());
}

void ContentLibImpl::updateContentMetadata(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "updateContentMetadata(%s, %s, %s)"), strNetId.c_str(), strVolume.c_str(), strContentName.c_str());
	::TianShanIce::Repository::ContentReplicaExPrx proxy;
	Ice::Identity ident;	
	ident.name = strContentName + "@" + strNetId + "$" + strVolume;
	ident.category = META_OBJECT;
	try
	{
		proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_lib._adapter->createProxy(ident));
		if(!proxy)
		{
			std::string contentReplicaId = strNetId + "$" + strVolume + "/" +  strContentName;
			proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(toContentReplica(contentReplicaId));
		}
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "updateContentMetadata get ContentReplicaEx Proxy[%s] caught(%s)"), ident.name.c_str()
			, ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "updateContentMetadata get ContentReplicaEx Proxy[%s] caught unknown error"), ident.name.c_str());
	}
	if(proxy)
	{
		try
		{
			proxy->getUpdateMetaData();
			proxy->getUpdateState();
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "updateContentMetadata [%s] UpdateMetaData caught(%s)"), ident.name.c_str()
				, ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "updateContentMetadata [%s] UpdateMetaData caught unknown error"), ident.name.c_str());
		}
	}
}

bool ContentLibImpl::getContentStoreProxy(const std::string& strNetId, TianShanIce::Storage::ContentStorePrx& contentStoreProxy)
{
	ZQ::common::MutexGuard mg(_mutex);
	std::map<std::string, TianShanIce::Storage::ContentStorePrx>::iterator contentStroeIt;
	contentStroeIt = _contentStroeProxies.find(strNetId);
	if (contentStroeIt == _contentStroeProxies.end())
	{
		// try to find from Database
		try
		{
			::TianShanIce::Repository::ContentStoreReplicaPrx csPrx = toStoreReplica(strNetId);
			if(csPrx)
			{
				contentStoreProxy = csPrx->theStore();
				_contentStroeProxies.insert(std::make_pair(strNetId, contentStoreProxy));
				return true;
			}
			else
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "getContentStoreProxy() : No content store net id match [%s]"), strNetId.c_str());
				return false;
			}
		}
		catch (TianShanIce::EntityNotFound)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "getContentStoreProxy() error : Entity Not Found"));
			return false;
		}
		catch (TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "getContentStoreProxy() caught(%s: %s)")
				, ex.ice_name().c_str(), ex.message.c_str());
			return false;
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "getContentStoreProxy() caught(%s)")
				, ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "getContentStoreProxy() unknown error"));
			return false;
		}
	}
	contentStoreProxy = contentStroeIt->second;
	return true;
}

bool ContentLibImpl::connectContentStore(const std::string& strEndpoint, std::string& strNetId)
{
	ZQ::common::MutexGuard mg(_mutex);
	bool bSuccess = false;
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	try
	{
		Ice::ObjectPrx base = _lib._adapter->getCommunicator()->stringToProxy(strEndpoint);
		contentStoreProxy = TianShanIce::Storage::ContentStorePrx::checkedCast(base);
		std::string stringToProxy = _lib._adapter->getCommunicator()->proxyToString(contentStoreProxy);
		if (!contentStoreProxy)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "connectContentStore() : Fail to get content store [%s] proxy"), strEndpoint.c_str());
			return false;
		}
		strNetId = contentStoreProxy->getNetId();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "connectContentStore() : get content store [%s] proxy caught an exception[%s]"), strEndpoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	bSuccess = true;
	if(_contentStroeProxies.find(strNetId) == _contentStroeProxies.end())
		_contentStroeProxies.insert(std::make_pair(strNetId, contentStoreProxy));
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(ContentLibImpl, "connectContentStore() : connect with endpoint [%s], net Id [%s]"), strEndpoint.c_str(), strNetId.c_str());
	return bSuccess;
}

bool ContentLibImpl::updateContentStore(const std::string& strNetId)
{
/*
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "updateContentStore(%s)"), strNetId.c_str());
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	if (!getContentStoreProxy(strNetId, contentStoreProxy))
	{
		return false;
	}
	TianShanIce::Storage::VolumeInfos volumeInfos;
	try
	{
		volumeInfos = contentStoreProxy->listVolumes("*", true);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "getVolumesFromStore() : catch an ice exception [%s] when list all volume"), ex.ice_name().c_str());
		return false;
	}
	for (TianShanIce::Storage::VolumeInfos::iterator volumeIt = volumeInfos.begin();
		volumeIt != volumeInfos.end(); volumeIt++)
	{
		try
		{
			TianShanIce::Storage::VolumePrx volumeProxy = contentStoreProxy->openVolume(volumeIt->name);
			if (!volumeProxy)
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openVolume() : volume proxy is empty"));
				continue;
			}
			addMetaVolume(strNetId, volumeIt->name); // store in DB
			TianShanIce::Storage::ContentInfos contentInfos;
			contentInfos.clear();
			try
			{
				TianShanIce::StrValues metaDataNames;
				ContentLibConfig::MetadataCategories::iterator mt_iter = _config.categories.begin();
				for (; mt_iter != _config.categories.end(); ++mt_iter) 
				{
					metaDataNames.push_back(mt_iter->category);
				}
// 				metaDataNames.push_back(METADATA_ProviderId);
// 				metaDataNames.push_back(METADATA_ProviderAssetId);
// 				metaDataNames.push_back(METADATA_nPVRCopy);
// 				metaDataNames.push_back(METADATA_SubscriberId);
// 				metaDataNames.push_back(METADATA_SourceUrl);
// 				metaDataNames.push_back(METADATA_ParentName);
// 				metaDataNames.push_back(METADATA_Comment);
// 				metaDataNames.push_back(METADATA_FileSize);
// 				metaDataNames.push_back(METADATA_SupportFileSize);
// 				metaDataNames.push_back(METADATA_PixelHorizontal);
// 				metaDataNames.push_back(METADATA_PixelVertical);
// 				metaDataNames.push_back(METADATA_BitRate);
// 				metaDataNames.push_back(METADATA_PlayTime);
// 				metaDataNames.push_back(METADATA_FrameRate);
// 				metaDataNames.push_back(METADATA_SourceType);
// 				metaDataNames.push_back(METADATA_LocalType);
// 				metaDataNames.push_back(METADATA_SubType);
// 				metaDataNames.push_back(METADATA_MD5CheckSum);
// 				metaDataNames.push_back(METADATA_ScheduledProvisonStart);
// 				metaDataNames.push_back(METADATA_ScheduledProvisonEnd);
// 				metaDataNames.push_back(METADATA_MaxProvisonBitRate);
// 				metaDataNames.push_back(METADATA_nPVRLeadCopy);
				::Ice::Long stampBegin = ZQTianShan::now();
				TianShanIce::Storage::VolumeExPrx  volumeExProxy = TianShanIce::Storage::VolumeExPrx::checkedCast(volumeProxy);
				if(!volumeExProxy)
					continue;
				TianShanIce::Storage::CachedContentListPrx cacheProxy = volumeExProxy->cachedListContents(3600000);
				if(!cacheProxy)
					continue;
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "cachedListContents , took %lldms"), ZQTianShan::now() - stampBegin);
				do 
				{
					
					if(!contentInfos.empty())
					{
						::Ice::Long nextBegin = ZQTianShan::now();
						TianShanIce::Storage::ContentInfos::const_iterator contentIt = contentInfos.end() - 1;
//						contentInfos = volumeProxy->listContents(metaDataNames, contentIt->name, PageSize);
						contentInfos = cacheProxy->next(metaDataNames, contentIt->name, PageSize, 3600000);
						glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "next , took %lldms"), ZQTianShan::now() - nextBegin);
						glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "listContents of volume[%s] , %d contents"), volumeIt->name.c_str(), contentInfos.size());
						Ice::Long syncStamp = ZQ::common::now();
						for (contentIt = contentInfos.begin() + 1; contentIt != contentInfos.end(); contentIt++)
						{
							TianShanIce::Storage::ContentPrx contentProxy;
							try
							{
								contentProxy = contentStoreProxy->openContentByFullname(contentIt->fullname);
								if (!contentProxy)
								{
									glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openContentByFullName(%s) : content proxy is empty"), contentIt->fullname.c_str());
									continue;
								}
							}
							catch (const Ice::Exception& ex)
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), contentIt->fullname.c_str());
								continue;
							}
							AddContentRequest* pRequest = new AddContentRequest(_env, strNetId, volumeIt->name, contentIt->name, *contentIt, contentProxy);
							if (pRequest)
							{
								pRequest->start();
							}
							else 
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create add content request for [%s@%s$%s] failed"), contentIt->name.c_str(), strNetId.c_str(), volumeIt->name.c_str());
							}
							if(_config.maxContent > 0 && contentCount++ > _config.maxContent)
							{
								glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "reach max content count [%d]"), _config.maxContent);
								return true;
							}
						} // end for all content
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "start to 1000 sync took %lld ms"), ZQ::common::now()-syncStamp);
					}
					else
					{
						::Ice::Long nextBegin = ZQTianShan::now();
//						contentInfos = volumeProxy->listContents(metaDataNames, "", PageSize);
						contentInfos = cacheProxy->next(metaDataNames, "", PageSize, 3600000);
						glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "next , took %lldms"), ZQTianShan::now() - nextBegin);
						glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "listContents of volume[%s] , %d contents"), volumeIt->name.c_str(), contentInfos.size());
						Ice::Long syncStamp = ZQ::common::now();
						for (TianShanIce::Storage::ContentInfos::const_iterator contentIt = contentInfos.begin(); 
							contentIt != contentInfos.end(); contentIt++)
						{
							TianShanIce::Storage::ContentPrx contentProxy;
							try
							{
								contentProxy = contentStoreProxy->openContentByFullname(contentIt->fullname);
								if (!contentProxy)
								{
									glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openContentByFullName(%s) : content proxy is empty"), contentIt->fullname.c_str());
									continue;
								}
							}
							catch (const Ice::Exception& ex)
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), contentIt->fullname.c_str());
								continue;
							}
							AddContentRequest* pRequest = new AddContentRequest(_env, strNetId, volumeIt->name, contentIt->name, *contentIt, contentProxy);
							if (pRequest)
							{
								pRequest->start();
							}
							else 
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create add content request for [%s@%s$%s] failed"), contentIt->name.c_str(), strNetId.c_str(), volumeIt->name.c_str());
							}
							if(_config.maxContent > 0 && contentCount++ > _config.maxContent)
							{
								glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "reach max content count [%d]"), _config.maxContent);
								return true;
							}
						} // end for all content
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "start to 1000 sync took %lld ms"), ZQ::common::now()-syncStamp);
					}
				} while (contentInfos.size() >= PageSize);
				cacheProxy->destory();
				glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "listContents of volume[%s] , took %lldms"), volumeIt->name.c_str(), ZQTianShan::now() - stampBegin);
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "listContent() : catch an exception[%s] when list content in volume[%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
				continue;
			}
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openVolume() : catch an ice exception [%s] when open volume [%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
			continue;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "getVolumesFromStore() : load volume[%s]"),(volumeIt->name).c_str());
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "updateContentStore() : leave update content store[%s]"), strNetId.c_str());
*/	
	return true;
}

bool ContentLibImpl::syncContentStore(const std::string& strNetId)
{
/*
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "syncContentStore(%s)"), strNetId.c_str());
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	if (!getContentStoreProxy(strNetId, contentStoreProxy))
	{
		return false;
	}
	// 1.list volumes from ContentStore
	TianShanIce::Storage::VolumeInfos volumeInfos;
	try
	{
		volumeInfos = contentStoreProxy->listVolumes("*", true);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "getVolumesFromStore() : catch an ice exception [%s] when list all volume"), ex.ice_name().c_str());
		return false;
	}

	// 2.list volumes in database
	TianShanIce::StrValues volumesInDB;
	TianShanIce::Properties searchForMetaData;
	TianShanIce::StrValues expectedMetaDataNames;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, strNetId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, METAVOLUME));
	TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(METAVOLUME, searchForMetaData, expectedMetaDataNames);
	for (TianShanIce::Repository::MetaObjectInfos::iterator itObj = ret.begin(); itObj < ret.end(); itObj++)
	{
		if(itObj->type == METAVOLUME)
			volumesInDB.push_back(itObj->id);
	}
	//sort by name
	std::sort(volumesInDB.begin(), volumesInDB.end());

	// 3.start the sync process
	int j = 0, k = 0;
	while (j < (int)volumeInfos.size() && k < (int)volumesInDB.size())
	{
		// if names equal
		if (volumeInfos[j].name == volumesInDB[k].substr(volumesInDB[k].find("$") + 1))
		{
			j ++;
			k ++;
			continue;
		}

		// if volume in DB is different from ContentStore's
		int i, count;
		bool bInContentStore = false; // to indicates whether or not the volume is in ContentStore list
		for (i = j, count = (int)volumeInfos.size(); i < count; i ++)
		{	// notice here the initial value of i is j
			// means the volume list from ContentStore is a sub list which after the position of current volume
			if (volumesInDB[k].substr(volumesInDB[k].find("$") + 1) == volumeInfos[i].name)
			{
				// volume in DB is also in ContentStore list
				// means it is a new one
				bInContentStore = true;
				break;
			}
		}

		if (bInContentStore)
		{
			// volume from ContentStore is a new one, so we have to add it into DB
			try
			{
				TianShanIce::Storage::VolumePrx volumeProxy = contentStoreProxy->openVolume(volumeInfos[j].name);
				if (!volumeProxy)
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openVolume() : volume proxy is empty"));
					j++;
					continue;
				}
				addMetaVolume(strNetId, volumeInfos[j].name); // store in DB
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openVolume() : catch an ice exception [%s] when open volume [%s]"), ex.ice_name().c_str(), (volumeInfos[j].name).c_str());
			}
			j ++;
		}
		else 
		{
			// current volume is not in ContentSotre list, so we have to erase it
			std::string volumeName= volumesInDB[k].substr(volumesInDB[k].find("$") + 1);
			deleteMetaVolume(strNetId, volumeName);
			k ++;
		}
	}

	while (j < (int)volumeInfos.size())
	{
		//add volumeInfos[j]
		try
		{
			TianShanIce::Storage::VolumePrx volumeProxy = contentStoreProxy->openVolume(volumeInfos[j].name);
			if (!volumeProxy)
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openVolume() : volume proxy is empty"));
				j++;
				continue;
			}
			addMetaVolume(strNetId, volumeInfos[j].name); // store in DB
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openVolume() : catch an ice exception [%s] when open volume [%s]"), ex.ice_name().c_str(), (volumeInfos[j].name).c_str());
		}
		j ++;
	}

	while (k < (int)volumesInDB.size())
	{
		std::string volumeName= volumesInDB[k].substr(volumesInDB[k].find("$") + 1);
		deleteMetaVolume(strNetId, volumeName);
		k ++;
	}

	//sync contents for each volume
	for (TianShanIce::Storage::VolumeInfos::iterator volumeIt = volumeInfos.begin();
		volumeIt != volumeInfos.end(); volumeIt++)
	{
		try
		{
			TianShanIce::Storage::VolumePrx volumeProxy = contentStoreProxy->openVolume(volumeIt->name);
			if (!volumeProxy)
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "openVolume() : volume proxy is empty"));
				continue;
			}
			//addMetaVolume(strNetId, volumeIt->name, volumeProxy); // store in DB
			TianShanIce::Storage::ContentInfos contentInfos;
			try
			{
				//	1.list contents from volume
				TianShanIce::StrValues metaDataNames;
				ContentLibConfig::MetadataCategories::iterator mt_iter = _config.categories.begin();
				for (; mt_iter != _config.categories.end(); ++mt_iter) 
				{
					metaDataNames.push_back(mt_iter->category);
				}
//				metaDataNames.push_back("*");
// 				metaDataNames.push_back(METADATA_ProviderId);
// 				metaDataNames.push_back(METADATA_ProviderAssetId);
// 				metaDataNames.push_back(METADATA_nPVRCopy);
// 				metaDataNames.push_back(METADATA_SubscriberId);
// 				metaDataNames.push_back(METADATA_SourceUrl);
// 				metaDataNames.push_back(METADATA_ParentName);
// 				metaDataNames.push_back(METADATA_Comment);
// 				metaDataNames.push_back(METADATA_FileSize);
// 				metaDataNames.push_back(METADATA_SupportFileSize);
// 				metaDataNames.push_back(METADATA_PixelHorizontal);
// 				metaDataNames.push_back(METADATA_PixelVertical);
// 				metaDataNames.push_back(METADATA_BitRate);
// 				metaDataNames.push_back(METADATA_PlayTime);
// 				metaDataNames.push_back(METADATA_FrameRate);
// 				metaDataNames.push_back(METADATA_SourceType);
// 				metaDataNames.push_back(METADATA_LocalType);
// 				metaDataNames.push_back(METADATA_SubType);
// 				metaDataNames.push_back(METADATA_MD5CheckSum);
// 				metaDataNames.push_back(METADATA_ScheduledProvisonStart);
// 				metaDataNames.push_back(METADATA_ScheduledProvisonEnd);
// 				metaDataNames.push_back(METADATA_MaxProvisonBitRate);
// 				metaDataNames.push_back(METADATA_nPVRLeadCopy);
//				contentInfos = volumeProxy->listContents(metaDataNames, "", -1);

				TianShanIce::Storage::ContentInfos tempInfos;
				tempInfos.clear();
				try{
					do 
					{
						if(!tempInfos.empty())
						{
							TianShanIce::Storage::ContentInfos::const_iterator contentIt = tempInfos.end() - 1;
							tempInfos = volumeProxy->listContents(metaDataNames, contentIt->name, PageSize);
							for (contentIt = tempInfos.begin() + 1; contentIt != tempInfos.end(); contentIt++)
							{
								contentInfos.push_back(*contentIt);
							}
						}
						else
						{
							tempInfos = volumeProxy->listContents(metaDataNames, "", PageSize);
							for (TianShanIce::Storage::ContentInfos::const_iterator contentIt = tempInfos.begin(); contentIt != tempInfos.end(); contentIt++)
							{
								contentInfos.push_back(*contentIt);
							}
						}
					} while (tempInfos.size() >= PageSize);
				}
				catch (const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "listContent() : catch an exception[%s] when list content in volume[%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
					continue;
				}

				// 2.list contents in database
				TianShanIce::StrValues contentsInDB;
				TianShanIce::Properties searchForMetaData;
				TianShanIce::StrValues expectedMetaDataNames;
				searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, strNetId));
				searchForMetaData.insert(TianShanIce::Properties::value_type(VOLUMENAME, volumeIt->name));
				searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, CONTENTREPLICA));
				try{
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "start to lookup contents from database of [%s]"), (volumeIt->name).c_str());
					TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup(CONTENTREPLICA, searchForMetaData, expectedMetaDataNames);
					for (TianShanIce::Repository::MetaObjectInfos::iterator itObj = ret.begin(); itObj < ret.end(); itObj++)
					{
//						if(itObj->type == CONTENTREPLICA)
							contentsInDB.push_back(itObj->id.substr(0, itObj->id.find("@")));
					}
					//sort by name
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "start to sort"));
					std::sort(contentsInDB.begin(), contentsInDB.end());
				}
				catch (const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "lookup() : catch an exception[%s] when lookup content in volume[%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
					continue;
				}
				catch (...)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "lookup() : catch an unexpected exception when lookup content in volume[%s]"), (volumeIt->name).c_str());
					continue;
				}

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "start to sync"));
				// 3.start the sync process
				int j = 0, k = 0;
				while (j < (int)contentInfos.size() && k < (int)contentsInDB.size())
				{
					// if names equal, further check if lastModified is same
					if (contentInfos[j].name == contentsInDB[k])
					{
						if(contentInfos[j].metaData.find(METADATA_StampLastUpdated) == contentInfos[j].metaData.end())
						{
							j ++;
							k ++;
							continue;
						}
						::TianShanIce::Repository::ContentReplicaExPrx proxy;
						Ice::Identity ident;	
						ident.name = contentsInDB[k] + "@" + strNetId + "$" + volumeIt->name;
						ident.category = META_OBJECT;
						std::string timeStamp;
						try
						{
							proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_lib._adapter->createProxy(ident));
							if(!proxy)
							{
								std::string contentReplicaId = strNetId + "$" + volumeIt->name + "/" +  contentInfos[j].name;
								proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(toContentReplica(contentReplicaId));
							}
							if(proxy)
							{
								timeStamp = proxy->getLastModified();
							}
						}
						catch (const Ice::Exception&)
						{
							j ++;
							k ++;
							continue;
						}
						if (contentInfos[j].metaData[METADATA_StampLastUpdated] != timeStamp && (contentInfos[j].state == TianShanIce::Storage::ContentState::csInService) && (contentInfos[j].metaData.size() > 0))
						{
							// means it has been modified
// 							std::string ident = contentInfos[j].name + "@" + strNetId + "$" + volumeIt->name;
// 							SyncItem item(SyncItem::itContent, SyncItem::stUpdate, ident);
// 							item.setInfo(contentInfos[j]);
// 							_env._pSyncThread->push(item);
// 							_env._pSyncThread->notify();
//							updateContentReplica(strNetId, volumeIt->name, contentInfos[j].name, contentInfos[j]);
							UpdateContentRequest* pRequest = new UpdateContentRequest(_env, strNetId, volumeIt->name, contentInfos[j].name, contentInfos[j]);
							if (pRequest)
							{
								pRequest->start();
							}
							else 
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create update content request for [%s@%s$%s] failed"), contentInfos[j].name.c_str(), strNetId.c_str(), volumeIt->name.c_str());
							}
						}
						j ++;
						k ++;
						continue;
					}

					// if content in DB is different from ContentStore's
					int i, count;
					bool bInContentStore = false; // to indicates whether or not the content is in ContentStore list
					for (i = j, count = (int)contentInfos.size(); i < count; i ++)
					{	// notice here the initial value of i is j
						// means the content list from ContentStore is a sub list which after the position of current content
						if (contentsInDB[k] == contentInfos[i].name)
						{
							// content in DB is also in ContentStore list
							// means it is a new one
							bInContentStore = true;
							break;
						}
					}

					if (bInContentStore)
					{
						// content from ContentStore is a new one, so we have to add it into DB
// 						std::string ident = contentInfos[j].name + "@" + strNetId + "$" + volumeIt->name;
// 						SyncItem item(SyncItem::itContent, SyncItem::stAddandUpdate, ident);
// 						item.setInfo(contentInfos[j]);
// 						_env._pSyncThread->push(item);
// 						_env._pSyncThread->notify();
						TianShanIce::Storage::ContentPrx contentProxy;
						try
						{
							contentProxy = contentStoreProxy->openContentByFullname(contentInfos[j].fullname);
							if (!contentProxy)
							{
								glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openContentByFullName(%s) : content proxy is empty"), contentInfos[j].fullname.c_str());
							}
							else
							{
								AddContentRequest* pRequest = new AddContentRequest(_env, strNetId, volumeIt->name, contentInfos[j].name, contentInfos[j], contentProxy);
								if (pRequest)
								{
									pRequest->start();
								}
								else 
								{
									glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create add content request for [%s@%s$%s] failed"), contentInfos[j].name.c_str(), strNetId.c_str(), volumeIt->name.c_str());
								}
							}
						}
						catch (const Ice::Exception& ex)
						{
							glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), contentInfos[j].fullname.c_str());
						}
						j ++;
					}
					else 
					{
// 						std::string ident = contentsInDB[k] + "@" + strNetId + "$" + volumeIt->name;
// 						SyncItem item(SyncItem::itContent, SyncItem::stDelete, ident);
// 						_env._pSyncThread->push(item);
// 						_env._pSyncThread->notify();
//						deleteContentReplica(strNetId, volumeIt->name, contentsInDB[k]);
						DeleteContentRequest* pRequest = new DeleteContentRequest(_env, strNetId, volumeIt->name, contentsInDB[k]);
						if (pRequest)
						{
							pRequest->start();
						}
						else 
						{
							glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create delete content request for [%s@%s$%s] failed"), contentsInDB[k].c_str(), strNetId.c_str(), volumeIt->name.c_str());
						}
						k ++;
					}
				}

				while (j < (int)contentInfos.size())
				{
					std::string ProviderId;
					std::string ProviderAssetId;
					TianShanIce::Storage::ContentPrx contentProxy;
					try
					{
						contentProxy = contentStoreProxy->openContentByFullname(contentInfos[j].fullname);
						if (!contentProxy)
						{
							glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openContentByFullName(%s) : content proxy is empty"), contentInfos[j].fullname.c_str());
							j++;
							continue;
						}
					}
					catch (const Ice::Exception& ex)
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), contentInfos[j].fullname.c_str());
						j++;
						continue;
					}
// 					std::string ident = contentInfos[j].name + "@" + strNetId + "$" + volumeIt->name;
// 					SyncItem item(SyncItem::itContent, SyncItem::stAddandUpdate, ident);
// 					item.setInfo(contentInfos[j]);
// 					_env._pSyncThread->push(item);
// 					_env._pSyncThread->notify();
					AddContentRequest* pRequest = new AddContentRequest(_env, strNetId, volumeIt->name, contentInfos[j].name, contentInfos[j], contentProxy);
					if (pRequest)
					{
						pRequest->start();
					}
					else 
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create add content request for [%s@%s$%s] failed"), contentInfos[j].name.c_str(), strNetId.c_str(), volumeIt->name.c_str());
					}
					j ++;
				}

				while (k < (int)contentsInDB.size())
				{
// 					std::string ident = contentsInDB[k] + "@" + strNetId + "$" + volumeIt->name;
// 					SyncItem item(SyncItem::itContent, SyncItem::stDelete, ident);
// 					_env._pSyncThread->push(item);
// 					_env._pSyncThread->notify();
//					deleteContentReplica(strNetId, volumeIt->name, contentsInDB[k]);
					DeleteContentRequest* pRequest = new DeleteContentRequest(_env, strNetId, volumeIt->name, contentsInDB[k]);
					if (pRequest)
					{
						pRequest->start();
					}
					else 
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create delete content request for [%s@%s$%s] failed"), contentsInDB[k].c_str(), strNetId.c_str(), volumeIt->name.c_str());
					}
					k ++;
				}
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "sync() : catch an exception[%s] when sync in volume[%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
				continue;
			}
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openVolume() : catch an ice exception [%s] when open volume [%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
			continue;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "getVolumesFromStore() : load volume[%s]"),(volumeIt->name).c_str());
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "syncContentStore() : leave sync ContentStore[%s]"), strNetId.c_str());
	
	//set last modified
	try
	{
		::TianShanIce::Repository::ContentStoreReplicaPrx csPrx = toStoreReplica(strNetId);
		if(csPrx)
		{
			::TianShanIce::Repository::ContentStoreReplicaExPrx cePrx = ::TianShanIce::Repository::ContentStoreReplicaExPrx::checkedCast(csPrx);
			cePrx->setLastSync(ZQTianShan::now());
		}
	}
	catch (TianShanIce::EntityNotFound)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "toStoreReplica() error : Entity Not Found"));
		return false;
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "setLastSync() caught(%s: %s)")
			, ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "setLastSync() caught(%s)")
			, ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		return false;
	}
*/
	return true;
}

bool ContentLibImpl::removeContentStore(const std::string&strNetId, const std::string& strEndpoint)
{
	ZQ::common::MutexGuard mg(_mutex);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "removeContentStore(%s)"), strNetId.c_str());
	// remove from map
	std::map<std::string, TianShanIce::Storage::ContentStorePrx>::iterator contentStroeIt;
	contentStroeIt = _contentStroeProxies.find(strNetId);
	if (contentStroeIt != _contentStroeProxies.end())
	{
		_contentStroeProxies.erase(contentStroeIt);
	}
	//remove content store records in DB
	TianShanIce::Properties searchForMetaData;
	TianShanIce::StrValues expectedMetaDataNames;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, strNetId));
	TianShanIce::Repository::MetaObjectInfos ret = _lib.proxy()->lookup("", searchForMetaData, expectedMetaDataNames);
	for (TianShanIce::Repository::MetaObjectInfos::iterator itObj = ret.begin(); itObj < ret.end(); itObj++)
	{	
		Ice::Identity ident;
		ident.name = itObj->id;
		ident.category = META_OBJECT;
		_lib.proxy()->removeObject(itObj->id);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "remove from MetaLib[%s:%s]"), itObj->id.c_str(), itObj->type.c_str());
		//remove instance in Adapter
		if(_lib._adapter->find(ident))
		{
			_lib._adapter->remove(ident);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "remove from Adapter[%s:%s]"), ident.name.c_str(), ident.category.c_str());
		}
	}
	return true;
}

void ContentLibImpl::updateContentReplica(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName, const ::TianShanIce::Storage::ContentInfo& Info)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "updateContentReplica(%s, %s, %s)"), strNetId.c_str(), strVolume.c_str(), strContentName.c_str());
	std::string objId = strContentName + "@" + strNetId + "$" + strVolume;
	try
	{
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.hintedType = ::TianShanIce::vtStrings;

//		if((Info.state == TianShanIce::Storage::csInService) && !Info.metaData.empty())
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibImpl, "updateContentMetadata(%s, %s, %s)"), strNetId.c_str(), strVolume.c_str(), strContentName.c_str());
			::TianShanIce::Repository::MetaDataMap metaDataMap, indexMetaDataMap;
			metaData.value = convertState(Info.state);
			metaDataMap.insert(std::make_pair(CONTENTSTATE, metaData));
			for(::TianShanIce::Properties::const_iterator it = Info.metaData.begin(); it != Info.metaData.end(); it++)
			{
				metaData.value = it->second;
				metaDataMap.insert(make_pair(it->first, metaData));
			}
			for (ContentLibConfig::MetadataCategories::iterator iter = _config.categories.begin(); iter != _config.categories.end(); ++iter) 
			{
				if(iter->indexFlag == 1)
				{
					if(metaDataMap.find(iter->category) != metaDataMap.end())
						indexMetaDataMap.insert(make_pair(iter->category, metaDataMap[iter->category]));
				}
			}
			_lib.setMetaDatas(objId, indexMetaDataMap, false);
/*
			::TianShanIce::Properties::const_iterator itFind = Info.metaData.find(METADATA_StampLastUpdated);
			if(itFind != Info.metaData.end())
			{
				metaData.value = itFind->second;
				metaDataMap.insert(std::make_pair(LASTUPDATED, metaData));
			}
*/
			::TianShanIce::Properties::const_iterator itFind = Info.metaData.find(METADATA_StampLastUpdated);
			if(itFind != Info.metaData.end())
			{
				Ice::Identity ident;	
				ident.name = strContentName + "@" + strNetId + "$" + strVolume;
				ident.category = META_OBJECT;
				::TianShanIce::Repository::ContentReplicaExPrx proxy;
				proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_lib._adapter->createProxy(ident));
				if(!proxy)
				{
					std::string contentReplicaId = strNetId + "$" + strVolume + "/" +  strContentName;
					proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(toContentReplica(contentReplicaId));
				}
				if(proxy)
				{
					proxy->setMetaDataMap(metaDataMap);
					proxy->setLastModified(itFind->second);
				}
			}
		}
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "Add or Update content [%s] caught %s: %s"), 
			objId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "Add or Update content [%s] caught %s"), 
			objId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "Add or Update content [%s] unexpect exception"), 
			objId.c_str());
	}
}
