#include "ContentLibRequest.h"
#include "ContentLibEnv.h"
#include "ContentLibUtil.h"
#include "ContentLibConfig.h"
#include "TimeUtil.h"
#include "MetaLibImpl.h"

#define MAP_SET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL

ConnectEventChannelRequest::ConnectEventChannelRequest(ContentLibEnv& env)
	: ZQ::common::ThreadRequest(*(env._pThreadPool)), _env(env), _bExit(false)
{
}

ConnectEventChannelRequest::~ConnectEventChannelRequest()
{
}

bool ConnectEventChannelRequest::init()
{
	return true;
}

int ConnectEventChannelRequest::run(void)
{
	while (!_bExit)
	{
		if (_env.ConnectEventChannel())
			_bExit = true;
		else 
			_event.wait(2000);
	}

	return 0;
}

void ConnectEventChannelRequest::final(int retcode, bool bCancelled)
{
	delete this;
}

// class SyncContentStoreRequest
SyncContentStoreRequest::SyncContentStoreRequest(ContentLibEnv& env, const std::string& strNetId)
	: ZQ::common::ThreadRequest(*(env._pThreadPool)), _env(env), _NetId(strNetId)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(SyncContentStoreRequest, "[%s] request sync ContentStore"), 
		strNetId.c_str());
}

SyncContentStoreRequest::~SyncContentStoreRequest()
{
}

bool SyncContentStoreRequest::init()
{
	return true;
}

int SyncContentStoreRequest::run(void)
{
	try
	{
		_env.sync(_NetId);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(SyncContentStoreRequest, "[%s] contentstore->sync() caught %s: %s"), 
			_NetId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(SyncContentStoreRequest, "[%s] contentstore->sync() caught %s"), 
			_NetId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(SyncContentStoreRequest, "[%s] contentstore->sync() unexpect exception"), 
			_NetId.c_str());
	}

	return 0;
}

void SyncContentStoreRequest::final(int retcode, bool bCancelled)
{
	delete this;
}

// class AddContentRequest
int AddContentRequest::_gCount =0;
AddContentRequest::AddContentRequest(ContentLibEnv& env, const std::string& strNetId, const std::string& strVolumeName, const std::string& strContentName, const ::TianShanIce::Storage::ContentInfo& Info, const TianShanIce::Storage::ContentPrx contentProxy)
	: ZQ::common::ThreadRequest(*(env._pThreadPoolForContents)),_env(env),  _NetId(strNetId), 
	_VolumeName(strVolumeName), _ContentName(strContentName), _Info(Info), _contentProxy(contentProxy)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AddContentRequest, "[%s@%s$%s] request add content"), 
		strContentName.c_str(), strNetId.c_str(), strVolumeName.c_str());
	ZQ::common::MutexGuard guard(_env._gMutex);
	++_gCount;
}

AddContentRequest::~AddContentRequest()
{
	ZQ::common::MutexGuard guard(_env._gMutex);
	--_gCount;
}

bool AddContentRequest::init()
{
	return true;
}

int AddContentRequest::run(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(AddContentRequest, "addContentReplica(%s, %s, %s)"), _NetId.c_str(), _VolumeName.c_str(), _ContentName.c_str());
	::Ice::Long stampBegin = ZQTianShan::now();
	std::string objId = _ContentName + "@" + _NetId + "$" + _VolumeName;
	::TianShanIce::Repository::MetaDataValue metaData;
	metaData.hintedType = ::TianShanIce::vtStrings;
	::Ice::Long stampAdd = ZQTianShan::now();
	::Ice::Identity identObj;
	identObj.category = META_OBJECT;
	identObj.name = _ContentName + "@" + _NetId + "$" + _VolumeName;
	try
	{
		ContentReplicaImplPtr pObj = new ContentReplicaImpl(*(_env._lib));
		pObj->ident = identObj;
		pObj->type = CONTENTREPLICA;
		pObj->stampCreated = ZQTianShan::now(); ///< timestamp as of created
		pObj->content = _contentProxy;
		if(_Info.metaData.find(METADATA_StampLastUpdated) != _Info.metaData.end())
			pObj->lastModified = _Info.metaData[METADATA_StampLastUpdated];
		else
			pObj->lastModified = "";
		pObj->netId = _NetId;
		pObj->volumeName = _VolumeName;
		pObj->contentName = _ContentName;

		::TianShanIce::Repository::MetaDataMap metaDataMap;
		if((_Info.state == TianShanIce::Storage::csInService) && (_Info.metaData.size() > 0))
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(AddContentRequest, "set NonIndex metadata(%s, %s, %s)"), _NetId.c_str(), _VolumeName.c_str(), _ContentName.c_str());
			for(::TianShanIce::Properties::iterator it = _Info.metaData.begin(); it != _Info.metaData.end(); it++)
			{
				metaData.value = it->second;
				metaDataMap.insert(make_pair(it->first, metaData));
			}
		}

		metaData.value = _NetId;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, NETID, metaData);
		metaData.value = _VolumeName;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, VOLUMENAME, metaData);
		metaData.value = _ContentName;
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, CONTENTNAME, metaData);
		metaData.value = convertState(_Info.state);
		MAP_SET(::TianShanIce::Repository::MetaDataMap, metaDataMap, CONTENTSTATE, metaData);
		pObj->metaDataMap = metaDataMap;

		pObj->state = _Info.state;
		_env._lib->addObject(pObj, identObj);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(AddContentRequest, "new ContentReplica [%s:%s] added, took %lldms"), identObj.name.c_str(), identObj.category.c_str(), ZQTianShan::now() - stampAdd);
		_env._pContentCacheThread->addContent(_NetId, _VolumeName, _ContentName);
	}
	catch (const Ice::AlreadyRegisteredException&)
	{
		
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddContentRequest, "Add content [%s] caught %s: %s"), 
			objId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return 0;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddContentRequest, "Add content [%s] caught %s"), 
			objId.c_str(), ex.ice_name().c_str());
		return 0;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddContentRequest, "Add content [%s] unexpect exception"), 
			objId.c_str());
		return 0;
	}

	try
	{
		::Ice::Long stampSet = ZQTianShan::now();
		::TianShanIce::Repository::MetaDataMap metaDataMap;		
		metaData.value = _NetId;
		metaDataMap.insert(std::make_pair( NETID, metaData));		

		metaData.value = _VolumeName;
		metaDataMap.insert(std::make_pair( VOLUMENAME, metaData));	

		metaData.value = _ContentName;
		metaDataMap.insert(std::make_pair( CONTENTNAME, metaData));

		metaData.value = convertState(_Info.state);
		metaDataMap.insert(std::make_pair( CONTENTSTATE, metaData));

		metaData.value = CONTENTREPLICA;
		metaDataMap.insert(std::make_pair(OBJECTTYPE, metaData));
/*
		if(_Info.metaData.find(METADATA_StampLastUpdated) != _Info.metaData.end())
		{
			metaData.value = _Info.metaData[METADATA_StampLastUpdated];
		}
		else
		{
			metaData.value = "";
		}
		metaDataMap.insert(std::make_pair(LASTUPDATED, metaData));
*/
		for (ContentLibConfig::MetadataCategories::iterator iter = _config.categories.begin(); iter != _config.categories.end(); ++iter) 
		{
			if(iter->indexFlag == 1)
			{
				if(_Info.metaData.find(iter->category) != _Info.metaData.end())
				{
					metaData.value = _Info.metaData[iter->category];
					metaDataMap.insert(std::make_pair(iter->category, metaData));
				}
			}
		}

		_env._lib->setMetaDatas(objId, metaDataMap, true);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(AddContentRequest, "set MetaDatas of [%s:%s], took %lldms"), identObj.name.c_str(), identObj.category.c_str(), ZQTianShan::now() - stampSet);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(AddContentRequest, "complete process content [%s], took %lldms"), objId.c_str(), ZQTianShan::now() - stampBegin);

	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddContentRequest, "Add content [%s] set MetaDatas caught %s"), 
			objId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddContentRequest, "Add content [%s] set MetaDatas caught unexpected exception"), 
			objId.c_str());
	}

	return 1;
}

