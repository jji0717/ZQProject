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
// Ident : $Id: TsAppBcast.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsAppBcast.ICE $
// 
// 2     10/21/14 3:31p Li.huang
// add bcastPublishpoint renew function
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 4     09-06-10 11:27 Li.huang
// add filteritem function
// 
// 3     09-03-18 14:13 Hui.shao
// 
// 2     09-01-15 15:11 Hui.shao
// 
// 1     09-01-14 17:54 Hui.shao
// created for the definiton of broadcast channel applications
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_AppBcast_ICE__
#define __ZQ_TianShanIce_AppBcast_ICE__

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

// -----------------------------
// namespace Broadcast
// -----------------------------
/// Broadcast represents some schedule-driven application definitions 
module Broadcast
{

// -----------------------------
// class BcastPublishPoint
// -----------------------------
/// a publish point of channel represents a channel which is available for on-demand ordering
["freeze:write"] 
class BcastPublishPoint extends ::TianShanIce::Application::BroadcastPublishPoint implements ::TianShanIce::Application::ChannelForPublishPoint
{
	::TianShanIce::StrValues itemSequence; ///< a sequence of ChannelItem::key to line the items up in this channel
	
	/// The expiration of this BcastPublishPoint in msec and GMT, can be renew-ed thru renew().
	/// It always starts with 0 when BcastPublishPoint is created until renew() is invoked. Once the expiration is
	/// a non-zero, the BcastPublishPoint becomes volatilizable. Whenever the expiration is met, the BcastPublishPoint will
	/// destory
	long	expiration;
	
	/// renew the BcastPublishPoint's lifecycle if this BcastPublishPoint is volatilizable
	/// @param TTL time-to-live for this BcastPublishPoint in msec from now
	void renew (long TTL);
};

/// a collection of the access to ::TianShanIce::Application::BroadcastPublishPoint
sequence < ::TianShanIce::Application::BroadcastPublishPoint* > NVODSupplementalChannels;

// -----------------------------
// class NVODPublishPoint
// -----------------------------
/// a channel publish point represents a NVOD channel as an aggregator of its main and suplemental channels
/// this NVODChannelPublishPoint itself also is the entry to access the main NVOD channel
["freeze:write"] 
class NVODChannelPublishPoint extends BcastPublishPoint
{
	NVODSupplementalChannels supplementalChannels; ///< the access to the suplemental channels of this NVOD channel
	
	 /// returns the reference to the suplemental channels of a NVOD channel
	 /// @return a collection of the access to BroadcastPublishPoint
	["cpp:const", "freeze:read"] NVODSupplementalChannels getSupplementalChannels();
};

sequence < ChannelItem > ChannnelItems;
// -----------------------------
// Service BcastPublisher
// -----------------------------
/// The facet of the ChannelOnDemand service to external channel administration such as the management UI.
/// The interface is also used to publish a channel for OnDemand ordering
interface BcastPublisher extends ::TianShanIce::Application::PointPublisher
{
	/// publish a new channel to broadcast
	///@param[in] name		 the channel name, must be unique in this ChannelPublisher service
	///@param[in] resourceRequirement the resource requirement of the channel
	///@param[in] props		 the optional properties of the channel such as the DB access account and more
	///@param[in] desc		 the channel description
	///@return    the access to the BcastPublishPoint object
	///@throw TianShanIce::InvalidParameter if the given channel name exists or illegal
	///@throw TianShanIce::ServerError if internal server error occurs to publish the channel
	BcastPublishPoint* createBcastPublishPoint(string name, SRM::ResourceMap resourceRequirement, TianShanIce::Properties props, string desc)
	throws
		InvalidParameter,
		NotSupported,
		ServerError;
	
	/// publish a new NVOD channel
	///@param[in] name		 the channel name, must be unique in this ChannelPublisher service
	///@param[in] resourceRequirement the resource requirement of the main channel
	///@param[in] iteration	 the iteration count of the channel within this NVOD channel
	///@param[in] interval	 the time interval in msec between two subchannels
	///@param[in] props		 the optional properties of the channel such as the DB access account and more
	///@param[in] desc		 the channel description
	///@return    the access to the NVODChannelPublishPoint object
	///@throw TianShanIce::InvalidParameter if the given channel name exists or illegal
	///@throw TianShanIce::ServerError if internal server error occurs to publish the channel
	NVODChannelPublishPoint* createNVODPublishPoint(string name, SRM::ResourceMap resourceRequirement, short iteration, int interval, TianShanIce::Properties props, string desc)
	throws
		InvalidParameter,
		NotSupported,
		ServerError;
		
	/// add a filter item 
	///@param[in] newItem     the configuration ChannelItem structure of the new item
	///@throw TianShanIce::InvalidParameter if the given item setup info is illegal
	///@throw TianShanIce::ServerError if internal server error occurs to add the item	
	void addFilterItem(ChannelItem newItem);
	
	///remove a filter item 
	///@param[in] itemName the itemName to remove
	///                     must be the alias if the item has alias when it was inserted
	///@throw TianShanIce::InvalidParameter if the given item name is invalid or not found
	///@throw TianShanIce::ServerError if internal server error occurs to replace the item
	void removeFilterItem(string itemName);
	
	/// list all filter item
	///@return a collection of ChannelItem
	///@throw ServerError
	["amd", "ami", "cpp:const", "freeze:read"]ChannnelItems listFilterItems();
};

// -----------------------------
// class BcastPurchase
// -----------------------------
/// represend a purchase in ChannelOnDemand application
///@note the purchase itself doesn't keep the sequence of contents, instead, it depends on the associated
///streamer's playlist to help it on this
["freeze:write"] 
class BcastPurchase extends ::TianShanIce::Application::Purchase
{
	///< access to own publish point
	BcastPublishPoint* bcastPP;
};

};
};
};

#endif // __ZQ_TianShanIce_AppBcast_ICE__
