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
// Ident : $Id: SentryCommand.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/SentryCommand.h $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 3     08-11-24 19:18 Xiaohui.chai
// add multithread access protection for PeerInfoRefreshing::_pendingNodes
// 
// 2     07-11-05 15:51 Xiaohui.chai
// changed LocalAdapterRefreshing's interface
// 
// 1     07-05-21 11:14 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_SentryCommand_H__
#define __ZQTianShan_SentryCommand_H__

#include "../common/TianShanDefines.h"

#include "SentryEnv.h"
#include "SentryImpl.h"

namespace ZQTianShan {
namespace Sentry {

// -----------------------------
// class SentryCommand
// -----------------------------
///
class SentryCommand : protected ZQ::common::ThreadRequest
{
protected:
	/// constructor
	///@note no direct instantiation of SentryCommand is allowed
    SentryCommand(SentryEnv& env);

public:

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void) { return 0; }
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	SentryEnv&     _env;
};

// -----------------------------
// class LocalAdapterRefreshing
// -----------------------------
class LocalAdapterRefreshing : public SentryCommand
{
public:

	/// constructor
    LocalAdapterRefreshing(
        SentryEnv& env,
        const ::ZQTianShan::Sentry::SentryEnv::ProcessInfo& processInfo,
        const std::string& adapterId,
        const ::Ice::Long lastChange,
        ::ZqSentryIce::AdapterCBPrx callback
        );

protected: // overwrite of SentryCommand

	virtual int run(void);

protected:

	::ZQTianShan::Sentry::SentryEnv::ProcessInfo _processInfo;
	::Ice::Long _lastChange;
	::std::string _adapterId;
    ::ZqSentryIce::AdapterCBPrx _callback;
};

// -----------------------------
// class PeerInfoRefreshing
// -----------------------------
class PeerInfoRefreshing : public SentryCommand
{
public:

	/// constructor
    PeerInfoRefreshing(SentryEnv& env, const ::std::string& nodeId, const std::string& sentrysvcPrx, Ice::Long lastChanged);
    virtual ~PeerInfoRefreshing();

    static bool isPending(const std::string& nodeId);

protected: // overwrite of SentryCommand

	virtual int run(void);

protected:

	std::string _nodeId;
	std::string _sentrysvcPrx;
	Ice::Long   _lastChanged;
	::ZqSentryIce::SentryServicePrx _proxy;

	static std::set<std::string> _pendingNodes;
    static ZQ::common::Mutex _pendingNodesLock;

};

}} // namespace

#endif // __ZQTianShan_SentryCommand_H__

