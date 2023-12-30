
#include "NGODStorePortal.h"
#include "NSSConfig.h"
#include "NSSCfgLoader.h"
#include "urlstr.h"
#include <TimeUtil.h>
#include "ContentUser.h"
#include "SystemUtils.h"

//extern char *DefaultConfigPath;
//extern ::ZQ::common::Config::Loader< ::ZQTianShan::NSS::NSSCfg > pConfig;
extern ::ZQTianShan::NSS::NSSBaseConfig::NSSHolder *pNSSBaseConfig;

#define cslog (store._log)
#define CSLOGFMT(_X) CLOGFMT(NGODCS, _X)

namespace ZQTianShan {
namespace ContentStore {

using namespace ZQTianShan::NGOD_CS;
// ----------------------------------------------
// implementation of ContentStore portal entries
// ----------------------------------------------

void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
		return;

	//config for contentstore information
	store._netId = pNSSBaseConfig->netId;
	if (store._netId.length() == 0)
	{
		char chHost[256] = {0};
		gethostname(chHost,sizeof(chHost));
		store._netId = chHost;
	}

	cslog(ZQ::common::Log::L_DEBUG, CSLOGFMT("initializePortal() initializing ContentStorelib for NGOD content store"));

	store._storeType = "A3CS";
	store._streamableLength = 100000;

	store._replicaGroupId = pNSSBaseConfig->_videoServer.StoreReplicaGroupId;
	store._replicaId = pNSSBaseConfig->_videoServer.StoreReplicaReplicaId;
	if (store._replicaId.length() == 0)
		store._replicaId = store._netId;

	store._replicaPriority = pNSSBaseConfig->_videoServer.StoreReplicaReplicaPriority;
	store._replicaTimeout = pNSSBaseConfig->_videoServer.StoreReplicaTimeout;
	store._contentEvictorSize = pNSSBaseConfig->_videoServer.DatabaseCacheContentSize;
	store._volumeEvictorSize = pNSSBaseConfig->_videoServer.DatabaseCacheVolumeSize;

//#ifdef _DEBUG
	store._autoFileSystemSync = true;
//#endif// _DEBUG


	try {
		cslog(ZQ::common::Log::L_DEBUG, CSLOGFMT("initializePortal() initializing portal context for NGOD content store"));
		NGODStorePortal* a3Ctr = new NGODStorePortal(store, 5, pNSSBaseConfig->_videoServer.ContentInterfaceSyncInterval);
		store._ctxPortal = (void*) a3Ctr;

		if (NULL == store._ctxPortal)
		{
			cslog(ZQ::common::Log::L_ERROR, CSLOGFMT("initializePortal() failed to initialize portal context"));
			return;
		}

		a3Ctr->start();
	}
	catch (...)
	{
		cslog(ZQ::common::Log::L_ERROR, CSLOGFMT("initializePortal() caught exception"));
		return;
	}
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store)
{
	try {
		if (NULL != store._ctxPortal)
		{
			cslog(ZQ::common::Log::L_DEBUG, CSLOGFMT("uninitializePortal() cleaning up the context NGOD content store"));

			NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);
			pPortalCtx->quit();
			
			SYS::sleep(1); // yield the current thread
			delete ((NGODStorePortal*) store._ctxPortal);
		}
	}
	catch (...)
	{
		cslog(ZQ::common::Log::L_ERROR, CSLOGFMT("uninitializePortal() caught exception"));
		return;
	}

	store._ctxPortal = NULL;
}

std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname)
{
	return pathname;
}

