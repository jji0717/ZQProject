// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : $Id: TsEdgeResource.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsEdgeResource.ICE $
// 
// 4     6/06/16 5:21p Li.huang
// 
// 4     6/06/16 10:08a Li.huang
// 
// 3     11/27/13 2:30p Ketao.zhang
// add AllocationIds
// 
// 2     7/01/13 2:01p Bin.ren
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 9     09-12-29 15:25 Li.huang
// 
// 8     09-12-18 9:36 Li.huang
// restore
// 
// 8     09-12-10 18:00 Li.huang
// remove restrictServiceGroups() and Add linkServiceGroups(),
// unlinkServiceGroups() interface
// 
// 7     09-11-02 11:33 Hui.shao
// 
// 6     09-10-30 11:55 Li.huang
// add findRFPortsByServiceGroup() and listServiceGroups() interface
// 
// 5     09-07-27 13:18 Hui.shao
// 
// 4     09-03-19 17:11 Hui.shao
// 
// 3     09-03-05 19:42 Hui.shao
// corrected some typo
// 
// 2     09-03-04 21:32 Hui.shao
// 
// 1     09-02-26 17:50 Hui.shao
// initial created
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_EdgeResource_ICE__
#define __ZQ_TianShanIce_EdgeResource_ICE__

#include <TsSRM.ICE>
module TianShanIce
{

// -----------------------------
// forward declarations
// -----------------------------

// -----------------------------
// namespace EdgeResource
// -----------------------------
/// EdgeResource is tightly share the definitions with SRM
module EdgeResource
{

class Allocation;
sequence <Allocation* > Allocations; ///< a collection of Allocation instances
sequence <ObjectInfo>  ObjectInfos;
#define EdgeDeviceInfo     ObjectInfo
#define EdgeChannelInfo    StatedObjInfo
#define AllocationInfo     StatedObjInfo

// -----------------------------
// class EdgePort
// -----------------------------
/// presents a RF port in QAM. Normally it is an allocation specified for
struct EdgePort
{
	short Id;                 ///< RF Port Id in the QAM
	int  powerLevel;          ///< power level in dbmv
	
/*
	/// the byte to sepcify to modulation used for this channel.
	/// @li     0x00 - Unknown modulation format
	/// @li     0x01-0x05 - reserved
	/// @li     0x06 - QAM 16 modulation format
	/// @li     0x07 - QAM 32 modulation format
	/// @li     0x08 - QAM 64 modulation format
	/// @li     0x09-0x0b - reserved
	/// @li     0x0c - QAM 128 modulation format
	/// @li     0x0d-0x0f - reserved
	/// @li     0x10 - QAM 256 modulation format
	/// @li     0x11-0xff - reserved
	short modulationFormat;
	
	/// The interleaving depth of the interleaver.
	/// @li     0x01 - fecI128J1,
	/// @li     0x02 - fecI128J2,
	/// @li     0x03 - fecI64J2,
	/// @li     0x04 - fecI128J3,
	/// @li     0x05 - fecI32J4,
	/// @li     0x06 - fecI128J4,
	/// @li     0x07 - fecI16J8,
	/// @li     0x08 - fecI128J5,
	/// @li     0x09 - fecI8J16,
	/// @li     0x0a - fecI128J6,
	/// @li     0x0b - fecI128J7,
	/// @li     0x0c - fecI128J8
	short interleaverMode;
	
	/// Interleaver level for FEC encoding.
	/// @li     0x01 = level 1,
	/// @li     0x02 = level 2
	short interleaverLevel;
*/	
	/// the resource to reserve to convey information to the set top about tuning information
	/// the values in the resource struct here are all rsAssigned and raMandatoryNonNegotiable
	/// @li "modulationFormat" the byte to sepcify to modulation used for this channel.
	///      0x00 - Unknown modulation format
	///      0x01-0x05 - reserved
	///      0x06 - QAM 16 modulation format
	///      0x07 - QAM 32 modulation format
	///      0x08 - QAM 64 modulation format
	///      0x09-0x0b - reserved
	///      0x0c - QAM 128 modulation format
	///      0x0d-0x0f - reserved
	///      0x10 - QAM 256 modulation format
	///      0x11-0xff - reserved
	/// @li "symbolRate" the integer to sepcify to the symbol rate
	/// @li "interleaveDepth" the byte to sepcify the used interleaving, reserved
	/// @li "modulationMode" the byte to sepcify the used modulation mode, reserved for now
	///      0x00 - no modulation
	///      0x01-0x08 - reserved
	///      0x09 - QAM 4 modulation mode
	///      0x0a-0xff - reserved
	/// @li "FEC" the byte to sepcify the type of forward error correction
	///      0x00 - FEC transmission system
	///      0x01 - FEC DAVIC
	///      0x02-0xff - reserved
	SRM::Resource resAtscModulationMode;