void AddContentRequest::final(int retcode, bool bCancelled)
{
	delete this;
}

LocateCmd::LocateCmd(ContentLibEnv& env, const ::TianShanIce::Repository::AMD_ContentLib_locateContentPtr& amdCB, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames, ::Ice::Int maxHop, bool pingOnly)
	: ZQ::common::ThreadRequest(*(env._pThreadPool)), _env(env),  _amdCB(amdCB), _searchForMetaData(searchForMetaData), 
	_expectedMetaDataNames(expectedMetaDataNames), _maxHop(maxHop), _pingOnly(pingOnly)
{
}

LocateCmd::~LocateCmd()
{
}

int LocateCmd::run(void)
{
	std::string lastError;
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "LocateCmd() enter"));
	try
	{
		TianShanIce::Repository::MetaObjectInfos ret = _env._lib->proxy()->lookup(CONTENTREPLICA, _searchForMetaData, _expectedMetaDataNames);
		_amdCB->ice_response(ret);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "LocateCmd() leave"));
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_amdCB->ice_exception(ex);
		return 1;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LocateCmd caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LocateCmd caught unknown exception");
		lastError = buf;
	}

	TianShanIce::ServerError ex("LocateCmd", 501, lastError);
	_amdCB->ice_exception(ex);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "LocateCmd() leave"));
	return 1;
}

// class DeleteContentRequest
DeleteContentRequest::DeleteContentRequest(ContentLibEnv& env, const std::string& strNetId, const std::string& strVolumeName, const std::string& strContentName)
	: ZQ::common::ThreadRequest(*(env._pThreadPoolForContents)), _env(env), _NetId(strNetId), 
	_VolumeName(strVolumeName), _ContentName(strContentName)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DeleteContentRequest, "[%s@%s$%s] request delete content"), 
		strContentName.c_str(), strNetId.c_str(), strVolumeName.c_str());
}

DeleteContentRequest::~DeleteContentRequest()
{
}

bool DeleteContentRequest::init()
{
	return true;
}

int DeleteContentRequest::run(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(DeleteContentRequest, "DeleteContentRequest(%s, %s, %s)"), _NetId.c_str(), _VolumeName.c_str(), _ContentName.c_str());
	::Ice::Long stampBegin = ZQTianShan::now();
	std::string objId = _ContentName + "@" + _NetId + "$" + _VolumeName;
	try
	{
		_env._lib->proxy()->removeObject(objId);
		_env._pContentCacheThread->removeContent(_NetId, _VolumeName, _ContentName);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(DeleteContentRequest, "complete delete content [%s], took %lldms"), objId.c_str(), ZQTianShan::now() - stampBegin);

	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DeleteContentRequest, "Delete content [%s] caught %s: %s"), 
			objId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DeleteContentRequest, "Delete content [%s] caught %s"), 
			objId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DeleteContentRequest, "Delete content [%s] unexpect exception"), 
			objId.c_str());
	}

	return 0;
}

void DeleteContentRequest::final(int retcode, bool bCancelled)
{
	delete this;
}

// class PageSizeContentRequest
PageSizeContentRequest::PageSizeContentRequest(ContentLibEnv& env, TianShanIce::Storage::ContentInfos& contentInfos, std::vector<std::string>& contentsInDB, const std::string& strNetId, const std::string& strVolumeName)
	: ZQ::common::ThreadRequest(*(env._pThreadPoolForContents)), _env(env), _ContentInfos(contentInfos), 
	_ContentsInDB(contentsInDB), _NetId(strNetId), _VolumeName(strVolumeName)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(PageSizeContentRequest, "[%s$%s] request %d content"), 
			strNetId.c_str(), strVolumeName.c_str(), contentInfos.size());
}

PageSizeContentRequest::~PageSizeContentRequest()
{
}

bool PageSizeContentRequest::init()
{
	return true;
}

int PageSizeContentRequest::run(void)
{
	if (_ContentInfos.empty())
	{
		return 0;
	}
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
	if(!_contentLibPtr)
		return 0;
	else
	{
		_contentLibPtr->getContentStoreProxy(_NetId, contentStoreProxy);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(PageSizeContentRequest, "PageSizeContentRequest(%s, %s)"), _NetId.c_str(), _VolumeName.c_str());
//	::Ice::Long stampBegin = ZQTianShan::now();
	try
	{
		ContentLibImpl::Ptr contentLibPtr;
		if(_env._contentlib)
		{
			contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
//			contentLibPtr->addContentReplica(_NetId, _VolumeName, _ContentName);
		}
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(PageSizeContentRequest, "caught an unexpected exception"));
	}
