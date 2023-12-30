#include "cspA3.h"

#include "NSSConfig.h"
#include "NSSCfgLoader.h"
#include "urlStr.h"
#include <TimeUtil.h>
#include "ContentUser.h"

//extern char *DefaultConfigPath;
//extern ::ZQ::common::Config::Loader< ::ZQTianShan::NSS::NSSCfg > pConfig;
extern ::ZQTianShan::NSS::NSSBaseConfig::NSSHolder *pNSSBaseConfig;

namespace ZQTianShan {

namespace ContentStore {

//A3ResponseMsg implement
A3ResponseMsg::A3ResponseMsg(const char* localIP, const int& port, ZQ::common::Log* pLog)
:SimpleServer(localIP,port,pLog),_pCtr(NULL),_pLog(pLog)
{
}

A3ResponseMsg::~A3ResponseMsg(void)
{

}

void A3ResponseMsg::setA3Handle(A3CSCtr* pCtr)
{
	_pCtr = pCtr;
}

//handle web message
int A3ResponseMsg::handleMsg(const std::string& type, const std::string& msg)
{
	assert(_pCtr != NULL);

	if(msg.empty()) 
	{
		CSPA3LOG(ZQ::common::Log::L_ERROR,"A3ResponseMsg::handleMsg() message is NULL");
		return MSG_INTERNAL_ERR;
	}
	
	std::string strVol;
	std::string strName;
	bool fileModified = false;	

	// TransferStatus
	// modify by lxm
	std::string strType;
	if (type.find(":") != std::string::npos)
	{
		::ZQ::common::URLStr urlStr(type.c_str());
		strType = urlStr.getPath();
	}
	else
		strType = type; 
	
	//if(stricmp(type.c_str(),TYPE_TRANSFER) == 0)
	if(stricmp(TYPE_TRANSFER, strType.c_str()) == 0)
	{	
		TransferStatus status;
		if(!_oprtXml.parseGetTransferStatus(status, msg.c_str(), static_cast<int>(msg.size()))) 
		{
			CSPA3LOG(ZQ::common::Log::L_ERROR,"A3ResponseMsg::handleMsg() parse TransferStatus message failed");
			return MSG_INTERNAL_ERR;
		}
		//content name
		strName = status.assetID;
		strName += "_";
		strName += status.providerID;
		
		//volume name
		strVol = status.volumeName;
		bool bFind = false;
		std::vector<A3CSCtr::VOLCONTENT>::iterator itVC;
		ZQ::common::MutexGuard mg(_pCtr->_lock);
		for(itVC = _pCtr->_volContents.begin(); itVC != _pCtr->_volContents.end(); itVC++)
		{
			if(stricmp(strVol.c_str(), itVC->_strVolName.c_str()) == 0)
			{
				bFind = true;
				break;
			}
		}
		if(!bFind)//not find the volume
		{
			CSPA3LOG(ZQ::common::Log::L_ERROR,"A3ResponseMsg::handleMsg() not find the volume name '%s'",strVol.c_str());
			return MSG_UNKNOWN_PAID;
		}


		std::map<std::string, A3ContentInfo::Ptr>::iterator it;		
		it = itVC->_contentsM.find(strName);
		if(it == itVC->_contentsM.end())//not the content,need add it?
		{
			itVC->_contentsM.insert(std::pair<std::string,A3ContentInfo::Ptr>(strName,NULL));
		}

		it = itVC->_contentsM.find(strName);
		if(it->second == NULL)
			it->second = new A3ContentInfo();

		if(status.state == (PENDING)) 
		{
			CSPA3LOG(ZQ::common::Log::L_INFO, "A3ResponseMsg::handleMsg() Content (%s -> %s) pending", 
					status.volumeName.c_str(), strName.c_str());

			it->second->contentState = status.state;
		}
		else if(status.state == (TRANSFER)) 
		{
			CSPA3LOG(ZQ::common::Log::L_INFO, "A3ResponseMsg::handleMsg() Content (%s -> %s) processed (%d%%)", 
				status.volumeName.c_str(), strName.c_str(), status.percentComplete);

			it->second->contentState = status.state;
		}
		else if(status.state == (STREAMABLE)) 
		{
			CSPA3LOG(ZQ::common::Log::L_INFO, "A3ResponseMsg::handleMsg() Content (%s -> %s) processed (%d%%)", 
					status.volumeName.c_str(), strName.c_str(), status.percentComplete);

			it->second->contentState = status.state;
			if(status.bitrate) 
				it->second->bitRate = status.bitrate;
		}
		else if(status.state == (COMPLETE)) 
		{
			CSPA3LOG(ZQ::common::Log::L_INFO, "A3ResponseMsg::handleMsg() Content (%s -> %s) complete", 
				status.volumeName.c_str(), strName.c_str());

			it->second->contentState = status.state;

			if(status.bitrate) 
				it->second->bitRate = status.bitrate;

			it->second->contentSize = status.contentSize;
			it->second->supportFileSize = status.supportFileSize;
			it->second->md5Checksum = status.md5Checksum;
			it->second->md5DateTime = status.md5DateTime;
		}
		else if(status.state == (FAILED)) 
		{
			CSPA3LOG(ZQ::common::Log::L_INFO, "A3ResponseMsg::handleMsg() Content (%s -> %s) failed,reasoncode (%d)", 
				status.volumeName.c_str(), strName.c_str(),status.reasonCode);

//			int code = status.reasonCode == 401 ?
//					   csexpUnauthorized : status.reasonCode == 409 ?
//					   csexpNoResource   : csexpInternalError;

			//delete the content
			it->second = NULL;
			itVC->_contentsM.erase(it);
			::TianShanIce::Properties params;
			params["sys.LastError"] = "400";
			params["sys.LastErrMsg"] = "server failed";

			_pCtr->_store.OnFileEvent(::TianShanIce::Storage::fseFileDeleted, strVol+"\\"+strName, params, ::Ice::Current());
			return MSG_OK;

		}
		else if(status.state == (CANCELED)) 
		{
			CSPA3LOG(ZQ::common::Log::L_INFO, "A3ResponseMsg::handleMsg() Content (%s -> %s) canceled", 
				status.volumeName.c_str(), strName.c_str());

			//delete the content?
//			it->second->contentState = status.state;
			it->second = NULL;
			itVC->_contentsM.erase(it);
			::TianShanIce::Properties params;
			_pCtr->_store.OnFileEvent(::TianShanIce::Storage::fseFileDeleted, strVol+"\\"+strName, params, ::Ice::Current());
			return MSG_OK;

		}
		else //unknown the state
		{
			CSPA3LOG(ZQ::common::Log::L_ERROR, "A3ResponseMsg::handleMsg() Content (%s -> %s) unknown status(%s)", 
				status.volumeName.c_str(), strName.c_str(),status.state.c_str());
			return MSG_OK;

		}

		fileModified =true;
	
	}
	// ContentChecksum
	//else if(stricmp(type.c_str(),TYPE_CHECKSUM) == 0)
	else if(stricmp(TYPE_CHECKSUM, strType.c_str()) == 0)
	{
		CSPA3LOG(ZQ::common::Log::L_DEBUG,"A3ResponseMsg::handleMsg() Have a %s msg",TYPE_CHECKSUM);
		ContentChecksum checkSum;
		if(!_oprtXml.parseContentChecksum(checkSum, msg.c_str(), static_cast<int>(msg.size()))) 
		{
			CSPA3LOG(ZQ::common::Log::L_ERROR,"A3ResponseMsg::handleMsg() parse ContentChecksum message failed");
			return MSG_INTERNAL_ERR;
		}
		strName = checkSum.assetID;
		strName += "_";
		strName += checkSum.providerID;

		//volume name
		strVol = checkSum.volumeName;
		bool bFind = false;
		std::vector<A3CSCtr::VOLCONTENT>::iterator itVC;
		ZQ::common::MutexGuard mg(_pCtr->_lock);
		for(itVC = _pCtr->_volContents.begin(); itVC != _pCtr->_volContents.end(); itVC++)
		{
			if(stricmp(strVol.c_str(), itVC->_strVolName.c_str()) == 0)
			{
				bFind = true;
				break;
			}
		}
		if(!bFind)//not find the volume
		{
			CSPA3LOG(ZQ::common::Log::L_ERROR,"A3ResponseMsg::handleMsg() not find the volume name '%s'",strVol.c_str());
			return MSG_UNKNOWN_PAID;
		}

		std::map<std::string, A3ContentInfo::Ptr>::iterator it;		
		it = itVC->_contentsM.find(strName);
		if(it == itVC->_contentsM.end())//not find the content
		{
			CSPA3LOG(ZQ::common::Log::L_ERROR,"A3ResponseMsg::handleMsg() not find content '%s -> %s'",strVol.c_str(),strName.c_str());
			return MSG_UNKNOWN_PAID;
		}

		if(it->second == NULL)
			it->second = new A3ContentInfo();
		
		if(checkSum.md5Checksum.length() > 0)
			it->second->md5Checksum = checkSum.md5Checksum;
		if(checkSum.md5DateTime.length() > 0)
			it->second->md5DateTime = checkSum.md5DateTime;

		fileModified =true;
	}
	else
	{
		CSPA3LOG(ZQ::common::Log::L_ERROR,"A3ResponseMsg::handleMsg() do not know the message type: %s",type.c_str());
		return MSG_INTERNAL_ERR;
	}
	
	if (fileModified)
	{
		::TianShanIce::Properties params;
		_pCtr->_store.OnFileEvent(::TianShanIce::Storage::fseFileModified, strVol+"\\"+strName, params, ::Ice::Current());
	}

	return MSG_OK;
}

//A3CSCtr implement
A3CSCtr::A3CSCtr(ContentStoreImpl& store)
:_store(store),_bQuit(false),_port(0),_pLog(&store._log),_hStop(NULL),_hBegin(NULL),_hHVolume(NULL),_pWebMsg(NULL),_bBegin(false)
{
	_strIP = pNSSBaseConfig->_videoServer.FeedbackIp;
	_port = pNSSBaseConfig->_videoServer.FeedbackPort;

	::std::string strRemoteHost;
	::std::stringstream ss;
	ss << pNSSBaseConfig->_videoServer.ContentInterfacePort;
	strRemoteHost = pNSSBaseConfig->_videoServer.ContentInterfaceIp + ":" + ss.str();

	_strHost = strRemoteHost + "/" + pNSSBaseConfig->_videoServer.ContentInterfacePath;

}

A3CSCtr::~A3CSCtr()
{
	quit();
}

void A3CSCtr::quit()
{
	_bQuit = true;
	if(_hStop != NULL)
		SetEvent(_hStop);

	if(_pWebMsg != NULL)
	{
		_pWebMsg->close();
		delete _pWebMsg;
		_pWebMsg = NULL;
	}

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

bool A3CSCtr::init(void)
{

	if(_pWebMsg == NULL)
		_pWebMsg = new A3ResponseMsg(_strIP.c_str(),_port,_pLog);
	if(_pWebMsg == NULL)
		return false;
	_pWebMsg->setA3Handle(this);

	if(_hStop == NULL)
		_hStop = CreateEvent(NULL,false,false,NULL);
	if(_hBegin == NULL)
		_hBegin = CreateEvent(NULL,true,false,NULL);
	if(_hHVolume == NULL)
		_hHVolume = CreateEvent(NULL,false,false,NULL);


	_pWebMsg->start();

	return true;
}

int A3CSCtr::run(void)
{	

	bool bInit = true;
	DWORD interval = pNSSBaseConfig->_videoServer.ContentInterfaceSyncInterval;
		
	HANDLE hands[2] = {0};
	hands[0] = _hStop;
	hands[1] = _hHVolume;

	while(!_bQuit)
	{
		{
		bool bFinish = true;
		ZQ::common::MutexGuard mg(_lock);
		std::vector<VOLCONTENT>::iterator itVC;
		for(itVC = _volContents.begin(); itVC != _volContents.end(); itVC++)
		{
			if(bInit && itVC->_contentsM.size()>0)//this volume content have initialized
				continue;

			VolumeInfo vinfo;
			vinfo.volumeName = itVC->_strVolName;
			CSPA3LOG(ZQ::common::Log::L_DEBUG,"A3CSCtr::run() begin to sync volume(%s) content", vinfo.volumeName.c_str());

			A3Request reqVI(_strHost,_pLog);
			if(reqVI.GetVolumeInfo(vinfo) == 200 /*&& vinfo.state == OPERATIONAL*/)
			{

				ContentInfo cInfo;
				cInfo.volumeName = vinfo.volumeName;

				std::vector<ContentInfo> contents;
				A3Request reqConts(_strHost,_pLog);
				if(reqConts.GetContentInfo(cInfo, contents) != 200)
				{
					CSPA3LOG(ZQ::common::Log::L_ERROR, "failed to get volume '%s' content info,response message (%s)",vinfo.volumeName.c_str(), reqConts.getStatusMessage().c_str());
					interval = 5*1000;
					bFinish = false;
					break;
				}
				//initialize fill contents map
				if(bInit)
				{					
					itVC->_contentsM.clear();
					std::vector<ContentInfo>::iterator it;
					std::string strName;
					for(it = contents.begin(); it < contents.end(); it++)
					{
						strName = (char*)(it->assetID.c_str());
						strName += "_";
						strName += (char*)(it->providerID.c_str());

						A3ContentInfo::Ptr ptr = new A3ContentInfo();
						ptr->contentSize = it->contentSize;
						ptr->supportFileSize = it->supportFileSize;
						ptr->createDate = it->createDate;
						ptr->md5Checksum = it->md5Checksum;
						ptr->md5DateTime = it->md5DateTime;
						ptr->contentState = it->contentState;
					
						itVC->_contentsM.insert(std::pair<std::string,A3ContentInfo::Ptr>(strName,ptr));
						
					}

				}
				else//update the content list(_contentsM)
				{
					
					std::map<std::string,A3ContentInfo::Ptr>::iterator itM;
					std::vector<ContentInfo>::iterator itV;
					std::string strName;
					bool bFind = false;
 					for(itM = itVC->_contentsM.begin(); itM != itVC->_contentsM.end(); itM++)
					{
						bFind = false;
						for(itV = contents.begin(); itV < contents.end(); itV++)
						{
							strName = itV->assetID + "_" + itV->providerID;
							if(stricmp(itM->first.c_str(),strName.c_str()) == 0)
							{
								bFind = true;
								contents.erase(itV);
								break;
							}							
						}
						//if not find the remove it and send event
						if(!bFind)
						{
							std::map<std::string,A3ContentInfo::Ptr>::iterator itNew = itM;
							if(itNew == itVC->_contentsM.begin())
								itNew = itVC->_contentsM.end();
							else
								itNew--;
							strName = itM->first;
							itM->second = NULL;
							itVC->_contentsM.erase(itM);
							
							if(itNew == itVC->_contentsM.end())
								itM = itVC->_contentsM.begin();
							else
								itM = itNew;
								
							CSPA3LOG(ZQ::common::Log::L_INFO, "volume(%s) content(%s) not find in content store, just delete", itVC->_strVolName.c_str(), strName.c_str());
							::TianShanIce::Properties params;
							_store.OnFileEvent(::TianShanIce::Storage::fseFileDeleted,itVC->_strVolName+"\\"+strName, params, ::Ice::Current());
						}
						//add for check status
						else//find and check the status
						{
							CSPA3LOG(ZQ::common::Log::L_DEBUG, "content(%s) state(%s) set to auto check", strName.c_str(), itM->second->contentState.c_str());
							if(itM->second == NULL)
							{
								//send GetTransferStatus
								TransferStatus transInfo;
								transInfo.assetID = itV->assetID;
								transInfo.providerID = itV->providerID;
								transInfo.volumeName = itVC->_strVolName;

								//A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(_store._ctxPortal);
								A3Request reqTC(getHostURL(), _pLog);
								int state = reqTC.GetTransferStatus(transInfo);
								if(state == 200)//ok
								{
									if (transInfo.state == COMPLETE || transInfo.state == STREAMABLE)
									{
										CSPA3LOG(ZQ::common::Log::L_INFO, "A3ResponseMsg::run() Content (%s -> %s) complete", itVC->_strVolName.c_str(), strName.c_str());
										itM->second = new A3ContentInfo();

										itM->second->contentSize		= transInfo.contentSize;
										itM->second->supportFileSize	= transInfo.supportFileSize;
										itM->second->md5Checksum		= transInfo.md5Checksum;
										itM->second->md5DateTime		= transInfo.md5DateTime;
										if (transInfo.bitrate)
											itM->second->bitRate		= transInfo.bitrate;
									}
								}
							}
						}
						/////////////////////
					}
					//have contents created
					for(itV = contents.begin(); itV < contents.end(); itV++)
					{
						strName = (char*)(itV->assetID.c_str());
						strName += "_";
						strName += (char*)(itV->providerID.c_str());
						
						A3ContentInfo::Ptr ptr = new A3ContentInfo();
						ptr->contentSize = itV->contentSize;
						ptr->supportFileSize = itV->supportFileSize;
						ptr->createDate = itV->createDate;
						ptr->md5Checksum = itV->md5Checksum;
						ptr->md5DateTime = itV->md5DateTime;
						ptr->contentState = itV->contentState;

						itVC->_contentsM.insert(std::pair<std::string,A3ContentInfo::Ptr>(strName,ptr));

						::TianShanIce::Properties params;
						_store.OnFileEvent(::TianShanIce::Storage::fseFileCreated, itVC->_strVolName+"\\"+strName, params, ::Ice::Current());
					}
				}
			}
			else
			{
				CSPA3LOG(ZQ::common::Log::L_ERROR, "failed to get volume (%s) info,response message (%s)",vinfo.volumeName.c_str(), reqVI.getStatusMessage().c_str());
				interval = 5*1000;
				bFinish = false;
				break;
			}
			

		}

		if(bFinish)//all volume have initialized
		{
			if(bInit)
			{
				SetEvent(_hBegin);
				_bBegin = true;
			}

			bInit = false;
			interval = pNSSBaseConfig->_videoServer.ContentInterfaceSyncInterval;
		}
		}//lock area

		DWORD re = WaitForMultipleObjects(2,hands,false,interval);
		if(re == WAIT_OBJECT_0)//stop
		{
			CSPA3LOG(ZQ::common::Log::L_DEBUG,"A3CSCtr::run() wait a stop event");
			break;
		}
		if(re == WAIT_OBJECT_0 +1)
		{
			bInit = true;
		}
	}
	
	CSPA3LOG(ZQ::common::Log::L_DEBUG,"A3CSCtr::run() this thread will quit");
	return 0;
}


//class ContentStoreImpl portal implement
void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
		return;

	//config for contentstore information
	store._netId = pNSSBaseConfig->netId;
	if(store._netId.length() == 0)
	{
		char chHost[256] = {0};
		gethostname(chHost,sizeof(chHost));
		store._netId = chHost;
	}
	store._storeType = "A3CS";
	store._streamableLength = 100000;

	store._replicaGroupId = pNSSBaseConfig->_videoServer.StoreReplicaGroupId;
	store._replicaId = pNSSBaseConfig->_videoServer.StoreReplicaReplicaId;
	if(store._replicaId.length() == 0)
		store._replicaId = store._netId;

	store._replicaPriority = pNSSBaseConfig->_videoServer.StoreReplicaReplicaPriority;
	store._replicaTimeout = pNSSBaseConfig->_videoServer.StoreReplicaTimeout;
	store._contentEvictorSize = pNSSBaseConfig->_videoServer.DatabaseCacheContentSize;
	store._volumeEvictorSize = pNSSBaseConfig->_videoServer.DatabaseCacheVolumeSize;

	A3CSCtr* a3Ctr = new A3CSCtr(store);
	store._ctxPortal = (void*) a3Ctr;
	if (NULL == store._ctxPortal)
		return;

	a3Ctr->start();

}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store)
{
	if (NULL != store._ctxPortal)
	{
		A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
		pA3Ctr->quit();

		delete ((A3CSCtr*) store._ctxPortal);
	}

	store._ctxPortal = NULL;

}

std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname)
{
	return pathname;
/*
	if(pathname.length() == 0)
		return "";
	
	size_t nE = pathname.length()-1;
	if(pathname[nE] == '\\' || pathname[nE] == '/')
		return pathname;
	else
		return pathname + "\\";
*/
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return false;
	}
	
	if(pathOfVolume.length() == 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::createPathOfVolume() the parameter pathOfVolum is NULL");
		return false;
	}
	
	std::string strVol;
	size_t sEnd = pathOfVolume.length() -1;
	if(pathOfVolume[sEnd] == '\\' || pathOfVolume[sEnd] == '/')
		strVol = pathOfVolume.substr(0,sEnd);
	else
		strVol = pathOfVolume;


	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	std::vector<A3CSCtr::VOLCONTENT>::iterator it;
	ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	for(it = pA3Ctr->_volContents.begin(); it != pA3Ctr->_volContents.end(); it++)
	{
		if( stricmp(it->_strVolName.c_str(),strVol.c_str()) == 0)//the volume existed
		{
			store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::createPathOfVolume() the volume (%s) existed",pathOfVolume.c_str());
			return false;
		}
	}
	
	A3CSCtr::VOLCONTENT volContent;
	volContent._strVolName = strVol;
	pA3Ctr->_volContents.push_back(volContent);

	SetEvent(pA3Ctr->_hHVolume);

	store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::createPathOfVolume() volume (%s) add to monitor",pathOfVolume.c_str());
	return true;
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return false;

	}

	if(pathOfVolume.length() == 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deletePathOfVolume() the parameter pathOfVolum is NULL");
		return false;
	}

	std::string strVol = pathOfVolume;
	size_t sL = strVol.length() -1;
	if(strVol[sL] == FNSEPC)
		strVol[sL] = '\0';

	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	bool bFind = false;
	std::vector<A3CSCtr::VOLCONTENT>::iterator it;
	ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	for(it = pA3Ctr->_volContents.begin(); it != pA3Ctr->_volContents.end(); it++)
	{
		if( stricmp(it->_strVolName.c_str(),strVol.c_str()) == 0)
		{
			bFind = true;
			pA3Ctr->_volContents.erase(it);
			store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::deletePathOfVolume() delete the volume (%s) to monitor ",strVol.c_str());
			break;
		}
	}
	if(!bFind)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deletePathOfVolume() not find volume (%s) to delete",strVol.c_str());
		return false;
	}

	return true;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		freeMB = 0;
		totalMB = 0;
		return;
	}

	if(rootPath == NULL || strlen(rootPath) == 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::getStorageSpace() toop path is NULL");
		freeMB = 0;
		totalMB = 0;
		return;
	}

	VolumeInfo vinfo;
	vinfo.volumeName = rootPath;
	
	size_t sEnd = vinfo.volumeName.length()-1;
	if(vinfo.volumeName[sEnd] == FNSEPC)
		vinfo.volumeName[sEnd] = '\0';
		
	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	A3Request reqVI(pA3Ctr->getHostURL(),&(store._log));
	int state = reqVI.GetVolumeInfo(vinfo);
	if((state == 200) && vinfo.state == OPERATIONAL)
	{
		freeMB = vinfo.freeSize;
		totalMB = vinfo.volumeSize;
	}
	else
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::getStorageSpace() request error,%s",reqVI.getStatusMessage().c_str());
		freeMB = 0;
		totalMB = 0;
	}
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store,std::string& fileName,const std::string& contentType)
{
	if(fileName.find("\n",0) != std::string::npos || fileName.find(" ",0) != std::string::npos
		|| fileName.find("\r",0) != std::string::npos || fileName.find("\t",0) != std::string::npos
//		|| fileName.find("\\",0) != std::string::npos || fileName.find("/",0) != std::string::npos
		)
		return false;
	
	//may be is a bug
	fileName = (char*)fileName.c_str();
	return true;
}

