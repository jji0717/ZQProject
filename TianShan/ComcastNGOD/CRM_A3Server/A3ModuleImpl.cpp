// FileName : A3ModuleImpl.cpp
// Author   : Junming Zheng
// Date     : 2009-05
// Desc     :


#include "A3ModuleImpl.h"
#include "A3Client.h"
#include "A3Common.h"

// 
const std::string contentCategory = "A3Content";
const std::string A3EventCategory = "Content";
const std::string A3CreateEvent = "Created";
const std::string A3DestroyEvent = "Destroyed";
const std::string A3StateChangeEvent = "StateChanged";
const std::string EventField_Content = "content";
const std::string EventField_Volume = "volume";
const std::string EventField_Name = "name";

using namespace CRG::Plugin::A3Server;

// ----------------------------------- A3ContentI------------------------------------------------

A3ContentI::A3ContentI(Ice::Identity contentIdent,  
					   TianShanIce::Storage::ContentPrx contentProxy, 
					   TianShanIce::Properties contentMetaData, 
					   TianShanIce::Storage::ContentState contentState, 
					   std::string strResponseURL)
{
	ident = contentIdent;
	content = contentProxy;
	metaData = contentMetaData;
	state = contentState;
	responseURL = strResponseURL;
	assetKey = ident.name.substr(0, ident.name.find("@"));
	fullVol =  ident.name.substr(ident.name.find("@") + 1);
}

A3ContentI::A3ContentI()
{

}

A3ContentI::~A3ContentI()
{

}

void A3ContentI::getAssetId(::std::string& providerId, ::std::string& providerAssetId, const ::Ice::Current& cur) const
{
	RLock rLock(*this);
	size_t nPosition = assetKey.find("#");
	if (std::string::npos == nPosition)
	{
		providerAssetId = "";
		providerId = "";
	}
	else
	{
		providerAssetId = assetKey.substr(0, nPosition);
		providerId = assetKey.substr(nPosition + 1);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3ContentI, "getAssetId() : PAID[%s], PID[%s]"), providerAssetId.c_str(), providerId.c_str());
}

Ice::Identity A3ContentI::getIdentity(const Ice::Current &cur) const
{
	RLock rLock(*this);
	return ident;
}

void A3ContentI::getVolumeInfo(std::string &contentStoreNetId, std::string &volumeName, const Ice::Current &cur) const
{

	RLock rLock(*this);
	size_t nPosition = fullVol.find("$");
	if (std::string::npos == nPosition)
	{
		contentStoreNetId = "";
		volumeName = "";
	}
	else
	{
		contentStoreNetId = fullVol.substr(0, nPosition);
		volumeName = fullVol.substr(nPosition + 1);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3ContentI, "getVolumeInfo : content store NetId[%s], volume[%s]"), contentStoreNetId.c_str(), volumeName.c_str());
}


TianShanIce::Properties A3ContentI::getMetaData(const ::Ice::Current& cur) const
{
	RLock rLock(*this);
	return metaData;
}

TianShanIce::Properties A3ContentI::getUpdateMetaData(const Ice::Current &cur)
{
	WLock wLock(*this);
	TianShanIce::Properties tempMetaData;
	bool bSuccess = true;
	try
	{
		tempMetaData.clear();
		tempMetaData = content->getMetaData();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3ContentI, "getUpdateMetaData() : catch an exception[%s]"), ex.ice_name().c_str());
		bSuccess = false;
	}
	if (bSuccess)
	{
		metaData.clear();
		metaData = tempMetaData;
	}
	return metaData;
}

TianShanIce::Storage::ContentState A3ContentI::getState(const ::Ice::Current& cur) const
{
	RLock rLock(*this);
	return state;
}

