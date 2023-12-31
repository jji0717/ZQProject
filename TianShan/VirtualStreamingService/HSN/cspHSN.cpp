#include "cspHSN.h"
#include "TMVSSCfgLoader.h"
#include <TimeUtil.h>
#include <sstream>

extern ZQ::common::Config::Loader< ::ZQTianShan::VSS::TM::TMVSSCfg >  pConfig;

namespace ZQTianShan {

namespace ContentStore {

//HSNCSCtr implement
HSNCSCtr::HSNCSCtr(ContentStoreImpl& store)
:_store(store)
,_bQuit(false)
,_pLog(&store._log)
,_hStop(NULL)
,_hBegin(NULL)
,_hHVolume(NULL)
{
}

HSNCSCtr::~HSNCSCtr()
{
	quit();
}

void HSNCSCtr::quit()
{
	_bQuit = true;
	if(_hStop != NULL)
		SetEvent(_hStop);

	if(_hStop != NULL)
	{
		CloseHandle(_hStop);
		_hStop = NULL;
	}
	if(_hBegin != NULL)
	{
		CloseHandle(_hBegin);
		_hBegin = NULL;
	}
	
	if(_hHVolume != NULL)
	{
		CloseHandle(_hHVolume);
		_hHVolume = NULL;
	}
}

bool HSNCSCtr::init(void)
{
	if(_hStop == NULL)
		_hStop = CreateEvent(NULL,false,false,NULL);
	if(_hBegin == NULL)
		_hBegin = CreateEvent(NULL,true,false,NULL);
	if(_hHVolume == NULL)
		_hHVolume = CreateEvent(NULL,false,false,NULL);

	return true;
}

int HSNCSCtr::run(void)
{	

	bool bInit = true;
	DWORD interval = 1;
		
	HANDLE hands[2] = {0};
	hands[0] = _hStop;
	hands[1] = _hHVolume;

	DWORD sTime = GetTickCount();

	while(!_bQuit)
	{
		DWORD re = WaitForMultipleObjects(2,hands,false,interval*1000);

		if(re == WAIT_OBJECT_0)//stop
		{
			CSPCISCOLOG(ZQ::common::Log::L_DEBUG,"HSNCSCtr::run() wait a stop event");
			break;
		}
		if(re == WAIT_OBJECT_0 +1)
		{
			bInit = true;
		}
	}
	
	CSPCISCOLOG(ZQ::common::Log::L_DEBUG,"HSNCSCtr::run() this thread will quit");
	return 0;
}


//class ContentStoreImpl portal implement
void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
		return;


	//config for content store information

	char chHost[256] = {0};
	gethostname(chHost,sizeof(chHost));
	store._netId = chHost;

	//store._storeType = pConfig._storeInfo.type;
	//store._streamableLength = atol(pConfig._storeInfo.streamableLength.c_str());

	//store._replicaGroupId = pConfig._storeInfo._storeReplica.groupId;
	//store._replicaId = pConfig._storeInfo._storeReplica.replicaId;
	//if(store._replicaId.length() == 0)
	//	store._replicaId = store._netId;

	//store._replicaPriority = pConfig._storeInfo._storeReplica.replicaPriority;
	//store._replicaTimeout = pConfig._storeInfo._storeReplica.timeout;
	//store._contentEvictorSize = pConfig._storeInfo._storeReplica.contentSize;
	//store._volumeEvictorSize = pConfig._storeInfo._storeReplica.volumeSize;


	HSNCSCtr* a3Ctr = new HSNCSCtr(store);
	store._ctxPortal = (void*) a3Ctr;
	if (NULL == store._ctxPortal)
		return;

	a3Ctr->start();

}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
	{
		HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
		pA3Ctr->quit();

		delete ((HSNCSCtr*) store._ctxPortal);
	}

	store._ctxPortal = NULL;

}

