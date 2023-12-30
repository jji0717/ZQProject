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
// Name  : STVPlaylistManager.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-7-12
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/PLAYLISTMOD/STVPlaylistManager.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// ===========================================================================

///@mainpage Scheduled ITV: Playlist & Playlist Management Module
///@section Background
/// The Scheduled TV(STV) system is intended to deliver uninterrupted linear scheduled streams in ITV VOD configurations. It extends and enhances the ITV platform by delivering the capabilities and functions most commonly found in Near Video On Demand (NVOD) systems and, provides an enhancement of the current SeaChange VOD Barker system. 
/// A schedule in STV system, represented by a series of streaming assets, is logically defined as a playlist.  A single playlist contains asset elements that being delivered to subscribers in a certain span of time.  The time duration varies from hours to months, basically depending on practical server requirement and customers' plan.
/// Dozens of playlists constitute a single channel, while the STV service maintains control of different channels.  This structure gives birth to a manager-functioned logical unit for all playlists, the playlist management.
///@section Purpose
///Playlist and Playlist Management (P&PM) Module works as a central management of the STV service.  It is mainly designed to manage all playlists and invoke corresponding actions on proper time to guarantee the time and resource correctness of the stream distribution.  Playlist mirror back up and crash recovery are also within P&PM Module purpose.
/// 	P&PM module takes SM command and corresponding XML stream as its input.
/// 	P&PM module output (TBD).
/// 	P&PM communicates with Filler&Barker module to fill/confuse assets.
/// 	P&PM mirrors all playlists into database and automatically recovers into last normal status after service restarts.


#if !defined(AFX_STVPLAYLISTMANAGER_H__91FEF100_CA2E_4D2B_B474_BCAEEFD25F5D__INCLUDED_)
#define AFX_STVPLAYLISTMANAGER_H__91FEF100_CA2E_4D2B_B474_BCAEEFD25F5D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// local module include
#include "STVChannel.h"
#include "STVPMTimerMan.h"

#include <map>

/// class STVPlaylistManager monitors and controls all STVChannel objects
class STVPlaylistManager  
{
public:
	//////////////////////////////////// constructor and destructor //////////////////////////////////////
	
	STVPlaylistManager();
	virtual ~STVPlaylistManager();

public:
	//////////////////////////////////// Attributes methods //////////////////////////////////////
	
	/// get timer object
	///@return			the pointer to the timer object
	static STVPMTimerMan*	getTimer() { return _timerMan; }
	
	/// get auto filler confuser
	///@return			the pointer to the auto filler confuser
	static AutoFiller*		getAutoF() { return _autoF; }

	/// get barker filler confuser
	///@return			the pointer to the barker filler confuser
	static BarkerFiller*	getBarkerF() { return _barkerF; }

	/// get fill length
	///@return			the length of filler, set by external main ctrl
	static DWORD			getFillLength() { return _fillLength; }

	/// set fill length
	///@param[in] length	the length of filler
	static void				setFillLength( DWORD length) 	{ _fillLength = length; }

	/// check if is running
	///@return			true if running, false else
	bool					isRunning() { return _isRunning; }

	/// double check to ensure that the timer man is running
	void					checkTimer(); 

public:
	///////////////////////////////////// Manager operations /////////////////////////////////////
	
	/// start manager, can NOT be invoked more than once
	///@return			true if start successfully
	bool			start();
	
	/// terminate manager, can NOT be invoked more than once
	///@return			true if terminate successfully
	bool			terminate();

	/// query for a channel according to channel id
	STVChannel*		queryCh(DWORD chid);

	/// scan all channels and start corresponding list
	///@param[in] keepslow	if it is the first time to start scan, keep it slow to avoid SRM collision
	///@return			how long should timer sleep for next time, in milli-seconds
	DWORD			scan(bool keepslow=false);

public:
	///////////////////////////////////// External interface /////////////////////////////////////
	
	/// create a new playlist according to information provided, and register it
	///@param[in] scheduleNO	the schedule number of this list
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@param[in] nType			the type of playlist, one among LISTTYPE_BAKKER, LISTYPE_PLAYLIST, LISTYPE_FILLER
	///@return				error code; STVSUCCESS if create successfully
	int OnNewPlayList(const char* scheduleNO, ZQ::common::IPreference*  pPortPref, ZQ::common::IPreference*  pPlayListPref, int nType);
 
	/// play specific asset immediately
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@param[in] pTimePref		the pointer to an XML structure containing info about time shift
	///@return				error code; STVSUCCESS if create successfully
	int OnPlayAssetImmediately (ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref, ZQ::common::IPreference* pTimePref);
	
	/// stop current asset playing and play filler immediately
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	int OnPlayFillerImmediately (ZQ::common::IPreference *pPortPref);
	
	/// play specific asset after current asset finishes
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@return				error code; STVSUCCESS if create successfully
	int OnPlaySkipToAsset (ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref);

	/// hold the next asset and play Filler until the command of play specific asset to un-hold it
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	int OnHoldAsset (ZQ::common::IPreference *pPortPref, ZQ::common::IPreference* pAssetPref);

	/// resume a channel
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	int OnChannelStartup(ZQ::common::IPreference* pPortPref);

