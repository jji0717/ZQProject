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
// Name  : PlaylistAppServ.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-31
// Desc  : impl for class PlaylistAppServ
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTAS/PlaylistAppServ.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 5     05-04-19 20:38 Bernie.zhao
// 
// 4     05-04-19 17:27 Bernie.zhao
// 
// 3     05-03-29 15:40 Kaliven.lee
// 
// 2     05-03-24 16:50 Bernie.zhao
// added soap call when acceptrdyasset failed
// 
// 1     05-02-16 11:56 Bernie.zhao
// ===========================================================================
//#ifndef ITV_TYPE_PRIMARY_ZQ_MUSICPLAYLIST
//#define	ITV_TYPE_PRIMARY_ZQ_MUSICPLAYLIST	0
//#endif
#include "PlaylistAppServ.h"
#include "ZQResource.h"

// global definition
PlaylistAppServ gPlaylistAS;
BaseSchangeServiceApplication *Application = &gPlaylistAS;
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

// static members initialization
bool		PlaylistAppServ::m_bRunning;
wchar_t		PlaylistAppServ::m_wszVerInfo[64];

DWORD		PlaylistAppServ::m_dwAppUID;
DWORD		PlaylistAppServ::m_dwType;
DWORD		PlaylistAppServ::m_dwInst;
DWORD		PlaylistAppServ::m_dwSoapTimeout;

wchar_t		PlaylistAppServ::m_wszSoapWSDLFilePath[MAX_PATH];
wchar_t		PlaylistAppServ::m_wszSoapWSMLFilePath[MAX_PATH];
wchar_t		PlaylistAppServ::m_wszSoapServiceName[MAX_PATH];
wchar_t		PlaylistAppServ::m_wszSoapPort[MAX_PATH];
wchar_t		PlaylistAppServ::m_wszSoapNamespace[MAX_PATH];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PlaylistAppServ::PlaylistAppServ()
{
	_pTheSSMan = NULL;

	// init static members
	m_bRunning = FALSE;
	wcscpy(m_wszVerInfo, _T(ZQ_PRODUCT_VER_STR3));

	m_dwAppUID		= 0x80002;
	m_dwType		= ITV_TYPE_PRIMARY_ZQ_MUSICPLAYLIST;
	m_dwInst		= 0x1;
	m_dwSoapTimeout	= DEFAULT_TIMEOUT;

	wcscpy(m_wszSoapWSDLFilePath, L".\\PlaylistSoapInterface.wsdl");
	wcscpy(m_wszSoapWSMLFilePath, L".\\PlaylistSoapInterface.wsml");
	wcscpy(m_wszSoapServiceName, L"PlaylistSoapInterfaceService");
	wcscpy(m_wszSoapPort, L"PlaylistSoapInterface");
	wcscpy(m_wszSoapNamespace, L"http://192.168.80.92:8080/telewest/services/PlaylistSoapInterface");
}

PlaylistAppServ::~PlaylistAppServ()
{
}

