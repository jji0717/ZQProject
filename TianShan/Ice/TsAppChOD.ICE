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
// Ident : $Id: ChannelOnDemand.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsAppChOD.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-01-15 14:16 Hui.shao
// renamed from the previous ChannelOnDemand.ICE, add it as a part of
// basicTianShan SDK
// 
// 46    08-12-26 18:12 Hui.shao
// replaced nonmutating
// 
// 45    08-11-18 10:45 Haoyuan.lu
// 
// 44    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 43    08-03-05 12:15 Guan.han
// 
// 43    08-03-04 18:53 Guan.han
// 
// 42    08-01-15 15:52 Guan.han
// added listChannelInfo() shell
// 
// 41    08-01-15 15:40 Hui.shao
// 
// 40    08-01-07 18:48 Guan.han
// 
// 39    08-01-02 11:42 Guan.han
// 
// 38    07-12-24 11:53 Guan.han
// 
// 37    07-12-12 12:07 Guan.han
// 
// 35    07-12-10 19:17 Hui.shao
// 
// 34    07-11-26 14:47 Xiaohui.chai
// update interface of ChannelPurchase::destroy 
//
// 33    07-03-14 12:32 Hongquan.zhang
// 
// 32    06-12-27 12:46 Guan.han
// 
// 31    06-12-26 12:51 Guan.han
// 
// 30    06-12-26 10:37 Guan.han
// 
// 29    06-12-22 11:20 Guan.han
// 
// 28    06-12-11 17:32 Jie.zhang
// 
// 27    06-12-01 16:28 Jie.zhang
// Log changed and add thread pool to process add/insert/replace/remove
// operation
// 
// 26    06-11-23 14:20 Jie.zhang
// 
// 25    06-11-20 16:39 Jie.zhang
// add getCurrentPosition
// 
// 24    06-11-13 14:16 Jie.zhang
// change the name of seek to seekToPosition
// 
// 23    06-11-13 10:37 Jie.zhang
// Add seek command
// 
// 22    06-10-20 17:49 Jie.zhang
// 
// 21    06-10-16 18:56 Jie.zhang
// 
// 20    10/16/06 4:46p Hui.shao
// make expireItem() as removeItem()
// 
// 19    10/10/06 11:32a Hui.shao
// 
// 18    10/09/06 4:39p Hui.shao
// added exception throws
// 
// 17    06-10-09 14:56 Jie.zhang
// description
// 
// 16    06-10-08 16:06 Jie.zhang
// 
// 15    06-09-30 15:27 Jie.zhang
// 
// 14    06-09-30 13:47 Jie.zhang
// 
// 13    06-09-26 11:18 Jie.zhang
// 
// 12    06-09-20 15:32 Hui.shao
// 
// 11    06-09-20 12:13 Jie.zhang
// add setDesc to ChannelPublishPoint
// 
// 10    06-09-20 10:44 Jie.zhang
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShanIce/Ice --output-dir .. ..\$(InputName).ice
// outputs:

#ifndef __ZQ_TianShanIce_AppChOD_ICE__
#define __ZQ_TianShanIce_AppChOD_ICE__

#include "TsApplication.ICE"
#include "TsStreamer.ICE"
#include "TsSRM.ICE"

module TianShanIce
{

// -----------------------------
// namespace Application
// -----------------------------
/// Application represents the basic entry definition of a business application to TianShan architecture
module Application
{

///@briedf ChannelOnDemand represents an application under TianShanIce architecture.
/// The application allow people publish a channel avaible for on-demand ordering, and can accept real-time ingestion from live broast channel
module ChannelOnDemand
{

// -----------------------------
// class ChannelPublishPoint
// -----------------------------
/// a publish point of channel represents a channel which is available for on-demand ordering
["freeze:write"] 
class ChannelPublishPoint extends ::TianShanIce::Application::OnDemandPublishPoint implements ::TianShanIce::Application::ChannelForPublishPoint
{
	::TianShanIce::StrValues itemSequence; ///< a sequence of ChannelItem::key to line the items up in this channel
};

// -----------------------------
// Service ChannelPublisher
// -----------------------------
/// The facet of the ChannelOnDemand service to external channel administration such as the management UI.
/// The interface is also used to publish a channel for OnDemand ordering
interface ChannelPublisher extends ::TianShanIce::Application::OnDemandPublisher
{
};

class ChannelOnDemandPurchase; // forward declaration

// -----------------------------
// class ChannelPurchase
// -----------------------------
/// represend a purchase in ChannelOnDemand application
///@note the purchase itself doesn't keep the sequence of contents, instead, it depends on the associated
///streamer's playlist to help it on this
["freeze:write"] 
class ChannelPurchase extends ::TianShanIce::Application::Purchase
{
	///< unque key of this object
	Ice::Identity ident;

	///< access to the playlist of streamserver
	::TianShanIce::Streamer::Playlist* playlist;

	///< the id of playlist
	string				playlistId;

	///< access to server session
	TianShanIce::SRM::Session* weiwoo;

	// the channel publish name
	string chlPubName;

	///< indicates whether to authorization on ia component or not.
	bool			bAuthorize;

	// get stream proxy
	::TianShanIce::Streamer::Playlist* getPlaylist();

	/// get playlist id
	string getPlaylistId();

	/// get channel name
	string getChannelName();

	/// get if need sync channel
	bool getIfNeedSyncChannel();

	// get if need authorizate on ia component
	bool getIfNeedAuthorize();

	/// destroy this purchase
	///@param[in] params some interested parameters when destroy
	///@note this operation is responsible to destroy all related PurchaseItemAssoc associated with this purchase
	void destroy(::TianShanIce::Properties params)
	throws
	  TianShanIce::InvalidParameter,
      TianShanIce::NotSupported,
      TianShanIce::ServerError;
};

// -----------------------------
// Service ChannelOnDemandApp
// -----------------------------
/// the facet of the ChannelOnDemand service to TianShanIce Architecture
interface ChannelOnDemandApp extends ::TianShanIce::Application::AppService
{
	/// create a purchase by client request, invoked directly by CREdget such as a RTSP request handler.
	/// some variables may be accessed by this application service to build up a purchase context
	///@param clientProperties the parameters forwarded from the client request
	///@param siteProperties for an application service designed for mulitple sites, the serverProperties may
	///                      contains a sub-configuration of a site such as the access to an asset management
	///@return the access to the purchase instance within the application
	ChannelPurchase* createPurchaseByCR(::TianShanIce::Properties clientProperties, ::TianShanIce::Properties siteProperties)
	throws
	  TianShanIce::InvalidParameter,
      TianShanIce::NotSupported,
      TianShanIce::ServerError;
};

// modules
};
};
};

#endif // __ZQ_TianShanIce_AppChOD_ICE__