TianShanIce::Storage::ContentState A3ContentI::getUpdateState(const ::Ice::Current& cur)
{
	WLock wLock(*this);
	TianShanIce::Storage::ContentState tempState;
	bool bSuccess = true;
	try
	{
		tempState = content->getState();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3ContentI, "getUpdateState() : catch an exception [%s]"), ex.ice_name().c_str());
		bSuccess = false;
	}
	if (bSuccess)
	{
		state = tempState;
	}
	return state;
}

TianShanIce::Storage::ContentPrx A3ContentI::theContent(const Ice::Current &cur) const
{
	RLock rLock(*this);
	return content;
}

std::string A3ContentI::getResponseURL(const Ice::Current &cur) const
{
	RLock rLock(*this);
	return responseURL;
}

TianShanIce::StatedObjInfo A3ContentI::getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& cur) const
{
	RLock rLock(*this);
	TianShanIce::StatedObjInfo a3StateInfo;
	a3StateInfo.state = ContentStateToState(state);
	a3StateInfo.ident = ident;
	a3StateInfo.props.clear();
	TianShanIce::Properties::const_iterator metaDataIt;
	for (TianShanIce::StrValues::const_iterator it = expectedMetaData.begin(); 
		it != expectedMetaData.end(); it++)
	{
		metaDataIt = metaData.find(*it);
		if (metaDataIt != metaData.end())
		{
			a3StateInfo.props.insert(std::make_pair(metaDataIt->first, metaDataIt->second));
		}
	}
	return a3StateInfo;
}

void A3ContentI::OnContentEvent(const std::string &contentEventName, const TianShanIce::Properties &params, const Ice::Current &cur)
{
}

//------------------------------------ A3FacedeI ------------------------------------------------

A3FacedeI::A3FacedeI(ZQADAPTER_DECLTYPE pAdapter, 
					 Freeze::EvictorPtr a3Content,
					 A3Module::AssetIdxPtr assetIdx, 
					 A3Module::VolumeIdxPtr volumeIdx,
					 A3Client* a3Client,
					 std::map<std::string, TianShanIce::Storage::ContentStorePrx>& contentStoreProxies,
					 std::string strAdminURL)
: _pAdapter(pAdapter),  _a3Content(a3Content), _assetIdx(assetIdx), _volumeIdx(volumeIdx), _a3Client(a3Client), 
_contentStroeProxies(contentStoreProxies), _state(TianShanIce::stInService), _strAdminURL(strAdminURL)
{

}
A3FacedeI::~A3FacedeI()
{
}

::A3Module::A3ContentPrx A3FacedeI::openA3Content(const ::std::string& provideId, 
												  const ::std::string& providerAssetId, 
												  const ::std::string& contentStoreNetId, 
												  const ::std::string& volumeName, 
												  const ::Ice::Current& cur)
{
	Ice::Identity ident;
	ident.category = contentCategory;
	ident.name = providerAssetId + "#" + provideId + "@" + contentStoreNetId + "$" + volumeName;
	::A3Module::A3ContentPrx contentProxy = NULL;
	try
	{
		Ice::ObjectPrx contentBase = _pAdapter->createProxy(ident);
		contentProxy = ::A3Module::A3ContentPrx::checkedCast(contentBase);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(A3FacedeI, "openA3Content() : catch an exception [%s] when create content[%s] proxy"), ex.ice_name().c_str(), ident.name.c_str());
		return NULL;
	}
	return contentProxy;
}

::A3Module::A3Contents A3FacedeI::findContentsByAsset(const ::std::string& providerId, 
													  const ::std::string& providerAssetId, 
													  const ::Ice::Current& cur)
{
	::A3Module::A3Contents contentProxies;
	contentProxies.clear();
	::A3Module::A3ContentPrx contentProxy = NULL;
	std::string strAssetId = providerAssetId + "#" + providerId;
	std::vector<Ice::Identity> contentsIdent = _assetIdx->find(strAssetId);
	std::vector<Ice::Identity>::iterator identIt = contentsIdent.begin();
	for (; identIt != contentsIdent.end(); identIt++)
	{
		try
		{
			contentProxy = ::A3Module::A3ContentPrx::uncheckedCast(_pAdapter->createProxy(*identIt));
			contentProxies.push_back(contentProxy);
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "findContentByAsset() : catch an exception [%s] when create content[%s] proxy"), ex.ice_name().c_str(), identIt->name.c_str());
		}
	}
	return contentProxies;
}