/*
bool ContentStoreImpl::fileExists(ContentStoreImpl& store, const std::string& mainFilePathname)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return false;
	}

	if(mainFilePathname.length() == 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::fileExists() content name is NULL");
		return false;
	}


	size_t nIndex = mainFilePathname.rfind(FNSEPS);
	if(nIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::fileExists() content name (%s) not contain volume name",mainFilePathname.c_str());
		return false;
	}

	std::string strVol = mainFilePathname.substr(0,nIndex);
	std::string strContent = mainFilePathname.substr(nIndex+1);

	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	
	ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	std::vector<A3CSCtr::VOLCONTENT>::iterator itVC;
	for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
	{
		if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
		{
			std::map<std::string,A3ContentInfo::Ptr>::iterator it;
			it = itVC->_contentsM.find(strContent);
			if(it != itVC->_contentsM.end())
				return true;
			else
				return false;			
		}
	}	

	return false;
}

bool ContentStoreImpl::isContentInUse(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	return false;
}
*/
uint64 ContentStoreImpl::checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname)
{
	uint64 re = 0;

	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return 0;
	}

	size_t nIndex = mainFilePathname.rfind(FNSEPC);
	if(nIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::checkResidentialStatus() parameter mainFilePathname (%s) not contain volume name",mainFilePathname.c_str());
		return 0;
	}
	std::string strVol = mainFilePathname.substr(0,nIndex);
	std::string strName = mainFilePathname.substr(nIndex+1);

	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	bool bExist = false;
	ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	std::vector<A3CSCtr::VOLCONTENT>::iterator itVC;
	std::map<std::string,A3ContentInfo::Ptr>::iterator it;
	for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
	{
		if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
		{			
			it = itVC->_contentsM.find(strName);
			if(it != itVC->_contentsM.end())
				bExist = true;
			else
				bExist = false;	
			break;
		}
	}
	//content exist?
	if((flagsToTest & RSDFLAG(frfResidential)))
	{
		if(bExist)
			re |= RSDFLAG(frfResidential);
	}

	if(!re)
		return 0;

	if((flagsToTest & RSDFLAG(frfWriting)))
	{
		if(bExist && it->second != NULL)
		{
			if((it->second->contentState == TRANSFER) || (it->second->contentState == STREAMABLE))
				re |= RSDFLAG(frfWriting);
		}
	}

	if((flagsToTest & RSDFLAG(frfAbsence)))
	{
		if(bExist && it->second != NULL)
		{
			if((it->second->contentState == CANCELED) || (it->second->contentState == FAILED) || (it->second->contentState == PENDING))//pending is?
				re |= RSDFLAG(frfAbsence);
		}
	}

	if((flagsToTest & RSDFLAG(frfCorrupt)))
	{
		if(bExist && it->second != NULL)
		{
			if((it->second->contentState == CANCELED) || (it->second->contentState == FAILED))
				re |= RSDFLAG(frfCorrupt);
		}
	}

	if(it->second != NULL)
		it->second =NULL;

	return (re & flagsToTest);
}

