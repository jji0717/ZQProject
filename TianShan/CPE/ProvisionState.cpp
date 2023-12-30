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
// Ident : $Id: ProvisionState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionState.cpp $
// 
// 10    1/12/16 8:55a Dejian.fei
// 
// 9     8/26/14 2:56p Hui.shao
// 
// 8     7/28/14 1:46p Li.huang
// fixed  bug 19274
// 
// 7     12/12/13 2:12p Hui.shao
// %lld/%llu for int64/uint64
// 
// 5     8/01/13 11:44a Ketao.zhang
// 
// 4     3/25/13 2:06p Hui.shao
// 
// 3     12/19/12 5:44p Li.huang
// 
// 2     8/02/11 11:36a Li.huang
// fix bug 14340
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 36    10-11-02 14:29 Li.huang
//  megre from 1.10
// 
// 35    09-08-19 12:22 Jie.zhang
// merge from 1.10
// 
// 40    09-07-21 17:54 Jie.zhang
// fix one log crash on Accepted::OnTimer(), %lld, but give a dword.
// 
// 39    09-07-14 15:50 Jie.zhang
// tuning for the quick-pause
// 
// 38    09-06-22 16:37 Jie.zhang
// fixed the issue of terminate called twice
// 
// 37    09-05-13 21:52 Jie.zhang
// 
// 36    09-05-08 16:17 Xia.chen
// 
// 35    09-05-08 14:17 Xia.chen
// 
// 34    09-05-05 17:25 Xia.chen
// add sessions to provisionstore map
// 
// 33    09-05-05 13:20 Jie.zhang
// 
// 32    09-03-19 16:00 Xia.chen
// amended cpesvc log info about  error msg for failing to initialize the
// HelperSession which leads to cpesvc crash because of using invalid
// session object.
// 
// 31    09-03-05 19:24 Xia.chen
// 
// 30    09-03-05 16:21 Jie.zhang
// 
// 29    09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 28    09-02-06 10:41 Jie.zhang
// 
// 27    08-12-15 17:52 Yixin.tian
// 
// 26    08-12-03 14:57 Jie.zhang
// 
// 25    08-11-18 11:09 Jie.zhang
// merge from TianShan1.8
// 
// 25    08-10-24 14:51 Jie.zhang
// fixed "cancelProvision" return error status
// 
// 24    08-06-26 12:34 Jie.zhang
// move keep() to a better place
// 
// 23    08-06-26 12:16 Jie.zhang
// add keep() to provisioning session so we could queryProgress correctly
// 
// 22    08-06-24 14:56 Jie.zhang
// changestatecmd removed, "setup","ontimer","onrestore" process in ice
// server dispatch thread.
// 
// 21    08-06-10 19:04 Jie.zhang
// 
// 20    08-05-30 18:26 Jie.zhang
// 
// 19    08-05-30 14:11 Jie.zhang
// 
// 18    08-05-17 19:08 Jie.zhang
// 
// 17    08-05-14 22:07 Jie.zhang
// 
// 16    08-05-13 11:31 Jie.zhang
// 
// 15    08-04-28 18:08 Xia.chen
// 
// 14    08-04-25 16:11 Jie.zhang
// 
// 13    08-04-09 18:16 Jie.zhang
// 
// 12    08-04-09 11:48 Hui.shao
// added ProvisionCost
// 
// 11    08-03-27 16:53 Jie.zhang
// 
// 13    08-03-25 14:07 Jie.zhang
// 
// 12    08-03-17 19:56 Jie.zhang
// 
// 11    08-03-07 18:14 Jie.zhang
// 
// 10    08-02-28 16:17 Jie.zhang
// 
// 9     08-02-21 18:27 Jie.zhang
// logs change and bug fixs
// 
// 8     08-02-21 13:51 Hongquan.zhang
// 
// 7     08-02-19 17:25 Hongquan.zhang
// 
// 6     08-02-19 16:09 Hongquan.zhang
// commit state change before update expiration 
// 
// 5     08-02-18 18:46 Jie.zhang
// changes check in
// 
// 4     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 3     08-02-14 18:47 Hui.shao
// impled ProvisionSessionBind callbacks
// 
// 2     08-02-14 12:16 Hui.shao
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#include "ProvisionState.h"

#include "CPEImpl.h"
#include "CPEEnv.h"
#include "ProvisionCmds.h"
#include "CPECfg.h"
#include "CPHInc.h"
#include "ErrorCode.h"

extern "C"
{
#include <stdlib.h>
#include <time.h>


}
#include <stdarg.h>
#include <stdio.h>

using namespace ZQ::common;

namespace ZQTianShan {
namespace CPE {

#define PROVSTATELOGFMT(_C, _X) CLOGFMT(_C, "provision[%s:%s(%d)] " _X), _sess.ident.name.c_str(), ProvisionStateBase::stateStr(_sess.state), _sess.state
#define PROVSTATEEXPFMT(_C, _ERRCODE, _X) EXPFMT(_C, _ERRCODE, "provision[%s:%s(%d)] " _X), _sess.ident.name.c_str(), ProvisionStateBase::stateStr(_sess.state), _sess.state

// -----------------------------
// class ProvisionStateBase
// -----------------------------
const char* ProvisionStateBase::stateStr(const ::TianShanIce::ContentProvision::ProvisionState state)
{
#define SWITCH_CASE_STATE(_ST)	case ::TianShanIce::ContentProvision::cps##_ST: return #_ST
	switch(state)
	{
		SWITCH_CASE_STATE(Created);
		SWITCH_CASE_STATE(Accepted);
		SWITCH_CASE_STATE(Wait);
		SWITCH_CASE_STATE(Ready);
		SWITCH_CASE_STATE(Provisioning);
		//		SWITCH_CASE_STATE(ProvisioningStreamable);
		SWITCH_CASE_STATE(Stopped);
	default:
		return "<Unknown>";
	}
#undef SWITCH_CASE_STATE
}

ProvisionStateBase::ProvisionStateBase(CPEEnv& env, ProvisionSessImpl& sess, const ::TianShanIce::ContentProvision::ProvisionState state)
: _env(env), _sess(sess), _oldState(sess.state), _theState(state)
{
}

int ProvisionStateBase::doAddResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvisionStateBase, 0501, "addResource() not allowed"));
	return 0; // dummy statement to avoid compile warnning
}