	/// shutdown a specific channel
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@return				error code; STVSUCCESS if create successfully
	int OnChannelShutdown(ZQ::common::IPreference* pPortPref);

	/// set channel configuration
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] dump			whether dump to disk or not
	///@return				error code; STVSUCCESS if create successfully
	int OnConfiguration(ZQ::common::IPreference* pConfigPref, bool dump=true);

	/// to send a schedule enquiry message
	///@param[out]	backstream		the xml stream of asset element status, which will be sent to SM
	///@param[in,out] backcount		the max count of stream input, and after the call succeeded, the actual count of output stream
	///@return				error code; STVSUCCESS if create successfully
	int OnConnected(char* backstream, DWORD* backcount);

	/// ISS get AElist from Playlist Manager
	///@param[in] chnlID		the ID of the stream that need AELlist (purchase ID in RTSP)
	///@param[out] outAElist	the reference of the AELlist
	///@return					error code; STVSUCCESS if create successfully
	int OnGetAElist(DWORD chnlID, AELIST& OutAEList);

	/// ISS call this function when modify stream is done, to free used PAELEMENT array
	///@param[in] chnlID		the ID of the stream that need AELlist (purchase ID in RTSP)
	///@return					error code; STVSUCCESS if create successfully
	int OnFreeAElist(DWORD chnlID);

	/// ISS set Stream Status feedback to _mapChnl table
	///@param[in] chnlID			the ID of the stream that need AELlist (purchase ID in RTSP)
	///@param[in] notiMsg			the feedback message from ISS about channel
	///@param[out]	backstream		the xml stream of asset element status, which will be sent to SM
	///@param[in,out] backcount		the max count of stream input, and after the call succeeded, the actual count of output stream
	///@return					error code; STVSUCCESS if create successfully
	int OnSetStreamStatus(DWORD chnlID, SSAENotification& notiMsg, char* backstream, DWORD* backcount);

private:
	///////////////////////////////////// channel management /////////////////////////////////////
	
	/// create and register a new channel into the pool
	bool			regCh(DWORD chid, const char* ip, const char* port, const char* nodegroup, const char* portid);

	/// unregister and delete the channel from pool
	bool			unregCh(DWORD chid);

	/// start channel
	int				startCh(DWORD chid);

	/// stop channel
	int				stopCh(DWORD chid);
	
	/// get channel ID according to the port info
	///@param[in]	chnlPref	the channel info
	///@return		the channel ID, 0 if chnlPref is NULL, or format error
	DWORD			getChnlID(ZQ::common::IPreference* chnlPref);

private:
	/// compose ISS AElist map pure playlist
	///@param[in]  chid		channel ID
	///@param[out] pAE		pointer to AELIST
	///@param[in]  pASp		pointer to ASSETS containing playlist AE
	///@return		ture if compose successfully
	bool			composeAE(DWORD chid, PAELIST pAE, PASSETS pASp);

	/// compose ISS AElist map for pure barker and filler list
	///@param[in]  chid		channel ID
	///@param[out] pAE		pointer to AELIST
	///@param[in] filledlist	pointer to filled barker pointer vector
	///@return		ture if compose successfully
	bool			composeAE(DWORD chid, PAELIST pAE, PASSETLIST* filledlist);

public:
	/// compare two SYSTEMTIME object
	///@param[in] t1	the time to compare with
	///@param[in] t2	the time to compare with
	///@return			-1 if t1 earlier than t2, 0 if equal, 1 if t1 later than t2
	static int		timeComp(SYSTEMTIME t1, SYSTEMTIME t2);

	/// subtract two SYSTEMTIME
	///@param[in] t1	the minuend
	///@param[in] t2	the subtrahend
	///@return			FFFFFFFF	-	if t1 earlier than t2 or the difference is greater than 1 year \n
	///                 seconds		-	if t1 later than t2, in term of seconds
	/// @remark			better call timeComp() first
	static DWORD	timeSub(SYSTEMTIME t1, SYSTEMTIME t2);

public:
	/// Com init class
	static ZQ::common::ComInitializer	_coInit;

private:
	///////////////////////////////////// Related objects /////////////////////////////////////
	
	/// timer man thread for handling lists time
	static STVPMTimerMan*				_timerMan;

	//////////////////////////////////// Filler attributes //////////////////////////////////////
	
	/// AutoFiller object pointer
	static AutoFiller*					_autoF;

	/// Barker object pointer
	static BarkerFiller*				_barkerF;

	/// Fill length for filler/barker
	static DWORD						_fillLength;

	//////////////////////////////////// Manager status //////////////////////////////////////
	
	/// manager status
	bool	_isRunning;

	/// indicate whether configuration file exist
	bool	_hasConfig;

	//////////////////////////////////// channel pool //////////////////////////////////////

	/// channel pool
	std::map<DWORD, STVChannel*>	_channelPool;

	/// mutex for channel pool
	ZQ::common::Mutex				_channelMutex;

	//////////////////////////////////// configure setting //////////////////////////////////////
	
	/// configuration file name
	std::string						_configName;

	/// mutex for configuration file
	ZQ::common::Mutex				_configMutex;
};

#endif // !defined(AFX_STVPLAYLISTMANAGER_H__91FEF100_CA2E_4D2B_B474_BCAEEFD25F5D__INCLUDED_)
