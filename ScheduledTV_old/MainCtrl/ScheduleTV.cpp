// ===========================================================================
// Copyright (c) 2004 by
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
// Name  : ScheduleTV.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-14
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/MainCtrl/ScheduleTV.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 24    05-08-30 19:59 Bernie.zhao
// 
// 1     05-08-30 18:29 Bernie.zhao
// ===========================================================================

#include "ScheduleTV.h"

#include "ScLog.h"
#include "comextra/ZqStringConv.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// for debug usage
/*
int fLogMemory = 1;       // Perform logging (0=no; nonzero=yes)?
int cBlocksAllocated = 0;  // Count of blocks allocated.
ScLog memlog("D:\\ITV\\STVDB\\mem.log", Log::L_DEBUG, 4*1024*1024);
CRITICAL_SECTION gMemCS;

// User-defined operator new.
void *operator new( size_t stAllocateBlock )
{
	::EnterCriticalSection(&gMemCS);
    static fInOpNew = 0;    // Guard flag.

    if( fLogMemory && !fInOpNew )
    {
        fInOpNew = 1;
		cBlocksAllocated++;
		glog(ZQ::common::Log::L_DEBUG, "Memory block %d allocated for %d bytes", cBlocksAllocated, stAllocateBlock);
        fInOpNew = 0;
    }
	::LeaveCriticalSection(&gMemCS);
    return malloc( stAllocateBlock );
}
// User-defined operator delete.
void operator delete( void *pvMem )
{
	::EnterCriticalSection(&gMemCS);
    static fInOpDelete = 0;    // Guard flag.
    if( fLogMemory && !fInOpDelete )
    {
        fInOpDelete = 1;
		cBlocksAllocated--;
		glog(ZQ::common::Log::L_DEBUG, "Memory block %d deallocated", cBlocksAllocated);
        fInOpDelete = 0;
    }
	::LeaveCriticalSection(&gMemCS);
    free( pvMem );
}

*/
//////////////////////////////////////////////////////////////////////////
ZQ::common::ScLog*	pGSdTrace=NULL;

ScheduleTV::ScheduleTV()
:_bClosed(FALSE)
{
	// initialize your variable
	
}
ScheduleTV::~ScheduleTV()
{
	
}

HRESULT ScheduleTV::OnInit()
{
//	Notice that the following ScLog should be ignored when using service rather than app
	// log for application, should ignore when encapsulated as a service
//	pGlog = new ScLog(m_wszLogPath, Log::L_DEBUG, 4*1024*1024);

	glog(ZQ::common::Log::L_NOTICE, L"********** ScheduleTV ITV-Playback Version: %s **********", _wcsVer);
	CoInitialize(NULL);

	try
	{
		// initialize static vars
		STVChannel::setBasePath(ZQ::comextra::Ws2As(m_wszMirrorPath).c_str());
		STVStreamUnit::_strRtspHostIP	= ZQ::comextra::Ws2As(m_wszRtspHostIP);
		STVStreamUnit::_strRtspURL		= ZQ::comextra::Ws2As(m_wszRtspURL);
		STVStreamUnit::_dwRtspHostPort	= m_dwRtspHostPort;
		STVStreamUnit::_dwRtspNsec		= m_dwRtspNsec;
	
		// initialize SM Connector	
		_pTheSMConnector = new CSMConnector();
		_pTheSMConnector->SetParam(this, 
									m_dwClientID,
									ZQ::comextra::Ws2As(m_wszBindIP).c_str(),
									ZQ::comextra::Ws2As(m_wszSMServerIP).c_str(),
									m_dwSMPort);
		_pTheSMConnector->SetTrace(m_dwTrace);
		if(m_dwTrace)
			pGSdTrace = new ZQ::common::ScLog(m_wszTraceFile, ZQ::common::Log::L_DEBUG, 64*1024*1024);
		
		glog(ZQ::common::Log::L_INFO, "ScheduleTV::OnInit()  SMConnector initialized");
		
		// initialize playlist manager
		_pThePlaylistMan = new STVPlaylistManager();
		STVPlaylistManager::setFillLength(m_dwFillLength);
				
		::CreateDirectoryW(m_wszMirrorPath,NULL);
		glog(ZQ::common::Log::L_INFO, "ScheduleTV::OnInit()  Playlist Manager initialized");

		// initialize Rtsp manager
		_pTheRtspMan = new RtspConnectionManager(20);
		glog(ZQ::common::Log::L_INFO, "ScheduleTV::OnInit()  RTSP Manager initialized");

		// initialize ISS
		TYPEINST typeInst;
		typeInst.s.dwType = m_dwType;
		typeInst.s.dwInst = m_dwInst;
		_pTheSSMan = CStreamSession::Instance(typeInst, m_dwAppUID); 
		glog(ZQ::common::Log::L_INFO, "ScheduleTV::OnInit()  ISS Manager initialized");

		// initialize stream unit
		_pStreamUnit = new STVStreamUnit();
		glog(ZQ::common::Log::L_INFO, "ScheduleTV::OnInit()  STV Stream Unit initialized");

		_bClosed = false;
	}
	catch(ZQ::common::Exception excp) {
		glog(ZQ::common::Log::L_WARNING, "ScheduleTV::OnInit()  Can not initialize SchduleTV, error: %s", excp.getString());
		return STVERROR;
	}
	catch(...) {
		glog(ZQ::common::Log::L_WARNING, "ScheduleTV::OnInit()  Can not initialize SchduleTV, some error occurred");
		return STVERROR;
	}
	glog(ZQ::common::Log::L_INFO, "********** ScheduleTV ITV-Playback initialization done **********");
	return STVSUCCESS;

}
HRESULT ScheduleTV::OnStart()
{
	// NOTE! the component start sequence must be considered!
	try
	{
		// start Rtsp Manager
		_pTheRtspMan->start();

		// start ISS Manager
		wchar_t wchIssXmlFile[MAX_PATH];
		wcscpy(wchIssXmlFile, m_wszMirrorPath);
		if(wchIssXmlFile[wcslen(wchIssXmlFile)-1]!=L'\\')
			wcscat(wchIssXmlFile, L"\\streamdata.xml");
		else
			wcscat(wchIssXmlFile, L"streamdata.xml");

		_pTheSSMan->Initialize(wchIssXmlFile);

		// start Playlist Manager
		_pThePlaylistMan->start();

		// start SM connector
		_pTheSMConnector->start();

	}
	catch(ZQ::common::Exception excp) {
		glog(ZQ::common::Log::L_WARNING, "ScheduleTV::OnStart()  Can not start SchduleTV, error: %s", excp.getString());
		return STVERROR;
	}
	catch(...) {
		glog(ZQ::common::Log::L_WARNING, "ScheduleTV::OnStart()  Can not start SchduleTV, some error occurred");
		return STVERROR;
	}
	
	glog(ZQ::common::Log::L_INFO, "********** ScheduleTV ITV-Playback Started **********");
	return STVSUCCESS;

}


