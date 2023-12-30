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
// Ident : $Id: SessionState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionState.h $
// 
// 8     1/16/17 4:58p Hui.shao
// pass the lasterr as the terminate reason
// 
// 7     10/13/16 3:52p Hui.shao
// 
// 7     10/13/16 3:48p Hui.shao
// 
// 6     11/10/15 6:43p Hui.shao
// 
// 5     11/10/15 4:22p Hui.shao
// 
// 5     11/10/15 1:49p Hui.shao
// ticket#18220 to prevent double destroy()
// 
// 4     10/28/15 3:27p Hui.shao
// 
// 3     3/09/11 4:42p Hongquan.zhang
// 
// 2     3/07/11 4:59p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 9     08-03-18 14:55 Hongquan.zhang
// lock servant if using it
// 
// 8     07-11-19 11:53 Hongquan.zhang
// 
// 7     07-03-13 17:12 Hongquan.zhang
// ===========================================================================

#ifndef __ZQTianShan_SessionState_H__
#define __ZQTianShan_SessionState_H__

#include "../common/TianShanDefines.h"

#include "WeiwooAdmin.h"
#include "WeiwooSvcEnv.h"

namespace ZQTianShan {
namespace Weiwoo {

class SessionImpl;

// -----------------------------
// class SessStateBase
// -----------------------------
///@note any necessary lock must performed outside of state process
class SessStateBase
{
protected:
	/// constructor
	///@note no direct instantiation of SessStateBase is allowed
    SessStateBase(WeiwooSvcEnv& env, SessionImpl& sess, const ::TianShanIce::State state);
    virtual ~SessStateBase() {}

public:	// impls of state activity
	
	/// The entry will be called when a session's state is about to change to this state
	///@note state change validation should be done here
	///@note at least, notification of state change should be fired here. keep this pure virutal
	///      so that no instantiation of SessStateBase will happen
	virtual void enter(const ::Ice::Current& c) {}

	/// An internal thread of Weiwoo service will keep polling each session, if it detected a session's
	/// PollingTimer is expired, the thread will call this entry based its state
	///@note be sure to renew the timer for next event after the scheduled work is completed
	virtual void OnTimer(const ::Ice::Current& c);

	/// this entry usually called by the successor SessStateBase to allow to clean up some session context
	virtual void leave(void) {}

public:	// impls of Session and SessionEx

    virtual void doDestroy(const ::Ice::Current& c);
	virtual ::Ice::Long doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c);
    virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
    virtual void doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c);
    virtual void doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c);
	
protected:

	::Ice::Long _addResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c);
	void _removeResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
	void _attachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c);
	void _commitState(bool fireEvent=true);
	void _updateExpiration(const ::Ice::Long newExpiration=0);
//	void _updateTimer(const ::Ice::Long newTimeout);
	void _destroySess(const ::Ice::Identity& ident);

protected:
	
	void		setLastErrorMsg( const std::string& errMsg, bool asTerminateReason = false);

	class ErrLog : public ZQ::common::Log
	{
	public:
		ErrLog(WeiwooSvcEnv& env,SessionImpl&  sess):_env(env),_sess(sess)
		{
		}
		//ZQ::common::Log& operator()(int level, const char *fmt, ...);		
		virtual void writeMessage(const char *msg, int level=-1);
	private:		
		WeiwooSvcEnv& _env;
		SessionImpl&  _sess;
	};

	WeiwooSvcEnv& _env;
	SessionImpl&  _sess;
	::TianShanIce::State _theState, _oldState;
	ErrLog			_errLog;
	std::string		_lastError;
};

// -----------------------------
// class SessStateBase
// -----------------------------
class SessionImpl;//forward declaration
class SessStateChangeGuard
{
public:
	SessStateChangeGuard(SessionImpl& sess, const ::TianShanIce::State targetState, bool idempotent =true);	
	virtual ~SessStateChangeGuard();

	
private:
	SessionImpl&			_sess;
};

// -----------------------------
// class SessStateNotProvisioned
// -----------------------------
class SessStateNotProvisioned : public SessStateBase
{
public:

    SessStateNotProvisioned(WeiwooSvcEnv& env, SessionImpl& sess) : SessStateBase(env, sess, ::TianShanIce::stNotProvisioned) {}
    virtual ~SessStateNotProvisioned() {} 

public:	// impls of state activity
	
	virtual void enter();
	virtual void OnTimer(const ::Ice::Current& c);
	// do nothing on leaving: virtual void leave(void) {}

public:	// impls of Session and SessionEx

    // for this state:
    // not allowed in this state: virtual void doDestroy(const ::Ice::Current& c);
	virtual ::Ice::Long doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c);
    virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
    virtual void doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c);
    virtual void doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c);
};

// -----------------------------
// class SessStateProvisioned
// -----------------------------
class SessStateProvisioned : public SessStateBase
{
public:

    SessStateProvisioned(WeiwooSvcEnv& env, SessionImpl& sess) : SessStateBase(env, sess, ::TianShanIce::stProvisioned) {}
    virtual ~SessStateProvisioned() {} 

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void leave(void);

public:	// impls of Session and SessionEx

    // for this state:
    // not allowed in this state: virtual void doDestroy(const ::Ice::Current& c);
	virtual ::Ice::Long doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c);
    virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
    virtual void doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c);
    virtual void doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c);
};

// -----------------------------
// class SessStateInService
// -----------------------------
class SessStateInService : public SessStateBase
{
public:

    SessStateInService(WeiwooSvcEnv& env, SessionImpl& sess) : SessStateBase(env, sess, ::TianShanIce::stInService) {}
    virtual ~SessStateInService() {}

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void leave(void);

public:	// impls of Session and SessionEx

	void	dummyInService( );

    // not allowed in this state: virtual void doDestroy(const ::Ice::Current& c);
	// not allowed in this state: virtual ::Ice::Long doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c);
    // not allowed in this state: virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
    // not allowed in this state: virtual void doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c);
    // not allowed in this state: virtual void doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c);
};

// -----------------------------
// class SessStateOutOfService
// -----------------------------
class SessStateOutOfService : public SessStateBase
{
public:

    SessStateOutOfService(WeiwooSvcEnv& env, SessionImpl& sess) : SessStateBase(env, sess, ::TianShanIce::stOutOfService) {}
    virtual ~SessStateOutOfService() {}

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	// not allowed in this state: virtual void leave(void);

protected:
	
//	bool	canEnterState( const ::TianShanIce::State& targetState , SessStateChangeGuard& guard );

	void	failedToDestroyInstance();

	std::string _strTeardownReason, _strTerminateReason;
	void _fixupReasons(void);
	
	virtual bool _cleanAttachmentIfAwait(bool bEnforce = false);

    // for this state:
    virtual void doDestroy(const ::Ice::Current& c);
	// not allowed in this state: virtual ::Ice::Long doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c);
    // not allowed in this state: virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
    // not allowed in this state: virtual void doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c);
    // not allowed in this state: virtual void doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c);
};

}} // namespace

#endif // __ZQTianShan_SessionState_H__
