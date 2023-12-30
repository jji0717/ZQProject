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
// Ident : $Id: SiteAdminImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SiteAdminImpl.cpp $
// 
// 3     12/12/13 2:00p Hui.shao
// %lld for int64
// 
// 2     4/23/12 5:59p Hui.shao
// call txn->destroy() anyway in postYTD()
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 50    09-04-23 14:21 Xiaohui.chai
// put sitename and path into private data
// 
// 49    09-02-20 14:14 Xiaohui.chai
// add preferred application override support
// 
// 48    08-11-24 15:30 Hongquan.zhang
// 
// 47    08-11-24 11:48 Hongquan.zhang
// 
// 46    08-10-07 12:44 Xiaohui.chai
// 
// 45    08-10-07 12:32 Xiaohui.chai
// watch txn on outofservice
// 
// 44    08-09-11 17:51 Xiaohui.chai
// Added feature: URL mode.
// 
// 43    08-08-19 12:43 Hongquan.zhang
// 
// 42    08-07-25 17:37 Xiaohui.chai
// Changed the MDB schema
// 
// 41    08-04-30 16:25 Xiaohui.chai
// 
// 40    08-04-21 15:50 Guan.han
// 
// 39    08-03-18 17:37 Xiaohui.chai
// changed interface of ListLiveTxn
// 
// 38    08-02-18 21:03 Hui.shao
// added per-site resource restriction on rtStorage and rtStreamer
// 
// 37    07-12-27 19:51 Hui.shao
// 
// 36    07-12-27 19:41 Hui.shao
// added resource restriction for virtual site
// 
// 35    07-12-26 12:00 Hongquan.zhang
// 
// 34    07-12-26 11:45 Hongquan.zhang
// fix bug in siteadmin 
// update site information will cover the previous useful information
// 
// 33    07-12-14 16:37 Xiaohui.chai
// 
// 32    07-12-13 19:42 Hui.shao
// 
// 31    07-12-13 18:36 Xiaohui.chai
// 
// 30    07-12-13 18:30 Hui.shao
// 
// 29    07-12-10 18:47 Hui.shao
// moved event out of txn
// 
// 28    07-11-26 15:34 Hongquan.zhang
// 
// 27    07-11-20 14:22 Hongquan.zhang
// 
// 26    07-10-25 14:09 Hongquan.zhang
// 
// 25    07-09-18 12:56 Hongquan.zhang
// 
// 24    07-08-30 15:38 Hongquan.zhang
// 
// 23    07-08-17 13:55 Hongquan.zhang
// 
// 22    07-08-15 16:18 Hongquan.zhang
// 
// 21    07-08-01 10:59 Hongquan.zhang
// 
// 20    07-07-27 14:43 Hongquan.zhang
// 
// 20    07-07-27 11:24 Hongquan.zhang
// 
// 19    07-07-25 16:00 Hongquan.zhang
// 
// 19    07-07-24 16:23 Hongquan.zhang
// 
// 18    07-07-05 12:06 Hongquan.zhang
// 
// 18    07-07-05 12:05 Hongquan.zhang
// 
// 17    07-07-04 17:48 Hongquan.zhang
// 
// 17    07-07-04 12:36 Hongquan.zhang
// 
// 16    07-06-28 13:12 Hongquan.zhang
// 
// 15    07-06-28 13:02 Hongquan.zhang
// 
// 14    07-06-21 15:37 Hongquan.zhang
// 
// 13    07-06-18 11:43 Hongquan.zhang
// 
// 12    07-06-18 10:08 Hongquan.zhang
// 
// 11    07-06-15 18:50 Hongquan.zhang
// 
// 10    07-06-15 18:48 Hongquan.zhang
// 
// 9     07-06-15 17:49 Hongquan.zhang
// 
// 8     07-06-06 18:34 Hongquan.zhang
// 
// 6     07-05-23 13:33 Hui.shao
// use _IceThrow
// 
// 5     07-05-14 15:50 Hongquan.zhang
// 
// 4     07-04-20 15:09 Hongquan.zhang
// 
// 3     07-04-12 13:46 Hongquan.zhang
// 
// 2     07-03-23 15:02 Hui.shao
// 
// 1     07-03-15 19:02 Hui.shao
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
// 10    06-07-17 14:46 Hui.shao
// initial impl of session manager
// 7     06-07-12 13:38 Hui.shao
// added logs
// 3     06-07-05 15:46 Hui.shao
// console demo ready
// ===========================================================================
#include <time.h>

#include "SiteAdminImpl.h"
#include "Log.h"
#include "URLStr.h"
#include "TsApplication.h"
#include "SACommand.h"
#include "TsSRM.h"
#include "SiteDefines.h"


#ifdef _WITH_EVENTSENDER_
	#include "eventsendermanager.h"
	extern ZQTianShan::Site::EventSenderManager*	g_pEventSinkMan;
#endif //_WITH_EVENTSENDER_

namespace ZQTianShan {
namespace Site {
	
using namespace ZQ::common;

typedef ::std::vector< Ice::Identity > IdentCollection;
#define IdentityToObj(_CLASS, _ID) ::TianShanIce::Site::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#define CATEG_Txn			"Txn"

#if defined ZQ_OS_MSWIN
	#define	SERVFMT(x,y) 	"[SiteAdminImpl]%s TID[%08u][%16s]\t"##y,sessid.c_str(),GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SERVFMT(x,y) 	"[SiteAdminImpl]%s TID[%08d][%16s]\t"##y,sessId.c_str(),pthread_self(),#x
#endif

// -----------------------------
// service SiteAdminImpl
// -----------------------------
SiteAdminImpl::SiteAdminImpl (SiteAdminSvcEnv& env)
: _env(env)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "add the interface \"%s\" on to the adapter"), SERVICE_NAME_BusinessRouter);

    _env._adapter->ZQADAPTER_ADD(_env._communicator, this, SERVICE_NAME_BusinessRouter);

#pragma message(__MSGLOC__"TODO:list all on-going transaction")

	//start up scan all on-going live Transaction , sum the total used bandwidth and total session count
	
	
	::TianShanIce::Site::LiveTxnPrx livetxn;

	Freeze::EvictorIteratorPtr itPtr = _env._eLiveTxn->getIterator("",1);
	typedef std::vector<Ice::Identity>	RESULT;
	RESULT	r;
	long bw = 0;
	if (itPtr)
	{
		while (itPtr->hasNext()) 
		{
			r.push_back(itPtr->next());
		}

		ZQ::common::MutexGuard gd(_usedResLocker);
		for(RESULT::iterator it=r.begin();it!=r.end();it++)
		{				
			try
			{
				livetxn = TianShanIce::Site::LiveTxnPrx::checkedCast(env._adapter->createProxy(*it));
				if(!livetxn)
					continue;
				UsedSiteResource usr;				
				::TianShanIce::Properties props = livetxn->getProperties();
				if (props.end() != props.find(SYS_PROP(bandwidth)))
					sscanf(props[SYS_PROP(bandwidth)].c_str(), "%d", &bw);					
				std::string	strSiteName = livetxn->getSitename();
				if(_usedResMap.find(strSiteName)!=_usedResMap.end())
				{
					usr = _usedResMap[strSiteName];
					usr._usedBandwidth += bw/1000;
					usr._usedBandwidth += (bw%1000)>0 ? 1:0 ;
					usr._usedSessions  ++;
					_usedResMap[strSiteName]=usr;
				}
				else
				{
					usr._usedBandwidth = bw/1000;
					usr._usedBandwidth += (bw%1000)>0 ? 1:0 ;
					usr._usedSessions	= 1;
					_usedResMap[strSiteName] = usr;
				}
				_env._txnWatchDog->WatchMe(it->name,1000);
			}
			catch(Ice::ObjectNotExistException&)
			{
			}
			catch (...) 
			{
			}
		}
	}

	for(USEDRESMAP::const_iterator it=_usedResMap.begin(); it != _usedResMap.end() ; it++)
	{
		envlog(ZQ::common::Log::L_INFO,
			("SiteAdmin construction:  usedbW(%lldKB) usedSession(%d) in Site(%s)"),
			it->second._usedBandwidth,it->second._usedSessions,it->first.c_str());
	}	

}

SiteAdminImpl::~SiteAdminImpl()
{
//	_env._adapter->remove(_thisPrx);
}

::std::string SiteAdminImpl::getAdminUri(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	::ZQTianShan::_IceThrow<::TianShanIce::NotImplemented> (envlog, EXPFMT(SiteAdmin, 101, __MSGLOC__ "TODO: impl here"));
	return ""; // dummy statement to avoid compiler error
}

