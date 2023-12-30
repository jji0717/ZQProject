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
// Ident : $Id: ScheduleTV.h$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : declaration of STV main control
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/MAINCTRL/ScheduleTV.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 15    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 14    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 13    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 12    04-10-26 16:09 Bernie.zhao
// 0.1.6 Oct/26
// 
// 11    04-10-19 17:21 Bernie.zhao
// for QA release
// 
// 10    04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 9     04-10-15 16:36 Bernie.zhao
// 
// 8     04-10-15 16:18 Bernie.zhao
// 
// 6     04-10-14 14:56 Bernie.zhao
// 
// 5     04-10-14 11:51 Kaliven.lee
// 
// 4     10/12/04 2:05p Jie.zhang
// 
// 3     10/11/04 7:24p Jie.zhang
// 
// 2     04-10-11 15:19 Bernie.zhao
// 
// 1     04-10-07 9:09 Bernie.zhao
// ===========================================================================

///@mainpage Scheduled ITV: Schedule TV Main Control 
///@section Background
/// The Scheduled TV(STV) system is intended to deliver uninterrupted linear scheduled streams in ITV VOD configurations. It extends and enhances the ITV platform by delivering the capabilities and functions most commonly found in Near Video On Demand (NVOD) systems and, provides an enhancement of the current SeaChange VOD Barker system. 
/// A schedule in STV system, represented by a series of streaming assets, is logically defined as a playlist.  A single playlist contains asset elements that being delivered to subscribers in a certain span of time.  The time duration varies from hours to months, basically depending on practical server requirement and customers' plan.
/// Dozens of playlists constitute a single channel, while the STV service maintains control of different channels.  This structure gives birth to a manager-functioned logical unit for all playlists, the playlist management.
///@section Purpose
///The STV Main Control works as a central manager of the STV service. It controls all other modules such as Playlist&PM, SM Interface, ISS Manager... etc.

#if !defined(AFX_SCHEDULETV_H__DE08CA9E_66C7_4A45_BBE4_A3A8FF711589__INCLUDED_)
#define AFX_SCHEDULETV_H__DE08CA9E_66C7_4A45_BBE4_A3A8FF711589__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// common include
#include "../STVMainHeaders.h"

// other module include
#include "../PlaylistMod/STVPlaylistManager.h"
#include "../RtspClient/RtspConnectionManager.h"
#include "../SMInterface/SMConnector.h"
#include "../IssStreamCtrl/StreamSession.h"

// local include
#include "STVStreamUnit.h"
#include "IPreference.h"


using namespace ZQ::common;

class ScheduleTV
{
public:
	ScheduleTV();
	~ScheduleTV();
public:
	// reserve for SeaChange Service
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	HRESULT	OnStart(void);
	HRESULT OnClose(void);

///@name Module reference methods
//@{ 
	
public:
	STVPlaylistManager*		getPlayListMan() { return _pThePlaylistMan; }
	RtspConnectionManager*	getRtspMan() { return _pTheRtspMan; }
	CSMConnector*			getSMConnector() { return _pTheSMConnector; }

	STVStreamUnit*			getStreamUnit() { return _pStreamUnit; }

//@}

///@name Interfaces
//@{
public:
	////////////////////////////////// Interface Transfer functions ////////////////////////////////////////
	
	///@name Interfaces for SM
	//@{
	/// create a new playlist according to information provided, and register it
	///@param[in] scheduleNO	the schedule number of this list
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@param[in] nType			the type of playlist, one among LISTTYPE_BAKKER, LISTYPE_PLAYLIST, LISTYPE_FILLER
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnNewPlayList(const char* scheduleNO, ZQ::common::IPreference*  pPortPref, ZQ::common::IPreference*  pPlayListPref, int nType);

	/// play specific asset immediately
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@param[in] pTimePref		the pointer to an XML structure containing info about time shift
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnPlayAssetImmediately (ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref, ZQ::common::IPreference* pTimePref);
	
	/// stop current asset playing and play filler immediately
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnPlayFillerImmediately (ZQ::common::IPreference *pPortPref);
	
	/// play specific asset after current asset finishes
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnPlaySkipToAsset (ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref);

	/// hold the next asset and play Filler until the command of play specific asset to un-hold it
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnHoldAsset (ZQ::common::IPreference *pPortPref, ZQ::common::IPreference* pAssetPref);

	/// resume a channel
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnChannelStartup(ZQ::common::IPreference* pPortPref);