void ProvisionStateBase::doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvisionStateBase, 0601, "removeResource() not allowed"));
}

////
void ProvisionStateBase::_commitState(bool fireEvent)
{
#if ICE_INT_VERSION / 100 >= 306
	WLock lock(_sess);
#else
	ProvisionSessImpl::WLock lock(_sess);
#endif

	_sess.state = _theState;
	if (_theState == _oldState)
	{
#ifdef _DEBUG
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvisionState, "_commitState() same state, ignore"));
#endif // _DEBUG
		return;
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvisionState, "committing state change: %s(%d) -> %s(%d)"),
			ProvisionStateBase::stateStr(_oldState), _oldState, ProvisionStateBase::stateStr(_theState), _theState);	

	std::string ownerPrxStr;

	try 
	{
		if (_sess.owner)
		{
			ownerPrxStr = _env._communicator->proxyToString(_sess.owner);

			TianShanIce::ContentProvision::ProvisionSessionBindPrx prx =
				TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(_sess.owner->ice_collocationOptimized(false));

			envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvisionState, "notify state change to the owner[%s]: %s(%d) -> %s(%d)"),
				ownerPrxStr.c_str(),	ProvisionStateBase::stateStr(_oldState), _oldState, ProvisionStateBase::stateStr(_theState), _theState);	
			
			::TianShanIce::Properties params;
#if ICE_INT_VERSION / 100 >= 306
			OnProvisionStateCBPtr onProStateCbPtr = new OnProvisionStateCB(_env);
			Ice::CallbackPtr genericCB = Ice::newCallback(onProStateCbPtr, &OnProvisionStateCB::OnProvisionStateChanged);
			prx->begin_OnProvisionStateChanged( _sess.contentKey, ZQTianShan::now(), _oldState, _theState, params, genericCB);
#else	
			prx->OnProvisionStateChanged_async(new OnProvisionStateChangedAmiCBImpl(_env), _sess.contentKey, ZQTianShan::now(), _oldState, _theState, params);
#endif
		}
	}
	catch(const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvisionState, "_commitState() error occurs when nodify owner[%s], exception[%s]"), ownerPrxStr.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvisionState, "_commitState() error occurs when nodify owner[%s], unknown exception"), ownerPrxStr.c_str());
	}

	if (!fireEvent)
		return;


#pragma message ( __MSGLOC__ "TODO: fire the state change (_oldState->_theState) notification here")

