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

#include "Log.h"
#include "AquaRecVirtualSess.h"
#include "AquaRecLeadSessColI.h"
#include "ErrorCode.h"
#include "TimeUtil.h"
#include "CdmiClientBase.h"

#define MOLOG			(glog)
#define VirSess			"AquaRecVirSess"


namespace ZQTianShan 
{
namespace ContentProvision
{
	

AquaRecVirtualSess::AquaRecVirtualSess()
{
//	_log = &ZQ::common::NullLogger;
	_bUninitialized = false;
}

AquaRecVirtualSess::~AquaRecVirtualSess()
{
	uninitialize();
	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] ~AquaRecVirtualSess() "), _strLogHint.c_str());

}

bool AquaRecVirtualSess::initialize()
{
	_strLogHint = _contentId;

	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] Entering initialize()"), _strLogHint.c_str());

	//tell the lead session collection to make a reservation for this session
	if (!makeReservation())
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VirSess, "[%s] failed to makeReservation() on lead session"), _strLogHint.c_str());
		return false;
	}

	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] initialize() successful"),_strLogHint.c_str());
	return true;
}

bool AquaRecVirtualSess::makeReservation()
{
	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] makeReservation()"), _strLogHint.c_str());

	if (!AquaRecLeadSessColI::instance()->addVirtualSession(this))
	{
		return false;
	}

	return true;
}

bool AquaRecVirtualSess::execute()
{
	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] execute()"), _strLogHint.c_str());

	setState(StateProcessing);

	MOLOG(ZQ::common::Log::L_INFO,CLOGFMT(VirSess, "[%s] execute started"), _strLogHint.c_str());
	return true;
}	



void AquaRecVirtualSess::uninitialize()
{
	if (_bUninitialized)
		return;

	AquaRecLeadSessColI::instance()->removeVirtualSession(this);
	_bUninitialized = true;
}

void AquaRecVirtualSess::notifyDestroy(const std::string& strErr, int nErrorCode)
{
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(VirSess, "[%s] notifyDestroy() with error[%s], code[%d]"),
		_strLogHint.c_str(), strErr.c_str(), nErrorCode);

	setLastError(strErr, nErrorCode);
	AquaRecLeadSessColI::instance()->removeVirtualSession(this);	
}	

void AquaRecVirtualSess::setLog(ZQ::common::Log* pLog)
{
	ZQ::common::setGlogger(pLog);
}

bool AquaRecVirtualSess::getMediaInfo(MediaInfo& mInfo)
{
	return AquaRecLeadSessColI::instance()->getMediaInfo(this, mInfo);
}

void AquaRecVirtualSess::updateScheduledTime(const std::string& startTimeUTC, const std::string& endTimeUTC)
{
	MOLOG(ZQ::common::Log::L_INFO ,CLOGFMT(VirSess,"[%s] updateScheduledTime() ennter update startTime [%s] ,endTime[%s] to startTime [%s], endTime[%s]"), _strLogHint.c_str(),_startTimeUTC.c_str(), _endTimeUTC.c_str(), startTimeUTC.c_str(), endTimeUTC.c_str());
	_startTimeUTC =  startTimeUTC;
	_endTimeUTC = endTimeUTC;
	AquaRecLeadSessColI::instance()->updateScheduledTime(this);
}
void AquaRecVirtualSess::getProgress(int64& procv, int64& total)
{
	int64 startTime = ZQ::common::TimeUtil::ISO8601ToTime(_startTimeUTC.c_str());
	int64 endTime = ZQ::common::TimeUtil::ISO8601ToTime(_endTimeUTC.c_str());
	procv = ((ZQ::common::TimeUtil::now() - startTime)/1000) * _bBandWidth;
	total =	((endTime - startTime)/1000) * _bBandWidth;
}
bool AquaRecVirtualSess::readContent(CdmiClientBase* ptrCdmiClient,std::string& bitrate)
{
	int64 rStartTime = ZQ::common::TimeUtil::now();
	if (bitrate.empty())
		return false;
	std::string strUri = _contentId + "/" +bitrate;
	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess,"[%s] readContent() enter to read content folder [%s]"),_strLogHint.c_str(),strUri.c_str());
	Json::Value containerJson;
	containerJson.clear();
	Json::FastWriter writer;
	std::string uri = ptrCdmiClient->pathToUri(strUri);
	CdmiClientBase::CdmiRetCode retCode = ptrCdmiClient->cdmi_ReadContainer(containerJson,uri);
	if (CdmiRet_SUCC(retCode))
	{
		std::string strRes = writer.write(containerJson);
		if (containerJson.isMember("children"))
		{
			Json::Value childrenContent = containerJson["children"];
			if (childrenContent.size() > 0)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(VirSess, "[%s]readContent() succeed return [%s]"),_strLogHint.c_str(),strRes.c_str());
				setContentState(true);
				MOLOG(ZQ::common::Log::L_INFO,CLOGFMT(VirSess,"[%s]readContent() leave took %dms"),_strLogHint.c_str(),(int)(ZQ::common::TimeUtil::now() - rStartTime));
				return true;
			}
			else
			{
				MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess,"[%s]readContent() no file in the content folder"),_strLogHint.c_str());
			}
		}
		else
		{
			MOLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VirSess,"[%s]readContent() failed to read content folder [missed children parameter] "),_strLogHint.c_str());
		}
	}
	else 
	{
		MOLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VirSess,"[%s]readContent() failed to read content folder  from server[%s],with error [%d==>%s]"),_strLogHint.c_str(),ptrCdmiClient->_rootURI.c_str(),retCode,CdmiClientBase::cdmiRetStr(retCode));
	}
	return false;
}

}
}

