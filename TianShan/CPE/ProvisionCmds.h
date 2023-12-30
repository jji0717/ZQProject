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
// Ident : $Id: ProvisionCmd.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionCmds.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 6     08-06-24 14:56 Jie.zhang
// changestatecmd removed, "setup","ontimer","onrestore" process in ice
// server dispatch thread.
// 
// 5     08-04-09 15:37 Hui.shao
// impl listMethods
// 
// 4     08-04-02 15:47 Hui.shao
// per CPC ICE changes
// 
// 3     08-02-21 15:08 Hui.shao
// added paged list
// 
// 2     08-02-18 20:57 Hui.shao
// impl list provision
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_ProvisionCmds_H__
#define __ZQTianShan_ProvisionCmds_H__

#include "../common/TianShanDefines.h"

#include "CPEImpl.h"
#include "CPEEnv.h"

namespace ZQTianShan {
namespace CPE {


// -----------------------------
// class TimerCmd
// -----------------------------
///
class TimerCmd : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    TimerCmd(CPEEnv& env, const ::Ice::Identity& provisionIdent);

protected: // impls of ScheduleTask

	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
	
protected:

	CPEEnv&		_env;
	::Ice::Identity		_identProv;
};

// -----------------------------
// class RestoreCmd
// -----------------------------
///
class RestoreCmd : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    RestoreCmd(CPEEnv& env, const ::Ice::Identity& provisionIdent);
	
protected: // impls of ScheduleTask
	
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
	
protected:
	
	CPEEnv&		_env;
	::Ice::Identity		_identProv;
};

// -----------------------------
// class ListProvisionCmd
// -----------------------------
///
class ListProvisionCmd : protected ZQ::common::ThreadRequest
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
    ListProvisionCmd(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listSessionsPtr& amdCB, CPEEnv& env, const ::std::string& methodType, const ::TianShanIce::StrValues& paramNames, const ::std::string& startId, ::Ice::Int maxCount);
	virtual ~ListProvisionCmd() {}
	
public:
	
	void execute(void) { start(); }
	
protected: // impls of ThreadRequest
	
	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }
	
protected:
	
	CPEEnv&  _env;
	::TianShanIce::ContentProvision::AMD_ContentProvisionService_listSessionsPtr _amdCB;
	::TianShanIce::StrValues _paramNames;
	std::string _methodType;
	std::string _startId;
	size_t _maxCount;
};

// -----------------------------
// class ListMethodCmd
// -----------------------------
///
class ListMethodCmd : protected ZQ::common::ThreadRequest
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
    ListMethodCmd(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listMethodsPtr& amdCB, CPEEnv& env);
	virtual ~ListMethodCmd() {}
	
public:
	
	void execute(void) { start(); }
	
protected: // impls of ThreadRequest
	
	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }
	
protected:
	
	CPEEnv&  _env;
	::TianShanIce::ContentProvision::AMD_ContentProvisionService_listMethodsPtr _amdCB;
};

}} // namespace

#endif // __ZQTianShan_ProvisionCmds_H__

