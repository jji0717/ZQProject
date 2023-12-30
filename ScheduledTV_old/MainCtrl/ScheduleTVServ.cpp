   // ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: ScheduleTVServ.cpp$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : implementation of STV main control service shell
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/MainCtrl/ScheduleTVServ.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 12    05-04-19 21:18 Bernie.zhao
// autobuild modification
// 
// 11    05-03-24 14:53 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 10    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 9     05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 8     04-12-16 14:55 Bernie.zhao
// 
// 7     04-12-01 17:37 Ken.qian
// 
// 6     04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 5     04-11-23 10:01 Bernie.zhao
// 
// 4     04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 3     04-10-28 18:19 Bernie.zhao
// before trip
// 
// 2     04-10-15 10:49 Kaliven.lee
// create file

// ===========================================================================
#include "ScheduleTVServ.h"
#include "ScheduleTV.h"
#include "Log.h"
#include "ScLog.h"
#include "comextra/ZqStringConv.h"
#include "ZQResource.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using ZQ::common::BaseSchangeServiceApplication;

ScheduleTV gSTV;
ScheduleTVServ gSTVServer;
BaseSchangeServiceApplication *Application = &gSTVServer;
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
//ZQ::common::Log * pGlog;

// default config
DWORD ScheduleTVServ::m_dwSMPort			= 4444;
DWORD ScheduleTVServ::m_dwClientID			= 1;
DWORD ScheduleTVServ::m_dwRtspNsec			= 5000;
DWORD ScheduleTVServ::m_dwRtspHostPort		= 554;
DWORD ScheduleTVServ::m_dwListenerFrequence = 500;
DWORD ScheduleTVServ::m_dwAppUID			= 0x80002;
DWORD ScheduleTVServ::m_dwType				= ITV_TYPE_PRIMARY_ZQ_SCHEDULEDTV;
DWORD ScheduleTVServ::m_dwInst				= 1;
DWORD ScheduleTVServ::m_dwFillLength		= DEFAULT_FILLLENGTH;
DWORD ScheduleTVServ::m_dwMaxSubChannel		= MAX_SUBCHNL;

wchar_t ScheduleTVServ::m_wszBindIP []		= L"";
wchar_t ScheduleTVServ::m_wszMirrorPath []	= L"D:\\itv\\ITVPLAYBACK\\";
wchar_t ScheduleTVServ::m_wszRtspHostIP []	= L"";
wchar_t ScheduleTVServ::m_wszRtspURL []		= L"";
wchar_t ScheduleTVServ::m_wszSMServerIP []	= L"";

ScheduleTVServ::ScheduleTVServ():
BaseSchangeServiceApplication()
{
	// initialize your variable
	wcscpy(gSTV._wcsVer, _T(ZQ_PRODUCT_VER_STR3));
}
ScheduleTVServ::~ScheduleTVServ()
{
	
}
HRESULT ScheduleTVServ::OnInit()
{
	// log 
	BaseSchangeServiceApplication::OnInit();
	pGlog = m_pReporter;
// read variable from the register

	// init SM config
	DWORD dwNum = 32*2; 
	getConfigValue(L"SMBindIP",m_wszBindIP,m_wszBindIP,&dwNum,true,true);
	dwNum = 32*2; 
	getConfigValue(L"SMServerIP",m_wszSMServerIP,m_wszSMServerIP,&dwNum,true,true);
	getConfigValue(L"SMClientID",&m_dwClientID,m_dwClientID,true,true);
	getConfigValue(L"SMBindPort",&m_dwSMPort,m_dwSMPort,true,true);
	
	wcscpy(gSTV.m_wszSMServerIP,m_wszSMServerIP);
	wcscpy(gSTV.m_wszBindIP,m_wszBindIP);
	gSTV.m_dwClientID = m_dwClientID;
	gSTV.m_dwSMPort= m_dwSMPort;
	
	// init PM config
	dwNum = MaxPath*2; 
	getConfigValue(L"PMMirrorPath",m_wszMirrorPath,m_wszMirrorPath,&dwNum,true,true);
	getConfigValue(L"PMFillLength",&m_dwFillLength, m_dwFillLength, true, true);
	getConfigValue(L"PMMaxSubChannel",&m_dwMaxSubChannel, m_dwMaxSubChannel, true, true);
	
	wcscpy(gSTV.m_wszMirrorPath,m_wszMirrorPath);
	gSTV.m_dwFillLength = m_dwFillLength;
	gSTV.m_dwMaxSubChannel = m_dwMaxSubChannel;
	
	// init RTSP config
	dwNum = 32*2;
	getConfigValue(L"RTSPHostIP",m_wszRtspHostIP,m_wszRtspHostIP,&dwNum,true,true);
	getConfigValue(L"RTSPHostPort",&m_dwRtspHostPort,m_dwRtspHostPort,true,true);
	getConfigValue(L"RTSPFreq",&m_dwListenerFrequence,m_dwListenerFrequence,true,true);
	getConfigValue(L"RTSPNsec",&m_dwRtspNsec,m_dwRtspNsec,true,true);
	dwNum = MAX_URL_LEN*2;
	getConfigValue(L"RTSPURL",m_wszRtspURL,m_wszRtspURL,&dwNum,true,true);

	wcscpy(gSTV.m_wszRtspHostIP,m_wszRtspHostIP);
	gSTV.m_dwRtspHostPort = m_dwRtspHostPort;
	gSTV.m_dwListenerFrequence = m_dwListenerFrequence;
	gSTV.m_dwRtspNsec = m_dwRtspNsec;
	wcscpy(gSTV.m_wszRtspURL,m_wszRtspURL);
	
		
	// init ISS config
	getConfigValue(L"ISSType",&m_dwType,m_dwType,true,true);
	getConfigValue(L"ISSInst",&m_dwInst,m_dwInst,true,true);
	getConfigValue(L"ISSAppUID",&m_dwAppUID,m_dwAppUID,true,true);
	
	gSTV.m_dwType = m_dwType;
	gSTV.m_dwInst = m_dwInst;
	gSTV.m_dwAppUID = m_dwAppUID;
		
	gSTV.OnInit();

	// TODO: this sleep is for ISS initialize & SRM server time
	::Sleep(5000);
	return STVSUCCESS;

}
HRESULT ScheduleTVServ::OnStart()
{
	// log 
	// start SM connector
	HRESULT hr = BaseSchangeServiceApplication::OnStart();
	gSTV.OnStart();	
	return hr;

}


HRESULT ScheduleTVServ::OnStop()
{
	// what is the behave of the stream when service close?
	HRESULT hr = BaseSchangeServiceApplication::OnStop() ;
	
	// close all modules
	gSTV.OnClose();
	
	return STVSUCCESS;
}

HRESULT ScheduleTVServ::OnUnInit()
{
	HRESULT hr =BaseSchangeServiceApplication::OnUnInit();

	// uninitialize modules
	gSTV.OnUnInit();	
	
	return STVSUCCESS;
}