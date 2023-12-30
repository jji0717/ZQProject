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
// Ident : $Id: admin.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/admin/admin.h $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 10    07-04-19 17:50 Hongquan.zhang
// ===========================================================================

#ifndef __admin_H__
#define __admin_H__

#include "../../common/TianShanDefines.h"
#include "../../common/EventChannel.h"

#include "../WeiwooAdmin.h"
#include "AdminConsole.h"
#include "../../SiteAdminSvc/SiteAdminSvc.h"


class WeiwooAdminConsole : public AdminConsole
{
	/*friend class DummyApp;*/
	friend class SessionEventSinkI;

public:
    WeiwooAdminConsole(const char* endpointBind = "tcp -p 11000");

public:

    void usage();
    void connect(const Args& args);
    void subscribe(const Args& args);

    // about serive groups
	void listSites();
    void updateSite(const Args& args);
	void removeSite(const Args& args);
	void showSite(const Args& args);
	void setSiteProp(const Args& siteName);

	/// about applications
	void listApps();
	void updateApp(const Args& args);
	void removeApp(const Args& args);

	/// about mount
	void mount(const Args& args);
	void unmount(const Args& args);

	virtual const char* getPrompt();

protected:

	Ice::CommunicatorPtr		_communicator;
	Ice::ObjectAdapterPtr		_adapter;
	TianShanIce::Site::SiteAdminPrx	 _bizPrx;
	
	//TianShanIce::Site::SessionAdminPrx  _sessPrx;

	TianShanIce::Events::EventChannelImpl::Ptr _sub;

	/*DummyApp::Ptr _app;*/
};

extern WeiwooAdminConsole gAdmin; // The current parser for bison/flex

#endif // __admin_H__