bool ContentStoreImpl::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return false;
	}

	size_t nIndex = mainFilePathname.rfind(FNSEPC);
	if(nIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() parameter mainFilePathname not contain volume name");
		return false;
	}
	std::string strVol = mainFilePathname.substr(0,nIndex);
	std::string strName = mainFilePathname.substr(nIndex+1);

	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	DeleteCancelContent delInfo;
	size_t  sI = strName.find("_");
	if(sI == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() the contentname '%s' is invalidate",strName.c_str());
		return false;
	}

	delInfo.assetID = strName.substr(0,sI);
	delInfo.providerID = strName.substr(sI+1);
	delInfo.volumeName = strVol;
	delInfo.reasonCode = DEL_OPR_INIT;


	A3Request reqDC(pA3Ctr->getHostURL(),&(store._log));
	int state = reqDC.DeleteContent(delInfo);

	if(state == 200)//delete the content ,the update the contentstore list
	{
		ZQ::common::MutexGuard mg(pA3Ctr->_lock);
		std::vector<A3CSCtr::VOLCONTENT>::iterator itVC;
		for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
		{
			if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
			{
				std::map<std::string,A3ContentInfo::Ptr>::iterator it;
				it = itVC->_contentsM.find(strName);
				if(it != itVC->_contentsM.end())
				{
					it->second = NULL;
					itVC->_contentsM.erase(it);					
				}
				break;		
			}
		}	

	}

	if(state == -1)//network interrupt
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::deleteFileByContent() delete content (%s) error,network interrupt",mainFilePathname.c_str());
		return false;
	}
	else if(state == 409)//try it again
	{
		store._log(ZQ::common::Log::L_WARNING,"ContentStoreImpl::deleteFileByContent() delete content (%s) error,%s",mainFilePathname.c_str(),reqDC.getStatusMessage().c_str());
		return false;
	}

	return true;
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath)
{
	ContentStoreImpl::FileInfos fInfos;	

	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return fInfos;
	}
	if(rootPath == NULL || strlen(rootPath) == 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::listMainFiles() root path is NULL");
		return fInfos;
	}
	
	std::string strVol = rootPath;
	size_t sLen = strVol.length() -1;
	if(strVol[sLen] == '\\' || strVol[sLen] == '/')
		strVol[sLen] = '\0'; 
	
	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	//wait until get the Content list
	{
		bool bInitF = true;
		ZQ::common::MutexGuard mg(pA3Ctr->_lock);
		std::vector<A3CSCtr::VOLCONTENT>::iterator itVCS;
		for(itVCS = pA3Ctr->_volContents.begin(); itVCS != pA3Ctr->_volContents.end(); itVCS++)
		{
			if(itVCS->_contentsM.size() == 0)
				bInitF = false;
		}
		if(!bInitF)//not initialized
			ResetEvent(pA3Ctr->_hBegin);
	}

	WaitForSingleObject(pA3Ctr->_hBegin,INFINITE);
	while(!pA3Ctr->_bBegin)
	{
		store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::listMainFiles() wait for filesystem sync");
		Sleep(1000);
		if (pA3Ctr->_bQuit)
		{
			store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::listMainFiles() catch thread exit signal");
			return fInfos;
		}
	}

	{
		SYSTEMTIME sysTime;
		GetSystemTime(&sysTime);
		char chIsoT[30] = {0};
		bool bR = ZQ::common::TimeUtil::Time2Iso(sysTime,chIsoT,sizeof(chIsoT),false);
		if(!bR)
		{
			store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::listMainFiles() can not convert system time to ISO time");;
		}

		bool bFind = false;
		ZQ::common::MutexGuard mg(pA3Ctr->_lock);
		std::vector<A3CSCtr::VOLCONTENT>::iterator itVC;
		for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
		{
			if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
			{
				std::map<std::string,A3ContentInfo::Ptr>::iterator it;
 				for(it = itVC->_contentsM.begin(); it != itVC->_contentsM.end(); it++)
				{
					ContentStoreImpl::FileInfo fInfo;
					fInfo.filename = it->first;
					fInfo.stampLastWrite = chIsoT;
					fInfos.push_back(fInfo);
				}
				bFind = true;
				break;
			}
		}
		if(!bFind)
			store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::listMainFiles() not find contents of root path (%s)",rootPath);

	}
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
	store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::populateAttrsFromFile() here ");
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return false;
	}

	if(mainFilePathname.length() == 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::populateAttrsFromFile() mainFilePathname is NULL");
		return false;
	}

	size_t sIndex = mainFilePathname.find(FNSEPS ,0);
	if(sIndex == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::populateAttrsFromFile() mainFilePathname (%s) is invalidate",mainFilePathname.c_str());
		return false;
	}
	std::string strVol = mainFilePathname.substr(0,sIndex);
	std::string strName = mainFilePathname.substr(sIndex+1);
	
	bool bFind = false;
	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);

	std::vector<A3CSCtr::VOLCONTENT>::iterator itVC;
	ZQ::common::MutexGuard mg(pA3Ctr->_lock);
	for(itVC = pA3Ctr->_volContents.begin(); itVC != pA3Ctr->_volContents.end(); itVC++)
	{
		if(stricmp(strVol.c_str(),(itVC->_strVolName).c_str()) == 0)
		{
			bFind = true;
			break;
		}
	}

	if(!bFind)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::populateAttrsFromFile() not find the volume (%s)",strVol.c_str());
		return false;
	}

	std::map<std::string,A3ContentInfo::Ptr>::iterator it;
	it = itVC->_contentsM.find(strName);
	if(it == itVC->_contentsM.end())//not the content
	{
		store._log(ZQ::common::Log::L_WARNING,"ContentStoreImpl::populateAttrsFromFile() not find the content (%s)",strName.c_str());
		//itVC->_contentsM.insert(std::pair<std::string,A3ContentInfo::Ptr>(strName,NULL));
		return false;
	}
		
	if(it->second == NULL)//the content not attribute,do what ?
	{
		store._log(ZQ::common::Log::L_WARNING,"ContentStoreImpl::populateAttrsFromFile() the content(%s) not attribute, just remove it and sync next time",strName.c_str());
		//itVC->_contentsM.erase(it);
		return false;
	}
	
	//content size
	char chFom[30] = {0};
	if(it->second->contentSize)
	{			
		sprintf(chFom,"%lld",it->second->contentSize*1024);
		content.metaData[METADATA_FileSize] = chFom;
		memset(chFom,0,sizeof(chFom));
	}
	//support file size
	if(it->second->supportFileSize)
	{
		sprintf(chFom,"%lld",it->second->supportFileSize*1024);
		content.metaData[METADATA_SupportFileSize] = chFom;
	}
	//md5Checksum
	if(it->second->md5Checksum.length())
		content.metaData[METADATA_MD5CheckSum] = it->second->md5Checksum;
	//bitRate
	if(it->second->bitRate)
	{
		memset(chFom,0,sizeof(chFom));
		sprintf(chFom,"%d",it->second->bitRate);
		content.metaData[METADATA_BitRate] = chFom;
	}
	//playtime = filesize*8/bitrate
	if((content.metaData[METADATA_FileSize]).length() >0 && (content.metaData[METADATA_BitRate]).length() > 0)
	{
		int bitRate = atoi((content.metaData[METADATA_BitRate]).c_str());
		if(bitRate > 0)
		{
			uint64 playtime = _atoi64((content.metaData[METADATA_FileSize]).c_str()) * 8 * 1000 /bitRate;
			std::stringstream os;
			char buf[128] = {0};
			os<<playtime;
			os>>buf;
			content.metaData[METADATA_PlayTime] = buf;
		}
	}

	return true;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& contentName, 
						const ::std::string& sourceUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return NULL;
	}

	if(startTimeUTC.empty() || stopTimeUTC.empty())
	{
		ZQTianShan::_IceThrow <::TianShanIce::InvalidParameter> (store._log, EXPFMT(ContentStoreImpl, TianShanIce::Storage::csexpInvalidTime, "submitProvision() empty startTimeUTC or stopTimeUTC"));
		return NULL;
	}

	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