::TianShanIce::State SiteAdminImpl::getState(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	::ZQTianShan::_IceThrow<::TianShanIce::NotImplemented> (envlog, EXPFMT(SiteAdmin, 201, __MSGLOC__ "TODO: impl here"));
	return ::TianShanIce::stInService; // dummy statement to avoid compiler error
}

::TianShanIce::Application::PurchasePrx SiteAdminImpl::resolvePurchase(const ::TianShanIce::SRM::SessionPrx& sess, 
																	   const ::Ice::Current& c)
{
	CONN_TRACE(c, SiteAdmin, resolvePurchase);

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
		::ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog, EXPFMT(SiteAdmin, 301, "resolvePurchase() sess[%s] no valid rtURI has been specified"), sessid.c_str());

	envlog(ZQ::common::Log::L_DEBUG, 
		SERVFMT(SiteAdmin, "resolvePurchase() sess[%s] resolve purchase for %s"), sessid.c_str(), tmpstr.c_str());

	ZQ::common::URLStr urlstr(tmpstr.c_str());
	std::string sitename, pathname;
    std::string urlHost = urlstr.getHost();
    std::string urlPath = urlstr.getPath();

    // verify the url base on scheme: DNS mode / path mode

    ZQ::common::stringHelper::STRINGVECTOR stdpath;
    ZQ::common::stringHelper::SplitString(urlPath, stdpath, "/", "/");

#define UrlMode gSiteAdminConfig.urlMode.c_str()
#define IsPathMode (0 == stricmp(UrlMode, "Path"))
#define IsDNSMode (!IsPathMode)
    if(IsDNSMode)
    {
        if(stdpath.size() == 1)
        { // use the url path as the app path
            sitename = urlHost;
            pathname = urlPath;
        }
        else
        { // invalid url
            ::ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog, EXPFMT(SiteAdmin, 301, "resolvePurchase() sess[%s] invalid URI [%s] in DNS mode."), sessid.c_str(), tmpstr.c_str());
        }
    }
    else if (IsPathMode)
    { // extract the site name and the app path from the url path
        switch(stdpath.size())
        {
        case 2:
            sitename = stdpath[0];
            pathname = stdpath[1];
            break;
        case 1: // only app path, use the default site
            envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "resolvePurchase() Use default site in Path mode"));
            sitename = DEFUALT_SITENAME;
            pathname = urlPath;
            break;
        default: // invalid url
            ::ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog, EXPFMT(SiteAdmin, 301, "resolvePurchase() sess[%s] invalid URI [%s] in Path mode"), sessid.c_str(), tmpstr.c_str());
        }
    }
    else
    { // shouldn't happen
        envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SiteAdmin, "resolvePurchase() Unknown url mode [%s]"), UrlMode);
    }


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
			envlog(ZQ::common::Log::L_WARNING, SERVFMT(SiteAdmin, "resolvePurchase() site %s not found, try default site"), sitename.c_str());
			siteProps = getSiteProperties(DEFUALT_SITENAME, c); // no exception catch here, let it go to the up layers
			sitename = DEFUALT_SITENAME; // adjust if succeeded
		}
	}
    {
        // add the sitename & path into private data
        TianShanIce::ValueMap pd;
        TianShanIce::Variant v;
        v.type = TianShanIce::vtStrings;
        v.bRange = false;

        v.strs.clear();
        v.strs.push_back(sitename);
        pd[PD_KEY_SiteName] = v; // site name

        v.strs.clear();
        v.strs.push_back(pathname);
        pd[PD_KEY_Path] = v; // path

        try
        {
            sess->setPrivateData2(pd);
        }
        catch(...)
        {
            envlog(ZQ::common::Log::L_WARNING, SERVFMT(SiteAdmin, "resolvePurchase() Failed to set site name (%s) and path (%s) into session private data."), sitename.c_str(), pathname.c_str());
        }
    }
	// step 2.1 if per-site quota has been defined, validate the quota
	::Ice::Long maxDownstreamBwKbps = 0;
	::Ice::Int  maxSessions = 0;
	try 
	{
		::std::string val = siteProps[SYS_PROP(maxDownstreamBwKbps)];
		sscanf(val.c_str(), "%lld", &maxDownstreamBwKbps);
	}
	catch (...) {}
	
	try 
	{
		::std::string val = siteProps[SYS_PROP(maxSessions)];
		sscanf(val.c_str(), "%d", &maxSessions);
	}
	catch (...) {}

	// step 2.2 query if per-site resource restriction is define
	::TianShanIce::SRM::ResourceMap siteResRestrictions;
	try 
	{
		siteResRestrictions = getSiteResourceRestricutions(sitename, c);
	}
	catch (...) {}


#define AUX_Fetch_Variant_String(vMap, key, value) {\
            TianShanIce::ValueMap::const_iterator it = vMap.find(key);\
            if((it != vMap.end()) && (it->second.type == TianShanIce::vtStrings) && (!it->second.strs.empty()))\
                value = it->second.strs[0];\
            else\
                value = "";\
            }
    // support the preferred app override
#define KEY_Pref_App_Endpoint "sys.PrefAppEndpoint"
    ::TianShanIce::Site::AppInfo appinfo;
    {
        TianShanIce::ValueMap pd = sess->getPrivateData();
        AUX_Fetch_Variant_String(pd, KEY_Pref_App_Endpoint, appinfo.endpoint);
        if(!appinfo.endpoint.empty())
        { // has preferred application
            // parse the endpoint
            std::string::size_type d = appinfo.endpoint.find(':');
            if(d != std::string::npos)
            { // extract the app name
                appinfo.name = appinfo.endpoint.substr(0, d);
            }
            else
            {
                appinfo.name.clear();
            }
            appinfo.desc = "Preferred application";

            envlog(ZQ::common::Log::L_INFO, SERVFMT(SiteAdmin, "resolvePurchase() detect preferred application [%s] at [%s]"), appinfo.name.c_str(), appinfo.endpoint.c_str());
        }
        else
        {
            appinfo = findApplication(sitename, pathname, c); // let the exception go to the upper layers if failed
        }
    }

	envlog(ZQ::common::Log::L_INFO, SERVFMT(SiteAdmin, "resolvePurchase() connect to application[%s] at \"%s\""),
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
		envlog(ZQ::common::Log::L_DEBUG, SERVFMT(SiteAdmin, "exception when connect to application: %s "), e.ice_name().c_str());
	}

	std::string proxystr;
	if (!app)
	{
		try 
		{
			// 3.2. trust the application published SERVICE_NAME_AppService interface
			proxystr=std::string(SERVICE_NAME_AppService ":") + appinfo.endpoint;
			app = ::TianShanIce::Application::AppServicePrx::checkedCast(_env._communicator->stringToProxy(proxystr));
		}
		catch(const ::Ice::Exception& e)
		{
			app = NULL;
			envlog(ZQ::common::Log::L_DEBUG, SERVFMT(SiteAdmin, "caught exception when connect to APP[%s]: %s"), 
				proxystr.c_str(), e.ice_name().c_str());
		}
	}
	else
	{
		proxystr = _env._adapter->getCommunicator()->proxyToString( app );
	}

	if (!app)
		::ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 302, "resolvePurchase() failed to connect to application[%s] at %s"), appinfo.name.c_str(), appinfo.endpoint.c_str());
	
