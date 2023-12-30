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
#include "AquaRecWrapper.h"
#include "CPH_AquaRecCfg.h"
#include "AquaRecLeadSessCol.h"
#include "AquaRecVirtualSessI.h"
#include "AquaRecLeadSessI.h"
#include "AquaRecVirtualSessFac.h"

#include "CdmiClientBase.h"

#define MOLOG			(glog)

using namespace ZQ::common;


namespace ZQTianShan 
{
namespace ContentProvision
{


AquaRecWrapper::AquaRecWrapper()
{
	_pVirSessFactory = NULL;
	_pool = NULL;
//	_log = &ZQ::common::NullLogger;

	_pVirSessFactory = NULL;
	_pCdmiClient = NULL;
}

AquaRecWrapper::~AquaRecWrapper()
{
	uninitialize();
}

// the following parameters are must(required), without it the system could not work
void AquaRecWrapper::setThreadPool(ZQ::common::NativeThreadPool* pPool)
{
	_pool = pPool;
}

void AquaRecWrapper::setLog(ZQ::common::Log* pLog)
{
	ZQ::common::setGlogger(pLog);
}

void AquaRecWrapper::setConfigPath(const char* szPath)
{
	_cfgPath = szPath;
}

bool AquaRecWrapper::loadConfig()
{
	_gCPHCfg.setLogger(&glog);

	// load configurations
	if (!_gCPHCfg.loadInFolder(_cfgPath.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(AquaRecWrapper, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());		

		return false;
	}

	_gCPHCfg.snmpRegister("");
	MOLOG(Log::L_INFO, CLOGFMT(AquaRecWrapper, "Load configuration from [%s] successfully"),
		_gCPHCfg.getConfigFilePath().c_str());

	return true;
}

bool AquaRecWrapper::initAquaRecModule()
{
	//³õÊ¼»¯ _CdmiClientBase
	//_pCdmiClient = new CDmiClientBase();
	
	_pCdmiClient = new CdmiClientBase(glog,cdmi_thpool,_gCPHCfg.aquaRootUri,_gCPHCfg.aquaContainer,_gCPHCfg.aquaFlag);
	if(NULL == _pCdmiClient)
		return false;
	_pCdmiClient->setTimeout(_gCPHCfg.connectTimeOut,_gCPHCfg.requestTimeOut);
	AquaRecLeadSessColI* pLeadSessCol = new AquaRecLeadSessCol(_pool, &glog, _pCdmiClient);
	if (NULL == pLeadSessCol)
		return false;
	AquaRecLeadSessColI::setInstance(pLeadSessCol);
	pLeadSessCol->setMaxSessionCount(_gCPHCfg.maxLeadsessionNum);
	pLeadSessCol->setMaxIdleTime(_gCPHCfg.leadsesslagAfterIdle);
      pLeadSessCol->startMonitor();
	_pVirSessFactory = new AquaRecVirtualSessFac();
	if (NULL == _pVirSessFactory)
		return false;
	return true;
}

void AquaRecWrapper::uninitAquaRecModule()
{
	if (AquaRecLeadSessColI::instance())
	{
		AquaRecLeadSessColI::instance()->stopMonitor();
	}
	
	AquaRecLeadSessColI::destroyInstance();

	if (_pVirSessFactory)
	{
		delete _pVirSessFactory;
		_pVirSessFactory = NULL;
	}

	if(_pCdmiClient)
	{
		delete _pCdmiClient;
		_pCdmiClient = NULL;
	}
}

bool AquaRecWrapper::initialize()
{
	if (!loadConfig())
		return false;

	if(!initAquaRecModule())
		return false;
	return true;
}

void AquaRecWrapper::uninitialize()
{
	uninitAquaRecModule();
}

AquaRecVirtualSessI* AquaRecWrapper::generateVirtualSession(const std::string& strChannelName, const std::string& contentId, const std::string& sessId, const std::string& startTimeUTC, const std::string& endTimeUTC, int nBandwidth)
{
	std::auto_ptr<AquaRecVirtualSessI> pVirtualSess(_pVirSessFactory->create());
	pVirtualSess->setLog(&glog);
	//set other parameters
    pVirtualSess->setContentId(contentId);
	pVirtualSess->setChannnelName(strChannelName);
	pVirtualSess->setSessionId(sessId);
    pVirtualSess->setScheduledTime(startTimeUTC, endTimeUTC);
    pVirtualSess->setBandWidth(nBandwidth);
	if (!pVirtualSess->initialize())
	{
		return NULL;
	}
	MOLOG(Log::L_INFO, CLOGFMT(AquaRecWrapper, "generateVirtualSession initialize new VirSess [%s] on channel [%s] ,startTime [%s] ,endTime [%s]successfully"),contentId.c_str(),strChannelName.c_str(),startTimeUTC.c_str(),endTimeUTC.c_str());
	return pVirtualSess.release();
}


}
}