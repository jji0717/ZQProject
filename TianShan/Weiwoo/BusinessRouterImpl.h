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
// Ident : $Id: BusinessRouterImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/BusinessRouterImpl.h $
// 
// 2     1/12/16 8:53a Dejian.fei
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 12    07-03-13 17:12 Hongquan.zhang
// ===========================================================================

#ifndef __ZQTianShan_WeiwooImpl_H__
#define __ZQTianShan_WeiwooImpl_H__

#include "../common/TianShanDefines.h"

#include "WeiwooAdmin.h"
#include "WeiwooSvcEnv.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace Weiwoo {

#define DEFUALT_SITENAME "."
// -----------------------------
// service BusinessRouterImpl
// -----------------------------
class BusinessRouterImpl : public ::TianShanIce::SRM::BusinessAdmin
{
public:
    BusinessRouterImpl(WeiwooSvcEnv& env);
	virtual ~BusinessRouterImpl();

public:	// impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// impls of BusinessRouter

    virtual ::TianShanIce::SRM::Sites listSites(const ::Ice::Current& c);
    virtual ::TianShanIce::Application::PurchasePrx resolvePurchase(const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);

public:	// impls of BusinessAdmin

    virtual bool updateSite(const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool removeSite(const ::std::string&, const ::Ice::Current& c);
    virtual bool setSiteProperties(const ::std::string& siteName, const ::TianShanIce::Properties& props, const ::Ice::Current& c);
    virtual ::TianShanIce::Properties getSiteProperties(const ::std::string& siteName, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::AppInfos listApplications(const ::Ice::Current& c);
    virtual bool updateApplication(const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool removeApplication(const ::std::string&, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::AppMounts listMounts(const ::std::string&, const ::Ice::Current& c);;
    virtual ::TianShanIce::SRM::AppMountPrx mountApplication(const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool unmountApplication(const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::AppInfo findApplication(const ::std::string&, const ::std::string&, const ::Ice::Current& c);

protected:
	
	WeiwooSvcEnv& _env;
};

// -----------------------------
// class AppMountImpl
// -----------------------------
//class AppMountImpl : public TianShanIce::SRM::AppMount, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class AppMountImpl : public TianShanIce::SRM::AppMount, public ICEAbstractMutexRLock
{
	friend class BusinessRouterImpl;

public:

    AppMountImpl(WeiwooSvcEnv& env);

	typedef ::IceInternal::Handle< AppMountImpl> Ptr;

    virtual void destroy(const ::Ice::Current&);
    virtual ::std::string getMountedPath(const ::Ice::Current& c) const;
    virtual ::std::string getAppName(const ::Ice::Current& c) const;

protected:

	WeiwooSvcEnv& _env;
};

}} // namespace

#endif // __ZQTianShan_WeiwooImpl_H__
