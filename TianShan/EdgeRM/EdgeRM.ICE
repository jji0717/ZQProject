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
// Ident : $Id: EdgeRM.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRM.ICE $
// 
// 20    6/06/16 5:20p Li.huang
// ETV-TV-NOW-CR038
// 
// 19    1/15/14 3:48p Bin.ren
// 
// 18    11/27/13 2:29p Ketao.zhang
// add diffAllocation
// 
// 17    11/13/13 1:32p Bin.ren
// 
// 16    11/08/13 2:11p Bin.ren
// add device update interface
// 
// 15    10/16/13 5:55p Bin.ren
// add sync deivces and routeName
// 
// 14    10/15/13 5:22p Bin.ren
// change ImportDeviceCmd from import only one device to import all
// devices in xml file
// 
// 13    10/11/13 5:49p Bin.ren
// add exportDevice interface
// 
// 12    9/24/13 2:42p Li.huang
// sync allocation
// 
// 11    9/11/13 4:13p Bin.ren
// 
// 10    9/11/13 1:40p Bin.ren
// 
// 9     9/11/13 1:26p Li.huang
// marshal alloction
// 
// 8     7/01/13 2:01p Bin.ren
// 
// 7     6/03/13 4:41p Li.huang
// add getEdgePort From EdgeDevice
// 
// 6     3/28/13 1:49p Bin.ren
// 
// 5     3/26/13 2:14p Li.huang
// add ProvisonPort
// 
// 4     3/20/13 11:36a Li.huang
// 
// 3     11/01/12 3:29p Li.huang
// 
// 2     10/31/12 2:47p Li.huang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 12    09-12-29 15:25 Li.huang
// 
// 11    09-12-18 9:33 Li.huang
// restore
// 
// 11    09-12-10 18:23 Li.huang
// associated servicegroup to freq range and modify import_device
// interface(add Properties parameter)
// 
// 10    09-11-26 15:42 Li.huang
// fix some bugs 
// 
// 9     09-11-18 16:04 Li.huang
// fix some bugs
// 
// 8     09-11-11 14:00 Li.huang
// 
// 7     09-10-30 11:58 Li.huang
// 
// 6     09-07-30 17:17 Xiaoming.li
// add getEdgePort for channelEx
// 
// 5     09-03-19 17:12 Hui.shao
// init draft of evaluate, commit and withdraw,
// plus the states of allocation
// 
// 4     09-03-09 17:29 Hui.shao
// 
// 3     09-03-05 19:41 Hui.shao
// defined program structure to impl
// 
// 2     09-03-04 21:31 Hui.shao
// 
// 1     09-02-26 17:53 Hui.shao
// initial created
// ===========================================================================
// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/common --output-dir .. ../$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/common --index "TianShanIce::EdgeResource::ChannelOfDevice,TianShanIce::EdgeResource::EdgeChannelEx,identDevice" ChannelOfDevice --output-dir .. ../$(InputName).ice
// rem $(ICE_ROOT)\bin\slice2freeze.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/common --index "TianShanIce::EdgeResource::ChannelOfAlloc,TianShanIce::EdgeResource::AllocationEx,identChannel" ChannelOfAlloc --output-dir .. ../$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/common --index "TianShanIce::EdgeResource::DeviceOfZONE,TianShanIce::EdgeResource::EdgeDeviceEx,deviceZone" DeviceOfZONE --output-dir .. ../$(InputName).ice


#ifndef __ZQ_TianShanIce_EdgeRM_ICE__
#define __ZQ_TianShanIce_EdgeRM_ICE__

#include <TianShanIce.ICE>
#include <TsEdgeResource.ICE>
#include "TianShanUtils.ICE"

