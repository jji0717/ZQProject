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
// Ident : $Id: ScheduleTVServ.h$
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : declaration of STV main control service Shell
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/MainCtrl/ScheduleTVServ.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     04-10-28 18:19 Bernie.zhao
// before trip
// 
// 2     04-10-15 10:49 Kaliven.lee
// Create file
// 
// 1     04-10-15 9:48 Kaliven.lee
// 

// ===========================================================================

///@mainpage Scheduled ITV Service: Schedule TV Main Control Service shell
///@section Background
///main function of this class is to provide SeaChange service shell and insulate 
///the kernel of the STV and service shell
///@section Purpose
///The STV Main Control shell works as a shell which encapsulated ScheduleTV 


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// common include
#include "../STVMainHeaders.h"
#include "baseSchangeServiceApplication.h"
// other module include






using ZQ::common::BaseSchangeServiceApplication;


class ScheduleTVServ  :public ZQ::common::BaseSchangeServiceApplication 
{
public:
	ScheduleTVServ();
	~ScheduleTVServ();
public:
	// reserve for SeaChange Service
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	HRESULT	OnStart(void);
	HRESULT OnStop(void);


	///////////////////////////////////// statistic attributes for manutil /////////////////////////////////////
	// all the member variable which is started with "m_" must be static variable, so that the manutil can monitor the process
public:
	// SM vars
	static wchar_t		m_wszBindIP[32];
	static wchar_t		m_wszSMServerIP[32];
	static DWORD		m_dwSMPort;
	static DWORD		m_dwClientID;

	// P&PM vars
	static wchar_t		m_wszMirrorPath[MAX_PATH];
	static DWORD		m_dwFillLength;
	static DWORD		m_dwMaxSubChannel;

	// Rtsp vars
	static wchar_t		m_wszRtspHostIP[32];
	static DWORD		m_dwRtspHostPort;
	static DWORD		m_dwListenerFrequence;
	static DWORD		m_dwRtspNsec;
	static wchar_t		m_wszRtspURL[MAX_URL_LEN];
	
	// ISS vars
	static DWORD		m_dwAppUID;
	static DWORD		m_dwType;
	static DWORD		m_dwInst;
};
