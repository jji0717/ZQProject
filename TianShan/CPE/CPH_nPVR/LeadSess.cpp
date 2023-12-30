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

#include "LeadSess.h"
#include "BaseClass.h"
#include "SubjectWrite.h"
#include "HostToIP.h"
#include "McastCapSrc.h"
#include "CPH_NPVRCfg.h"
#include "RTFProc.h"
#include "VstrmNPVRTarget.h"
#include "TargetFactoryI.h"
#include "NTFSSource.h"
#include "NPVRExtendFile.h"
#include "VirtualSessI.h"
#include "CPH_NPVR.h"

#define MOLOG			(*_pLog)
#define LeaderS			"LeaderS"


using namespace ZQ::common;

namespace ZQTianShan 
{
namespace ContentProvision
{

LeadSess::LeadSess(NativeThreadPool* pool, ZQ::common::BufferPool* pAlloc, FileIoFactory* pFileIoFactory)
	:ThreadRequest(*pool)
{
	_bMediaInfoReady = false;
	SetMemAlloc(pAlloc);

	_pFileIoFactory = pFileIoFactory;

	_pEvent = NULL;

	_bInitialized = false;
	_bExecuted = false;
	_bStoped = false;

	_hThreadExited = CreateEvent(NULL, TRUE, TRUE, NULL);
}

LeadSess::~LeadSess()
{
	uninitialize();

	if (_pEvent)
	{
		_pEvent->onDestroy(this);
	}
	_nNetSelector->freeInterface(_strFilename);
}

int LeadSess::run()
{
	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] run() thread enter"), _strLogHint.c_str());
	ResetEvent(_hThreadExited);

	bool bRet = false;
	try
	{
		bRet = BaseGraph::Run();
		if (!bRet)
		{
			std::string strErr = BaseGraph::GetLastError();
			int nErrCode = GetLastErrorCode();

			setLastError(strErr, nErrCode);
		}
	}
	catch(...)
	{
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] BaseGraph::Run() caught unknown exception"), _strLogHint.c_str());
		setLastError("Lead session caught unknown exception", 0);
	}

	try
	{
		BaseGraph::Close();
	}
	catch(...)
	{
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] BaseGraph::Close() caught unknown exception"), _strLogHint.c_str());		
	}

	SetEvent(_hThreadExited);
	_nNetSelector->freeInterface(_strFilename);
	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] run() thread leave"), _strLogHint.c_str());
	return 0;
}

bool LeadSess::initialize()
{
	MutexGuard	op(_lock);

	if (_bInitialized)
		return true;

	int nMaxBandwidth = _nBandwidth;

	_strLogHint = _strFilename;

	std::map<std::string, int> exMap;
	
	bool bTest = _gCPHCfg.enableNtfsSource;
	BaseSource *pSourceA= NULL;
	if (bTest)
	{
		NTFSIOSource* pNtfsSource = (NTFSIOSource*)SourceFactory::Create(SOURCE_TYPE_NTFSSRC);
		AddFilter(pNtfsSource);    //only after this, the log handle will be parsed in
		pNtfsSource->setMaxBandwidth(nMaxBandwidth);	
		pNtfsSource->setFileName(_gCPHCfg.szNTFSSource);
		pNtfsSource->setUtfFlag(false);
		pSourceA = pNtfsSource;
	}
	else
	{
		DWORD timeoutInterval = _gCPHCfg.timeoutInterval;
		std::string localIpUntrans; 

		if (!_nNetSelector->allocInterface(nMaxBandwidth,localIpUntrans,_strFilename))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(LeaderS, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
			return false;
		}
		if (!HostToIP::translateHostIP(localIpUntrans.c_str(),_strLocalIP))//translate host name to ip
			_strLocalIP = localIpUntrans;
		MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] multicast local ip is %s"), _strLogHint.c_str(),_strLocalIP.c_str());
		McastCapSource* pSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC);
		AddFilter(pSource);
		pSource->setInspectPara(_strMulticastIp,_nPort,timeoutInterval,_strLocalIP);
		pSource->setDelayDataNotify(0);

		//dumper parameters
		pSource->enableDump(_gCPHCfg.enableDump);
		pSource->setDumpPath(_gCPHCfg.szDumpPath);
		pSource->deleteDumpOnSuccess(_gCPHCfg.deleteDumpOnSuccess);
		pSourceA = pSource;
	}

	bool bH264Type = false;
	if (_ctType == RtiParams::H264)
	{
		bH264Type = true;
		exMap.insert(std::make_pair(std::string(".FFR"),0));
	}
	else
	{
		exMap.insert(std::make_pair(std::string(".FF"),0));
		exMap.insert(std::make_pair(std::string(".FR"),0));
	}

	RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF);
	AddFilter(pProcess);
	pProcess->setTrickFile(exMap);
	if (bH264Type)
	{
		pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
	}

	{
		VstrmNPVRTarget* pTarget = (VstrmNPVRTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_VSTRMNPVR);
		if(!AddFilter(pTarget))
			return false;
		pTarget->setFilename(_strFilename.c_str());
		pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
		pTarget->enablePacingTrace(_gCPHCfg.enablePacingTrace);
		pTarget->setEnableRAID1ForIndex(_gCPHCfg.enableRAID1ForIndex);

		pTarget->setBandwidth(nMaxBandwidth);		
		
		pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
		pTarget->enableMD5(false);
		pTarget->enableStreamableEvent(false);
		pTarget->enablePacing(true);
		pTarget->setTrickFile(exMap);
//		pTarget->setDeleteOnFailure(false);
		pTarget->setWriteNotification(this);
		if (bH264Type)
		{
			pTarget->setTypeH264();
		}
		InitPins();

		if (!ConnectTo(pSourceA, 0, pProcess, 0))
			return false;

		if (!ConnectTo(pProcess, 0, pTarget, 0))
			return false;
		if (!ConnectTo(pProcess, 1, pTarget, 1))
			return false;
		if (!ConnectTo(pProcess, 2, pTarget, 2))
			return false;

		if (!bH264Type)
			if (!ConnectTo(pProcess, 3, pTarget, 3))
				return false;
	}

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!BaseGraph::Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LeaderS, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		//	setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	_bInitialized = true;

	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