// 	::TianShanIce::ContentProvision::ProvisionSessionPrx sessPrx = IdentityToObjEnv(_env, ProvisionSession,  _sess.ident);
// 	//commitStateChange
// 	TianShanIce::Site::TxnServicePrx txnPrx=_env.getTxnServicePrx();
// 	if(sessPrx && txnPrx && fireEvent)
// 	{
// 		try
// 		{
// //::TianShanIce::Properties::value_type(SYS_PROP(stampResolved), TimeToUTC(now(), buf, sizeof(buf)) ==NULL ? "" : buf));
// 			//Update system properties
// 			TianShanIce::Properties prop;
// 			if(_theState == TianShanIce::stInService)
// 			{//inService Set sys properties
// 
// 				//set stampServe
// 				char szBuf[128];
// 				prop[SYS_PROP(stampServe)]	=	TimeToUTC(now(),szBuf,sizeof(szBuf))==NULL ? "":szBuf ;
// 
// 				//set bandwidth
// 				TianShanIce::SRM::ResourceMap& resourceMap = _sess.resources;
// 				Ice::Long	bw2Alloc;				
// 				TianShanIce::SRM::Resource& res=resourceMap[TianShanIce::SRM::rtTsDownstreamBandwidth];
// 				if(res.resourceData.end() != res.resourceData.find("bandwidth") && !res.resourceData["bandwidth"].bRange && res.resourceData["bandwidth"].lints.size() >0 )
// 				{
// 					bw2Alloc=res.resourceData["bandwidth"].lints[0];
// 				}
// 				else
// 				{
// 					_env._log(ZQ::common::Log::L_ERROR,"No bandwidth resource with sess:%s",_sess.sessId.c_str());
// 				}
// 
// 				sprintf(szBuf,"%lld",bw2Alloc);
// 				prop[SYS_PROP(bandwidth)]	=	std::string(szBuf);
// 
// 				//get serviceGroup ID
// 				//serviceGroupID
// 				Ice::Int service// 	long nextWait =(long) ( (_sess.expiration <=0) ? _env._ttlIdleSess : (_sess.expiration - now()));	
// //	long ticketWait;
// //	try
// //	{
// //		if(_sess.ticket)
// //			ticketWait = _sess.ticket->getLeaseLeft();
// //		else
// //			ticketWait = _env._ttlIdleSess;
// //	}
// //	catch (...) 
// //	{
// //		envlog(ZQ::common::Log::L_ERROR,
// //			PROVSTATELOGFMT(ProvisionState,"Can't get ticket expiration,set it to default %d"),_env._ttlIdleSess);
// //		ticketWait = _env._ttlIdleSess ;
// //	}	
// //	nextWait =nextWait > ticketWait ?ticketWait:nextWait;
// #pragma message(__MSGLOC__"TODO:check the ticket expiration here")
// 
// 	if (nextWait <=0)
// 	{
// 		envlog(ZQ::common::Log::L_DEBUG,
// 				PROVSTATELOGFMT(ProvisionState, "met expiration %llu, entering stOutOfService"),
// 				_sess.expiration);
// 		
// 		if (_sess.privdata.find ( SYS_PROP(terminateReason) ) == _sess.privdata.end() )
// 		{			
// 			//Add terminateReason here
// 			TianShanIce::Variant varTerminateReason ;
// 			varTerminateReason.type = TianShanIce::vtStrings;
// 			varTerminateReason.strs.clear();
// 			
// 			char	szBuf[1024];
// 			sprintf(szBuf,"2120%d0 session timeout from state[%s]:",_theState,ZQTianShan::ObjStateStr(_theState));
// 			
// 			std::string	strErr = szBuf;
// 			//get last error
// 			TianShanIce::Variant& varLastErr = _sess.privdata[SYS_PROP(lastError_desc)];
// 			if (varLastErr.type == TianShanIce::vtStrings && varLastErr.strs.size() > 0) 
// 			{
// 				strErr =strErr + varLastErr.strs[0];
// 			}	
// 			
// 			varTerminateReason.strs.push_back(strErr);
// 			
// 			
// 			_sess.setPrivateData(SYS_PROP(terminateReason), varTerminateReason,c);
// 		}
// 		ProvStateReady(_env, _sess).enter();
// 	}
// 	else
// 	{
// 		_env._watchDog.watchSession(_sess.ident, nextWait);
// 	}
// GroupID;
// 				TianShanIce::SRM::Resource& resNodeGroupID = resourceMap[::TianShanIce::SRM::rtServiceGroup];
// 				if(resNodeGroupID.resourceData.end()!=res.resourceData.find("id")&&
// 					resNodeGroupID.resourceData["id"].ints.size()>0	)
// 				{
// 					serviceGroupID = 	resNodeGroupID.resourceData["id"].ints[0];
// 					sprintf(szBuf,"%d",serviceGroupID);
// 					prop[SYS_PROP(serviceGroupID)]	=	std::string(szBuf);
// 
// 				}
// 				else
// 				{
// 					_env._log(ZQ::common::Log::L_ERROR,"No ServiceGroupID resource with sess:%s",_sess.sessId.c_str());
// 				}
// 
// 				//set storage link
// 				TianShanIce::Transport::PathTicketPrx ticketPrx=_sess.ticket;
// 				TianShanIce::Transport::StorageLinkPrx storageLinkPrx=ticketPrx->getStorageLink();
// 				if(!storageLinkPrx)
// 				{
// 					_env._log(ZQ::common::Log::L_ERROR,"no storage link with ticket:%s sess:%s",ticketPrx->getIdent().name.c_str(),_sess.sessId.c_str());
// 					//return ???
// 				}
// 				else
// 				{
// 					prop[SYS_PROP(storageLink)]	= _env._adapter->getCommunicator()->proxyToString(storageLinkPrx);
// 					prop[SYS_PROP(contentStore)]	= storageLinkPrx->getStorageId();
// 				}
// 				
// 				//set stream link
// 				TianShanIce::Transport::StreamLinkPrx streamLinkPrx=ticketPrx->getStreamLink();
// 				if(!streamLinkPrx)
// 				{
// 					_env._log(ZQ::common::Log::L_ERROR,"no Stream link with ticket:%s sess:%s",ticketPrx->getIdent().name.c_str(),_sess.sessId.c_str());
// 					//return ???
// 				}
// 				else
// 				{
// 					prop[SYS_PROP(Streamer)]		= streamLinkPrx->getStreamerId(); //_sess.streamPrxStr;
// 				}
// 				
// 
// 				//set allocateCost
// 				int	iCost	=	ticketPrx->getCost();
// 				sprintf(szBuf,"%d",iCost);
// 				prop[SYS_PROP(allocateCost)]=	std::string(szBuf);
// 				
// 				//streamPrxStr
// 				try
// 				{
// 					TianShanIce::Streamer::StreamPrx strmPrx= TianShanIce::Streamer::StreamPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(_sess.streamPrxStr));
// 					if (strmPrx) 
// 					{
// 						prop[SYS_PROP(streamID)] = strmPrx->getIdent().name;
// 					}
// 				}
// 				catch (const Ice::Exception& ex) 
// 				{
// 					_env._log(ZQ::common::Log::L_ERROR,"Ice exception[%s] when get streamID with stream proxy string[%s]",ex.ice_name().c_str(),_sess.streamPrxStr.c_str());
// 				}
// 				catch (...) 
// 				{
// 					_env._log(ZQ::common::Log::L_ERROR,"Unknown exception when get streamID with stream proxy string[%s]",_sess.streamPrxStr.c_str());
// 				}
// 
// 				
// 			}
// 			else if ( _theState == TianShanIce::stOutOfService ) 
// 			{
// 				//send out the OutOfService event
// 				TianShanIce::ValueMap& valMap = _sess.privdata;
// 				TianShanIce::Variant varTeardownReason = valMap[SYS_PROP(teardownReason)];
// 				TianShanIce::Variant varTerminateReason = valMap[SYS_PROP(terminateReason)];
// 
// 				if (varTerminateReason.type == TianShanIce::vtStrings && varTerminateReason.strs.size()>0 ) 
// 				{
// 					prop[SYS_PROP(terminateReason)] = varTerminateReason.strs[0];
// 
// 					if (gCPEServiceConfig.lMixTeardownReasonAndTerminateReason >= 1) 
// 						prop[SYS_PROP(teardownReason)] = varTerminateReason.strs[0];
// 				}
// 				if ( varTeardownReason.type == TianShanIce::vtStrings && varTeardownReason.strs.size() > 0 ) 
// 				{
// 					prop[SYS_PROP(teardownReason)] = varTeardownReason.strs[0];
// 				}
// 			}
// 			txnPrx->commitStateChange_async(_env._commitStateChangePtr, _sess.ident.name,_theState,sessPrx,prop);
// 		}
// 		catch (TianShanIce::BaseException& ex)
// 		{
// 			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionManager,"publish OnStateChanged event to LiveTxn error:%s"),ex.message.c_str());
// 		}
// 		catch (Ice::Exception & ex)
// 		{
// 			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionManager,"publish OnStateChanged event to LiveTxn error:%s"),ex.ice_name().c_str());
// 		}
// 		catch (...)
// 		{
// 			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionManager, "exception occurs when publish OnStateChanged event to LiveTxn"));
// 		}
// 	}
// 	if (sessPrx && _env._sessEventPublisher)
// 	{
// 		try 
// 		{
// 			envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvisionState, "Sink the event: %s(%d) -> %s(%d) OK"),
// 				::ZQTianShan::ObjStateStr(_oldState), _oldState, ::ZQTianShan::ObjStateStr(_theState), _theState);	
// 			ZQ::common::MutexGuard gd(_env._lockSessEventSink);
// 			_env._sessEventPublisher->OnStateChanged(_sess.sessId, _env._communicator->proxyToString(sessPrx),
// 														_oldState,_theState);
// 		}
// 		catch (Ice::Exception& ex) 
// 		{
// 			envlog(ZQ::common::Log::L_ERROR,
// 				CLOGFMT(SessionManager,"catch an ice exception when publish event:%s"),ex.ice_name().c_str());
// 		}
// 		catch(...)
// 		{
// 			envlog(ZQ::common::Log::L_ERROR, 
// 				CLOGFMT(SessionManager, "exception occurs when publish OnStateChanged event"));
// 		}
// 	}
// 
	envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvisionState, "commit state change: %s(%d) -> %s(%d) OK"),
			ProvisionStateBase::stateStr(_oldState), _oldState, ProvisionStateBase::stateStr(_theState), _theState);	
}