/*
	for ( TianShanIce::Storage::ContentInfos::iterator itorContent = _ContentInfos.begin(); itorContent != _ContentInfos.end(); itorContent++)
	{
		::TianShanIce::Storage::ContentInfo& contentinfo = *itorContent;
		std::vector<std::string>::iterator itorContentDB = _ContentDB.begin();
		for(; itorContentDB != _ContentDB.end(); itorContentDB++)
		{
			if(contentinfo.name == *itorContentDB || *itorContentDB > contentinfo.name)
				break;
		}
	}
*/
	// start the sync process
	int j = 0, k = 0;
	while (j < (int)_ContentInfos.size() && k < (int)_ContentsInDB.size() && _ContentsInDB[k] <= _ContentInfos[_ContentInfos.size()-1].name)
	{
		// if names equal, further check if lastModified is same
		if (_ContentInfos[j].name == _ContentsInDB[k])
		{
			if(_ContentInfos[j].metaData.find(METADATA_StampLastUpdated) == _ContentInfos[j].metaData.end() || (_ContentInfos[j].state != TianShanIce::Storage::csInService) || _ContentInfos[j].metaData.empty())
			{
				j ++;
				k ++;
				continue;
			}
			::TianShanIce::Repository::ContentReplicaExPrx proxy;
			Ice::Identity ident;	
			ident.name = _ContentsInDB[k] + "@" + _NetId + "$" + _VolumeName;
			ident.category = META_OBJECT;
			std::string timeStamp;
			try
			{
				proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_env._lib->_adapter->createProxy(ident));
				if(!proxy)
				{
					std::string contentReplicaId = _NetId + "$" + _VolumeName + "/" +  _ContentInfos[j].name;
					proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_contentLibPtr->toContentReplica(contentReplicaId));
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
			if (_ContentInfos[j].metaData[METADATA_StampLastUpdated] != timeStamp)
			{
				// means it has been modified
				UpdateContentRequest* pRequest = new UpdateContentRequest(_env, _NetId, _VolumeName, _ContentInfos[j].name, _ContentInfos[j]);
				if (pRequest)
				{
					pRequest->start();
				}
				else 
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create update content request for [%s@%s$%s] failed"), _ContentInfos[j].name.c_str(), _NetId.c_str(), _VolumeName.c_str());
				}
			}
			j ++;
			k ++;
			continue;
		}

		// if content in DB is different from ContentStore's
		int i, count;
		bool bInContentStore = false; // to indicates whether or not the content is in ContentStore list
		for (i = j, count = (int)_ContentInfos.size(); i < count; i ++)
		{	// notice here the initial value of i is j
			// means the content list from ContentStore is a sub list which after the position of current content
			if (_ContentsInDB[k] == _ContentInfos[i].name)
			{
				// content in DB is also in ContentStore list
				// means it is a new one
				bInContentStore = true;
				break;
			}
		}

		if (bInContentStore)
		{
			try
			{
				TianShanIce::Storage::ContentPrx contentProxy = contentStoreProxy->openContentByFullname(_ContentInfos[j].fullname);
				if (!contentProxy)
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openContentByFullName(%s) : content proxy is empty"), _ContentInfos[j].fullname.c_str());
				}
				else
				{
					AddContentRequest* pRequest = new AddContentRequest(_env, _NetId, _VolumeName, _ContentInfos[j].name, _ContentInfos[j], contentProxy);
					if (pRequest)
					{
						pRequest->start();
					}
					else 
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create add content request for [%s@%s$%s] failed"), _ContentInfos[j].name.c_str(), _NetId.c_str(), _VolumeName.c_str());
					}
				}
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), _ContentInfos[j].fullname.c_str());
			}
			j ++;
		}
		else 
		{
			DeleteContentRequest* pRequest = new DeleteContentRequest(_env, _NetId, _VolumeName, _ContentsInDB[k]);
			if (pRequest)
			{
				pRequest->start();
			}
			else 
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create delete content request for [%s@%s$%s] failed"), _ContentsInDB[k].c_str(), _NetId.c_str(), _VolumeName.c_str());
			}
			k ++;
		}
	}

	while (j < (int)_ContentInfos.size())
	{
		std::string ProviderId;
		std::string ProviderAssetId;
		TianShanIce::Storage::ContentPrx contentProxy;
		try
		{
			contentProxy = contentStoreProxy->openContentByFullname(_ContentInfos[j].fullname);
			if (!contentProxy)
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(ContentLibImpl, "openContentByFullName(%s) : content proxy is empty"), _ContentInfos[j].fullname.c_str());
				j++;
				continue;
			}
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), _ContentInfos[j].fullname.c_str());
			j++;
			continue;
		}
		AddContentRequest* pRequest = new AddContentRequest(_env, _NetId, _VolumeName, _ContentInfos[j].name, _ContentInfos[j], contentProxy);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create add content request for [%s@%s$%s] failed"), _ContentInfos[j].name.c_str(), _NetId.c_str(), _VolumeName.c_str());
		}
		j ++;
	}

	while (k < (int)_ContentsInDB.size() && _ContentsInDB[k] <= _ContentInfos[_ContentInfos.size()-1].name)
	{
		DeleteContentRequest* pRequest = new DeleteContentRequest(_env, _NetId, _VolumeName, _ContentsInDB[k]);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibImpl, "create delete content request for [%s@%s$%s] failed"), _ContentsInDB[k].c_str(), _NetId.c_str(), _VolumeName.c_str());
		}
		k ++;
	}

	return 0;
}

void PageSizeContentRequest::final(int retcode, bool bCancelled)
{
	delete this;
}

// class UpdateContentRequest
UpdateContentRequest::UpdateContentRequest(ContentLibEnv& env, const std::string& strNetId, const std::string& strVolumeName, const std::string& strContentName, const ::TianShanIce::Storage::ContentInfo& Info)
	: ZQ::common::ThreadRequest(*(env._pThreadPoolForContents)), _env(env), _NetId(strNetId), 
	_VolumeName(strVolumeName), _ContentName(strContentName), _Info(Info)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(UpdateContentRequest, "[%s@%s$%s] request update content"), 
		strContentName.c_str(), strNetId.c_str(), strVolumeName.c_str());
}

UpdateContentRequest::~UpdateContentRequest()
{
}

bool UpdateContentRequest::init()
{
	return true;
}

int UpdateContentRequest::run(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(UpdateContentRequest, "UpdateContentRequest(%s, %s, %s)"), _NetId.c_str(), _VolumeName.c_str(), _ContentName.c_str());
	::Ice::Long stampBegin = ZQTianShan::now();
	std::string objId = _ContentName + "@" + _NetId + "$" + _VolumeName;
	try
	{
		if(_env._contentlib)
		{
			ContentLibImpl::Ptr contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
			if(!_Info.metaData.empty())
				contentLibPtr->updateContentReplica(_NetId, _VolumeName, _ContentName, _Info);
			else
				contentLibPtr->updateContentMetadata(_NetId, _VolumeName, _ContentName);
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(UpdateContentRequest, "complete update content [%s], took %lldms"), objId.c_str(), ZQTianShan::now() - stampBegin);

	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(UpdateContentRequest, "Update content [%s] caught %s: %s"), 
			objId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(UpdateContentRequest, "Update content [%s] caught %s"), 
			objId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(UpdateContentRequest, "Update content [%s] unexpect exception"), 
			objId.c_str());
	}

	return 0;
}

void UpdateContentRequest::final(int retcode, bool bCancelled)
{
	delete this;
}

/// add by HL
//////////////////////////////////////
/////////class FullSyncRequest ///////
//////////////////////////////////////
FullSyncRequest::FullSyncRequest( ContentLibEnv& env, const std::string netId)
	: ZQ::common::ThreadRequest(*(env._pThreadPool)), _env(env), _netId(netId)
{
	ThreadRequest::setPriority(150);// lower than DEFAULT_REQUEST_PRIO
	_volumeInfos.clear();
	_contentStoreProxy = NULL;
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] [%s] Full Data Sync request"), SYS::getCurrentThreadID(), netId.c_str());
}
FullSyncRequest::~FullSyncRequest()
{
}
bool FullSyncRequest::init()
{
	return true;
}
int FullSyncRequest::run(void)
{
	try
	{
		::Ice::Long lStart = ZQTianShan::now();
		ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
		if(!_contentLibPtr)
			return false;
		if (!_contentLibPtr->getContentStoreProxy(_netId, _contentStoreProxy))
		{
			return false;
		}
        if(!syncVolume())
			return false;
		if(!syncContent())
			return false;
		//set last modified
		try
		{
			ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
			if(!_contentLibPtr)
				return false;
			::TianShanIce::Repository::ContentStoreReplicaPrx csPrx = _contentLibPtr->toStoreReplica(_netId);
			if(csPrx)
			{
				::TianShanIce::Repository::ContentStoreReplicaExPrx cePrx = ::TianShanIce::Repository::ContentStoreReplicaExPrx::checkedCast(csPrx);
				cePrx->setLastSync(ZQTianShan::now());
			}
		}
		catch (TianShanIce::EntityNotFound)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] toStoreReplica() error : Entity Not Found"), SYS::getCurrentThreadID());
			return false;
		}
		catch (TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] setLastSync() caught(%s: %s)"), SYS::getCurrentThreadID()
				, ex.ice_name().c_str(), ex.message.c_str());
			return false;
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] setLastSync() caught(%s)"), SYS::getCurrentThreadID()
				, ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			return false;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "[ThreadID=%u] full data sync [%s] successfully took %lld ms"), SYS::getCurrentThreadID(), _netId.c_str(), (Ice::Long)(ZQTianShan::now() - lStart));
	}
	catch (::TianShanIce::BaseException &ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] full data sync [%s] catch exception(%s)"), SYS::getCurrentThreadID(), _netId.c_str(), ex.ice_name().c_str());
	}
	catch (::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] full data sync [%s] catch exception(%s)"), SYS::getCurrentThreadID(), _netId.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] full data sync [%s] catch unknown exception"), SYS::getCurrentThreadID(), _netId.c_str());
	}
	return -1;
}
void FullSyncRequest::final(int retcode, bool bCancelled)
{
	delete this;
}