#define ASSET_PORTAL(_FUNC) if (NULL == store._ctxPortal) { \
	cslog(ZQ::common::Log::L_ERROR, CSLOGFMT(#_FUNC " asset portal context failed")); return false; }
									

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume, const std::string& volumeName)
{
	ASSET_PORTAL(createPathOfVolume);

	NGODStorePortal::VolumeInfo vi;
	vi.targetName  = pathOfVolume;

	size_t pos;
	// Cisco's NGOD Volume is named as "/files"
	// pos = vi.targetName.find_first_not_of(FNSEPS LOGIC_FNSEPS);
	// if (pos >0)
	// 	vi.targetName = vi.targetName.substr(pos);
	// pos = vi.targetName.find_last_not_of(FNSEPS LOGIC_FNSEPS);
	// if (pos >0)
	//	vi.targetName = vi.targetName.substr(0, pos+1);

	// trim slash off the volume name
	std::string volname = volumeName;
	pos = volname.find_first_not_of(FNSEPS LOGIC_FNSEPS);
	if (pos >0)
		volname = volname.substr(pos);
	pos = volname.find_last_not_of(FNSEPS LOGIC_FNSEPS);
	if (pos >0)
		volname = volname.substr(0, pos+1);

	if (vi.targetName.length() <=0 || volname.length() <=0)
	{
		cslog(ZQ::common::Log::L_ERROR, CSLOGFMT("illegal parameter: pathOfVolume[%s] volname[%s]"), pathOfVolume.c_str(), volname.c_str());
		return false;
	}
	
	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);

	{
		ZQ::common::MutexGuard g(pPortalCtx->_lockVolumes);
		NGODStorePortal::VolumeMap::iterator it = pPortalCtx->_volumeMap.find(volname);

		if (pPortalCtx->_volumeMap.end() != it)
		{
			store._log(ZQ::common::Log::L_WARNING, CSLOGFMT("createPathOfVolume() volume[%s] has already existed"), volname.c_str());
			return false;
		}

		//	pPortalCtx->_volumeMap.insert(NGODStorePortal::VolumeMap::value_type(vi.name, vi));
		MAPSET(NGODStorePortal::VolumeMap, pPortalCtx->_volumeMap, volname, vi);
		MAPSET(TianShanIce::Properties, pPortalCtx->_volTargetNameIdx, vi.targetName, volname);

		store._log(ZQ::common::Log::L_INFO, CSLOGFMT("createPathOfVolume() volume[%s] to target[%s] added"), volname.c_str(), vi.targetName.c_str());
	}

	pPortalCtx->wakeup();
	return true;
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	ASSET_PORTAL(deletePathOfVolume);

	if (pathOfVolume.length() <=0)
	{
		cslog(ZQ::common::Log::L_ERROR, CSLOGFMT("deletePathOfVolume() illegal parameter: pathOfVolume[%s]"), pathOfVolume.c_str());
		return false;
	}

	std::string targetName = pathOfVolume;
	size_t len = targetName.length();
	if (FNSEPC == targetName[len -1] || LOGIC_FNSEPC == targetName[len -1])
		targetName = targetName.substr(0, len -1);

	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);

	ZQ::common::MutexGuard g(pPortalCtx->_lockVolumes);
	TianShanIce::Properties::iterator it = pPortalCtx->_volTargetNameIdx.find(targetName);
	std::string volName;
	if (pPortalCtx->_volTargetNameIdx.end() != it)
		volName = it->second;

	pPortalCtx->_volumeMap.erase(volName);
	pPortalCtx->_volTargetNameIdx.erase(targetName);

	store._log(ZQ::common::Log::L_DEBUG, CSLOGFMT("deletePathOfVolume() volume[%s] to target[%s] removed from monitoring"), volName.c_str(), targetName.c_str());

	return true;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath)
{
	if (store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "getStorageSpace() The point to the portal is NULL"));
		freeMB = 0;
		totalMB = 0;
		return;
	}

	if (rootPath == NULL || strlen(rootPath) == 0)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "getStorageSpace() top path is NULL"));
		freeMB = 0;
		totalMB = 0;
		return;
	}

	std::string strVN = rootPath;
	size_t sEnd = strVN.length()-1;
	if(strVN[sEnd] == FNSEPC)
		strVN[sEnd] = '\0';

	A3Request::MessageCtx msgData;
	msgData.params["volumeName"] = strVN;

	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);
	A3Request req(*pPortalCtx);
	int state = req.request(A3Request::A3_GetVolumeInfo, msgData);
	if ((state == 200) && atoi(msgData.params["state"].c_str()) == A3Request::OPERATIONAL)
	{
		freeMB = atoi(msgData.params["freeSize"].c_str());
		totalMB = atoi(msgData.params["volumeSize"].c_str());
	}
	else
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "getStorageSpace() request error, %s, http return code is not 200 or xml state attritbute is not 200"), req.getStatusMessage().c_str());
		freeMB = 0;
		totalMB = 0;
	}
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store,std::string& fileName,const std::string& contentType)
{
	if (fileName.find("\n",0) != std::string::npos || fileName.find(" ",0) != std::string::npos
		|| fileName.find("\r",0) != std::string::npos || fileName.find("\t",0) != std::string::npos
//		|| fileName.find("\\",0) != std::string::npos || fileName.find("/",0) != std::string::npos
		)
		return false;
	
	//may be is a bug
	fileName = (char*)fileName.c_str();
	return true;
}