int ProvisionStateBase::_addResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c)
{
	::TianShanIce::SRM::Resource res;
	res.attr = ::TianShanIce::SRM::raMandatoryNonNegotiable;
	res.status = ::TianShanIce::SRM::rsRequested;
	res.resourceData = resData;

#pragma message ( __MSGLOC__ "TODO: validate acceptable resources here")

	_sess.resources[type] = res; // simply take it
	
	envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvisionState, "added resource[%s] status(%s) attr(%s)"),
									ZQTianShan::ResourceTypeStr(type),
									ZQTianShan::ResourceStatusStr(res.status),
									ZQTianShan::ResourceAttrStr(res.attr));

	return _sess.resources.size();
}

void ProvisionStateBase::_removeResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{
	_sess.resources.erase(type);
}

void ProvisionStateBase::_updateExpiration(const ::Ice::Long newExpiration)
{

#if ICE_INT_VERSION / 100 >= 306
	WLock lock(_sess);
#else
	ProvisionSessImpl::WLock lock(_sess);
#endif

	_sess.expiration = newExpiration;
	if (_sess.expiration >0)
	{
		int64 nTime = newExpiration-now();
		if (nTime<0)
			nTime = 0;

		if (nTime> 7*24*3600*1000) // could not be more than 10 days
			nTime = 7*24*3600*1000;

		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvisionState, "update expiration with %d(ms)"), nTime);
		_env._watchDog.watchSession(_sess.ident, (long)nTime);	
	}
	else envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvisionState, "_updateExpiration() skip updating an invalid expiration value"));
}

void ProvisionStateBase::_destroySess()
{
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvisionState, "_destroySess() enter"));
	try
	{
		_env._eProvisionSession->remove(_sess.ident);
		envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvisionState, "_destroySess() session removed from DB"));

		if (!_sess.owner || !_sess.subMask.psmDestroy)
			return;

		std::string ownerPrxStr =_env._communicator->proxyToString(_sess.owner);
		TianShanIce::ContentProvision::ProvisionSessionBindPrx prx =
			TianShanIce::ContentProvision::ProvisionSessionBindPrx::uncheckedCast(_sess.owner->ice_collocationOptimized(false));

		static const ::TianShanIce::Properties params;
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvisionState, "notify session destroy to the owner[%s]"), ownerPrxStr.c_str());	
#if ICE_INT_VERSION / 100 >= 306
		OnProvisionStateCBPtr onProStateCbPtr = new OnProvisionStateCB(_env);
		Ice::CallbackPtr genericCB = Ice::newCallback(onProStateCbPtr, &OnProvisionStateCB::OnProvisionDestroyed);
		prx->begin_OnProvisionDestroyed( _sess.contentKey, ZQTianShan::now(), params, genericCB);
#else
		prx->OnProvisionDestroyed_async(new OnProvisionDestroyedAmiCBImpl(_env), _sess.contentKey, ZQTianShan::now(), params);
#endif
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvisionState, "_destroySess() object already gone from the container, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvisionState, "_destroySess() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvisionState, "_destroySess() caught unknown exception"));
	}

}

void ProvisionStateBase::OnRestore(const ::Ice::Current& c)
{
 	::Ice::Long stampWakeup = _sess.expiration >0 ? _sess.expiration : (ZQTianShan::now() + UNATTEND_TIMEOUT);
 	char buf[64];
 	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvisionState, "OnRestore() reset timer at %s"), ZQTianShan::TimeToUTC(stampWakeup, buf, sizeof(buf) -2));
 	_updateExpiration(stampWakeup);
}

// -----------------------------
// class ProvStateCreated
// -----------------------------
void ProvStateCreated::enter(void)
{
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateCreated, "enter()"));

	#pragma message ( __MSGLOC__ "TODO: handle how to restore from the safestore")
		// only allowed to be entered from the state of stNotProvisioned
		switch(_sess.state)
		{
		case ::TianShanIce::ContentProvision::cpsCreated:
			break; // do nothing, and continue with the initialization steps

		case ::TianShanIce::ContentProvision::cpsAccepted:
		case ::TianShanIce::ContentProvision::cpsWait:
		case ::TianShanIce::ContentProvision::cpsReady:
		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvStateCreated, 0101, "not allowed to enter"));
		}
	}

	_commitState(false); // do not send the state change for this state
	_updateExpiration(UNATTEND_TIMEOUT +now());
}

void ProvStateCreated::OnTimer(const ::Ice::Current& c)
{
	if (_sess.expiration <= 0)
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateCreated, "OnTimer() uninitialized expiration, set to expire in %d msec"), UNATTEND_TIMEOUT);
		_updateExpiration(now() + UNATTEND_TIMEOUT); // do not allow 0 expiration at this state
	}
	else
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvStateCreated, "OnTimer() long unattended provision record, force to stop"));
		ProvStateStopped(_env, _sess).enter();		
	}
}

void ProvStateCreated::OnRestore(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvStateCreated, "OnRestore() state[Created] session goes to state[Stopped]"));
#if ICE_INT_VERSION / 100 >= 306
	WLock lock(_sess);
#else
	ProvisionSessImpl::WLock lock(_sess);
#endif

	ProvStateStopped(_env, _sess).enter(); 
}


int ProvStateCreated::doAddResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c)
{
	return _addResource(type, resData, c);
}

void ProvStateCreated::doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{
	_removeResource(type, c);
}