#pragma message(__MSGLOC__ "TODO: restrict the resource if there is any restriction")
	
	if (siteResRestrictions.end() != siteResRestrictions.find(TianShanIce::SRM::rtStorage))
	{
		envlog(ZQ::common::Log::L_DEBUG, 
			SERVFMT(SiteAdmin, "resolvePurchase() sess[%s] site[%s] has storage restriction, applying onto session"),
			sessid.c_str(), sitename.c_str());

		const ::TianShanIce::SRM::Resource& resOfSite = siteResRestrictions[TianShanIce::SRM::rtStorage];
		
		if (resMap.end() == resMap.find(TianShanIce::SRM::rtStorage))
		{
			envlog(ZQ::common::Log::L_DEBUG, 
				SERVFMT(SiteAdmin, "no resource[rtStorage] in session[%s], apply site[%s] restriction directly"), 
				sessid.c_str(), sitename.c_str());

			sess->addResourceEx(TianShanIce::SRM::rtStorage, resOfSite);
		}
		else
		{
			const ::TianShanIce::SRM::Resource& resOfSess = resMap[TianShanIce::SRM::rtStorage];
			::TianShanIce::SRM::Resource resIntersection;
			if (!InterRestrictResource(resOfSess, resOfSite, resIntersection))
				::ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, 
					EXPFMT(SiteAdmin, 312, "resolvePurchase() failed to intersection the resource[rtStorage] between session[%s] and site[%s] restriction"), 
					sessid.c_str(), sitename.c_str());
				
			resIntersection.attr	= resOfSess.attr;
			resIntersection.status	= resOfSess.status;
			sess->addResourceEx(TianShanIce::SRM::rtStorage, resIntersection);
		}
	}
	
	if (siteResRestrictions.end() != siteResRestrictions.find(TianShanIce::SRM::rtStreamer))
	{
		envlog(ZQ::common::Log::L_DEBUG, 
			SERVFMT(SiteAdmin, "resolvePurchase() sess[%s] site[%s] has streamer restriction, applying onto session"),
			sessid.c_str(), sitename.c_str());

		const ::TianShanIce::SRM::Resource& resOfSite = siteResRestrictions[TianShanIce::SRM::rtStreamer];
		
		if (resMap.end() == resMap.find(TianShanIce::SRM::rtStreamer))
		{
			envlog(ZQ::common::Log::L_DEBUG, 
				SERVFMT(SiteAdmin, "no resource[rtStreamer] in session[%s], apply site[%s] restriction directly"), 
				sessid.c_str(), sitename.c_str());

			sess->addResourceEx(TianShanIce::SRM::rtStreamer, resOfSite);
		}
		else
		{
			const ::TianShanIce::SRM::Resource& resOfSess = resMap[TianShanIce::SRM::rtStreamer];
			::TianShanIce::SRM::Resource resIntersection;
			if (!InterRestrictResource(resOfSess, resOfSite, resIntersection))
				::ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 313, "resolvePurchase() failed to intersection the resource[rtStreamer] between session[%s] and site[%s] restriction"), sessid.c_str(), sitename.c_str());
				
			resIntersection.attr = resOfSess.attr;
			resIntersection.status = resOfSess.status;
			sess->addResourceEx(TianShanIce::SRM::rtStreamer, resIntersection);
		}
	}

	//create purchase
	::TianShanIce::Application::PurchasePrx purchasePrx;
	try
	{
		try
		{
			envlog(ZQ::common::Log::L_DEBUG,
				SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] createPurchase through [%s]"),
				sessid.c_str(),proxystr.c_str());

			DWORD dwStart = GetTickCount();
			purchasePrx = app->createPurchase(sess, siteProps);
			DWORD dwNow = GetTickCount();
			envlog(ZQ::common::Log::L_DEBUG,
				SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] createPurchase through [%s] ok and use time [%d]"),
				sessid.c_str(), proxystr.c_str(), dwNow - dwStart);

			dwStart = dwNow;
			purchasePrx->provision();
			dwNow = GetTickCount();

			envlog(ZQ::common::Log::L_DEBUG, 
				SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] purchase was provisioned, took %dms"), 
				sessid.c_str(), dwNow -dwStart);
		}
		catch (const ::TianShanIce::BaseException& ex) 
		{
			envlog(ZQ::common::Log::L_ERROR,
				SERVFMT(SiteAdmin,"TianShan exception caught when create and provison purchase:%s"),
				ex.ice_name().c_str() );
			_IceReThrow(TianShanIce::ServerError, ex);
		}
		catch (Ice::Exception& ex) 
		{
			envlog(ZQ::common::Log::L_ERROR,SERVFMT(SiteAdmin,"ice exception [%s]  caught when CreatePurchase for Session %s"),
				ex.ice_name().c_str(),sessid.c_str());
			ex.ice_throw();
		}

		envlog(ZQ::common::Log::L_DEBUG,
			SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] get resource from SRM session"), 
			sessid.c_str());
		TianShanIce::SRM::ResourceMap resMap =  sess->getReources();

		envlog(ZQ::common::Log::L_DEBUG,
			SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] get resource from SRM session ok and find bandwidth resource"),
			sessid.c_str());

		TianShanIce::SRM::ResourceMap::const_iterator itRes=resMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
		if( itRes == resMap.end() )
		{
			//no bandwidth is found,invalid purchase
			TianShanIce::Properties prop;
			prop[SYS_PROP(terminateReason)]= "213001 no bandwidth information is given after purchase is resolved";
			TianShanIce::Variant var ;
			var.type = TianShanIce::vtStrings;
			var.bRange = false;
			var.strs.clear ();
			var.strs.push_back ( prop[SYS_PROP(terminateReason)] );			
			sess->setPrivateData ( SYS_PROP(terminateReason) , var);
			purchasePrx->detach(sessid , prop);
			purchasePrx=NULL;
			envlog(ZQ::common::Log::L_ERROR,SERVFMT(SiteAdmin,"No bandwidth is got after provision purchase"));
			::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(SiteAdmin, 303, "resolvePurchase() sess[%s] no bandwidth information is given after purchase is resolved"), sessid.c_str());
		}

		TianShanIce::ValueMap::const_iterator itVal=itRes->second.resourceData.find("bandwidth");
		if(itVal == itRes->second.resourceData.end()||itVal->second.lints.size() <= 0)
		{
			TianShanIce::Properties prop;
			prop[SYS_PROP(terminateReason)]= "213001 no bandwidth information is given after purchase is resolved";
			TianShanIce::Variant var ;
			var.bRange = false;
			var.type = TianShanIce::vtStrings;
			var.strs.clear ();
			var.strs.push_back ( prop[SYS_PROP(terminateReason)] );			
			sess->setPrivateData ( SYS_PROP(terminateReason) , var);
			purchasePrx->detach(sessid , prop);
			purchasePrx=NULL;
			envlog(ZQ::common::Log::L_ERROR,SERVFMT(SiteAdmin,"No bandwidth is got after provision purchase"));
			::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(SiteAdmin, 304, "resolvePurchase() sess[%s] no bandwidth information is given after purchase is resolved"), sessid.c_str());
		}

		Ice::Long lBandWidth = itVal->second.lints[0];
		envlog(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] get bandwidth [%lld] through SRM session"),sessid.c_str(),lBandWidth);
		//check if bandwidth or session count exceed
		Ice::Long curUsedBW=0;
		Ice::Long curUsedSession=0;
		{
			//find current used bandwidth and session count
			ZQ::common::MutexGuard gd(_usedResLocker);
			USEDRESMAP::iterator itUsedRes = _usedResMap.find(sitename);
			if( itUsedRes == _usedResMap.end() )
			{
				//create one
//				purchasePrx->detach(sess);
//				purchasePrx=NULL;
//				::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT0(SiteAdmin, "resolvePurchase() Can't find used resource with sitename=%s"),sitename.c_str());
				UsedSiteResource usr;
				usr._usedBandwidth = 0;
				usr._usedSessions  = 0;
				_usedResMap[sitename]=usr;
			}
			itUsedRes = _usedResMap.find(sitename);
			
			Ice::Long lTempBandwith= (lBandWidth/1000) + (lBandWidth%1000 > 0 ? 1 : 0);
			if(maxDownstreamBwKbps < itUsedRes->second._usedBandwidth + lTempBandwith 
				|| maxSessions < itUsedRes->second._usedSessions + 1)
			{
				TianShanIce::Properties prop;
				char	szBuf[1024];
				sprintf(szBuf,"213002 not enough resource for create a new purchase with site=%s neededBW=%lld(KBPS) "
					"currentBW=%lld(KB),currentSession=%d,totalBW=%lld(KB),totalSess=%d",
					sitename.c_str(),lTempBandwith,
					itUsedRes->second._usedBandwidth,itUsedRes->second._usedSessions,
					maxDownstreamBwKbps,maxSessions);
				
				std::string	strTerminateReason = szBuf;
				prop[SYS_PROP(terminateReason)]= strTerminateReason;
				TianShanIce::Variant var ;
				var.bRange = false;
				var.type = TianShanIce::vtStrings;
				var.strs.clear ();
				var.strs.push_back ( strTerminateReason );
				
				sess->setPrivateData ( SYS_PROP(terminateReason) , var);
				purchasePrx->detach(sessid,prop);
				purchasePrx=NULL;
				::ZQTianShan::_IceThrow<TianShanIce::Site::OutOfQuota>(envlog,
					EXPFMT(SiteAdmin, 305, "resolvePurchase() session[%s] not enough resource for create a new purchase with site=%s neededBW=%lld(KBPS) "
					"currentBW=%lld(KB),currentSession=%d,totalBW=%lld(KB),totalSess=%d"),
					sessid.c_str(),
					sitename.c_str(),lTempBandwith,
					itUsedRes->second._usedBandwidth,itUsedRes->second._usedSessions,
					maxDownstreamBwKbps,maxSessions);
			}

			itUsedRes->second._usedBandwidth += lTempBandwith;
			itUsedRes->second._usedSessions++;

			curUsedBW	=	 itUsedRes->second._usedBandwidth;
			curUsedSession = itUsedRes->second._usedSessions;
		}
		envlog(ZQ::common::Log::L_DEBUG,
					SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] adjusted current used bandwidth to [%lld] and used sessions to [%lld]"),
					sessid.c_str(), curUsedBW,curUsedSession);
		//create liveTxn
		envlog(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin,"resolvePurchase() sess[%s] prepare a new live txn with ID [%s]"),sessid.c_str(),sessid.c_str());
		LiveTxnImpl::Ptr pTxn = new LiveTxnImpl(_env,sessid);
		char buf[32];
		//CATEG_Txn
		pTxn->lastState	 = TianShanIce::stInService;
		pTxn->sessId	 = sessid;
		pTxn->ident.name = sessid; pTxn->ident.category = DBFILENAME_Txn;
		pTxn->siteName   = sitename;
		pTxn->mountedPath    = pathname;
		pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(appName), appinfo.name));
		pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(appEndpoint), appinfo.endpoint));
		
		pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(stampCommitted), ""));
		pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(stampStopped), ""));
		pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(allocateCost), ""));
		
		pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(stampResolved), TimeToUTC(now(), buf, sizeof(buf)) ==NULL ? "" : buf));

		sprintf(buf,"%lld",lBandWidth);
		pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(bandwidth), buf));
        
        //Insert client parameters here
        {
            TianShanIce::ValueMap pd = sess->getPrivateData();

            std::string val;
            AUX_Fetch_Variant_String(pd, "ClientRequest#clientSessionId", val);
            pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(clientSessionId), val));
            AUX_Fetch_Variant_String(pd, "ClientRequest#orginalUrl", val);
            pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(orginalUrl), val));
            AUX_Fetch_Variant_String(pd, "ClientRequest#clientAddress", val);
            pTxn->properties.insert(::TianShanIce::Properties::value_type(SYS_PROP(clientAddress), val));