uint64 ContentStoreImpl::checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname)
{
	uint64 ret = 0;

	if (store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "the point to the portal is NULL"));
		return 0;
	}

	size_t nIndex = mainFilePathname.rfind(FNSEPC);
	if (nIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "checkResidentialStatus() parameter mainFilePathname (%s) not contain volume name"), mainFilePathname.c_str());
		return 0;
	}
	std::string targetVolName = mainFilePathname.substr(0, nIndex);
	std::string shortContentName       = mainFilePathname.substr(nIndex +1);

	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);
	bool bExist = false;

	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "checkResidentialStatus() parameter mainFilePathname[%s] associated target[%s] content[%s]"), mainFilePathname.c_str(), targetVolName.c_str(), shortContentName.c_str());

	NGODStorePortal::ContentInfo ci;
	if (pPortalCtx->queryInfoByContent(ci, shortContentName, targetVolName, true))
		bExist = true;

/*

		NGODStorePortal::ContentMap::iterator it;
	for (itVol = pPortalCtx->_volumeMap.begin(); itVol != pPortalCtx->_volumeMap.end(); itVol++)
	{
		if (stricmp(strVol.c_str(),(itVol->first).c_str()) == 0)
		{			
			it = (itVol->second).contents.find(strName);
			if (it != (itVol->second).contents.end())
				bExist = true;
			else
				bExist = false;	

			break;
		}
	}
*/
	
	//content exist?
	if ((flagsToTest & RSDFLAG(frfResidential)))
	{
		if (bExist)
			ret |= RSDFLAG(frfResidential);
	}

	if (!ret)
		return 0;

	if ((flagsToTest & RSDFLAG(frfWriting)))
	{
		if (bExist)
		{
			if (stricmp(ci.contentState.c_str(), A3_STATE_TRANSFER) == 0 || stricmp(ci.contentState.c_str(), A3_STATE_STREAMABLE) == 0)
				ret |= RSDFLAG(frfWriting);
		}
	}

	if ((flagsToTest & RSDFLAG(frfAbsence)))
	{
		if (bExist)
		{
			if (stricmp(ci.contentState.c_str(), A3_STATE_CANCELED) == 0 || stricmp(ci.contentState.c_str(), A3_STATE_FAILED) == 0 || stricmp(ci.contentState.c_str(), A3_STATE_PENDING) == 0)//pending is?
				ret |= RSDFLAG(frfAbsence);
		}
	}

	if ((flagsToTest & RSDFLAG(frfCorrupt)))
	{
		if (bExist)
		{
			if (stricmp(ci.contentState.c_str(), A3_STATE_CANCELED) == 0 || stricmp(ci.contentState.c_str(), A3_STATE_FAILED) == 0)
				ret |= RSDFLAG(frfCorrupt);
		}
	}

	return (ret & flagsToTest);
}

bool ContentStoreImpl::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	ASSET_PORTAL(deleteFileByContent);

	size_t nIndex = mainFilePathname.rfind(FNSEPC);
	if (nIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "deleteFileByContent() parameter mainFilePathname not contain volume name"));
		return false;
	}

//	std::string strVol  = mainFilePathname.substr(0, nIndex);
//	std::string strName = mainFilePathname.substr(nIndex +1);

	std::string targetVolName		   = mainFilePathname.substr(0, nIndex);
	std::string shortContentName       = mainFilePathname.substr(nIndex +1);

	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);

	size_t  sI = shortContentName.find("_");
	if (sI == std::string::npos)
	{
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "deleteFileByContent() the contentname[%s] is invalidate"), shortContentName.c_str());
		return true;
	}

	A3Request::MessageCtx msgData;
	msgData.params["assetID"]    = shortContentName.substr(0,sI);
	msgData.params["providerID"] = shortContentName.substr(sI+1);
	msgData.params["volumeName"] = targetVolName;
	char chdelcode[10] = {0};
	sprintf(chdelcode, "%d", A3Request::DEL_OPR_INIT);
	msgData.params["reasonCode"] = chdelcode;

	A3Request req(*pPortalCtx);
	int state = req.request(A3Request::A3_DeleteContent, msgData);

	if ((state >= 200 && state < 300) || 404==state || 410 ==state) //delete the content, the update the contentstore list
	{
		ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
		TianShanIce::Properties::iterator itIdx = pPortalCtx->_volTargetNameIdx.find(targetVolName);
		std::string volName;
		if (pPortalCtx->_volTargetNameIdx.end() != itIdx)
			volName = itIdx->second;

		NGODStorePortal::VolumeMap::iterator itVol = pPortalCtx->_volumeMap.find(volName);
		if (pPortalCtx->_volumeMap.end() != itVol)
		{
			itVol->second.contents.erase(shortContentName);
			store._log(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "deleteFileByContent() content[%s] portal record removed from vol[%s]=>[%s] per[%d]"), shortContentName.c_str(), volName.c_str(), targetVolName.c_str(), state);
		}

		return true;
