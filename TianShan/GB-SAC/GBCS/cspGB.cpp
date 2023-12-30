#include "GBCSportal.h"
#include "GBCSConfig.h"
#include "GBCSCfgLoader.h"
#include "urlstr.h"
#include <TimeUtil.h>
#include "ContentUser.h"
#include "SystemUtils.h"

#include "GBCSa4Cmd.h"
#include "GBCSa5Cmd.h"

extern ::ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder *pGBCSBaseConfig;

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
	 GlobalReqType::setReqCmdType(GlobalReqType::GB_A4_REQ);
	 if (NULL != store._ctxPortal)
		 return;

	 //config for contentstore information
	 store._netId = pGBCSBaseConfig->netId;
	 if (store._netId.length() == 0)
	 {
		 char chHost[256] = {0};
		 gethostname(chHost,sizeof(chHost));
		 store._netId = chHost;
	 }

	 cslog(ZQ::common::Log::L_DEBUG, CSLOGFMT("initializePortal() initializing ContentStorelib for NGOD content store"));

	 store._storeType = "GB_A4";
	 store._streamableLength = 100000;

	 store._replicaGroupId = pGBCSBaseConfig->_videoServer.StoreReplicaGroupId;
	 store._replicaId = pGBCSBaseConfig->_videoServer.StoreReplicaReplicaId;
	 if (store._replicaId.length() == 0)
		 store._replicaId = store._netId;

	 store._replicaPriority = pGBCSBaseConfig->_videoServer.StoreReplicaReplicaPriority;
	 store._replicaTimeout = pGBCSBaseConfig->_videoServer.StoreReplicaTimeout;
	 store._contentEvictorSize = pGBCSBaseConfig->_videoServer.DatabaseCacheContentSize;
	 store._volumeEvictorSize = pGBCSBaseConfig->_videoServer.DatabaseCacheVolumeSize;

	 //#ifdef _DEBUG
	 store._autoFileSystemSync = true;
	 //#endif// _DEBUG


	 try {
		 cslog(ZQ::common::Log::L_DEBUG, CSLOGFMT("initializePortal() initializing portal context for NGOD content store"));
		 GBCSStorePortal* a3Ctr = new GBCSStorePortal(store, 5, pGBCSBaseConfig->_videoServer.ContentInterfaceSyncInterval);
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

			GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);
			pPortalCtx->quit();
			
			SYS::sleep(1); // yield the current thread
			delete ((GBCSStorePortal*) store._ctxPortal);
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
									

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume, const std::string& volname)
{
	ASSET_PORTAL(createPathOfVolume);

	if (pathOfVolume.length() <=0 || volname.length() <=0)
	{
		cslog(ZQ::common::Log::L_ERROR, CSLOGFMT("illegal parameter: pathOfVolume[%s] volname[%s]"), pathOfVolume.c_str(), volname.c_str());
		return false;
	}
	
	GBCSStorePortal::VolumeInfo vi;
	vi.targetName  = pathOfVolume;
	size_t len = vi.targetName.length();
	if (FNSEPC == vi.targetName[len -1] || LOGIC_FNSEPC == vi.targetName[len -1])
		vi.targetName = vi.targetName.substr(0, len -1);

	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);

	{
		ZQ::common::MutexGuard g(pPortalCtx->_lockVolumes);
		GBCSStorePortal::VolumeMap::iterator it = pPortalCtx->_volumeMap.find(volname);

		if (pPortalCtx->_volumeMap.end() != it)
		{
			store._log(ZQ::common::Log::L_WARNING, CSLOGFMT("createPathOfVolume() volume[%s] has already existed"), volname.c_str());
			return false;
		}

		//	pPortalCtx->_volumeMap.insert(GBCSStorePortal::VolumeMap::value_type(vi.name, vi));
		MAPSET(GBCSStorePortal::VolumeMap, pPortalCtx->_volumeMap, volname, vi);
		MAPSET(TianShanIce::Properties, pPortalCtx->_volTargetNameIdx, vi.targetName, volname);

		store._log(ZQ::common::Log::L_INFO, CSLOGFMT("createPathOfVolume() volume[%s] to target[%s] has been already added"), volname.c_str(), vi.targetName.c_str());
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

	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);

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
	store._log(ZQ::common::Log::L_WARNING,"getStorageSpace(), gbcs not implement yet ");//ngb have not command like A3_GetVolumeInfo
	freeMB = 0;
	totalMB = 0;
	return;
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
	uint64 ret = false;
	if (store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return 0;
	}

	size_t nIndex = mainFilePathname.rfind(FNSEPC);
	if (nIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR, "ContentStoreImpl::checkResidentialStatus() parameter mainFilePathname (%s) not contain volume name",mainFilePathname.c_str());
		return 0;
	}
	std::string targetVolName = mainFilePathname.substr(0, nIndex);
	std::string shortContentName = mainFilePathname.substr(nIndex +1);
	std::string volName;

	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);
	bool bExist = false;

	ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
	TianShanIce::Properties::iterator it = pPortalCtx->_volTargetNameIdx.find(targetVolName);
	if (pPortalCtx->_volTargetNameIdx.end() != it)
		volName = it->second;

	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, "checkResidentialStatus() parameter mainFilePathname[%s] associated as vol[%s] target[%s] content[%s]"), mainFilePathname.c_str(), volName.c_str(), targetVolName.c_str(), shortContentName.c_str());
	GBCSStorePortal::VolumeMap::iterator itVol = pPortalCtx->_volumeMap.find(volName);
	if (pPortalCtx->_volumeMap.end() == itVol)
	{
		if (!pContent)
			return ret;

		GBCSReq  gbCSReq(&(store._log), pPortalCtx->getA3Url());
		IGBCSCmd * requestCmd = NULL;
		int errorCode = 0;
		ContentImpl* content = pContent.get();
		std::string contentName = contentFullName;
		if (GlobalReqType::GB_A4_REQ == GlobalReqType::getReqCmdType())
		{
			requestCmd = new A4FileStateReq(contentName, *content);
		}
		else if(GlobalReqType::GB_A5_REQ == GlobalReqType::getReqCmdType())
		{
			requestCmd = new A5StreamStateReq(contentName, *content);
		}

		std::string respStatus("NULL");
		GBCSCmdPtr gardGBmemory(requestCmd);
		errorCode = gbCSReq.sendRequest(requestCmd);
		std::map<std::string, std::string> responseMap = gbCSReq.getStatusMsg();
		std::map<std::string, std::string>::iterator it = responseMap.find("parseHttpResponse");
		if (it != responseMap.end())
			respStatus = it->second;

		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, "checkResidentialStatus() mainFilePathname[%s] vol[%s] target[%s] content[%s], errorCode[%d], respStatus[%s]"), 
			mainFilePathname.c_str(), volName.c_str(), targetVolName.c_str(), shortContentName.c_str(), errorCode, respStatus.c_str());

		if (errorCode < 200 || errorCode >= 300
			&& respStatus != GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED])
			return ret;
	}

	GBCSStorePortal::ContentMap::iterator itContent = (itVol->second).contents.find(shortContentName);
	if (itContent != (itVol->second).contents.end())
		bExist = true;
	else
		bExist = false;	

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
			if (stricmp(itContent->second.contentState.c_str(), PORTAL_STATE_TRANSFER) == 0 || stricmp(itContent->second.contentState.c_str(), PORTAL_STATE_STREAMABLE) == 0)
				ret |= RSDFLAG(frfWriting);
		}
	}

	if ((flagsToTest & RSDFLAG(frfAbsence)))
	{
		if (bExist)
		{
			if (stricmp(itContent->second.contentState.c_str(), PORTAL_STATE_CANCELED) == 0 || stricmp(itContent->second.contentState.c_str(), PORTAL_STATE_FAILED) == 0 || stricmp(itContent->second.contentState.c_str(), PORTAL_STATE_PENDING) == 0)//pending is?
				ret |= RSDFLAG(frfAbsence);
		}
	}

	if ((flagsToTest & RSDFLAG(frfCorrupt)))
	{
		if (bExist)
		{
			if (stricmp(itContent->second.contentState.c_str(), PORTAL_STATE_CANCELED) == 0 || stricmp(itContent->second.contentState.c_str(), PORTAL_STATE_FAILED) == 0)
				ret |= RSDFLAG(frfCorrupt);
		}
	}

	return (ret & flagsToTest);
}

