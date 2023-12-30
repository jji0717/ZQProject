
// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
//
// ===========================================================================


#include "Log.h"
#include "A3ProvEventSink.h"
#include "CPHInc.h"
#include "TsStorage.h"
#include "ErrorCode.h"
#include "ContentImpl.h"
#include "strHelper.h"
#include "A3MsgEnv.h"
#include "ContentUser.h"
#include "ContentSysMD.h"
#define MOLOG	(_log)

using namespace ZQ::common;
using namespace ::TianShanIce::Storage;
namespace CRM
{
	namespace A3Message
	{
ProvisionEventSink::ProvisionEventSink(Log& log, A3MsgEnv& env)
	: _log(log), _env(env)								
{

}

void ProvisionEventSink::OnProvisionStateChanged(
		const ProvisionContentKey& contentKey, 
		::Ice::Long timeStamp, 
		::TianShanIce::ContentProvision::ProvisionState prevState, 
		::TianShanIce::ContentProvision::ProvisionState currentState, 
		const ::TianShanIce::Properties& params, 
		const ::Ice::Current& ic)
{

}

void ProvisionEventSink::OnProvisionProgress(
	const ProvisionContentKey& contentKey, 
	::Ice::Long timeStamp, 
	::Ice::Long processed, 
	::Ice::Long total, 
	const ::TianShanIce::Properties&params, 
	const ::Ice::Current& ic)
{
	MOLOG(Log::L_DEBUG, CLOGFMT(ProvisionEvent, "OnProvisionProgress() contentKey[%s] vol[%s] content[%s] processing event"), 
		contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());

	char buf[256];
	std::string ext, strParams;
	std::vector<std::string> vecExt;
	std::vector<std::string>::iterator vecIter;

	::TianShanIce::Properties csProEvent;
	sprintf(buf, FMT64, total);
	csProEvent["sys.ProgressTotal"] = buf;
	sprintf(buf, FMT64, processed);
	csProEvent["sys.ProgressProcessed"] = buf;
	
	TianShanIce::Properties::const_iterator iter;
	iter = params.find(EVTPM_MEMBERFILEEXTS);
	if (iter != params.end()) {
		csProEvent["sys.memberFileExts"] = iter->second;
		strParams += std::string(" memberFileExts[") + iter->second + "]";
		ext = iter->second;
//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] memberFileExts:[%s]"), contentKey.content.c_str(), iter->second.c_str());
	}

	iter = params.find(EVTPM_OPENFORWRITE);
	if (iter != params.end()) {
		csProEvent["sys.OpenForWrite"] = iter->second;
		strParams += std::string(" OpenForWrite[") + iter->second + "]";
//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] OpenForWrite:[%s]"), contentKey.content.c_str(), iter->second.c_str());
	}

#ifdef PROCESSED_BY_TIME
	iter = params.find("realplaytime");
	if (iter != params.end())
		csProEvent["sys.realplaytime"] = iter->second;

	iter = params.find("allplaytime");
	if (iter != params.end())
		csProEvent["sys.allplaytime"] = iter->second;
#endif//PROCESSED_BY_TIME

	vecExt = ZQ::common::stringHelper::split(ext, ';');
	for (vecIter = vecExt.begin();vecIter != vecExt.end();vecIter++)
	{
		strParams += std::string("; memberFile[") + (*vecIter) + "]" ;
		iter = params.find(*vecIter);
		if (iter != params.end()) {
			std::vector<std::string> vecSize = ZQ::common::stringHelper::split(iter->second,'-');
			std::string begOffset,endOffset;
			if (vecSize.size() == 2)
			{
				begOffset = vecSize[0];
				endOffset = vecSize[1];
				std::string strKeyFileSize;
				std::string strKeyOffset; 
				if (vecIter == vecExt.begin())
				{
					strKeyFileSize = std::string("sys.FileSize");
					strKeyOffset = std::string("sys.FirstOffset");
					csProEvent[strKeyFileSize] = csProEvent["sys.ProgressTotal"];
					
				}
				else
				{
					strKeyFileSize = std::string("sys.FileSize.")+(*vecIter);
					strKeyOffset = std::string("sys.FirstOffset.")+(*vecIter);
					csProEvent[strKeyFileSize] = endOffset;
				}
				csProEvent[strKeyOffset] = begOffset;
				strParams += std::string(" range[") + begOffset + "~" + csProEvent[strKeyFileSize] + "]";

//				MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] FileExt[%s],FirstOffset[%s],FileSize[%s]"), contentKey.content.c_str(),(*vecIter).c_str(),begOffset.c_str(),csProEvent[strKeyFileSize].c_str());
			}

		}
	}
	

	try
	{
		char buf[64];
		memset(buf, 0, sizeof(buf));
		csProEvent[SYS_PROP(State)] =  A3AquaBase::stateStr(TianShanIce::Storage::csProvisioning);
		strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

		csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
		strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";

		//updateMetadataFile(contentKey, csProEvent);
		sendTransferStatus(contentKey, csProEvent[SYS_PROP(State)]);
//		_csPrx->OnProvisionEvent(peProvisionProgress, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "OnProvisionProgress() cont[%s] processed, params:%s"), contentKey.content.c_str(), strParams.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "OnProvisionProgress() cont[%s] caught exception[%s]"), contentKey.content.c_str(), ex.ice_name().c_str());
	}

}

void ProvisionEventSink::OnProvisionStarted(const ProvisionContentKey& contentKey, 
	::Ice::Long, const ::TianShanIce::Properties& params, 
	const ::Ice::Current& ic)
{
	::TianShanIce::Properties csProEvent;

	MOLOG(Log::L_DEBUG, CLOGFMT(ProvisionEvent, "OnProvisionStarted() contentKey[%s] vol[%s] content[%s] processing event"), 
		contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());

	std::string strParams;
	TianShanIce::Properties::const_iterator iter;

	iter = params.find(EVTPM_MPEGBITRATE);
	if (iter != params.end()) {
		csProEvent[METADATA_BitRate] = iter->second;
		strParams += std::string(" Bitrate[") + iter->second + "]";
//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] MPEG Bitrate: %s"), contentKey.content.c_str(), iter->second.c_str());
	}

	iter = params.find(EVTPM_FRAMERATE);
	if(iter != params.end()) {
		csProEvent[METADATA_FrameRate] = iter->second;
		strParams += std::string(" Framerate[") + iter->second + "]";

//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] Framerate: %s"), contentKey.content.c_str(), iter->second.c_str());
	}

	iter = params.find(EVTPM_VIDEOHEIGHT);
	if (iter != params.end()) {
		csProEvent[METADATA_PixelVertical] = iter->second;
		strParams += std::string(" VideoHeight[") + iter->second + "]";

//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] VideoHeight: %s"), contentKey.content.c_str(), iter->second.c_str());
	}		

	iter = params.find(EVTPM_VIDEOWIDTH);
	if (iter != params.end()) {
		csProEvent[METADATA_PixelHorizontal] = iter->second;
		strParams += std::string(" VideoWidth[") + iter->second + "]";

//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] VideoWidth: %s"), contentKey.content.c_str(), iter->second.c_str());
	}

	iter = params.find(EVTPM_INDEXEXT);
	if (iter != params.end()) {
		csProEvent["sys.IndexFileExt"] = iter->second;
		strParams += std::string(" IndexFileExt[") + iter->second + "]";

//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] IndexFileExt: %s"), contentKey.content.c_str(), iter->second.c_str());
	}

	iter = params.find(EVTPM_SOURCETYPE);
	if (iter != params.end()) {
		csProEvent["sys.SourceType"] = iter->second;
		strParams += std::string(" SourceType[") + iter->second + "]";
	}

	try
	{
		char buf[64];
		memset(buf, 0, sizeof(buf));
		csProEvent[SYS_PROP(State)] =  A3AquaBase::stateStr(TianShanIce::Storage::csProvisioning);
		strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

		csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
		strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";

		//updateMetadataFile(contentKey, csProEvent);
		sendTransferStatus(contentKey, csProEvent[SYS_PROP(State)]);
//		_csPrx->OnProvisionEvent(peProvisionStarted, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "OnProvisionStarted() cont[%s] processed, params:%s"), contentKey.content.c_str(), strParams.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "OnProvisionStarted() cont[%s] caught exception[%s]"), contentKey.content.c_str(), ex.ice_name().c_str());
	}
}

void ProvisionEventSink::OnProvisionStopped(
	const ProvisionContentKey& contentKey, 
	::Ice::Long timeStamp, bool errorOccurred, const ::TianShanIce::Properties& params, const ::Ice::Current& ic)
{
	::TianShanIce::Properties csProEvent;

	MOLOG(Log::L_DEBUG, CLOGFMT(ProvisionEvent, "OnProvisionStopped() contentKey[%s] vol[%s] content[%s] processing event"), 
		contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());

	std::string strTotalSize, strPlaytime, strSupportFilesize, strMd5sum;
	std::string strMsg;
	std::string strMsgCode;

	std::string strParams;
	::TianShanIce::Properties::const_iterator it = params.find(EVTPM_TOTOALSIZE);
	if (it!=params.end())
	{
		strTotalSize = it->second;
		csProEvent[METADATA_FileSize] = strTotalSize;
		strParams += std::string(" FileSize[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [FileSize]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_PLAYTIME);
	if(it != params.end()) 
	{
		strPlaytime = it->second;
		csProEvent[METADATA_PlayTime] = strPlaytime;
		strParams += std::string(" Playtime[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [Playtime]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_SUPPORTFILESIZE);
	if(it != params.end()) 
	{
		strSupportFilesize = it->second;
		csProEvent[METADATA_SupportFileSize] = strSupportFilesize;
		strParams += std::string(" SupportFileSize[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [SupportFileSize]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_MD5CHECKSUM);
	if(it != params.end()) 
	{
		strMd5sum = it->second;
		csProEvent[METADATA_MD5CheckSum] = strMd5sum;
		strParams += std::string(" MD5[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [MD5 Checksum]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_OPENFORWRITE);
	if (it != params.end())
	{
		csProEvent["sys.OpenForWrite"] = it->second;
		strParams += std::string(" OpenForWrite[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [OpenForWrite]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	std::string strAugmentationPid, strEncrypt, strAugmentBitrate, strOrigateBitate;
	it = params.find(EVTPM_AUGMENTATIONPIDS);
	if(it != params.end()) 
	{
		strAugmentationPid = it->second;
		csProEvent[METADATA_AugmentationPids] = strAugmentationPid;
		strParams += std::string(" AugmentationpPid[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [AugmentationpPid]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_AUGMENTATEDBITRATE);
	if(it != params.end()) 
	{
		strAugmentBitrate = it->second;
		csProEvent[METADATA_AugmentedBitRate] = strAugmentBitrate;
		strParams += std::string(" AugmentedBitRate[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [AugmentedBitRate]: %s", contentKey.content.c_str(), it->second.c_str());
	}
	it = params.find(EVTPM_ORIGINALBITRATE);
	if(it != params.end()) 
	{
		strOrigateBitate = it->second;
		csProEvent[METADATA_OriginalBitRate] = strOrigateBitate;
		strParams += std::string(" OriginalBitRate[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [OriginalBitRate]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_PREENCRYPTION);
	if(it != params.end()) 
	{
		strEncrypt = it->second;
		csProEvent[METADATA_PreEncryption] = strEncrypt;
		strParams += std::string(" PreEncryption[") + it->second + "]";
//		glog(Log::L_INFO, "(%s) [Encryption]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_ERRORCODE);
	if (it!=params.end())
	{
		strMsgCode = it->second;

		if (!strMsgCode.empty())
			strParams += std::string(" ErrCode[") + it->second + "]";
//			glog(Log::L_INFO, "(%s) [error code]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_ERRORMESSAGE);
	if (it!=params.end())
	{
		strMsg = it->second;
		if (!strMsg.empty())
			strParams += std::string(" ErrMsg[") + it->second + "]";
//			glog(Log::L_INFO, "(%s) [error message]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	int errCode = atoi(strMsgCode.c_str());
	
	int nCSErrorCode;
	char buf[256];
	if (!errorOccurred)
	{
		nCSErrorCode = cs200OK;	//200 means ok
	}
	else
	{
		if(errCode == ERRCODE_VSTRM_DISK_FULL) {
			nCSErrorCode = csexpVstrmDiskFull;
		}
		else if(errCode == ERRCODE_VSTRM_BANDWIDTH_EXCEEDED) {
			nCSErrorCode = csexpVstrmBwExceeded;
		}
		else if(errCode == ERRCODE_VSTRM_NOT_READY) {
			nCSErrorCode = csexpVstrmNotReady;
		}
		else if(errCode == ERRCODE_INVALID_SRCURL) {
			nCSErrorCode = csexpInvalidSourceURL;
		}
		else if(errCode == ERRCODE_USER_CANCELED) {
			nCSErrorCode = csexpUserCanceled;
		}
		else {
			nCSErrorCode = csexpInternalError;
		}	
	}
	sprintf(buf, "%d", nCSErrorCode);	

	csProEvent["sys.LastError"] = buf;
	uint32 lastError = (uint32) atoi(buf);

	csProEvent["sys.LastErrMsg"] = strMsg;

	memset(buf, 0, 256);
	std::string ext;
	std::vector<std::string> vecExt;
	std::vector<std::string>::iterator vecIter;

	TianShanIce::Properties::const_iterator iter;
	iter = params.find(EVTPM_MEMBERFILEEXTS);
	if (iter != params.end()) {
		csProEvent["sys.memberFileExts"] = iter->second;
		ext = iter->second;
		strParams += std::string(" memberFileExts[") + iter->second + "]";

//		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] memberFileExts:[%s]"), contentKey.content.c_str(), iter->second.c_str());
	}

	vecExt = ZQ::common::stringHelper::split(ext, ';');
	for (vecIter = vecExt.begin();vecIter != vecExt.end();vecIter++)
	{
		iter = params.find(*vecIter);
		strParams += std::string("; memberFile[") + (*vecIter) + "]" ;
		if (iter != params.end()) {
			std::vector<std::string> vecSize = ZQ::common::stringHelper::split(iter->second,'-');
			std::string begOffset,endOffset;
			if (vecSize.size() == 2)
			{
				begOffset = vecSize[0];
				endOffset = vecSize[1];
				std::string strKeyFileSize;
				std::string strKeyOffset; 
				if (vecIter == vecExt.begin())
				{
					strKeyFileSize = std::string("sys.FileSize");
					strKeyOffset = std::string("sys.FirstOffset");
					//csProEvent[strKeyFileSize] = csProEvent["sys.ProgressTotal"];
				}
				else
				{
					strKeyFileSize = std::string("sys.FileSize.")+(*vecIter);
					strKeyOffset = std::string("sys.FirstOffset.")+(*vecIter);
					csProEvent[strKeyFileSize] = endOffset;
				}
				
				csProEvent[strKeyOffset] = begOffset;
				strParams += std::string(" range[") + begOffset + "~" + csProEvent[strKeyFileSize] + "]";

				// MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] FileExt[%s],FirstOffset[%s],FileSize[%s]"), contentKey.content.c_str(),(*vecIter).c_str(),begOffset.c_str(),csProEvent[strKeyFileSize].c_str());
			}

		}
	}

	try
	{
		char buf[64];
		memset(buf, 0, sizeof(buf));
		//上传成功，更新MetadataFile和MainFileMetadata
		if (lastError < 300)
		{
			csProEvent[SYS_PROP(State)] =  A3AquaBase::stateStr(TianShanIce::Storage::csInService);

			strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

			csProEvent[SYS_PROP(StampProvisioned)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
			strParams += std::string(" sys.StampProvisioned[") + csProEvent[SYS_PROP(StampProvisioned)]  + "]";

			csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
			strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";

			//updateMetadataFile(contentKey, csProEvent);
			//updateMainFileMetadata(contentKey);
			sendTransferStatus(contentKey, csProEvent[SYS_PROP(State)]);
		}
		else //上传失败，清掉临时文件
		{
			csProEvent[SYS_PROP(State)] =  A3AquaBase::stateStr(TianShanIce::Storage::csOutService);
			strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

			csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
			strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";

			sendTransferStatus(contentKey, csProEvent[SYS_PROP(State)]);
			/*
			std::string strMdataFileName;
			ContentInfo cntInfo = _env.getContentInfo(contentKey.content);
			if(cntInfo.paid.empty() || cntInfo.pid.empty())
			{
			strMdataFileName = contentKey.content + strMdataExtension;
			}
			else
			{
			strMdataFileName = _env._aquaContentMdata->getAquaContentMDName(cntInfo.paid, cntInfo.pid);
			}
			_env._aquaContentMdata->deleteFile(strMdataFileName); 
			*/
		}
		_env._cpeSessionMgr->removeSession(contentKey.content);
//		_csPrx->OnProvisionEvent(peProvisionStopped, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "OnProvisionStopped() cont[%s] processed, params:%s"), contentKey.content.c_str(), strParams.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "OnProvisionStopped() cont[%s] caught exception[%s]"), contentKey.content.c_str(), ex.ice_name().c_str());
	}
}

void ProvisionEventSink::OnProvisionStreamable(
	const ProvisionContentKey& contentKey, 
	::Ice::Long timeStamp, 
	bool streamable, 
	const ::TianShanIce::Properties& params, 
	const ::Ice::Current& ic)
{
	MOLOG(Log::L_DEBUG, CLOGFMT(ProvisionEvent, "OnProvisionStreamable() contentKey[%s] vol[%s] content[%s] processing event"), 
		contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());

	::TianShanIce::Properties csProEvent;
	std::string strParams;

	try
	{
		char buf[64];
		memset(buf, 0, sizeof(buf));
		csProEvent[SYS_PROP(State)] =  A3AquaBase::stateStr(TianShanIce::Storage::csProvisioningStreamable);
		strParams += std::string(" sys.State[") + csProEvent[SYS_PROP(State)]  + "]";

		csProEvent[SYS_PROP(StampLastUpdated)] = ZQTianShan::TimeToUTC(ZQ::common::now(), buf, sizeof(buf));
		strParams += std::string(" sys.StampLastUpdated[") + csProEvent[SYS_PROP(StampLastUpdated)]  + "]";

		//updateMetadataFile(contentKey, csProEvent);
		sendTransferStatus(contentKey, csProEvent[SYS_PROP(State)]);
//		_csPrx->OnProvisionEvent(peProvisionStreamable, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "OnProvisionStreamable() cont[%s] processed, params:%s"), contentKey.content.c_str(), strParams.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "OnProvisionStreamable() cont[%s] caught exception[%s]"), contentKey.content.c_str(), ex.ice_name().c_str());
	}
}

void ProvisionEventSink::OnProvisionDestroyed(
	const ProvisionContentKey& contentKey, 
	::Ice::Long timeStamp, 
	const ::TianShanIce::Properties& params, 
	const ::Ice::Current& ic)
{

}
/*
	void ProvisionEventSink::updateMetadataFile(const ProvisionContentKey& contentKey, const ::TianShanIce::Properties& csProEvent)
	{
		if(!_env._bUpdate)
			return;
		std::string strMdataFileName;
		ContentInfo cntInfo = _env.getContentInfo(contentKey.content);
		if(cntInfo.paid.empty() || cntInfo.pid.empty())
		{
			strMdataFileName = contentKey.content + strMdataExtension;
		}
		else
		{
			strMdataFileName = _env._aquaContentMdata->getAquaContentMDName(cntInfo.paid, cntInfo.pid);
		}
		_env._aquaContentMdata->updateMetadata(strMdataFileName,csProEvent); 
	}

	void ProvisionEventSink::updateMainFileMetadata(const ProvisionContentKey& contentKey)
	{
		if(!_env._bUpdate)
			return;
		std::string strMdataFileName;
		std::string strMainFileName;
		ContentInfo cntInfo = _env.getContentInfo(contentKey.content);
		if(cntInfo.paid.empty() || cntInfo.pid.empty())
		{
			strMdataFileName = contentKey.content + _env._aquaContentMdata->getAquaMdataFileExtension();
			strMainFileName = contentKey.content + _env._aquaContentMdata->getMainFileExtension();
		}
		else
		{
			strMdataFileName = _env._aquaContentMdata->getAquaContentMDName(cntInfo.paid, cntInfo.pid);
			strMainFileName = _env._aquaContentMdata->getMainFileName(cntInfo.paid, cntInfo.pid);
		}
		///1. read metadata from content.mdata File
		TianShanIce::Properties metadatas;
		int cdmiCode;
		if(!_env._aquaContentMdata->getMDFileMetadata(strMdataFileName,metadatas, cdmiCode))
			return ;

	    ///2.update metadata to mainFile metadata
		_env._aquaContentMdata->updateMainFileMetadata(strMainFileName,metadatas);
	}
*/
	void  ProvisionEventSink::sendTransferStatus(const ProvisionContentKey& contentKey, const std::string& contentState)
	{
		std::string strFilesize = "", strSupportFilesize = "", strMD5CheckSum = "";
		std::string strResponseURL = "";
		std::string strPID, strPAID, strVolume;

		std::string  strBitRate="0";
		Ice::Long  bitRate = 0, duration = 0,filesize = 0;

		std::string strMdataFileName;
		ContentInfo cntInfo = _env.getContentInfo(contentKey.content);
		if(cntInfo.paid.empty() || cntInfo.pid.empty())
		{
			strMdataFileName = contentKey.content + strMdataExtension;
		}
		else
		{
			strPAID = cntInfo.paid;
			strPID = cntInfo.pid;
			strMdataFileName = _env._aquaContentMdata->getAquaContentMDName(cntInfo.paid, cntInfo.pid);
		}

		if(!_env._aquaContentMdata)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()[%s]failed to get metedata info NULL AquaContentMData Handle"),contentKey.content.c_str());

			return;
		}
		TianShanIce::Properties metadatas;
		int cdmiCode;
		if(!_env._aquaContentMdata->getMDFileMetadata(strMdataFileName, metadatas, cdmiCode))
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()[%s]failed to get metedata info from MDfile[%s]"),
				contentKey.content.c_str(), strMdataFileName.c_str());
			return;
		}

		for (TianShanIce::Properties::const_iterator iter = metadatas.begin(); iter != metadatas.end(); iter++)
		{
			if(iter->first == METADATA_FileSize)
			{
				strFilesize = iter->second;
			}
			else if(iter->first == METADATA_SupportFileSize)
			{
				strSupportFilesize = iter->second;
			}
			else if(iter->first == METADATA_MD5CheckSum)
			{
				strMD5CheckSum = iter->second;
			}
			else if(iter->first == METADATA_ProviderAssetId)
			{
				strPAID = iter->second;
			}
			else if(iter->first == METADATA_ProviderId)
			{
				strPID = iter->second;
			}
			else if(iter->first == METADATA_ResponseURL)
			{
				strResponseURL = iter->second;
			}
			else if(iter->first == USER_PROP(Volume))
			{
				strVolume = iter->second; 
			}
			else if(iter->first == METADATA_BitRate)
			{
				strBitRate = iter->second;
			}
		}

		if ("" == strResponseURL)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(A3MessageEventSinkI, "sendTransferStatus()[%s] No Response URL to send status"),contentKey.content.c_str());
			return;
		}
		int reasonCode = 200;
		if (contentState == "OutService" || contentState == "Cleaning")
		{
			reasonCode = 500;
		}

		std::ostringstream buf;
		buf << XML_HEADER ;
		buf << "<TransferStatus " ;
		buf <<    "providerID=\""   << strPID                            << "\" ";
		buf <<    "assetID=\""      << strPAID                           << "\" ";
		buf <<    "volumeName=\""   << strVolume                        << "\" ";
		buf <<    "state=\""        << eventStateToA3State(contentState) << "\" ";
		buf <<    "reasonCode=\""   << reasonCode                        << "\" ";
		if (contentState == "InService(3)" || contentState == "InService")
		{
			{
				Ice::Long filesize,supportFileSize;
				sscanf((char*)strFilesize.c_str(), FMT64, &filesize);
				sscanf((char*)strSupportFilesize.c_str(),FMT64, &supportFileSize);

				filesize = filesize / 1024;
				supportFileSize = supportFileSize /1024;

				char tmp[40];
				memset(tmp, 0 , 40);
				sprintf(tmp, FMT64, filesize);
				strFilesize = tmp;

				memset(tmp, 0 , 40);
				sprintf(tmp, FMT64, supportFileSize);
				strSupportFilesize = tmp;
			}

			buf <<    "contentSize=\""       << strFilesize        << "\" ";
			buf <<    "bitrate=\""       << strBitRate        << "\" ";
			buf <<    "supportFileSize=\""   << strSupportFilesize << "\" ";
			buf <<    "md5Checksum=\""       << strMD5CheckSum     << "\" ";
			buf <<    "md5DateTime=\""       << GenerateUTCTime()               << "\" ";
		}
		sscanf(strBitRate.c_str(), FMT64, &bitRate);
		sscanf(strFilesize.c_str(), FMT64, &filesize);
		if(bitRate != 0)
		{
			duration = filesize * 8 / bitRate;
		}

		buf <<    "duration=\""       << duration << "\" ";
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MessageEventSinkI, "********filesize[%lld]bitRate[%lld]Duration[%lld]"), filesize, bitRate, duration);
		buf << "/>";
		std::string strRequestBoby = buf.str();
		_env._A3Client->SendRequest(contentKey.content, strResponseURL, strRequestBoby);
	}
}}