// -----------------------------
// class ProvStateAccepted
// -----------------------------
void ProvStateAccepted::enter(void)
{
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateAccepted, "enter()"));
		
		// only allowed to be entered from the state of stNotProvisioned
		switch(_sess.state)
		{
		case ::TianShanIce::ContentProvision::cpsAccepted:
			return; // do nothing

		case ::TianShanIce::ContentProvision::cpsCreated:
			ProvStateCreated(_env, _sess).leave(); break;
			
		case ::TianShanIce::ContentProvision::cpsWait:
		case ::TianShanIce::ContentProvision::cpsReady:
		case ::TianShanIce::ContentProvision::cpsProvisioning:
		case ::TianShanIce::ContentProvision::cpsStopped:
		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 101, "enter() not allowed from this state"));
		}

		try {
			
			// step 1. associate the proper method type
	#pragma message ( __MSGLOC__ "TODO: automation to resolve the method type")
			if (_sess.methodType.empty())
				ZQTianShan::_IceThrow<TianShanIce::NotSupported> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 102, "enter() not supported to automatically associate methodType"));
			
			// step 2. find the CP helper by mothodType
			ZQTianShan::ContentProvision::ICPHelper* pHelper =_env._provisionFactory->findHelper(_sess.methodType.c_str());
			if (NULL == pHelper)
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 103, "failed to locate CPH for method[%s]"), _sess.methodType.c_str());
			
			// step 3. validate the setup
			if (!pHelper->validateSetup(&_sess))
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 104, "failed to pass setup validation for method[%s]"), _sess.methodType.c_str());
			// step 4. validate the load
			TianShanIce::ValueMap& resBw = _sess.resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
			if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 105, "bandwidth not specified"));
			
			TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
			if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 105, "bandwidth not specified"));

			int nBandwith = (uint32) var1.lints[0];
			if (!_env.provisionCost(_sess.ident, _sess.methodType, _sess.scheduledStart/1000, _sess.scheduledEnd/1000, nBandwith/1000))
				ZQTianShan::_IceThrow<TianShanIce::ContentProvision::OutOfResource> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 106, "book session for method[%s] out of resource"), _sess.methodType.c_str());
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			ex.ice_throw();
		}
		catch (const ::Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 0110, "enter() caught exception[%s]"), ex.ice_name().c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateAccepted, 0110, "enter() caught unknown exception"));
		}

		// step 5. update the timout to wakeup at (_sess.scheduledStart + MAX_START_DELAY)
		char buf[64];
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateAccepted, "enter() renew to expire %dms after scheduledStart[%s]"), 
			_gCPECfg._dwMaxStartDelay, ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2));
	}

	_commitState();
	_updateExpiration(_sess.scheduledStart + _gCPECfg._dwMaxStartDelay);
}

void ProvStateAccepted::OnTimer(const ::Ice::Current& c)
{
	char buf[64];
	::Ice::Long stampNow = ZQTianShan::now();
	if (stampNow >= _sess.scheduledStart + _gCPECfg._dwMaxStartDelay)
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvStateAccepted, "OnTimer() uncommitted session, %lld ms missed scheduledStart[%s], force to the session to stop"), stampNow - _sess.scheduledStart, ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2));
		char tmp[256];
		sprintf(tmp, "%lldms missed scheduledStart[%s], force to stop",  stampNow - _sess.scheduledStart, ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2));
		_sess.notifyError(ERRCODE_SCHEDULE_EXPIRED, tmp);
		ProvStateStopped(_env, _sess).enter();
		return;
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateAccepted, "OnTimer() renew to expire %d ms after scheduledStart[%s]"), 
		_gCPECfg._dwMaxStartDelay, ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2));
	_updateExpiration(_sess.scheduledStart + _gCPECfg._dwMaxStartDelay);
}

void ProvStateAccepted::OnRestore(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateAccepted, "OnRestore() force to state[Wait]"));

#if ICE_INT_VERSION / 100 >= 306
	WLock lock(_sess);
#else
	ProvisionSessImpl::WLock lock(_sess);
#endif

	::TianShanIce::ContentProvision::ProvisionState target = ::TianShanIce::ContentProvision::cpsWait;
	
	try
	{
		_env._pProvStore.addProvisionStore(_sess.ident, _sess.methodType, _sess.scheduledStart/1000, _sess.scheduledEnd/1000, _sess._getBandwidth()/1000);
		ProvStateWait(_env, _sess).enter(); 
		return;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) exception[%s] %s"), 
			ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) exception[%s]"),
			ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) unknown exception"), 
			ProvisionStateBase::stateStr(target), target);
	}
	
	ProvStateStopped(_env, _sess).enter(); 
}


// -----------------------------
// class ProvStateWait
// -----------------------------
void ProvStateWait::enter(void)
{
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateWait, "enter()"));

		// only allowed to be entered from the state of cpsAccepted
		// and cpsProvisioning if this is a stScheduledRestorable session
		switch(_sess.state)
		{
		case ::TianShanIce::ContentProvision::cpsWait:
		case ::TianShanIce::ContentProvision::cpsReady:
		case ::TianShanIce::ContentProvision::cpsStopped:
			return; // do nothing

		case ::TianShanIce::ContentProvision::cpsAccepted:
			ProvStateAccepted(_env, _sess).leave(); break;

		case ::TianShanIce::ContentProvision::cpsProvisioning:
			if (::TianShanIce::ContentProvision::stScheduledRestorable == _sess.stType)
			{
				ProvStateProvisioning(_env, _sess).leave();
				break;
			}
			// no break statement here;

		case ::TianShanIce::ContentProvision::cpsCreated:
		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvStateWait, 0101, "enter() not allowed from this state"));
		}

		// update the timeout to wakeup at (_sess.scheduledStart - _sess.preload)
		if (_sess.preload <0)
			_sess.preload =0;
	}

	char buf[64];
	::Ice::Long stampPreload = _sess.scheduledStart - _sess.preload;
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateWait, "enter() renew to preload %dms before scheduledStart: %s"), _sess.preload, ZQTianShan::TimeToUTC(stampPreload, buf, sizeof(buf)-2));

	_commitState();
	_updateExpiration(stampPreload);
}