::A3Module::A3Contents A3FacedeI::listContentsByVolume(const std::string &contentStoreNetId, 
													   const std::string &volumeName, 
													   const Ice::Current &cur)
{
	::A3Module::A3Contents contentProxies;
	contentProxies.clear();
	::A3Module::A3ContentPrx contentProxy = NULL;
	std::string strFullVol = contentStoreNetId + "$" + volumeName;
	std::vector<Ice::Identity> contentsIdent = _volumeIdx->find(strFullVol);
	std::vector<Ice::Identity>::iterator identIt = contentsIdent.begin();
	for (; identIt != contentsIdent.end(); identIt++)
	{
		try
		{
			contentProxy = ::A3Module::A3ContentPrx::uncheckedCast(_pAdapter->createProxy(*identIt));
			contentProxies.push_back(contentProxy);
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "listContentByVolume() : catch an exception [%s] when create content[%s] proxy"), ex.ice_name().c_str(), identIt->name.c_str());
		}
	}
	return contentProxies;
}

::A3Module::A3Assets A3FacedeI::listAssets(const ::std::string&providerId, 
										   const ::std::string&providerAssetId, 
										   ::Ice::Int maxCount,
										   bool included,
										   const ::Ice::Current& cur)
{
	IceUtil::RWRecMutex::RLock lock(_assetMutex);
	::A3Module::A3Assets a3Assets;
	a3Assets.clear();
	::A3Module::A3Assets::iterator assetIter;
	if (providerId.empty() || providerAssetId.empty())
	{
		assetIter = _a3Assets.begin();
	}
	else
	{
		assetIter = _a3Assets.find(providerAssetId + "_" + providerId);
		if (!included)
		{
			if (assetIter != _a3Assets.end())
			{
				assetIter++;
			}
			else
			{
				return a3Assets; // return empty list;
			}
		}
	}
	for (Ice::Int i = 0; i < maxCount && assetIter != _a3Assets.end(); i++)
	{
		a3Assets.insert(*assetIter);
		assetIter++;
	}
	return a3Assets;
}

bool A3FacedeI::deleteA3Content(const ::std::string& provideId, 
								const ::std::string& providerAssetId, 
								const ::std::string& contentStoreNetId, 
								const ::std::string& volumeName, 
								const ::Ice::Current& cur /* = ::Ice::Current */)
{
	try
	{
		TianShanIce::Storage::ContentPrx contentProxy = NULL;
		contentProxy = openA3Content(provideId, providerAssetId, contentStoreNetId, volumeName)->theContent();
		contentProxy->destroy();
	}
	catch (const Ice::Exception& ex)
	{
		return false;
	}
	deleteContentFromEvictor(provideId, providerAssetId, contentStoreNetId, volumeName);
	return true;
}

::std::string A3FacedeI::getAdminUri(const Ice::Current &cur)
{
	return _strAdminURL;
}

::TianShanIce::State A3FacedeI::getState(const Ice::Current &cur)
{
	return _state;
}