#undef AUX_Fetch_Variant_String
        }
		pTxn->SRMSess	= sess;
		
		envlog(ZQ::common::Log::L_DEBUG, SERVFMT(SiteAdmin,"resolvePurchase() add the new live txn with ID [%s] into DB"),	sessid.c_str());

		_env._eLiveTxn->add(pTxn, pTxn->ident);	
		
		envlog(ZQ::common::Log::L_INFO, 
			SERVFMT(SiteAdmin, "create a new live txn for [sess: %s] site: %s; app: %s ;Bandwidth: %lld ."
			"now CurBW=%lld(KB) CurSess=%lld MaxBW=%lld(KB) MaxSess=%d"),
			sessid.c_str(), sitename.c_str(), appinfo.endpoint.c_str(),lBandWidth,
			curUsedBW,curUsedSession,maxDownstreamBwKbps,maxSessions);				
	}
	catch (TianShanIce::BaseException& ex) 
	{
		envlog(ZQ::common::Log::L_ERROR,
			SERVFMT(SiteAdmin,"TianShan exception caught when create purchase:%s"),
			ex.ice_name().c_str());

		_IceReThrow(TianShanIce::ServerError, ex);
	}
	catch (Ice::Exception& ex) 
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(SiteAdmin, 306, "ResolvePuchase() ice exception [%s] when resolvePurchase"),ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(SiteAdmin, 307, "ResolvePuchase() unexpect error when resolve purchase"));
	}
	return purchasePrx;
}

// record operations start here
// ----------------------------------

#define IMPL_ListRecords(_COLLECTION, _DICT) \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "list " #_COLLECTION));\
	::TianShanIce::Site::##_COLLECTION collection; \
	{ \
		ZQ::common::MutexGuard gd(_env._lock##_DICT); \
		for (TianShanIce::Site::##_DICT::const_iterator it = _env._p##_DICT->begin(); \
		it != _env._p##_DICT->end(); it++) \
			collection.push_back(it->second); \
	} \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "%d " #_COLLECTION " found"), collection.size());\
	return collection;

::TianShanIce::Site::VirtualSites SiteAdminImpl::listSites(const ::Ice::Current& c) const
{
	CONN_TRACE(c, SiteAdmin, listSites);

	IMPL_ListRecords(VirtualSites, SiteDict);
}

::TianShanIce::Site::AppInfos SiteAdminImpl::listApplications(const ::Ice::Current& c) const
{
	CONN_TRACE(c, SiteAdmin, listApplications);

	IMPL_ListRecords(AppInfos, AppDict);
}

#define IMPL_UpdateRecord(_DICT, _KEY, _REC)  { \
	Freeze::TransactionHolder txHolder(_env._conn); \
	{ \
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "update " #_DICT "[%s]"), _KEY.c_str());\
		ZQ::common::MutexGuard gd(_env._lock##_DICT); \
		_env._p##_DICT->put(TianShanIce::Site::##_DICT##::value_type(_KEY, _REC)); \
	} \
	txHolder.commit(); }

#define IMPL_FetchRecord(_DICT,_KEY,_REC)\
{\
::TianShanIce::Site::##_DICT::iterator it = _env._p##_DICT->find (_KEY);\
if (it!=_env._p##_DICT->end ())\
{\
	_REC = it->second;\
}\
}
bool SiteAdminImpl::updateSite(const ::std::string& name, const ::std::string& desc, const ::Ice::Current& c)
{
	CONN_TRACE(c, SiteAdmin, updateSite);

	TianShanIce::Site::VirtualSite site;
	IMPL_FetchRecord(SiteDict,name,site);
	site.name = name;
	site.desc = desc;

	IMPL_UpdateRecord(SiteDict, name, site);
	//must inform that need update the USEDRESMAP
	ZQ::common::MutexGuard gd(_usedResLocker);
	USEDRESMAP::iterator it = _usedResMap.find(name);	
	
	UsedSiteResource usr;

	if( it == _usedResMap.end() )
	{	
		usr._usedBandwidth = 0;
		usr._usedSessions = 0;
		_usedResMap.insert(USEDRESMAP::value_type(name,usr) );
	}
	else
	{
		usr = it->second;
		_usedResMap.erase(it);
		_usedResMap.insert(USEDRESMAP::value_type(name,usr) );
	}
	return true;
}

bool SiteAdminImpl::updateApplication(const ::std::string& name, const ::std::string& endpoint, const ::std::string& desc, const ::Ice::Current& c)
{
	CONN_TRACE(c, SiteAdmin, updateApplication);

	TianShanIce::Site::AppInfo appinfo;
	IMPL_FetchRecord(AppDict,name,appinfo);
	appinfo.name	 = name;
	appinfo.endpoint = endpoint;
	appinfo.desc	 = desc;

	IMPL_UpdateRecord(AppDict, name, appinfo);
	return true;
}

#define IMPL_RemoveRecord(_DICT, _KEY)  { \
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "remove " #_DICT "[%s]"), _KEY.c_str());\
	Freeze::TransactionHolder txHolder(_env._conn); \
	ZQ::common::MutexGuard gd(_env._lock##_DICT); \
	_env._p##_DICT->erase(_KEY); \
	txHolder.commit(); }

#define IMPL_CleanLinkByRecord(_LINK, _IDX, _KEY) { \
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "clean " #_LINK " by " #_IDX "[%s]"), _KEY.c_str());\
		IdentCollection linkIds = _env._idx##_IDX->find(_KEY); \
		for (IdentCollection::iterator it = linkIds.begin(); it < linkIds.end(); it++) \
		{ \
			try	{ \
				::TianShanIce::Site::##_LINK##Prx link = IdentityToObj(_LINK, *it); \
				if (link) link->destroy(); \
			} catch(...) {} \
		} }

bool SiteAdminImpl::removeSite(const ::std::string& name, const ::Ice::Current& c)
{
	CONN_TRACE(c, SiteAdmin, removeSite);

	IMPL_RemoveRecord(SiteDict, name);
	IMPL_CleanLinkByRecord(AppMount, SiteToMount, name);

	return true;
}

bool SiteAdminImpl::removeApplication(const ::std::string& name, const ::Ice::Current& c)
{
	CONN_TRACE(c, SiteAdmin, removeApplication);

	IMPL_RemoveRecord(AppDict, name);
	IMPL_CleanLinkByRecord(AppMount, AppToMount, name);

	return true;
}

::TianShanIce::Properties SiteAdminImpl::getSiteProperties(const ::std::string& siteName, const ::Ice::Current& c) const
{
	::TianShanIce::Properties props;

	try 
	{
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		TianShanIce::Site::SiteDict::iterator it = _env._pSiteDict->findByName(siteName);
		if (_env._pSiteDict->end() == it)
			::ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog, EXPFMT(SiteAdmin, 401, "getSiteProperties(%s) site not found"), siteName.c_str());

		props = it->second.properties;

		char buf[32];
		sprintf(buf, "%lld", it->second.maxDownstreamBwKbps);
		props.insert(::TianShanIce::Properties::value_type(SYS_PROP(maxDownstreamBwKbps), buf));
		sprintf(buf, "%d", it->second.maxSessions);
		props.insert(::TianShanIce::Properties::value_type(SYS_PROP(maxSessions), buf));
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 402, "getSiteProperties(%s) properties not found, exception[%s]"), siteName.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 402, "getSiteProperties(%s) properties not found, unkown exception"), siteName.c_str());
	}

	return props;
}

