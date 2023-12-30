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
// $Log: /ZQProjs/TianShan/Sentry/SentryImpl.h $
// 
// 2     1/11/16 6:08p Dejian.fei
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 2     08-06-06 12:06 Xiaohui.chai
// added getWebView()
// 
// 1     07-05-21 11:14 Hui.shao
// 
// 12    07-03-13 17:12 Hongquan.zhang
// 
// 11    07-01-05 10:59 Hongquan.zhang
// ===========================================================================

#ifndef __ZQTianShan_SentryImpl_H__
#define __ZQTianShan_SentryImpl_H__

#include "../common/TianShanDefines.h"
#include "../common/ZqSentryIce.h"

#include "SentryEnv.h"

namespace ZQTianShan {
namespace Sentry {

// -----------------------------
// loopback service AdapterCollectorImpl
// -----------------------------
//class AdapterCollectorImpl : public ::ZqSentryIce::AdapterCollector, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class AdapterCollectorImpl : public ::ZqSentryIce::AdapterCollector, public ICEAbstractMutexRLock
{
public:
    AdapterCollectorImpl(SentryEnv& env);
	virtual ~AdapterCollectorImpl();

public:	// impls of AdapterCollector

    virtual ::Ice::Int updateAdapter(::Ice::Int processId, const ::std::string& adapterId, ::Ice::Long lastChange, const ::Ice::Identity& identAdapterCB, const ::Ice::Current& c);
    virtual ::std::string getRootUrl(const ::Ice::Current& c);

protected:
	SentryEnv& _env;
};

// -----------------------------
// public service SentryServiceImpl
// -----------------------------
//class SentryServiceImpl : public ::ZqSentryIce::SentryService, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class SentryServiceImpl : public ::ZqSentryIce::SentryService, public ICEAbstractMutexRLock
{
public:
    SentryServiceImpl(SentryEnv& env);
	virtual ~SentryServiceImpl();

public:	// impls of NodeService

    virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

    virtual void getGroupAddress(::std::string& McastIp, ::Ice::Int& port, const ::Ice::Current& c);
    virtual void getProcessorInfo(::std::string& processor, ::Ice::Int& count, ::Ice::Long& frequencyMhz, const ::Ice::Current& c);
    virtual ::std::string getOSType(const ::Ice::Current& c);
    virtual ::std::string getRootUrl(const ::Ice::Current& c);
    virtual ::ZqSentryIce::ServiceInfos listServices(const ::Ice::Current& c);
    virtual ::Ice::Int stopDaemonByPid(::Ice::Int pid, bool restart, const ::Ice::Current& c);

    virtual ::ZqSentryIce::WebView getWebView(const ::Ice::Current& c);
protected:

	SentryEnv& _env;
	
};


}} // namespace

#endif // __ZQTianShan_SentryImpl_H__
