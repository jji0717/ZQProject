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
// Ident : $Id: SessionCommand.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionCommand.cpp $
// 
// 4     10/28/15 3:25p Hui.shao
// log fmt
// 
// 3     3/09/11 4:42p Hongquan.zhang
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
// 12    08-08-14 15:07 Hui.shao
// merged from 1.7.10
// 
// 12    08-07-08 14:57 Hui.shao
// fix per ICE 3.3 definiton
// 
// 11    07-11-26 15:38 Hongquan.zhang
// modify because Purchase::detach has been changed
// 
// 10    07-10-17 12:29 Hongquan.zhang
// 
// 9     07-06-21 16:30 Hongquan.zhang
// 
// 8     07-05-09 17:45 Hongquan.zhang
// 
// 7     07-03-19 15:19 Hongquan.zhang
// 
// 6     07-03-13 17:12 Hongquan.zhang
// 
// 5     06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 4     9/21/06 4:36p Hui.shao
// batch checkin 20060921
// 
// 3     06-09-12 20:19 Hui.shao
// added SessionWatchDog
// ===========================================================================

#include "SessionCommand.h"
#include "SessionState.h"

namespace ZQTianShan {
namespace Weiwoo {

// -----------------------------
// class SessionCommand
// -----------------------------
SessionCommand::SessionCommand(WeiwooSvcEnv& env, SessionImpl& sess)
: ThreadRequest(env._thpool), _env(env), _pSess(&sess)
{
#if ICE_INT_VERSION / 100 < 303
	try
	{
		env._eSession->keep(sess.ident);
		_bKeep = true;
	}
	catch (const Ice::Exception&) 
	{
		_bKeep = false;
	}
#endif
}
SessionCommand::~SessionCommand()
{
#if ICE_INT_VERSION / 100 < 303
	try
	{
		if(_bKeep)
		{
			_env._eSession->release(_pSess->ident);
			_bKeep = false;
		}		
	}
	catch (const Ice::Exception&) 
	{		
	}
#endif
}
// -----------------------------
// class SessionProvisionCommand
// -----------------------------
SessionProvisionCommand::SessionProvisionCommand(const ::TianShanIce::SRM::AMD_Session_provisionPtr& amdCB, WeiwooSvcEnv& env, SessionImpl& sess)
: SessionCommand(env, sess), _amdCB(amdCB)
{
}

int SessionProvisionCommand::run(void)
{
	try 
	{
		std::string	strSessID = _pSess->ident.name;
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionProvisionCommand, "sess[%s] enter Provision"), strSessID.c_str());
		SessStateProvisioned(_env, *_pSess).enter();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(SessionProvisionCommand, "sess[%s] Leave Provision"), strSessID.c_str());
		_amdCB->ice_response();
		return 0;
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		_amdCB->ice_exception(ex);
	}
	catch (const ::Ice::Exception& e)
	{
		_amdCB->ice_exception(e);
	}
    catch(const ::std::exception& ex)
    {
		_amdCB->ice_exception(ex);
    }
    catch(...)
    {
		_amdCB->ice_exception();
    }

	return 1;
}

// -----------------------------
// class SessionServeCommand
// -----------------------------
SessionServeCommand::SessionServeCommand(const ::TianShanIce::SRM::AMD_Session_servePtr& amdCB, WeiwooSvcEnv& env, SessionImpl& sess)
: SessionCommand(env, sess), _amdCB(amdCB)
{
}

int SessionServeCommand::run(void)
{
	try {
		SessStateInService(_env, *_pSess).enter();
		_amdCB->ice_response();
		return 0;
	}
	catch (TianShanIce::BaseException& e) 
	{
		_amdCB->ice_exception(e);
	}

	catch (const ::Ice::Exception& e)
	{
		_amdCB->ice_exception(e);
	}
    catch(const ::std::exception& ex)
    {
		_amdCB->ice_exception(ex);
    }
    catch(...)
    {
		_amdCB->ice_exception();
    }

	return 1;
}

// -----------------------------
// class SessionTimerCommand
// -----------------------------
SessionTimerCommand::SessionTimerCommand(WeiwooSvcEnv& env, const ::Ice::Identity& sessIdent)
:ThreadRequest(env._thpool), _env(env), _identSess(sessIdent)
{
}

int SessionTimerCommand::run(void)
{
	::TianShanIce::SRM::SessionExPrx sess;

	try
	{
		sess = IdentityToObj(SessionEx, _identSess);
		sess->OnTimer();
		return 0;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionTimerCommand, "sess[%s] exception occurs: %s:%s"), _identSess.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionTimerCommand, "sess[%s] exception occurs: %s"), _identSess.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionTimerCommand, "sess[%s] unknown exception occurs"), _identSess.name.c_str());
	}

	// when reaches here, an exception might occur, when re-post a timer command to ensure no action is dropped
	try
	{
		_env._watchDog.watchSession(_identSess, _env._ttlIdleSess);
	}
	catch(...)
	{
	}

	return -1;
}

void SessionTimerCommand::final(int retcode, bool bCancelled)
{
	if (bCancelled)
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionTimerCommand, "sess[%s] user canceled timer activity"), _identSess.name.c_str());

	delete this;
}





}} // namespace