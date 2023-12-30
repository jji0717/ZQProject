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
// Ident : $Id: ZqSpotIce.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/ZQSpot/ZqSpotIce.ice $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "ZqSpotIce::InterfaceIndex,ZqSpotIce::InterfaceInfo,name,case-insensitive" InterfaceIndex $(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe --ice -I$(ICE_ROOT)/slice --dict "ZqSpotIce::SpotDict,ZqSpotIce::SpotKey,ZqSpotIce::SpotInfo" --dict-index "ZqSpotIce::SpotDict,key" SpotDict $(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe --ice -I$(ICE_ROOT)/slice --dict "ZqSpotIce::NodeDict,string,ZqSpotIce::NodeInfo" NodeDict --dict-index "ZqSpotIce::NodeDict,nodeguid" $(InputName).ice

#ifndef __ZQ_Spot_ICE__
#define __ZQ_Spot_ICE__

#include <Ice/Communicator.ice>
#include <Ice/ObjectAdapter.ice>
#include <Ice/Logger.ice>

module ZqSpotIce
{

// -----------------------------
// class Communicator
// -----------------------------
/// a redirector of communicator
local class Communicator extends Ice::Communicator
{
	Ice::Communicator ic;
};

// -----------------------------
// class ObjectAdapter
// -----------------------------
/// a redirector of ObjectAdapter
local class ObjectAdapter extends Ice::ObjectAdapter
{
	Ice::ObjectAdapter ia;
};

// -----------------------------
// class Logger
// -----------------------------
/// implementation of Logger
local class Logger extends Ice::Logger
{
};

// key to index the ZQ Spot
struct SpotKey
{
	// the pid of the server application
	int  processId;

    // The server node. 
    string nodeguid;
};

// -----------------------------
// callbacks SpotBind
// -----------------------------
/// callbacks of the Spot about its events, the per-spot interface status sink
/// spot client must call SpotQuery::sinkEvents()
interface SpotBind
{
	/// event fired when an new interface has been newly added during runtime
	///@param spot			used to identify the event trigger spot
	///@param interfacename	the name of interface that has load change
	void OnInterfaceAdded(SpotKey spot, string interfacename);

	/// event fired when an interface is removed from the trigger spot
	///@param spot			used to identify the event trigger spot
	///@param interfacename	the name of interface that has load change
	void OnInterfaceRemoved(SpotKey spot, string interfacename);
};

interface LoadBind
{
	/// event fired when an interface's load is changed and exceeded the user's monitor window
	///@param spot			used to identify the event trigger spot
	///@param interfacename	the name of interface that has load change
	///@param newLoad		the new load value after change
    void OnLoadChanged(SpotKey spot, string interfacename, int newLoad);
};

sequence<string> InterfaceNames;
 
// -----------------------------
// callbacks SpotQuery
// -----------------------------
/// the common interface exported by every spot.
interface SpotQuery
{
	/// list all the available interface exported from the target spot
	///@param[out] names	a list of the available interface names
	///@preturn				the count of the interface names
	int listAllInterface(out InterfaceNames names);

	/// query the load value of an interface on target spot
	///@param interfacename	the name of interface about to query
	///@preturn				the value of the load in a range of 0-10000
	///                     0     - highly available,
	///                     10000 - fully allocated, 10000+ means unavailable
	int getInterfaceLoad(string interfacename);

	/// bind the SpotBind instance to subscribe Spot events
	/// every SpotQuery proxy (client) can only has one SpotBind instance to sink events, a later binding
	/// will overwrite any existing
	///@param bind			the instance of SpotBind about to sink
	void sinkEvents(SpotBind* bind);

	/// subscribe the load change of a specific on Spot events
	/// every SpotQuery proxy (client) can only has one SpotBind instance to sink events, a later binding
	/// will overwrite any existings
	///@param interfacename	the name of interface about to monitor, "*" means all the interface on the spot
	///@preturn				false if failed
	bool subscribeLoadChange(LoadBind* bind, string interfacename, int loadWindow);

	/// unsubscribe the load change of a specific interface on Spot events
	/// every SpotQuery proxy (client) can only has one SpotBind instance to sink events, a later binding
	/// will overwrite any existings
	///@param interfacename	the name of interface, "*" means all the interface on the spot
	///@preturn				false if failed
	bool unsubscribeLoadChange(LoadBind* bind, string interfacename);

	/// get the spot operating system information
	///@preturn				a description of the spot os in the format of ...
	string getOsInfo();

	/// get the process info of the host where the spot is running on
	///@preturn				a description of the host processor
	string getProcessorInfo();
};


/*
// data structure used inside of Spot
// ----------------------------------

// Information about an adapter registered with the &IceGrid; registry.
class InterfaceInfo
{
	// the name of factory or the classname of the object it can hatch
	string name;

//    // the adapter's endpoint, in the format of <FactoryName>@<adapter's endpoint>
//	string endpoint;

	// the key to SpotInfo
	SpotKey spot;

	// a load value ranged within 0-10000
	//    0     - highly available,
	//    10000 - fully allocated, 10000+ means unavailable
	int		load;

	// the timestamp of last load update
	int		lastupdate;
};

// --index "ZqSpotIce::InterfaceIndex,ZqSpotIce::InterfaceInfo,name,case-insensitive" InterfaceIndex $(InputName).ice

// Information about a server application or process
class SpotInfo
{
    // the unique key
	SpotKey key;

    // the name of the executable
	string application;

	// the query interface that also that contains the default adapter endpoint. 
	SpotQuery* query;
};

// --dict "ZqSpotIce::SpotDict,ZqSpotIce::SpotKey,ZqSpotIce::SpotInfo" --dict-index "ZqSpotIce::SpotDict,key" SpotDict $(InputName).ice


// Information about an &IceGrid; node.
struct NodeInfo
{
	// the unique key: guid of the node
	string nodeguid;

	// The operating system name and version
    string os;

	// the network name of the node
    string hostname;

    // The description of processors.
    string processor;
};

// --dict "ZqSpotIce::NodeDict,string,ZqSpotIce::NodeInfo" NodeDict --dict-index "ZqSpotIce::NodeDict,guid" $(InputName).ice
*/
};

#endif // __ZQ_ICE_Locator_ICE__
