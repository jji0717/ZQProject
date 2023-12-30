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
// Ident : $Id: TsApplication.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsApplication.ICE $
// 
// 3     4/23/14 11:37a Hui.shao
// 
// 2     4/23/14 11:35a Hui.shao
// enh#18974 Purchase to export the playlist after provisioning
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 22    09-05-26 10:44 Hui.shao
// 
// 21    09-03-18 14:12 Hui.shao
// 
// 20    09-01-14 18:30 Hui.shao
// corrected some comments
// 
// 19    09-01-14 17:54 Hui.shao
// 
// 18    08-08-14 14:27 Hui.shao
// merge from 1.7.10
// 
// 18    08-07-07 15:40 Hui.shao
// 
// 17    07-12-20 13:55 Hui.shao
// 
// 16    07-12-20 13:49 Hui.shao
// added throwing SRM::InvalidResource for createPurchase()
// 
// 15    07-11-23 14:56 Hui.shao
// modified detach() to ami and carry more parameters, not misleads
// application service to access session object during detach()
// 
// 13    07-07-04 20:03 Hui.shao
// 
// 12    07-04-27 11:06 Hui.shao
// 
// 11    07-04-26 19:17 Hui.shao
// added Purchase::getParameters()
// 
// 10    07-03-13 12:37 Hui.shao
// renamed namespace Weiwoo to SRM, AccreditPath to Transport
// 
// 9     9/22/06 2:53p Hui.shao
// 
// 8     9/21/06 4:35p Hui.shao
// batch checkin on 20060921
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_Application_ICE__
#define __ZQ_TianShanIce_Application_ICE__

#include "TianShanIce.ICE"
#include "TsSRM.ICE"
#include "TsStreamer.ICE"

#include <Ice/Identity.ice>

module TianShanIce
{

module Streamer
{
	class Stream;
	class Playlist;
};

// -----------------------------
// namespace Application
// -----------------------------
/// Application represents the basic entry definition of a business application to TianShan architecture
module Application
{

// -----------------------------
// exception UriUnavailable
// -----------------------------
/// exception thrown when the requested URI is unavailable
exception UriUnavailable extends InvalidParameter
{
};

sequence < Streamer::PlaylistItemSetupInfo > PlaylistInfo;

// -----------------------------
// class Purchase
// -----------------------------
///@brief represents an order or purchase in the business logic reflect to TianShan session
///@note as of today, we recommands only one SRM session can be bound onto a purchase
["freeze:write"]
class Purchase
{
	/// list all the TianShan session instances bound on this purchase
	///@return a collection of session
	///@note as of today, only one session (the first one) is valid in a purchase
	["cpp:const", "freeze:read"] SRM::Session* getSession()
	throws
      ServerError;

	/// provision the purchase 
	///@throw InvalidParameter if failed to provision a purchase with the context of in the Session
	void provision()
	throws
      NotSupported,
	  InvalidParameter,
      ServerError;

	/// render this purchase onto a stream instance, normally the implementation needs to fill in
	/// the content to be streamed, if the stream instance is a playlist then the list should be built
	/// at this moment
	///@param[in] stream the destination stream session to render this purchase
	///@param[in] sess the session to render, especially reserved for 1 purchase to m session application
	///                implementation. purchase per session please pass NULL in this parameter
	///                NULL to use the default bound session instance
	///@throw InvalidParameter if the given stream instance or session cotext is invalid
	void render(Streamer::Stream* stream, SRM::Session* sess)
	throws
      NotSupported,
	  InvalidParameter,
      ServerError;

	/// detach a session from this purchase transaction. this will be called when a SRM::Session is end.
	/// For the 1-purchase to 1-session implementation, it can also be trusted as a destroy() request
	///@note all the resource bound on this session including the stream instance is no more usefull after this call
	///@note it is up to the application implementation to destroy this purchanse instance.
	///@param[in] sessId the id of the session to detach
	///@param[in] params some parameters may be interested by the application when detach
	///             - SYS_PROP(teardownReason) the end user's reason to teardown the session
	///             - SYS_PROP(terminateReason) the server-side reason to terminate the session
	["ami"] void detach(string sessId, Properties params)
	throws
      ServerError;

	/// Bookmark the current rendering position. The purchase may query the stream instance for the bookmarked
	/// position
	///@param[in] title  the name of this bookmark about to create
	///@param[in] stream the stream session of this purchase, especially reserved for 1 purchase to m session
	///                  application, for the single-session purchase, it is recommended to pass NULL
	void bookmark(string title, SRM::Session* sess)
	throws
      NotSupported,
      ServerError;

