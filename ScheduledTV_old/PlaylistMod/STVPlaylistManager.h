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
// Ident : $Id: STVPlaylist.h$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : declaration of STV playlist
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/PlaylistMod/STVPlaylistManager.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 26    05-06-27 18:04 Bernie.zhao
// June.27/2005.  Fixed interface error with SM when creating new STV list
// 
// 25    05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 24    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 23    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 22    04-12-16 14:55 Bernie.zhao
// 
// 21    04-12-06 11:46 Bernie.zhao
// 
// 20    04-11-23 10:02 Bernie.zhao
// 
// 19    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 18    04-10-26 16:08 Bernie.zhao
// 0.1.6 Oct/26
// 
// 17    04-10-25 15:26 Bernie.zhao
// added status report
// 
// 16    04-10-24 19:04 Bernie.zhao
// end of 2004/Oct/24
// 
// 15    04-10-24 11:43 Bernie.zhao
// before 24/Oct
// 
// 14    04-10-18 18:46 Bernie.zhao
// 
// 13    04-10-18 10:33 Bernie.zhao
// test 1 ok
// 
// 12    04-10-15 16:14 Bernie.zhao
// 
// 11    04-10-14 11:04 Bernie.zhao
// node missing pro fixed
// 
// 10    04-10-13 15:37 Bernie.zhao
// fixed top&bottom asset error
// 
// 9     04-10-12 16:56 Bernie.zhao
// 
// 8     04-10-12 14:04 Bernie.zhao
// added log
// 
// 7     04-10-11 20:43 Bernie.zhao
// leak pro fixed?? at least part of
// 
// 6     10/11/04 7:43p Jie.zhang
// 
// 
// 5     04-10-11 19:28 Bernie.zhao
// 
// 4     04-10-11 15:20 Bernie.zhao
// mem problem?
// 
// 3     04-09-30 14:32 Bernie.zhao
// before National Season
// 
// 2     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 1     04-09-16 11:23 Bernie.zhao
// Playlist and Mgm module
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
/// 	P&PM calls IDS API for asset element information.
/// 	P&PM mirrors all playlists into database and automatically recovers into last normal status after service restarts.


#if !defined(AFX_STVPLAYLISTMANAGER_H__91FEF100_CA2E_4D2B_B474_BCAEEFD25F5D__INCLUDED_)
#define AFX_STVPLAYLISTMANAGER_H__91FEF100_CA2E_4D2B_B474_BCAEEFD25F5D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable: 4786)
#include <map>

// local module include
#include "STVPlaylist.h"
#include "STVPMTimerMan.h"

#include "Locks.h"
using namespace ZQ::common;

#define DB_FILE_EXT			"iel"
#define DB_CONF_NAME		"config.cfg"
//#define DB_FILE_NAMELENGTH	17

#define	MAX_ASSETNO		65535
#define	DEFAULT_FILLLENGTH	3600			// 1 hour

#define DEFAULT_OUTOFDATE_TIME	86400		// 1 day

#define DEFAULT_LAST_CHANCE_NPT	20			// if a stream has less than this value left, will not start it from mid point

#define UNKNOWN_UID		0xFFFFFFFF

typedef struct _ChnlInfo {
	bool	validation;		// whether channel is valid
	int		status;			// channel status
	DWORD	nodegroup;		// channel destination nodegroup
	char	ipaddr[32];		// channel destination ip address
	DWORD	ipport;			// channel destination ip port
	SSAENotification	StreamStatus;	// channel ISS status feedback
	STVPlaylist*	pCurrList;	// current playing list
} ChannelInfo;

typedef struct _GlobalAElist {
	STVPlaylist*	pGFiller;	// pointer to the global filler
	AELIST			gAE;		// global filler AE list
} GlobalAElist;

/// class STVPlaylistManager monitors and controls all STVPlaylist objects
//##ModelId=4147B84B0157
class STVPlaylistManager  
{
protected:
	friend class STVPMTimerMan;
	friend class STVPlaylist;
	///////////////////////////////////// Related objects /////////////////////////////////////
	
	/// timer man thread for handling lists time
	//##ModelId=4147B84B01C6
	STVPMTimerMan*		_TimerMan;

	/// pool mutex between two threads
	//##ModelId=4147B84B01B6
	ZQ::common::Mutex*			_ListMutex;