	/// shutdown a specific channel
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnChannelShutdown(ZQ::common::IPreference* pPortPref);

	/// Send channel configuration
	///@param[in]	pConfigPref		the xml structure containing channel configuration information
	///@return						error code, 0 indicate STVSUCCESS
	HRESULT OnConfigration(IPreference* pConfigPref);

	/// to send a schedule enquiry message
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnConnected();

	//@}

	///@name Interfaces for Playlist&PM
	//@{	
	/// Start a new stream with specific playlist
	///@param[in] ChnlID	the ID of the channel that should start
	///@param[in] schNO		the schedule number of the list need start
	///@param[in] ieIndex	the index of the ie in the list that need start
	///@return				error code; STVSUCCESS if create successfully
	HRESULT OnStartStream(DWORD ChnlID, const char* schNO, int ieIndex);

	/// Kill an existed stream with specific playlist
	///@param[in]	ChnlID			the ID of the channel that need stop stream
	///@return						error code, 0 indicate STVSUCCESS
	HRESULT OnStopStream(DWORD ChnlID);

	//@}

	///@name Interfaces for ISS
	//@{
	/// Get AElist of specified channel, for modification of asset elements use
	///@param[in]	chnlID			the ID of the specified channel
	///@param[out]	OutAEList		the AELIST object to fill
	///@return						error code, 0 indicate STVSUCCESS
	HRESULT OnGetAElist(DWORD chnlID, AELIST& OutAEList);

	/// Free Asset Element array in AElist after the call of OnGetAElist()
	///@param[in]	chnlID			the ID of the specified channel
	///@return						error code, 0 indicate STVSUCCESS
	HRESULT OnFreeAElist(DWORD chnlID);

	/// Send asset elements status for the specified channel feedback
	///@param[in]	chnlID			the ID of the specified channel
	///@param[in]	status			the asset elements status to feedback
	///@return						error code, 0 indicate STVSUCCESS
	HRESULT OnSendAEStatus(DWORD chnlID, SSAENotification& status);

	/// Send stream status for the specified channel feedback
	///@param[in]	chnlID			the ID of the specified channel
	///@param[in]	status			the stream status to feedback
	///@return						error code, 0 indicate STVSUCCESS
	HRESULT OnSendStreamStatus(DWORD chnlID, SSStreamNotification status);
	//@}

	///@name Interfaces for RTSP Manager
	//@{
	
	//@}

//@}

	///////////////////////////////////// main ctrl status /////////////////////////////////////
private:
	bool _bClosed;

public:
	wchar_t	_wcsVer[16];

	///////////////////////////////////// all module references /////////////////////////////////////
private:
	STVPlaylistManager*			_pThePlaylistMan;
	RtspConnectionManager*		_pTheRtspMan;
	CSMConnector*				_pTheSMConnector;
	CStreamSession*				_pTheSSMan;

	STVStreamUnit*				_pStreamUnit;


	///////////////////////////////////// statistic attributes for manutil /////////////////////////////////////
	// all the member variable which is started with "m_" must be static variable, so that the manutil can monitor the process
public:
	//log Vars
	wchar_t		m_wszLogPath[MAX_PATH];

	// SM vars
	wchar_t		m_wszBindIP[32];
	wchar_t		m_wszSMServerIP[32];
	DWORD		m_dwSMPort;
	DWORD		m_dwClientID;
	DWORD		m_dwTrace;
	wchar_t		m_wszTraceFile[MAX_PATH];

	// P&PM vars
	wchar_t		m_wszMirrorPath[MAX_PATH];
	DWORD		m_dwFillLength;
	DWORD		m_dwMaxSubChannel;
	DWORD		m_dwLastChanceNPT;
	DWORD		m_dwPenaltyIncrRate;

	// Rtsp vars
	wchar_t		m_wszRtspHostIP[32];
	DWORD		m_dwRtspHostPort;
	DWORD		m_dwListenerFrequence;
	DWORD		m_dwRtspNsec;
	wchar_t		m_wszRtspURL[MAX_URL_LEN];
	DWORD		m_dwRtspTrace;
	
	// ISS vars
	DWORD		m_dwAppUID;
	DWORD		m_dwType;
	DWORD		m_dwInst;
};

#endif // !defined(AFX_SCHEDULETV_H__DE08CA9E_66C7_4A45_BBE4_A3A8FF711589__INCLUDED_)