	/// Describes the channel used for tuning sessions.
	/// @li "edgeDeviceName" the name of the edge device
	/// @li "edgeDeviceIP" the input IP of the edge device, 1-to-1 sequentially map to edgeDeviceName items if they exist
	/// @li "edgeDeviceZone" the zone of the edge device
	SRM::Resource resPhysicalChannel;
};

// -----------------------------
// class EdgeChannel
// -----------------------------
/// presents a frequency on a RF port in QAM

sequence <string> AllocationIds;
["freeze:write"]  
class EdgeChannel
{
	Ice::Identity ident;      ///< name unique key to locate the channel, ident.name equals channelId formatted as <deviceName>/<RFId>.<ChNum>
//	string deviceName;        ///< name index to edge device that belongs to
//	short  RFId;              ///< seq index to the RF port within the EdgeDevice
//	short  chNum;             ///< the channel number within the RF port

	TianShanIce::State deviceState; ///< the state of the channel according to the edge device report
	long  stampLastUpdated;   ///< the timestamp when the edge device confirms the state of the channel
	
	bool  enabled;            ///< false if don't wish to make allocation on the channel per administraton
	
	EdgePort ePort;           ///< the attributes of the edge port
	long freqRF;              ///< RF frequency in Hz
		
	int   TSID;
	int   NITPID;             
	
	int   startUDPPort;
	byte  udpPortStepByPn;
	int   startProgramNumber;
	int   maxSessions;
	
	long  lowBandwidthUtilization;
	long  highBandwidthUtilization;
	
	int   intervalPAT;
	int   intervalPMT;
	
	Properties  metaData;   ///< the metadata of the channel
	
	/// enable the channel available for allocating
	///@param[in] toEnable true if the channel is available for allocating
	///@return the enabled/disabled status of the channel
	bool enable(bool toEnable);
	
	/// get the unique Id of this RF channel
	///@return the Id of this RF channel
	["cpp:const", "freeze:read"] string getId();

	/// get the state of this RF channel according the device status
	///@return the channel state
	["cpp:const", "freeze:read"] TianShanIce::State getState();

	/// get the hiberarchy of this RF channel, including the QAM and RF port it belongs to
	///@param[out] deviceName the unique device name of the QAM
	///@param[out] EdgePort the port Id within the QAM
	///@param[out] chNum the channel seq number of the RF port
	["cpp:const", "freeze:read"] void getHiberarchy(out string deviceName, out short EdgePort, out short chNum);
	
	/// get the information of this channel with the expected metadata
	///@param[in] expectedMetaData the names of the expected metadata wish to know
	///@return the channel info structed via EdgeChannelInfo
	["cpp:const", "freeze:read"] EdgeChannelInfo getInfo(StrValues expectedMetaData);

	/// get the committed or per-reserved allocation on this channel
	///@return a collection of allocation instances
	["cpp:const", "freeze:read"] Allocations getAllocations();
	["cpp:const", "freeze:read"] AllocationIds getAllocationIds();
	
	
};

sequence <EdgeChannelInfo > EdgeChannelInfos; ///< a collection of ChannelInfo
sequence <EdgePort > EdgePortInfos; ///< a collection of EdgePort

// dictionary<int, Variant> ServiceGroupsMap; ///< map of id to EdgePort
dictionary<string, Variant> RoutesMap; ///< map of rount name to EdgePort

// -----------------------------
// class EdgeDevice
// -----------------------------
/// presents QAM device that has a unique interface to communicate with
["freeze:write"] 
class EdgeDevice
{
	Ice::Identity  ident;        ///< the unique key of the QAM device
	string         deviceZone;
	string         type;
	string         vendor;
	string         model;
	string         desc;
	
