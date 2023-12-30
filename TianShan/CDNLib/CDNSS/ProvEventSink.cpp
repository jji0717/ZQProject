
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
#include "ProvEventSink.h"
#include "CPHInc.h"
#include "TsStorage.h"
#include "ErrorCode.h"
#include "ContentImpl.h"

#define MOLOG	(_log)

using namespace ZQ::common;
using namespace ::TianShanIce::Storage;

ProvisionEventSink::ProvisionEventSink(::TianShanIce::Storage::ContentStoreExPrx csPrx, Log& log)
	:_csPrx(csPrx), _log(log)									
{

}

void ProvisionEventSink::init(::TianShanIce::Storage::ContentStoreExPrx	csPrx)
{
	_csPrx = csPrx;
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
	MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "EVENT(CPE) - Content (%s -> %s ->%s) process progress"), 
		contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());

    std::ostringstream oss;
	::TianShanIce::Properties csProEvent;
    oss << total;
	csProEvent["sys.ProgressTotal"] = oss.str().c_str();
    oss.str("");
    oss << processed;
	csProEvent["sys.ProgressProcessed"] = oss.str().c_str();

	try
	{
		_csPrx->OnProvisionEvent(peProvisionProgress, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "ContentStore OnProvisionEvent() caught exception[%s] in OnProvisionProgress"), ex.ice_name().c_str());
	}

}

void ProvisionEventSink::OnProvisionStarted(const ProvisionContentKey& contentKey, 
	::Ice::Long, const ::TianShanIce::Properties& params, 
	const ::Ice::Current& ic)
{
	::TianShanIce::Properties csProEvent;

	MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "EVENT(CPE) - Content (%s -> %s ->%s) process start"), 
		contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());

	TianShanIce::Properties::const_iterator iter;

	iter = params.find(EVTPM_MPEGBITRATE);
	if (iter != params.end()) {
		csProEvent[METADATA_BitRate] = iter->second;

		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] MPEG Bitrate: %s"), contentKey.content.c_str(), iter->second.c_str());
	}

	iter = params.find(EVTPM_FRAMERATE);
	if(iter != params.end()) {
		csProEvent[METADATA_FrameRate] = iter->second;

		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] Framerate: %s"), contentKey.content.c_str(), iter->second.c_str());
	}

	iter = params.find(EVTPM_VIDEOHEIGHT);
	if (iter != params.end()) {
		csProEvent[METADATA_PixelVertical] = iter->second;

		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] VideoHeight: %s"), contentKey.content.c_str(), iter->second.c_str());
	}		

	iter = params.find(EVTPM_VIDEOWIDTH);
	if (iter != params.end()) {
		csProEvent[METADATA_PixelHorizontal] = iter->second;

		MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "[%s] VideoWidth: %s"), contentKey.content.c_str(), iter->second.c_str());
	}

	try
	{
		_csPrx->OnProvisionEvent(peProvisionStarted, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "ContentStore OnProvisionEvent() caught exception[%s] in OnProvisionStopped"), ex.ice_name().c_str());
	}
}

void ProvisionEventSink::OnProvisionStopped(
	const ProvisionContentKey& contentKey, 
	::Ice::Long timeStamp, bool errorOccurred, const ::TianShanIce::Properties& params, const ::Ice::Current& ic)
{
	::TianShanIce::Properties csProEvent;

	MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, 
                    "EVENT(CPE) - Content (%s -> %s ->%s) process %s"), 
		            contentKey.contentStoreNetId.c_str(), 
                    contentKey.volume.c_str(), 
                    contentKey.content.c_str(), 
                    errorOccurred?"failed":"stopped");
	
	std::string strTotalSize, strPlaytime, strSupportFilesize, strMd5sum;
	std::string strMsg;
	std::string strMsgCode;

	::TianShanIce::Properties::const_iterator it = params.find(EVTPM_TOTOALSIZE);
	if (it!=params.end())
	{
		strTotalSize = it->second;
		csProEvent[METADATA_FileSize] = strTotalSize;
		glog(Log::L_INFO, "(%s) [FileSize]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_PLAYTIME);
	if(it != params.end()) 
	{
		strPlaytime = it->second;
		csProEvent[METADATA_PlayTime] = strPlaytime;
		glog(Log::L_INFO, "(%s) [Playtime]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_SUPPORTFILESIZE);
	if(it != params.end()) 
	{
		strSupportFilesize = it->second;
		csProEvent[METADATA_SupportFileSize] = strSupportFilesize;
		glog(Log::L_INFO, "(%s) [SupportFileSize]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_MD5CHECKSUM);
	if(it != params.end()) 
	{
		strMd5sum = it->second;
		csProEvent[METADATA_MD5CheckSum] = strMd5sum;
		glog(Log::L_INFO, "(%s) [MD5 Checksum]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_ERRORMESSAGE);
	if (it!=params.end())
	{
		strMsg = it->second;

		if (!strMsg.empty())
			glog(Log::L_INFO, "(%s) [error message]: %s", contentKey.content.c_str(), it->second.c_str());
	}

	it = params.find(EVTPM_ERRORCODE);
	if (it!=params.end())
	{
		strMsgCode = it->second;

		if (!strMsgCode.empty())
			glog(Log::L_INFO, "(%s) [error code]: %s", contentKey.content.c_str(), it->second.c_str());
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
	csProEvent["sys.LastErrMsg"] = strMsg;

	try
	{
		_csPrx->OnProvisionEvent(peProvisionStopped, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "ContentStore OnProvisionEvent() caught exception[%s] in OnProvisionStopped"), ex.ice_name().c_str());
	}
}

void ProvisionEventSink::OnProvisionStreamable(
	const ProvisionContentKey& contentKey, 
	::Ice::Long timeStamp, 
	bool streamable, 
	const ::TianShanIce::Properties& params, 
	const ::Ice::Current& ic)
{
	MOLOG(Log::L_INFO, CLOGFMT(ProvisionEvent, "EVENT(CPE) - Content (%s -> %s ->%s) content streamable"), 
		contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), contentKey.content.c_str());

	::TianShanIce::Properties csProEvent;

	try
	{
		_csPrx->OnProvisionEvent(peProvisionStreamable, contentKey.contentStoreNetId, contentKey.volume, contentKey.content, csProEvent);
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(ProvisionEvent, "ContentStore OnProvisionEvent() caught exception[%s] in OnProvisionStopped"), ex.ice_name().c_str());
	}
}

void ProvisionEventSink::OnProvisionDestroyed(
	const ProvisionContentKey& contentKey, 
	::Ice::Long timeStamp, 
	const ::TianShanIce::Properties& params, 
	const ::Ice::Current& ic)
{

}