bool FullSyncRequest::syncVolume()
{
	// 1.list volumes from ContentStore
	try
	{
		_volumeInfos = _contentStoreProxy->listVolumes("*", true);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] getVolumesFromStore() : catch an ice exception [%s] when list all volume"), SYS::getCurrentThreadID(), ex.ice_name().c_str());
		return false;
	}

	// 2.list volumes in database
	TianShanIce::StrValues volumesInDB;
	TianShanIce::Properties searchForMetaData;
	TianShanIce::StrValues expectedMetaDataNames;
	searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, _netId));
	searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, METAVOLUME));
	TianShanIce::Repository::MetaObjectInfos ret = _env._lib->proxy()->lookup(METAVOLUME, searchForMetaData, expectedMetaDataNames);
	for (TianShanIce::Repository::MetaObjectInfos::iterator itObj = ret.begin(); itObj < ret.end(); itObj++)
	{
		volumesInDB.push_back(itObj->id.substr(itObj->id.find("$") + 1));
	}
	//sort by name
	std::sort(volumesInDB.begin(), volumesInDB.end());

	ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
	if(!_contentLibPtr)
		return false;
	TianShanIce::Storage::VolumeInfos::iterator itorVol;
	TianShanIce::StrValues::iterator itorVolDB;
	for(itorVol = _volumeInfos.begin(); itorVol != _volumeInfos.end(); itorVol++)
	{
		::TianShanIce::Storage::VolumeInfo& volumeinfo = *itorVol;
		itorVolDB = std::find(volumesInDB.begin(), volumesInDB.end(), volumeinfo.name);
		///indicate this volume is exists in localDB, check it need to be updated?
		if(itorVolDB != volumesInDB.end())
		{
			volumesInDB.erase(itorVolDB);
			/*
			if(update)
			{
				updata this volume;
			}
			*/
		}
		///indicate this volume is not exists in localDB, it's new.
		else
		{
            ///add volume to db
			_contentLibPtr->addMetaVolume(_netId, volumeinfo.name);
		}
	}
	///if volumesInDB is not NULL, indicate those volume is not in contentStore, delete them
	for(itorVolDB = volumesInDB.begin(); itorVolDB != volumesInDB.end(); itorVolDB++)
	{
	    //delete this volume;
		std::string volumeName= (*itorVolDB).substr((*itorVolDB).find("$") + 1);
		_contentLibPtr->deleteMetaVolume(_netId, volumeName);
	}

	return true;
}
bool FullSyncRequest::syncContent()
{
	ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
	if(!_contentLibPtr)
		return false;
	for (TianShanIce::Storage::VolumeInfos::iterator volumeIt = _volumeInfos.begin();
		volumeIt != _volumeInfos.end(); volumeIt++)
	{
		try
		{
			TianShanIce::Storage::VolumePrx volumeProxy = _contentStoreProxy->openVolume(volumeIt->name);
			if (!volumeProxy)
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] openVolume() : volume proxy is empty"), SYS::getCurrentThreadID());
				continue;
			}

			// 1.list contents in database
			std::map<std::string, std::string> contentsInDB;
			TianShanIce::Properties searchForMetaData;
			TianShanIce::StrValues expectedMetaDataNames;
//			expectedMetaDataNames.push_back(LASTUPDATED);
			searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, _netId));
			searchForMetaData.insert(TianShanIce::Properties::value_type(VOLUMENAME, volumeIt->name));
			searchForMetaData.insert(TianShanIce::Properties::value_type(OBJECTTYPE, CONTENTREPLICA));
			try{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] start to lookup contents from database of [%s]"), SYS::getCurrentThreadID(), (volumeIt->name).c_str());
				TianShanIce::Repository::MetaObjectInfos ret = _env._lib->proxy()->lookup(CONTENTREPLICA, searchForMetaData, expectedMetaDataNames);
				for (TianShanIce::Repository::MetaObjectInfos::iterator itObj = ret.begin(); itObj < ret.end(); itObj++)
				{
//					const std::string& timestamp = itObj->metaDatas[LASTUPDATED].value;
					contentsInDB.insert(std::make_pair(itObj->id.substr(0, itObj->id.find("@")),""));
				}
				//sort by name