HRESULT  ScheduleTV::OnNewPlayList(const char* scheduleNO, ZQ::common::IPreference* pPortPref, ZQ::common::IPreference* pPlayListPref, int nType)
{
	// log 
	return _pThePlaylistMan->OnNewPlayList(scheduleNO, pPortPref, pPlayListPref, nType);
}

HRESULT  ScheduleTV::OnPlayAssetImmediately(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref, ZQ::common::IPreference* pTimePref)
{
	// log
	return _pThePlaylistMan->OnPlayAssetImmediately(pPortPref, pPlayListPref, pTimePref);
}

HRESULT  ScheduleTV::OnHoldAsset(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference* pAssetPref)
{
	// log
	return _pThePlaylistMan->OnHoldAsset(pPortPref, pAssetPref);
}

HRESULT  ScheduleTV::OnPlaySkipToAsset(ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref)
{
	// log
	return _pThePlaylistMan->OnPlaySkipToAsset(pPortPref, pPlayListPref);
}

HRESULT  ScheduleTV::OnPlayFillerImmediately(ZQ::common::IPreference *pPortPref)
{
	// log
	return _pThePlaylistMan->OnPlayFillerImmediately(pPortPref);
}

HRESULT	ScheduleTV::OnChannelStartup(ZQ::common::IPreference* pPortPref)
{
	// TODO: log
	return _pThePlaylistMan->OnChannelStartup(pPortPref);
}

HRESULT ScheduleTV::OnChannelShutdown(ZQ::common::IPreference* pPortPref)
{
	// TODO: log
	return _pThePlaylistMan->OnChannelShutdown(pPortPref);
}

HRESULT ScheduleTV::OnConfigration(IPreference* pConfigPref)
{
	// TODO: log
	return _pThePlaylistMan->OnConfiguration(pConfigPref);
}

HRESULT ScheduleTV::OnConnected()
{
	char buff[2048];
	DWORD	dwCount =2048;
	int retval =_pThePlaylistMan->OnConnected(buff, &dwCount);
	if(retval!=STVSUCCESS)
		return retval;
	_pTheSMConnector->SendEnquireSchedule(buff);
	return STVSUCCESS;
}