module TianShanIce
{

module EdgeResource
{

struct AllocLink
{
	Ice::Identity identAlloc;
	long  bandwidth; ///< the bandwidth of the allocation reserved
	long  stampLinked;
	long  stampCommitted;
	long  stampUpdated;
};

struct UsageLink
{
	long  bandwidth; ///< the bandwidth of the allocation reserved
	long  stampUpdated;
};

dictionary <int, UsageLink > ChannelUsageNodeMap; ///< the map of out-going PN of the EdgeChannel according to the monitoring of EdgeDevice status.
												  ///< some channel resources may be used but not under the ERM's management, this data structure is
                                                  ///< designed to gather those channels and eliminate them from the allocation thru ERM

dictionary <int, AllocLink > AllocationMap; ///< the map of out-going PN of the EdgeChannel to committed allocation map

struct ProvisionPort
{
  int         enabled;
  string      inbandMarker;
  int         reportTrafficMismatch;
  int         jitterBuffer;
};

["freeze:write"] 
class EdgeChannelEx extends EdgeChannel implements TianShanUtils::TimeoutObj
{
	Ice::Identity  identDevice; // index to the device that the channel belongs to
	AllocationMap  allocMap;
	ProvisionPort  provPort;
	string		   routeName; // index route to channel
	
	ChannelUsageNodeMap busyPN;
	IValues serviceGroupRestriction;
	
	/// entry from the EdgeDeviceHelper to update the attributes of the channel
	["ami"] void updateAttributes(Properties attrs); 
	
	void addAllocLink(int programNumber, AllocLink link);
	AllocLink getAllocLink(Ice::Identity identAlloc, int programNumber);
	
	/// entry to get edgePort for some information
	EdgePort getEdgePort();
	
	/// entry to get ProvisionPort for some information
	ProvisionPort getProvisionPort();

	/// entry of record restore during service restart
	void OnRestore();
	
	/// evaluate to host an allocation on this channel
	///@param[in] resRequirement the per-channel resource requirement carried in the Allocation, QAM channel
	///           may take resReqPerChannel[rtAtscModulationMode] to narrow the usage
	///@param[out] the resource result that this channel can offer per the requirement
	///@return the cost of this channel to provide for the resource requirement, 10000 max, 10001 unavailable
	// ["amd"] 
	int evaluate(SRM::ResourceMap resRequirement, out SRM::ResourceMap resResult)
	throws
	  InvalidStateOfArt,
      NotSupported,
	  ServerError;

	/// commit an allocation on this channel
	///@param[in] identAlloc the identity of the allocation to commit
	///@param[in] resRequirement the per-channel resource requirement carried in the Allocation, QAM channel
	///           may take resReqPerChannel[rtAtscModulationMode] to narrow the usage
	///@param[out] the resource result that this channel can offer per the requirement
	// ["amd"]
	void commit(Ice::Identity identAlloc, SRM::ResourceMap resRequirement, out SRM::ResourceMap resResult)
	throws
	  InvalidStateOfArt,
      NotSupported,
	  ServerError;
	  
	/// withdraw the resource usage of an Allocation when the Allocation is being free-ed or give up the pre-reservation
	///@param[in] identAlloc the identity of the allocation to withdraw
	///@param[in] programNumber the program number of the resource employment
	// ["amd"]
	void withdraw(Ice::Identity identAlloc, int programNumber)
	throws
	  InvalidStateOfArt,
      NotSupported,
	  ServerError;
};

struct EdgePortEx
{
	EdgePort portAttr;
///	IValues  serviceGroups;
//	ServiceGroupsMap serviceGroups; // associated servicegroup to freq range
	RoutesMap	routes;
};

dictionary<short, EdgePortEx> EdgePortMap; ///< map of id to EdgePort

["freeze:write"] 
class EdgeDeviceEx extends EdgeDevice implements TianShanUtils::TimeoutObj
{
	EdgePortMap edgePorts;
	
	void addEdgePort(EdgePort port)
	throws
		InvalidParameter;
		
	void removeEdgePort(short portId)
	throws
		InvalidParameter,
		InvalidStateOfArt;
		
	EdgePort getEdgePort(short portId)
	throws
		InvalidParameter;
	
	void addChannel(short portId, int chId, Properties attributes)
	throws
		InvalidParameter;
		