A3Module::A3ContentPrx A3FacedeI::addContentToEvictor(const std::string& strPID, 
													  const std::string& strPAID, 
													  const std::string& strNetId,
													  const std::string& strVolume, 
													  const std::string& strResponseURL)
{
	// check if in evictor
	std::string strFullName = strVolume + "/" + strPAID + "_" + strPID;
	std::string strLogName = strNetId + strFullName;
	A3Module::A3ContentPrx a3ContentProxy = openA3Content(strPID, strPAID, strNetId, strVolume);
	if (a3ContentProxy)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(A3FacedeI, "content[%s] is already in evictor"), strLogName.c_str());
		return a3ContentProxy;
	}

	// get content info from content store
	TianShanIce::Storage::ContentPrx contentProxy = NULL;
	if (!openContentByFullname(strNetId, strFullName, contentProxy))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "Fail to add content [%s] into evictor"), strLogName.c_str());
		return NULL;
	}
	TianShanIce::Properties contentMetaData;
	TianShanIce::Storage::ContentState contentState;
	try
	{
		contentMetaData = contentProxy->getMetaData();
		contentState = contentProxy->getState();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "Fail to add content [%s] into evictor as catch an ice exception[%s]"), strLogName.c_str(), ex.ice_name().c_str());
		return NULL;
	}

	Ice::Identity contentIdent;
	contentIdent.category = contentCategory;
	contentIdent.name = strPAID + "#" + strPID + "@" + strNetId + "$" + strVolume;
	A3ContentIPtr a3ContentPtr = new (std::nothrow) A3ContentI(contentIdent, contentProxy, 
		contentMetaData, contentState, strResponseURL);
	try
	{
		a3ContentProxy = A3Module::A3ContentPrx::uncheckedCast(_a3Content->add(a3ContentPtr, contentIdent));
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "Fail to add content [%s] into evictor as catch an ice exception[%s]"), strLogName.c_str(), ex.ice_name().c_str());
		return NULL;
	}
    addAsset(strPID, strPAID);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "Success to add content [%s] into evictor"), strLogName.c_str());
	return a3ContentProxy;
}

A3Module::A3ContentPrx A3FacedeI::addContentToEvictor(const Ice::Identity& contentIdent, 
													  const std::string& strResponseURL)
{
	size_t nPosition = contentIdent.name.find("#");
	std::string strPAID = contentIdent.name.substr(0, nPosition);

	size_t nEnd = contentIdent.name.find("@");
	std::string strPID = contentIdent.name.substr(nPosition + 1, nEnd - nPosition -1);

	nPosition = contentIdent.name.find("$");
	std::string strNetId = contentIdent.name.substr(nEnd + 1, nPosition - nEnd -1 );
	std::string strVolume = contentIdent.name.substr(nPosition + 1);

	std::string strFullName = strVolume + "/" + strPAID + "_" + strPID;
	std::string strLogName = strNetId + strFullName;

	A3Module::A3ContentPrx a3ContentProxy = openA3Content(strPID, strPAID, strNetId, strVolume);
	if (a3ContentProxy)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(A3FacedeI, "content[%s] is already in evictor"), strLogName.c_str());
		return a3ContentProxy;
	}

	TianShanIce::Storage::ContentPrx contentProxy = NULL;
	if (!openContentByFullname(strNetId, strFullName, contentProxy))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "Fail to add content [%s] into evictor"), strLogName.c_str());
		return NULL;
	}
	TianShanIce::Properties contentMetaData;
	TianShanIce::Storage::ContentState contentState;
	try
	{
		contentMetaData = contentProxy->getMetaData();
		contentState = contentProxy->getState();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "Fail to add content [%s] into evictor as catch an ice exception[%s]"), strLogName.c_str(), ex.ice_name().c_str());
		return NULL;
	}
	A3ContentIPtr a3ContentPtr = new (std::nothrow) A3ContentI(contentIdent, contentProxy, 
		contentMetaData, contentState, strResponseURL);
	try
	{
		a3ContentProxy = A3Module::A3ContentPrx::uncheckedCast(_a3Content->add(a3ContentPtr, contentIdent));
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "Fail to add content [%s] into evictor as catch an ice exception[%s]"), strLogName.c_str(), ex.ice_name().c_str());
		return NULL;
	}
    addAsset(strPID, strPAID);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "Success to add content [%s] into evictor"), strLogName.c_str());
	return a3ContentProxy;
}