	string         tftpUrl;   ///< the TFTP URL of the device
	string         adminUrl;  ///< the URL to the device's administration interface
	
	/// get the unique name of the QAM device
	///@return the unique name of the QAM device
	["cpp:const", "freeze:read"] idempotent string getName();
	
	/// get the information of the QAM device
	///@param[in] expectedMetaData the names of the expected metadata wish to know
	///@return the device info structed via EdgeDeviceInfo
	["cpp:const", "freeze:read"] EdgeDeviceInfo getInfo(StrValues expectedMetaData);
	
	/// get the known RF port Ids of the edge device
	///@return all the known RF ports of the edge device
	["cpp:const", "freeze:read"] EdgePortInfos listEdgePorts();
	
	/// list the channels of this edge device
	///@param[in] EdgePortId to specify of which RF port to list the channels, negative if wish to list all
	///           the channels of the QAM regardless RF ports
	///@param[in] expectedMetaData the names of the expected metadata wish to know
	///@param[in] enabledOnly true if wish to filter out those disabled channels
	///@return a collection of EdgeChannelInfo
	["amd", "cpp:const", "freeze:read"] EdgeChannelInfos listChannels(short EdgePortId, StrValues expectedMetaData, bool enabledOnly);
	
	/// populate the channels of a given RF port
	///@param[in] EdgePortId to specify of which RF port to populate the channels
	///@param[in] expectedMetaData the names of the expected metadata wish to know
	///@param[in] enabledOnly true if wish to filter out those disabled channels
	///@return a collection of EdgeChannelInfo to describe all the channels of the given RF port, including 
	///         the newly populated channels
	///@throw InvalidParameter if the given EdgePortId is out of range in the edge device
	["amd", "ami"] EdgeChannelInfos populateChannels(short EdgePortId)
	throws
		InvalidParameter;
		
	/// restricts the service group that can employ this channel
	///@param[in] servicegroups a collection of servicegroups that are allowed to employ this channel
	///@note empty servicegroups means reset the restriction and allow all
//TODO	void restrictServiceGroups(short EdgePortId, IValues servicegroups);
	//void linkServiceGroups(short EdgePortId, int servicegroup, Variant freqs);
	//void unlinkServiceGroups(short EdgePortId, int servicegroup);
	void linkRoutes(short EdgePortId, string routeName, Variant freqs)
	throws
		InvalidParameter;
	void unlinkRoutes(short EdgePortId, string routeName)
	throws
		InvalidParameter;
	
	///get the restricted servicegroup of the channel
	///@return the restricted servicegroup of this channel
	///@sa restrictServiceGroups()
	// ["cpp:const", "freeze:read"] ServiceGroupsMap getServiceGroupRestriction(short EdgePortId);
	["cpp:const", "freeze:read"] RoutesMap getRoutesRestriction(short EdgePortId);
	

	/// open a RF channel of the QAM
	///@param[in] EdgePortId to specify of which RF port
	///@param[in] chNum to specify the seq number of channel in the RF port
	///@return the access to channel instance, NULL if the channel is not available
	///@throw InvalidParameter if the given EdgePortId or chNum is invalid
	EdgeChannel* openChannel(short EdgePortId, short chNum);
	