	["ami"] void updateAttributes(Properties attrs); 
	["amd"] StrValues listAllocationIds();
	
	/// entry of record restore during service restart
	void OnRestore();
};

struct ChannelAssoc
{
	Ice::Identity  identCh;
	EdgeChannelEx* ch;
	
	int   costOfAlloc;
	long  stampEvaluated;

	/// the resource about ChannelCandidate, must have
	///@li unique value of rtPhysicalChannel["edgeDeviceName"], rtPhysicalChannel["edgeDeviceIP"], rtPhysicalChannel["edgeDeviceZone"]
	///@li res[rtAtscModulationMode]
	SRM::ResourceMap resources; ///< the resource about ChannelCandidate, must have rtPhysicalChannel[]
	long  stampEmployed;  ///< the timestamp that the allocation reserve or commit to employ this channel
	short pn; ///< the program number of the channel that has been used in the allocation, initialized with -1;
};

sequence <ChannelAssoc> ChannelAssocation;

["freeze:write"] 
class AllocationEx extends Allocation implements TianShanUtils::TimeoutObj
{
	ChannelAssocation channelAssocs;
	
	/// entry of record restore during service restart
	void OnRestore();

	void setRetrytimes(int retimes);

	int  retrytimes;
	
	string sessionGroup; ///s6client sessionGroup;
	string onDemandSessionId;///s6client onDemandSession ID
	
	string qamSessionId;///eqam server SessionId ,R6 server SessionId
	string qamSessGroup;///eqam session Group, R6 session Group
	
    void setSessionGroup(string sessGroup);
    void setOnDemandSessionId(string onDemandSessId);
    void setqamSessionId(string sessId);
    void setqamSessGroup(string sessGroup);
    
    string getOwnerKey();
    string getSessionGroup();
    string getOnDemandSessionId();
    string getqamSessionId();
    string getqamSessGroup();
    ChannelAssocation getChannelAssocs();
};

struct AlloctionRecord
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
	
	ChannelAssocation channelAssocs;
	int  retrytimes;
	string sessionGroup; ///s6client sessionGroup;
	string onDemandSessionId;///s6client onDemandSession ID
	string qamSessionId;///eqam server SessionId ,R6 server SessionId
	string qamSessGroup;///eqam session Group, R6 session Group
	AllocLink link;
};

struct DeviceRec
{
	string deviceName;
	string deviceZone;
};
sequence <DeviceRec> DevicesRecs;
sequence <EdgeDevice*> EdgeDevices;

sequence <BValues> AlloctionValues;
// -----------------------------
// Service EdgeRM
// -----------------------------
/// represents a manager of the sessions within SRM
interface EdgeRM extends EdgeResouceManager, ReplicaSubscriber, TianShanUtils::TimeoutObj
{
	["amd"] EdgeDevices importDevice(string name, string deviceZone, string xmlDefFile, bool bCompress, Properties props);
	["amd"] EdgeDevices importDeviceXML(string name, string deviceZone, Properties props, string xmlBody);
	
	// a short to open a EdgeChannel directly with the full channel name
	EdgeChannelEx* openChannel(string chName);
	
	["amd", "ami", "cpp:const"] AlloctionValues exportAllocations(string deviceName, string since);
	["amd", "ami", "cpp:const"] int exportDevices(string deviceName, string xmlFileName);
	["amd", "ami", "cpp:const"] string exportDeviceXML(string deviceName);
	void forceBackupMode(string netId, int mode);
	
	//@param allocIds[in] the Id list of the allocations known in the caller
	//@param allocIdsOfGone[out] the callee determined those allocations that have gone, and output the list
	//@return the new allocations recently created in the callee but not listed in allocIds
	["amd", "ami", "cpp:const"] AlloctionValues diffAllocations(string deviceName, StrValues allocIds, out StrValues allocIdsOfGone);

};

};
};

#endif // __ZQ_TianShanIce_EdgeRM_ICE__

