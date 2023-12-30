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
// $Log: /ZQProjs/Generic/ZQSpot/ZqSpotApp.ice $
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
// exception NotImplemented
// -----------------------------
exception NotImplemented
{
	string message;
};

enum State
{
	/// the object is created but hasn't been provisioned
	st_NotProvisioned;

	/// The service or object has been provisioned, but is not presently intended to perform its function.
	st_OutOfService,

    /// The service or object has been provisioned, and is intended to provide its function.
    st_InService      
};

// -----------------------------
// exception InvalidStateOfArt
// -----------------------------
exception InvalidStateOfArt
{
	string message;
};

// -----------------------------
// interface Service
// -----------------------------
/// a redirector of communicator
interface Service
{
	string getAdminUri() throws NotImplemented;

	State getState();	
};

};

#endif // __ZQ_ICE_Locator_ICE__
