// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: PathCommand.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathCommand.cpp $
// 
// 4     3/22/17 6:29p Hui.shao
// CLOGFMT
// 
// 3     6/18/15 11:11a Hui.shao
// ticket#17865 to export serviceGroup usage via csv
// 
// 2     3/07/11 4:53p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 17    09-02-25 11:30 Hongquan.zhang
// receive and update replicas information
// 
// 16    08-08-14 14:58 Hui.shao
// merged from 1.7.10
// 
// 16    08-07-08 13:42 Hui.shao
// fixed for ICE 3.3 new syntax
// 
// 15    08-03-18 14:50 Hongquan.zhang
// must lock servant when using it
// 
// 14    07-12-14 11:37 Hongquan.zhang
// Check in for updating ErrorCode
// 
// 13    07-10-17 12:21 Hongquan.zhang
// 
// 12    07-09-18 12:55 Hongquan.zhang
// 
// 10    07-08-30 15:47 Hongquan.zhang
// 
// 9     07-06-21 15:35 Hongquan.zhang
// 
// 8     07-05-24 11:18 Hongquan.zhang
// 
// 7     07-03-14 12:33 Hongquan.zhang
// 
// 6     07-02-26 17:51 Hongquan.zhang
// 
// 5     06-12-25 16:58 Hui.shao
// fixed glog to envlog; _throw with envlog
// 
// 4     06-09-19 11:48 Hui.shao
// ===========================================================================

#include "PathCommand.h"

namespace ZQTianShan {
namespace AccreditedPath {

// -----------------------------
// class TicketNarrowCommand
// -----------------------------
TicketNarrowCommand::TicketNarrowCommand(const ::TianShanIce::Transport::AMD_PathTicket_narrowPtr& amdCB, 
										 PathSvcEnv& env,
										 ::ZQTianShan::AccreditedPath::ADPathTicketImpl& ticket,
										 const ::TianShanIce::SRM::SessionPrx& sess)
: ThreadRequest(env._thpool), _env(env), _Ticket(ticket), _sess(sess),  _amdCB(amdCB) {
#if ICE_INT_VERSION / 100 < 303
	try
	{
		env._ePathTicket->keep(ticket.ident);
		_bKeep = true;
	}
	catch (const Ice::Exception& ex) 
	{
		_bKeep = false;
	}
#endif
}

TicketNarrowCommand::~TicketNarrowCommand()
{
#if ICE_INT_VERSION / 100 < 303
	try
	{
		if(_bKeep)
		{
			_env._ePathTicket->release(_Ticket.ident);
			_bKeep = false;
		}		
	}
	catch (const Ice::Exception& ex) 
	{		
	}
#endif	
}

int TicketNarrowCommand::run(void)
{
#pragma message ( __MSGLOC__ "TODO: do we need mutex to avoid ticket field RW conflict?")
//		ADPathTicketImpl::WLock sync(*_pTicket);

	SessCtx sessCtx;
	try
	{
		//Get SRM session ID
		sessCtx.sessId = _sess->getId();
		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(PathTicket,"Session[%s] narrow with ticket [%s]"),
									sessCtx.sessId.c_str() , _Ticket.ident.name.c_str() );
		
		// start validate the session context here
		
		// v.1 validate the session state
		sessCtx.state = _sess->getState();
		
		sessCtx.strPrx = _env._communicator->proxyToString(_sess);
		
		sessCtx.resources = _sess->getReources();
		sessCtx.privdata  = _sess->getPrivateData();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(TicketNarrowCommand, "run() failed to retrieve session context for committng"));
	}

	try 
	{
		
		{
			//Must lock here because PHO may modify some properties of PathTicket			
			if (_env.pathHelperMgr().doNarrow(&_Ticket, sessCtx))
				_Ticket.state = TianShanIce::stInService;
		}
			
		if (TianShanIce::stInService != _Ticket.state)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog,"TicketNarrowCommand",1001, CLOGFMT(PathTicket, 
							"narrow() failed to turn ticket %s to-\"stInService\""), _Ticket.ident.name.c_str());
		}
		
		// reset the lease if necessary
		::Ice::Current dummyCurrent;
		if (_Ticket.getLeaseLeft(dummyCurrent) < DEFAULT_ALLOCATE_TICKET_LEASETERM)
			_Ticket.renew(DEFAULT_ALLOCATE_TICKET_LEASETERM, dummyCurrent);
		
		envlog(ZQ::common::Log::L_INFO,CLOGFMT(PathTicket,"Session[%s] narrow ok with ticket [%s]"),
								sessCtx.sessId.c_str(),_Ticket.ident.name.c_str());
		_amdCB->ice_response();
		return 0;
	}
	catch (const TianShanIce::BaseException& ex ) 
	{
		_amdCB->ice_exception(ex);
	}
	catch (const ::Ice::Exception& e)
	{
		_amdCB->ice_exception(e);
	}
    catch(const ::std::exception& ex)
    {
		_amdCB->ice_exception(ex);
    }
    catch(...)
    {
		_amdCB->ice_exception();
    }
	envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(PathTicket,"Session[%s] narrowed with ticket [%s] ok and return to client"),
									sessCtx.sessId.c_str() , _Ticket.ident.name.c_str() );

	return 1;
}
ReserveTicketCommand::ReserveTicketCommand(const ::TianShanIce::Transport::AMD_PathManager_reservePathsPtr& amdCB, 
							AccreditedPathsImpl& owner,
							::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintedLease, 
							const ::TianShanIce::SRM::SessionPrx& sess,const Ice::Current& c)
							:ThreadRequest(owner._env._thpool),_owner(owner),_sess(sess),_amdCB(amdCB),_current(c)
{
	_maxCost=maxCost;
	_maxTickets=maxTickets;
	_hintedLease=hintedLease;
}

ReserveTicketCommand::~ReserveTicketCommand()
{
	
}

int ReserveTicketCommand::run()
{
	::TianShanIce::ValueMap context;
	try
	{
		TianShanIce::Transport::PathTickets  tickets= _owner.reservePathsEx(_maxCost,_maxTickets,
																	_hintedLease,context,_sess,context,_current);
		_amdCB->ice_response(tickets);
	}
	catch( const TianShanIce::BaseException& ex )
	{
		_amdCB->ice_exception(ex);
	}
	catch(const Ice::Exception& ex)
	{
		_amdCB->ice_exception(ex);
	}
	catch(...)
	{
		_owner._env._log(ZQ::common::Log::L_ERROR, CLOGFMT(ReserveTicketCommand, "reservePathsEx() throws exception"));
	}	
	return 0;
}

SumServiceGroupBwCommand::SumServiceGroupBwCommand(PathSvcEnv& env, const ::TianShanIce::IValues& dirtySvcGroups)
: ThreadRequest(env._thpool), _env(env), _dirties(dirtySvcGroups)
{}

int SumServiceGroupBwCommand::run(void)
{
	int64 stampStart = ZQ::common::now();
	if (_env._stampLastSum >= stampStart -30000) // no often than 30sec
		return 1;

	_env._stampLastSum = stampStart;

	for (size_t i=0; i < _dirties.size(); i++)
	{
		_env.sumAssigned(_dirties[i]);
		::SYS::sleep(1); // yield
	}

	return 0;
}

}} // namespace