/*
		for (itVol = pPortalCtx->_volumeMap.begin(); itVol != pPortalCtx->_volumeMap.end(); itVol++)
		{
			if (stricmp(strVol.c_str(),(itVol->first).c_str()) == 0)
			{
				NGODStorePortal::ContentMap::iterator it;
				it = itVol->second.contents.find(strName);
				if (it != itVol->second.contents.end())
				{
					itVol->second.contents.erase(it);					
				}
				break;		
			}
		}	
*/
	}

	switch (state)
	{
	case -1:
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "deleteFileByContent() delete content[%s] error[%d]: network interrupt"), mainFilePathname.c_str(), state);
		return false;

	case 409:
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "deleteFileByContent() delete content[%s] error[%d]: %s"), mainFilePathname.c_str(), state, req.getStatusMessage().c_str());
		return false;

	default:
		store._log(ZQ::common::Log::L_WARNING, CLOGFMT(NGODCS, "deleteFileByContent() delete content[%s] error[%d]: %s"), mainFilePathname.c_str(), state, req.getStatusMessage().c_str());
		return false;
	}

	return true;
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath)
//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt) 
{

	if (NULL == store._ctxPortal)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(NGODCS, 1004, "listMainFiles() illegal portal context"));

	std::string targetVolName = (NULL != rootPath) ? rootPath : "";
	std::string volName;

	size_t len = targetVolName.length();
	if (targetVolName[len -1] == FNSEPC) 
		targetVolName = targetVolName.substr(0, len -1); 
	
	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);

	//wait until get the Content list
	bool bNeedWait = false, bListed= false;
	uint cWait =0;
	int64 stamp1 = ZQTianShan::now();
	ContentStoreImpl::FileInfos ret;

	while (!pPortalCtx->bQuit())
	{
		if (bNeedWait)
		{
			pPortalCtx->_hStartList.wait(1* 60 *1000); // up to 1min
			if (pPortalCtx->bQuit()) 
				break;
			cWait++;
		}

		{
			ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
			TianShanIce::Properties::iterator it = pPortalCtx->_volTargetNameIdx.find(targetVolName);
			if (pPortalCtx->_volTargetNameIdx.end() != it)
				volName = it->second;

			if (pPortalCtx->_volumeMap.end() == pPortalCtx->_volumeMap.find(volName))
				ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(NGODCS, 1001, "listMainFiles() volume[%s] not found, rootPath=%s"), volName.c_str(), rootPath);
			
			NGODStorePortal::VolumeInfo& vi = pPortalCtx->_volumeMap[volName];
			if (vi.stampLastSync == 0)
			{
				bNeedWait = (cWait < CONTENT_SYNC_TIMEOUT_MINUTE);
				store._log((cWait > (CONTENT_SYNC_TIMEOUT_MINUTE-2) ?ZQ::common::Log::L_WARNING : ZQ::common::Log::L_DEBUG), CLOGFMT(NGODCS, "listMainFiles() volume[%s] has never been sync-ed, tried this query for %dmin ..."), volName.c_str(), cWait);
//				pPortalCtx->_hStartList.reset();
				if (bNeedWait)
					continue;
			}

			bNeedWait = false;

			ContentStoreImpl::FileInfo fi;
			char stampBuf[64] = {0};
			ZQTianShan::TimeToUTC(ZQTianShan::now(), stampBuf, sizeof(stampBuf));
			
			for (NGODStorePortal::ContentMap::const_iterator it = vi.contents.begin(); it != vi.contents.end(); it ++)
			{
				fi.filename = it->second.name;
				fi.stampLastWrite = stampBuf;
				ret.push_back(fi);
			}

			bListed = true;
		}

		break;
	}

	if (!bListed)
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (store._log, EXPFMT(NGODCS, 1002, "listMainFiles() volume[%s] failed to list the recent contents"), volName.c_str());

	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "listMainFiles() volume[%s] found %d contents, took "FMT64"ms"), volName.c_str(), ret.size(), ZQTianShan::now() -stamp1);

	return ret;
}

std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename)
{
	size_t nS = memberFilename.rfind(FNSEPS);
	if(nS != std::string::npos)
		return memberFilename.substr(nS+1);

	return memberFilename;
}


bool ContentStoreImpl::completeRenaming(ContentStoreImpl& store, const ::std::string& oldName, const ::std::string& newName)
{
	return false; // NGOD A3 never supports renaming
}

bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname)
{
	ASSET_PORTAL(populateAttrsFromFile);

	if (mainFilePathname.length() ==0)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "populateAttrsFromFile() mainfile is NULL"));
		return false;
	}

	size_t pos = mainFilePathname.find(FNSEPS ,0);
	if (std::string::npos == pos)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "populateAttrsFromFile() invalid mainFilePathname[%s] with no " FNSEPS), mainFilePathname.c_str());
		return false;
	}

	store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODCS, "populateAttrsFromFile() enter"));
	std::string targetVolName = mainFilePathname.substr(0, pos);
	std::string contentName = mainFilePathname.substr(pos +1);
	
	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);

	NGODStorePortal::ContentInfo ci;
	if (!pPortalCtx->queryInfoByContent(ci, contentName, targetVolName, true))
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "populateAttrsFromFile() mainFilePathname[%s] of targetVol[%s] not found"), mainFilePathname.c_str(), targetVolName.c_str());
		return false;
	}

	/*
	{
		ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
		TianShanIce::Properties::iterator it = pPortalCtx->_volTargetNameIdx.find(targetVolName);
		if (pPortalCtx->_volTargetNameIdx.end() != it)
			volName = it->second;

		NGODStorePortal::VolumeMap::const_iterator itVol = pPortalCtx->_volumeMap.find(volName);
		if (pPortalCtx->_volumeMap.end() == itVol)
		{
			store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "populateAttrsFromFile() mainFilePathname[%s]: volume[%s] not found"), mainFilePathname.c_str(), volName.c_str());
			return false;
		}

		NGODStorePortal::ContentMap::const_iterator itCont = itVol->second.contents.find(contentName);
		if (itVol->second.contents.end() == itCont)
		{
			store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "populateAttrsFromFile() mainFilePathname[%s]: content[%s] not found in volume[%s]"), mainFilePathname.c_str(), contentName.c_str(), volName.c_str());
			return false;
		}

		ci = itCont->second;
	}
	*/
		
	store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(NGODCS, "populateAttrsFromFile() updating content[%s] metadata"), contentName.c_str());

	char buf[256];
	if (ci.contentSize >0) // METADATA_FileSize
	{
		sprintf(buf, FMT64, ci.contentSize);
		MAPSET(::TianShanIce::Properties, content.metaData, METADATA_FileSize, buf);
	}

	if (ci.supportFileSize >0) // METADATA_SupportFileSize
	{
		sprintf(buf, FMT64, ci.supportFileSize);
		MAPSET(::TianShanIce::Properties, content.metaData, METADATA_SupportFileSize, buf);
	}

	if (ci.bitRate >0) // METADATA_BitRate
	{
		sprintf(buf, "%d", ci.bitRate);
		MAPSET(::TianShanIce::Properties, content.metaData, METADATA_BitRate, buf);
	}

	if (!ci.md5Checksum.empty()) // METADATA_MD5CheckSum
		MAPSET(::TianShanIce::Properties, content.metaData, METADATA_MD5CheckSum, ci.md5Checksum);

	if (ci.bitRate >0 && ci.contentSize >0) // METADATA_PlayTime
	{
		// playtime = filesize*8 /bitrate
		sprintf(buf, FMT64, ci.contentSize * 8 * 1000/ ci.bitRate);
		MAPSET(::TianShanIce::Properties, content.metaData, METADATA_PlayTime, buf);
	}

	return true;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& contentName, 
						const ::std::string& sourceUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
//			throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
 //                  TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	if (NULL == store._ctxPortal)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "submitProvision() illegal portal context: NULL"));
		return NULL;
	}

	if (startTimeUTC.empty() || stopTimeUTC.empty())
	{
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(ContentStore, TianShanIce::Storage::csexpInvalidTime, "submitProvision() empty startTimeUTC or stopTimeUTC"));
		return NULL;
	}

	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);
