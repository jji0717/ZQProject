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
// Ident : $Id: BusinessRouterImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/BusinessRouterImpl.cpp $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 22    07-03-13 17:12 Hongquan.zhang
// 
// 21    06-12-25 15:51 Hui.shao
// uniform the throw
// 
// 20    06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 19    06-12-25 12:22 Hongquan.zhang
// 
// 18    06-12-13 18:48 Hongquan.zhang
// 
// 17    9/21/06 4:36p Hui.shao
// batch checkin 20060921
// 
// 16    06-09-12 20:19 Hui.shao
// added SessionWatchDog
// 
// 15    06-08-29 12:33 Hui.shao
// 
// 14    06-08-28 18:30 Hui.shao
// 
// 13    06-08-25 14:27 Hui.shao
// 
// 12    06-08-24 19:23 Hui.shao
// 
// 11    06-07-31 18:33 Hui.shao
// 
// 10    06-07-17 14:46 Hui.shao
// initial impl of session manager
// 7     06-07-12 13:38 Hui.shao
// added logs
// 3     06-07-05 15:46 Hui.shao
// console demo ready
// ===========================================================================

#include "BusinessRouterImpl.h"
#include "Log.h"
#include "URLStr.h"
#include "TsApplication.h"

namespace ZQTianShan {
namespace Weiwoo {
	
using namespace ZQ::common;

typedef ::std::vector< Ice::Identity > IdentCollection;
#define IdentityToObj(_CLASS, _ID) ::TianShanIce::SRM::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))

// -----------------------------
// service BusinessRouterImpl
// -----------------------------
BusinessRouterImpl::BusinessRouterImpl(WeiwooSvcEnv& env)
: _env(env)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "add the interface \"%s\" on to the adapter"), SERVICE_NAME_BusinessRouter);
    _env._adapter->add(this, _env._communicator->stringToIdentity(SERVICE_NAME_BusinessRouter));
}

BusinessRouterImpl::~BusinessRouterImpl()
{
//	_env._adapter->remove(_thisPrx);
}

::std::string BusinessRouterImpl::getAdminUri(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQ::common::_throw<::TianShanIce::NotImplemented> (envlog, __MSGLOC__ "TODO: impl here");
	return ""; // dummy statement to avoid compiler error
}

::TianShanIce::State BusinessRouterImpl::getState(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQ::common::_throw<::TianShanIce::NotImplemented> (envlog, __MSGLOC__ "TODO: impl here");
	return ::TianShanIce::stInService; // dummy statement to avoid compiler error
}

::TianShanIce::Application::PurchasePrx BusinessRouterImpl::resolvePurchase(const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c)
{
	CONN_TRACE(c, BusinessRouter, resolvePurchase);

	// step 1. read URI from session
	std::string tmpstr, sessid = sess->getId();	
	::TianShanIce::SRM::ResourceMap resMap = sess->getReources();

	// try to get the bandwidth requirement from the session context
	if (resMap.end() != resMap.find(::TianShanIce::SRM::rtURI))
	{
		::TianShanIce::SRM::Resource& resUri = resMap[::TianShanIce::SRM::rtURI];
		::TianShanIce::Variant& uri = resUri.resourceData["uri"];
		if (resUri.resourceData.end() != resUri.resourceData.find("uri") && 
										!resUri.resourceData["uri"].bRange &&
										resUri.resourceData["uri"].strs.size() >0)
			tmpstr = resUri.resourceData["uri"].strs[0];
	}

	if (tmpstr.empty())
	{		
		ZQ::common::_throw<::TianShanIce::InvalidParameter> (envlog, "BusinessRouter::resolvePurchase() sess[%s] no valid rtURI has been specified", sessid.c_str());
	}

	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(BusinessRouter, "resolvePurchase() sess[%s] resolve purchase for %s"), sessid.c_str(), tmpstr.c_str());

	ZQ::common::URLStr urlstr(tmpstr.c_str());
	std::string sitename = urlstr.getHost();
	std::string pathname = urlstr.getPath();

	// step 2. address the site and application
	::TianShanIce::Properties siteProps;

	try 
	{
		siteProps = getSiteProperties(sitename, c);
	}
	catch (...)
	{
		if (0 != sitename.compare(DEFUALT_SITENAME))
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(BusinessRouter, "resolvePurchase() site %s not found, try default site"), sitename.c_str());
			siteProps = getSiteProperties(DEFUALT_SITENAME, c); // no exception catch here, let it go to the up layers
			sitename = DEFUALT_SITENAME; // adjust if succeeded
		}
	}
	
	::TianShanIce::SRM::AppInfo appinfo = findApplication(sitename, pathname, c); // let the exception go to the up layers if failed

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(BusinessRouter, "resolvePurchase() connect to application[%s] at \"%s\""),
																appinfo.name.c_str(), appinfo.endpoint.c_str());

	// step 3. connect to the application
	::TianShanIce::Application::AppServicePrx app;

	try {
		// 3.1. trust the endpoint as the full proxy string
		app = ::TianShanIce::Application::AppServicePrx::checkedCast(_env._communicator->stringToProxy(appinfo.endpoint));
	}
	catch(const ::Ice::Exception& e)
	{
		app = NULL;
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "exception when connect to application: %s "), e.ice_name().c_str());
	}

	if (!app)
	{
		try 
		{
			// 3.2. trust the application published SERVICE_NAME_AppService interface
			std::string proxystr=std::string(SERVICE_NAME_AppService ":") + appinfo.endpoint;
			app = ::TianShanIce::Application::AppServicePrx::checkedCast(_env._communicator->stringToProxy(proxystr));
		}
		catch(const ::Ice::Exception& e)
		{
			app = NULL;
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "exception: %s"), e.ice_name().c_str());
		}
	}


	if (!app)
		ZQ::common::_throw <TianShanIce::ServerError> (envlog, "resolvePurchase() failed to connect to application[%s] at %s", appinfo.name.c_str(), appinfo.endpoint.c_str());

	return app->createPurchase(sess, siteProps);
}

