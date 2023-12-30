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
// $Log: /ZQProjs/ScheduledTV_old/PlaylistMod/STVPlaylist.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 22    05-06-27 18:04 Bernie.zhao
// June.27/2005.  Fixed interface error with SM when creating new STV list
// 
// 21    05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 20    05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 19    05-01-05 17:16 Bernie.zhao
// modified to support STV status feedback
// 
// 18    04-12-16 14:55 Bernie.zhao
// 
// 17    04-12-06 11:46 Bernie.zhao
// 
// 16    04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 15    04-10-26 16:08 Bernie.zhao
// 0.1.6 Oct/26
// 
// 14    04-10-25 15:26 Bernie.zhao
// added status report
// 
// 13    04-10-24 19:04 Bernie.zhao
// end of 2004/Oct/24
// 
// 12    04-10-18 18:46 Bernie.zhao
// 
// 11    04-10-15 16:14 Bernie.zhao
// 
// 10    04-10-14 11:04 Bernie.zhao
// node missing pro fixed
// 
// 9     10/13/04 5:22p Jie.zhang
// 
// 8     04-10-13 16:57 Bernie.zhao
// 
// 7     04-10-13 15:37 Bernie.zhao
// fixed top&bottom asset error
// 
// 6     04-10-12 16:56 Bernie.zhao
// 
// 5     04-10-12 14:04 Bernie.zhao
// added log
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

#if !defined(AFX_STVPLAYLIST_H__62DBA64B_3AB7_456A_954F_1FB752EB632D__INCLUDED_)
#define AFX_STVPLAYLIST_H__62DBA64B_3AB7_456A_954F_1FB752EB632D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// common include
#include "XMLPreference.h"
#include "Locks.h"
#include "itvservicetypes.h"
#include "issapi.h"

//#include "../../Common/comextra/include/ZqStringConv.h"

// local module include
//#include "STVPlaylistManager.h"

// other module include
#include "../STVMainHeaders.h"
#include "../Filler/AutoFiller.h"
#include "../Filler/BarkerFiller.h"
#include "../IssStreamCtrl/stv_ssctrl.h"
//#include "../testiss/TestIss.h"
//#include "../testiss/StreamSession.h"

#include "ScLog.h"
#include <string>

#define	MAX_SUBCHNL			100

#define		DEFAULT_RETRYTIMES		3

#define	INIT_NORMAL			0
#define INIT_RESTORE		1

#define CHNL_NONE				0	// not any list or global filler
#define CHNL_NSTARTING			1	// normal list starting
#define CHNL_NSTOPPING			2	// normal list stopping
#define CHNL_NPLAYING			3	// normal list playing
#define	CHNL_GSTARTING			4	// global filler starting
#define CHNL_GSTOPPING			5	// global filler stopping
#define CHNL_GPLAYING			6	// global filler playing


#define PLSTAT_IDLE			0		// playlist is idle
#define PLSTAT_ASSOCIATED	1		// playlist has prepared AE list
#define PLSTAT_ALLOCATING	2		// playlist that has resource allocated, being setup
#define PLSTAT_PLAYING		3		// playlist that is playing currently
#define PLSTAT_DESTROYING	4		// playlist that is being destroyed
#define PLSTAT_OUTOFDATE	5		// playlist that is not playing, and will NOT be playing anymore


#define DB_DEFAULT_PATH		"D:\\ITV\\STVDB\\"

#define XML_DBMIRROR				"DBMirrorPlayList"
#define XML_DBCONFIG				"DBConfiguration"
// XML file key words
#define XML_STATUSINFO				"StatusInformation"
#define XML_STATUSINFO_EVENT		"Event"
#define XML_STATUSINFO_EVENT_ID		"ID"
#define XML_STATUSINFO_EVENT_TIME	"Time"
#define XML_STATUSINFO_EVENT_STATUS	"Status"
#define XML_STATUSINFO_EVENT_ERRORCODE	"ErrorCode"


#define XML_SCHEDULEINFO			"ScheduleInformation"
#define XML_STATUS					"Status"
#define XML_STATUS_CURRENT			"Current"
#define XML_STATUS_CURRENT_STATE	"State"
#define XML_STATUS_CURRENT_PLAYINGELE		"PlayingElement"
#define XML_STATUS_CURRENT_IDS		"IDSQueried"