//	std::string strName = content.ident.name.substr(content.identVolume.name.length()+1);//content.getName(Ice::Current());

	NGODStorePortal::ContentInfo newCI;
	newCI.name            = contentName;
	newCI.contentSize     = 0;
	newCI.supportFileSize = 0;
	newCI.stampCreated    = ZQTianShan::now();
	newCI.md5Checksum     = "";
	newCI.stampMD5        = 0;
	newCI.contentState    = "";

	size_t pos = contentName.find("_");
	if (std::string::npos == pos)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "provision() illegal contentname[%s] to chop for PID PAID"), contentName.c_str());

	A3Request::MessageCtx msgData;
	msgData.params["assetID"]    = contentName.substr(0,pos);
	msgData.params["providerID"] = contentName.substr(pos+1);

	//get mount path
	TianShanIce::Storage::VolumeExPrx vol =  content._volume();
	std::string volName = vol->getName();

	std::string targetVolName = vol->getMountPath();
	size_t len = targetVolName.length();
	if (FNSEPC == targetVolName[len -1] || LOGIC_FNSEPC == targetVolName[len -1])
		targetVolName = targetVolName.substr(0, len -1);

	msgData.params["volumeName"] = targetVolName;

	{
		char buf[1024] = {0};
		snprintf(buf, sizeof(buf) -2, "http://%s/", pPortalCtx->getResponseAddr().c_str());
		msgData.params["responseURL"] = buf;
	}

	//source URL parse
	std::string strS = sourceUrl;
	size_t index = strS.find("@",0);
	size_t sBI = strS.find("//");
	size_t sEI = strS.find(":", sBI);

	std::string proto;
	if (sBI != std::string::npos)
		proto = strS.substr(0, sBI+2);

	if (index == std::string::npos) //not find
		msgData.params["sourceURL"] = strS;
	else
	{
		if (sBI == std::string::npos || sEI == std::string::npos || sEI > index)
		{
			ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(ContentStoreImpl, TianShanIce::Storage::csexpInvalidSourceURL, "sourceURL '%s' invalidate"),sourceUrl.c_str());
			return NULL;
		}

		msgData.params["sourceURL"] = proto + strS.substr(index+1);
		msgData.params["userName"] = strS.substr(sBI+2,sEI-sBI-2);
		msgData.params["password"] = strS.substr(sEI+1,index-sEI-1);
	}
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "submitProvision() parsed sourceUrl[%s]: url[%s] user[%s] passwd[%s]"), sourceUrl.c_str(), msgData.params["sourceURL"].c_str(), msgData.params["userName"].c_str(), msgData.params["password"].c_str());

	std::transform(proto.begin(), proto.end(), proto.begin(), (int(*)(int)) tolower);

	bool bCapture =false;
	if (proto == "udp://")
		bCapture = true;

	if (startTimeUTC.length() >12)
		msgData.params["captureStart"] = startTimeUTC;

	if (stopTimeUTC.length() >12)
		msgData.params["captureEnd"] = stopTimeUTC;

	char chbitrate[30] = {0};
	if (maxTransferBitrate <= 0)
	{
		sprintf(chbitrate,"%d", pNSSBaseConfig->_videoServer.vols[0].defaultBitRate);
		msgData.params["transferBitRate"] = chbitrate;
		
	}
	else
	{
		sprintf(chbitrate, "%d", maxTransferBitrate);
		msgData.params["transferBitRate"] = chbitrate;
	}

	::TianShanIce::Properties::iterator iter = content.metaData.find(METADATA_SubscriberId);
	if (content.metaData.end() != iter)
		MAPSET(::TianShanIce::Properties, msgData.params, "homeID", iter->second); // for the case of per-subscribe nPVR
	else
	{
		 // ??? for WHICH 3rd party's Video Server?
		// Cisco video server always requires homeID
		if (std::string::npos != pNSSBaseConfig->_videoServer.ContentInterfaceMode.find("Cisco"))
			MAPSET(::TianShanIce::Properties, msgData.params, "homeID", "0");
	}
		
	// vendor based customization start here
	if (0 == pNSSBaseConfig->_videoServer.ContentInterfaceMode.compare("SeaChange"))
	{
		::TianShanIce::Properties props;
		// processing SeaChange's customized Content metadata: usefileset, cscontenttype and cscontenttype
		if (::std::string::npos == sourceType.find(":"))
			props["cscontenttype"] = sourceType;
		else
		{
			props["usefileset"] = "TRUE";
			::TianShanIce::StrValues strValue = ::ZQ::common::stringHelper::split(sourceType, ':');
			if (strValue.size() == 2)
			{
				if (0 == strValue[1].compare("VVX"))
					props["cscontenttype"] = "MPEG2TS";
				else if (0 == strValue[1].compare("VV2"))
					props["cscontenttype"] = "H264";
			}
		}
		msgData.table.push_back(props);
	}
	else if (0 == pNSSBaseConfig->_videoServer.ContentInterfaceMode.compare("Huawei"))
	{
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "content[%s] submitProvision() customizing for Huawei"), content.ident.name.c_str());
		if (!bCapture)
		{
			msgData.params.erase("captureStart");
			msgData.params.erase("captureEnd");
		}
	}
	else if (0 == pNSSBaseConfig->_videoServer.ContentInterfaceMode.compare("Concurrent"))
	{
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "content[%s] submitProvision() customizing for Concurrent"), content.ident.name.c_str());
		
		// JSYX CR009- Concurrent doesn't take homeID anyway
		msgData.params.erase("homeID");

		// JSYX CR009- Concurrent doesn't take captureStart and captureEnd for non-recording
		if (!bCapture)
		{
			msgData.params.erase("captureStart");
			msgData.params.erase("captureEnd");
		}
	}

	if (pNSSBaseConfig->_videoServer.urlPercentDecodeOnOutgoingMsg > 0)
	{
		char buf[2048];
		buf[sizeof(buf)-1] = 0 ;
		std::string& url = msgData.params["sourceURL"];
		if ( ZQ::common::URLStr::decode(url.c_str(), buf, sizeof(buf)-2) )
			url = buf;
	}

	bool bRecordAdded = false;
	{
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "submitProvision() adding portal record vol[%s]-target[%s] content[%s]"), 
				volName.c_str(), targetVolName.c_str(), newCI.name.c_str());

		ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
		NGODStorePortal::VolumeMap::iterator itVol = pPortalCtx->_volumeMap.find(volName);
		if (pPortalCtx->_volumeMap.end() == itVol)
		{
			store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "submitProvision() content[%s] portal record volume[%s] target[%s] not found"), content.ident.name.c_str(), volName.c_str(), targetVolName.c_str());
			return NULL;
		}

		if (itVol->second.contents.end() == itVol->second.contents.find(newCI.name))
		{
			itVol->second.contents.insert(NGODStorePortal::ContentMap::value_type(newCI.name, newCI));
			bRecordAdded = true;
		}
	}

	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "content[%s] submitProvision() issuing A3 request"), content.ident.name.c_str());
	A3Request req(*pPortalCtx);
	int errorCode = req.request(A3Request::A3_TransferContent, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode >=200 && errorCode< 300) //ok
	{
		store._log(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "submitProvision() provideID[%s] assetID[%s] volumeName[%s] captureStart[%s] captureEnd[%s] transferBitRate[%s] sourceURL[%s] responseURL[%s] ,provisioning submitted [%s]"), 
				msgData.params["providerID"].c_str(), msgData.params["assetID"].c_str(), msgData.params["volumeName"].c_str(), msgData.params["captureStart"].c_str(), msgData.params["captureEnd"].c_str(), msgData.params["transferBitRate"].c_str(),
				msgData.params["sourceURL"].c_str(), msgData.params["responseURL"].c_str(), statusMsg.c_str());

		return NULL;
	}

	if (bRecordAdded)
	{
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "submitProvision() withdrawing portal record vol[%s]-target[%s] content[%s] per TransferContent failed[%d]"), 
					volName.c_str(), targetVolName.c_str(), newCI.name.c_str(), errorCode);

		ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
		NGODStorePortal::VolumeMap::iterator itVol = pPortalCtx->_volumeMap.find(volName);
		if (pPortalCtx->_volumeMap.end() != itVol)
			itVol->second.contents.erase(newCI.name);
	}

	// merge to the exising errorCode of TianShanIce::Storage
	switch (errorCode)
	{
	case 400:
	case 403:
	case 452:
		errorCode = TianShanIce::Storage::csexpInvalidParam;
		break;
	case 401:
		errorCode = TianShanIce::Storage::csexpUnauthorized;
		break;
	case 404:
		errorCode = TianShanIce::Storage::csexpContentNotFound;
		break;
	case 409:
		errorCode = TianShanIce::Storage::csexpNoResource;
		break;
	case 451:
		errorCode = TianShanIce::Storage::csexpUnsupportProto;
		break;
	case 500:
	case -1:
	default:
		errorCode = TianShanIce::Storage::csexpInternalError;
		break;
	}

	ZQTianShan::_IceThrow<TianShanIce::ServerError>(store._log, EXPFMT(ContentStoreImpl, errorCode, "content[%s] provision failed: %s"), contentName.c_str(), statusMsg.c_str());

	return NULL;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
						::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	return NULL; //???
}