std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname)
{
	return pathname;
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	//if(store._ctxPortal == NULL)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
	//	return false;
	//}
	//
	//if(pathOfVolume.length() == 0)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::createPathOfVolume() the parameter pathOfVolum is NULL");
	//	return false;
	//}
	//
	//std::string strVol;
	//size_t sEnd = pathOfVolume.length() -1;
	//if(pathOfVolume[sEnd] == '\\' || pathOfVolume[sEnd] == '/')
	//	strVol = pathOfVolume.substr(0,sEnd);
	//else
	//	strVol = pathOfVolume;


	//HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
	//std::vector<HSNCSCtr::VOLCONTENT>::iterator it;
	//ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	//for(it = pA3Ctr->_volContents.begin(); it != pA3Ctr->_volContents.end(); it++)
	//{
	//	if( stricmp(it->_strVolName.c_str(),strVol.c_str()) == 0)//the volume existed
	//	{
	//		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::createPathOfVolume() the volume (%s) existed",pathOfVolume.c_str());
	//		return false;
	//	}
	//}
	//
	//HSNCSCtr::VOLCONTENT volContent;
	//volContent._strVolName = strVol;
	//pA3Ctr->_volContents.push_back(volContent);

	//SetEvent(pA3Ctr->_hHVolume);

	store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::createPathOfVolume() volume dummy add to monitor");
	return true;
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	//if(store._ctxPortal == NULL)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
	//	return false;

	//}

	//if(pathOfVolume.length() == 0)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deletePathOfVolume() the parameter pathOfVolum is NULL");
	//	return false;
	//}

	//std::string strVol = pathOfVolume;
	//size_t sL = strVol.length() -1;
	//if(strVol[sL] == FNSEPC)
	//	strVol[sL] = '\0';

	//HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
	//bool bFind = false;
	//std::vector<HSNCSCtr::VOLCONTENT>::iterator it;
	//ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	//for(it = pA3Ctr->_volContents.begin(); it != pA3Ctr->_volContents.end(); it++)
	//{
	//	if( stricmp(it->_strVolName.c_str(),strVol.c_str()) == 0)
	//	{
	//		bFind = true;
	//		pA3Ctr->_volContents.erase(it);
	//		store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::deletePathOfVolume() delete the volume (%s) to monitor ",strVol.c_str());
	//		break;
	//	}
	//}
	//if(!bFind)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deletePathOfVolume() not find volume (%s) to delete",strVol.c_str());
	//	return false;
	//}

	return true;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath)
{
	//CISCO soap content store don't support this
	freeMB = 0;
	totalMB = 0;
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store,std::string& fileName,const std::string& contentType)
{
	if(fileName.find("\n",0) != std::string::npos || fileName.find(" ",0) != std::string::npos
		|| fileName.find("\r",0) != std::string::npos || fileName.find("\t",0) != std::string::npos
//		|| fileName.find("\\",0) != std::string::npos || fileName.find("/",0) != std::string::npos
		)
		return false;
	
	//may be is a bug
	fileName = strupr((char*)fileName.c_str());
	return true;
}

uint64 ContentStoreImpl::checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname)
{
	//uint64 re = 0;

	//if(store._ctxPortal == NULL)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
	//	return 0;
	//}

	//size_t nIndex = mainFilePathname.rfind(FNSEPC);
	//if(nIndex == std::string::npos)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::checkResidentialStatus() parameter mainFilePathname (%s) not contain volume name",mainFilePathname.c_str());
	//	return 0;
	//}
	//std::string strVol = mainFilePathname.substr(0,nIndex);
	//std::string strName = mainFilePathname.substr(nIndex+1);

	//HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
	//bool bExist = false;
	//ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	//std::vector<HSNCSCtr::VOLCONTENT>::iterator itVC;
	//std::map<std::string,CiscoAIMContentInfo::Ptr>::iterator it;
	//for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
	//{
	//	if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
	//	{			
	//		it = itVC->_contentsM.find(strName);
	//		if(it != itVC->_contentsM.end())
	//			bExist = true;
	//		else
	//			bExist = false;	
	//		break;
	//	}
	//}
	////content exist?
	//if((flagsToTest & RSDFLAG(frfResidential)))
	//{
	//	if(bExist)
	//		re |= RSDFLAG(frfResidential);
	//}

	//if(!re)
	//	return 0;

	//if((flagsToTest & RSDFLAG(frfWriting)))
	//{
	//	if(bExist && it->second != NULL)
	//	{
	//		if((it->second->contentState == TRANSFER) || (it->second->contentState == STREAMABLE))
	//			re |= RSDFLAG(frfWriting);
	//	}
	//}

	//if((flagsToTest & RSDFLAG(frfAbsence)))
	//{
	//	if(bExist && it->second != NULL)
	//	{
	//		if((it->second->contentState == CANCELED) || (it->second->contentState == FAILED) || (it->second->contentState == PENDING))//pending is?
	//			re |= RSDFLAG(frfAbsence);
	//	}
	//}
	//
	//if((flagsToTest & RSDFLAG(frfCorrupt)))
	//{
	//	if(bExist && it->second != NULL)
	//	{
	//		if((it->second->contentState == CANCELED) || (it->second->contentState == FAILED))
	//			re |= RSDFLAG(frfCorrupt);
	//	}
	//}

	//if(it->second != NULL)
	//	it->second =NULL;

	//return (re & flagsToTest);
	return 1;
}

