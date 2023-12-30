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
// Ident : $Id: SessionCommand.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionCommand.h $
// 
// 2     3/07/11 4:57p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 8     08-08-14 15:06 Hui.shao
// merged from 1.7.10
// 
// 8     08-07-08 13:40 Hui.shao
// fixed for ICE 3.3 new syntax
// 
// 7     07-11-26 15:38 Hongquan.zhang
// modify because Purchase::detach has been changed
// 
// 6     07-10-17 12:29 Hongquan.zhang
// 
// 5     07-03-19 15:19 Hongquan.zhang
// 
// 4     07-03-13 17:12 Hongquan.zhang
// 
// 1     06-08-24 10:42 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_SessionCommand_H__
#define __ZQTianShan_SessionCommand_H__

#include "../common/TianShanDefines.h"

#include "WeiwooAdmin.h"
#include "WeiwooSvcEnv.h"
#include "SessionImpl.h"
#include <TsApplication.h>

namespace ZQTianShan {
namespace Weiwoo {

// -----------------------------
// class SessionCommand
// -----------------------------
///
class SessionCommand : protected ZQ::common::ThreadRequest
{
protected:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
    SessionCommand(WeiwooSvcEnv& env, SessionImpl& sess);
	virtual ~SessionCommand();

public:

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void) { return 0; }
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	WeiwooSvcEnv&     _env;
	SessionImpl::Ptr  _pSess;
#if ICE_INT_VERSION / 100 < 303
	bool			  _bKeep;
#endif
};

// -----------------------------
// class SessionProvisionCommand
// -----------------------------
///
class SessionProvisionCommand : public SessionCommand
{
public:

	/// constructor
    SessionProvisionCommand(const ::TianShanIce::SRM::AMD_Session_provisionPtr& amdCB, WeiwooSvcEnv& env, SessionImpl& sess);

protected: // overwrite of SessionCommand

	virtual int run(void);

protected:

	::TianShanIce::SRM::AMD_Session_provisionPtr _amdCB;
};

// -----------------------------
// class SessionServeCommand
// -----------------------------
///
class SessionServeCommand : public SessionCommand
{
public:

	/// constructor
    SessionServeCommand(const ::TianShanIce::SRM::AMD_Session_servePtr& amdCB, WeiwooSvcEnv& env, SessionImpl& sess);

protected: // overwrite of SessionCommand

	virtual int run(void);

protected:

	::TianShanIce::SRM::AMD_Session_servePtr _amdCB;
};

// -----------------------------
// class SessionTimerCommand
// -----------------------------
///
class SessionTimerCommand : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    SessionTimerCommand(WeiwooSvcEnv& env, const ::Ice::Identity& sessIdent);

protected: // impls of ScheduleTask

	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
	
protected:

	WeiwooSvcEnv&		_env;
	::Ice::Identity		_identSess;
};



}} // namespace

#endif // __ZQTianShan_SessionCommand_H__