//	std::string strName = content.ident.name.substr(content.identVolume.name.length()+1);//content.getName(Ice::Current());
	std::string strName = contentName;
	size_t index = strName.find("_");
	if(index == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::submitProvision() the contentname '%s' is invalidate",strName.c_str());
		return NULL;
	}
	TransferInfo transInfo;
	transInfo.assetID = strName.substr(0,index);
	transInfo.providerID = strName.substr(index+1);
//	transInfo.volumeName = content.identVolume.name.substr(1);

	//get mount path
	TianShanIce::Storage::VolumeExPrx vP =  content._volume();
	transInfo.volumeName = vP->getMountPath();
	size_t sL = transInfo.volumeName.length()-1;
	if(transInfo.volumeName[sL] == FNSEPC)
		transInfo.volumeName[sL] = '\0';
	

	::std::string strLocalURL;
	::std::stringstream ss;
	ss << pA3Ctr->_port;

	//transInfo.responseURL = "http://192.168.81.114:180";
	transInfo.responseURL = "http://" + pA3Ctr->_strIP + ":" + ss.str() + "/";

//	transInfo.sourceURL = sourceUrl;

	std::string strS = sourceUrl;
	index = strS.find("@",0);
	if(index == std::string::npos)//not find
		transInfo.sourceURL = strS;
	else
	{
		size_t sBI = strS.find("//");
		size_t sEI = strS.find(":",sBI);
		if(sBI == std::string::npos || sEI == std::string::npos || sEI > index)
		{
			ZQTianShan::_IceThrow <::TianShanIce::InvalidParameter> (store._log, EXPFMT(ContentStoreImpl, TianShanIce::Storage::csexpInvalidSourceURL, "sourceURL '%s' invalidate"),sourceUrl.c_str());
			return NULL;
		}
		transInfo.sourceURL = strS.substr(0,sBI+2) + strS.substr(index+1);
		
		transInfo.userName = strS.substr(sBI+2,sEI-sBI-2);
		transInfo.password = strS.substr(sEI+1,index-sEI-1);
	}

	transInfo.captureStart = startTimeUTC;
	transInfo.captureEnd = stopTimeUTC;
	if (maxTransferBitrate <= 0)
		transInfo.transferBitRate = pNSSBaseConfig->_videoServer.vols[0].defaultBitRate;
	else
		transInfo.transferBitRate = maxTransferBitrate;

	::TianShanIce::Properties::iterator iter = content.metaData.find(METADATA_SubscriberId);
	if (iter != content.metaData.end())
		transInfo.homeId = iter->second;
	if (pNSSBaseConfig->_videoServer.ContentInterfaceMode.compare("SeaChange") == 0)
	{		
		//get cscontenttype
		if (sourceType.find(":") != ::std::string::npos)
		{
			transInfo.element["usefileset"] = "TRUE";
			::TianShanIce::StrValues strValue = ::ZQ::common::stringHelper::split(sourceType, ':');
			if (strValue.size() == 2)
			{
				if (strValue[1] == "VVX")
					transInfo.element["cscontenttype"] = "MPEG2TS";
				else if (strValue[1] == "VV2")
					transInfo.element["cscontenttype"] = "H264";
			}
		}
		else
			transInfo.element["cscontenttype"] = sourceType;
	}
	A3Request reqTC(pA3Ctr->getHostURL(),&(store._log));
	int state = reqTC.TransferContent(transInfo);
	if(state == 200)//ok
		return NULL;

	switch(state) {
		case (400):
		case (403):
		case (452):
			state = TianShanIce::Storage::csexpInvalidParam;
			break;
		case (401):
			state = TianShanIce::Storage::csexpUnauthorized;
			break;
		case (404):
			state = TianShanIce::Storage::csexpContentNotFound;
			break;
		case (409):
			state = TianShanIce::Storage::csexpNoResource;
			break;
		case (451):
			state = TianShanIce::Storage::csexpUnsupportProto;
			break;
		case (500):
		case (-1):
		default:
			state = TianShanIce::Storage::csexpInternalError;
			break;
	}

	if(state == TianShanIce::Storage::csexpInternalError)//network interrupt or server error
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(store._log,EXPFMT(ContentStoreImpl, state, "failed to provision '%s'"),strName.c_str());
	else//invalidaae state
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(store._log,EXPFMT(ContentStoreImpl, state, "failed to provision '%s'"),strName.c_str());

	return NULL;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
						::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	return NULL;
}