// record operations start here
// ----------------------------------

#define IMPL_ListRecords(_COLLECTION, _DICT) \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "list " #_COLLECTION));\
	::TianShanIce::SRM::##_COLLECTION collection; \
	{ \
		ZQ::common::MutexGuard gd(_env._lock##_DICT); \
		for (TianShanIce::SRM::##_DICT::const_iterator it = _env._p##_DICT->begin(); \
		it != _env._p##_DICT->end(); it++) \
			collection.push_back(it->second); \
	} \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "%d " #_COLLECTION " found"), collection.size());\
	return collection;

::TianShanIce::SRM::Sites BusinessRouterImpl::listSites(const ::Ice::Current& c)
{
	CONN_TRACE(c, BusinessRouter, listSites);

	IMPL_ListRecords(Sites, SiteDict);
}

::TianShanIce::SRM::AppInfos BusinessRouterImpl::listApplications(const ::Ice::Current& c)
{
	CONN_TRACE(c, BusinessRouter, listApplications);

	IMPL_ListRecords(AppInfos, AppDict);
}

#define IMPL_UpdateRecord(_DICT, _KEY, _REC)  { \
	Freeze::TransactionHolder txHolder(_env._connBiz); \
	{ \
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "update " #_DICT "[%s]"), _KEY.c_str());\
		ZQ::common::MutexGuard gd(_env._lock##_DICT); \
		_env._p##_DICT->put(TianShanIce::SRM::##_DICT##::value_type(_KEY, _REC)); \
	} \
	txHolder.commit(); }

bool BusinessRouterImpl::updateSite(const ::std::string& name, const ::std::string& desc, const ::Ice::Current& c)
{
	CONN_TRACE(c, BusinessRouter, updateSite);

	TianShanIce::SRM::Site site;
	site.name = name;
	site.desc = desc;

	IMPL_UpdateRecord(SiteDict, name, site);
	return true;
}

bool BusinessRouterImpl::updateApplication(const ::std::string& name, const ::std::string& endpoint, const ::std::string& desc, const ::Ice::Current& c)
{
	CONN_TRACE(c, BusinessRouter, updateApplication);

	TianShanIce::SRM::AppInfo appinfo;
	appinfo.name	 = name;
	appinfo.endpoint = endpoint;
	appinfo.desc	 = desc;

	IMPL_UpdateRecord(AppDict, name, appinfo);
	return true;
}

#define IMPL_RemoveRecord(_DICT, _KEY)  { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "remove " #_DICT "[%s]"), _KEY.c_str());\
	Freeze::TransactionHolder txHolder(_env._connBiz); \
	ZQ::common::MutexGuard gd(_env._lock##_DICT); \
	_env._p##_DICT->erase(_KEY); \
	txHolder.commit(); }

#define IMPL_CleanLinkByRecord(_LINK, _IDX, _KEY) { \
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "clean " #_LINK " by " #_IDX "[%s]"), _KEY.c_str());\
		IdentCollection linkIds = _env._idx##_IDX->find(_KEY); \
		for (IdentCollection::iterator it = linkIds.begin(); it < linkIds.end(); it++) \
		{ \
			try	{ \
				::TianShanIce::SRM::##_LINK##Prx link = IdentityToObj(_LINK, *it); \
				if (link) link->destroy(); \
			} catch(...) {} \
		} }

bool BusinessRouterImpl::removeSite(const ::std::string& name, const ::Ice::Current& c)
{
	CONN_TRACE(c, BusinessRouter, removeSite);

	IMPL_RemoveRecord(SiteDict, name);
	IMPL_CleanLinkByRecord(AppMount, SiteToMount, name);

	return true;
}

bool BusinessRouterImpl::removeApplication(const ::std::string& name, const ::Ice::Current& c)
{
	CONN_TRACE(c, BusinessRouter, removeApplication);

	IMPL_RemoveRecord(AppDict, name);
	IMPL_CleanLinkByRecord(AppMount, AppToMount, name);

	return true;
}

::TianShanIce::Properties BusinessRouterImpl::getSiteProperties(const ::std::string& siteName, const ::Ice::Current& c)
{
	ZQ::common::MutexGuard gd(_env._lockSiteDict);
	TianShanIce::SRM::SiteDict::iterator it = _env._pSiteDict->findByName(siteName);
	if (_env._pSiteDict->end() == it)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(BusinessRouter, "getSiteProperties(%s) site not found"), siteName.c_str());
		ZQ::common::_throw<::TianShanIce::ServerError> (envlog, "BusinessRouter::getSiteProperties(%s) site not found", siteName.c_str());
	}

	try {
		return it->second.properties;
	}
	catch(...)
	{
		ZQ::common::_throw<::TianShanIce::ServerError> (envlog, "BusinessRouter::getSiteProperties(%s) properties not found", siteName.c_str());
	}
}