	/// Get application purchase oriented parameters
	///@param[in] expectedParameter a list of all the expected paramter names
	///@param[in] inParams          the input parameters and values for application service to refer
	///@param[out] outParams        the result parameters returned from the application
	///@return the number count of parameter names of outParams
	///@note all the parameter names are case sensitive when they are passed thru this method
	["cpp:const", "freeze:read"] int getParameters(StrValues expectedParameter, ValueMap inParams, out ValueMap outParams)
	throws
	  InvalidParameter,
      NotSupported,
      ServerError;
      
	/// query for the expected playlist of the purchase after provisioning
	///@return the playlist of the purchase
	["cpp:const", "freeze:read"] PlaylistInfo getPlaylistInfo()
		throws ServerError, NotSupported,
			   InvalidStateOfArt;
};

// -----------------------------
// service AppService
// -----------------------------
/// represents the basic entry points to a business logic in TianShan
interface AppService extends TianShanIce::BaseService
{
	/// create a purchase, invoked by BusinessRouter after it selected this application service.
	/// some variables may be accessed by this application service to build up a purchase context
	///@param sess the SRM session instance contains a collection of resources and session contexts
	///@param siteProperties for an application service designed for mulitple sites, the serverProperties may
	///                      contains a sub-configuration of a site such as the access to an asset management
	///@return the access to the purchase instance within the application
	///@throw InvalidParameter if failed to create a purchase with the context of in the Session
	///@throw SRM::InvalidResource if failed to create a purchase with the resource of in the Session
	Purchase* createPurchase(SRM::Session* sess, Properties siteProperties)
	throws
	  InvalidParameter,
	  SRM::InvalidResource,
      NotSupported,
      ServerError;
	
};

// -----------------------------
// class PublishPoint
// -----------------------------
/// a publish point represents a media asset that is available for on-demand ordering
["freeze:write"]
class PublishPoint
{
	::Ice::Identity ident;  ///< its name equals to unique channel name (or id), in lower-case

	string desc;         ///< the description of this PublishPoint
	::TianShanIce::Properties properties; ///< contains the per-PublishPoint properties
	int maxBitrate;       ///< the bitrate, in bps, reserved for the PublishPoint, no item in the PublishPoint should exceed this, 0- will use the server-wide default value based on the configuration
	string type;
	
	/// get the type of the PublishPoint
	///@return the type of the PublishPoint
	["cpp:const", "freeze:read"] idempotent string getType();

	/// get the PublishPoint name
	///@return the unique name of the PublishPoint
	["cpp:const", "freeze:read"] idempotent string getName();

	/// get the description of the PublishPoint
	///@return the description string of the PublishPoint
	["cpp:const", "freeze:read"] idempotent string getDesc();

	/// get the maxBitrate of the PublishPoint
	///@return the maxBitrate of the PublishPoint
	["cpp:const", "freeze:read"] int getMaxBitrate();

	/// set the maxBitrate of the PublishPoint	
	///@param[in] desc	the channel description
	void setMaxBitrate(int maxBitrate);

	/// update the PublishPoint propertis
	///@param[in] newProps the new property map to update
	void setProperties(::TianShanIce::Properties newProps);
	
	/// set the description of the PublishPoint	
	///@param[in] desc	the PublishPoint description
	void setDesc(string desc);

	/// get the PublishPoint properties
	///@return the property map bound on this PublishPoint
	["cpp:const", "freeze:read"] ::TianShanIce::Properties getProperties();
	
	/// destroy this PublishPoint
	void destroy();
	
	::TianShanIce::StrValues replicas; ///< a sequence of ContentStore NetId/Volume that this publish point is restricted onto

	/// restrict this publish point only on a set of specific ContentStore volumes,
	/// this will overwrite the existing restriction. The new restriction only affects the
	/// new sessions but will not take effect on those in-service sessions
	///@param[in] contentStoreNetIds a collection of NetIds of ContentStore, an empty collection
	///           means this channel is availalbe on ALL the ContentStore known by the system;
	///           by default, there is no restriction when a channel is newly created with
	///           PointPublisher::publish() or the methods derived from it
	///@throw TianShanIce::InvalidParameter if the given item name is invalid or not found
	///@throw TianShanIce::ServerError if internal server error occurs to replace the item
	void restrictReplica(::TianShanIce::StrValues contentStoreNetIds)
		throws TianShanIce::InvalidParameter, TianShanIce::ServerError;