void ProvStateWait::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampPreload = _sess.scheduledStart - _sess.preload;
	::Ice::Long stampNow = ZQTianShan::now();

	char buf[64];

	//if the end time reachs, or the duration less than smallest value, give up
	if (stampNow >= _sess.scheduledEnd - _gCPECfg._minDurationSeconds*1000)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvStateWait, "OnTimer() scheduledEnd[%s] is too close to current, give up ingest"), ZQTianShan::TimeToUTC(_sess.scheduledEnd, buf, sizeof(buf)-2));
		_sess.notifyError(0, "schedule end time is too close to current time");		

		ProvStateStopped(_env, _sess).enter(); 
		return;
	}

	//if preload time arrives
	if (stampNow >= stampPreload)
	{
		envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateWait, "OnTimer() preload %dms before scheduledStart[%s] is ment, entering state[cpsReady]"), _sess.preload, ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2));

		::TianShanIce::ContentProvision::ProvisionState target = ::TianShanIce::ContentProvision::cpsReady;
		try 
		{
			ProvStateReady(_env, _sess).enter(); 
			return;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvStateWait, "OnTimer() failed to change state to %s(%d) exception[%s] %s"), 
				ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvStateWait, "OnTimer() failed to change state to %s(%d) exception[%s]"),
				ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvStateWait, "OnTimer() failed to change state to %s(%d) unknown exception"), 
				ProvisionStateBase::stateStr(target), target);
		}
		
		ProvStateStopped(_env, _sess).enter(); 
		return;
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateWait, "OnTimer() renew to preload %dms before scheduledStart: %s"), _sess.preload, ZQTianShan::TimeToUTC(stampPreload, buf, sizeof(buf)-2));
	_updateExpiration(stampPreload);
}

void ProvStateWait::OnRestore(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateWait, "OnRestore() force to OnTimer()"));
	OnTimer(c);

	_env._pProvStore.addProvisionStore(_sess.ident,_sess.methodType,_sess.scheduledStart/1000,_sess.scheduledEnd/1000,_sess._getBandwidth()/1000);
}

// -----------------------------
// class ProvStateReady
// -----------------------------
void ProvStateReady::enter(void)
{	
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateReady, "enter()"));

		//this line is very important, if no this line, the queryProgress will return a wrong number because of evictor logic
#if ICE_INT_VERSION / 100 >= 303
		Freeze::BackgroundSaveEvictorPtr pProvsionEvt = Freeze::BackgroundSaveEvictorPtr::dynamicCast(_env._eProvisionSession);
		pProvsionEvt->keep(_sess.ident);
#else
		_env._eProvisionSession->keep(_sess.ident);
#endif

		// only allowed to be entered from the state of cpsAccepted
		// and cpsProvisioning if this is a stScheduledRestorable session
		switch(_sess.state)
		{
		case ::TianShanIce::ContentProvision::cpsReady:
		case ::TianShanIce::ContentProvision::cpsProvisioning:
		case ::TianShanIce::ContentProvision::cpsStopped:
			return; // do nothing

		case ::TianShanIce::ContentProvision::cpsWait:
			ProvStateWait(_env, _sess).leave();
			break;
			
		case ::TianShanIce::ContentProvision::cpsCreated:
		case ::TianShanIce::ContentProvision::cpsAccepted:
		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0101, "enter() not allowed from this state"));
		}
	#if 1
		try {
		
			if (_sess.methodType.empty())
				ZQTianShan::_IceThrow<TianShanIce::NotSupported> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0102, "enter() not supported to automatically associate methodType"));
			//modify trickspeed if the content have userMetedata "user.WishedTrickSpeeds"
			TianShanIce::Properties::const_iterator it = _sess.props.find(CPHPM_WISHEDTRICKSPEEDS);
			if (it != _sess.props.end())
			{
				_sess.trickSpeeds.clear();
				std::string trickspeeds = it->second;
				TianShanIce::StrValues strTricks;
				strTricks = ZQ::common::stringHelper::split(trickspeeds, ';');
				for(int i = 0; i < strTricks.size(); i++)
				{
					float trick  = 0;
					sscanf(strTricks[i].c_str(), "%f", &trick);
					_sess.trickSpeeds.push_back(trick);
				}
			}

			// step 1. find the CP helper by mothodType
			ZQTianShan::ContentProvision::ICPHelper* pHelper =_env._provisionFactory->findHelper(_sess.methodType.c_str());
			if (NULL == pHelper)
				ZQTianShan::_IceThrow<TianShanIce::NotSupported> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0103, "failed to locate CPH for method[%s]"), _sess.methodType.c_str());

			// step 2. create a helper session on the located provision helper
			ZQTianShan::ContentProvision::ICPHSession* pCPHSess = pHelper->createHelperSession(_sess.methodType.c_str(), &_sess);
			if (NULL == pCPHSess)
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0104, "failed to create a HelperSession for method[%s]"), _sess.methodType.c_str());
			
			// step 3. initialize the helper session
			if (!pCPHSess->preLoad())
			{
				std::string errmsg = pCPHSess->getErrorMsg();
				_sess.notifyError(pCPHSess->getErrorCode(), pCPHSess->getErrorMsg());
				try
				{
					pCPHSess->terminate(_sess._bSucc);
				}catch(...){}
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0105, "failed to initialize the HelperSession with error:[%s]"),errmsg.c_str());
			}

		}
		catch (const TianShanIce::BaseException& ex) 
		{
			ex.ice_throw();
		}
		catch (const ::Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0110, "enter() caught exception[%s]"), ex.ice_name().c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0110, "enter() caught unknown exception"));
		}
	#else
		ZQTianShan::ContentProvision::ICPHSession* pCPHSess = NULL;
		try {
			
			if (_sess.methodType.empty())
				ZQTianShan::_IceThrow<TianShanIce::NotSupported> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0102, "enter() not supported to automatically associate methodType"));
			
			// step 1. find the CP helper by mothodType
			ZQTianShan::ContentProvision::ICPHelper* pHelper =_env._provisionFactory->findHelper(_sess.methodType.c_str());
			if (NULL == pHelper)
				ZQTianShan::_IceThrow<TianShanIce::NotSupported> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0103, "failed to locate CPH for method[%s]"), _sess.methodType.c_str());
			
			// step 2. create a helper session on the located provision helper
			pCPHSess = pHelper->createHelperSession(_sess.methodType.c_str(), &_sess);
			if (NULL == pCPHSess)
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0104, "failed to create a HelperSession for method[%s]"), _sess.methodType.c_str());
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			ex.ice_throw();
		}
		catch (const ::Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0110, "enter() caught exception[%s]"), ex.ice_name().c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0110, "enter() caught unknown exception"));
		}

		// step 3. initialize the helper session
		if (!pCPHSess->preLoad())
		{
			_sess.notifyError(pCPHSess->getErrorCode(), pCPHSess->getErrorMsg());
			try
			{
				pCPHSess->terminate(_sess._bSucc);
				}catch(...){}
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0105, "failed to initialize the HelperSession"));
		}
	#endif
	}
	
	// step 5. update the timeout to wakeup at (_sess.scheduledStart)	
	_commitState();
	
	char buf[64];
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateReady, "enter() renew to timeout at scheduledStart[%s]"), ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2));	
	_updateExpiration(_sess.scheduledStart);
	
}