void A3FacedeI::deleteContentFromEvictor(const std::string& strPID, 
										 const std::string& strPAID, 
										 const std::string& strNetId,
										 const std::string& strVolume)
{
	Ice::Identity ident;
	ident.category = contentCategory;
	ident.name = strPAID + "#" + strPID + "@" + strNetId + "$" + strVolume;
	try
	{
		_a3Content->remove(ident);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "Fail to delete content[%s] from evictor[%s]"), ident.name.c_str(), ex.ice_name().c_str());
		return ;
	}
	deleteAsset(strPID, strPAID);
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "Success to delete content[%s] from evictor"), ident.name.c_str());
}

void A3FacedeI::deleteContentFromEvictor(const Ice::Identity& contentIdent)
{
	try
	{
		_a3Content->remove(contentIdent);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "Fail to delete content[%s] from evictor[%s]"), contentIdent.name.c_str(), ex.ice_name().c_str());
		return;
	}
	deleteAsset(contentIdent);
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "Success to delete content[%s] from evictor"), contentIdent.name.c_str());
}

A3Module::A3ContentPrx A3FacedeI::createContent(const std::string& strPID, 
												const std::string& strPAID, 
												const std::string& strNetId,
												const std::string& strVolume, 
												const std::string& strResponseURL, 
												std::string strContentType)
{
	TianShanIce::Storage::VolumePrx volumeProxy = NULL;
	if (!openVolume(strNetId, strVolume, volumeProxy))
	{
		return NULL;
	}
	if ("" == strContentType)
	{
		strContentType = TianShanIce::Storage::ctMPEG2TS;
	}
	TianShanIce::Storage::ContentPrx contentProxy = NULL;
	std::string strContentName = strPAID + "_" + strPID;
	try
	{
		// true : create if not exist;
		contentProxy = volumeProxy->openContent(strContentName, strContentType, true);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "createContent() : catch an ice exception [%s] when create content [%s]"), ex.ice_name().c_str(), strContentName.c_str());
		return NULL;
	}
	return addContentToEvictor(strPID, strPAID, strNetId, strVolume, strResponseURL);
}

bool A3FacedeI::getVolumeInfo(const std::string& strNetId, const std::string& strVolume, 
							  Ice::Long& totalMB, Ice::Long& freeMB, int& state)
{
	
	TianShanIce::Storage::VolumePrx volumeProxy = NULL;
	if (!openVolume(strNetId, strVolume, volumeProxy))
	{
		return false;
	}
	state = 200;
	try
	{
		volumeProxy->getCapacity(freeMB, totalMB);
	}
	catch (const Ice::Exception& ex)
	{
		state = 501;
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "getVolumeInfo() : catch an ice exception [%s] when get capacity of volume [%s]"), ex.ice_name().c_str(), strVolume.c_str());
		return false;
	}
	return true;
}

bool A3FacedeI::openVolume(const std::string& strNetId, const std::string& strVolumeName, 
						   TianShanIce::Storage::VolumePrx& volumeProxy)
{
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	if (!getContentStoreProxy(strNetId, contentStoreProxy))
	{
		return false;
	}
	try
	{
		volumeProxy = contentStoreProxy->openVolume(strVolumeName);
		if (!volumeProxy)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "openVolume() : volume proxy is empty"));
			return false;
		}
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "openVolume() : catch an ice exception [%s] when open volume [%s]"), ex.ice_name().c_str(), strVolumeName.c_str());
		return false;
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "openVolume() : success to open volume[%s]"), strVolumeName.c_str());
	return true;
}

bool A3FacedeI::openContentByFullname(const std::string& strNetId, 
									  const std::string& strContentName, 
									  TianShanIce::Storage::ContentPrx& contentProxy)
{
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	if (!getContentStoreProxy(strNetId, contentStoreProxy))
	{
		return false;
	}
	try
	{
		contentProxy = contentStoreProxy->openContentByFullname(strContentName);
		if (!contentProxy)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "openContentByFullName() : content proxy is empty"));
			return false;
		}
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "openContentByFullName() : catch an ice exception [%s] when open content [%s]"), ex.ice_name().c_str(), strContentName.c_str());
		return false;
	}
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "openContentByFullName() : success to open content[%s]"), strContentName.c_str());
	return true;
}