std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return "";
	}

	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	ExposeContentInfo expInfo;
//	std::string strName = content.ident.name.substr(content.identVolume.name.length()+1);//content.getName(Ice::Current());
	std::string strName = contentkey.content;
	size_t index = strName.find("_");
	if(index == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::getExportURL() the contentname '%s' is invalidate",strName.c_str());
		return "";
	}
	expInfo.assetID = strName.substr(0,index);
	expInfo.providerID = strName.substr(index+1);

//	expInfo.volumeName = content.identVolume.name.substr(1);
	TianShanIce::Storage::VolumeExPrx vP =  content._volume();
	expInfo.volumeName = vP->getMountPath();
	size_t sL = expInfo.volumeName.length()-1;
	if(expInfo.volumeName[sL] == FNSEPC)
		expInfo.volumeName[sL] = '\0';

//	expInfo.transferBitRate = content.getBitRate(Ice::Current());
	char chBit[10] = {0};
	sprintf(chBit,"%d",transferBitrate);
	expInfo.transferBitRate = chBit;
	expInfo.protocol = transferProtocol;
	
	ExposeResponse resp;
	A3Request reqEC(pA3Ctr->getHostURL(),&(store._log));
	int state = reqEC.ExposeContent(expInfo,resp);	
	if(state == 200)//ok,fill the attribute
	{
		params.clear();
		store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::getExportURL() expose content '%s' successful", strName.c_str());

		ttl = resp.ttl;
		typedef std::pair<std::string,std::string> strPair;
		char form[50] = {0};
		sprintf(form,"%d",resp.ttl);
		params.insert(strPair(TianShanIce::Storage::expTTL,form));
		params.insert(strPair(TianShanIce::Storage::expUserName,resp.userName));
		params.insert(strPair(TianShanIce::Storage::expPassword,resp.password));
		memset(form,0,sizeof(form));
		sprintf(form,"%d",resp.transferBitRate);
		params.insert(strPair(TianShanIce::Storage::expTransferBitrate,form));
		return resp.URL;
	}

	//error
	switch(state) {
		case (404):
			state = TianShanIce::Storage::csexpContentNotFound;
			break;
		case (409):
			state = TianShanIce::Storage::csexpContentNotReady;
			break;
		case (451):
			state = TianShanIce::Storage::csexpUnsupportProto;
			break;
		case (453):
			state = TianShanIce::Storage::csexpNoResource;
			break;
		case (-1):
		default:
			state = TianShanIce::Storage::csexpInternalError;
			break;
	}
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
		store._log,
		EXPFMT(ContentStoreImpl, state, "failed to export content '%s', '%s'"),
		strName.c_str(), 
		reqEC.getStatusMessage().c_str()
	);

	return "";

}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return;
	}

	A3CSCtr* pA3Ctr = reinterpret_cast<A3CSCtr*>(store._ctxPortal);
	DeleteCancelContent cancelInfo;
	std::string strName = content.ident.name.substr(content.identVolume.name.length()+1);//content.getName(Ice::Current());;
	size_t index = strName.find("_",0);
	if(index == std::string::npos)
	{
		store._log(ZQ::common::Log::L_ERROR,"ContentStoreImpl::cancelProvision() the contentname '%s' is invalidate",strName.c_str());
		return;
	}
	cancelInfo.assetID = strName.substr(0,index);
	cancelInfo.providerID = strName.substr(index+1);

//	cancelInfo.volumeName = content.identVolume.name.substr(1);
	TianShanIce::Storage::VolumeExPrx vP =  content._volume();
	cancelInfo.volumeName = vP->getMountPath();
	size_t sL = cancelInfo.volumeName.length()-1;
	if(cancelInfo.volumeName[sL] == FNSEPC)
		cancelInfo.volumeName[sL] = '\0';

	cancelInfo.reasonCode = CANCEL_OPR_INIT;
	A3Request reqCT(pA3Ctr->getHostURL(),&(store._log));
	int state = reqCT.CancelTransfer(cancelInfo);

	if(state == 200)//ok
	{
		store._log(ZQ::common::Log::L_DEBUG,"ContentStoreImpl::cancelProvision() the content '%s' cancel transfer implement successful",strName.c_str());
		return;
	}

	if(state == 404)//network interrupt
		state = TianShanIce::Storage::csexpContentNotFound;
	else
		state = TianShanIce::Storage::csexpInternalError;

	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
		store._log,
		EXPFMT(ContentStoreImpl, state, "failed to cnacelprovision content '%s', '%s'"),
		strName.c_str(), 
		reqCT.getStatusMessage().c_str()
		);
}

void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{
}

}
}