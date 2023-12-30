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
// Ident : $Id: ChannelOnDemandEx.ICE $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChannelOnDemandEx.ICE $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 18    09-10-08 16:11 Haoyuan.lu
// 
// 17    09-04-24 17:19 Haoyuan.lu
// 
// 16    09-02-06 17:00 Haoyuan.lu
// command line for new namespace 
// $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice
// -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. ..\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice
// -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --dict
// "ChannelOnDemand::ChannelItemDict,string,TianShanIce::Application::Chan
// nelOnDemand::ChannelItemEx" ChannelItemDict ..\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice
// -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index
// "ChannelOnDemand::Purchase2ItemAssoc,TianShanIce::Application::ChannelO
// nDemand::PurchaseItemAssoc,purchaseIdent" Purchase2ItemAssoc
// ..\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice
// -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index
// "ChannelOnDemand::CtrlNum2ItemAssoc,TianShanIce::Application::ChannelOn
// Demand::PurchaseItemAssoc,playlistCtrlNum" CtrlNum2ItemAssoc
// ..\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice
// -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index
// "ChannelOnDemand::ChannelNameIndex,TianShanIce::Application::ChannelOnD
// emand::ChannelPublishPoint,onDemandName,case-insensitive"
// ChannelNameIndex ..\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice
// -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index
// "ChannelOnDemand::PlaylistId2Purchase,TianShanIce::Application::Channel
// OnDemand::ChannelPurchase,playlistId,case-insensitive"
// PlaylistId2Purchase ..\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice
// -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index
// "ChannelOnDemand::Channel2Purchase,TianShanIce::Application::ChannelOnD
// emand::ChannelPurchase,chlPubName,case-insensitive" Channel2Purchase
// ..\$(InputName).ice
// 
// 15    08-11-18 10:46 Haoyuan.lu
// 
// 14    08-10-28 15:40 Haoyuan.lu
// 
// 13    08-05-21 13:01 Haoyuan.lu
// 
// 12    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 11    08-03-05 12:15 Guan.han
// 
// 11    08-03-04 18:53 Guan.han
// 
// 10    07-12-24 11:53 Guan.han
// 
// 9     07-08-15 14:43 Jie.zhang
// 
// 8     07-08-02 15:43 Jie.zhang
// 
// 7     07-07-06 14:47 Jie.zhang
// add a confige: enablechannelmaxdurationcheck and add some logic to
// avoid null playlist handle
// 
// 6     06-12-26 12:51 Guan.han
// 
// 5     06-10-08 16:06 Jie.zhang
// 
// 4     06-09-30 13:47 Jie.zhang
// 
// 3     06-09-28 14:07 Jie.zhang
// add ChannelPublisherEx and getChannelByStream
// 
// 2     06-09-26 11:18 Jie.zhang
// 
// 1     9/25/06 2:12p Hui.shao
// ===========================================================================
//	build steps:$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. ..\$(InputName).ice  
//		$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --dict "ChannelOnDemand::ChannelItemDict,string,ChannelOnDemand::ChannelItemEx" ChannelItemDict ..\$(InputName).ice  
//		$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::ChannelNameIndex,ChannelOnDemand::ChannelPublishPoint,onDemandName,case-insensitive" ChannelNameIndex ..\$(InputName).ice  
//		$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::PlaylistId2Purchase,ChannelOnDemand::ChannelPurchase,playlistId,case-insensitive" PlaylistId2Purchase ..\$(InputName).ice  
//		$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::Channel2Purchase,ChannelOnDemand::ChannelPurchase,chlPubName,case-insensitive" Channel2Purchase ..\$(InputName).ice  
//		$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::ChItem2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,channelItemKey,case-insensitive" ChItem2ItemAssoc ..\$(InputName).ice  
//		$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::Purchase2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,purchaseIdent" Purchase2ItemAssoc ..\$(InputName).ice  
//		$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::CtrlNum2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,playlistCtrlNum" CtrlNum2ItemAssoc ..\$(InputName).ice
//	outputs:
//		../$(InputName).h
//		../$(InputName).cpp