	///query for the ContentStore volumes that this publish point was restricted onto
	///@return a collection of the NetId of contentStore instances
	///@throw TianShanIce::ServerError if internal server error occurs to replace the item
	["cpp:const", "freeze:read"] ::TianShanIce::StrValues listReplica()
		throws TianShanIce::InvalidParameter, TianShanIce::ServerError;
};

// -----------------------------
// struct PublishPointInfo
// -----------------------------
/// The publish point information exported from the publisher
struct PublishPointInfo
{
	string name;
	string type;
	TianShanIce::Properties params;
};

sequence < PublishPointInfo > PublishPointInfos;

// -----------------------------
// Service Inteface PointPublisher
// -----------------------------
/// The facet of the application service to external administration such as the management UI.
/// The interface is also used to publish a media asset for ordering
interface PointPublisher
{

	/// publish a new media point to be available for ordering
	///@param[in] name		 the publish point name, must be unique in this PointPublisher service
	///@param[in] maxBitrate the bitrate, in bps, reserved for the channel, no item in the publish point should exceed this
	///                      0 - will use the default value based on the configuration
	///@param[in] desc		 the publish point description
	///@return    the access to the PublishPoint object
	///@throw TianShanIce::InvalidParameter if the given publish point name exists or illegal
	///@throw TianShanIce::ServerError if internal server error occurs to publish the media point
	PublishPoint* publish(string name, int maxBitrate, string desc)
	throws
		InvalidParameter,
		NotSupported,
		ServerError;

	/// open a specific publish point
	///@param[in] name		the unqine publish point name
	///@return    the access to the PublishPoint object
	///@throw TianShanIce::InvalidParameter if the specific publish point doesn't exist
	///@throw TianShanIce::ServerError if internal server error occurs to open the publish point
	PublishPoint* open(string name)
	throws
		InvalidParameter,
		ServerError;

	///list all published channels
	///@return a collection of publish point unique names
	///@throw TianShanIce::ServerError if internal server error occurs to list the publish points
	::TianShanIce::StrValues list()
	throws
		ServerError;

	/// list the publish point information by on-demand publish point name
	///@param[in] paramNames specify the name of returning parameters of each transaction, known parameters are:
	///    - "desc" the description of the channel publish point
	///    - "maxBitrate" the max bitrate of the channel publish point
	///@return a collection of PublishPointInfo
	///@throw ServerError
	["amd", "ami", "cpp:const", "freeze:read"]  PublishPointInfos listPublishPointInfo(TianShanIce::StrValues paramNames)
	throws
		ServerError,
		NotImplemented;
};

// -----------------------------
// class OnDemandPublishPoint
// -----------------------------
/// a publish point represents a media asset that is available for on-demand ordering
["freeze:write"]
class OnDemandPublishPoint extends PublishPoint
{
	bool bAllowDemand; ///< flag to indicate whether current channel is usable
	string onDemandName;  ///< the channel name for on-demand order, this could be repeatable in the system

	///query for the on-demand publish point name
	///@return the publish point name available for on-demand order
	///@throw TianShanIce::ServerError if internal server error occurs to replace the item
	["cpp:const", "freeze:read"] string getOnDemandName()
		throws TianShanIce::InvalidParameter, TianShanIce::ServerError;

	///set the on-demand publish point name
	///@throw TianShanIce::ServerError if internal server error occurs to replace the item
	void setOnDemandName(string name);

	/// get the status of the channel
	///@return the status of the channel
	["cpp:const", "freeze:read"] bool isAvailableOnDemand();

	/// enable or disable this publish point from ordering although the entity still exist in the system\n
	/// when a publish point is initially created, it is enabled by default unless be called
	/// ChannelPublishPoint::enable(false);
	///@param bEnable false to disable this publish point
	void allowDemand(bool bEnable);
};

// -----------------------------
// class BroadcastPublishPoint
// -----------------------------
/// a publish point represents a media asset that is performing broadcast
["freeze:write"]
class BroadcastPublishPoint extends PublishPoint
{
	SRM::ResourceMap				resources;
	SRM::Session*                   session;

	/// requireResource
    /// add the required resource for this broadcast publish point
	/// The Session then instantiates a Resource object and adds it to its list. When the 
	/// negotiateResources operation is invoked, these will be the Resources that are negotiated.
    /// Each Resource is assigned a unique number by the Session.
	///@param type the enum value of ResouceType to identify which resource to be added
	///@param res the prepared Resource structure
    long requireResource(SRM::ResourceType type, SRM::Resource res)
	throws
	  InvalidStateOfArt,
	  NotSupported,
      InvalidParameter;