#define	XML_PORT					"Port"
#define XML_PORT_CHANNELID			"ChannelID"
#define XML_PORT_SUBCHANNELID		"SubChannelID"
#define XML_PORT_DEVICEID			"DeviceID"
#define XML_PORT_PORTID				"PortID"
#define XML_PORT_IPADRESS			"IPAddress"
#define XML_PORT_IPPORT				"IPPort"

#define XML_CHANNEL					"Channel"

#define XML_PLAYLIST				"PlayList"
#define XML_PLAYLIST_LISTNUMBER		"ListNumber"
#define XML_PLAYLIST_ELEMENTNUMBER	"ElementNumber"
// for barker
#define XML_PLAYLIST_PLAYMODE		"PlayMode"
#define XML_PLAYLIST_EFFECTTIME		"EffectiveDatetime"
#define XML_PLAYLIST_ENDTIME		"EndDatetime"

#define XML_PLAYLIST_ASSET			"Asset"
#define XML_PLAYLIST_ASSET_NO		"NO"
#define XML_PLAYLIST_ASSET_ID		"ID"
#define XML_PLAYLIST_ASSET_ASSETID	"AssetID"
#define XML_PLAYLIST_ASSET_TYPE		"Type"
#define XML_PLAYLIST_ASSET_BEGIN	"Begin"
#define XML_PLAYLIST_ASSET_TIME		"Time"
#define XML_PLAYLIST_ASSET_ISFILLER "isFiller"
#define XML_PLAYLIST_ASSET_DURATION	"Duration"
#define XML_PLAYLIST_ASSET_AENUMBER	"AeNumber"	// from IDS
// from IDS query
#define XML_PLAYLIST_ASSET_AE		"Element"
#define XML_PLAYLIST_ASSET_AE_NO	"NO"
#define XML_PLAYLIST_ASSET_AE_ID	"ElementID"
#define XML_PLAYLIST_ASSET_AE_DURATION	"Duration"
#define XML_PLAYLIST_ASSET_AE_BITRATE	"BitRate"
#define XML_PLAYLIST_ASSET_AE_CUEIN		"CueIn"
#define XML_PLAYLIST_ASSET_AE_CUEOUT		"CueOut"
#define XML_PLAYLIST_ASSET_AE_CUEDURATION	"CueDuration"
#define XML_PLAYLIST_ASSET_AE_EVENT			"Event"
#define XML_PLAYLIST_ASSET_AE_STATUS		"Status"

#define XML_FILLERLIST				"FillerList"
#define XML_FILLERLIST_LISTNUMBER	"ListNumber"
#define XML_FILLERLIST_PLAYMODE		"PlayMode"
#define XML_FILLERLIST_EFFECTTIME	"EffectiveDatetime"
#define XML_FILLERLIST_ENDTIME		"EndDatetime"
#define XML_FILLERLIST_ASSET		"Asset"
#define XML_FILLERLIST_ASSET_NO		"NO"
#define XML_FILLERLIST_ASSET_ID		"ID"
#define XML_FILLERLIST_ASSET_BEGIN	"Begin"
#define XML_FILLERLIST_ASSET_DURATION	"Duration"
#define XML_FILLERLIST_ASSET_PRIORITY	"Priority"
#define XML_FILLERLIST_ASSET_AENUMBER		"AeNumber"	// from IDS
// from IDS query
#define XML_FILLERLIST_ASSET_AE		"Element"
#define XML_FILLERLIST_ASSET_AE_NO	"NO"
#define XML_FILLERLIST_ASSET_AE_ID	"ElementID"
#define XML_FILLERLIST_ASSET_AE_DURATION	"Duration"
#define XML_FILLERLIST_ASSET_AE_BITRATE		"BitRate"
#define XML_FILLERLIST_ASSET_AE_CUEIN		"CueIn"
#define XML_FILLERLIST_ASSET_AE_CUEOUT		"CueOut"
#define XML_FILLERLIST_ASSET_AE_CUEDURATION	"CueDuration"

/// class STVPlaylist stands for a playlist schedule

class STVPlaylistManager;

