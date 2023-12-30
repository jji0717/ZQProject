// ===========================================================================
// Copyright (c) 2008 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#include "AquaRecLeadSess.h"
#include "BaseClass.h"
#include "CPH_AquaRecCfg.h"
#include "AquaRecVirtualSessI.h"
#include "CPH_AquaRec.h"
#include "CdmiClientBase.h"
#define MOLOG			(*_pLog)
#define LeaderS			"AquaRecLeaderS"


using namespace ZQ::common;

namespace ZQTianShan 
{
namespace ContentProvision
{

AquaRecLeadSess::AquaRecLeadSess(NativeThreadPool* pool, CdmiClientBase* pCdmiClient)
	:ThreadRequest(*pool), _pCdmiClient(pCdmiClient)
{
	_bMediaInfoReady = false;

	_pEvent = NULL;
	_bInitialized = false;
	_bExecuted = false;
	_bStoped = false;
	//initialize the _mediaInfo 
	_mediaInfo.bitrate = 2400;
	_mediaInfo.videoBitrate = 2400;
	_mediaInfo.videoResolutionH = 720;
	_mediaInfo.videoResolutionV = 1280;
	_mediaInfo.filesize = 10240000;
	_mediaInfo.framerate = 23.98;
	_mediaInfo.playTime = 7200;
	//_hThreadExited = CreateEvent(NULL, TRUE, TRUE, NULL);
	_lastIdleStamp = SYS::getTickCount();
	_bitrateContainers.clear();
}

AquaRecLeadSess::~AquaRecLeadSess()
{
	uninitialize();

	if (_pEvent)
	{
		_pEvent->onDestroy(this);
	}
}

int AquaRecLeadSess::run()
{
	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] run() thread enter"), _strLogHint.c_str());
	int32 waitInterval = 180000;
	if (_gCPHCfg.leadsessReadInterval > waitInterval)
		waitInterval = _gCPHCfg.leadsessReadInterval;
	try
	{
		while(!_bStoped)
		{
			// cancel: TODO step 1. determine if it is okay to quit the loop
			// TODO step 2. check those virtual session started but not yet streamable, see if they can enter streamable

			_hLeadSessWait.wait(waitInterval);
			if (_bStoped)
				break;
			ZQ::common::MutexGuard gd(_lockVirSesss);
			TianShanIce::StrValues currentSourceBitrates;
			currentSourceBitrates.clear();
			TianShanIce::StrValues newSourceBitrates;
			newSourceBitrates.clear();
			if(readBitratesFolder(currentSourceBitrates))
				compareBitrates(currentSourceBitrates,newSourceBitrates);
			bool retCreate = true;
			if (!_bStoped && !newSourceBitrates.empty())
			{
				retCreate = createBitratesFolder(newSourceBitrates);
				updateBFSubscribers(newSourceBitrates);
			}
			if(retCreate && !currentSourceBitrates.empty())
				_bitrateContainers = currentSourceBitrates;
			AquaRecVirSessLists::iterator iter = _aquaRecVirSessList.begin();
			for(;!_bStoped && iter != _aquaRecVirSessList.end(); iter++)
			{
				if((*iter)->getState() == AquaRecVirtualSessI::StateProcessing && !(*iter)->getContentState())
				{
					for (TianShanIce::StrValues::iterator itBR = _bitrateContainers.begin(); itBR <_bitrateContainers.end(); itBR++)
					{
						if((*iter)->readContent(_pCdmiClient,*itBR))
							break;
					}
				}
			}
		}
	}
	catch(...)
	{
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] BaseGraph::Run() caught unknown exception"), _strLogHint.c_str());
		setLastError("Lead session caught unknown exception", 0);
	}
/*
	try
	{
		BaseGraph::Close();
	}
	catch(...)
	{
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] BaseGraph::Close() caught unknown exception"), _strLogHint.c_str());		
	}
*/
	_hThreadQuit.signal();
	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] run() thread leave"), _strLogHint.c_str());
	return 0;
}