	/// get all the resource collection bound in the session
	///@return the resource collection
	["cpp:const", "freeze:read"] SRM::ResourceMap getResourceRequirement();

    /// The removeResource operation allows a previously defined Resource to be retired.
	///@param type the enum value of ResouceType to identify which resource to be removed
    void withdrawResourceRequirement(SRM::ResourceType type)
	throws
	  InvalidStateOfArt,
      InvalidParameter;

	/// setup the broadcasting publishing point, the server will setup a session on SRM and provision it
	void setup()
	throws
		InvalidStateOfArt,
		NotSupported,
		ServerError;

    /// Get the associated SRM::Session
    /// Once the channel has been set up, an SRM::Session would be associated. The
	///@return the access to the SRM::Session instance
    SRM::Session* getSession()
	throws
	  InvalidStateOfArt;

	/// start the broadcasting publishing point.
	void start()
	throws
		InvalidStateOfArt,
		NotSupported,
		ServerError;
	
	/// stop the broadcast publishing point.
	///@note stop just stop streaming on the associated session, the session and resource bound in the session
	/// will not be release util destroy this publish point
	void stop()
	throws
		InvalidStateOfArt,
		NotSupported,
		ServerError;
		
	/// get the channel up time in msec
	long getUpTime()
	throws
		InvalidStateOfArt;
};

// -----------------------------
// Inteface OnDemandPublisher
// -----------------------------
/// The facet of the application service to external administration such as the management UI.
/// The interface is also used to publish a media asset for ordering
interface OnDemandPublisher extends PointPublisher
{

	/// publish a new ondemand publish point to be available for ordering
	///@param[in] name		 the publish point name, must be unique in this PointPublisher service
	///@param[in] onDemandName	 the publish point name if the publish point name is repeatable, this will be used for matching the on-demand order
	///@param[in] maxBitrate the bitrate, in bps, reserved for the publish point, no item in the publish point should exceed this
	///                      0 - will use the default value based on the configuration
	///@param[in] props		 the optional properties of the publish point such as the DB access account and more
	///@param[in] desc		 the publish point description
	///@return    the access to the OnDemandPublishPoint object
	///@throw TianShanIce::InvalidParameter if the given publish point name exists or illegal
	///@throw TianShanIce::ServerError if internal server error occurs to publish the media point
	OnDemandPublishPoint* publishEx(string name, string onDemandName, int maxBitrate, TianShanIce::Properties props, string desc)
		throws TianShanIce::InvalidParameter, TianShanIce::ServerError;

	/// list the publish point information by on-demand publish point name
	///@param[in] onDemandName specify the onDemand name to list the channel publish points; empty or "*" will list all regardless on-demand name
	///@param[in] paramNames specify the name of returning parameters of each transaction, known parameters are:
	///    - "desc" the description of the publish point
	///    - "maxBitrate" the max bitrate of the publish point
	///@return a collection of PublishPointInfo
	///@throw ServerError
	["amd", "ami", "cpp:const", "freeze:read"] PublishPointInfos listOnDemandPointInfo(string onDemandName, TianShanIce::StrValues paramNames)
	throws
		ServerError,
		NotImplemented;
};

// -----------------------------
// struct ChannelItem
// -----------------------------
/// dictionary record,
struct ChannelItem
{
	string	contentName;	///< the content name

	string	broadcastStart; ///< the absolute start-time of this item on the source live channel, in GMT, e.g. 2006-10-10T20:00:00
	string	expiration;		///< the time that this item should be expired and no more availabe for ordering, empy for disable, in GMT, e.g. 2006-10-10T20:00:00

	bool	playable;		///< indicate whether this content is playable, reserved
	bool	forceNormalSpeed;	///< indicate whether the stream will turn back to normal speed once this content starts

	long	inTimeOffset;	///< if splicingEnabled is TRUE, this is the splicing point begin time; otherwise this is the content begin time offset
	long	outTimeOffset;	///< if splicingEnabled is TRUE, this is the splicing point end time; otherwise this is the content end time offset

	bool	spliceIn;		///< true if splice in is enabled
	bool	spliceOut;		///< true if splice out is enabled

	long	lastModified;	/// a time stamp indicates the last modified time
	
	::TianShanIce::Properties properties; ///< contains the per-item properties
};

// -----------------------------
// interface ChannelForPublishPoint
// -----------------------------
/// The interface to extend the definiton of PublishPoint to represent the channel and list operations
interface ChannelForPublishPoint
{
	/// get the sequence of ChannelItem::key in this channel
	///@return the sequence strings of ChannelItem::key
	["cpp:const", "freeze:read"] ::TianShanIce::StrValues getItemSequence();