void SiteAdminImpl::updateSiteResourceLimited(const ::std::string& siteName, ::Ice::Long maxBw, ::Ice::Int maxSessions, const ::Ice::Current& /* = ::Ice::Current( */)
{
	TianShanIce::Site::VirtualSite record;
	try 
	{
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		TianShanIce::Site::SiteDict::iterator it = _env._pSiteDict->findByName(siteName);
		if (_env._pSiteDict->end() == it)
		{
			envlog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(SiteAdmin, "updateSiteResourceLimited(%s) site not found"), 
				siteName.c_str());
			return ;
		}
		
		record = it->second;
	}
	catch(...) {}
	
	record.maxDownstreamBwKbps	= maxBw;
	record.maxSessions			= maxSessions;
	
	Freeze::TransactionHolder txHolder(_env._conn);
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "updateSiteResourceLimited(%s) adn MaxBW[%lld] MaxSession[%d]"), 
														siteName.c_str(),maxBw,maxSessions);
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		_env._pSiteDict->put(TianShanIce::Site::SiteDict::value_type(siteName, record));
	}
	txHolder.commit();
	//I must change the site properties
	//::std::string val = siteProps[SYS_PROP(maxDownstreamBwKbps)];
	//::std::string val = siteProps[SYS_PROP(maxSessions)];
	::Ice::Current c;
	:: TianShanIce :: Properties siteProps= getSiteProperties(siteName,c);

	char szBuf[256];
	ZeroMemory(szBuf,sizeof(szBuf));
	sprintf(szBuf,"%lld",maxBw);
	siteProps[SYS_PROP(maxDownstreamBwKbps)] = szBuf;

	ZeroMemory(szBuf,sizeof(szBuf));
	sprintf(szBuf,"%d",maxSessions);
	siteProps[SYS_PROP(maxSessions)] =szBuf;
	
	setSiteProperties(siteName,siteProps,c);
	
}

bool SiteAdminImpl::setSiteProperties(const ::std::string& siteName, const ::TianShanIce::Properties& props, const ::Ice::Current& c)
{
	TianShanIce::Site::VirtualSite record;

	try 
	{
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		TianShanIce::Site::SiteDict::iterator it = _env._pSiteDict->findByName(siteName);
		if (_env._pSiteDict->end() == it)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdmin, "setSiteProperties(%s) site not found"), siteName.c_str());
			return false;
		}
		
		record = it->second;
	}
	catch(...) {}

	::TianShanIce::Properties::const_iterator itNewProp = props.begin();
	for ( ;itNewProp != props.end() ; itNewProp++ )
	{
		record.properties[itNewProp->first] = itNewProp->second;			
	}	

	Freeze::TransactionHolder txHolder(_env._conn);
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "setSiteProperties(%s) update properties"), siteName.c_str());
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		_env._pSiteDict->put(TianShanIce::Site::SiteDict::value_type(siteName, record));
	}
	txHolder.commit();

	return true;
}

::TianShanIce::Site::AppMounts SiteAdminImpl::listMounts(const ::std::string& siteName, const ::Ice::Current& c) const
{
	::TianShanIce::Site::AppMounts links;
	IdentCollection identities;
	try {
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "listMounts(%s) search for mounts by site"), siteName.c_str());
		ZQ::common::MutexGuard gd(_env._lockAppMount);
		identities = _env._idxSiteToMount->find(siteName);
	} catch(...) {}
	
	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++)
	{
		::TianShanIce::Site::AppMountPrx linkprx = (IdentityToObj(AppMount, *it));
		if (linkprx)
			links.push_back(linkprx);
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "listMounts(%s) %d application mounts found"), siteName.c_str(), links.size());
	return links;
}

::TianShanIce::Site::AppMountPrx SiteAdminImpl::mountApplication(const ::std::string& siteName, const ::std::string& mountPath, const ::std::string& appName, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "mountApplication(%s) \"%s\" => app[%s]"), siteName.c_str(), mountPath.c_str(), appName.c_str());
	AppMountImpl::Ptr link = new AppMountImpl(_env);
	link->ident.name = IceUtil::generateUUID(); link->ident.category = "AppMount";
	link->siteName = siteName; link->mountedPath = mountPath; link->appName = appName;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "check if the site and application information exists"));
	// check if the site and application information has been configed
	{
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
	if (_env._pSiteDict->end() == _env._pSiteDict->findByName(siteName))
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError>(envlog, EXPFMT(SiteAdmin, 501, "No such site \"%s\""), siteName.c_str());

	if (_env._pAppDict->end() == _env._pAppDict->findByName(appName))
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 502, "No such application \"%s\""), appName.c_str());

	}
    // be sure that no same mount exists
	::TianShanIce::Site::AppMountPrx mount;

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "clean up the mount with same path but different application, or address the existing the matched mount"));
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
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "add a new mount"));
		// add the new mountage
		_env._eAppMount->add(link, link->ident);
		mount = IdentityToObj(AppMount, link->ident);
	}

    return mount;
}

bool SiteAdminImpl::unmountApplication(const ::std::string& siteName, const ::std::string& mountedPath, const ::Ice::Current& c)
{
	IdentCollection ids = _env._idxSiteToMount->find(siteName);
	
	int count =0;
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "unmountApplication() address the mount: [%s/%s] to unmount"), siteName.c_str(), mountedPath.c_str());
	for (IdentCollection::iterator it = ids.begin(); it < ids.end(); it++)
	{
		try	{
			::TianShanIce::Site::AppMountPrx mount = IdentityToObj(AppMount, *it);
			if (mount->getMountedPath() == mountedPath)
			{
				mount->destroy();
				count++;
			}
		} catch(...) {}
	}

	return (count>0);
}

::TianShanIce::Site::AppInfo SiteAdminImpl::findApplication(const ::std::string& siteName, const ::std::string& mountedPath, const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "findApplication() address the mount: [%s/%s]"), siteName.c_str(), mountedPath.c_str());

	IdentCollection ids ;
	try
	{
		ids = _env._idxSiteToMount->find(siteName);
	}
	catch (Ice::Exception& ex)
	{
		ids.clear();
		envlog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(SiteAdmin,"catch ice exception when find application from index:%s"),ex.ice_name().c_str());
	}
	catch (...)
	{
		ids.clear();
		envlog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(SiteAdmin, "unexpect error when find application from index"));
	}
	IdentCollection::iterator it = ids.begin();
	::TianShanIce::Site::AppMountPrx mount;
	for (it = ids.begin(); it < ids.end(); it++)
	{
		try	
		{
			mount = IdentityToObj(AppMount, *it);
			if (mount->getMountedPath() == mountedPath)
				break;
		}
		catch (::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(SiteAdmin,"catch ice exception when find application:%s"),ex.ice_name().c_str());
		}
		catch(...) 
		{
			envlog(ZQ::common::Log::L_DEBUG,
				CLOGFMT(SiteAdmin, "unexpect error when find application"));
		}
	}

	if (ids.end() ==it)
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 601, "findApplication() failed to address the mount: [%s/%s]"), siteName.c_str(), mountedPath.c_str());

	mount = IdentityToObj(AppMount, *it);

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "opening application info: [%s]"), mount->getAppName().c_str());
	ZQ::common::MutexGuard gd(_env._lockAppDict);
	::TianShanIce::Site::AppDict::iterator itDict =  _env._pAppDict->findByName(mount->getAppName());

	if (_env._pAppDict->end() == itDict)
		::ZQTianShan::_IceThrow <::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 602, "findApplication() failed to find application info: [%s]"), mount->getAppName());

	return (itDict->second);
}



