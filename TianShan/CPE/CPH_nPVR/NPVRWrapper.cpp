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
#include "NPVRWrapper.h"
#include "CPH_NPVRCfg.h"
#include "VstrmFileIoFactory.h"
#include "NtfsFileIoFactory.h"
#include "NPVRTargetFactory.h"
#include "LeadSessCol.h"
#include "RTFProc.h"
#include "McastCapSrc.h"
#include "HostToIP.h"
#include "VirtualSessI.h"
#include "LeadSessI.h"
#include "VirtualSessFac.h"
#include "SessionGroupId.h"
#include "WPCapThread.h"



#define MOLOG			(glog)
#define NPVRWrp			"NPVRWrp"


using namespace ZQ::common;


namespace ZQTianShan 
{
namespace ContentProvision
{


NPVRWrapper::NPVRWrapper()
{
	_pVirSessFactory = NULL;
	_pool = NULL;
//	_log = &ZQ::common::NullLogger;
	_pAlloc = NULL;
	
	_pVirSessFactory = NULL;
	_pFileIoFactory = NULL;

	_bGraphModuleInited = false;
}

NPVRWrapper::~NPVRWrapper()
{
	uninitialize();
}

// the following parameters are must(required), without it the system could not work
void NPVRWrapper::setThreadPool(ZQ::common::NativeThreadPool* pPool)
{
	_pool = pPool;
}

void NPVRWrapper::setLog(ZQ::common::Log* pLog)
{
	ZQ::common::setGlogger(pLog);
}

void NPVRWrapper::setConfigPath(const char* szPath)
{
	_cfgPath = szPath;
}

void NPVRWrapper::setMemoryAllocator(ZQ::common::BufferPool* pAlloc)
{
	_pAlloc = pAlloc;
}

bool NPVRWrapper::loadConfig()
{
	_gCPHCfg.setLogger(&glog);

	// load configurations
	if (!_gCPHCfg.loadInFolder(_cfgPath.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NPVRWrp, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());		

		return false;
	}

	_gCPHCfg.snmpRegister("");
	MOLOG(Log::L_INFO, CLOGFMT(NPVRWrp, "Load configuration from [%s] successfully"),
		_gCPHCfg.getConfigFilePath().c_str());

	return true;
}

bool NPVRWrapper::initFileIoFactory()
{
	if (_gCPHCfg.enableTestNTFS)
	{
		NtfsFileIoFactory* pFactory = new NtfsFileIoFactory();
		pFactory->setRootDir(_gCPHCfg.szNTFSOutputDir);
		_pFileIoFactory = pFactory;
	}
	else
	{
		VstrmFileIoFactory* pfactory = new VstrmFileIoFactory();
		pfactory->setBandwidthManageClientId(_gCPHCfg.vstrmBwClientId);
		pfactory->setDisableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle);
		pfactory->setEnableCacheForIndex(_gCPHCfg.enableCacheForIndex);
		pfactory->setEnableRAID1ForIndex(_gCPHCfg.enableRAID1ForIndex);
		_pFileIoFactory = pfactory;
	}

	_pFileIoFactory->setLog(&glog);
	if (!_pFileIoFactory->initialize())
		return false;

	return true;
}

void NPVRWrapper::uninitFileIoFactory()
{
	if (_pFileIoFactory)
	{
		_pFileIoFactory->uninitialize();
		delete _pFileIoFactory;
		_pFileIoFactory = NULL;
	}
}

bool NPVRWrapper::initGraphModule(FileIoFactory* pFileIoFactory)
{
	//Graph target factory
	NPVRTargetFactory* pTargetFac = new NPVRTargetFactory(pFileIoFactory);
	TargetFactoryI::setInstance(pTargetFac);

	//RTF initialize
	{
		RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes, 
			_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);
	}

	_bGraphModuleInited = true;
 