bool ContentStoreImpl::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	//if(store._ctxPortal == NULL)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
	//	return false;
	//}

	//size_t nIndex = mainFilePathname.rfind(FNSEPC);
	//if(nIndex == std::string::npos)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() parameter mainFilePathname not contain volume name");
	//	return false;
	//}
	//std::string strVol = mainFilePathname.substr(0,nIndex);
	//std::string strName = mainFilePathname.substr(nIndex+1);

	//HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
	//size_t  sI = strName.find("_");
	//if(sI == std::string::npos)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() the content name '%s' is invalidate",strName.c_str());
	//	return false;
	//}

	//_CISCO1__DeletePackage inStruct;
	//inStruct.PackageName = &strName;
	//_CISCO1__DeletePackageResponse outStruct;
	//pA3Ctr->_ciscoAIMSoap11Impl->DeletePackage(inStruct, outStruct);

	//ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	//std::vector<HSNCSCtr::VOLCONTENT>::iterator itVC;
	//for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
	//{
	//	if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
	//	{
	//		std::map<std::string,CiscoAIMContentInfo::Ptr>::iterator it;
	//		it = itVC->_contentsM.find(strName);
	//		if(it != itVC->_contentsM.end())
	//		{
	//			it->second = NULL;
	//			itVC->_contentsM.erase(it);					
	//		}
	//		break;		
	//	}
	//}

	//if(outStruct.DeleteResult == NULL)//soap operation fail
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() delete content (%s) error,soap call return NULL",mainFilePathname.c_str());
	//	return false;
	//}
	//else if(outStruct.DeleteResult->compare("SUCCESS") != 0)
	//{
	//	store._log(ZQ::common::Log::L_WARNING,"ContentStoreImpl::deleteFileByContent() delete content (%s) error%s",mainFilePathname.c_str(), pA3Ctr->_ciscoAIMSoap11Impl->printSoapFault((struct SOAP_ENV__Fault *)(outStruct.DeleteResult)));
	//	return false;
	//}
	//else
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() delete content (%s) success",mainFilePathname.c_str());
	//	return true;
	//}
	return true;
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath)
{
	ContentStoreImpl::FileInfos fInfos;	

	//if(store._ctxPortal == NULL)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
	//	return fInfos;
	//}
	//if(rootPath == NULL || strlen(rootPath) == 0)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::listMainFiles() root path is NULL");
	//	return fInfos;
	//}
	//
	//std::string strVol = rootPath;
	//size_t sLen = strVol.length() -1;
	//if(strVol[sLen] == '\\' || strVol[sLen] == '/')
	//	strVol[sLen] = '\0'; 
	//
	//HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
	////wait until get the Content list
	//{
	//	bool bInitF = true;
	//	ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	//	std::vector<HSNCSCtr::VOLCONTENT>::iterator itVCS;
	//	for(itVCS = pA3Ctr->_volContents.begin(); itVCS != pA3Ctr->_volContents.end(); itVCS++)
	//	{
	//		if(itVCS->_contentsM.size() == 0)
	//			bInitF = false;
	//	}
	//	if(!bInitF)//not initialized
	//		ResetEvent(pA3Ctr->_hBegin);
	//}

	//WaitForSingleObject(pA3Ctr->_hBegin,INFINITE);


	//{
	//	SYSTEMTIME sysTime;
	//	GetSystemTime(&sysTime);
	//	char chIsoT[30] = {0};
	//	bool bR = ZQ::common::TimeUtil::Time2Iso(sysTime,chIsoT,sizeof(chIsoT),false);
	//	if(!bR)
	//	{
	//		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::listMainFiles() can not convert system time to ISO time");;
	//	}

	//	bool bFind = false;
	//	ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	//	std::vector<HSNCSCtr::VOLCONTENT>::iterator itVC;
	//	for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
	//	{
	//		if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
	//		{
	//			std::map<std::string,CiscoAIMContentInfo::Ptr>::iterator it;
 //				for(it = itVC->_contentsM.begin(); it != itVC->_contentsM.end(); it++)
	//			{
	//				ContentStoreImpl::FileInfo fInfo;
	//				fInfo.filename = it->first;
	//				fInfo.stampLastWrite = chIsoT;
	//				fInfos.push_back(fInfo);
	//			}
	//			bFind = true;
	//			break;
	//		}
	//	}
	//	if(!bFind)
	//		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::listMainFiles() not find contents of root path (%s)",rootPath);

	//}
	return fInfos;

}