//				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "start to sort"));
//				std::sort(contentsInDB.begin(), contentsInDB.end());
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] lookup() : catch an exception[%s] when lookup content in volume[%s]"), SYS::getCurrentThreadID(), ex.ice_name().c_str(), (volumeIt->name).c_str());
				continue;
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] lookup() : catch an unexpected exception when lookup content in volume[%s]"), SYS::getCurrentThreadID(), (volumeIt->name).c_str());
				continue;
			}

			TianShanIce::StrValues metaDataNames;
			ContentLibConfig::MetadataCategories::iterator mt_iter = _config.categories.begin();
			for (; mt_iter != _config.categories.end(); ++mt_iter) 
			{
				metaDataNames.push_back(mt_iter->category);
			}

			TianShanIce::Storage::ContentInfos::iterator itorContent;
			TianShanIce::Storage::CachedContentListPrx cacheProxy;
			::std::string startContentName = "";
			std::map<std::string, std::string>::iterator itorContentDB;
			itorContentDB = contentsInDB.begin();
			try
			{
				int left = 0;
				TianShanIce::Storage::VolumeExPrx  volumeExProxy = TianShanIce::Storage::VolumeExPrx::checkedCast(volumeProxy);
				if (!volumeExProxy)
					continue;
				int nreturn = 0;
				do {
					////
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] before cachedListContents %s"), SYS::getCurrentThreadID(), startContentName.c_str());
					cacheProxy = volumeExProxy->cachedListContents(3600000);
					int npage=0;
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] after cachedListContents"), SYS::getCurrentThreadID());
					do {
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] before next %d pages"), SYS::getCurrentThreadID(), npage++);
						TianShanIce::Storage::ContentInfos contentinfos = cacheProxy->next(metaDataNames, startContentName, PageSize, 3600000);
						nreturn = contentinfos.size();
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] after next return %d"), SYS::getCurrentThreadID(), nreturn);
						left = cacheProxy->left();
						itorContent = contentinfos.begin();
						Ice::Long syncStamp = ZQ::common::now();
						glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "[ThreadID=%u] start to sync , total <%d>, left <%d> , local database <%d>"), SYS::getCurrentThreadID(), cacheProxy->size(), left, contentsInDB.size());
						std::map<std::string, std::string>::iterator itWrap;
						for(; itorContent != contentinfos.end(); itorContent++)
						{
							::TianShanIce::Storage::ContentInfo& contentinfo = *itorContent;
							for(; itorContentDB != contentsInDB.end(); itorContentDB++)
							{
								if(contentinfo.name == itorContentDB->first || itorContentDB->first > contentinfo.name)
									break;
							}
							///indicate this content is exists in localDB, check it need to be updated?
							if(itorContentDB != contentsInDB.end() && contentinfo.name == itorContentDB->first)
							{
//								std::string timeStamp = itorContentDB->second;
								itWrap = itorContentDB;
								itWrap++;
								contentsInDB.erase(itorContentDB);
								itorContentDB = itWrap;

								//if need update
								if(itorContent->metaData.find(METADATA_StampLastUpdated) == itorContent->metaData.end())// || (itorContent->state != TianShanIce::Storage::csInService) || (itorContent->metaData.size() <= 0))
								{
									// METADATA_StampLastUpdated unavailable
									continue;
								}
								try
								{
									::TianShanIce::Repository::ContentReplicaExPrx proxy;
									Ice::Identity ident;	
									ident.name = itorContent->name + "@" + _netId + "$" + volumeIt->name;
									ident.category = META_OBJECT;
									std::string timeStamp;
									try
									{
										proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_env._lib->_adapter->createProxy(ident));
										if(!proxy)
										{
											std::string contentReplicaId = _netId + "$" + volumeIt->name + "/" +  itorContent->name;
											proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_contentLibPtr->toContentReplica(contentReplicaId));
										}
										if(proxy)
										{
											timeStamp = proxy->getLastModified();
										}
									}
									catch (const Ice::Exception&)
									{
										continue;
									}

									if (itorContent->metaData[METADATA_StampLastUpdated] != timeStamp)
									{
										UpdateContentRequest* pRequest = new UpdateContentRequest(_env, _netId, volumeIt->name, itorContent->name, *itorContent);
										if (pRequest)
										{
											pRequest->start();
										}
										else 
										{
											glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "create update content request for [%s@%s$%s] failed"), itorContent->name.c_str(), _netId.c_str(), volumeIt->name.c_str());
										}
									}
									else
									{
										glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "do not need to update for [%s@%s$%s], ignore"), itorContent->name.c_str(), _netId.c_str(), volumeIt->name.c_str());
									}
								}
								catch (const Ice::Exception& ex)
								{
									glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "UpdateContentRequest() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), contentinfo.name.c_str());
									continue;
								}
								catch (...)
								{
									glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "UpdateContentRequest() : catch an unknown exception [%d] when open content [%s]"), SYS::getLastErr(), contentinfo.name.c_str());
									continue;
								}
							}
							///indicate this content is not exists in localDB, it's new.
							// new AddContentRequest for each contnetInfo
							else
							{
								///add content to db
								TianShanIce::Storage::ContentPrx contentProxy;
								try
								{
									contentProxy = _contentStoreProxy->openContentByFullname(itorContent->fullname);
									if (!contentProxy)
									{
										glog(ZQ::common::Log::L_WARNING, CLOGFMT(FullSyncRequest, "openContentByFullName(%s) : content proxy is empty"), itorContent->fullname.c_str());
									}
									else
									{
										AddContentRequest* pRequest = new AddContentRequest(_env, _netId, volumeIt->name, itorContent->name, *itorContent, contentProxy);
										if (pRequest)
										{
											pRequest->start();
										}
										else 
										{
											glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "create add content request for [%s@%s$%s] failed"), itorContent->name.c_str(), _netId.c_str(), volumeIt->name.c_str());
										}
									}
								}
								catch (const Ice::Exception& ex)
								{
									glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), itorContent->fullname.c_str());
								}
								catch (...)
								{
									glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "openContentByFullName() : catch an unknown exception [%d] when open content [%s]"), SYS::getLastErr(), itorContent->fullname.c_str());
									continue;
								}
							}
						}
						if(contentinfos.size() > 0)
						{
							TianShanIce::Storage::ContentInfos::const_iterator contentIt = contentinfos.end() - 1;
							startContentName = contentIt->name;
						}
						glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "[ThreadID=%u] after sync took %lld ms"), SYS::getCurrentThreadID(), ZQ::common::now()-syncStamp);
						if (AddContentRequest::count() > 2000) //PageSize*2
						{
							cacheProxy->destory();
							glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "[ThreadID=%u] destroy CachedContentList because of too many requests[%d]"), SYS::getCurrentThreadID(), AddContentRequest::count());
							break;
						}
						else
						{
							glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "[ThreadID=%u] start to get next page"), SYS::getCurrentThreadID());
						}
					} while (left > 0 || nreturn>=PageSize);

					while ((left > 0 || nreturn>=PageSize) && AddContentRequest::count() > 2000) //PageSize*2
					{
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "[ThreadID=%u] %d AddContentRequest left"), SYS::getCurrentThreadID(), AddContentRequest::count());
						SYS::sleep(100);
					}

				} while (left > 0 || nreturn>=PageSize);
				cacheProxy->destory();
				glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "[ThreadID=%u] destroy CachedContentList because no content left in CachedContentList"), SYS::getCurrentThreadID());
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] listContent() : catch an exception[%s] when list content in volume[%s]"), SYS::getCurrentThreadID(), ex.ice_name().c_str(), (volumeIt->name).c_str());
				try
				{
					if(cacheProxy)
						cacheProxy->destory();
				}
				catch (...)
				{	
				}				
				continue;
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] listContent() : catch an unknown exception [%d] when list content in volume[%s]"), SYS::getCurrentThreadID(), SYS::getLastErr(), (volumeIt->name).c_str());
				try
				{
					if(cacheProxy)
						cacheProxy->destory();
				}
				catch (...)
				{	
				}	
				continue;
			}