void A3FacedeI::updateContentStore(const std::string& strNetId)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3FacedeI, "updateContentStore() : do update content store[%s]"), strNetId.c_str());
	StringVector a3VolumeNames;
	if (!getVolumes(strNetId, a3VolumeNames))
	{
		return;
	}
	Ice::Identity ident;
	std::vector<Ice::Identity> idents;
	for (StringVector::iterator volumeIt = a3VolumeNames.begin(); volumeIt != a3VolumeNames.end(); 
		volumeIt++)
	{
		TianShanIce::Storage::VolumePrx volumeProxy = NULL;
		if (!openVolume(strNetId, *volumeIt, volumeProxy))
		{
			continue;
		}
		TianShanIce::StrValues contentNames;
		try
		{
			contentNames = volumeProxy->listContent("*");
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "updateA3ContentLib() : catch an exception[%s] when list content in volume[%s]"), ex.ice_name().c_str(), volumeIt->c_str());
			continue;
		}
		size_t nPosition = 0;
		for (TianShanIce::StrValues::const_iterator contentIt = contentNames.begin(); 
			contentIt != contentNames.end(); contentIt++)
		{
			nPosition = contentIt->find("_");
			if (nPosition != std::string::npos)
			{
				ident.category = contentCategory;
				ident.name = contentIt->substr(0, nPosition) + "#" + contentIt->substr(nPosition + 1) +
					"@" + strNetId + "$" + *volumeIt;
				idents.push_back(ident);
			}
		} // end for all content
	} // end for all volume
	std::vector<Ice::Identity>::iterator identIt;
	A3Module::A3Contents a3Contents;
	for (StringVector::iterator volumeIt = a3VolumeNames.begin(); volumeIt != a3VolumeNames.end(); 
		volumeIt++)
	{
		a3Contents = listContentsByVolume(strNetId, *volumeIt);
		::A3Module::A3ContentPrx contentProxy = NULL;
		for (A3Module::A3Contents::iterator contentIt = a3Contents.begin(); 
			contentIt != a3Contents.end(); contentIt++)
		{
			ident = (*contentIt)->getIdentity();
			identIt = std::find(idents.begin(), idents.end(), ident);
			if ( identIt != idents.end()) // already in freeze, update it
			{
				idents.erase(identIt); 
				try
				{
					contentProxy = ::A3Module::A3ContentPrx::uncheckedCast(_pAdapter->createProxy(ident));
					contentProxy->getUpdateMetaData();
					contentProxy->getUpdateState();
				}
				catch (const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "catch an exception when update a3 content"), ex.ice_name().c_str());
				}
				addAsset(ident);
			}
			else
			{
				_a3Content->remove(ident); // erase from freeze
			}
		}
	}
	for (identIt = idents.begin(); identIt != idents.end(); identIt++)
	{
		addContentToEvictor(*identIt);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3FacedeI, "updateContentStore() : leave update content store[%s]"), strNetId.c_str());
}

void A3FacedeI::updateA3ContentLib()
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "updateA3ContentLib() : do updateA3ContentLib()"));
	std::map<std::string, TianShanIce::Storage::ContentStorePrx>::iterator contentStroeIt;
	for (contentStroeIt = _contentStroeProxies.begin(); contentStroeIt != _contentStroeProxies.end();
		contentStroeIt++)
	{
		updateContentStore(contentStroeIt->first);
	} 
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3FacedeI, "updateA3ContentLib() : leave updateA3ContentLib()")); 
}