void SiteAdminImpl::commitStateChange(const ::std::string& sessid, ::TianShanIce::State state,
									  const ::TianShanIce::SRM::SessionPrx& sess,
									  const ::TianShanIce::Properties& props, const ::Ice::Current& c)
{
	Ice::Identity ident;
	//CATEG_Txn
	ident.name = sessid; ident.category = DBFILENAME_Txn;
	
	::TianShanIce::Site::LiveTxnPrx livetxn ;
	try
	{
		livetxn = IdentityToObj(LiveTxn,ident);
	}
	catch(...)
	{
		livetxn = NULL;
	}
	
	if (!livetxn)
		::ZQTianShan::_IceThrow <::TianShanIce::Site::NoSuchTxn> (envlog, EXPFMT(SiteAdmin, 701, "no such txn %s in the live txn database"), sessid.c_str());
	try 
	{	
		livetxn->setState(state);
		for (::TianShanIce::Properties::const_iterator it = props.begin(); it != props.end(); it++)
		{
			std::string key = it->first;
			if (key.empty())
				continue;
				
			if (0!=key.compare(0, strlen(SYS_PROP_PREFIX), SYS_PROP_PREFIX))
				key = ::std::string(SYS_PROP_PREFIX) + key;
			livetxn->setProperty(key, it->second);
		}
        { // trace the state changed time
            char tmBuf[64] = {0};
            const char* nowUTC = ZQTianShan::TimeToUTC(ZQTianShan::now(), tmBuf, sizeof(tmBuf));
            if(nowUTC)
            {
                switch(state)
                {
                case TianShanIce::stProvisioned:
                    livetxn->setProperty(SYS_PROP(ProvisionedAt), nowUTC);
                    break;
                case TianShanIce::stInService:
                    livetxn->setProperty(SYS_PROP(InServiceAt), nowUTC);
                    break;
                case TianShanIce::stOutOfService:
                    livetxn->setProperty(SYS_PROP(OutOfServiceAt), nowUTC);
                    break;
                }
            }
        }
		if ( state ==  TianShanIce::stInService ) 
		{
#ifdef _WITH_EVENTSENDER_
			//post event to eventSender
			envlog(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin,"LiveTxn is in stInService state,post event to eventSender"));
			MSGSTRUCT msg;
			TianShanIce::Properties prop=props;
			msg.category = "Session";
			msg.eventName= "SessionInService";
			msg.id = 3001;
			char buf[256];
			memset(buf,0,sizeof(buf));
			msg.timestamp = SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);
			memset(buf,0,sizeof(buf));
			msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";

			msg.property["sessionId"]			= sessid;
			msg.property["contentStoreNetId"]	= prop[SYS_PROP(contentStore)];
			msg.property["streamerNetId"]		= prop[SYS_PROP(Streamer)];
			msg.property["serviceGroupId"]		= prop[SYS_PROP(serviceGroupID)];
			msg.property["downstreamBandwidth"]	= prop[SYS_PROP(bandwidth)];
			msg.property["streamId"]			= prop[SYS_PROP(streamID)];
			//msg.property["stampLocal"]			= ZQTianShan::Site::FormatLocalTime(buf,sizeof(buf));
			g_pEventSinkMan->PostEvent(msg);
#endif//_WITH_EVENTSENDER_
		}
		else if ( state == TianShanIce::stOutOfService ) 
		{
            // monitor the session
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin,"txn[%s] is at state OutOfService. Watch it in 1 hr."), sessid.c_str());
            _env._txnWatchDog->WatchMe(sessid, 3600 * 1000);

#ifdef _WITH_EVENTSENDER_
			//post event to eventSender
			envlog(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin,"LiveTxn is in stInService state,post event to eventSender"));
			TianShanIce::Properties prop=props;
			
			MSGSTRUCT msg;			
			msg.category = "Session";
			msg.eventName= "SessionOutOfService";			
			char buf[256];
			memset(buf,0,sizeof(buf));

			msg.sourceNetId = gethostname(buf,sizeof(buf)-1)==0?buf:"";

			msg.id = 4001;
			memset(buf,0,sizeof(buf));
			msg.timestamp = SystemTimeToUTC(ZQTianShan::now(),buf,sizeof(buf)-1);
			msg.property["sessionId"]			= sessid;
			msg.property["teardownReason"]		= prop[SYS_PROP(teardownReason)];
			msg.property["terminateReason"]		= prop[SYS_PROP(terminateReason)];			
			//msg.property["stampLocal"]			= FormatLocalTime(buf,sizeof(buf));

			g_pEventSinkMan->PostEvent(msg);

#endif //_WITH_EVENTSENDER_

			envlog(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin,"LiveTxn is in stOutOfService State,PostYTD"),sessid.c_str());
			::Ice::Current c;
			postYTD(sessid,c);
		}
	}
	catch(const ::TianShanIce::ServerError& ex)
	{
		ex.ice_throw();
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow <::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 702, "commitStateChange(id=%s) caught exception:%s"), sessid.c_str(), ex.ice_name().c_str());
	}
}

void SiteAdminImpl::listLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_listLiveTxnPtr& amdCB, const ::std::string& siteName, const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames, const ::std::string& startId, int maxCount, const ::Ice::Current& c) const
{
	try {
		(new ListTxnCommand(amdCB, _env, siteName, appMount, paramNames, startId, maxCount))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdminImpl,"listLiveTxn_async() failed to list Txn: site[%s] mount[%s]"),
			siteName.c_str(), appMount.c_str());
		amdCB->ice_exception(::TianShanIce::ServerError("SiteAdmin", 500, "failed to generate ListTxnCommand"));
	}
}


void SiteAdminImpl::trace_async(const ::TianShanIce::Site::AMD_TxnService_tracePtr& amdCB, const ::std::string& sessId, const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg, const ::Ice::Current& c)
{
	try {
		(new SaveEventCommand(amdCB, _env, sessId, category, eventCode, eventMsg))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdminImpl,"trace_async() failed to record event session[%s] %s:%s %s"),
			sessId.c_str(), category.c_str(), eventCode.c_str(), eventMsg.c_str());
		amdCB->ice_exception(::TianShanIce::ServerError("SiteAdmin", 500, "failed to generate SaveEventCommand"));
	}
}