//##ModelId=4147B84B02CE
class STVPlaylist  
{
public:
	//////////////////////////////////// Attribute Methods //////////////////////////////////////

	/// set database mirror directory
	///@param[in] dbdir		the database directory path

	//##ModelId=4147B84B035B
	void setDBdir( const wchar_t* dbdir) { wcscpy(_dbdir, dbdir); }
	/// get database mirror directory
	///@return			the database directory path

	//##ModelId=4147B84B036C
	const wchar_t* getDBdir() { return _dbdir; }

	/// get database mirror file name, with path
	///@return		the database mirror file name with path
	const wchar_t* getDBfile() { return _dbfilename; }

	/// set playlist status and flush it into database mirror file
	///@param[in] statcode	the current status

	void setPLStatus( int statcode ) 
	{	
		if(_IsGlobal)
			return;
		
		if(statcode==_PLstatus)
			return;

		_PLstatus=statcode; 
		if( statcode==PLSTAT_IDLE 
			|| statcode==PLSTAT_PLAYING
			|| statcode==PLSTAT_OUTOFDATE )
		{
			setStatus(XML_STATUS_CURRENT, XML_STATUS_CURRENT_STATE, statusitoa(_PLstatus).c_str());
		}
	}

	/// get playlist current status
	///@return		the current status of playlist

	int	getPLStatus() { return _PLstatus; }

	/// set current playing asset NO and flush it into db mirror
	///@param playingasset	the current playing asset
	void setPlayingAsset(DWORD playingasset) { _PlayingAsset = playingasset; }

	/// get current playing asset NO
	///@return		the current playing asset NO
	int getPlayingAsset() { return _PlayingAsset; }
	
	/// get playlist type
	///@return		the playlist type
	int getPLType() { return _PLtype; }

	/// get fill type (for barker/filler only)
	///@return		the playlist fill type
	FILLTYPE getFillType() { return _Filltype; }
	

	/// set playlist channel information
	///@param[in] chnlcode		the channel id
	void setPLchannel(DWORD chnlcode) { _PLchannel = chnlcode; }

	/// get playlist channel information
	///@return		the channel id
	DWORD	getPLchannel() { return _PLchannel; }

	/// get playlist top asset no
	///@return		the top asset NO
	DWORD getTopAsset() { return _TopAsset; }

	/// set playlist top asset no
	///@param[in] topasset		the top asset NO
	void	setTopAsset(DWORD topasset) { _TopAsset = topasset; }

	/// get playlist bottom asset no
	///@return		the bottom asset NO
	DWORD getBottomAsset() { return _BottomAsset; }

	/// set playlist bottom asset no
	///@param[in] bottomasset	the bottom asset NO
	void	setBottomAsset(DWORD bottomasset) { _BottomAsset = bottomasset; }
	
	/// get start from middle point flag
	///@return	the mid start flag
	bool	getStartFromMid() { return _StartFromMid; }

	/// set start from middle point flag
	///@param[in]	startfrommid	the flag
	void	setStartFromMid(bool startfrommid) { _StartFromMid = startfrommid; }

	/// get start point from middle
	///@return	the mid start point
	DWORD	getStartPoint() { return _StartPoint; }
	
	/// set start point from middle
	///@param[in]	startpoint		the start point , in seconds
	void	setStartPoint(DWORD startpoint) { _StartPoint=startpoint; }

	/// get playlist length
	///@return	the length of this playlist, in seconds
	DWORD	getPLLength() { return _PLLength; }

	/// get isGlobal attribute
	///@return	true if global, false otherwise
	bool	isGlobal() { return _IsGlobal; }

	/// check if retry times over some criteria
	///@return	true if not over 5, else false
	bool	retryTest() { return(_RetryTimes>(DEFAULT_RETRYTIMES*2)?false:true); }

	/// increase retry times
	void	retryIncr() { _RetryTimes++; }

	/// reset retry times
	void	retryReset() { _RetryTimes = 0; }
	
	
public:
	//////////////////////////////////// DB Mirror Methods //////////////////////////////////////
	
	/// update the current playlist to DB
	/// @return	true if update successfully
	
	//##ModelId=4147B84B036D
	bool updateToDB();

	/// update the current playlist to DB
	/// @param filename		extra DB mirror file if needed
	/// @return	true if update successfully