bool BusinessRouterImpl::setSiteProperties(const ::std::string& siteName, const ::TianShanIce::Properties& props, const ::Ice::Current& c)
{
	TianShanIce::SRM::Site record;

	try {
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		TianShanIce::SRM::SiteDict::iterator it = _env._pSiteDict->findByName(siteName);
		if (_env._pSiteDict->end() == it)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(BusinessRouter, "setSiteProperties(%s) site not found"), siteName.c_str());
			return false;
		}
		
		record = it->second;
	}
	catch(...) {}

	record.properties = props;

	Freeze::TransactionHolder txHolder(_env._connBiz);
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "setSiteProperties(%s) update properties"), siteName.c_str());
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		_env._pSiteDict->put(TianShanIce::SRM::SiteDict::value_type(siteName, record));
	}
	txHolder.commit();

	return true;
}

::TianShanIce::SRM::AppMounts BusinessRouterImpl::listMounts(const ::std::string& siteName, const ::Ice::Current& c)
{
	::TianShanIce::SRM::AppMounts links;
	IdentCollection identities;
	try {
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "listMounts(%s) search for mounts by site"), siteName.c_str());
		ZQ::common::MutexGuard gd(_env._lockAppMount);
		identities = _env._idxSiteToMount->find(siteName);
	} catch(...) {}
	
	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++)
	{
		::TianShanIce::SRM::AppMountPrx linkprx = (IdentityToObj(AppMount, *it));
		if (linkprx)
			links.push_back(linkprx);
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "listMounts(%s) %d application mounts found"), siteName.c_str(), links.size());
	return links;
}