std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params)
{
	std::string url;
	if (NULL == store._ctxPortal)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "getExportURL() illegal portal context: NULL"));
		return url;
	}

	std::string contentName = contentkey.content;
	size_t pos = contentName.find("_");
	if (std::string::npos == pos)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "getExportURL() illegal contentname[%s] to chop for PID PAID"), contentName.c_str());

	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);
	A3Request::MessageCtx msgData;

	msgData.params["assetID"] = contentName.substr(0, pos);
	msgData.params["providerID"] = contentName.substr(pos +1);

//	msgData.volumeName = content.identVolume.name.substr(1);
	TianShanIce::Storage::VolumeExPrx vol =  content._volume();
	msgData.params["volumeName"] = vol->getMountPath();
	pos = msgData.params["volumeName"].length() -1;
	if (msgData.params["volumeName"][pos] == FNSEPC)
		msgData.params["volumeName"] = msgData.params["volumeName"].substr(0, pos);

//	msgData.transferBitRate = content.getBitRate(Ice::Current());
	char buf[64] = {0};
	snprintf(buf, sizeof(buf) -2, "%d", transferBitrate);
	msgData.params["transferBitRate"] = buf;
	msgData.params["protocol"] = transferProtocol;

	std::string A3Url = pPortalCtx->getA3Url();
	
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "getExportURL() content[%s] querying SS[%s] for PID[%s] PAID[%s]"), contentName.c_str(), A3Url.c_str(), msgData.params["assetID"].c_str(), msgData.params["providerID"].c_str());

	A3Request req(*pPortalCtx);
	int errorCode = req.request(A3Request::A3_ExposeContent, msgData);	
	std::string statusMsg = req.getStatusMessage();
	url = msgData.params["URL"];

	if (errorCode >=200 && errorCode < 300) //ok, fill the attribute
	{
		params.clear();
		store._log(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "getExportURL() content[%s] succeeded: %s"), contentName.c_str(), statusMsg.c_str());

		MAPSET(::TianShanIce::Properties, params, TianShanIce::Storage::expTTL, msgData.params["ttl"]);
		MAPSET(::TianShanIce::Properties, params, TianShanIce::Storage::expUserName, msgData.params["userName"]);
		MAPSET(::TianShanIce::Properties, params, TianShanIce::Storage::expPassword, msgData.params["password"]);
		MAPSET(::TianShanIce::Properties, params, TianShanIce::Storage::expTransferBitrate, msgData.params["transferBitRate"]);

		return url;
	}

	// merge to the exising errorCode of TianShanIce::Storage
	switch(errorCode)
	{
	case 404:
		errorCode = TianShanIce::Storage::csexpContentNotFound;
		break;
	case 409:
		errorCode = TianShanIce::Storage::csexpContentNotReady;
		break;
	case 451:
		errorCode = TianShanIce::Storage::csexpUnsupportProto;
		break;
	case 453:
		errorCode = TianShanIce::Storage::csexpNoResource;
		break;
	case -1:
	default:
		errorCode = TianShanIce::Storage::csexpInternalError;
		break;
	}

	ZQTianShan::_IceThrow<TianShanIce::ServerError>(store._log, EXPFMT(ContentStoreImpl, errorCode, "content[%s] getExportURL failed: %s"), contentName.c_str(), statusMsg.c_str());
	return url; // dummy statement
}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
//		throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	if (NULL == store._ctxPortal)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "cancelProvision() illegal portal context: NULL"));
		return;
	}

	NGODStorePortal* pPortalCtx = reinterpret_cast<NGODStorePortal*>(store._ctxPortal);

	std::string contentName = content._name(); //content.getName(Ice::Current());;
	size_t pos = contentName.find("_");
	if (std::string::npos == pos)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "cancelProvision() illegal contentname[%s] to chop for PID PAID"), contentName.c_str());

	A3Request::MessageCtx msgData;
	msgData.params["assetID"] = contentName.substr(0,pos);
	msgData.params["providerID"] = contentName.substr(pos+1);

	TianShanIce::Storage::VolumeExPrx vol =  content._volume();
	msgData.params["volumeName"] = vol->getMountPath();
	pos = msgData.params["volumeName"].length()-1;
	if (msgData.params["volumeName"][pos] == FNSEPC)
		msgData.params["volumeName"] = msgData.params["volumeName"].substr(0, pos);

	std::string A3Url = pPortalCtx->getA3Url();
	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODCS, "cancelProvision() content[%s] querying SS[%s] for PID[%s] PAID[%s]"), contentName.c_str(), A3Url.c_str(), msgData.params["assetID"].c_str(), msgData.params["providerID"].c_str());
	
	char chreson[10] = {0};
	sprintf(chreson, "%d", A3Request::CANCEL_OPR_INIT);

	msgData.params["reasonCode"] = chreson;

	A3Request req(*pPortalCtx);
	int errorCode = req.request(A3Request::A3_CancelTransfer, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode >=200 && errorCode < 300) //ok
	{
		store._log(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "cancelProvision() content[%s] succeeded: %s"), contentName.c_str(), statusMsg.c_str());
		return;
	}

	if (errorCode == 404) //network interrupt
		errorCode = TianShanIce::Storage::csexpContentNotFound;
	else
		errorCode = TianShanIce::Storage::csexpInternalError;

	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(store._log, EXPFMT(ContentStoreImpl, errorCode, "content[%s] cancelProvision failed: %s"), contentName.c_str(), statusMsg.c_str());
}

void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{
}

}} // namespace