bool A3FacedeI::getContentStoreProxy(const std::string& strNetId, 
									 TianShanIce::Storage::ContentStorePrx& contentStoreProxy )
{
	std::map<std::string, TianShanIce::Storage::ContentStorePrx>::iterator contentStroeIt;
	contentStroeIt = _contentStroeProxies.find(strNetId);
	if (contentStroeIt == _contentStroeProxies.end())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "getContentStoreProxy() : No content store net id match [%s]"), strNetId.c_str());
		return false;
	}
	contentStoreProxy = contentStroeIt->second;
	return true;
}

bool A3FacedeI::getVolumes(const std::string& strNetId, std::vector<std::string>& volumeNames)
{
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
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgHandler, "getVolumesFromStore() : catch an ice exception [%s] when list all volume"), ex.ice_name().c_str());
		return false;
	}
	for (TianShanIce::Storage::VolumeInfos::iterator volumeIt = volumeInfos.begin();
		volumeIt != volumeInfos.end(); volumeIt++)
	{
		volumeNames.push_back(volumeIt->name);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgHandler, "getVolumesFromStore() : load volume[%s]"),(volumeIt->name).c_str());
	}
	return true;
}

void A3FacedeI::sendTransferStatus(const std::string& strPID, 
								   const std::string& strPAID, 
								   const std::string& strNetId,
								   const std::string& strVolume, 
								   const std::string& contentState)
{
	A3Module::A3ContentPrx contentProxy =openA3Content(strPID, strPAID, strNetId, strVolume);
	if (!contentProxy)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "sendTransferStatus() : Content Not exist"));
		return ;
	}

	// update a3 content lib first
	TianShanIce::Properties metaData;
	bool bSuccess = true;
	if (contentState == "InService(3)")
	{
		try
		{
			metaData = contentProxy->getUpdateMetaData();
			contentProxy->getUpdateState();
		}
		catch (const Ice::Exception& ex)
		{
			bSuccess = false;
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "sendTransferStatus() : catch an exception[%s] when update a3 content"), ex.ice_name().c_str());
		}
	}

	std::string strResponseURL = contentProxy->getResponseURL();
	if ("" == strResponseURL)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3FacedeI, "sendTransferStatus() : No Response URL to send status"));
		return;
	}
	int reasonCode = 200;
	if (contentState == "OutService(4)" || contentState == "Cleaning(5)")
	{
		reasonCode = 500;
	}
	std::string strFullVol = strNetId + strVolume;
	std::ostringstream buf;
	buf << XML_HEADER ;
	buf << "<TransferStatus " ;
	buf <<    "providerID=\""   << strPID                            << "\" ";
	buf <<    "assetID=\""      << strPAID                           << "\" ";
	buf <<    "volumeName=\""   << strFullVol                        << "\" ";
	buf <<    "state=\""        << eventStateToA3State(contentState) << "\" ";
	buf <<    "reasonCode=\""   << reasonCode                        << "\" ";
	if (contentState == "InService" && bSuccess)
	{
		buf <<    "contentSize=\""       << metaData["sys.FileSize"]        << "\" ";
		buf <<    "supportFileSize=\""   << metaData["sys.SupportFileSize"] << "\" ";
		buf <<    "md5Checksum=\""       << metaData["sys.MD5CheckSum"]     << "\" ";
		buf <<    "md5DateTime=\""       << GenerateUTCTime()               << "\" ";
	}
	buf << "/>";
	std::string strContent = buf.str();
	_a3Client->SendRequest(strResponseURL, strContent);
}

