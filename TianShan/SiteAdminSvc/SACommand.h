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
// Ident : $Id: SessionCommand.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SACommand.h $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 3     08-03-18 17:37 Xiaohui.chai
// changed interface of ListLiveTxn
// 
// 2     07-12-10 18:47 Hui.shao
// moved event out of txn
// ===========================================================================

#ifndef __ZQTianShan_SACommand_H__
#define __ZQTianShan_SACommand_H__

#include "../common/TianShanDefines.h"

#include "SiteAdminSvc.h"
#include "SiteAdminSvcEnv.h"
#include "SiteAdminImpl.h"

namespace ZQTianShan {
namespace Site {

// -----------------------------
// class SaveEventCommand
// -----------------------------
///
class SaveEventCommand : protected ZQ::common::ThreadRequest
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
    SaveEventCommand(const ::TianShanIce::Site::AMD_TxnService_tracePtr& amdCB, SiteAdminSvcEnv& env, const ::std::string& sessId, const ::std::string& category, const ::std::string& eventCode, const ::std::string& eventMsg);
    SaveEventCommand(::Ice::Long stamp, SiteAdminSvcEnv& _env, const ::std::string& sessId, const ::std::string& category, ::std::string& eventCode, const ::std::string& eventMsg);
	virtual ~SaveEventCommand() { _event=NULL; }

public:

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	SiteAdminSvcEnv&  _env;
	::TianShanIce::Site::AMD_TxnService_tracePtr _amdCB;

	TxnEventImpl::Ptr _event;
	::Ice::Long _stamp;
};

// -----------------------------
// class ListTxnCommand
// -----------------------------
///
class ListTxnCommand : protected ZQ::common::ThreadRequest
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
    ListTxnCommand(const ::TianShanIce::Site::AMD_TxnService_listLiveTxnPtr& amdCB, SiteAdminSvcEnv& env, const ::std::string& siteName, const ::std::string& appMount, const ::TianShanIce::StrValues& paramNames, const ::std::string& startId, int maxCount);
	virtual ~ListTxnCommand() {}
	
public:
	
	void execute(void) { start(); }
	
protected: // impls of ThreadRequest
	
	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }
	
protected:
	
	SiteAdminSvcEnv&  _env;
	::TianShanIce::Site::AMD_TxnService_listLiveTxnPtr _amdCB;

	::std::string _siteName, _appMount;
	::TianShanIce::StrValues _paramNames;

    uint32 _maxCount;
    ::std::string _startId;
};

}} // namespace

#endif // __ZQTianShan_SessionCommand_H__