	/// IdsSession object pointer
//	IdsSession*		_Builder;

	//////////////////////////////////// Filler attributes //////////////////////////////////////
	
	/// AutoFiller object pointer
	AutoFiller*		_autoF;

	/// Barker object pointer
	BarkerFiller*	_barkerF;

	/// pointer to Com init class
	//##ModelId=4147B84B031D
	ZQ::common::ComInitializer* _dbinit;
	
	/// Fill length for filler/barker
	DWORD	_FillLength;

	//////////////////////////////////// Manager status //////////////////////////////////////
	
	/// manager status
	//##ModelId=4147B84B0167
	bool	_IsRunning;

	/// indicate whether configuration file exist
	bool	_HasConfig;

	/// DB mirror files path
	//##ModelId=4147B84B01D5
	std::wstring		_DBpath;

	/// top asset number
	DWORD	_TopAsset;

	/// max sub channel count
	DWORD	_MaxSubChnl;

	//////////////////////////////////// Playlist pools //////////////////////////////////////
	
	/// pool containing playlists under control
	//##ModelId=4147B84B01A6
	std::vector<STVPlaylist*>	_ListPool;
	
	/// pool containing barker set
	std::vector<STVPlaylist*>	_BarkerPool;

	/// pool containing filler set
	std::vector<STVPlaylist*>	_SetPool;

	/// pool containing global filler
	std::vector<STVPlaylist*>	_GSetPool;

	//////////////////////////////////// ISS maps //////////////////////////////////////
	/// mutex for map of AELIST
	ZQ::common::Mutex	_mapAELISTMutex;

	/// map of AELIST from IDS, for future ISS usage
	std::map<DWORD, AELIST>		_mapAELIST;

	/// mutex for global filler
	ZQ::common::Mutex	_gAEListMutex;
	
	/// AELIST for global filler
	GlobalAElist _gAEList;

	/// mutex for status report
	ZQ::common::Mutex		_mapChnlMutex;

	/// map of status report from ISS, for future SM Interface usage
	std::map<DWORD, ChannelInfo>		_mapChnl;

public:
	//////////////////////////////////// Attributes methods //////////////////////////////////////
	
	/// get com initializer object
	///@return			the com init object pointer
	ZQ::common::ComInitializer* getComInit() { return _dbinit; }
	
	/// get timer object
	///@return			the timer object ponter
	STVPMTimerMan* getTimer() { return _TimerMan; }
	
	/// set the Database path
	///@param[in] path	the absolute directory path of the database
	///@remarks		if not call this function, the path will be set to DB_DEFAULT_PATH
	inline void	setDBpath(std::wstring path)
	{
		_DBpath = path;
		if(_DBpath[_DBpath.length()-1]!='\\')
			_DBpath=_DBpath+L"\\";
		char buffpath[MAX_PATH];
		DWORD dwCount = MAX_PATH;
		wcstombs(buffpath, _DBpath.c_str(), dwCount);
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::setDBpath()  Database mirror path set to %s", buffpath);
	}

	/// get the Database path
	///@return		the absolute directory path of the database
	inline std::wstring getDBpath(){ return _DBpath; }

	/// get fill length
	///@return		the length of filler, set by external main ctrl
	DWORD getFillLength() { return _FillLength; }

	/// set fill length
	///@param[in] length	the length of filler
	void setFillLength( DWORD length) 
	{ 
		_FillLength = length; 
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::setFillLength()  Fill Length set to %d", _FillLength);
	}
	
	/// get max sub channel number
	///@return		max sub channel number
	DWORD getMaxSubChnl() { return _MaxSubChnl; }

	/// set max sub channel number
	///@param[in] maxchnl		the number of max sub channel
	void setMaxSubChnl( DWORD maxchnl )
	{
		_MaxSubChnl = maxchnl;
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  STVPlaylistManager::setMaxChnl()  Max Sub-Channel number set to %d", _MaxSubChnl);
	}

	/// get last sent AE notification of specified channel
	///@param[in] purchaseID	the purchaseID related with the channel
	///@param[out] retNoti		the output notification
	///@return		success or not
	bool getLastNoti(DWORD purchaseID, SSAENotification& retNoti);

