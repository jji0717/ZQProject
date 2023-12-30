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
// Ident : $Id: SiteAdminImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SiteAdminImpl.h $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 11    08-04-21 15:50 Guan.han
// 
// 10    08-03-18 17:37 Xiaohui.chai
// changed interface of ListLiveTxn
// 
// 9     07-12-27 19:41 Hui.shao
// added resource restriction for virtual site
// 
// 8     07-12-13 19:43 Hui.shao
// 
// 7     07-12-13 18:27 Hui.shao
// 
// 6     07-12-10 18:47 Hui.shao
// moved event out of txn
// 
// 5     07-06-06 18:35 Hongquan.zhang
// 
// 4     07-04-20 15:09 Hongquan.zhang
// 
// 3     07-04-12 13:46 Hongquan.zhang
// 
// 2     07-03-23 15:02 Hui.shao
// 
// 1     07-03-15 19:02 Hui.shao
// 
// 12    07-03-13 17:12 Hongquan.zhang
// ===========================================================================

#ifndef __ZQTianShan_WeiwooImpl_H__
#define __ZQTianShan_WeiwooImpl_H__

#include "../common/TianShanDefines.h"
#include <ZQ_common_conf.h>
#include <locks.h>

#include "SiteAdminSvc.h"
#include "SiteAdminSvcEnv.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace Site {

#define DEFUALT_SITENAME "."
// -----------------------------
// service SiteAdminImpl
// -----------------------------
class VirtualSiteImpl : public TianShanIce::Site::VirtualSite , public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
private:
};

class SiteAdminImpl : public ::TianShanIce::Site::SiteAdmin
{
public:
    SiteAdminImpl(SiteAdminSvcEnv& env);
	virtual ~SiteAdminImpl();

public:	// impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// impls of BusinessRouter
	
