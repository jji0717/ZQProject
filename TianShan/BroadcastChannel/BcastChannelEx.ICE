// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: BcastChannelEx.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastChannelEx.ICE $
// 
// 2     10/21/14 3:32p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     09-07-06 9:48 Li.huang
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_AppBcastEx_ICE__
#define __ZQ_TianShanIce_AppBcastEx_ICE__

#include "TsAppBcast.ICE"

module TianShanIce
{

// -----------------------------
// namespace Application
// -----------------------------
/// Application represents the basic entry definition of a business application to TianShan architecture
module Application
{

// -----------------------------
// namespace Broadcast
// -----------------------------
/// Broadcast represents some schedule-driven application definitions 
module Broadcast
{
// -----------------------------
// struct ChannelItemEx
// -----------------------------
/// dictionary record,
struct ChannelItemEx
{
	string	key;			///< a combination string in the format of "<ChannelName>#<ContentName>", case-insensitive
	ChannelItem setupInfo;
	long	flags;			///< additional flags, reserved
	
	long	broadcastStart; ///< the absolute start-time of this item on the source live channel, in GMT, msec
	long	expiration;		///< the time that this item should be expired and no more availabe for ordering, in GMT, msec
	bool    isFilter;

};
class BcastPublishPointEx;

struct ChannelItemAssocData
{
	Ice::Identity ident;  ///< unque key of this object

	Ice::Identity	bcastPPIdent;  ///< index entry from  BcastPublishpoint
	string			channelItemKey; ///< index entry from ChannelItem
	int				playlistCtrlNum; ///< index entry from playlist
	long			lastModified; // record the channel item's last modified time
};

["freeze:write"]
class ChannelItemAssoc
{
	Ice::Identity ident;  ///< unque key of this object

	Ice::Identity	bcastPPIdent;  ///< index entry from bcastPublishPoint
	string			channelItemKey; ///< index entry from ChannelItem
	int				playlistCtrlNum; ///< index entry from playlist
	long			lastModified; // record the channel item's last modified time

	/// get  the user ctrl number of the item in a purchase
	///@return the user ctrl number of the item in a purchase
	["cpp:const", "freeze:read"] int		getCtrlNum();

	/// get the channel item key
	///@return the channel item key
	["cpp:const", "freeze:read"] string	getChannelItemKey();

	/// get the setup information of published item 
	///@return the setup information of published item 
	["cpp:const", "freeze:read"] ChannelItemEx getChannelItem();
	
	/// get the access to the broadcastpublishpoint
	///@return the access to the broadcastpublishpoint
	["cpp:const", "freeze:read"] BcastPublishPointEx*   getBcastPublishPointEx();
	
	["cpp:const", "freeze:read"] ChannelItemAssocData getData();
	
	/// destroy the association
	void destroy();
};
// -----------------------------
// class BcastPublishPoint
// -----------------------------
/// a publish point of channel represents a channel which is available for on-demand ordering
["freeze:write"] 
class BcastPublishPointEx extends BcastPublishPoint
{
	///< access to the playlist of streamserver
	::TianShanIce::Streamer::Playlist* playlist;

	///< the id of playlist
	string				playlistId;
	
	bool				bInService;	// indicates whether the purchase has been initialized successfully
	bool				bNeedSyncChannel;
    long                timestamp;
    string				mainChName; // indicates the NVOD PublishPoint main channel;
    bool				bIsNVODMainCh; // indicates wheter the NVOD PublishPoint;
    bool				bIsNOVDSupCh; // indicates wheter the NVOD PublishPoint;
    
    string				currentItem; //indicates the last item in playlist
    bool                persistent;
    
    void OnEndOfStream(string playlistId);

	void OnStreamExit(string playlistId);

	void OnEndOfItem(int userCtrlNum);

	// appendChnlItem[in], the channel item to be appended.
	bool appendPlaylistItem(ChannelItemEx appendChnlItem);

	// insertPosKey[in], a channel item's position before which the new channel item will be inserted.
	// insertChnlItem[in], the channel item to be inserted.
	bool insertPlaylistItem(string insertPosKey, ChannelItemEx insertChnlItem);

	// removeItemKey[in], the channel item to be removed. this value is a combination of channel name and item name. [ChannelName#ItemName]
	bool removePlaylistItem(string removeItemKey);

	// oldItemKey[in], the channel item to be replaced. this value is a combination of channel name and item name. [ChannelName#ItemName]
	// replaceChnlItem[in], the channel item to be replace the old one.
	bool replacePlaylistItem(string oldItemKey, ChannelItemEx replaceChnlItem);

	/// synchronous the playlist according to the channel
	bool syncPlaylist();
	
	bool isNOVDMainCh();
	bool isNOVDSuppCh();
	
	bool isInService();
	
	long getExpiration();
	bool isPersistent();
	
	bool activate();
	void pingStream();
};

// -----------------------------
// class NVODPublishPoint
// -----------------------------
/// a channel publish point represents a NVOD channel as an aggregator of its main and suplemental channels
/// this NVODChannelPublishPoint itself also is the entry to access the main NVOD channel
["freeze:write"] 
class NVODChannelPublishPointEx extends NVODChannelPublishPoint
{
  BcastPublishPointEx* mainCh;///the main broadcast channel
  short iteration;            ///the iteration count of the channel within this NVOD channel
  int interval;               ///the time interval in msec between two subchannels
  
  /// the time interval in msec between two subchannels
  /// @return the time interval between two subchannels
  int getInterval();
  short getIteration(); 
  
  bool isInService();  
  bool activate();        
};
// -----------------------------
// class FilterItem
// -----------------------------
/// management the filter item 
["freeze:write"] 
class FilterItems
{
    ::TianShanIce::StrValues itemSequence; 
    
	/// add a filter item 
	///@param[in] newItem     new itemname
	void addFilterItem(string itemName);
	
	///remove a filter item 
	///@param[in] itemName the itemName to remove
	///                     must be the alias if the item has alias when it was inserted
	void removeFilterItem(string itemName);
	
	/// list all filter item
	///@return a collection of filter item 
	::TianShanIce::StrValues getFilterItemSequence(); 
	
	/// find filter item
	///@return a filter item info   
	TianShanIce::Application::ChannelItem findFilterItem(string itemName);    
};
};
};
};

#endif // __ZQ_TianShanIce_AppBcastEx_ICE__