std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename)
{
	if(memberFilename.length() == 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::memberFileNameToContentName() memberFilename is NULL");
		return "";
	}
	
	size_t nS = memberFilename.rfind(FNSEPS);
	if(nS == std::string::npos)
		return memberFilename;
	else
		return memberFilename.substr(nS+1);
}


bool ContentStoreImpl::completeRenaming(ContentStoreImpl& store, const ::std::string& oldName, const ::std::string& newName)
{
	return true;
}

bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname)
{
	//store._log(ZQ::common::Log::L_DEBUG,"A3::populateAttrsFromFile() here ");
	//if(store._ctxPortal == NULL)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
	//	return false;
	//}

	//if(mainFilePathname.length() == 0)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::populateAttrsFromFile() mainFilePathname is NULL");
	//	return false;
	//}

	//size_t sIndex = mainFilePathname.find(FNSEPS ,0);
	//if(sIndex == std::string::npos)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::populateAttrsFromFile() mainFilePathname (%s) is invalidate",mainFilePathname.c_str());
	//	return false;
	//}
	//std::string strVol = mainFilePathname.substr(0,sIndex);
	//std::string strName = mainFilePathname.substr(sIndex+1);
	//

	//bool bFind = false;
	//HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);

	//std::vector<HSNCSCtr::VOLCONTENT>::iterator itVC;
	//ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	//for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
	//{
	//	if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
	//	{
	//		bFind = true;
	//		break;
	//	}
	//}

	//if(!bFind)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::populateAttrsFromFile() not find the volume (%s)",strVol.c_str());
	//	return false;
	//}

	//std::map<std::string,CiscoAIMContentInfo::Ptr>::iterator it;
	//it = itVC->_contentsM.find(strName);
	//if(it == itVC->_contentsM.end())//not the content
	//{
	//	store._log(ZQ::common::Log::L_WARNING,"ContentStoreImpl::populateAttrsFromFile() not find the content (%s)",strName.c_str());
	//	return false;
	//}
	//	
	//if(it->second == NULL)//the content not attribute,do what ?
	//	return false;

	return true;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& contentName,
						const ::std::string& sourceUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	//if(store._ctxPortal == NULL)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
	//	return NULL;
	//}

	//if(startTimeUTC.empty() || stopTimeUTC.empty())
	//{
	//	ZQTianShan::_IceThrow <::TianShanIce::InvalidParameter> (store._log, EXPFMT(ContentStoreImpl, TianShanIce::Storage::csexpInvalidTime, "submitProvision() empty startTimeUTC or stopTimeUTC"));
	//	return NULL;
	//}

	//HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
	//std::string strName = contentName;
	//size_t index = strName.find("_");
	//if(index == std::string::npos)
	//{
	//	store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::submitProvision() the content name '%s' is invalidate",strName.c_str());
	//	return NULL;
	//}

	//_CISCO1__IngestPackage inStruct;
	//inStruct.ADIURL = new ::std::string(sourceUrl);
	//inStruct.PackageName = &strName;
	//inStruct.MetaDataOnly = 0;
	//inStruct.DoAsync = 0; 

	//_CISCO1__IngestPackageResponse outStruct;
	//outStruct.IngestResult;
	//pA3Ctr->_ciscoAIMSoap11Impl->IngestPackage(inStruct, outStruct);
	//delete inStruct.ADIURL;

	//if(outStruct.IngestResult->compare("SUCCESS") != 0)
	//{
	//	ZQTianShan::_IceThrow<TianShanIce::ServerError>(store._log,EXPFMT(ContentStoreImpl, 400, "failed to provision '%s'"), strName.c_str());
	//}

	//store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::submitProvision()  content(name:%s) operation success",strName.c_str());

	return NULL;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
						::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	return NULL;
}

std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content, const std::string& contentName, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return "";
	}

	HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);

	return "";

}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return;
	}

	HSNCSCtr* pA3Ctr = reinterpret_cast<HSNCSCtr*>(store._ctxPortal);
}

void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{
}

}
}