	/// get channel status
	///@param[in] chnlID		the id of the channel
	///@return					the status
	int	getChnlState(DWORD chnlID)
	{
		std::map<DWORD, ChannelInfo>::const_iterator iter = _mapChnl.find(chnlID);
		if(iter == _mapChnl.end())
			return CHNL_NONE;

		return iter->second.status;
	}

	/// set channel status
	///@param[in] chnlID		the id of the channel
	///@param[in] status		the state to set
	void setChnlState(DWORD chnlID, int status)
	{
		std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(chnlID);
		if(iter == _mapChnl.end())
			return;

		iter->second.status = status;
	}

	/// get channel current list
	///@param[in] chnlID		the id of the channel
	///@return					the pointer to the list
	STVPlaylist* getChnlCurrList(DWORD chnlID)
	{
		std::map<DWORD, ChannelInfo>::const_iterator iter = _mapChnl.find(chnlID);
		if(iter == _mapChnl.end())
			return NULL;

		return iter->second.pCurrList;
	}

	/// set channel current list
	///@param[in] chnlID		the id of the channel
	///@param[in] pList			the pointer to the playing list
	void	setChnlCurrList(DWORD chnlID, STVPlaylist* pList)
	{
		std::map<DWORD, ChannelInfo>::iterator iter = _mapChnl.find(chnlID);
		if(iter == _mapChnl.end())
			return;

		iter->second.pCurrList = pList;
	}

	/// get current global filler set
	///@return			the pointer to the global filler set
	STVPlaylist* getCurrGlobalFiller()
	{
		return _gAEList.pGFiller;
	}
	
public:
	//////////////////////////////////// constructor and destructor //////////////////////////////////////
	
	//##ModelId=4147B84B01E4
	STVPlaylistManager();
	//##ModelId=4147B84B01F4
	virtual ~STVPlaylistManager();

public:
	///////////////////////////////////// Manager operations /////////////////////////////////////
	
	/// start manager, can NOT be invoked more than once
	/// @return			true if start successfully

	//##ModelId=4147B84B01F6
	bool start();
	
	/// terminate manager, can NOT be invoked more than once
	/// @return			true if terminate successfully

	//##ModelId=4147B84B01F7
	bool terminate();

public:
	///////////////////////////////////// Playlist management /////////////////////////////////////
	
	/// create a new playlist (maybe from a DB mirror file)
	/// @param mirrorfile	the DB mirror file used to create, including full path
	/// @return			pointer to new playlist, or NULL if failed

	//##ModelId=4147B84B0203
	STVPlaylist* createList(const wchar_t* mirrorfile = NULL);

	/// register the playlist into the pool
	/// @param newlist	the playlist to register
	/// @return			true if register successfully

	//##ModelId=4147B84B0213
	bool reg(STVPlaylist* newlist);

	/// unregister the playlist from pool
	/// @param exlist	the playlist to unregister
	/// @return			true if unregister successfully

	//##ModelId=4147B84B0222
	bool unreg(STVPlaylist* exlist);
	
	/// get channel ID according to the port info
	///@param[in] chnlPref	the channel info
	///@return		the channel ID
	DWORD getChnlID(ZQ::common::IPreference* chnlPref);

	/// query playlist by channel information
	/// @param[in]	deviceid	the nodegroup id of the query channel
	/// @param[in]	ipaddr	the IP address of the query channel
	/// @param[in]	ipport	the IP port of the query channel
	/// @param[out]	outlist	the list containing qualified playlists
	///@param[in]	listtype	the type of list, LIST_PLAYLIST, LIST_BARKER or LIST_FILLER
	/// @return		number of qualified playlists

	//##ModelId=4147B84B0232
	int queryByChnl(std::string deviceid, std::string ipaddr, std::string ipport, std::vector<STVPlaylist*> &outlist, int listtype = LISTTYPE_PLAYLIST);
	
	/// query playlist by channel information
	/// @param[in]	chnlPref	pointer to an XML structure containing channel information
	/// @param[out]	outlist	the list containing qualified playlists
	///@param[in]	listtype	the type of list, LIST_PLAYLIST, LIST_BARKER or LIST_FILLER
	/// @return		number of qualified playlists

	int queryByChnl(ZQ::common::IPreference* chnlPref, std::vector<STVPlaylist*> &outlist, int listtype = LISTTYPE_PLAYLIST);

