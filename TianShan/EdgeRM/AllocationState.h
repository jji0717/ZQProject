// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Poscontention, use,
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
// Ident : $Id: AllocState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/AllocationState.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 8     09-11-18 16:04 Li.huang
// fix some bugs
// 
// 7     09-10-30 11:58 Li.huang
// 
// 6     09-09-28 16:10 Li.huang
// 
// 5     09-07-27 17:00 Xiaoming.li
// add lock for allocation
// 
// 4     09-03-26 15:17 Hui.shao
// impl of state inservice and outofservice
// 
// 3     09-03-19 17:33 Hui.shao
// removed state::addResource()
// 
// 2     09-03-19 17:12 Hui.shao
// init draft of evaluate, commit and withdraw,
// plus the states of allocation
// 
// 1     09-03-05 19:41 Hui.shao
// initially created
// ===========================================================================

#ifndef __ZQTianShan_EdgeRM_AllocState_H__
#define __ZQTianShan_EdgeRM_AllocState_H__

#include "../common/TianShanDefines.h"

#include "EdgeRMImpl.h"
#include "EdgeRMEnv.h"
namespace ZQTianShan {
namespace EdgeRM {

class AllocationImpl;

// -----------------------------
// class AllocStateBase
// -----------------------------
///@note any necessary lock must performed outside of state process
class AllocStateBase
{
public:
	/// constructor
	///@note no direct instantiation of AllocStateBase is allowed
    AllocStateBase(EdgeRMEnv& env, AllocationImpl& alloc, const ::TianShanIce::State state);
    virtual ~AllocStateBase() {}

public:
	
	/// convert the state code to the display string
	static const char* stateStr(const ::TianShanIce::State state);
	static ::TianShanIce::State stateId(const char* stateStr);

	// impls of state activity
	
	/// The entry will be called when a contention's state is about to change to this state
	///@note state change validation should be done here
	///@note at least, notification of state change should be fired here. keep this pure virutal
	///      so that no instantiation of AllocStateBase will happen
	virtual void enter(const ::Ice::Current& c) {}

	/// this entry usually called by the successor AllocStateBase to allow to clean up some contention context
	virtual void leave(void);

public:	// impls of Provisioncontention and ProvisioncontentionEx

	/// An internal thread of ContentStore service will keep polling each contention, if it detected a contention's
	/// PollingTimer is expired, the thread will call this entry based its state
	///@note be sure to renew the timer for next event after the scheduled work is completed
	virtual void OnTimer(const ::Ice::Current& c);
	
	/// After ContentStore is restarted, the Provisioncontention objects will be restored from the database, a restore will be called
	/// for each contention, the state must how to respond this restoring
	virtual void OnRestore(const ::Ice::Current& c) { OnTimer(c); }

protected:

	void _renew(const ::Ice::Long newExpiration=0);
	void _commitState(bool fireEvent=true, const ::std::string& msg="");
	void _destroy();

protected:
	
	EdgeRMEnv& _env;
	AllocationImpl&  _alloc;
	::TianShanIce::State _oldState, _theState;
	std::string		_lastError;
	Ice::Long		_stampCreated;
};

// -----------------------------
// class AllocStateNotProvisioned
// -----------------------------
class AllocStateNotProvisioned : public AllocStateBase
{
public:

    AllocStateNotProvisioned(EdgeRMEnv& env, AllocationImpl& alloc)
	: AllocStateBase(env, alloc, ::TianShanIce::stNotProvisioned) {}

public:	// impls of state activity
	
	virtual void enter();

public:
};

// -----------------------------
// class AllocStateProvisioned
// -----------------------------
class AllocStateProvisioned : public AllocStateBase
{
public:

    AllocStateProvisioned(EdgeRMEnv& env, AllocationImpl& alloc)
	: AllocStateBase(env, alloc, ::TianShanIce::stProvisioned) {}

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnRestore(const ::Ice::Current& c){_env._watchDog.watch(_alloc.ident, 0);}
	//virtual void OnRestore(const ::Ice::Current& c) { OnTimer(c); }
};

// -----------------------------
// class AllocStateInService
// -----------------------------
class AllocStateInService : public AllocStateBase
{
public:

	AllocStateInService(EdgeRMEnv& env, AllocationImpl& alloc) :
	  AllocStateBase(env, alloc, ::TianShanIce::stInService){}

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c){_env._watchDog.watch(_alloc.ident, 0);}
	//virtual void OnRestore(const ::Ice::Current& c) { OnTimer(c); }
};

// -----------------------------
// class AllocStateOutOfService
// -----------------------------
class AllocStateOutOfService : public AllocStateBase
{
public:
	
    AllocStateOutOfService(EdgeRMEnv& env, AllocationImpl& alloc) 
	: AllocStateBase(env, alloc, ::TianShanIce::stOutOfService) {}
	
public:	// impls of state activity
	
	virtual void enter();
	
	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const ::Ice::Current& c){_env._watchDog.watch(_alloc.ident, 0);}
	//virtual void OnRestore(const ::Ice::Current& c) { OnTimer(c); }
};

}} // namespace

#endif // __ZQTianShan_EdgeRM_AllocState_H__