bool AquaRecLeadSess::initialize()
{
	MutexGuard	op(_lock);

	if (_bInitialized)
		return true;

	if (!BaseGraph::Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LeaderS, "[%s] failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		//	setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	_bInitialized = true;

	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

bool AquaRecLeadSess::execute()
{
	MutexGuard	op(_lock);

	if (_bExecuted)
		return true;

	if (!Start())
	{
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] failed to start graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}
	
	_bExecuted = true;
	return ThreadRequest::start();
}

void AquaRecLeadSess::stop()
{
	MutexGuard	op(_lock);

	if (!_bStoped)
	{
		BaseGraph::Stop();
		_bStoped = true;
	}
	_hLeadSessWait.signal();
}

void AquaRecLeadSess::uninitialize()
{
	MutexGuard	op(_lock);
// 	if (_hThreadExited == INVALID_HANDLE_VALUE)
// 		return;
	if (!_bStoped)
	{
		BaseGraph::Stop();
	}
//	DWORD dwRet = WaitForSingleObject(_hThreadExited, _gCPHCfg.leadsesslagAfterIdle);
	SYS::SingleObject::STATE sRet = _hThreadQuit.wait(_gCPHCfg.leadsesslagAfterIdle);
	if (sRet == SYS::SingleObject::TIMEDOUT)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(LeaderS, "[%s] Close() wait for LeadSess thread exit timeout"), _strLogHint.c_str());		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] Closed"), _strLogHint.c_str());		
	}

	BaseGraph::Close();

	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] stop with status[%s], message[%s]"), _strLogHint.c_str(), 
		(IsErrorOccurred()?"failure":"success"), IsErrorOccurred()?_strLastErr.c_str():"");		
	
// 	CloseHandle(_hThreadExited);
// 	_hThreadExited = INVALID_HANDLE_VALUE;
}

void AquaRecLeadSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	_mediaInfo = mInfo;
	_bMediaInfoReady = true;
}             

void AquaRecLeadSess::setLog(ZQ::common::Log* pLog)
{
	BaseGraph::SetLog(pLog);
}

bool AquaRecLeadSess::getMediaInfo(MediaInfo& mInfo)
{
	mInfo = _mediaInfo;
	return true;
}

void AquaRecLeadSess::final(int retcode, bool bCancelled)
{
	delete this;
}

bool AquaRecLeadSess::updateScheduledTime(AquaRecVirtualSessI* pVirtualSess)
{
   ZQ::common::MutexGuard gd(_lockVirSesss);
    return updateSubscribers();
}