/*
			try{
				TianShanIce::Storage::VolumeExPrx  volumeExProxy = TianShanIce::Storage::VolumeExPrx::checkedCast(volumeProxy);
				if (!volumeExProxy)
					continue;
				cacheProxy = volumeExProxy->cachedListContents(3600000);
				if (!cacheProxy)
					continue;
			}
			catch (const TianShanIce::ServerError& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "cachedListContents() : catch an exception[%s] when list content in volume[%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
				continue;
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "cachedListContents() : catch an exception[%s] when list content in volume[%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
				continue;
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "cachedListContents() : catch an unknown exception [%d] when list content in volume[%s]"), SYS::getLastErr(), (volumeIt->name).c_str());
				continue;
			}

			std::map<std::string, std::string>::iterator itorContentDB;
			itorContentDB = contentsInDB.begin();
			try{
				do 
				{
					if(!contentinfos.empty())
					{
						TianShanIce::Storage::ContentInfos::const_iterator contentIt = contentinfos.end() - 1;
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "listContents from start <%s>"), contentIt->name.c_str());
//						contentinfos = volumeProxy->listContents(metaDataNames, contentIt->name, PageSize);
						contentinfos = cacheProxy->next(metaDataNames, contentIt->name, PageSize, 3600000);
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "listContents return %d"), contentinfos.size());
//						itorContent = contentinfos.begin() + 1;
					}
					else
					{
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "listContents from start <>"));
//						contentinfos = volumeProxy->listContents(metaDataNames, "", PageSize);
						contentinfos = cacheProxy->next(metaDataNames, "", PageSize, 3600000);
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "listContents return %d"), contentinfos.size());
					}
					itorContent = contentinfos.begin();
					Ice::Long syncStamp = ZQ::common::now();
					glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "start to sync , total <%d>, left <%d> , local database <%d>"), cacheProxy->size(), cacheProxy->left(), contentsInDB.size());
					for(; itorContent != contentinfos.end(); itorContent++)
					{
						::TianShanIce::Storage::ContentInfo& contentinfo = *itorContent;
//						itorContentDB = std::find(contentsInDB.begin(), contentsInDB.end(), contentinfo.name);
						for(; itorContentDB != contentsInDB.end(); itorContentDB++)
						{
							if(contentinfo.name == itorContentDB->first || itorContentDB->first > contentinfo.name)
								break;
						}
						///indicate this content is exists in localDB, check it need to be updated?
						if(itorContentDB != contentsInDB.end() && contentinfo.name == itorContentDB->first)
						{
							std::string timeStamp = itorContentDB->second;

							itorContentDB = contentsInDB.erase(itorContentDB);

							//if need update
							if(itorContent->metaData.find(METADATA_StampLastUpdated) == itorContent->metaData.end())
							{
								// METADATA_StampLastUpdated unavailable
								continue;
							}
// 							ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_env._contentlib);
// 							if(!_contentLibPtr)
// 								continue;
// 							::TianShanIce::Repository::ContentReplicaExPrx proxy;
// 							Ice::Identity ident;	
// 							ident.name = *itorContentDB + "@" + _netId + "$" + volumeIt->name;
// 							ident.category = META_OBJECT;
							try
							{
// 								proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_env._lib->_adapter->createProxy(ident));
// 								if(!proxy)
// 								{
// 									std::string contentReplicaId = _netId + "$" + volumeIt->name + "/" +  *itorContentDB;
// 									proxy = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(_contentLibPtr->toContentReplica(contentReplicaId));
// 								}
// 								if(proxy)
// 								{
// 									timeStamp = proxy->getLastModified();
// 								}
								if (itorContent->metaData[METADATA_StampLastUpdated] != timeStamp && (itorContent->state == TianShanIce::Storage::ContentState::csInService) && (itorContent->metaData.size() > 0))
								{
									UpdateContentRequest* pRequest = new UpdateContentRequest(_env, _netId, volumeIt->name, itorContent->name, *itorContent);
									if (pRequest)
									{
										pRequest->start();
									}
									else 
									{
										glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "create update content request for [%s@%s$%s] failed"), itorContent->name.c_str(), _netId.c_str(), volumeIt->name.c_str());
									}
								}
								else
								{
									glog(ZQ::common::Log::L_DEBUG, CLOGFMT(FullSyncRequest, "do not need to update for [%s@%s$%s], ignore"), itorContent->name.c_str(), _netId.c_str(), volumeIt->name.c_str());
								}
							}
							catch (const Ice::Exception& ex)
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "UpdateContentRequest() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), contentinfo.name.c_str());
								continue;
							}
							catch (...)
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "UpdateContentRequest() : catch an unknown exception [%d] when open content [%s]"), SYS::getLastErr(), contentinfo.name.c_str());
								continue;
							}
						}
						///indicate this content is not exists in localDB, it's new.
						else
						{
							///add content to db
							TianShanIce::Storage::ContentPrx contentProxy;
							try
							{
								contentProxy = _contentStoreProxy->openContentByFullname(itorContent->fullname);
								if (!contentProxy)
								{
									glog(ZQ::common::Log::L_WARNING, CLOGFMT(FullSyncRequest, "openContentByFullName(%s) : content proxy is empty"), itorContent->fullname.c_str());
								}
								else
								{
									AddContentRequest* pRequest = new AddContentRequest(_env, _netId, volumeIt->name, itorContent->name, *itorContent, contentProxy);
									if (pRequest)
									{
										pRequest->start();
									}
									else 
									{
										glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "create add content request for [%s@%s$%s] failed"), itorContent->name.c_str(), _netId.c_str(), volumeIt->name.c_str());
									}
								}
							}
							catch (const Ice::Exception& ex)
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), itorContent->fullname.c_str());
							}
							catch (...)
							{
								glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "openContentByFullName() : catch an unknown exception [%d] when open content [%s]"), SYS::getLastErr(), itorContent->fullname.c_str());
								continue;
							}
						}
					}
					glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "start to 1000 sync took %lld ms"), ZQ::common::now()-syncStamp);
				} while (contentinfos.size() >= PageSize);
				cacheProxy->destory();
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "listContent() : catch an exception[%s] when list content in volume[%s]"), ex.ice_name().c_str(), (volumeIt->name).c_str());
				try
				{
					cacheProxy->destory();
				}
				catch (...)
				{	
				}				
				continue;
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "listContent() : catch an unknown exception [%d] when list content in volume[%s]"), SYS::getLastErr(), (volumeIt->name).c_str());
				try
				{
					cacheProxy->destory();
				}
				catch (...)
				{	
				}	
				continue;
			}
*/
			///if volumesInDB is not NULL, indicate those volume is not in contentStore, delete them
			for(itorContentDB = contentsInDB.begin(); itorContentDB != contentsInDB.end(); itorContentDB++)
			{
				//delete this content;]
				DeleteContentRequest* pRequest = new DeleteContentRequest(_env, _netId, volumeIt->name, itorContentDB->first);
				if (pRequest)
				{
					pRequest->start();
				}
				else 
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] create delete content request for [%s@%s$%s] failed"), SYS::getCurrentThreadID(), (itorContentDB->first).c_str(), _netId.c_str(), volumeIt->name.c_str());
				}
			}
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] openVolume() : catch an exception[%s] when open volume[%s]"), SYS::getCurrentThreadID(), ex.ice_name().c_str(), (volumeIt->name).c_str());
			continue;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(FullSyncRequest, "[ThreadID=%u] openVolume() : catch an unknown exception [%d] when list open volume[%s]"), SYS::getCurrentThreadID(), SYS::getLastErr(), (volumeIt->name).c_str());
			continue;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(FullSyncRequest, "[ThreadID=%u] syncContent() : leave sync volume[%s]"), SYS::getCurrentThreadID(), (volumeIt->name).c_str());
	}

	return true;
}

//////////////////////////////////////
/////////class LocateByPidPAidCmd ///////
//////////////////////////////////////
LocateByPidPAidCmd::LocateByPidPAidCmd(ContentLibEnv& env, const ::TianShanIce::Repository::AMD_ContentLib_locateContentByPIDAndPAIDPtr& amdCB, const ::std::string& netId, const ::std::string& volumeId, const ::std::string& providerId, const ::std::string& providerAssetId, const ::TianShanIce::StrValues& expectedMetaDataNames)
	: ZQ::common::ThreadRequest(*(env._pThreadPool)), _env(env), _amdCB(amdCB), _netId(netId), _volumeId(volumeId), 
	_providerId(providerId), _providerAssetId(providerAssetId), _expectedMetaDataNames(expectedMetaDataNames)
{
}

LocateByPidPAidCmd::~LocateByPidPAidCmd()
{
}

