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
// Ident : $Id: ContentState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentState.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 5     09-12-22 14:06 Jie.zhang
// merge from TianShan1.10
// 
// 4     08-11-24 12:44 Hui.shao
// Provisioning::OnTimer() test scheduleEnd and provision session
// 
// 3     08-11-15 14:02 Hui.shao
// switch state by checking residential status
// 
// 2     08-10-28 17:57 Hui.shao
// test the last write if it is worthy to populate attributes
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// 
// 4     08-07-31 18:43 Hui.shao
// restrict on state for provision-related operation
// 
// 3     08-07-21 11:51 Hui.shao
// check in the works of last weekend
// 
// 2     08-07-18 15:12 Hui.shao
// 
// 1     08-07-15 14:19 Hui.shao
// initial check in
// ===========================================================================

#ifndef __ZQTianShan_ContentState_H__
#define __ZQTianShan_ContentState_H__

#include "../common/TianShanDefines.h"

#include "ContentImpl.h"

namespace ZQTianShan {
namespace ContentStore {

class ContentImpl;

// -----------------------------
// class ContentStateBase
// -----------------------------
///@note any necessary lock must performed outside of state process
class ContentStateBase
{
public:
	/// constructor
	///@note no direct instantiation of ContentStateBase is allowed
    ContentStateBase(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::Storage::ContentState state);
    virtual ~ContentStateBase() {}

public:
	
	/// convert the state code to the display string
	static const char* stateStr(const ::TianShanIce::Storage::ContentState state);
	static ::TianShanIce::Storage::ContentState stateId(const char* stateStr);

	// impls of state activity
	
	/// The entry will be called when a contention's state is about to change to this state
	///@note state change validation should be done here
	///@note at least, notification of state change should be fired here. keep this pure virutal
	///      so that no instantiation of ContentStateBase will happen
	virtual void enter(const ::Ice::Current& c) {}

	/// this entry usually called by the successor ContentStateBase to allow to clean up some contention context
	virtual void leave(void) {}

public:	// impls of Provisioncontention and ProvisioncontentionEx

	/// An internal thread of ContentStore service will keep polling each contention, if it detected a contention's
	/// PollingTimer is expired, the thread will call this entry based its state
	///@note be sure to renew the timer for next event after the scheduled work is completed
	virtual void OnTimer(const ::Ice::Current& c);
	
	/// After ContentStore is restarted, the Provisioncontention objects will be restored from the database, a restore will be called
	/// for each contention, the state must how to respond this restoring
	virtual void OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c);

	void doPopulateFromFs(const ::Ice::Current& c);

protected:

	void _updateExpiration(const ::Ice::Long newExpiration=0);
	void _commitState(bool fireEvent=true, const ::std::string& msg="");
	bool _populateAttrFromFs();
	void _destroy();
	void _cancelProvision();

	//return true if the InService condition is met, else return false
	bool checkInServiceCondition(uint64 residentialStatusFlags);

protected:
	
	ContentStoreImpl& _store;
	ContentImpl&  _content;
	::TianShanIce::Storage::ContentState _oldState, _theState;
	std::string		_lastError;
};

// -----------------------------
// class ContentStateNotProvisioned
// -----------------------------
class ContentStateNotProvisioned : public ContentStateBase
{
public:

    ContentStateNotProvisioned(ContentStoreImpl& store, ContentImpl& content)
		: ContentStateBase(store, content, ::TianShanIce::Storage::csNotProvisioned) {}

public:	// impls of state activity
	
	virtual void enter();
	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c);
//	virtual void doPopulateFromFs(const ::Ice::Current& c);

public:
};

// -----------------------------
// class ContentStateProvisioning
// -----------------------------
class ContentStateProvisioning : public ContentStateBase
{
public:

    ContentStateProvisioning(ContentStoreImpl& store, ContentImpl& content)
		: ContentStateBase(store, content, ::TianShanIce::Storage::csProvisioning) {}

public:	// impls of state activity
	
	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c);
	virtual void leave(void);

};

// -----------------------------
// class ContentStateProvisioningStreamable
// -----------------------------
class ContentStateProvisioningStreamable : public ContentStateBase
{
public:

	ContentStateProvisioningStreamable(ContentStoreImpl& store, ContentImpl& content)
		: ContentStateBase(store, content, ::TianShanIce::Storage::csProvisioningStreamable) {}

public:	// impls of state activity

	virtual void enter();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c);
	virtual void leave(void);

public:	// impls of contention and contentionEx

};

// -----------------------------
// class ContentStateInService
// -----------------------------
class ContentStateInService : public ContentStateBase
{
public:

    ContentStateInService(ContentStoreImpl& store, ContentImpl& content) :
	  ContentStateBase(store, content, ::TianShanIce::Storage::csInService) {}

public:	// impls of state activity
	
	virtual void enter();
	virtual void leave();

	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c);
};

// -----------------------------
// class ContentStateOutService
// -----------------------------
class ContentStateOutService : public ContentStateBase
{
public:
	
    ContentStateOutService(ContentStoreImpl& store, ContentImpl& content) 
		: ContentStateBase(store, content, ::TianShanIce::Storage::csOutService) {}
	
public:	// impls of state activity
	
	virtual void enter();
	virtual void leave(void);
	
	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c);
//	virtual void doPopulateFromFs(const ::Ice::Current& c) {}
};

// -----------------------------
// class ContentStateCleaning
// -----------------------------
class ContentStateCleaning : public ContentStateBase
{
public:
	
    ContentStateCleaning(ContentStoreImpl& store, ContentImpl& content) 
		: ContentStateBase(store, content, ::TianShanIce::Storage::csCleaning) {}
	
public:	// impls of state activity
	
	virtual void enter();
	
	virtual void OnTimer(const ::Ice::Current& c);
	virtual void OnRestore(const std::string& stampLastFileWrite, const ::Ice::Current& c);
	virtual void doPopulateFromFs(const ::Ice::Current& c) {}

protected: // no one should be able to call leave() except self
	virtual void leave(void);
};

}} // namespace

#endif // __ZQTianShan_ContentState_H__