	/// remove the edge device from the management of this Edge Resource Management
	///@throw InvalidStateOfArt if some allocating is being processed
	void destroy()
	throws
	  InvalidStateOfArt,
      NotSupported,
	  ServerError;
};

// -----------------------------
// callback AllocationOwner
// -----------------------------
/// represents a manager of the sessions within SRM
interface AllocationOwner
{
	/// If an allocation is not renewed in time, when such an allocation is expiring, ERM will check the owner
	/// for the confirmation to expire the allocation. 0 or negative from the owner will expire the allocation
	/// imediately, otherwise, the allocation would be renewed.
	///@param[in] ownerContextKey the context key of the owner given when allocate
	///@param[in] alloc the access to this allocation
	///@return the time in msec to renew this allocation, 0 or negative will lead the allocation expire immediately
	///@note a certern Ice::ObjectNotExists exception when accessing the AllocationOwner instance will also lead
	///      to the procedure of OnAllocationExpiring() =0
	["ami"] int OnAllocationExpiring(string ownerContextKey, Allocation* alloc);
};

// -----------------------------
// class Allocation
// -----------------------------
///@brief presents an allocation for edge resource
["freeze:write"] 
class Allocation
{
	Ice::Identity  ident;          ///< the unique key of the allocation
	
	TianShanIce::State state;    ///< state of the allocation

	SRM::ResourceMap resources;  ///< resource bound on this allocation
	
	AllocationOwner* owner;      ///< entry to notify the allocation owner
	string ownerKey;             ///< the context key of owner given when allocating
	
	int       udpPort;           ///< the input UDP port to the QAM
	int       programNumber;     ///< the output MPEG program number
	float     maxJitter;
	string    sourceIP;          ///< the source IP of the input traffic
	long      bandwidth;         ///< the reserved bandwidth in Kbps of the allocation
	
	long      stampCreated;      ///< the timestamp of the allocation has been created
	long      stampProvisioned;  ///< the timestamp of the allocation has been provisioned
	long      stampCommitted;    ///< the timestamp of the allocation has been committed
	
	long      expiration;        ///< The expiration data-time of this allocation, can be renew-ed thru API renew().

	/// get the allocation id
	///@ return the allocation id in a compact string format
	["cpp:const", "freeze:read"] idempotent string getId(); 

	/// get the allocation state
	///@ return state of the allocation
	["cpp:const", "freeze:read"] TianShanIce::State getState(); 

	/// access to the owner who created and maintainance the allocation
	["cpp:const", "freeze:read"] AllocationOwner* getOwner(); 

	/// get the access to the channel where this allocation is work on
	["cpp:const", "freeze:read"] EdgeChannel* getChannel()
	throws
      InvalidStateOfArt,
      ServerError;
	
	/// get the Id to the channel where this allocation is work on
	["cpp:const", "freeze:read"] AllocationInfo getInfo(); 

	///	destroy the allocation and free the resource
	void destroy()
		throws ServerError;

	/// addResource
    /// The addResourceEx operation allows the guts of a Resource to be sent to a Session. 
	/// The Session then instantiates a Resource object and adds it to its list. When the 
	/// negotiateResources operation is invoked, these will be the Resources that are negotiated.
    /// Each Resource is assigned a unique number by the Session.
	///@param type the enum value of ResouceType to identify which resource to be added
	///@param res the prepared Resource structure
    long addResource(SRM::ResourceType type, SRM::Resource res)
	throws
      SRM::InvalidResource,
	  NotSupported,
      InvalidParameter;

	/// get all the resource collection bound in the session
	///@return the resource collection
	["cpp:const", "freeze:read"] SRM::ResourceMap getResources();

    /// The removeResource operation allows a previously defined Resource to be retired.
	///@param type the enum value of ResouceType to identify which resource to be removed
    void removeResource(SRM::ResourceType type);

    /// perform the provision for the session. At this step, the session will be associated with
	/// a purchase of application, if necessary resouce requirement or private data have not been
	/// collected before provision() is invoked, this operation will be rejected
	///@param[in] maxChannelCandidates the max number of the channel candidates, quit validating if this has been met
	///@param[in] reserveNow true if wish the PN to be reserved at this step. When this reservation happens,
	///           maxChannelCandidates is force to be a smaller number per the configuration of EdgeRM
	///@note resource rtURI is a must to associate the purchase, all other necessary resource or
	///      runtime parameters are up to the application logic
	///@note the destination state of the session will be turned to stProvisioned if succeeded
	///@throw InvalidStateOfArt if try to provsion from a illeagal state
	///@throw InvalidResource if a required resource has not been collected piror to this operation
	///@throw InvalidParameter if a required parameter has not been collected piror to this operation
	///@throw InvalidStateOfArt if the current state of session does not perform this operation
    void provision(short maxChannelCandidates, bool reserveNow)
	throws
      SRM::InvalidResource,
	  InvalidStateOfArt,
      InvalidParameter,
      NotSupported,
	  ServerError;

