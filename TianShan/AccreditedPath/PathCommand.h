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
// Ident : $Id: PathCommand.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathCommand.h $
// 
// 2     6/18/15 11:11a Hui.shao
// ticket#17865 to export serviceGroup usage via csv
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 10    08-08-14 14:58 Hui.shao
// merged from 1.7.10
// 
// 10    08-07-08 13:42 Hui.shao
// fixed for ICE 3.3 new syntax
// 
// 9     08-03-18 14:50 Hongquan.zhang
// must lock servant when using it
// 
// 8     07-10-17 12:21 Hongquan.zhang
// 
// 7     07-09-18 14:02 Hongquan.zhang
// 
// 6     07-03-14 12:33 Hongquan.zhang
// 
// 5     06-09-19 11:46 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_PathCommand_H__
#define __ZQTianShan_PathCommand_H__

#include "../common/TianShanDefines.h"
#include "PathManagerImpl.h"
#include "NativeThreadPool.h"

namespace ZQTianShan {
namespace AccreditedPath {

// -----------------------------
// class TicketNarrowCommand
// -----------------------------
///
class TicketNarrowCommand : protected ZQ::common::ThreadRequest
{
public:
	/// constructor
    TicketNarrowCommand(const ::TianShanIce::Transport::AMD_PathTicket_narrowPtr& amdCB,
						PathSvcEnv& env, 
						::ZQTianShan::AccreditedPath::ADPathTicketImpl& ticket, 
						const ::TianShanIce::SRM::SessionPrx& sess);

	virtual ~TicketNarrowCommand();
	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	PathSvcEnv& _env;
	::ZQTianShan::AccreditedPath::ADPathTicketImpl&			_Ticket;
	::TianShanIce::SRM::SessionPrx						_sess;
	::TianShanIce::Transport::AMD_PathTicket_narrowPtr	_amdCB;
#if ICE_INT_VERSION / 100 < 303
	bool		_bKeep;
#endif
};

class ReserveTicketCommand : protected ZQ::common::ThreadRequest
{
public:
	/// constructor
    ReserveTicketCommand(const ::TianShanIce::Transport::AMD_PathManager_reservePathsPtr& amdCB, 
							AccreditedPathsImpl& owner,
							::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintedLease, 
							const ::TianShanIce::SRM::SessionPrx& sess,
							const Ice::Current& c);
	virtual ~ReserveTicketCommand();

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	AccreditedPathsImpl&									_owner;
	::TianShanIce::SRM::SessionPrx							_sess;
	::TianShanIce::Transport::AMD_PathManager_reservePathsPtr _amdCB;
	Ice::Int												_maxCost;
	::Ice::Int												_maxTickets;
	::Ice::Int												_hintedLease; 
	const Ice::Current&										_current;
#if ICE_INT_VERSION / 100 < 303
	bool		_bKeep;
#endif
};

// -----------------------------
// class TicketNarrowCommand
// -----------------------------
///
class SumServiceGroupBwCommand : protected ZQ::common::ThreadRequest
{
public:
	/// constructor
    SumServiceGroupBwCommand(PathSvcEnv& env, const ::TianShanIce::IValues& dirtySvcGroups);
	virtual ~SumServiceGroupBwCommand() {}
	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	PathSvcEnv& _env;
	::TianShanIce::IValues _dirties;
};


}} // namespace

#endif // __ZQTianShan_PathCommand_H__

