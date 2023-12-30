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
// Ident : $Id: TianShanIce.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPE.ICE $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 5     10-04-07 15:24 Jie.zhang
// add parameter on updateProgress
// 
// 4     08-11-18 10:59 Jie.zhang
// merge from TianShan1.8
// 
// 3     08-04-09 11:48 Hui.shao
// added ProvisionCost
// 
// 2     08-03-27 16:15 Jie.zhang
// 
// 2     08-03-17 19:56 Jie.zhang
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================
// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/tianshan/AccreditedPath --output-dir .. $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice  --index "TianShanIce::ContentProvision::ContentToProvision,TianShanIce::ContentProvision::ProvisionSessionEx,contentKey" ContentToProvision --output-dir .. $(InputDir)\$(InputName).ice
// outputs:
//	../$(InputName).h
//	../$(InputName).cpp
//  ../ContentToProvision.cpp
//  ../ContentToProvision.h

#ifndef __ZQ_TianShanIce_CPE_ICE__
#define __ZQ_TianShanIce_CPE_ICE__

#include "TsContentProv.ICE"

#include <Ice/Identity.ice>

module TianShanIce
{

module ContentProvision
{

// -----------------------------
// class ProvisionSessionEx
// -----------------------------
/// extension of ProvisionSession for some internal accesses
["freeze:write"] 
class ProvisionSessionEx extends ProvisionSession
{
	Ice::Identity ident;

	["cpp:const", "freeze:read"] string getProvisionId();
	["cpp:const", "freeze:read"] long getBandwidth();

	/// CPH must set the session type values if passed validateSetup()
	void setSessionType(ProvisionType ptype, StartType stype)
		throws ServerError, ClientError, InvalidStateOfArt;

	void updateProgress(long processed, long total, TianShanIce::Properties params)
		throws ServerError, ClientError, InvalidStateOfArt;

	void notifyStarted(TianShanIce::Properties params)
		throws ServerError, ClientError, InvalidStateOfArt;

	void notifyStopped(bool errorOccurred, TianShanIce::Properties params)
		throws ServerError, ClientError, InvalidStateOfArt;

	void notifyStreamable(bool streamable)
		throws ServerError, ClientError, InvalidStateOfArt;

	void OnTimer();
	void OnRestore();

	/// the entry for the local push trigger
	void forceToStart()
		throws ServerError, ClientError, InvalidStateOfArt;

	int preload; //< estimated time to prepare a session before scheduled start. will be updated by CPH 
	int linger;
	long expiration;
};


// -----------------------------
// service impl ContentProvisionEngine
// -----------------------------
/// implemetation of the service interface of ContentProvisionEngine (CPE)
interface ContentProvisionEngine extends ContentProvisionService
{
};

};
};

#endif // __ZQ_TianShanIce_CPE_ICE__