HRESULT PlaylistAppServ::OnInit()
{
	HRESULT hr;

	try
	{
		//////////////////////////////////////////////////////////////////////////
		// log init
		hr = BaseSchangeServiceApplication::OnInit();
		pGlog = m_pReporter;
		//m_pReporter->setReportLevel(ALL_LOGS, Log::L_NOTICE);

		//////////////////////////////////////////////////////////////////////////
		// get registry config
		DWORD dwNum=0;
		DWORD dwError = 0;
		manageVar(L"Version", MAN_STR, (DWORD)&m_wszVerInfo, TRUE, &dwError);
		manageVar(L"IsRunning", MAN_BOOL, (DWORD)&m_bRunning, TRUE, &dwError);

		getConfigValue(L"ISSAppUID", &m_dwAppUID, m_dwAppUID, true, true);
		getConfigValue(L"ISSInst", &m_dwInst, m_dwInst, true, true);
		getConfigValue(L"SoapTimeout", &m_dwSoapTimeout, m_dwSoapTimeout, true, true);
			
		dwNum = MAX_PATH*2;
		getConfigValue(L"SoapWSDLFilePath", m_wszSoapWSDLFilePath, m_wszSoapWSDLFilePath, &dwNum, true, true);
		dwNum = MAX_PATH*2;
		getConfigValue(L"SoapWSMLFilePath", m_wszSoapWSMLFilePath, m_wszSoapWSMLFilePath, &dwNum, true, true);
		dwNum = MAX_PATH*2;
		getConfigValue(L"SoapServiceName", m_wszSoapServiceName, m_wszSoapServiceName, &dwNum, true, true);
		dwNum = MAX_PATH*2;
		getConfigValue(L"SoapPort", m_wszSoapPort, m_wszSoapPort, &dwNum, true, true);
		dwNum = MAX_PATH*2;
		getConfigValue(L"SoapTargetNamespace", m_wszSoapNamespace, m_wszSoapNamespace, &dwNum, true, true);
						 
		glog(Log::L_INFO,L"********************************************************");
		glog(Log::L_INFO,L"*            Music Playlist %s            *", m_wszVerInfo);
		glog(Log::L_INFO,L"********************************************************");
		
		//////////////////////////////////////////////////////////////////////////
		// init ISS
		TYPEINST tmpTypeInst;
		tmpTypeInst.s.dwType = m_dwType;
		tmpTypeInst.s.dwInst = m_dwInst;
		
		_pTheSSMan = CStreamSession::Instance(tmpTypeInst, m_dwAppUID);
		
		_pTheSSMan->SetSoapWSDLFilePath(m_wszSoapWSDLFilePath);
		_pTheSSMan->SetSoapWSMLFilePath(m_wszSoapWSMLFilePath);
		_pTheSSMan->SetSoapServiceName(m_wszSoapServiceName);
		_pTheSSMan->SetSoapPort(m_wszSoapPort);
		_pTheSSMan->SetSoapNamespace(m_wszSoapNamespace);
		_pTheSSMan->SetSoapTimeout(m_dwSoapTimeout);
	}
	catch(...){
		glog(ZQ::common::Log::L_ERROR, "PlaylistAppServ::OnInit: Encountered some sort of error during initialization");
	}

	glog(Log::L_INFO,L"*****             Initialization done              *****");
	
	return S_OK;
}

HRESULT PlaylistAppServ::OnUnInit()
{
	HRESULT hr;
	try
	{
		hr = BaseSchangeServiceApplication::OnUnInit();
		if(m_bRunning)
			OnStop();
	
		m_bRunning = FALSE;

		if(_pTheSSMan) {
			_pTheSSMan->FreeInstance();
			_pTheSSMan = NULL;
		}
	}
	catch(...){
		glog(ZQ::common::Log::L_ERROR, "PlaylistAppServ::OnUnInit: Encountered some sort of error during uninitialization");
	}

	glog(Log::L_INFO,L"*****            Uninitialization done             *****");
	return hr;
}

HRESULT PlaylistAppServ::OnStart()
{
	HRESULT hr;

	try
	{
		hr = BaseSchangeServiceApplication::OnStart();
		if(!_pTheSSMan->Initialize()) {
			return E_FAIL;
		}
		m_bRunning = TRUE;
		glog(Log::L_DEBUG,L"PlaylistAppServ::OnStart: Session Manager started");
	}
	catch(...){
		glog(ZQ::common::Log::L_ERROR, "PlaylistAppServ::OnStart: Encountered some sort of error during start");
	}
	
	return hr;
}

HRESULT PlaylistAppServ::OnStop()
{
	HRESULT hr;

	try
	{
		hr = BaseSchangeServiceApplication::OnStop();
		_pTheSSMan->UnInitialize();
		m_bRunning = FALSE;
		glog(Log::L_DEBUG,L"PlaylistAppServ::OnStart: Session Manager stopped");
	}
	catch(...){
		glog(ZQ::common::Log::L_ERROR, "PlaylistAppServ::OnStop: Encountered some sort of error during stop");
	}
	
	return S_OK;
}