bool LeadSess::execute()
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

void LeadSess::stop()
{
	MutexGuard	op(_lock);

	if (!_bStoped)
	{
		BaseGraph::Stop();
		_bStoped = true;
	}
}

void LeadSess::uninitialize()
{
	MutexGuard	op(_lock);

	if (_hThreadExited == INVALID_HANDLE_VALUE)
		return;

	if (!_bStoped)
	{
		BaseGraph::Stop();
	}

	DWORD dwRet = WaitForSingleObject(_hThreadExited, _gCPHCfg.leadsesslagAfterIdle);
	if (dwRet == WAIT_TIMEOUT)
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
	
	notifyObserverDestroy();

	CloseHandle(_hThreadExited);
	_hThreadExited = INVALID_HANDLE_VALUE;
}

void LeadSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	_mediaInfo = mInfo;
	_bMediaInfoReady = true;
}

bool LeadSess::notifyWrite(const std::string& file, void* pBuf, int nLen)
{
	notifyObserverWrite(file,pBuf,nLen);
	return true;
}                

void LeadSess::setLog(ZQ::common::Log* pLog)
{
	BaseGraph::SetLog(pLog);
}

bool LeadSess::getMediaInfo(MediaInfo& mInfo)
{
	if (!_bMediaInfoReady)
		return false;

	mInfo = _mediaInfo;
	return true;
}

void LeadSess::final(int retcode, bool bCancelled)
{
	delete this;
}

void LeadSess::makeReservation(ObserverWriteI* pObserver)
{
	SubjectWrite::makeReservation(pObserver);

	MOLOG(Log::L_INFO, CLOGFMT(LeaderS, "[%s] makeReservation(), current lead session path name: %s"), _strLogHint.c_str(), 
		getSessionGroup().getPathName().c_str());
}

//#if 0
//if (!updateExtionFile())
//{
//	MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VirSess, "[%s] failed to updateExtionFile()"), _strLogHint.c_str());
//	return false;
//}
//#endif
bool LeadSess::updateExtionFile(const std::string& strVirtualName)
{
	NPVRExtendFileI* pNPVRExt = new NPVRExtendFile(_pFileIoFactory);
	if (!pNPVRExt)
		return false;

	pNPVRExt->setLog(_pLog);

	std::string subExtionFile = getSessionGroup().getPathName() + ".xml";
	pNPVRExt->setFileName(subExtionFile);

	if (pNPVRExt->load())
	{		
		MOLOG(ZQ::common::Log::L_INFO,CLOGFMT(LeaderS, "[%s] extension file %s loaded"),
			_strLogHint.c_str(), subExtionFile.c_str());
	}

	std::string subIndexFile = getSessionGroup().getPathName();
	if (_ctType == MPEG2)
		subIndexFile += ".vvx";
	else
		subIndexFile += ".vv2";

	pNPVRExt->addSubFile(subIndexFile);
	pNPVRExt->addSubscriber(strVirtualName);	

	if (!pNPVRExt->save())
		return false;

	MOLOG(ZQ::common::Log::L_INFO,CLOGFMT(LeaderS, "[%s] updateExtionFile(%s) successfully"),
		_strLogHint.c_str(), subExtionFile.c_str());
	return true;
}


void LeadSess::registerObserver( ObserverWriteI* pObserver )
{
	SubjectWrite::registerObserver(pObserver);

#if 0
	VirtualSessI* pVirSess = (VirtualSessI*)pObserver;
	MutexGuard op(_lock);

	updateExtionFile(pVirSess->getSessionGroup().getSourceFile());
#endif
}

bool LeadSess::isInitialized()
{
	MutexGuard	op(_lock);
	return _bInitialized;
}

bool LeadSess::isExecuted()
{
	MutexGuard	op(_lock);
	return _bExecuted;
}

}
}