int LocateByPidPAidCmd::run(void)
{
	std::string lastError;
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "LocateByPidPAidCmd() enter"));
	try
	{
		TianShanIce::Repository::MetaObjectInfos ret;
//		ZQTianShan::IdentCollection _objsFound;
		::Ice::Long stampStart = ZQTianShan::now();

		try {
			::Ice::Long stampSearch = ZQTianShan::now();
			ZQTianShan::IdentCollection paidids, pidids;
#if ICE_INT_VERSION / 100 >= 306
			RLockT <IceUtil::RWRecMutex> lk(_env._lib->_metaDataContainerLocker);
#else
			IceUtil::RLockT <IceUtil::RWRecMutex> lk(_env._lib->_metaDataContainerLocker);
#endif
			std::string ctnkeyname = "user.ProviderAssetId";

			// 1. search idents for paid
			ZQTianShan::MetaLib::MetaLibImpl::MetaDataContainerMap::iterator itMetaData = _env._lib->_metaDataContainerMap.find(ctnkeyname);
			if (_env._lib->_metaDataContainerMap.end() == itMetaData)
			{
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (glog, EXPFMT(LocateByPidPAidCmd, 3001, "LocateByPidPAidCmd() look up metaData[%s] is non index"), ctnkeyname.c_str());;
			}
			TianShanIce::Repository::ValueIdxPtr idxValue = itMetaData->second.idxValue;
			if(idxValue)
				paidids = idxValue->find(_providerAssetId);
			else
			{
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (glog, EXPFMT(LocateByPidPAidCmd, 3001, "LocateByPidPAidCmd() look up metaData[%s] is non index"), ctnkeyname.c_str());
			}
			if(paidids.empty())
			{
				TianShanIce::Repository::MetaObjectInfos result;
				result.clear();
				_amdCB->ice_response(result);
				return 1;
			}
			::Ice::Long stampSort = ZQTianShan::now();
			::std::sort(paidids.begin(), paidids.end());
			glog(ZQ::common::Log::L_INFO, CLOGFMT(LocateByPidPAidCmd, "found %d records matched [%s=%s] took total %lldms: find=%lldms, sort=%lldms"), paidids.size(), ctnkeyname.c_str(), _providerAssetId.c_str(),
				ZQTianShan::now() - stampSearch, stampSort - stampSearch, ZQTianShan::now() - stampSort);
			
/*
			// 2. search idents for pid
			stampSearch = ZQTianShan::now();
			ctnkeyname = "user.ProviderId";
			itMetaData = _env._lib->_metaDataContainerMap.find(ctnkeyname);
			if (_env._lib->_metaDataContainerMap.end() == itMetaData)
			{
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (glog, EXPFMT(LocateByPidPAidCmd, 3001, "LocateByPidPAidCmd() look up metaData[%s] is non index"), ctnkeyname.c_str());;
			}

			idxValue = itMetaData->second.idxValue;
			if(idxValue != NULL)
				pidids = idxValue->find(_providerId);
			else
			{
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (glog, EXPFMT(LocateByPidPAidCmd, 3001, "LocateByPidPAidCmd() look up metaData[%s] is non index"), ctnkeyname.c_str());
			}
			if(pidids.empty())
			{
				TianShanIce::Repository::MetaObjectInfos result;
				result.clear();
				_amdCB->ice_response(result);
				return 1;
			}
			stampSort = ZQTianShan::now();
			::std::sort(pidids.begin(), pidids.end());
			glog(ZQ::common::Log::L_INFO, CLOGFMT(LocateByPidPAidCmd, "found %d records matched [%s=%s] took total %lldms: find=%lldms, sort=%lldms"), pidids.size(), ctnkeyname.c_str(), _providerId.c_str(),
				ZQTianShan::now() - stampSearch, stampSort - stampSearch, ZQTianShan::now() - stampSort);

			// 3. get intersection idents
			ZQTianShan::IdentCollection::iterator itResult = paidids.begin();

			for (ZQTianShan::IdentCollection::iterator itObj = pidids.begin(); itResult < paidids.end() && itObj < pidids.end(); itObj++)
			{
				::Ice::Identity identObj = _env._lib->covertToObjectIdent(*itObj);
				while (itResult < paidids.end() && itResult->name.compare(identObj.name) <0)
					itResult ++;

				if(itResult == paidids.end())
					break;

				std::string nid = identObj.name.substr(identObj.name.find("@") + 1, identObj.name.find_last_of("$") - identObj.name.find("@") - 1);
				std::string vid = identObj.name.substr(identObj.name.find("$") + 1, identObj.name.size());
				if(_netId != "" && nid.compare(_netId) < 0)
					continue;
				if(_volumeId != "" && vid.compare(_volumeId) < 0)
					continue;
				if (itResult->name.compare(identObj.name) ==0)
					_objsFound.push_back(identObj);
			}

			::Ice::Long stampFinish = ZQTianShan::now();

			if(_objsFound.empty())
			{
				TianShanIce::Repository::MetaObjectInfos result;
				result.clear();
				_amdCB->ice_response(result);
				return 1;
			}

			glog(ZQ::common::Log::L_INFO, CLOGFMT(LocateByPidPAidCmd, "found %d records matched all expressions, took %lldms"), _objsFound.size(), stampFinish - stampStart);
*/
			TianShanIce::Repository::MetaObjectInfos result;

			for (ZQTianShan::IdentCollection::iterator itObj = paidids.begin(); itObj < paidids.end(); itObj++)
			{
				::Ice::Identity identObj;
				identObj.name = itObj->name;
				identObj.category = META_OBJECT;

				::TianShanIce::Repository::MetaObjectInfo objInfo;
				objInfo.id = identObj.name;
				objInfo.type = identObj.category;

				::TianShanIce::Repository::MetaDataMap metaDataMap;

//				if(!_expectedMetaDataNames.empty())
//				{
					try {
						::TianShanIce::Repository::LibMetaObjectPrx libObjectPrx =::TianShanIce::Repository::LibMetaObjectPrx::checkedCast(_env._lib->_adapter->createProxy(identObj));
						metaDataMap = libObjectPrx->getMetaDataMap();
						if (metaDataMap["user.ProviderId"].value != _providerId)
							continue;
					}
					catch(const ::Ice::Exception& ex)
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(LocateByPidPAidCmd, "getMetaDataMap of [%s] caught exception[%s]"), identObj.name.c_str(), ex.ice_name().c_str());
						continue;
					}
					catch(...)
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(LocateByPidPAidCmd, "getMetaDataMap of [%s] caught unknown exception"), identObj.name.c_str());
						continue;
					}

					for (size_t i=0; i < _expectedMetaDataNames.size(); i++)
					{
						::TianShanIce::Repository::MetaDataMap::iterator map_iter = metaDataMap.find(_expectedMetaDataNames[i]);
						if(map_iter != metaDataMap.end())
							objInfo.metaDatas.insert(::TianShanIce::Repository::MetaDataMap::value_type(_expectedMetaDataNames[i], map_iter->second));
					}