#ifndef __ZQ_ChannelOnDemand_ChannelOnDemandEx_ICE__
#define __ZQ_ChannelOnDemand_ChannelOnDemandEx_ICE__

//#define USE_OLD_NS  // The switch to use the old namespace

#ifdef USE_OLD_NS
#  include <..\Ice\ChannelOnDemand.ICE>
#  define  NS_PREFIX(_CLS) _CLS
#else
#  include <..\Ice\TsAppChOD.ICE>
#  define  NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#endif // USE_OLD_NS

#ifndef USE_OLD_NS
module TianShanIce {
module Application {
#endif //!USE_OLD_NS

module ChannelOnDemand
{

// -----------------------------
// struct ChannelItemEx
// -----------------------------
/// dictionary record,
struct ChannelItemEx
{
	string	key;			///< a combination string in the format of "<ChannelName>#<ContentName>", case-insensitive

//	NS_PREFIX(ChannelItem) setupInfo;
	ChannelItem setupInfo;

	long	broadcastStart; ///< the absolute start-time of this item on the source live channel, in GMT, msec
	long	expiration;		///< the time that this item should be expired and no more availabe for ordering, in GMT, msec

	long	flags;			///< additional flags, reserved
};

struct PurchaseItemData
{
	Ice::Identity ident;  ///< unque key of this object

	Ice::Identity	purchaseIdent;  ///< index entry from purchase
	string			channelItemKey; ///< index entry from ChannelItem
	int				playlistCtrlNum; ///< index entry from playlist
	long			lastModified; // record the channel item's last modified time
};

// -----------------------------
// class PurchaseItemAssoc
// -----------------------------
/// the association n this ChannelOnDemand application to link an item of a purchase to different records
["freeze:write"]
class PurchaseItemAssoc
{
	Ice::Identity ident;  ///< unque key of this object

	Ice::Identity	purchaseIdent;  ///< index entry from purchase
	string			channelItemKey; ///< index entry from ChannelItem
	int				playlistCtrlNum; ///< index entry from playlist
	long			lastModified; // record the channel item's last modified time

	/// get  the user ctrl number of the item in a purchase
	///@return the user ctrl number of the item in a purchase
	["cpp:const", "freeze:read"] int		getCtrlNum();

	/// get the channel item key
	///@return the channel item key
	["cpp:const", "freeze:read"] string	getChannelItemKey();

	/// get the access to the purchase
	///@return the access to the purchase
	["cpp:const", "freeze:read"] ChannelPurchase*   getPurchase();

	/// get the setup information of published item 
	///@return the setup information of published item 
	["cpp:const", "freeze:read"] ChannelItemEx getChannelItem();

	["cpp:const", "freeze:read"] PurchaseItemData getData();

	/// destroy the association
	void destroy();
};

class ChannelPurchaseEx extends ChannelPurchase
{
	///< a flag that the current playlist is out of date with channel due to some
	///< item is being played when channel is updated thru ChannelManager
	bool				bNeedSyncChannel;
	int					nNodeGroupId;			///< node group id for COM mode
	int					nCreateTime;			///< create time, in seconds, from c function time(0)

	string				serverSessionId;		///< it is weiwoo server session ID 
	string				clientSessionId;		///< it is RTSP client session ID 
	string				homeId;					///< home id
	string				smardcardId;			///< device data to include smardcardId, macAddress
	string				macAddress;
	string				providerId;
	string				providerAssetId;
	string				broadcastTime;		///< The broadcast time, Format: 20061010T203059, and it is local time. it is in 24H.

	bool				bInService;	// indicates whether the purchase has been initialized successfully
	
    ::TianShanIce::Properties properties; ///< contains the per-ChannelPurchaseEx properties
	/// get server session id
	string getServerSessionId();

	/// get client session id
	string getClientSessionId();

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

	bool isInService();
};

interface ChannelPublisherEx extends ChannelPublisher
{
	/// get channel name by stream
	string getChannelByStream(string streamID);
};

}; // module ChannelOnDemand

#ifndef USE_OLD_NS
}; // module TianShanIce
}; // module Application
#endif //!USE_OLD_NS

#endif // __ZQ_ChannelOnDemand_ChannelOnDemandEx_ICE__
