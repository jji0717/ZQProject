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
// Ident : $Id: SessionImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionImpl.h $
// 
// 3     1/12/16 8:53a Dejian.fei
// 
// 2     3/09/11 4:42p Hongquan.zhang
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 17    08-03-18 14:55 Hongquan.zhang
// lock servant if using it
// 
// 16    07-11-19 11:53 Hongquan.zhang
// 
// 15    07-06-21 16:30 Hongquan.zhang
// 
// 14    07-05-24 11:35 Hongquan.zhang
// 
// 13    07-04-12 14:02 Hongquan.zhang
// 
// 12    07-03-13 17:12 Hongquan.zhang
// 
// 11    07-01-05 10:59 Hongquan.zhang
// ===========================================================================

#ifndef __ZQTianShan_SessionImpl_H__
#define __ZQTianShan_SessionImpl_H__

#include "../common/TianShanDefines.h"

#include "WeiwooAdmin.h"
#include "WeiwooSvcEnv.h"

namespace ZQTianShan {
namespace Weiwoo {

// -----------------------------
// service SessionManagerImpl
// -----------------------------
class SessionManagerImpl : public ::TianShanIce::SRM::SessionAdmin
{
public:
    SessionManagerImpl(WeiwooSvcEnv& env);
	virtual ~SessionManagerImpl();

public:	// impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// impls of SessionManager

    virtual ::TianShanIce::SRM::SessionPrx openSession(const ::std::string& sessId, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::SessionPrx createSession(const ::TianShanIce::SRM::Resource& assetUri, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::SessionPrx createSessionBySSSI(const ::TianShanIce::SRM::Resource& assetUri, const ::TianShanIce::SRM::ResourceMap& resSSSI, const ::Ice::Current& c);

public:	// impls of SessionAdmin

protected:
	Ice::Identity _localId;
	WeiwooSvcEnv& _env;
};

// -----------------------------
// class SessionImpl
// -----------------------------
//class SessionImpl : public TianShanIce::SRM::SessionEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class SessionImpl : public TianShanIce::SRM::SessionEx, public ICEAbstractMutexRLock
{	
	friend class SessStateBase;
	friend class SessStateChangeGuard;

public:
	static ::std::string generateSessionID(void);

public:

    SessionImpl(WeiwooSvcEnv& env);
	typedef ::IceInternal::Handle< SessionImpl > Ptr;

public:	// impls of Session

    virtual void destroy(const ::Ice::Current& c);

    virtual ::std::string getId(const ::Ice::Current& c) const;
    virtual ::Ice::ObjectPrx getOwner(const ::Ice::Current& c) const;
    virtual ::TianShanIce::State getState(const ::Ice::Current& c) const;

    virtual void setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& value, const ::Ice::Current& c);
	virtual ::Ice::Int setPrivateData2(const ::TianShanIce::ValueMap& valMap, const ::Ice::Current& = ::Ice::Current());
    virtual ::TianShanIce::ValueMap getPrivateData(const ::Ice::Current& c) const;
    virtual ::Ice::Long addResource(::TianShanIce::SRM::ResourceType type, ::TianShanIce::SRM::ResourceAttribute attribute, const ::TianShanIce::ValueMap& resourceData, const ::Ice::Current& c);
	::Ice::Long addResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::ResourceMap getReources(const ::Ice::Current& c) const;
    virtual void removeResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c);
    virtual void negotiateResources(::TianShanIce::State state, const ::Ice::Current& c);
    virtual ::TianShanIce::Application::PurchasePrx getPurchase(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Streamer::StreamPrx getStream(const ::Ice::Current& c) const;
    virtual void renew(::Ice::Long TTL, const ::Ice::Current& c);

    virtual void provision_async(const ::TianShanIce::SRM::AMD_Session_provisionPtr& amdCB, const ::Ice::Current& c);
    virtual void provision(const ::Ice::Current& c);
    virtual void serve_async(const ::TianShanIce::SRM::AMD_Session_servePtr& amdCB, const ::Ice::Current& c);
    virtual void serve(const ::Ice::Current& c);

	virtual void onRestore(const ::Ice::Current& = ::Ice::Current()) ;

public:	// impls of SessionEx

    virtual void OnTimer(const ::Ice::Current& c);
    virtual void attachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c);
	
public:
	std::string				_lastError;
	::TianShanIce::State	_stateChangeTargetState;
protected:

	WeiwooSvcEnv&			_env;
	bool					_bStateInChange;	

	//ZQ::common::Mutex		_updateTimerMutex;
};

}} // namespace

#endif // __ZQTianShan_SessionImpl_H__