//				}

				result.push_back(objInfo);
			}

			_amdCB->ice_response(result);
			glog(ZQ::common::Log::L_INFO, CLOGFMT(LocateByPidPAidCmd, "finish the lookup, took %lldms"), ZQTianShan::now() - stampStart);
			return 1;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_amdCB->ice_exception(ex);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "LocateByPidPAidCmd caught exception[%s: %s]", ex.ice_name().c_str(), ex.message.c_str());
			lastError = buf;
		}
		catch(const ::Ice::Exception& ex)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "LocateByPidPAidCmd caught exception[%s]", ex.ice_name().c_str());
			lastError = buf;
		}
		catch(...)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "LocateByPidPAidCmd caught unknown exception");
			lastError = buf;
		}

		TianShanIce::ServerError ex("LocateByPidPAidCmd", 501, lastError);
		_amdCB->ice_exception(ex);

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateByPidPAidCmd, "LocateByPidPAidCmd() leave"));
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_amdCB->ice_exception(ex);
		return 1;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LocateByPidPAidCmd caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LocateByPidPAidCmd caught unknown exception");
		lastError = buf;
	}

	TianShanIce::ServerError ex("LocateByPidPAidCmd", 501, lastError);
	_amdCB->ice_exception(ex);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateByPidPAidCmd, "LocateByPidPAidCmd() leave"));
	return 1;
}

//////////////////////////////////////
/////////class LocateVolumesByNidCmd ///////
//////////////////////////////////////
LocateVolumesByNidCmd::LocateVolumesByNidCmd(ContentLibEnv& env, const ::TianShanIce::Repository::AMD_ContentLib_locateVolumesByNetIDPtr& amdCB, const ::std::string& netId, const ::TianShanIce::StrValues& expectedMetaDataNames)
	: ZQ::common::ThreadRequest(*(env._pThreadPool)), _env(env), _amdCB(amdCB), _netId(netId), 
	_expectedMetaDataNames(expectedMetaDataNames)
{
}

LocateVolumesByNidCmd::~LocateVolumesByNidCmd()
{
}

int LocateVolumesByNidCmd::run(void)
{
	std::string lastError;
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibImpl, "LocateVolumesByNidCmd() enter"));
	try
	{
		TianShanIce::Repository::MetaObjectInfos ret;
		ZQTianShan::IdentCollection _objsFound, volumeIds;
		::Ice::Long stampStart = ZQTianShan::now();

		try {
			::Ice::Long stampSearch = ZQTianShan::now();
#if ICE_INT_VERSION / 100 >= 306
			RLockT <IceUtil::RWRecMutex> lk(_env._lib->_metaDataContainerLocker);
#else
			IceUtil::RLockT <IceUtil::RWRecMutex> lk(_env._lib->_metaDataContainerLocker);
#endif
			std::string ctnkeyname = OBJECTTYPE;

			// 1. search idents of METAVOLUME
			ZQTianShan::MetaLib::MetaLibImpl::MetaDataContainerMap::iterator itMetaData = _env._lib->_metaDataContainerMap.find(ctnkeyname);
			if (_env._lib->_metaDataContainerMap.end() == itMetaData)
			{
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (glog, EXPFMT(LocateVolumesByNidCmd, 3001, "LocateByNidVidCmd() look up metaData[%s] is non index"), ctnkeyname.c_str());;
			}
			TianShanIce::Repository::ValueIdxPtr idxValue = itMetaData->second.idxValue;
			if(idxValue)
				volumeIds = idxValue->find(METAVOLUME);
			else
			{
				ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter> (glog, EXPFMT(LocateVolumesByNidCmd, 3001, "LocateByNidVidCmd() look up metaData[%s] is non index"), ctnkeyname.c_str());
			}
			if(volumeIds.empty())
			{
				TianShanIce::Repository::MetaObjectInfos result;
				result.clear();
				_amdCB->ice_response(result);
				return 1;
			}
			::Ice::Long stampSort = ZQTianShan::now();
			::std::sort(volumeIds.begin(), volumeIds.end());
			glog(ZQ::common::Log::L_INFO, CLOGFMT(LocateVolumesByNidCmd, "found %d records matched [%s=%s] took total %lldms: find=%lldms, sort=%lldms"), volumeIds.size(), OBJECTTYPE, METAVOLUME,
				ZQTianShan::now() - stampSearch, stampSort - stampSearch, ZQTianShan::now() - stampSort);

			// 1. compare NetId
			for (ZQTianShan::IdentCollection::iterator itObj = volumeIds.begin(); itObj < volumeIds.end(); itObj++)
			{
				std::string nid = itObj->name.substr(itObj->name.find("@") + 1, itObj->name.find_first_of("$") - itObj->name.find("@") - 1);
				if (_netId.compare(nid) == 0 || _netId == "")
					_objsFound.push_back(*itObj);
			}

			TianShanIce::Repository::MetaObjectInfos result;

			for (ZQTianShan::IdentCollection::iterator itObj = _objsFound.begin(); itObj < _objsFound.end(); itObj++)
			{
				::Ice::Identity identObj;
				identObj.name = itObj->name;
				identObj.category = META_OBJECT;

				::TianShanIce::Repository::MetaObjectInfo objInfo;
				objInfo.id = identObj.name;
				objInfo.type = identObj.category;

				::TianShanIce::Repository::MetaDataMap metaDataMap;

				if(!_expectedMetaDataNames.empty())
				{
					try {
						::TianShanIce::Repository::LibMetaObjectPrx libObjectPrx =::TianShanIce::Repository::LibMetaObjectPrx::checkedCast(_env._lib->_adapter->createProxy(identObj));
						metaDataMap = libObjectPrx->getMetaDataMap();
					}
					catch(const ::Ice::Exception& ex)
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(LocateVolumesByNidCmd, "getMetaDataMap of [%s] caught exception[%s]"), identObj.name.c_str(), ex.ice_name().c_str());
						continue;
					}
					catch(...)
					{
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(LocateVolumesByNidCmd, "getMetaDataMap of [%s] caught unknown exception"), identObj.name.c_str());
						continue;
					}

					for (size_t i=0; i < _expectedMetaDataNames.size(); i++)
					{
						::TianShanIce::Repository::MetaDataMap::iterator map_iter = metaDataMap.find(_expectedMetaDataNames[i]);
						if(map_iter != metaDataMap.end())
							objInfo.metaDatas.insert(::TianShanIce::Repository::MetaDataMap::value_type(_expectedMetaDataNames[i], map_iter->second));
					}
				}

				result.push_back(objInfo);
			}

			_amdCB->ice_response(result);
			glog(ZQ::common::Log::L_INFO, CLOGFMT(LocateVolumesByNidCmd, "finish the lookup, took %lldms"), ZQTianShan::now() - stampStart);
			return 1;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_amdCB->ice_exception(ex);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "LocateVolumesByNidCmd caught exception[%s: %s]", ex.ice_name().c_str(), ex.message.c_str());
			lastError = buf;
		}
		catch(const ::Ice::Exception& ex)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "LocateVolumesByNidCmd caught exception[%s]", ex.ice_name().c_str());
			lastError = buf;
		}
		catch(...)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "LocateVolumesByNidCmd caught unknown exception");
			lastError = buf;
		}

		TianShanIce::ServerError ex("LocateVolumesByNidCmd", 501, lastError);
		_amdCB->ice_exception(ex);

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateVolumesByNidCmd, "LocateVolumesByNidCmd() leave"));
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_amdCB->ice_exception(ex);
		return 1;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LocateVolumesByNidCmd caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "LocateVolumesByNidCmd caught unknown exception");
		lastError = buf;
	}

	TianShanIce::ServerError ex("LocateVolumesByNidCmd", 501, lastError);
	_amdCB->ice_exception(ex);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateVolumesByNidCmd, "LocateVolumesByNidCmd() leave"));
	return 1;
}