	/// query playlist by channel number(purchase ID)
	///@param[in]	chnlNum		the chnlNum of specific _PLchannel value
	///@param[out]	outlist		the list containing qualified playlists
	///@param[in]	listtype	the type of list, LIST_PLAYLIST, LIST_BARKER or LIST_FILLER
	///@return		number of qualified playlists
	int queryByChnl(DWORD chnlNum, std::vector<STVPlaylist*> &outlist, int listtype = LISTTYPE_PLAYLIST);

	/// query global fillers
	///@param[out] outlist		the list containing qualified filler
	/// @return		number of qualified fillers
	int queryGlobalFiller(std::vector<STVPlaylist*> &outlist);

	/// get current playing playlist of specific channel
	///@param[in]	chnlNum		the chnlNum of specific _PLchannel value
	///@param[out]	listtype	the type of list current playing, LIST_PLAYLIST, LIST_BARKER or LIST_FILLER
	///@param[in]	status		which status should be queried, PLSTAT_PLAYING, PLSTAT_ALLOCATING or PLSTAT_DESTROYING
	///@return		pointer to the qualified playlist, NULL if no playlist found
	STVPlaylist* queryCurrentPL(DWORD chnlNum, int& listtype, int status=PLSTAT_PLAYING);

	/// extract effective filler/barker set
	///@param inlist	containing the sets to verify
	///@return			the current effective set, of NULL if no set effective
	STVPlaylist*	getEffectiveSet(std::vector<STVPlaylist*> inlist);

	/// extract playlists which are about to setup
	/// @param critAssoci	the time criteria used to extract pool for SETUP
	/// @param critPlay		the time criteria used to extract pool for PLAY
	/// @param outlists		the output, containing all the extracted lists
	/// @return				the minimum time for next sleep

	//##ModelId=4147B84B0242
	DWORD extract(unsigned long critAssoci, unsigned long critPlay, std::vector<STVPlaylist*>& outlists);

	/// purify playlists which are duplicated due to Unique Asset No
	/// call this function will cause all playlists that have assets NO between bottomNo and topNO deleted
	/// and unregistered from manager
	///@param[in]	chnlID		the channel ID of the list that need purify
	///@param[in]	bottomNO	the bottom NO of asset that need purify
	///@param[in]	topNO		the top NO of asset that need purify
	///@param[in]	nextTime	the next time point of asset that need purify
	///@param[in]	nType		the type of list that need purify
	///@return					the number of playlists that had been purified, -1 if this playlist itsself should be unregistered
	int purify(DWORD chnlID, DWORD bottomNO, DWORD topNO, SYSTEMTIME nextTime, int nType);
	
	/// get all playlists currently in pool
	/// @return			a vector containing all playlists

	//##ModelId=4147B84B0252
	int getPlaylists(std::vector<STVPlaylist*>& outlists);

	/// restore playlist info from DB mirror after maybe a crash restart
	/// @return			the number of playlists restored

	//##ModelId=4147B84B0261
	int restore();

	/// get channel related information 
	///@param[in]	Uid				the unique id of the channel
	///@param[out]	deviceid		the node group of itv system
	///@param[out]	ipaddr			the destination ip address of playlist
	///@param[out]	port			the destination port of playlist
	///@return						ture if get successfully
	bool getChnlInfo(DWORD Uid, std::string& deviceid, std::string& ipaddr, std::string& port);

	///////////////////////////////////// External interface /////////////////////////////////////
	
	/// create a new playlist according to information provided, and register it
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@param[in] nType			the type of playlist, one among LISTTYPE_BAKKER, LISTYPE_PLAYLIST, LISTYPE_FILLER
	///@return				error code; STVSUCCESS if create successfully
	int OnNewPlayList(ZQ::common::IPreference*  pPortPref, ZQ::common::IPreference*  pPlayListPref,  int nType);
 
	/// play specific asset immediately
	///@param[in] pPortPref		the pointer to an XML structure containing info about Channel
	///@param[in] pPlayListPref	the pointer to an XML structure containing info about playlist
	///@return				error code; STVSUCCESS if create successfully
	int OnPlayAssetImmediately (ZQ::common::IPreference *pPortPref, ZQ::common::IPreference *pPlayListPref);
	
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
	int OnConfigration(ZQ::common::IPreference* pConfigPref, bool dump=TRUE);

