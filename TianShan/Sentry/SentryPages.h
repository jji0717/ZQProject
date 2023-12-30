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
// Ident : $Id: SentryPages.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/SentryPages.h $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 15    09-05-18 17:07 Xiaohui.chai
// new implementment
// 
// 14    08-07-14 19:33 Xiaohui.chai
// Added proxy page.
// 
// 13    08-06-10 11:32 Xiaohui.chai
// supported parameter all for NavPage
// 
// 12    08-02-19 15:53 Xiaohui.chai
// Show the active service name instead of interface name in the left
// panel.
// 
// 11    08-01-21 16:34 Xiaohui.chai
// 
// 10    08-01-14 16:04 Xiaohui.chai
// interface changed:
// rename IHttpResponser to IHttpResponse
// rename IHttpRequestCtx::Responser to IHttpRequestCtx::Response
// add GetMethodType() to IHttpRequestCtx
// 
// 9     07-12-21 14:36 Xiaohui.chai
// move transport map page to AdminCtrl_web.dll
// 
// 8     07-11-23 14:37 Xiaohui.chai
// added site pages
// 
// 7     07-11-07 14:47 Xiaohui.chai
// 
// 6     07-11-05 16:05 Xiaohui.chai
// add neighbors page, neighbors map page, local interfaces page
// 
// 5     07-10-19 18:18 Xiaohui.chai
// 
// 4     07-10-16 15:25 Xiaohui.chai
// 
// 3     07-06-07 12:31 Hui.shao
// refresh service information
// 
// 2     07-06-04 14:46 Hui.shao
// separated html pages from env
// ===========================================================================

#ifndef __ZQTianShan_SentryPages_H__
#define __ZQTianShan_SentryPages_H__

#include "../common/TianShanDefines.h"
#include "SentryHttpImpl.h"

class HttpRequestCtx;

namespace ZQTianShan {
namespace Sentry {

// -----------------------------
// class SentryPages
// -----------------------------
///
class SentryPages
{
protected:

	friend class SentryEnv;

	/// constructor
	///@note no direct instantiation of SentryCommand is allowed
    SentryPages(SentryEnv& env, const char* htmldir=NULL);
    const char* setHomeDir(const char* htmldir);

public:
	
	virtual ~SentryPages() {}

    const char* getHomeDir() const { return _htmldir.c_str(); };

    void pushHeader(IHttpResponse& out, const char* templatename);
    void pushFooter(IHttpResponse& out, const char* templatename = NULL);
    void SystemPage(HttpRequestCtx *);
    void prepareLayoutInfo();

	typedef struct _NavNode
	{
//		enum { NNE_ROOT, NNE_LOCALSERVER, NNE_NEIGHBOR, NNE_SITE,
//			 NNE_VSITE, NNE_SITEMAP, NNE_TRANSMAP, NNE_SITESVC, NNE_OTHER } type;
		std::string name, displayname, href;
		std::vector < struct _NavNode > children;
    } NavNode;

    NavNode getLocalActiveEntries();
private:
    void navPage(HttpRequestCtx *);

    // local pages
    void localHostPage(HttpRequestCtx *);
    void localServicesPage(HttpRequestCtx *);
    void localProcessesPage(HttpRequestCtx *);
    void localInterfacesPage(HttpRequestCtx *);

    // neighbors pages
    void neighborsPage(HttpRequestCtx *);
    void neighborsMapPage(HttpRequestCtx *);
    void globalServicePage(HttpRequestCtx *);

    // web proxy
    void proxyPage(HttpRequestCtx *); // implemented in WebProxy.cpp
private:
    typedef void (SentryPages::*PageFunc)(HttpRequestCtx *);
    struct Page
    {
        const char* name;
        PageFunc func;
    };
    static Page _pageTbl[];
protected:

	SentryEnv&     _env;
	std::string    _htmldir;

	NavNode			_navLocal;
	NavNode			_navNeighbor;
	NavNode			_navSite;
};

}} // namespace

#endif // __ZQTianShan_SentryPages_H__