void SiteAdminImpl::postYTD(const ::std::string& sessid, const ::Ice::Current& c)
{
	Ice::Identity ident;
	ident.name = sessid; ident.category = DBFILENAME_Txn;
	::TianShanIce::Site::LiveTxnPrx livetxn = IdentityToObj(LiveTxn, ident);
	
	if (!livetxn)
		::ZQTianShan::_IceThrow <::TianShanIce::Site::NoSuchTxn> (envlog, EXPFMT(SiteAdmin, 801, "no such txn %s in the live txn database"), sessid.c_str());
		
	if (gSiteAdminConfig.lTxnDataEnabled)
	{
		try
		{
			char sqlBuff[1024];
			sqlBuff[sizeof(sqlBuff) - 1] = '\0';
			// insert the base info
			_snprintf(sqlBuff, sizeof(sqlBuff) - 1, "INSERT INTO Sessions(Session, Site, Path) VALUES(\'%s\', \'%s\', \'%s\')", 
				livetxn->getSessId().c_str(), livetxn->getSitename().c_str(), livetxn->getPath().c_str());
			_env._mdbLog.executeSql(sqlBuff);

			// update the property fields
			{
				std::ostringstream cmdBuf;
				TianShanIce::Properties txnProps = livetxn->getProperties();
				cmdBuf	<< "UPDATE Sessions SET "
						<< "Storage='" << txnProps[SYS_PROP(contentStore)]
						<< "',Streamer='" << txnProps[SYS_PROP(Streamer)]
						<< "',ServiceGroup='" << txnProps[SYS_PROP(serviceGroupID)]
						<< "',Bandwidth='" << txnProps[SYS_PROP(bandwidth)]
						<< "',Stream='" << txnProps[SYS_PROP(streamID)]
						<< "',ProvisionedAt='" << txnProps[SYS_PROP(ProvisionedAt)]
						<< "',InServiceAt='" << txnProps[SYS_PROP(InServiceAt)]
						<< "',OutOfServiceAt='" << txnProps[SYS_PROP(OutOfServiceAt)]
						<< "',TeardownReason='" << txnProps[SYS_PROP(teardownReason)]
						<< "',TerminateReason='" << txnProps[SYS_PROP(terminateReason)]
						<< "',ClientSessionId='" << txnProps[SYS_PROP(clientSessionId)]
						<< "',ClientAddress='" << txnProps[SYS_PROP(clientAddress)]
						<< "',OrginalUrl='" << txnProps[SYS_PROP(orginalUrl)]
						<< "' WHERE Session='" << livetxn->getSessId() << "'";

				std::string sqlCmd = cmdBuf.str();
				_env._mdbLog.executeSql(sqlCmd.c_str());
			}

			std::vector<::Ice::Identity> eventTxns;
			eventTxns = _env._idxTxnToEvent->find(ident);
			int evt_cur, evt_count;
			for (evt_cur = 0, evt_count = eventTxns.size(); evt_cur < evt_count; evt_cur ++)
			{
				::TianShanIce::Site::TxnEventPrx evt = IdentityToObjEnv(_env, TxnEvent, eventTxns[evt_cur]);
				TianShanIce::Properties props;
				TianShanIce::StrValues params;
				params.push_back("stampUTC");
				params.push_back("category");
				params.push_back("eventCode");
				params.push_back("eventMsg");
				props = evt->getEventInfo(params);
				_snprintf(sqlBuff, sizeof(sqlBuff) - 1, "insert into Events(Session, stampUTC, category, eventCode, eventMsg) values(\'%s\', \'%s\', \'%s\', \'%s\', \'%s\')", 
					ident.name.c_str(), props["stampUTC"].c_str(), props["category"].c_str(), props["eventCode"].c_str(), props["eventMsg"].c_str());
				_env._mdbLog.executeSql(sqlBuff);
			}
		}
		catch (const ZQ::common::MdbLogError& ex)
		{
			envlog(ZQ::common::Log::L_WARNING,SERVFMT(SiteAdmin,"Write Txn data to MdbLog caught %s"), ex.getString());
		}
		catch (const Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_WARNING,SERVFMT(SiteAdmin,"Write Txn data to MdbLog caught %s"), ex.ice_name().c_str());
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_WARNING,SERVFMT(SiteAdmin,"Write Txn data to MdbLog caught unexpect error"));
		}
	}

	try 
	{
		Ice::Long lBandwidth=0;
		do {	
			ZQ::common::MutexGuard gd(_env._lockSiteDict);
			envlog( ZQ::common::Log::L_INFO, SERVFMT(SiteAdmin,"PostYTD with LiveTxn sessID=%s"), sessid.c_str());
			std::string siteName= livetxn->getSitename();
			::TianShanIce::Site::SiteDict::iterator it=_env._pSiteDict->findByName(siteName);
			if (_env._pSiteDict->end() == it)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdmin, "Can't find the site with sitename=%s, maybe something wrong in the configuration"), siteName.c_str());
				break;
			}

			// read the bandwidth
			const ::TianShanIce::Properties props=livetxn->getProperties();
			TianShanIce::Properties::const_iterator itTxnOutOfService = props.find(SYS_PROP(LiveTxnOutOfService));
			if ( props.end() == itTxnOutOfService && itTxnOutOfService->second== "yes" ) 
			{
				envlog(ZQ::common::Log::L_INFO, SERVFMT(SiteAdmin, "PostYTD() current liveTxn[%s] has already been out of service"), sessid.c_str());
				break;
			}

			TianShanIce::Properties::const_iterator itBandWidth =props.find(SYS_PROP(bandwidth));

			if (props.end() != itBandWidth)
			{
				std::string strBandwidth = itBandWidth->second; //props[SYS_PROP(bandwidth)];
				sscanf(strBandwidth.c_str() , "%lld", &lBandwidth);
			}
			envlog(ZQ::common::Log::L_DEBUG,SERVFMT(SiteAdmin, "PostYTD() liveTxn[%s] got bandwidth[%lld]"), sessid.c_str(), lBandwidth);

			{
				ZQ::common::MutexGuard gdUsed(_usedResLocker);

				//check again
				TianShanIce::Properties::const_iterator itTxnOutOfService = props.find(SYS_PROP(LiveTxnOutOfService));
				if ( itTxnOutOfService!=props.end() && itTxnOutOfService->second== "yes" ) 
				{
					envlog(ZQ::common::Log::L_INFO,SERVFMT(SiteAdmin,"PostYTD() current liveTxn has already been destroyed: %s"), sessid.c_str());
					break;
				}

				USEDRESMAP::iterator itUsed = _usedResMap.find( siteName);
				if (_usedResMap.end() != itUsed)
				{
					Ice::Long lTempBandwidth=(lBandwidth/1000) + ((lBandwidth %1000) >0 ?1:0);
					itUsed->second._usedBandwidth -= lTempBandwidth;
					itUsed->second._usedSessions--;
				}

				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin,"PostYTD() current usedBandwidth(%lldKBPS), currentSessions(%d), totalBandWidth(%lldKBPS) totalSession(%d)"),
					itUsed->second._usedBandwidth , itUsed->second._usedSessions, it->second.maxDownstreamBwKbps , it->second.maxSessions );

				livetxn->setProperty(SYS_PROP(LiveTxnOutOfService), "yes");
			}
		} while (0);

		livetxn->destroy();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdmin, "PostYTD() txn[%s] destroy() called returned %lldbps"), sessid.c_str(), lBandwidth);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		ex.ice_throw();
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow <::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 802, "postYTD(id=%s) caught exception:%s"), sessid.c_str(), ex.ice_name().c_str());
	}
}

void SiteAdminImpl::setUserProperty(const ::std::string& sessId, const ::std::string& key, const ::std::string& value, const ::Ice::Current& c)
{
	if (key.empty())
		return;
		
	Ice::Identity ident;
	ident.name = sessId; ident.category = DBFILENAME_Txn;
	::TianShanIce::Site::LiveTxnPrx livetxn = IdentityToObj(LiveTxn, ident);
	
	if (!livetxn)
		::ZQTianShan::_IceThrow <::TianShanIce::Site::NoSuchTxn> (envlog, EXPFMT(SiteAdmin, 901, "no such txn %s in the live txn database"), sessId.c_str());
		
	try {
		livetxn->setProperty((0!=key.compare(0, strlen(USER_PROP_PREFIX), USER_PROP_PREFIX)) ?(std::string(USER_PROP_PREFIX) + key) : key, value);
	}
	catch(const ::TianShanIce::ServerError& ex)
	{
		ex.ice_throw();
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow <::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 902, "setUserProperty(id=%s) caught exception:%s"), sessId.c_str(), ex.ice_name().c_str());
	}
}

void SiteAdminImpl::dumpLiveTxn_async(const ::TianShanIce::Site::AMD_TxnService_dumpLiveTxnPtr& amdCB, const ::std::string& sessId, const ::std::string& beginFormat, const ::std::string& traceFormat, const ::std::string& endFormat, const ::Ice::Current& c)
{
	Ice::Identity ident;
	ident.name = sessId; ident.category = DBFILENAME_Txn;
	::TianShanIce::Site::LiveTxnPrx livetxn = IdentityToObj(LiveTxn, ident);
	
	if (!livetxn)
		::ZQTianShan::_IceThrow <::TianShanIce::Site::NoSuchTxn> (envlog, EXPFMT(SiteAdmin, 1001, "no such txn %s in the live txn database"), sessId.c_str());
		
	::std::string txnstr;
#pragma message ( __MSGLOC__ "TODO: impl here")
//	return txnstr;
}