void ProvStateReady::leave(void)
{
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateReady, "leave()"));

	if (!_sess._bSucc)
	{
//		::Ice::Long stampNow = ZQTianShan::now();

		try {

			ZQTianShan::ContentProvision::ICPHSession* pCPHSess = _env._provisionFactory->findHelperSession(_sess.ident.name.c_str());
			if (NULL != pCPHSess)
				pCPHSess->terminate(_sess._bSucc);

			envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateReady, "leave() helperSession has been destroyed"));
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			ex.ice_throw();
		}
		catch (const ::Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0110, "leave() caught exception[%s]"), ex.ice_name().c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0110, "leave() caught unknown exception"));
		}
	}
}


void ProvStateReady::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = ZQTianShan::now();

	char buf[64];
	if (::TianShanIce::ContentProvision::stPushTrigger != _sess.stType && stampNow >= _sess.scheduledStart)
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateReady, "OnTimer() scheduledStart[%s] is met, entering state[cpsProvisioning]"), ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2));

		::TianShanIce::ContentProvision::ProvisionState target = ::TianShanIce::ContentProvision::cpsProvisioning;
		try 
		{
			ProvStateProvisioning(_env, _sess).enter(); 
			return;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvStateReady, "OnTimer() failed to change state to %s(%d) exception[%s] %s"), 
				ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvStateReady, "OnTimer() failed to change state to %s(%d) exception[%s]"),
				ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ProvStateReady, "failed to change state to %s(%d) unknown exception"), 
				ProvisionStateBase::stateStr(target), target);
		}
		
		ProvStateStopped(_env, _sess).enter(); 
		return;
	}

	if (::TianShanIce::ContentProvision::stPushTrigger == _sess.stType && stampNow >= _sess.scheduledStart + _gCPECfg._dwMaxStartDelay)
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateReady, "OnTimer() PushTrigger session scheduledStart[%s] has been missed longer than %dms but no ingest, force to stop"), ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2), _gCPECfg._dwMaxStartDelay);
		char tmp[256];
		sprintf(tmp, "scheduledStart[%s] has been missed longer than %dms, force to stop", ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2), _gCPECfg._dwMaxStartDelay);
		_sess.notifyError(ERRCODE_SCHEDULE_EXPIRED, tmp);

		ProvStateStopped(_env, _sess).enter(); 
		return;
	}

	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateReady, "OnTimer() renew to timeout at scheduledStart[%s] with window %dms"), 
		ZQTianShan::TimeToUTC(_sess.scheduledStart, buf, sizeof(buf)-2), _gCPECfg._dwMaxStartDelay);
	_updateExpiration(_sess.scheduledStart + _gCPECfg._dwMaxStartDelay);
}

void ProvStateReady::OnRestore(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateReady, "OnRestore() force to re-procedure the HelperSession initialization from state[Wait]"));
	//ProvisionSessImpl::WLock lock(_sess);
#if ICE_INT_VERSION / 100 >= 306
	WLock lock(_sess);
#else
	ProvisionSessImpl::WLock lock(_sess);
#endif

	::TianShanIce::ContentProvision::ProvisionState target = ::TianShanIce::ContentProvision::cpsReady;
	try 
	{
		_env._pProvStore.addProvisionStore(_sess.ident,_sess.methodType,_sess.scheduledStart/1000,_sess.scheduledEnd/1000,_sess._getBandwidth()/1000);
		_sess.state = ::TianShanIce::ContentProvision::cpsWait;

		ProvStateReady(_env, _sess).enter(); 
		return;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) exception[%s] %s"), 
			ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) exception[%s]"),
			ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) unknown exception"), 
			ProvisionStateBase::stateStr(target), target);
	}
	
	ProvStateStopped(_env, _sess).enter(); 
}

// -----------------------------
// class ProvStateProvisioning
// -----------------------------
void ProvStateProvisioning::enter(void)
{	
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateProvisioning, "enter()"));
		
		// only allowed to be entered from the state of cpsAccepted
		// and cpsProvisioning if this is a stScheduledRestorable session
		switch(_sess.state)
		{
		case ::TianShanIce::ContentProvision::cpsProvisioning:
		case ::TianShanIce::ContentProvision::cpsStopped:
			return; // do nothing
			
		case ::TianShanIce::ContentProvision::cpsReady:
			ProvStateReady(_env, _sess).leave();
			break;
			
		case ::TianShanIce::ContentProvision::cpsWait:
		case ::TianShanIce::ContentProvision::cpsCreated:
		case ::TianShanIce::ContentProvision::cpsAccepted:
		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvStateProvisioning, 0101, "enter() not allowed from this state"));
		}

		try {

			ZQTianShan::ContentProvision::ICPHSession* pCPHSess = _env._provisionFactory->findHelperSession(_sess.ident.name.c_str());
			if (NULL == pCPHSess)
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateProvisioning, 0102, "failed to find HelperSession[%s] "), _sess.ident.name.c_str());

			if (!pCPHSess->prime())
			{
				std::string strErr = pCPHSess->getErrorMsg();
				_sess.notifyError(pCPHSess->getErrorCode(), pCPHSess->getErrorMsg());

				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateReady, 0105, "failed to initialize the HelperSession"));
			}
			
			pCPHSess->execute();		
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			ex.ice_throw();
		}
		catch (const ::Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateProvisioning, 0110, "enter() caught exception[%s]"), ex.ice_name().c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateProvisioning, 0110, "enter() caught unknown exception"));
		}
		
		if (_sess.linger <=0)
			_sess.linger =0;
	}

	char buf[64];
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateProvisioning, "enter() renew to timeout at scheduledEnd[%s] with linger %dms"), ZQTianShan::TimeToUTC(_sess.scheduledEnd, buf, sizeof(buf)-2), _sess.linger);
	_commitState();
	_updateExpiration(_sess.scheduledEnd + _sess.linger);
}

