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
// $Log: /ZQProjs/TianShan/SiteAdminSvc/admin/admin.h $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 4     08-01-22 11:57 Hui.shao
// 
// 3     08-01-22 11:56 Build
// 
// 2     08-01-22 11:47 Hui.shao
// 
// 10    07-04-19 17:50 Hongquan.zhang
// ===========================================================================

#ifndef __admin_H__
#define __admin_H__

#include "../../common/TianShanDefines.h"
#ifdef WITH_ICESTORM
#  include "../../common/EventChannel.h"
#endif // WITH_ICESTORM

#include "AdminConsole.h"
#include "../../SiteAdminSvc/SiteAdminSvc.h"


class SAConsole : public AdminConsole
{
	/*friend class DummyApp;*/
	friend class SessionEventSinkI;
	
public:
    SAConsole(const char* endpointBind = "tcp -p 11001");
	
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
	
	/// about transaction
	void listTxn(const Args& args);
	void trace(const Args& args);
	void dumpTxnXml(const Args& args);

	virtual const char* getPrompt();
	
protected:
	
	Ice::CommunicatorPtr		_communicator;
	Ice::ObjectAdapterPtr		_adapter;
	TianShanIce::Site::SiteAdminPrx	 _sasPrx;
	//TianShanIce::Site::TxnServicePrx _txnPrx;
	
	//TianShanIce::Site::SessionAdminPrx  _sessPrx;
	
#ifdef WITH_ICESTORM
	TianShanIce::Events::EventChannelImpl::Ptr _sub;
#endif // WITH_ICESTORM
};

extern SAConsole gAdmin; // The current parser for bison/flex

#endif // __admin_H__