	//##ModelId=4147B84B037A
	bool updateToDB(const wchar_t* filename);

	/// restore playlist from DB
	/// @param dbfilename	the db file
	/// @return	a STVPlaylist object constructed from DB

	//##ModelId=4147B84B038A
	bool restoreFromDB( const wchar_t* dbfilename);

public:
	///////////////////////////////////// Playlist Ctrl /////////////////////////////////////

	///@param channelnode	the pointer to an XML structure containing channel information
	///@param listnode		the pointer to an XML structure containing each playlist information
	///@param listtype		the type of playlist, one among LISTTYPE_BAKKER, LISTYPE_PLAYLIST, LISTYPE_FILLER
	///@return error code, STVSUCCESS when create successfully

	//##ModelId=4147B84B033D

	int create(ZQ::common::IPreference* channelnode, ZQ::common::IPreference* listnode, int listtype);
	/// verify if channel belongs to this playlist
	///@param deviceid	the nodegroup id of the query channel
	///@param ipaddr	the IP address of the query channel
	///@param ipport	the IP port of the query channel
	///@return		ture if it is, false else

	//##ModelId=4147B84B0399
	bool isMyChannel(std::string deviceid, std::string ipaddr, std::string ipport);

	/// get the begin time of this playlist
	/// @return			the begin time with format as SYSTEMTIME

	//##ModelId=4147B84B03A9
	SYSTEMTIME getNextTime();

	/// check if current time between effective time span of the playlist (only for filler list and barker list)
	///@param currtime		the current time to compare with
	///@return	ture if current time is within effective time, otherwise false
	bool	isInEffective(SYSTEMTIME currtime);

	/// get channel related information about this playlist
	///@param[out]	deviceid		the node group of itv system
	///@param[out]	ipaddr			the destination ip address of playlist
	///@param[out]	port			the destination port of playlist
	///@return						ture if get successfully
//	bool getChnlInfo(std::string& deviceid, std::string& ipaddr, std::string& port);


	/// generate xml feedback stream
	///@param[in]	feedback		the status feedback from ISS table
	///@param[out]	backstream		the xml stream of asset element status, which will be sent to SM
	///@param[in,out] backcount		the max count of stream input, and after the call succeeded, the actual count of output stream
	///@return		true if update successfully, false otherwise
	///@remarks		this function do NOT update current playing Element. If you need to update current playing element, use updatePlayingElemEx
	bool updatePlayingElem(const SSAENotification feedback, char* backstream, DWORD* backcount);

	/// update current playing asset NO and generate xml feedback stream
	///@param[in]	feedback		the status feedback from ISS table
	///@param[out]	backstream		the xml stream of asset element status, which will be sent to SM
	///@param[in,out] backcount		the max count of stream input, and after the call succeeded, the actual count of output stream
	///@return		true if update successfully, false otherwise
	bool updatePlayingElemEx(const SSAENotification feedback, char* backstream, DWORD* backcount);

public:
	///////////////////////////////////// External Interfaces /////////////////////////////////////
	int	OnPLIdsSession(PASSETS pAssetList);	

public:
	/// convert status code to string
	static std::string statusitoa(int statcode)
	{
		std::string retstr = "";
		switch(statcode) {
		case PLSTAT_IDLE:
			retstr = "Idle";
			break;
		case PLSTAT_ASSOCIATED:
			retstr = "Associated";
			break;
		case PLSTAT_ALLOCATING:
			retstr = "Allocating";
			break;
		case PLSTAT_PLAYING:
			retstr = "Playing";
			break;
		case PLSTAT_DESTROYING:
			retstr = "Destroying";
			break;
		case PLSTAT_OUTOFDATE:
			retstr = "Outofdate";
			break;
		}

		return retstr;
	}

	/// convert string to status code
	static int	statusatoi(std::string statstr)
	{
		if(statstr=="Idle")
			return PLSTAT_IDLE;
		else if(statstr=="Associated")
			return PLSTAT_ASSOCIATED;
		else if(statstr=="Allocating")
			return PLSTAT_ALLOCATING;
		else if(statstr=="Playing")
			return PLSTAT_PLAYING;
		else if(statstr=="Destroying")
			return PLSTAT_DESTROYING;
		else if(statstr=="Outofdate")
			return PLSTAT_OUTOFDATE;
		else
			return -1;
	}