::TianShanIce::SRM::AppMountPrx BusinessRouterImpl::mountApplication(const ::std::string& siteName, const ::std::string& mountPath, const ::std::string& appName, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "mountApplication(%s) \"%s\" => app[%s]"), siteName.c_str(), mountPath.c_str(), appName.c_str());
	AppMountImpl::Ptr link = new AppMountImpl(_env);
	link->ident.name = IceUtil::generateUUID(); link->ident.category = "AppMount";
	link->siteName = siteName; link->mountedPath = mountPath; link->appName = appName;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "check if the site and application information exists"));
	// check if the site and application information has been configed
	if (_env._pSiteDict->end() == _env._pSiteDict->findByName(siteName))
		ZQ::common::_throw<::TianShanIce::ServerError>(envlog, CLOGFMT(BusinessRouter, "No such site \"%s\""), siteName.c_str());

	if (_env._pAppDict->end() == _env._pAppDict->findByName(appName))
		ZQ::common::_throw<::TianShanIce::ServerError> (envlog, CLOGFMT(BusinessRouter, "No such application \"%s\""), appName.c_str());

    // be sure that no same mount exists
	::TianShanIce::SRM::AppMountPrx mount;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "clean up the mount with same path but different application, or address the existing the matched mount"));
	IdentCollection ids = _env._idxSiteToMount->find(siteName);
	IdentCollection::iterator it = ids.begin();
	for (it = ids.begin(); it < ids.end(); it++)
	{
		try	{

			mount = IdentityToObj(AppMount, *it);
			
			if (mount && mount->getMountedPath() == mountPath)
			{
				if(mount->getAppName() != appName) // remove the old mountage
					mount->destroy();
				else
					break;
			}

		} catch(...) {}
	}

	if (it == ids.end())
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "add a new mount"));
		// add the new mountage
		_env._eAppMount->add(link, link->ident);
		mount = IdentityToObj(AppMount, link->ident);
	}

    return mount;
}

bool BusinessRouterImpl::unmountApplication(const ::std::string& siteName, const ::std::string& mountedPath, const ::Ice::Current& c)
{
	IdentCollection ids = _env._idxSiteToMount->find(siteName);
	
	int count =0;
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "address the mount: [%s/%s] to unmount"), siteName.c_str(), mountedPath.c_str());
	for (IdentCollection::iterator it = ids.begin(); it < ids.end(); it++)
	{
		try	{
			::TianShanIce::SRM::AppMountPrx mount = IdentityToObj(AppMount, *it);
			if (mount->getMountedPath() == mountedPath)
			{
				mount->destroy();
				count++;
			}
		} catch(...) {}
	}

	return (count>0);
}

::TianShanIce::SRM::AppInfo BusinessRouterImpl::findApplication(const ::std::string& siteName, const ::std::string& mountedPath, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "address the mount: [%s/%s]"), siteName.c_str(), mountedPath.c_str());

	IdentCollection ids = _env._idxSiteToMount->find(siteName);
	IdentCollection::iterator it = ids.begin();
	::TianShanIce::SRM::AppMountPrx mount;
	for (it = ids.begin(); it < ids.end(); it++)
	{
		try	{
			mount = IdentityToObj(AppMount, *it);
			if (mount->getMountedPath() == mountedPath)
				break;

		} catch(...) {}
	}

	if (ids.end() ==it)
		ZQ::common::_throw<::TianShanIce::ServerError> (envlog, "BusinessRouter::findApplication() failed to address the mount: [%s/%s]", siteName.c_str(), mountedPath.c_str());

	mount = IdentityToObj(AppMount, *it);

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(BusinessRouter, "opening application info: [%s]"), mount->getAppName().c_str());
	ZQ::common::MutexGuard gd(_env._lockAppDict);
	::TianShanIce::SRM::AppDict::iterator itDict =  _env._pAppDict->findByName(mount->getAppName());

	if (_env._pAppDict->end() == itDict)
		ZQ::common::_throw <::TianShanIce::ServerError> (envlog, "BusinessRouter::findApplication() failed to find application info: [%s]", mount->getAppName());

	return (itDict->second);
}

// -----------------------------
// class AppMountImpl
// -----------------------------
AppMountImpl::AppMountImpl(WeiwooSvcEnv& env)
:_env(env)
{
}

void AppMountImpl::destroy(const ::Ice::Current& c)
{
	WLock sync(*this);
	try
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AppMount, "destroy(id=%s)"), c.id.name.c_str());
		_env._eAppMount->remove(c.id);
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AppMount, "destroy(id=%s) object already gone, ignore"), c.id.name.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQ::common::_throw <::TianShanIce::ServerError> (envlog, CLOGFMT(AppMount, "destroy(id=%s) caught DB exception:"), c.id.name.c_str(), ex.ice_name().c_str());
	}
}

::std::string AppMountImpl::getMountedPath(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return mountedPath;
}

::std::string AppMountImpl::getAppName(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return appName;
}


}} // namespace