bool ContentStoreImpl::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);
	if (NULL == pPortalCtx)
	{
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "provision() reinterpret_cast store ctxPortal[NULL], "));
		return NULL;
	}

	size_t nIndex = mainFilePathname.rfind(FNSEPC);
	if (nIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() parameter mainFilePathname not contain volume name");
		return false;
	}

	std::string targetVolName		   = mainFilePathname.substr(0, nIndex);
	std::string shortContentName       = mainFilePathname.substr(nIndex +1);

	size_t  sI = shortContentName.find("_");
	if (sI == std::string::npos)
	{
		store._log(ZQ::common::Log::L_WARNING,"ContentStoreImpl::deleteFileByContent() the contentname[%s] is invalidate", shortContentName.c_str());
		return true;
	}

	GBCSReq  gbCSReq(&(store._log), pPortalCtx->getA3Url());
	int  errorCode = 0;
	if (GlobalReqType::GB_A4_REQ == GlobalReqType::getReqCmdType())   //a4 cmd default
	{
		A4FileDelete a4FileDel(shortContentName, content);
		errorCode = gbCSReq.sendRequest(&a4FileDel);
	}	
	else if(GlobalReqType::GB_A5_REQ == GlobalReqType::getReqCmdType())  //a5 cmd
	{
	   // n/a
		store._log(ZQ::common::Log::L_INFO, CLOGFMT(GBCSStorePortal, "deleteFileByContent() content[%s] status[A5 not defined]"), shortContentName.c_str());
		return true;
	}

	std::string respStatus("NULL");
	std::map<std::string, std::string> responseMap = gbCSReq.getStatusMsg();
	std::map<std::string, std::string>::iterator it = responseMap.find("parseHttpResponse");
	if (it != responseMap.end())
		respStatus = it->second;

	store._log(ZQ::common::Log::L_INFO, CLOGFMT(GBCSStorePortal, "deleteFileByContent() content[%s] status[%s]"), shortContentName.c_str(), respStatus.c_str());
	if (errorCode >=200 && errorCode < 300 && respStatus == GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED]) //ok
        return true;

	return false;
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath)
//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt) 
{
	if (NULL == store._ctxPortal)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(GBCSStorePortal, 1004, "listMainFiles() illegal portal context"));

	std::string targetVolName = (NULL != rootPath) ? rootPath : "";
	std::string volName;

	size_t len = targetVolName.length();
	if (targetVolName[len -1] == FNSEPC) 
		targetVolName = targetVolName.substr(0, len -1); 
	
	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);

	//wait until get the Content list
	bool bNeedWait = false, bListed= false;
	int64 stamp1 = ZQTianShan::now();
	ContentStoreImpl::FileInfos ret;	

	while (!pPortalCtx->bQuit())
	{
		if (bNeedWait)
		{
			pPortalCtx->_hStartList.wait(10* 60 *1000); // up to 10min
			if (pPortalCtx->bQuit()) 
				break;
		}

		{
			ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
			TianShanIce::Properties::iterator it = pPortalCtx->_volTargetNameIdx.find(targetVolName);
			if (pPortalCtx->_volTargetNameIdx.end() != it)
				volName = it->second;

			if (pPortalCtx->_volumeMap.end() == pPortalCtx->_volumeMap.find(volName))
				ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(GBCSStorePortal, 1001, "listMainFiles() volume[%s] not found, rootPath=%s"), volName.c_str(), rootPath);
			
			GBCSStorePortal::VolumeInfo& vi = pPortalCtx->_volumeMap[volName];
			if (vi.stampLastSync == 0 )
			{
				bNeedWait = true;
				store._log(ZQ::common::Log::L_WARNING, CLOGFMT(GBCSStorePortal, "listMainFiles() volume[%s] has never been sync-ed, pending this query..."), volName.c_str());
//				pPortalCtx->_hStartList.reset();
				continue;
			}

			bNeedWait = false;

			ContentStoreImpl::FileInfo fi;
			char stampBuf[64] = {0};
			ZQTianShan::TimeToUTC(ZQTianShan::now(), stampBuf, sizeof(stampBuf));
			
			for (GBCSStorePortal::ContentMap::const_iterator it = vi.contents.begin(); it != vi.contents.end(); it ++)
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
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (store._log, EXPFMT(GBCSStorePortal, 1002, "listMainFiles() volume[%s] failed to list the recent contents"), volName.c_str());

	store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, "listMainFiles() volume[%s] found %d contents, took "FMT64"ms"), volName.c_str(), ret.size(), ZQTianShan::now() -stamp1);

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
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() mainfile is NULL"));
		return false;
	}

	size_t pos = mainFilePathname.find(FNSEPS ,0);
	if (std::string::npos == pos)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() invalid mainFilePathname[%s] with no " FNSEPS), mainFilePathname.c_str());
		return false;
	}

	store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() enter"));
	std::string targetVolName = mainFilePathname.substr(0, pos);
	std::string contentName = mainFilePathname.substr(pos +1);
	std::string volName;
	
	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);

	GBCSStorePortal::ContentInfo ci;
	
	{
		ZQ::common::MutexGuard mg(pPortalCtx->_lockVolumes);
		TianShanIce::Properties::iterator it = pPortalCtx->_volTargetNameIdx.find(targetVolName);
		if (pPortalCtx->_volTargetNameIdx.end() != it)
			volName = it->second;

		GBCSStorePortal::VolumeMap::iterator itVol = pPortalCtx->_volumeMap.find(volName);
		if (pPortalCtx->_volumeMap.end() == itVol)
		{
			store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() mainFilePathname[%s]: volume[%s] not found"), mainFilePathname.c_str(), volName.c_str());
			return false;
		}

		GBCSStorePortal::ContentMap::const_iterator itCont = itVol->second.contents.find(contentName);
		if (itVol->second.contents.end() == itCont)
		{
			GBCSReq  gbCSReq(&(store._log), pPortalCtx->getA3Url());
			IGBCSCmd * requestCmd = NULL;
			int errorCode = 0;
			if (GlobalReqType::GB_A4_REQ == GlobalReqType::getReqCmdType())
			{
				requestCmd = new A4FileStateReq(contentName, content);
			}
			else if(GlobalReqType::GB_A5_REQ == GlobalReqType::getReqCmdType())
			{
				requestCmd = new A5StreamStateReq(contentName, content);
			}

			std::string respStatus("NULL");
			GBCSCmdPtr gardGBmemory(requestCmd);
			errorCode = gbCSReq.sendRequest(requestCmd);
			std::map<std::string, std::string> responseMap = gbCSReq.getStatusMsg();
			std::map<std::string, std::string>::iterator it = responseMap.find("parseHttpResponse");
			if (it != responseMap.end())
				respStatus = it->second;

			if (errorCode < 200 || errorCode >= 300
				&& respStatus != GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED])
			{
				store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() mainFilePathname[%s]: content[%s] not found in volume[%s]"), mainFilePathname.c_str(), contentName.c_str(), volName.c_str());
				return false;
			}

			// step2. insert record into itVol->second.contents if succ 
			store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() mainFilePathname[%s] vol[%s] target[%s] content[%s], errorCode[%d], respStatus[%s]"), 
				mainFilePathname.c_str(), volName.c_str(), targetVolName.c_str(), contentName.c_str(), errorCode, respStatus.c_str());
			
			GBCSStorePortal::ContentInfo addCI;
			addCI.name = contentName;
			itVol->second.contents.insert(GBCSStorePortal::ContentMap::value_type(addCI.name, addCI));

			itCont = itVol->second.contents.find(contentName);
			if (itVol->second.contents.end() == itCont)
			{
				store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() mainFilePathname[%s]: content[%s] not found in volume[%s] after add this to map"), mainFilePathname.c_str(), contentName.c_str(), volName.c_str());
				return false;
			}
		}

		ci = itCont->second;
	}
		
	store._log(ZQ::common::Log::L_DEBUG,CLOGFMT(GBCSStorePortal, "populateAttrsFromFile() updating content metadata"));

	char buf[256];
	if (ci.contentSize >0) // METADATA_FileSize
	{
		sprintf(buf, FMT64, ci.contentSize*1024);
		MAPSET(::TianShanIce::Properties, content.metaData, METADATA_FileSize, buf);
	}

	if (ci.supportFileSize >0) // METADATA_SupportFileSize
	{
		sprintf(buf, FMT64, ci.supportFileSize*1024);
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
		sprintf(buf, FMT64, ci.contentSize * 1024 * 8 * 1000/ ci.bitRate);
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
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "submitProvision() illegal portal context: NULL"));
		return NULL;
	}

	if (startTimeUTC.empty() || stopTimeUTC.empty())
	{
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(ContentStore, TianShanIce::Storage::csexpInvalidTime, "submitProvision() empty startTimeUTC or stopTimeUTC"));
		return NULL;
	}

	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);
	if (NULL == pPortalCtx)
	{
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "provision() reinterpret_cast store ctxPortal[NULL], "));
		return NULL;
	}

	size_t pos = contentName.find("_");
	if (std::string::npos == pos)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "provision() illegal contentname[%s] to chop for PID PAID"), contentName.c_str());

	//source URL parse
	int maxTransBT = maxTransferBitrate;
	int errorCode = -1;
	std::string strS = sourceUrl;
	std::string proto;
	size_t index = strS.find("@",0);
	size_t sBI   = strS.find("//");
	size_t sEI   = strS.find(":", sBI);

	if ( (index != std::string::npos)
		&&(sBI == std::string::npos || sEI == std::string::npos || sEI > index))
	{
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(ContentStoreImpl, TianShanIce::Storage::csexpInvalidSourceURL, "sourceURL '%s' invalidate"),sourceUrl.c_str());
		return NULL;
	}
	
	if (sBI != std::string::npos)
	{
		proto = strS.substr(0, sBI+2);
		std::transform(proto.begin(), proto.end(), proto.begin(), (int(*)(int)) tolower);
	}

	if (maxTransferBitrate <= 0)// TODO: make default value
		maxTransBT = pGBCSBaseConfig->_videoServer.vols[0].defaultBitRate; 

	GBCSReq  gbCSReq(&(store._log), pPortalCtx->getA3Url());
	IGBCSCmd * reqCmd = NULL;
	if (proto != "udp://")   //a4 cmd default
	{
		reqCmd = new A4FilePropagationReq(sourceUrl, contentName, content, pPortalCtx, maxTransBT);
		GlobalReqType::setReqCmdType(GlobalReqType::GB_A4_REQ);
	}	
	else   //a5 cmd
	{
		std::string responseAddr = pPortalCtx->getResponseAddr();
		reqCmd = new A5StreamIngestReq(sourceUrl, contentName, content, store, maxTransBT, sourceType, startTimeUTC, stopTimeUTC, responseAddr);
		GlobalReqType::setReqCmdType(GlobalReqType::GB_A5_REQ);
	}
	
	errorCode = gbCSReq.sendRequest(reqCmd);
	GBCSCmdPtr memoryGaurd(reqCmd);
	std::string respStatus("NULL");
	std::map<std::string, std::string> responseMap = gbCSReq.getStatusMsg();
	std::map<std::string, std::string>::iterator it = responseMap.find("parseHttpResponse");
	if (it != responseMap.end())
		respStatus = it->second;

	if (errorCode >=200 && errorCode< 300 && respStatus == GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED]) //ok
	{
		store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(GBCSStorePortal, 
			"submitProvision() contentName[%s] startTimeUTC[%s] stopTimeUTC[%s] maxTransBT[%u] sourceURL[%s], provisioning submitted [%s]"), 
			contentName.c_str(), startTimeUTC.c_str(), stopTimeUTC.c_str(), maxTransBT, sourceUrl.c_str(), respStatus.c_str());
		return NULL;
	}

	ZQTianShan::_IceThrow<TianShanIce::ServerError>(store._log, EXPFMT(ContentStoreImpl, errorCode, "content[%s] getExportURL failed: %s"), contentName.c_str(), respStatus.c_str());
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
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "getExportURL() illegal portal context: NULL"));
		return url;
	}

	std::string contentName = contentkey.content;
	size_t pos = contentName.find("_");
	if (std::string::npos == pos)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "getExportURL() illegal contentname[%s] to chop for PID PAID"), contentName.c_str());

	ZQTianShan::_IceThrow<TianShanIce::ServerError>(store._log, EXPFMT(ContentStoreImpl, TianShanIce::Storage::csexpUnsupportProto, "content[%s] getExportURL failed; NGB A4 and A5 not implement yet"), contentName.c_str());
	return url; // dummy statement
}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
//		throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	if (NULL == store._ctxPortal)
	{
		store._log(ZQ::common::Log::L_ERROR, CLOGFMT(GBCSStorePortal, "cancelProvision() illegal portal context: NULL"));
		return;
	}

	GBCSStorePortal* pPortalCtx = reinterpret_cast<GBCSStorePortal*>(store._ctxPortal);

	std::string contentName = content._name(); //content.getName(Ice::Current());;
	size_t pos = contentName.find("_");
	if (std::string::npos == pos)
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (store._log, EXPFMT(Content, 1005, "cancelProvision() illegal contentname[%s] to chop for PID PAID"), contentName.c_str());


	GBCSReq  gbCSReq(&(store._log), pPortalCtx->getA3Url());
	int errorCode = 0;
	IGBCSCmd * reqCmd = NULL;
	if (GlobalReqType::GB_A4_REQ == GlobalReqType::getReqCmdType())
		reqCmd = new A4FilePropagationCancel(contentName, content);
	else if(GlobalReqType::GB_A5_REQ == GlobalReqType::getReqCmdType())
		reqCmd = new A5StreamIngestCancel(contentName, content); 

	GBCSCmdPtr memoryGaurd(reqCmd);
	errorCode = gbCSReq.sendRequest(reqCmd);
	std::map<std::string, std::string> responseMap = gbCSReq.getStatusMsg();
	std::string respStatus = responseMap["parseHttpResponse"];
	if (errorCode >=200 && errorCode < 300 && respStatus == GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED]) //ok
	{
		store._log(ZQ::common::Log::L_INFO, CLOGFMT(GBCSStorePortal, "cancelProvision() content[%s] status: %s"), contentName.c_str(), respStatus.c_str());
		return;
	}

	if (errorCode == 404) //network interrupt
		errorCode = TianShanIce::Storage::csexpContentNotFound;
	else
		errorCode = TianShanIce::Storage::csexpInternalError;

	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(store._log, EXPFMT(ContentStoreImpl, errorCode, "content[%s] cancelProvision failed: %s"), contentName.c_str(), respStatus.c_str());
}

void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{

}

}
}