	///find an item according to item name
	///@param[in] itemName		the name of the item to search, case insensitive
	///@return a copy of the item found
	///@remarks	if the item can not be found, returned item would be an invalid item with
	///         both key and name as empty string
	///@throw TianShanIce::InvalidParameter if the given item name is invalid or not found
	["cpp:const", "freeze:read"] ChannelItem findItem(string itemName)
		throws TianShanIce::InvalidParameter, TianShanIce::ServerError, TianShanIce::NotSupported;

	///append an item at the end of the channel
	///@param[in] newItem     the configuration ChannelItem structure of the new item
	///@note the key of the input newItem structure will be adjust by the service
	///@throw TianShanIce::InvalidParameter if the given item setup info is illegal
	///@throw TianShanIce::ClientError if the new adding exceeds the max number of items allowed
	///@throw TianShanIce::ServerError if internal server error occurs to append the item
	void appendItem(ChannelItem newItem)
		throws TianShanIce::ClientError, TianShanIce::ServerError, TianShanIce::NotSupported;

	///append an item at the end of the channel
	///The alias of the new item allow people to insert the same item into the list repeatly
	///but still is able to identify it in the list
	///@param[in] newItem     the configuration ChannelItem structure of the new item
	///@param[in] alias       the alias of the new item that is being inserted, empty means no alias
	///@note the key of the input newItem structure will be adjust by the service
	///@throw TianShanIce::InvalidParameter if the given item setup info is illegal
	///@throw TianShanIce::ClientError if the new adding exceeds the max number of items allowed
	///@throw TianShanIce::ServerError if internal server error occurs to append the item
	void appendItemAs(ChannelItem newItem, string alias)
		throws TianShanIce::ClientError, TianShanIce::ServerError, TianShanIce::NotSupported;

	///insert an item of the channel before a specific list item
	///@param[in] atItemName  the name of the item to insert before 
	///@param[in] newItem     the configuration ChannelItem structure of the new item
	///@note the key of the input newItem structure will be adjust by the service
	///@throw TianShanIce::InvalidParameter if the given item setup info is illegal
	///                                  or the atItemName not found
	///@throw TianShanIce::ClientError if the new adding exceeds the max number of items allowed
	///@throw TianShanIce::ServerError if internal server error occurs to insert the item
	void insertItem(string atItemName, ChannelItem newItem)
		throws TianShanIce::ClientError, TianShanIce::ServerError;

	///insert an item of the channel with alias before a specific list item. 
	///The alias of the new item allow people to insert the same item into the list repeatly
	///but still is able to identify it in the list
	///@param[in] atItemName  the name of the item to insert before 
	///                       must be the alias if the at-item has alias when it was inserted
	///@param[in] newItem     the configuration ChannelItem structure of the new item
	///@param[in] alias       the alias of the new item that is being inserted, empty means no alias
	///@note the key of the input newItem structure will be adjust by the service
	///@throw TianShanIce::InvalidParameter if the given item setup info is illegal
	///                                  or the atItemName not found
	///@throw TianShanIce::ClientError if the new adding exceeds the max number of items allowed
	///@throw TianShanIce::ServerError if internal server error occurs to insert the item
	void insertItemAs(string atItemName, ChannelItem newItem, string newName)
		throws TianShanIce::ClientError, TianShanIce::ServerError;

	///replace an item of the channel by the item name
	///@param[in] oldItemName the name of the old item to replace.
	///                       must be the alias if the old item has alias when it was inserted
	///@param[in] newItem     the configuration ChannelItem structure of the new item
	///@note the key of the input newItem structure will be adjust by the service
	///@throw TianShanIce::InvalidParameter if the given item setup info is illegal
	///                                  or the oldItemName not found
	///@throw TianShanIce::ServerError if internal server error occurs to replace the item
	void replaceItem(string oldItemName, ChannelItem newItem)
		throws TianShanIce::InvalidParameter, TianShanIce::ServerError;

	///remove an item from the channel
	///@param[in] itemName the itemName to remove
	///                     must be the alias if the item has alias when it was inserted
	///@throw TianShanIce::InvalidParameter if the given item name is invalid or not found
	///@throw TianShanIce::ServerError if internal server error occurs to replace the item
	void removeItem(string itemName)
		throws TianShanIce::InvalidParameter, TianShanIce::ServerError;
};

};
};

#endif // __ZQ_TianShanIce_Application_ICE__
