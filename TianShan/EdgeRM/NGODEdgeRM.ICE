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
// Ident : $Id: NgodERM.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/NGODEdgeRM.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-02-26 17:53 Hui.shao
// initial created
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_NgodERM_ICE__
#define __ZQ_TianShanIce_NgodERM_ICE__

#include <TsEdgeResource.ICE>
#include <Ice/Identity.ice>

module TianShanIce
{

// -----------------------------
// Service NgodERM
// -----------------------------
/// represents a manager of the sessions within SRM
interface NgodERM extends EdgeResouceManager
{
	/// create a Allocation with initail resource requriement
	///@param[in] assetUri	the asset URI required in the session
	///@return    a session instance, NULL if failed
	///@note	  resouce requirement must be specified by calling Session::addResource()
	Allocation* allocate(ResourceMap resourceReq, ::TianShan::StrValues preferChannels, long TTL, AllocationOwner* owner, string ownerContextKey)
	throws ServerError;

	/// open an existing allocation with a given id
	///@param[in] Id	the allocation id
	///@return    access to the allocation instance, NULL if failed
	Allocation* openAllocation(string Id)
	throws ServerError;
	
	listDevices();
	listChannels();
	listAllocations();
};

};
};

#endif // __ZQ_TianShanIce_ERM_ICE__