	//multicast capture initialize
	{
		WinpCapThreadInterface* pCaptureInterface;
		pCaptureInterface = new WinpCapThreadInterface();
		pCaptureInterface->setKernelBufferBytes(_gCPHCfg.winpcapKernelBufferInMB*1024*1024);
		pCaptureInterface->setMinBytesToCopy(_gCPHCfg.winpcapMinBufferToCopyInKB*1024);

		MulticastCaptureInterface::setInstance(pCaptureInterface);
		pCaptureInterface->setLog(&glog);
		
		for(size_t i=0;i<_gCPHCfg.nInterface.size();i++)
		{
			std::string strLocalIp;
			if (!HostToIP::translateHostIP(_gCPHCfg.nInterface[i].strIp.c_str(), strLocalIp))//translate host name to ip
				strLocalIp = _gCPHCfg.nInterface[i].strIp;

			pCaptureInterface->addNIC(strLocalIp, _gCPHCfg.nInterface[i].totalBandwidth);
		}

		if (!pCaptureInterface->init())
		{
			MOLOG(Log::L_INFO, CLOGFMT(NPVRWrp, "Failed to initialize capture interface with error: %s"),
				pCaptureInterface->getLastError().c_str());
			return false;
		}
	}
	
	return true;
}

void NPVRWrapper::uninitGraphModule()
{
	if (_bGraphModuleInited)
	{
		MulticastCaptureInterface::destroyInstance();
	
		RTFProcess::uninitRTFLib();

		TargetFactoryI::destroyInstance();

		_bGraphModuleInited = false;
	}
}

bool NPVRWrapper::initNPVRModule()
{
	LeadSessCol* pLeadSessCol = new LeadSessCol(_pool, _pAlloc, _pFileIoFactory, &glog);
	LeadSessColI::setInstance(pLeadSessCol);
	pLeadSessCol->setMaxSessionCount(_gCPHCfg.maxLeadsessionNum);
	pLeadSessCol->setMaxIdleTime(_gCPHCfg.leadsesslagAfterIdle);
    pLeadSessCol->startMonitor();

	_pVirSessFactory = new VirtualSessFac(_pFileIoFactory);

	return true;
}

void NPVRWrapper::uninitNPVRModule()
{
	if (LeadSessColI::instance())
	{
		LeadSessColI::instance()->stopMonitor();
	}
	
	LeadSessColI::destroyInstance();

	if (_pVirSessFactory)
	{
		delete _pVirSessFactory;
		_pVirSessFactory = NULL;
	}
}

bool NPVRWrapper::initialize()
{
	if (!loadConfig())
		return false;

	if (!initFileIoFactory())
		return false;

	if (!initGraphModule(_pFileIoFactory))
		return false;

	if (!initNPVRModule())
		return false;

	return true;
}

void NPVRWrapper::uninitialize()
{
	uninitNPVRModule();

	uninitGraphModule();

	uninitFileIoFactory();
}

bool NPVRWrapper::isValidSession(const std::string& strFilename)
{
	SessionGroupId sgId(strFilename);
	
	return (strFilename != sgId.getPathName());
}

VirtualSessI* NPVRWrapper::generateVirtualSession(int nBandwidth, const std::string& strFilename, const std::string& multicastIp, int multicastPort, bool bH264Type)
{
	if (!isValidSession(strFilename))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NPVRWrp, "Invalid NPVR session filename [%s]"), strFilename.c_str());
		return NULL;
	}

	std::vector<std::string> vec;
	if (bH264Type)
	{
		vec.push_back(".ffr");
		vec.push_back(".vv2");
	}
	else
	{
		vec.push_back(".ff");
		vec.push_back(".fr");
		vec.push_back(".vvx");
	}

	std::auto_ptr<VirtualSessI> pVirtualSess(_pVirSessFactory->create());
	
	//RTI parameters
	pVirtualSess->setLog(&glog);
	pVirtualSess->setFilename(strFilename);
	pVirtualSess->setFileSuffix(vec);
	pVirtualSess->setSessionGroup(strFilename);
	pVirtualSess->setBandwidth(nBandwidth);
	pVirtualSess->setMulticast(multicastIp, multicastPort);
	pVirtualSess->setContentType(bH264Type?RtiParams::H264:RtiParams::MPEG2);
	pVirtualSess->enableMD5sum(_gCPHCfg.enableMD5);
	pVirtualSess->setTestMode(_gCPHCfg.enableTestNTFS);

	if (!pVirtualSess->initialize())
	{
		//how 
		return NULL;
	}

	return pVirtualSess.release();
}


}
}