bool AquaRecLeadSess::add(AquaRecVirtualSessI* pVirtualSess)
{
		ZQ::common::MutexGuard gd(_lockVirSesss);
		if (_bitrateContainers.empty())
		{
			if(!readBitratesFolder(_bitrateContainers))
				return false;
		}
		// nPVR step 4. for a new session joined if there is any, CPM_AquaRec creates container <cdmiroot>/npvr/contents/<contentid>; AND under it, create all known subcontainers
		for (TianShanIce::StrValues::iterator itBR = _bitrateContainers.begin(); itBR <_bitrateContainers.end(); itBR++)
		{
			if(!createFolder(pVirtualSess,*itBR))
				return false;
		}
		_aquaRecVirSessList.push_back(pVirtualSess);

		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] add virtual session [%s]"),_strLogHint.c_str(),pVirtualSess->getContentId().c_str());		
	
	 updateSubscribers();
	_lastIdleStamp = SYS::getTickCount();
	return true;
}
bool AquaRecLeadSess::remove(AquaRecVirtualSessI* pVirtualSess)
{
	ZQ::common::MutexGuard gd(_lockVirSesss);
	AquaRecVirSessLists::iterator iter = std::find(_aquaRecVirSessList.begin(), _aquaRecVirSessList.end(), pVirtualSess);
	if(iter != _aquaRecVirSessList.end())
	{
		if (!pVirtualSess->getContentState())
		{
			for (TianShanIce::StrValues::iterator iter = _bitrateContainers.begin(); iter <_bitrateContainers.end(); iter++)
			{
				if(pVirtualSess->readContent(_pCdmiClient,*iter))
					break;
			}
		}
		if (pVirtualSess->getContentState())
		{
			pVirtualSess->setState(AquaRecVirtualSessI::StateSuccess);
		}
		else
		{
			std::string strError = "there are no file in the content  bitrates folder";
			(*iter)->setLastError(strError,ERRCODE_AQUAREC_FAILTORECORD);
		}
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s]remove virtual session [%s] "),_strLogHint.c_str(),(*iter)->getContentId().c_str());		
		_aquaRecVirSessList.erase(iter);
	}
	updateSubscribers();
	_lastIdleStamp = SYS::getTickCount();
   return true;
}
bool AquaRecLeadSess::createFolder( AquaRecVirtualSessI* pVirtualSess ,const std::string &bitrate)
{
	int64 lStartTime = ZQ::common::TimeUtil::now();
	std::string strContentName = pVirtualSess->getContentId();
	strContentName += "/" + bitrate;
	std::string contentUri = _pCdmiClient->pathToUri(strContentName);
	CdmiClientBase::CdmiRetCode createRet = _pCdmiClient->nonCdmi_CreateContainer(contentUri);
	if(CdmiRet_SUCC(createRet) || CdmiClientBase::cdmirc_Conflict == createRet)
	{
		MOLOG(Log::L_INFO,CLOGFMT(LeaderS,"[%s] create bitrate folder[%s] successfully took %dms"), _strLogHint.c_str(), contentUri.c_str(), (int)(ZQ::common::TimeUtil::now() - lStartTime));
		return true;
	}
	else
	{
		MOLOG(Log::L_ERROR,CLOGFMT(LeaderS,"[%s] failed to create bitrate folder[%s] with error [%d ==>%s] took %dms"), _strLogHint.c_str(), contentUri.c_str(), createRet, CdmiClientBase::cdmiRetStr(createRet), (int)(ZQ::common::TimeUtil::now() - lStartTime));
		return false;
	}
}
bool AquaRecLeadSess::createBitratesFolder(TianShanIce::StrValues& currentSourceBitrates)
{
	bool retCreate = true;
	for (TianShanIce::StrValues::iterator itNewBR = currentSourceBitrates.begin(); itNewBR <currentSourceBitrates.end(); itNewBR++)
		for (AquaRecVirSessLists::iterator iter = _aquaRecVirSessList.begin(); iter < _aquaRecVirSessList.end(); iter++)
		{
			if(!createFolder(*iter,*itNewBR))
				retCreate = false;
		}
	return retCreate;
}
bool AquaRecLeadSess::readBitratesFolder(TianShanIce::StrValues& currentSourceBitrates)
{
	int64 lStartTime = ZQ::common::TimeUtil::now();
	currentSourceBitrates.clear();
	std::string readUri = _pCdmiClient->pathToUri(_channnelName);
	Json::Value cdmiReadeRet;
	cdmiReadeRet.clear();
	CdmiClientBase::CdmiRetCode readRet = _pCdmiClient->cdmi_ReadContainer(cdmiReadeRet,readUri);
	if (CdmiRet_SUCC(readRet))
	{
		if (cdmiReadeRet.isMember("children"))
		{
			Json::Value childValue = cdmiReadeRet["children"];
			if (childValue.size() > 0)
			{
				for(int i=0;i<childValue.size();i++)
				{
					Json::Value childStr = childValue[i];
					std::string strContent = childStr.asString();
					if (!strContent.empty() && (strContent[strContent.length()-1] == '/' || strContent[strContent.length() -1] == '\\'))
					{
						if (strContent[0] == '/' || strContent[0] == '\\')
							strContent = strContent.substr(1);
						currentSourceBitrates.push_back(strContent);
					}
				}
			}
		}
		else //(!cdmiReadeRet.isMember("children"))
		{
			MOLOG(Log::L_ERROR,CLOGFMT(LeaderS,"[%s] failed to read bitrates folder [missed children metadata] took %dms"),_strLogHint.c_str(), (int)(ZQ::common::TimeUtil::now() - lStartTime));
			currentSourceBitrates.clear();
			return false;
		}

		if(currentSourceBitrates.empty())
		{
			MOLOG(Log::L_WARNING, CLOGFMT(LeaderS, "[%s] there are no bitrate folder in source channel"), _strLogHint.c_str());
		}

		std::string bitratefolder = "";
		for(int i = 0; i < currentSourceBitrates.size(); i++)
		{
			bitratefolder += currentSourceBitrates[i];
			bitratefolder += ",";
		}
		///TODO: add  took time
		MOLOG(Log::L_INFO,CLOGFMT(LeaderS,"[%s] read bitrates folder list[%s] took %dms"), _strLogHint.c_str(), bitratefolder.c_str(), (int)(ZQ::common::TimeUtil::now() - lStartTime));
	}
	else //(!CdmiRet_SUCC(readRet))
	{
		MOLOG(Log::L_ERROR,CLOGFMT(LeaderS,"[%s] failed to read bitrates folder with error[%d==>%s] took %dms"),_strLogHint.c_str(), readRet, CdmiClientBase::cdmiRetStr(readRet), (int)(ZQ::common::TimeUtil::now() - lStartTime));
		currentSourceBitrates.clear();
		return false;
	}
	return true;
}
void AquaRecLeadSess::compareBitrates(TianShanIce::StrValues& currentSourceBitrates,TianShanIce::StrValues& newSourceBitrates)
{
	for (TianShanIce::StrValues::iterator itCurrentBR = currentSourceBitrates.begin(); itCurrentBR< currentSourceBitrates.end();itCurrentBR++)
		if (_bitrateContainers.end() == std::find(_bitrateContainers.begin(),_bitrateContainers.end(),*itCurrentBR))
			newSourceBitrates.push_back(*itCurrentBR);
}
bool AquaRecLeadSess::updateBFSubscribers(TianShanIce::StrValues& bitrateContainers)
{
	Json::Value sendJson;
	sendJson.clear();
	for(AquaRecVirSessLists::iterator iter = _aquaRecVirSessList.begin(); iter < _aquaRecVirSessList.end(); iter++)
	{
		Json::Value sessJson;
		sessJson.clear();
		std::string strConentId = (*iter)->getContentId();
		std::string strSessId = (*iter)->getSessId();
		std::string startTimeUTC, endTimeUTC;
		(*iter)->getScheduledTime(startTimeUTC, endTimeUTC);
		sessJson["start"] = startTimeUTC;
		sessJson["end"] = endTimeUTC;
		//TODO cut off the prefix
		std::string strCid("");
		int index = strConentId.find_last_of('/');
		if (index == -1)
			strCid = strConentId;
		else
			strCid.assign(strConentId.begin()+index+1,strConentId.end());
		sessJson["cid"] = strCid; 
		sessJson["id"] = strSessId;
		sessJson["destName"] = _gCPHCfg.destName;
		sendJson .append(sessJson);
	} // for subscriber
	Json::Value sendValue;
	for (TianShanIce::StrValues::iterator itBR = bitrateContainers.begin(); itBR <bitrateContainers.end(); itBR++)
	{
		int64 lStartTime = ZQ::common::TimeUtil::now();
		sendValue.clear();
		for (int i = 0;i < sendJson.size();i++)
		{
			Json::Value sessValue = sendJson[i];
			if (sessValue.isMember("cid"))
			{
				std::string sCid =sessValue["cid"].asString();
				sCid = sCid +"/" + *itBR;
				sessValue["cid"] = sCid;
				sendValue.append(sessValue);
			}
			else //(!sessValue.isMember("cid"))
				MOLOG(Log::L_WARNING, CLOGFMT(LeaderS, "[%s] updateBFSubscribers() there are no cid in current json"),_strLogHint.c_str());
		}
		Json::Value metadata;
		metadata.clear();
		metadata["npvr_subscribers"] = sendValue;
		Json::Value result;
		std::string location;
		std::string srcContainerName = _channnelName + "/" +*itBR;
		std::string uri = _pCdmiClient->pathToUri(srcContainerName +"?metadata:npvr_subscribers");
		CdmiClientBase::CdmiRetCode retCode = _pCdmiClient->cdmi_UpdateContainerEx(result,location, uri,metadata);
		if(CdmiRet_SUCC(retCode))
			MOLOG(Log::L_DEBUG, CLOGFMT(LeaderS, "[%s] update bitrate folder [%s] subscribers list successfully took %dms"), _strLogHint.c_str(), (*itBR).c_str(),  (int)(ZQ::common::TimeUtil::now() - lStartTime));
		else
			MOLOG(Log::L_ERROR,CLOGFMT(LeaderS,"[%s] failed to update bitrate folder [%s] subscribers list with error [%d==>%s]  took %dms"), _strLogHint.c_str(),  (*itBR).c_str(), retCode, CdmiClientBase::cdmiRetStr(retCode),  (int)(ZQ::common::TimeUtil::now() - lStartTime));	
	}//for bitrateContainers
	//test read metadata
	/*for (TianShanIce::StrValues::iterator itBR = bitrateContainers.begin(); itBR < bitrateContainers.end(); itBR++)
	{
		Json::Value readResult;
		readResult.clear();
		std::string strUri = _channnelName + "/" + *itBR;
		std::string rootUri = _pCdmiClient->pathToUri(strUri+"?metadata");
		CdmiClientBase::CdmiRetCode retReadCode = _pCdmiClient->cdmi_ReadContainer(readResult,rootUri);
		Json::FastWriter readWriter;
		if (CdmiRet_SUCC(retReadCode))
		{
			std::string readRetValue = readWriter.write(readResult);
			MOLOG(Log::L_DEBUG, CLOGFMT(LeaderS, "[%s]updateSubscribers successed to test_read the aqua and return [%s]"),_strLogHint.c_str(),readRetValue.c_str());
		}
		else
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(LeaderS, "[%s]updateSubscribers failed to test_read the aqua and return "),_strLogHint.c_str());
		}
	}*/

	return true;
}
bool AquaRecLeadSess::updateSubscribers()
{
	int64 uStart = ZQ::common::TimeUtil::now();
	MOLOG(Log::L_DEBUG,CLOGFMT(LeaderS,"[%s] updateSubscribers() enter"),_strLogHint.c_str());
	if (_aquaRecVirSessList.empty())
	{	
		MOLOG(Log::L_INFO,CLOGFMT(LeaderS,"[%s] updateSubscribers()  no virtual session"), _channnelName.c_str());
		return true;
	}
	// step 1. calls cdmi_ReadContainer(uri="<cdmiroot>/npvr/sources/<channel>/")
	// read the subcontainers into currentSourceBitrates
	// TODO
	//step 2. compare _bitrateContainers with currentSourceBitrates, determine the new bitrates into newSourceBitrates
	TianShanIce::StrValues currentSourceBitrates;
	currentSourceBitrates.clear();
	TianShanIce::StrValues newSourceBitrates;
	newSourceBitrates.clear();
	if(readBitratesFolder(currentSourceBitrates))
		compareBitrates(currentSourceBitrates,newSourceBitrates);
	// step 3.
	if (!newSourceBitrates.empty())
	{
		if(createBitratesFolder(newSourceBitrates))
			_bitrateContainers = currentSourceBitrates;
		else
			updateBFSubscribers(newSourceBitrates);
	}    
	// step 4.
//  	if(!currentSourceBitrates.empty())
// 		_bitrateContainers = currentSourceBitrates;
	updateBFSubscribers(_bitrateContainers);
	MOLOG(Log::L_INFO,CLOGFMT(LeaderS,"[%s] updateSubscribers() leave took %dms"),_strLogHint.c_str(), (int)(ZQ::common::TimeUtil::now() - uStart));
	return true;
}
//#if 0
//if (!updateExtionFile())
//{
//	MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VirSess, "[%s] failed to updateExtionFile()"), _strLogHint.c_str());
//	return false;
//}
//#endif

bool AquaRecLeadSess::isInitialized()
{
	MutexGuard	op(_lock);
	return _bInitialized;
}

bool AquaRecLeadSess::isExecuted()
{
	MutexGuard	op(_lock);
	return _bExecuted;
}

bool AquaRecLeadSess::isIdle()
{
//	Guard<Mutex>  opLock(_lock);	
	ZQ::common::MutexGuard gd(_lockVirSesss);
	return _isIdle();
}

// in mili-second
uint64 AquaRecLeadSess::getIdleTime()
{
//	Guard<Mutex>  opLock(_lock);	
	ZQ::common::MutexGuard gd(_lockVirSesss);
	if (!_isIdle())
		return 0;

	return SYS::getTickCount() - _lastIdleStamp;
}
bool AquaRecLeadSess::_isIdle()
{
	return (!_aquaRecVirSessList.size());
}

}
}