HRESULT ScheduleTV::OnGetAElist(DWORD chnlID, AELIST& OutAEList)
{
	// TODO: log
	return _pThePlaylistMan->OnGetAElist(chnlID, OutAEList);
}

HRESULT ScheduleTV::OnFreeAElist(DWORD chnlID)
{
	// TODO: log
	return _pThePlaylistMan->OnFreeAElist(chnlID);
}

HRESULT ScheduleTV::OnSendAEStatus(DWORD chnlID, SSAENotification& status)
{
	// TODO: log
	char xmlbuff[MAX_SEND_XML_BUF];
	DWORD xmlcount =MAX_SEND_XML_BUF;
	memset(xmlbuff, 0, MAX_SEND_XML_BUF);
	
	int err = _pThePlaylistMan->OnSetStreamStatus(chnlID, status, xmlbuff, &xmlcount);
	if(err!=STVSUCCESS) {
		glog(ZQ::common::Log::L_ERROR, "ScheduleTV::OnSendAEStatus()  Can not update AE status on channel %06ld : %s", chnlID, GetSTVErrorDesc(err));
		return err;
	}

	if(xmlcount!=0) {	// stream has content
		glog(ZQ::common::Log::L_DEBUG, "ScheduleTV::OnSendAEStatus()  AE status on channel %d updated", chnlID);
		_pTheSMConnector->SendStatusFeedback(xmlbuff);
	}
	return STVSUCCESS;
}

HRESULT ScheduleTV::OnSendStreamStatus(DWORD chnlID, SSStreamNotification status)
{
	// TODO: log
	return STVSUCCESS;
}

HRESULT ScheduleTV::OnStartStream(DWORD ChnlID, const char* schNO, int ieIndex)
{
	int err = STVSUCCESS;
	
	// get channel
	STVChannel* pCh =_pThePlaylistMan->queryCh(ChnlID);
	if(pCh == NULL)
	{
		glog(ZQ::common::Log::L_ERROR, "ScheduleTV::OnStartStream()  Could not find channel %06ld : %s", ChnlID, GetSTVErrorDesc(err));
		return STVINVALIDCHNL;
	}

	// set channel status to starting
	pCh->setStatus(STVChannel::STAT_STARTING);

	// compose AEList
	err = pCh->compose(schNO, ieIndex);
	if(err!=STVSUCCESS)	{
		glog(ZQ::common::Log::L_ERROR, "ScheduleTV::OnStartStream()  Cound not compose AE list: %s", GetSTVErrorDesc(err));
		return err;
	}
	
	return _pStreamUnit->OnStartStream(ChnlID, schNO, ieIndex);
}

HRESULT ScheduleTV::OnStopStream(DWORD ChnlID)
{
	// get channel
	STVChannel* pCh =_pThePlaylistMan->queryCh(ChnlID);
	if(pCh == NULL)
	{
		glog(ZQ::common::Log::L_ERROR, "ScheduleTV::OnStopStream()  Could not find channel %06ld", ChnlID);
		return STVINVALIDCHNL;
	}

	// set channel status to starting
	pCh->setStatus(STVChannel::STAT_STOPPING);
	
	return _pStreamUnit->OnStopStream(ChnlID);
}

HRESULT ScheduleTV::OnClose()
{
	try {
	
		_pThePlaylistMan->getTimer()->OnTerminate();
		
		_pTheRtspMan->terminate();
		
		_pThePlaylistMan->terminate();
		
		_pTheSMConnector->close();
		
		_pTheSSMan->UnInitialize();
	}
	catch(ZQ::common::Exception excep) {
		glog(ZQ::common::Log::L_WARNING, L"NOTIFY  Encountered some sort of error during termination");
	}
	catch(...) {
		glog(ZQ::common::Log::L_WARNING, "NOTIFY  Encountered some sort of error during termination");
		return STVERROR;
	}
	
	_bClosed  = TRUE;
	glog(ZQ::common::Log::L_INFO, "********** ScheduleTV ITV-Playback terminated **********");
	return STVSUCCESS;
}

HRESULT ScheduleTV::OnUnInit()
{

	if(_pThePlaylistMan)
		delete _pThePlaylistMan;
	if(_pTheRtspMan)
		delete _pTheRtspMan;
	if(_pTheSMConnector)
		delete _pTheSMConnector;

	if(_pTheSSMan)
		CStreamSession::FreeInstance();

	if(_pStreamUnit)
		delete _pStreamUnit;
	
	if(pGSdTrace)
	{
		delete pGSdTrace;
		pGSdTrace = NULL;
	}
	
	glog(ZQ::common::Log::L_INFO, "********** ScheduleTV ITV-Playback uninitialization done **********");
	CoUninitialize();
	return STVSUCCESS;
}