    virtual ::TianShanIce::Site::VirtualSites listSites(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Application::PurchasePrx resolvePurchase(const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);

public:	// impls of SiteAdmin

    virtual bool updateSite(const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool removeSite(const ::std::string&, const ::Ice::Current& c);
    virtual bool setSiteProperties(const ::std::string& siteName, const ::TianShanIce::Properties& props, const ::Ice::Current& c);
    virtual ::TianShanIce::Properties getSiteProperties(const ::std::string& siteName, const ::Ice::Current& c) const;
    virtual ::TianShanIce::Site::AppInfos listApplications(const ::Ice::Current& c) const;
    virtual bool updateApplication(const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool removeApplication(const ::std::string&, const ::Ice::Current& c);
    virtual ::TianShanIce::Site::AppMounts listMounts(const ::std::string&, const ::Ice::Current& c) const;
    virtual ::TianShanIce::Site::AppMountPrx mountApplication(const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool unmountApplication(const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual ::TianShanIce::Site::AppInfo findApplication(const ::std::string&, const ::std::string&, const ::Ice::Current& c) const;

    virtual void commitStateChange(const ::std::string& sessId, ::TianShanIce::State state, const ::TianShanIce::SRM::SessionPrx& sess, const ::TianShanIce::Properties& props, const ::Ice::Current& c);

    virtual void listLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_listLiveTxnPtr& amdCB, const ::std::string& siteName, const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames, const ::std::string &startId, int maxCount, const ::Ice::Current& c) const;
    virtual void trace_async(const ::TianShanIce::Site::AMD_TxnService_tracePtr& amdCB, const ::std::string& sessId, const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg, const ::Ice::Current& c);

    virtual void postYTD(const ::std::string& sessId, const ::Ice::Current& c);
    virtual void setUserProperty(const ::std::string& sessId, const ::std::string& key, const ::std::string& value, const ::Ice::Current& c);

    virtual void dumpLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_dumpLiveTxnPtr& amdCB, const ::std::string& sessId, const ::std::string& beginFormat, const ::std::string& traceFormat, const ::std::string& endFormat, const ::Ice::Current& c);
//    virtual void dumpHistoryTxn_async(const ::TianShanIce::Site::AMD_TxnService_dumpHistoryTxnPtr&, const ::std::string& sessId, const ::std::string& beginFormat, const ::std::string& traceFormat, const ::std::string& endFormat, const ::Ice::Current& c);

    ::TianShanIce::Site::LiveTxnPrx newTxn(const ::std::string& sessId, ::Ice::Long bandwidth, const ::std::string& siteName, const ::std::string& appName, const ::std::string& mountPath, const ::Ice::Current& c);

	 virtual void updateSiteResourceLimited(const ::std::string& siteName, ::Ice::Long maxBW, ::Ice::Int maxSessions, const ::Ice::Current& c);
	 virtual void restrictSiteResources(const ::std::string& siteName, const ::TianShanIce::SRM::ResourceMap& resources, const ::Ice::Current& c);
	 virtual ::TianShanIce::SRM::ResourceMap getSiteResourceRestricutions(const ::std::string& siteName, const ::Ice::Current& c) const;

protected:
	
	SiteAdminSvcEnv&	_env;

	typedef struct _tagUsedSiteResource
	{
		Ice::Int			_usedSessions;
		Ice::Long			_usedBandwidth;
	} UsedSiteResource;

	typedef std::map<std::string,UsedSiteResource>		USEDRESMAP;
	USEDRESMAP				_usedResMap;
	ZQ::common::Mutex		_usedResLocker;
};

// -----------------------------
// class AppMountImpl
// -----------------------------
class AppMountImpl : public TianShanIce::Site::AppMount, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
	friend class SiteAdminImpl;

public:

    AppMountImpl(SiteAdminSvcEnv& env);

	typedef ::IceInternal::Handle< AppMountImpl> Ptr;

    virtual void destroy(const ::Ice::Current&);
    virtual ::std::string getMountedPath(const ::Ice::Current& c) const;
    virtual ::std::string getAppName(const ::Ice::Current& c) const;

protected:

	SiteAdminSvcEnv& _env;
};

// -----------------------------
// class LiveTxnImpl
// -----------------------------
class LiveTxnImpl : public TianShanIce::Site::LiveTxn, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
	friend class SiteAdminImpl;

public:

    LiveTxnImpl(SiteAdminSvcEnv& env);
	LiveTxnImpl(SiteAdminSvcEnv& env,const std::string& id);

	::std::string toString();

	typedef ::IceInternal::Handle< LiveTxnImpl> Ptr;

    virtual void destroy(const ::Ice::Current& c);

    virtual ::std::string getSessId(const ::Ice::Current& c) const;
	virtual ::std::string getSitename(const ::Ice::Current& c) const;
	virtual ::std::string getPath(const ::Ice::Current& c) const;
    virtual ::TianShanIce::State getState(const ::Ice::Current& c) const;

    virtual void setState(::TianShanIce::State state, const ::Ice::Current& c);

    virtual void setProperty(const ::std::string& key, const ::std::string& value, const ::Ice::Current& c);
    virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& c) const;

//    virtual void trace(const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg, const ::Ice::Current& c) const;

	virtual void	onTimer(const ::Ice::Current& c);

protected:
	void updateTimer(Ice::Long lMilliSeconds);

protected:

	SiteAdminSvcEnv&	_env;
	bool				_isAlive;
	
	::Ice::Long _lastTimeout; // thredhold to give up getting the destroy confirmation from SRM
};

// -----------------------------
// class TxnEventImpl
// -----------------------------
class TxnEventImpl : public TianShanIce::Site::TxnEvent, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
	friend class SiteAdminImpl;
	
public:
	typedef ::IceInternal::Handle< TxnEventImpl> Ptr;
	
    TxnEventImpl(SiteAdminSvcEnv& env);
	
    virtual void get(::Ice::Identity& identTxn, ::std::string& stampUTC, ::std::string& category, ::std::string& eventCode, ::std::string& eventMsg, const ::Ice::Current& c) const;

	virtual ::TianShanIce::Properties getEventInfo(const ::TianShanIce::StrValues& params, const ::Ice::Current& = ::Ice::Current()) const;
	
protected:
	
	SiteAdminSvcEnv&	_env;
};

}} // namespace

#endif // __ZQTianShan_WeiwooImpl_H__