void SiteAdminImpl::restrictSiteResources(const ::std::string& siteName, const ::TianShanIce::SRM::ResourceMap& resources, const ::Ice::Current& c)
{
	TianShanIce::Site::VirtualSite record;
	
	try 
	{
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		TianShanIce::Site::SiteDict::iterator it = _env._pSiteDict->findByName(siteName);
		if (_env._pSiteDict->end() == it)
			::ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (_env._log, EXPFMT(SiteAdmin, 1001, "restrictSiteResources() failed to find site[%s]"), siteName.c_str());
		
		record = it->second;
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_env._log, EXPFMT(SiteAdmin, 1002, "restrictSiteResources() failed to find site[%s], exception [%s]"), siteName.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_env._log, EXPFMT(SiteAdmin, 1002, "restrictSiteResources() failed to find site[%s], unkown exception"), siteName.c_str());
	}

	static const ::TianShanIce::SRM::ResourceType acceptableTypes[] = {
		::TianShanIce::SRM::rtStorage,
		::TianShanIce::SRM::rtStreamer,
		::TianShanIce::SRM::rtServiceGroup };

	for (int i =0; i< sizeof(acceptableTypes) / sizeof(::TianShanIce::SRM::ResourceType); i++)
	{
		const ::TianShanIce::SRM::ResourceType rt = acceptableTypes[i];
		::TianShanIce::SRM::ResourceMap::const_iterator it = resources.find(rt);
		if (resources.end() != it)
		{
			record.restrictedResources[rt] = it->second;
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "restrictSiteResources(%s) update resource[%s]"), siteName.c_str(), ZQTianShan::ResourceTypeStr(rt));
		}
	}
	
	try 
	{
		Freeze::TransactionHolder txHolder(_env._conn);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdmin, "restrictSiteResources(%s) update properties"), siteName.c_str());
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		_env._pSiteDict->put(TianShanIce::Site::SiteDict::value_type(siteName, record));
		txHolder.commit();
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_env._log, EXPFMT(SiteAdmin, 1003, "restrictSiteResources() failed to submit changes of site[%s], exception [%s]"), siteName.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_env._log, EXPFMT(SiteAdmin, 1003, "restrictSiteResources() failed to submit changes of site[%s], unkown exception"), siteName.c_str());
	}
}

::TianShanIce::SRM::ResourceMap SiteAdminImpl::getSiteResourceRestricutions(const ::std::string& siteName, const ::Ice::Current& c) const
{
	try 
	{
		ZQ::common::MutexGuard gd(_env._lockSiteDict);
		TianShanIce::Site::SiteDict::iterator it = _env._pSiteDict->findByName(siteName);
		if (_env._pSiteDict->end() == it)
			::ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog, EXPFMT(SiteAdmin, 1101, "getSiteResourceRestricutions(%s) site not found"), siteName.c_str());
		else
			return it->second.restrictedResources;
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog, EXPFMT(SiteAdmin, 1102, "getSiteResourceRestricutions(%s) caught exception[%s]"), siteName.c_str(), ex.ice_name().c_str());
	}

	return ::TianShanIce::SRM::ResourceMap(); // dummy return to avoid complie warning
}

/*
void SiteAdminImpl::dumpHistoryTxn_async(const ::TianShanIce::Site::AMD_TxnService_dumpHistoryTxnPtr& amdCB, const ::std::string& sessId, const ::std::string& beginFormat, const ::std::string& traceFormat, const ::std::string& endFormat, const ::Ice::Current& c)
{
	::std::string txnstr;
#pragma message ( __MSGLOC__ "TODO: impl here")
//	return txnstr;
}
*/


// -----------------------------
// class AppMountImpl
// -----------------------------
AppMountImpl::AppMountImpl(SiteAdminSvcEnv& env)
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
		::ZQTianShan::_IceThrow <::TianShanIce::ServerError> (envlog, EXPFMT(AppMount, 1101, "destroy(id=%s) caught DB exception:"), c.id.name.c_str(), ex.ice_name().c_str());
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

// -----------------------------
// class LiveTxnImpl
// -----------------------------
LiveTxnImpl::LiveTxnImpl(SiteAdminSvcEnv& env)
:_env(env), _isAlive(true), _lastTimeout(0)
{
	//updateTimer(5000);
}
LiveTxnImpl::LiveTxnImpl(SiteAdminSvcEnv& env,const std::string& id)
:_env(env), _isAlive(true)
{
	lastState = TianShanIce::stInService;
	sessId =id;
	updateTimer( 1*3600*1000 + rand()%15*60*1000 );
}

void LiveTxnImpl::updateTimer(Ice::Long lMilliSec)
{
	_env._txnWatchDog->WatchMe(sessId,lMilliSec);
}

void LiveTxnImpl::onTimer( const ::Ice::Current& c)
{
	bool bNeedDestroy =false;

	//check if session is alive
	try
	{
		//WLock sync(*this);
		//envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(LiveTxn,"OnTimer() Txn:%s check avaiable"),sessId.c_str());
		SRMSess->ice_ping();

#pragma message(__MSGLOC__"TODO:timer的时间是否需要配置")
		updateTimer( 1*3600*1000 + rand()%( 15*60*1000 ) );
		_lastTimeout = 0;
	}
	catch (Ice::ObjectNotExistException&)
	{		
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(LiveTxn,"txn[%s] SRM session not exist, post LiveTxn to YTD database"),sessId.c_str());
		bNeedDestroy =true;
	}
	catch(...)
	{
		::Ice::Long stampNow = ZQTianShan::now();
		if (_lastTimeout <=0)
			_lastTimeout = stampNow;

		if (stampNow - _lastTimeout <6*3600*1000)
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(LiveTxn,"txn[%s] failed to ping SRM session, will retry in 1 hr"), sessId.c_str());
			updateTimer(3600* 1000);
		}
		else
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(LiveTxn,"txn[%s] failed to keep touch with SRM session in 6 hr, cleanup"), sessId.c_str());
			bNeedDestroy = true;
		}
	}

	if (!bNeedDestroy)
    {
        if(TianShanIce::stOutOfService == lastState)
        {
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LiveTxn,"txn[%s] is at state OutOfService. Watch it in 1 hr."), sessId.c_str());
            updateTimer(3600 * 1000);
        }
		return;
    }

	std::string LivTxnSrvEndPoint = std::string(SERVICE_NAME_BusinessRouter":") + _env._endpoint;
	try
	{
		TianShanIce::Site::TxnServicePrx txnSrv = TianShanIce::Site::TxnServicePrx::checkedCast(_env._communicator->stringToProxy(LivTxnSrvEndPoint) );
		if(!txnSrv)
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(LiveTxn,"failed to connect to TxnService"));
		else
			txnSrv->postYTD(sessId);
	}
	catch (...) {}
}

void LiveTxnImpl::destroy(const ::Ice::Current& c)
{
	WLock sync(*this);
	try
	{
#pragma message ( __MSGLOC__ "TODO: flush the txn data into the YTD log file")
		

		//Change the flag and move the liveTxn into YTD in another thread
		_isAlive = false;
		_env._liveTxnTransfer->AddSess(c.id.name);
		_env._txnWatchDog->UnWatchMe(c.id.name);
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AppMount, "destroy(id=%s) object already gone, ignore"), c.id.name.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow <::TianShanIce::ServerError> (envlog, EXPFMT(AppMount, 1201, "destroy(id=%s) caught DB exception:"), c.id.name.c_str(), ex.ice_name().c_str());
	}
}

void LiveTxnImpl::setProperty(const ::std::string& key, const ::std::string& value, const ::Ice::Current& c)
{
	WLock sync(*this);
	properties.insert(::TianShanIce::Properties::value_type(key, value));
}

::TianShanIce::State LiveTxnImpl::getState(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return lastState;
}

void LiveTxnImpl::setState(::TianShanIce::State state, const ::Ice::Current& c)
{
	WLock sync(*this);
	lastState = state;
}

::TianShanIce::Properties LiveTxnImpl::getProperties(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return properties;
}

::std::string LiveTxnImpl::getSessId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return sessId;
}

::std::string LiveTxnImpl::getPath(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return mountedPath;
}

std::string LiveTxnImpl::getSitename(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return siteName;
}

// -----------------------------
// class TxnEventImpl
// -----------------------------
TxnEventImpl::TxnEventImpl(SiteAdminSvcEnv& env)
:_env(env)
{
}

void TxnEventImpl::get(::Ice::Identity& identTxn, ::std::string& stampUTC, ::std::string& category, ::std::string& eventCode, ::std::string& eventMsg, const ::Ice::Current& c) const
{
	RLock sync(*this);
	identTxn = this->identTxn;
	stampUTC = this->stampUTC;
	category = this->category;
	eventCode = this->eventCode;
	eventMsg = this->eventMsg;
}

::TianShanIce::Properties TxnEventImpl::getEventInfo(const ::TianShanIce::StrValues& params, const ::Ice::Current& c) const
{
	::TianShanIce::Properties ret;
	int i, count;
	for (i = 0, count = params.size(); i < count; i ++)
	{
		if (strcmp(params[i].c_str(), "stampUTC") == 0)
		{
			ret["stampUTC"] = stampUTC;
		}
		else if (strcmp(params[i].c_str(), "category") == 0)
		{
			ret["category"] = category;
		}
		else if (strcmp(params[i].c_str(), "eventCode") == 0)
		{
			ret["eventCode"] = eventCode;
		}
		else if (strcmp(params[i].c_str(), "eventMsg") == 0)
		{
			ret["eventMsg"] = eventMsg;
		}
	}

	return ret;
}
	

}} // namespace