	/// The negotiateResources operation, at the state of stProvisioned, is intended to initiate resource 
    /// negotiation with extended module. 
	///@throw InvalidStateOfArt if try to negotiate resources from a illeagal state
	["amd"] void negotiateResources()
	throws
	  InvalidStateOfArt,
	  SRM::OutOfResource,
      NotSupported,
	  ServerError;

    /// prepare the session to be ready for serving client. At this step, the session will
	///@li certain all the negociateable resource requirement and perform resource selection.
	///@li create a stream session on a picked StreamService and apply the selected Transport path
	///@li ask application to render the purchase on to the stream session
	///@note the destination state of the session will be turned to stInService if succeeded
	///@note error occurs after any of certain resource allocation will lead the session to a 
	///      stOutOfService state
	///@note no resource requirement collection thru addResource() will be allowed after this
	///@throw InvalidResource if a required resource has not been collected piror to this operation
	///@throw InvalidStateOfArt if try to serve from a illeagal state
	///@throw InvalidParameter if a required parameter has not been collected piror to this operation
	///@throw InvalidStateOfArt if the current state of session does not perform this operation
	///@throw OutOfResource if the system is running out of resource to serve this session
    void serve()
	throws
	  InvalidStateOfArt,
      SRM::InvalidResource,
	  SRM::OutOfResource,
      InvalidParameter,
      NotSupported,
	  ServerError;

	/// renew the allocation's lifecycle
	/// @param TTL time-to-live for this allocation in msec from now
	///@throw InvalidStateOfArt if try to renew from a illeagal state
	void renew (long TTL);

};

sequence <EdgeDeviceInfo > EdgeDeviceInfos; ///< a collection of EdgeDeviceInfos
sequence <AllocationInfo > AllocationInfos; ///< a collection of AllocationInfos

// -----------------------------
// Service EdgeResouceManager
// -----------------------------
/// represents the service interface entry of edge resources management and allocation
interface EdgeResouceManager extends TianShanIce::BaseService, ReplicaQuery
{
	/// create a Allocation with initail resource requriement
	///@param[in] assetUri	the asset URI required in the session
	///@return    a session instance, NULL if failed
	///@note	  resouce requirement must be specified by calling Session::addResource()
	Allocation* createAllocation(SRM::ResourceMap resRequirement, int TTL, AllocationOwner* owner, string ownerContextKey)
	throws ServerError;

	/// open an existing allocation with a given id
	///@param[in] Id	the allocation id
	///@return    access to the allocation instance, NULL if failed
	Allocation* openAllocation(string Id)
	throws ServerError;

	["amd", "ami", "cpp:const"] AllocationInfos listAllocations(string deviceName, short portId, short chNum, StrValues expectedMetaData);
	
	["cpp:const"]  EdgeDeviceInfos listDevices(StrValues expectedMetaData);
	["amd", "ami"]  EdgeDevice* addDevice(string name, string deviceGroup, string vendor, string model, string adminUrl, string tftpUrl);
	EdgeDevice* openDevice(string name);
	
	/// list all the RF ports that connects to the specified service group Id
	///@param[in] serviceGrpId the service group to lookup
	///@return a collection of EdgePortInfo sorted by RF ident
	//EdgePortInfos findRFPortsByServiceGroup(int serviceGrpId);
	EdgePortInfos findRFPortsByRouteName(string routeName);

	/// list all servicegroups known in the system
	///@return a collection of object info sorted by id, the ident.name of ObjectInfo <serviceGroupId>, props["RFPortCount"] equals
	///        the count of RF ports linked to the servicegroup
	//ObjectInfos listServiceGroups();
	ObjectInfos listRouteNames();
};

};
};

#endif // __ZQ_TianShanIce_ERM_ICE__
