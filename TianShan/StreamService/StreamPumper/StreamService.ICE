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
// Ident : $Id: StreamSmithAdim.ICE $
// Branch: $Name:  $
//@author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/StreamService/StreamPumper/StreamService.ICE $
// 
// 2     12/21/11 4:31p Hongquan.zhang
// 
// 1     10-12-17 15:11 Hongquan.zhang
// 
// 3     10-04-12 16:07 Hui.shao
// renamed some source files
// 
// 2     09-04-23 17:08 Fei.huang
// * fix case for gcc
// 
// 1     09-03-17 11:58 Hongquan.zhang
// ice definition initial check in
// 
// 15    08-07-08 14:53 Hui.shao
// fix per ICE 3.3 definitions
// 
// 14    07-12-24 11:01 Hongquan.zhang
// 
// 13    07-05-15 14:28 Hongquan.zhang
// 
// 12    07-05-09 17:45 Hongquan.zhang
// 
// 11    07-03-14 12:33 Hongquan.zhang
// 
// 10    07-03-07 11:56 Hongquan.zhang
// 
// 9     07-02-08 11:09 Hongquan.zhang
// 
// 8     06-11-23 15:24 Hongquan.zhang
// 
// 7     06-11-23 15:12 Hongquan.zhang
// 
// 6     06-11-23 14:21 Hongquan.zhang
// 
// 5     06-11-21 16:53 Hongquan.zhang
// 
// 4     06-10-19 18:07 Hongquan.zhang
// 
// 3     06-09-19 11:42 Hongquan.zhang
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_StreamService_ICE__
#define __ZQ_TianShanIce_StreamService_ICE__

#include "TsStreamer.ICE"
#include "TsSRM.ICE"

/*
$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice
$(ICE_ROOT)/bin/slice2freeze -I$(ICE_ROOT)/slice --dict "TianShanIce::Streamer::PlaylistDict,string,TianShanIce::Streamer::PlaylistEx" --dict-index "TianShanIce::Streamer::PlaylistDict,guid" PlaylistDict $(InputName).ICE
$(ICE_ROOT)/bin/slice2freeze -I$(ICE_ROOT)/slice --index "TianShanIce::Streamer::IndexItemToPlaylist,TianShanIce::Streamer::PlaylistItem,guid,case-insensitive" ItemToPlaylist $(InputName).ICE

./$(InputName).h
./$(InputName).cpp
./PlaylistDict.h
./PlaylistDict.cpp
./ItemToPlaylist.h
./ItemToPlaylist.cpp
*/

module TianShanIce
{

// -----------------------------
// forward declarations
// -----------------------------
module Weiwoo
{
	class Session;
};

// -----------------------------
// namespace Streamer
// -----------------------------
/// Streamer represents the streaming session management subsystem within the TianShan architecture
module Streamer
{

// -----------------------------
// struct PlaylistAttr
// -----------------------------
/// runtime attributes of the playlist in Vstream and StreamSmith
//struct PlaylistAttr
//{
//	string			Guid;				///< unique id to identify this playlist instance
//		
//	string			StreamSmithSiteName; ///< reserved for standalone StreamSmith
//
//	string			ResourceGuid;		///< reserved for standalone StreamSmith
//	string			ClientSessionID;	///< reserved for standalone StreamSmith
//	string			endPoint;			///< reserved for standalone StreamSmith
//
//	int				MaxRate;			///< max bitrate of this playlist
//	int				MinRate;			///< min bitrate of this playlist
//	int				NowRate;			///< the current bitrate of this playlist
//
//	string			destIP;				///< destionation IP if at IP mode
//	int				destPort;			///< destination Port
//
//	string			destMac;			///< destination mac address
//		
//	int				vstrmPort;			///< vstream port employed for this playlist
//	int				programNumber;		///< MPEG program number
//	short			playlistState;		///< to identify current playlist state
//		
//	int				currentCtrlNum;		///< to identify which item is playing	
//	int				vstrmSessID;		///< vstream session id of the current playing item
//
//	int				streamPID;
//	
//	Properties		property;
//};

// -----------------------------
// class PlaylistEx
// -----------------------------
/// entension of Playlist for some internal access
["freeze:write"] 
class PlaylistEx extends TianShanIce::Streamer::Playlist
{
//	string			guid;
//	PlaylistAttr	attr;
//	PlaylistAttr	getAttr();
//	void			updateAttr(PlaylistAttr attr);

	/// set associated Weiwoo session
	///@param[in] sess access to the session about to bind with this playlist
	///@remark    
	void			attachSession(SRM::Session* sess);

	///set destination IP and port
	///@param destIP destination IP string
	///@param port destination udp port
	void			setDestination(string destIP,int port);
	
	///set destination mac
	///@param strMac destination mac address
	void			setDestMac(string strMac);

	///set source stream service port
	void			setSourceStrmPort(int port);

	///set per stream PID
	void			setPID(int pid);
};

/// a container of playlist ID to receive the query result
sequence<string> PlaylistIDs;

// -----------------------------
// struct PlaylistAttr
// -----------------------------
/// detailed item attributes with runtime inform
//struct PlaylistItemAttr
//{
//	string					PlaylistGuid;		///< guid to ident the playlist that this item belongs to
//
//	PlaylistItemSetupInfo	itemSetupInfo;		///< the attributes when the item was setup 
//
//	int						InternalCtrlNum;	///< internal identification to address this item in a playlist
//	int						CtrlNumber;			///< user's control number for easy adderss this item in this playslist
//
//	int						vStrmSessID;		///< vstream session id assigned to this item
//	short					SessState;			///< the latest vstream session state of this item
//	Properties				property;
//};

// -----------------------------
// class PlaylistItem
// -----------------------------
/// access to an item within a playlist, for internal use
//["freeze:write"] 
//class PlaylistItem
//{
//	string				guid;	//??
//	PlaylistItemAttr	attr;	///< item attributes
//
//	/// destroy this item instance
//	void				destroy();
//
//	/// get the attributes
//	///@return the attributes of this item
//	["cpp:const", "freeze:read"] PlaylistItemAttr	getAttr();	
//
//	/// overwrite the attributes
//	///@note internal use, do not export
//	///@param[in] attr the new attributes
//	void				updateAttr(PlaylistItemAttr attr);
//};


sequence<int>	SpigotBoards;
// -----------------------------
// Service StreamSmithAdmin
// -----------------------------
/// represents a server-side playlist-based streamer service as a entry to access stream sessions
interface StreamSmithAdmin extends StreamSmith
{
	/// list all the on-going playlists
	///@return	  a collection of playlist's GUID
	PlaylistIDs		listPlaylists();
	
	/// find or create a playlist by a given session
	///@param[in] guid the guid of the playlist about to access
	///@param[in] bCreateIfNotExist true if need to create a new playlist when it doesn't exist
	///@param[in] EdgeCards which edge can be used
	///@return	  access to an existing playlist with the give guid
	Playlist*		openPlaylist(string guid, SpigotBoards EdgeCards, bool bCreateIfNotExist);



};

};
};

#endif // __ZQ_TianShanIce_StreamService_ICE__