	/// to send a schedule enquiry message
	///@param[out]	backstream		the xml stream of asset element status, which will be sent to SM
	///@param[in,out] backcount		the max count of stream input, and after the call succeeded, the actual count of output stream
	///@return				error code; STVSUCCESS if create successfully
	int OnConnected(char* backstream, DWORD* backcount);

	/// query the asset elements of specified playlist and stores it in _mapAELIST
	///@param[in] pl			the pointer to playlist that need query
	///@return				error code; STVSUCCESS if create successfully
	int OnIDSquery(STVPlaylist* pl);

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
	///@param[in] chnlID		the ID of the stream that need AELlist (purchase ID in RTSP)
	///@param[in] notiMsg		the feedback message from ISS about channel
	///@param[out]	backstream		the xml stream of asset element status, which will be sent to SM
	///@param[in,out] backcount		the max count of stream input, and after the call succeeded, the actual count of output stream
	///@return					error code; STVSUCCESS if create successfully
	int OnSetStreamStatus(DWORD chnlID, SSAENotification notiMsg, char* backstream, DWORD* backcount);

	/// play playlist immediately
	///@param[in] pList		the pointer to the playlist object
	///@param[in] ChnlID	the ID of the channel that should start
	///@return				error code; STVSUCCESS if create successfully
	int OnPlayImmediately (STVPlaylist* pList, DWORD ChnlID);

public:
	///////////////////////////////////// Attributes methods /////////////////////////////////////
	//##ModelId=4147B84B0262
	bool	isRunning() { return _IsRunning; }

	void	checkTimer(); 
	
	/// get he IdsSession object
	///@return		the pointer to the IdsSession object
	//IdsSession*	getBuilder() { return _Builder; }
	
protected:
	/// check the file name for validation
	/// @param fname	the name of file to check
	/// @return			true if valid

	//##ModelId=4147B84B0271
	bool isValidFile(std::wstring fname);

	/// get IDS information
	///@return		ture if init IdsSession successfully
	//bool initIdsSession();

	/// compose ISS AElist map pure playlist
	///@param[in]  pl		pointer to playlist
	///@param[out] pAE		pointer to AELIST
	///@param[in]  pASp		pointer to ASSETS containing playlist AE
	///@return		ture if compose successfully
	bool composeAE(STVPlaylist* pl, PAELIST pAE, PASSETS pASp);

	/// compose ISS AElist map for playlist with filler
	///@param[in]  pl		pointer to playlist
	///@param[out] pAE		pointer to AELIST
	///@param[in] pASp		pointer to ASSETS containing playlist AE
	///@param[in] filledlist	pointer to filled filler pointer vector
	///@return		ture if compose successfully
	bool composeAE(STVPlaylist* pl, PAELIST pAE, PASSETS pASp, PASSETLIST* filledlist);

	/// compose ISS AElist map for pure barker and filler list
	///@param[in]  pl		pointer to filler/barker list
	///@param[out] pAE		pointer to AELIST
	///@param[in] filledlist	pointer to filled barker pointer vector
	///@return		ture if compose successfully
	bool composeAE(STVPlaylist* pl, PAELIST pAE, PASSETLIST* filledlist);

public:
	/// compare two SYSTEMTIME object
	/// @param t1,t2	the time to compare with
	/// @return			-1 if t1 earlier than t2, 0 if equal, 1 if t1 later than t2

	//##ModelId=4147B84B0280
	static int timeComp(SYSTEMTIME t1, SYSTEMTIME t2);

	/// subtract two SYSTEMTIME
	/// @param t1,t2	the time to deal with
	/// @return			FFFFFFFF	-	if t1 earlier than t2 or the difference is greater than 1 year
	///                 seconds		-	if t1 later than t2, in term of seconds
	/// @remark			better call timeComp() first
	
	//##ModelId=4147B84B0291
	static unsigned long timeSub(SYSTEMTIME t1, SYSTEMTIME t2);
};

#endif // !defined(AFX_STVPLAYLISTMANAGER_H__91FEF100_CA2E_4D2B_B474_BCAEEFD25F5D__INCLUDED_)
