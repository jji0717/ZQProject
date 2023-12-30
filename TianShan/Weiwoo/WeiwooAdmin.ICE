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
// $Log: /ZQProjs/TianShan/Weiwoo/WeiwooAdmin.ICE $
// 
// 2     3/09/11 4:42p Hongquan.zhang
// 
// 1     10-11-12 16:08 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 20    10-06-18 16:19 Hui.shao
// removed index IdToSess
// 
// 19    09-06-04 17:10 Hui.shao
// support of the override timeout of session
// 
// 18    08-08-14 15:04 Hui.shao
// merged from 1.7.10
// 
// 18    08-07-08 14:56 Hui.shao
// fixed per ICE 3.3 definitions
// 
// 17    07-04-12 14:02 Hongquan.zhang
// 
// 16    07-03-28 16:42 Hui.shao
// moved business router to namespace Site
// 
// 15    07-03-15 15:55 Hui.shao
// 
// 14    07-03-13 14:38 Hongquan.zhang
// 
// 13    9/21/06 4:36p Hui.shao
// batch checkin 20060921
// 
// 1     06-09-18 19:06 Hui.shao
// ===========================================================================
// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. ../$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::SRM::IdToSess,TianShanIce::SRM::SessionEx,sessId,case-sensitive" IdToSess --output-dir .. ../$(InputName).ice
// outputs:
//	$(InputName).h
//	$(InputName).cpp
//  IdToSess.cpp
//  IdToSess.h

#ifndef __ZQ_TianShanIce_WeiwooAdmin_ICE__
#define __ZQ_TianShanIce_WeiwooAdmin_ICE__

#include <../Ice/TsSRM.ICE>

#include <Ice/Identity.ice>

module TianShanIce
{

module SRM
{

// -----------------------------
// class SessionEx
// -----------------------------
/// extension of Session for some internal accesses
["freeze:write"] 
class SessionEx extends Session
{
	Ice::Identity ident;
	long overrideExp; ///< the expiration to override any runtime expiration of the session if this has been set
	
	/// attach a purchase from the business logic into the session
	void attachPurchase(Application::Purchase* purchase);
	
	void onRestore();

	void OnTimer();
};

// -----------------------------
// Service SessionAdmin
// -----------------------------
/// extension of SessionManager to admin the sessions
interface SessionAdmin extends SessionManager
{
};


};
};

#endif // __ZQ_TianShanIce_WeiwooAdmin_ICE__
