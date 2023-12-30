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
// Ident : $Id: ProvisionState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionState.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     08-05-17 19:08 Jie.zhang
// 
// 2     08-03-27 16:53 Jie.zhang
// 
// 2     08-03-07 18:14 Jie.zhang
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_ProvisionState_H__
#define __ZQTianShan_ProvisionState_H__

#include "../common/TianShanDefines.h"

#include "CPE.h"
#include "CPEEnv.h"

namespace ZQTianShan {
namespace CPE {

class ProvisionSessImpl;

// -----------------------------
// class ProvisionStateBase
// -----------------------------
///@note any necessary lock must performed outside of state process
class ProvisionStateBase
{
protected:
	/// constructor
	///@note no direct instantiation of ProvisionStateBase is allowed
    ProvisionStateBase(CPEEnv& env, ProvisionSessImpl& sess, const ::TianShanIce::ContentProvision::ProvisionState state);
    virtual ~ProvisionStateBase() {}

public:
	
	/// convert the state code to the display string
	static const char* stateStr(const ::TianShanIce::ContentProvision::ProvisionState state);

	// impls of state activity
	
	/// The entry will be called when a session's state is about to change to this state
	///@note state change validation should be done here
	///@note at least, notification of state change should be fired here. keep this pure virutal
	///      so that no instantiation of ProvisionStateBase will happen
	virtual void enter(const ::Ice::Current& c) {}

	/// this entry usually called by the successor ProvisionStateBase to allow to clean up some session context
	virtual void leave(void) {}

public:	// impls of ProvisionSession and ProvisionSessionEx

	/// An internal thread of CPE service will keep polling each session, if it detected a session's
	/// PollingTimer is expired, the thread will call this entry based its state
	///@note be sure to renew the timer for next event after the scheduled work is completed
	virtual void OnTimer(const ::Ice::Current& c) {}
	
	/// After CPE is restarted, the ProvisionSession objects will be restored from the database, a restore will be called
	/// for each session, the state must how to respond this restoring
	virtual void OnRestore(const ::Ice::Current& c);

	virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
	virtual int doAddResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c);
	
protected:

	int _addResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c);
	void _removeResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
	void _commitState(bool fireEvent=true);
	void _updateExpiration(const ::Ice::Long newExpiration=0);
	void _destroySess();

protected:
	
	CPEEnv& _env;
	ProvisionSessImpl&  _sess;
	::TianShanIce::ContentProvision::ProvisionState _oldState, _theState;
	std::string		_lastError;
};

// -----------------------------
// class ProvStateCreated
// -----------------------------
class ProvStateCreated : public ProvisionStateBase
{
public:

    ProvStateCreated(CPEEnv& env, ProvisionSessImpl& sess) : ProvisionStateBase(env, sess, ::TianShanIce::ContentProvision::cpsCreated) {}

public:	// impls of state activity
	
	virtual void enter();
	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c);
	// do nothing on leaving: virtual void leave(void) {}

public:	// impls of Session and SessionEx

    // for this state:
    // not allowed in this state: virtual void doDestroy(const ::Ice::Current& c);
	virtual int doAddResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c);
	virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
};

// -----------------------------
// class ProvStateAccepted
// -----------------------------
class ProvStateAccepted : public ProvisionStateBase
{
public:

    ProvStateAccepted(CPEEnv& env, ProvisionSessImpl& sess) : ProvisionStateBase(env, sess, ::TianShanIce::ContentProvision::cpsAccepted) {}

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c);
//	virtual void leave(void);

public:	// impls of Session and SessionEx

    // for this state:
	// not allowed in this state: virtual int doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c);
	// not allowed in this state: virtual void doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
};

// -----------------------------
// class ProvStateWait
// -----------------------------
class ProvStateWait : public ProvisionStateBase
{
public:

    ProvStateWait(CPEEnv& env, ProvisionSessImpl& sess) : ProvisionStateBase(env, sess, ::TianShanIce::ContentProvision::cpsWait) {}

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c);
//	virtual void leave(void);

public:	// impls of Session and SessionEx

};

// -----------------------------
// class ProvStateReady
// -----------------------------
class ProvStateReady : public ProvisionStateBase
{
public:

    ProvStateReady(CPEEnv& env, ProvisionSessImpl& sess) : ProvisionStateBase(env, sess, ::TianShanIce::ContentProvision::cpsReady) {}

public:	// impls of state activity
	
	virtual void enter();
	virtual void leave();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c);
};

// -----------------------------
// class ProvStateProvisioning
// -----------------------------
class ProvStateProvisioning : public ProvisionStateBase
{
public:
	
    ProvStateProvisioning(CPEEnv& env, ProvisionSessImpl& sess) : ProvisionStateBase(env, sess, ::TianShanIce::ContentProvision::cpsProvisioning) {}
	
public:	// impls of state activity
	
	virtual void enter();
	virtual void leave(void);
	
	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c);
};

// -----------------------------
// class ProvStateStopped
// -----------------------------
class ProvStateStopped : public ProvisionStateBase
{
public:
	
    ProvStateStopped(CPEEnv& env, ProvisionSessImpl& sess) : ProvisionStateBase(env, sess, ::TianShanIce::ContentProvision::cpsStopped) {}
	
public:	// impls of state activity
	
	virtual void enter();
	
	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c);
};

}} // namespace

#endif // __ZQTianShan_ProvisionState_H__