	/// convert CueTime format to Seconds
	static DWORD Cue2Sec(std::string CueTime);

protected:
	/// create a new DB name according to playlist info
	/// @param channelnode	the channel info of the playlist
	/// @param listnode		the assets info of the playlist
	/// @param listtype		the type info of the playlist
	/// @return	true if create successfully
	
	//##ModelId=4147B84B03AA
	int createDBName(ZQ::common::IPreference* channelnode, ZQ::common::IPreference* listnode, int listtype);

	/// initialize, such as the Memory, XML structure and database mirror file
	/// @param typerestore	INIT_NORMAL indicates a normal init, INIT_RESTORE indicates a restore init from DB
	/// @return true if init successfully
	
	//##ModelId=4147B84B03C8
	bool init(int typerestore=INIT_NORMAL);

	/// add asset elements to playlist XML structure
	///@param[in] listasset		the list containing info about asset and ae from IDS
	///@return		ture if attach successfully
//	bool attachAE(ASSETS listasset);

	/// fetch asset elements from playlist XML structure
	///@param[out] listasset	the list containing output asset and ae previous queried from IDS
	///@return		ture if fetch successfully
	bool fetchAE(PASSETS plistasset);

	/// set an attribute in Status section of corresponding XML structure and database mirror
	///@param[in] name		the name of the node
	///@param[in] key		the key of the node
	///@param[in] value		the value of the key
	///@remarks				if the key value is already existed, it will be replaced; 
	///						\if the key is not existed, a new key and value will be created;
	///						\if the node name is not even existed, a new node, key and value will be created;

	void setStatus(std::string name, std::string key, std::string value);

	/// get an attribute in Status section of corresponding XML structure and database mirror
	///@param[in] name		the name of the node
	///@param[in] key		the key of the node
	///@param[out] value	the value of the key
	///@return				true if value got successfully
	bool getStatus(std::string name, std::string key, std::string& value);

protected:
	/////////////////////////////////// DB related attributes ///////////////////////////////////////
	//friend class STVPlaylistManager;
	
	/// pointer to STVPlaylistManager
	STVPlaylistManager*	_mgm;

	/// db file name
	//##ModelId=4147B84B02DE
	wchar_t	_dbfilename[MAX_PATH];
	//##ModelId=4147B84B02EE
	wchar_t	_dbdir[MAX_PATH];

	/// pointer to IPreferenceDoc factory class
	//##ModelId=4147B84B032D
	ZQ::common::XMLPrefDoc* _dbdoc;

	/// playlist type
	int	_PLtype;

	/// fill type for barker/filler list
	FILLTYPE	_Filltype;

	/// global filler
	bool	_IsGlobal;
	
	/// playlist status
	int	_PLstatus;

//	/// filler/barker set status
//	bool _SetIDS;
	
	/// indicate whether should calculate start time from middle point
	bool _StartFromMid;

	/// when does this playlist start from middle point
	DWORD _StartPoint;

	/// mark of the last playing asset
	DWORD	_PlayingAsset;

	/// playlist channel id
	DWORD	_PLchannel;

	/// bottom asset no
	DWORD	_BottomAsset;
	
	/// top asset no
	DWORD	_TopAsset;

	/// first begin time for schedule playlist( or effective start time for filler/barker )
	SYSTEMTIME	_BeginTime;

	/// effective end time for filler/barker only
	SYSTEMTIME	_EndTime;
	
	/// how long is this playlist for schedule playlist only
	DWORD _PLLength;

	/// indicate how many times has this list been retried, only used for normal playlist
	DWORD _RetryTimes;

	ZQ::common::Mutex	_AttribLock;

	/////////////////////////////////// Asset related attributes ///////////////////////////////////////

public:
	//////////////////////////////////// Constructor and Destructor /////////////////////////////////////
	//##ModelId=4147B84B033C
	STVPlaylist(STVPlaylistManager*	mgm);
	
	//##ModelId=4147B84B034D
	~STVPlaylist();
};

#endif // !defined(AFX_STVPLAYLIST_H__62DBA64B_3AB7_456A_954F_1FB752EB632D__INCLUDED_)