void A3FacedeI::post(const ::std::string& category, ::Ice::Int eventId, const ::std::string& eventName, 
						const ::std::string& stampUTC, const ::std::string& sourceNetId, 
						const ::TianShanIce::Properties& params, const ::Ice::Current& cur)
{
	if (category != A3EventCategory || _contentStroeProxies.find(sourceNetId) == _contentStroeProxies.end())
	{
		return;
	}
	std::string strName = params.find(EventField_Name) != params.end() ? params.find(EventField_Name)->second : "";
	if ("" == strName)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3EventI, "OnEvent() : Content Name is empty"));
		return;
	}
	std::string strVolume = params.find(EventField_Volume) != params.end() ? params.find(EventField_Volume)->second : "";
	if ("" == strVolume)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(A3EventI, "OnEvent() : Volume Name is empty"));
		return;
	}
	std::string strFullName = strVolume + "/" + strName;
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3EventI, "OnEvent() : receive an event (Event name : %s, CotentName[%s]  from [%s]"), eventName.c_str(), strFullName.c_str(), sourceNetId.c_str());
	size_t nPosition = strName.find("_");
	std::string strPAID = strName.substr(0, nPosition);
	std::string strPID = strName.substr(nPosition + 1);
	if (eventName == A3CreateEvent)
	{
		addContentToEvictor(strPID, strPAID, sourceNetId, strVolume);
		return;
	}
	if (eventName == A3DestroyEvent)
	{
		deleteContentFromEvictor(strPID, strPAID, sourceNetId, strVolume);
		return;
	}
	if (eventName == A3StateChangeEvent)
	{
		std::string contentState =  params.find("newState") != params.end() ? params.find("newState")->second : "";
		sendTransferStatus(strPID, strPAID, sourceNetId, strVolume, contentState);
		return;
	}
}

void A3FacedeI::ping(Ice::Long timestamp, const Ice::Current &cur)
{
}

void A3FacedeI::addAsset(const Ice::Identity &contentIdent)
{
	size_t nPosition = contentIdent.name.find("#");
	std::string strPAID = contentIdent.name.substr(0, nPosition);

	size_t nEnd = contentIdent.name.find("@");
	std::string strPID = contentIdent.name.substr(nPosition + 1, nEnd - nPosition -1);
	addAsset(strPID, strPAID);
}

void A3FacedeI::addAsset(const std::string &strPID, const std::string &strPAID)
{
	IceUtil::RWRecMutex::WLock lock(_assetMutex);
	std::string strName = strPAID + "_" + strPID;
	A3Module::A3Assets::iterator assetIter = _a3Assets.find(strName);
	if (assetIter != _a3Assets.end())
	{
		(assetIter->second)++;
	}
	else
	{
		_a3Assets.insert(std::make_pair(strName, 1));
	}
}

void A3FacedeI::deleteAsset(const Ice::Identity &contentIdent)
{
	size_t nPosition = contentIdent.name.find("#");
	std::string strPAID = contentIdent.name.substr(0, nPosition);

	size_t nEnd = contentIdent.name.find("@");
	std::string strPID = contentIdent.name.substr(nPosition + 1, nEnd - nPosition -1);
	deleteAsset(strPID, strPAID);
}

void A3FacedeI::deleteAsset(const std::string& strPID, const std::string& strPAID)
{
	IceUtil::RWRecMutex::WLock lock(_assetMutex);
	std::string strName = strPAID + "_" + strPID;
	A3Module::A3Assets::iterator assetIter = _a3Assets.find(strName);
	if (assetIter != _a3Assets.end())
	{
		if (1 == assetIter->second)
		{
			_a3Assets.erase(assetIter);
		}
		else
		{
			(assetIter->second)--;
		}
	}
}


// --------------------------------- A3 Content factory ------------------------------

A3ContentFactory::A3ContentFactory(ZQADAPTER_DECLTYPE pAdapter)
:_pAdapter(pAdapter)
{
	if (_pAdapter)
	{
		Ice::CommunicatorPtr ic = _pAdapter->getCommunicator();
		glog(ZQ::common::Log::L_INFO, CLOGFMT(A3ContentFactory, "add factory into communicator"));
		ic->addObjectFactory(this, A3ContentI::ice_staticId());
	}
}

A3ContentFactory::~A3ContentFactory()
{

}

Ice::ObjectPtr A3ContentFactory::create(const std::string &type)
{
	if (A3ContentI::ice_staticId() == type)
	{
		return new A3ContentI();
	}
	return NULL;
}

void A3ContentFactory::destroy()
{

}