void ProvStateProvisioning::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = ZQTianShan::now();
	
	char buf[64];
	if (stampNow >= _sess.scheduledEnd + _sess.linger)
	{
		envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateProvisioning, "OnTimer() scheduledEnd[%s] with linger %dms has been met, force to stop"), ZQTianShan::TimeToUTC(_sess.scheduledEnd, buf, sizeof(buf)-2), _sess.linger);		
		ProvStateStopped(_env, _sess).enter(); 
		return;
	}
	
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateProvisioning, "OnTimer() renew to timeout at scheduledEnd[%s] with linger %dms"), ZQTianShan::TimeToUTC(_sess.scheduledEnd, buf, sizeof(buf)-2), _sess.linger);
	_updateExpiration(_sess.scheduledEnd + _sess.linger);
}

void ProvStateProvisioning::leave()
{	
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateProvisioning, "leave()"));

//	::Ice::Long stampNow = ZQTianShan::now();
	
	try {
		
		ZQTianShan::ContentProvision::ICPHSession* pCPHSess = _env._provisionFactory->findHelperSession(_sess.ident.name.c_str());

#pragma message ( __MSGLOC__ "TODO: detail with stop reason: 1) terminated by timer, 2) cancelled by user, 3) terminated by CPHSession")
		if (NULL != pCPHSess)
			pCPHSess->terminate(_sess._bSucc);

		envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateProvisioning, "leave() helperSession has been destroyed"));
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		ex.ice_throw();
	}
	catch (const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateProvisioning, 0110, "leave() caught exception[%s]"), ex.ice_name().c_str());
	}
    catch(...)
    {
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, PROVSTATEEXPFMT(ProvStateProvisioning, 0110, "leave() caught unknown exception"));
    }
}

void ProvStateProvisioning::OnRestore(const ::Ice::Current& c)
{
#if ICE_INT_VERSION / 100 >= 306
	WLock lock(_sess);
#else
	ProvisionSessImpl::WLock lock(_sess);
#endif
	if (::TianShanIce::ContentProvision::stScheduledRestorable == _sess.stType && _sess.scheduledEnd > ZQ::common::TimeUtil::now())
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateProvisioning, "OnRestore() force to re-procedure the HelperSession initialization from state[Wait]"));

		::TianShanIce::ContentProvision::ProvisionState target = ::TianShanIce::ContentProvision::cpsReady;
		try
		{
			_env._pProvStore.addProvisionStore(_sess.ident,_sess.methodType,_sess.scheduledStart/1000,_sess.scheduledEnd/1000,_sess._getBandwidth()/1000);
			_sess.state = ::TianShanIce::ContentProvision::cpsWait;
			ProvStateReady(_env, _sess).enter(); 	
			return;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) exception[%s] %s"), 
				ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) exception[%s]"),
				ProvisionStateBase::stateStr(target), target, ex.ice_name().c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, PROVSTATELOGFMT(ChangeState, "failed to change state to %s(%d) unknown exception"), 
				ProvisionStateBase::stateStr(target), target);
		}
		
		ProvStateStopped(_env, _sess).enter(); 			
	}
	else
	{
		envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateReady, "OnRestore() send session error notification"));
		_sess.notifyError(ERRCODE_USER_CANCELED, "CPESvc restarted");

		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvStateProvisioning, "OnRestore() ever started non-restoreable session goes to state[Stopped]"));
		ProvStateStopped(_env, _sess).enter(); 	
	}
}

// -----------------------------
// class ProvStateStopped
// -----------------------------
void ProvStateStopped::enter(void)
{	
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateStopped, "enter()"));
	
	try {
		switch(_sess.state)
		{
		case ::TianShanIce::ContentProvision::cpsStopped:
			return; // do nothing
			
		case ::TianShanIce::ContentProvision::cpsCreated:
			ProvStateCreated(_env, _sess).leave();
			break;

		case ::TianShanIce::ContentProvision::cpsAccepted:
			ProvStateAccepted(_env, _sess).leave();
			break;
			
		case ::TianShanIce::ContentProvision::cpsWait:
			ProvStateWait(_env, _sess).leave();
			break;

		case ::TianShanIce::ContentProvision::cpsReady:
			ProvStateReady(_env, _sess).leave();
			break;
	
		case ::TianShanIce::ContentProvision::cpsProvisioning:
			ProvStateProvisioning(_env, _sess).leave();
			break;

		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, PROVSTATEEXPFMT(ProvStateStopped, 0101, "enter() not allowed from this state"));
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvStateStopped, "enter() ignore the excpetion[%s] when leaving prev state: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvStateStopped, "enter() ignore the excpetion[%s] when leaving prev state"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_WARNING, PROVSTATELOGFMT(ProvStateStopped, "enter() ignore the unknown excpetion when leaving prev state"));
	}

	::Ice::Long timeToWakeup = ZQTianShan::now() + _gCPECfg._dwStopRemainTimeout;
	char buf[64];
	envlog(ZQ::common::Log::L_DEBUG, PROVSTATELOGFMT(ProvStateStopped, "enter() wakeup to clean at [%s] with linger %dms"),
		ZQTianShan::TimeToUTC(timeToWakeup, buf, sizeof(buf)-2), _gCPECfg._dwStopRemainTimeout);

	_commitState();
	_updateExpiration(timeToWakeup);
}

void ProvStateStopped::OnTimer(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateStopped, "OnTimer() cleaning session"));
	try {
		leave();
	}
	catch(...) {}

	if (_env._pProvStore.delProvisionStore(_sess.ident))
	{
		envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateStopped, "OnTimer() successfully delete provision[%s] from provisionStore map"), _sess.ident.name.c_str());
	}
	_destroySess();
}

void ProvStateStopped::OnRestore(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_INFO, PROVSTATELOGFMT(ProvStateStopped, "OnRestore() force to OnTimer()"));
	OnTimer(c);
}

}} // namespace

