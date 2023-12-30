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
// Ident : $Id: SessionState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionState.cpp $
// 
// 41    1/16/17 4:58p Hui.shao
// pass the lasterr as the terminate reason
// 
// 40    11/22/16 6:20p Hui.shao
// 
// 38    10/13/16 3:52p Hui.shao
// 
// 41    10/13/16 3:48p Hui.shao
// 
// 40    10/13/16 11:36a Hui.shao
// 
// 39    10/13/16 11:31a Hui.shao
// 
// 38    10/11/16 4:22p Hui.shao
// 
// 37    9/21/16 2:44p Hui.shao
// 
// 33    2/25/16 4:50p Hui.shao
// added rtServiceGroup
// 
// 34    2/23/16 10:34a Hui.shao
// added Weiwoo session id as the unique deliveryId to hint
// 
// 32    1/21/16 11:27a Hui.shao
// ticket#18765, DWH¡Á????¦Ì3????¡¥¨¬?SessionId2?¨¤?group
// 
// 31    11/10/15 6:43p Hui.shao
// 
// 30    11/10/15 4:31p Hui.shao
// 
// 29    11/10/15 4:22p Hui.shao
// 
// 30    11/10/15 1:49p Hui.shao
// ticket#18220 to prevent double destroy()
// 
// 28    11/10/15 12:00p Hui.shao
// ticket#18220 to prevent double destroy()
// 
// 27    10/28/15 3:27p Hui.shao
// 
// 29    10/28/15 3:25p Hui.shao
// log fmt
// 
// 26    8/13/15 3:25p Hui.shao
// merged reasonstr determination from V1.16
// 
// 25    2/06/15 11:49a Hui.shao
// 
// 24    1/07/15 2:03p Hui.shao
// 
// 25    1/07/15 2:00p Hui.shao
// 
// 24    12/18/14 2:05p Hui.shao
// 
// 23    11/26/14 9:42a Hui.shao
// moved the reason-determination piror to detaching purchase
// 
// 22    11/19/14 6:57p Hui.shao
// 
// 21    11/19/14 6:55p Hui.shao
// 
// 20    11/04/14 11:08a Hui.shao
// 
// 19    8/21/14 4:37p Hui.shao
// 
// 18    8/11/14 2:26p Hongquan.zhang
// 
// 17    7/07/14 4:54p Hui.shao
// TIMEOUT_YIELD_AFTER_ERROR_OCCURED
// 
// 16    6/11/14 3:17p Hui.shao
// 
// 15    6/05/14 11:12a Hui.shao
// 
// 14    5/05/14 2:53p Build
// save the selected PathTicketId into Session::privData
// 
// 13    5/05/14 2:37p Build
// 
// 12    12/12/13 2:16p Hui.shao
// %lld/%llu for int64/uint64
// 
// 10    5/24/13 2:34p Build
// 
// 9     4/24/13 5:16p Build
// 
// 8     11/26/12 8:29p Hui.shao
// 
// 7     3/18/11 3:11p Hongquan.zhang
// 
// 6     3/10/11 3:29p Hongquan.zhang
// 
// 5     3/10/11 3:26p Hongquan.zhang
// 
// 4     3/10/11 3:23p Hongquan.zhang
// 
// 3     3/09/11 4:42p Hongquan.zhang
// 
// 2     3/07/11 4:59p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 69    10-06-18 17:04 Build
// 
// 68    09-10-08 17:10 Hui.shao
// 
// 67    09-08-24 17:20 Hongquan.zhang
// 
// 66    09-06-05 17:17 Li.huang
// 
// 65    09-06-05 14:13 Hongquan.zhang
// modify some typo 
// 
// 64    09-06-04 17:10 Hui.shao
// support of the override timeout of session
// 
// 63    08-11-25 15:03 Xiaoming.li
// use map::find method
// 
// 62    08-11-24 15:29 Hongquan.zhang
// 
// 61    08-08-11 15:43 Hongquan.zhang
// 
// 60    08-07-08 15:59 Hongquan.zhang
// 
// 59    08-05-14 18:09 Hongquan.zhang
// 
// 58    08-04-23 11:11 Hongquan.zhang
// migrate configuration to ConfigHelper
// Add Ice Performance Tunning configuration when OpenDB
// 
// 57    08-03-18 14:55 Hongquan.zhang
// lock servant if using it
// 
// 56    07-12-21 15:13 Hongquan.zhang
// 
// 55    07-12-14 11:39 Hongquan.zhang
// Update Error Code
// 
// 54    07-12-05 15:49 Hongquan.zhang
// 
// 53    07-11-26 15:38 Hongquan.zhang
// modify because Purchase::detach has been changed
// 
// 52    07-11-19 11:53 Hongquan.zhang
// 
// 51    07-10-31 14:43 Hongquan.zhang
// 
// 50    07-10-25 14:07 Hongquan.zhang
// 
// 49    07-10-17 12:29 Hongquan.zhang
// 
// 48    07-09-18 12:56 Hongquan.zhang
// 
// 47    07-08-30 16:22 Hongquan.zhang
// 
// 46    07-08-30 15:44 Hongquan.zhang
// 
// 43    07-07-02 15:26 Hongquan.zhang
// 
// 42    07-07-02 12:12 Hongquan.zhang
// 
// 41    07-06-28 17:08 Hongquan.zhang
// 
// 40    07-06-26 13:29 Hongquan.zhang
// 
// 39    07-06-21 15:37 Hongquan.zhang
// 
// 38    07-06-18 10:28 Hongquan.zhang
// 
// 37    07-06-06 16:16 Hongquan.zhang
// 
// 36    07-05-24 11:35 Hongquan.zhang
// 
// 35    07-05-16 16:57 Hongquan.zhang
// 
// 34    07-05-15 15:02 Hongquan.zhang
// 
// 33    07-05-10 10:20 Hongquan.zhang
// 
// 32    07-05-09 18:16 Hongquan.zhang
// 
// 31    07-05-09 17:45 Hongquan.zhang
// 
// 29    07-04-12 14:02 Hongquan.zhang
// 
// 28    07-03-28 16:42 Hui.shao
// moved business router to namespace Site
// 
// 27    07-03-21 16:04 Hui.shao
// add to commit path ticket
// 
// 26    07-03-19 15:19 Hongquan.zhang
// 
// 25    07-03-13 17:12 Hongquan.zhang
// 
// 24    07-03-07 14:47 Hongquan.zhang
// 
// 23    07-03-01 15:27 Hongquan.zhang
// 
// 22    07-02-26 17:51 Hongquan.zhang
// 
// 21    07-01-11 16:09 Hongquan.zhang
// 
// 20    07-01-09 15:15 Hongquan.zhang
// 
// 19    07-01-05 14:43 Hui.shao
// moved the static proxy pathmgr into WeiwooEnv
// 
// 18    07-01-05 10:59 Hongquan.zhang
// 
// 17    06-12-28 16:45 Hongquan.zhang
// 
// 16    06-12-25 15:51 Hui.shao
// uniform the throw
// 
// 15    06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 14    06-12-25 12:22 Hongquan.zhang
// 
// 13    06-12-13 18:48 Hongquan.zhang
// 
// 12    06-10-16 10:23 Hongquan.zhang
// 
// 11    10/09/06 6:53p Hui.shao
// 
// 10    10/09/06 4:17p Hui.shao
// count procedure latencies
// 
// 9     06-09-13 12:04 Hui.shao
// a simple edge to eliminate timer calls during state change procedure
// 
// 8     06-09-12 20:19 Hui.shao
// added SessionWatchDog
// 
// 7     06-08-29 12:33 Hui.shao
// 
// 6     06-08-28 18:29 Hui.shao
// 
// 5     06-08-25 14:27 Hui.shao
// 
// 4     06-08-24 19:23 Hui.shao
// 
// 3     06-08-21 17:05 Hui.shao
// 
// 2     06-08-17 20:09 Hui.shao
// 
// 1     06-08-16 20:33 Hui.shao
// ===========================================================================



#include "SessionState.h"

#include "SessionImpl.h"
#include "WeiwooSvcEnv.h"
#include "SessionCommand.h"
#include "WeiwooConfig.h"
#include "SystemUtils.h"
#include <Ice/Ice.h>


#include "../Ice/TsSite.h"
#include "../Ice/TsApplication.h"
#include "../Ice/TsTransport.h"
#include "../Ice/TsStreamer.h"

#include <TianShanIceHelper.h>

extern "C"
{
#include <stdlib.h>
#include <time.h>
}
#include <stdarg.h>
#include <stdio.h>

using namespace ZQ::common;

namespace ZQTianShan {
namespace Weiwoo {

#define SESSIONSTATEFMT(_C, _X) "%-12s sess[%s:%s(%d)] " _X, #_C, _sess.ident.name.c_str(), ::ZQTianShan::ObjStateStr(_sess.state), _sess.state
#define SESSIONSTATELOGFMT(_C, _X) CLOGFMT(_C, "sess[%s:%s(%d)] " _X), _sess.ident.name.c_str(), ::ZQTianShan::ObjStateStr(_sess.state), _sess.state

#if ICE_INT_VERSION / 100 >= 306
	#define SESSWLOCK(_SESS)	WLockT<SessionImpl> gd(_SESS)
	#define SESSRLOCK(_SESS)	RLockT<SessionImpl> gd(_SESS)
#else
	#define SESSWLOCK(_SESS)	IceUtil::WLockT<SessionImpl> gd(_SESS)
	#define SESSRLOCK(_SESS)	IceUtil::RLockT<SessionImpl> gd(_SESS)
#endif

SessStateChangeGuard::SessStateChangeGuard(SessionImpl& sess, const ::TianShanIce::State targetState, bool bIdempotent)
: _sess(sess)
{
	while (TianShanIce::stInService == _sess._env._serviceState || TianShanIce::stProvisioned == _sess._env._serviceState)
	{
		{
			SESSWLOCK(_sess);
			if (!bIdempotent && _sess._stateChangeTargetState == targetState)
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_sess._env._log, "SessionState", 400, SESSIONSTATEFMT(SessState, "non-idempotent state-change to %s(%d) double submitted"), ::ZQTianShan::ObjStateStr(targetState), targetState);

			if (_sess._bStateInChange)
			{
				SYS::sleep(1);
				continue;
			}

			_sess._bStateInChange = true;
			_sess._stateChangeTargetState = targetState;

#ifdef _DEBUG
			_sess._env._log(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessionState, "state in changing to %s(%d)"), ::ZQTianShan::ObjStateStr(targetState), targetState);
#endif
			return;
		} // end of locker
	}

	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_sess._env._log, "SessionState", 400, SESSIONSTATEFMT(SessState, "state-change to %s(%d) terminated due to serviceState[%s(%d)]"), ::ZQTianShan::ObjStateStr(targetState), targetState, ::ZQTianShan::ObjStateStr(_sess._env._serviceState), _sess._env._serviceState);
}

SessStateChangeGuard::~SessStateChangeGuard() 
{
	SESSWLOCK(_sess);
	_sess._bStateInChange = false; 
	_sess._stateChangeTargetState = _sess.state; // reset to that has been committed
}

// -----------------------------
// class SessStateBase
// -----------------------------
SessStateBase::SessStateBase(WeiwooSvcEnv& env, SessionImpl& sess, const ::TianShanIce::State state)
: _env(env), _sess(sess), _theState(state), _oldState(sess.state), _errLog(env, sess)
{
//	_env._eSession.keep(_sess.ident);
}

void SessStateBase::doDestroy(const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "SessStateBase", 1001, SESSIONSTATEFMT(SessionState, "destory() not allowed"));
}

::Ice::Long SessStateBase::doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "SessStateBase", 1011, SESSIONSTATEFMT(SessionState, "addResourceEx() not allowed"));
	return 0; // dummy statement to avoid compile warnning
}

void SessStateBase::doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "SessStateBase", 1021, SESSIONSTATEFMT(SessionState, "removeResource() not allowed"));
}

void SessStateBase::doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "SessStateBase", 1031, SESSIONSTATEFMT(SessionState, "negotiateResources() not allowed"));
}

void SessStateBase::doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c)
{
	ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "SessStateBase", 1041, SESSIONSTATEFMT(SessionState, "attachPurchase() not allowed"));
}

void SessStateBase::setLastErrorMsg( const std::string& errMsg, bool asTerminateReason)
{
	TianShanIce::Variant var;
	var.type = TianShanIce::vtStrings;
	var.bRange	= false;
	var.strs.clear();
	var.strs.push_back(errMsg);
	SESSWLOCK(_sess);
	_sess.privdata[SYS_PROP(lastError_desc)] = var;
	if (asTerminateReason)
	{
		_sess.privdata[SYS_PROP(terminateReason)] = var;
	}

	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessionState, "setLastErrorMsg() %s"), errMsg.c_str());
}

void SessStateBase::ErrLog::writeMessage(const char *msg, int level/*=-1*/)
{
	_env._log( level, msg  );	
}

void SessStateBase::OnTimer(const ::Ice::Current& c)
{
	long nextWait =(long) ( (_sess.expiration <=0) ? _env._ttlIdleSess : (_sess.expiration - now()));	
//	long ticketWait;
//	try
//	{
//		if(_sess.ticket)
//			ticketWait = _sess.ticket->getLeaseLeft();
//		else
//			ticketWait = _env._ttlIdleSess;
//	}
//	catch (...) 
//	{
//		envlog(ZQ::common::Log::L_ERROR, 
//			SESSIONSTATEFMT(SessionState, "Can't get ticket expiration, set it to default %d"), _env._ttlIdleSess);
//		ticketWait = _env._ttlIdleSess ;
//	}	
//	nextWait =nextWait > ticketWait ?ticketWait:nextWait;
#pragma message(__MSGLOC__"TODO:check the ticket expiration here")

	if (nextWait <=0 )
	{
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessionState, "met expiration %llu, entering stOutOfService"), 
				_sess.expiration);

		{		
			bool bFoundTerminateReason = false;
			{
				SESSRLOCK(_sess);
				bFoundTerminateReason = _sess.privdata.find ( SYS_PROP(terminateReason) ) != _sess.privdata.end();
			}

			if ( !bFoundTerminateReason )
			{			
				//Add terminateReason here
				TianShanIce::Variant varTerminateReason ;
				varTerminateReason.bRange = false;
				varTerminateReason.type = TianShanIce::vtStrings;
				varTerminateReason.strs.clear();
				
				char szBuf[4096], *p=szBuf;
				snprintf(p, szBuf + sizeof(szBuf)-2 - p, "2120%d0 session quit from state[%s]:", _theState, ZQTianShan::ObjStateStr(_theState)); p += strlen(p);

				if (_sess.expiration == _sess.overrideExp)
					snprintf(p, szBuf + sizeof(szBuf)-2 - p, "forced by OverrideTimeout"); p += strlen(p);

				//get last error
				{
					SESSRLOCK(_sess);
					//edit by lxm for debug at 2008-11-25
					::TianShanIce::ValueMap::iterator iter = _sess.privdata.find(SYS_PROP(lastError_desc));
					if (iter != _sess.privdata.end())
					{
						TianShanIce::Variant& varLastErr = (*iter).second;
						if (varLastErr.type == TianShanIce::vtStrings && varLastErr.strs.size() > 0) 
						{
							snprintf(p, szBuf + sizeof(szBuf)-2 - p, varLastErr.strs[0].c_str() ); p += strlen(p);
						}
					}

					//TianShanIce::Variant& varLastErr = _sess.privdata[SYS_PROP(lastError_desc)];
					//if (varLastErr.type == TianShanIce::vtStrings && varLastErr.strs.size() > 0) 
					//{
					//	strErr =strErr + varLastErr.strs[0];
					//}
				}
				
				varTerminateReason.strs.push_back(szBuf);
				_sess.setPrivateData(SYS_PROP(terminateReason), varTerminateReason, c);
				envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessionState, "teminateReason is built: %s"), szBuf);
			}
		}

		SessStateOutOfService(_env, _sess).enter();
	}
	else
	{
		_env._watchDog.watchSession(_sess.ident, nextWait);
	}
}

////
void SessStateBase::_commitState(bool fireEvent)
{
	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessionState, "commit state change: %s(%d) -> %s(%d)"), 
			::ZQTianShan::ObjStateStr(_oldState), _oldState, ::ZQTianShan::ObjStateStr(_theState), _theState);	

#pragma message ( __MSGLOC__ "TODO: fire the state change (_oldState->_theState) notification here")
	::TianShanIce::SRM::SessionPrx sessPrx = IdentityToObj(Session, _sess.ident);
	//commitStateChange
	TianShanIce::Site::TxnServicePrx txnPrx=_env.getTxnServicePrx();
	if (sessPrx && txnPrx && fireEvent)
	{
		try
		{
			//::TianShanIce::Properties::value_type(SYS_PROP(stampResolved), TimeToUTC(now(), buf, sizeof(buf)) ==NULL ? "" : buf));
			//Update system properties
			TianShanIce::Properties prop;
			if(_theState == TianShanIce::stInService)
			{//inService Set sys properties

				//set stampServe
				char szBuf[128];
				prop[SYS_PROP(stampServe)]= TimeToUTC(now(), szBuf, sizeof(szBuf))==NULL ? "":szBuf ;

				//set bandwidth
				TianShanIce::SRM::ResourceMap resourceMap;
				{
					SESSRLOCK(_sess);
					resourceMap = _sess.resources;
				}

				Ice::Long	bw2Alloc;				
				TianShanIce::SRM::Resource& res = resourceMap[TianShanIce::SRM::rtTsDownstreamBandwidth];
				if(res.resourceData.end() != res.resourceData.find("bandwidth") && 
					!res.resourceData["bandwidth"].bRange && 
					res.resourceData["bandwidth"].lints.size() >0 )
				{
					bw2Alloc=res.resourceData["bandwidth"].lints[0];
				}
				else
				{
					_env._log(ZQ::common::Log::L_ERROR, "No bandwidth resource with sess:%s", _sess.sessId.c_str());
				}

				sprintf(szBuf, FMT64, bw2Alloc);
				prop[SYS_PROP(bandwidth)]	=	std::string(szBuf);

				//get serviceGroup ID
				//serviceGroupID
				Ice::Int serviceGroupID;
				TianShanIce::SRM::Resource& resNodeGroupID = resourceMap[::TianShanIce::SRM::rtServiceGroup];
				if( resNodeGroupID.resourceData.end() != resNodeGroupID.resourceData.find("id") &&
					resNodeGroupID.resourceData["id"].ints.size()>0	)
				{
					serviceGroupID = 	resNodeGroupID.resourceData["id"].ints[0];
					sprintf(szBuf, "%d", serviceGroupID);
					prop[SYS_PROP(serviceGroupID)]	=	std::string(szBuf);

				}
				else
				{
					_env._log(ZQ::common::Log::L_ERROR, "No ServiceGroupID resource with sess:%s", _sess.sessId.c_str());
				}

				//set storage link
				TianShanIce::Transport::PathTicketPrx ticketPrx; 

				{
					SESSRLOCK(_sess);
					ticketPrx = _sess.ticket;
				}

				TianShanIce::Transport::StorageLinkPrx storageLinkPrx=ticketPrx->getStorageLink();
				if(!storageLinkPrx)
				{
					_env._log(ZQ::common::Log::L_ERROR, "no storage link with ticket:%s sess:%s", ticketPrx->getIdent().name.c_str(), _sess.sessId.c_str());
					//return ???
				}
				else
				{
					prop[SYS_PROP(storageLink)]	= _env._adapter->getCommunicator()->proxyToString(storageLinkPrx);
					std::string storageId = storageLinkPrx->getStorageId();
					prop[SYS_PROP(contentStore)]	= storageId;
					ZQTianShan::Util::updateResourceData( _sess.resources, TianShanIce::SRM::rtStorage, "NetworkId", storageId);
				}

				//set stream link
				TianShanIce::Transport::StreamLinkPrx streamLinkPrx=ticketPrx->getStreamLink();
				if(!streamLinkPrx)
				{
					_env._log(ZQ::common::Log::L_ERROR, "no Stream link with ticket:%s sess:%s", ticketPrx->getIdent().name.c_str(), _sess.sessId.c_str());
					//return ???
				}
				else
				{
					std::string streamerId = streamLinkPrx->getStreamerId();
					prop[SYS_PROP(Streamer)]		= streamerId;
					prop[SYS_PROP(streamer)]		= streamerId;
					ZQTianShan::Util::updateResourceData( _sess.resources, TianShanIce::SRM::rtStreamer, "NetworkId", streamerId);
				}


				//set allocateCost
				int	iCost	=	ticketPrx->getCost();
				sprintf(szBuf, "%d", iCost);
				prop[SYS_PROP(allocateCost)]=	std::string(szBuf);

				//streamPrxStr
				try
				{
					TianShanIce::Streamer::StreamPrx strmPrx= TianShanIce::Streamer::StreamPrx::uncheckedCast(_env._adapter->getCommunicator()->stringToProxy(_sess.streamPrxStr));
					if (strmPrx) 
					{
						prop[SYS_PROP(streamID)] = strmPrx->getIdent().name;
					}
				}
				catch (const Ice::Exception& ex) 
				{
					_env._log(ZQ::common::Log::L_ERROR, "exception[%s] occurs when get streamID with stream proxy string[%s]", ex.ice_name().c_str(), _sess.streamPrxStr.c_str());
				}
				catch (...) 
				{
					_env._log(ZQ::common::Log::L_ERROR, "caught exception when get streamID with stream proxy string[%s]", _sess.streamPrxStr.c_str());
				}


			}
			else if ( _theState == TianShanIce::stOutOfService ) 
			{
				//send out the OutOfService event
				SESSRLOCK(_sess);

				// add the TEARDOWN/TERMINATE reason string
				TianShanIce::ValueMap::const_iterator itPD = _sess.privdata.find(SYS_PROP(terminateReason));
				if ((_sess.privdata.end() != itPD) && (itPD->second.type == TianShanIce::vtStrings) && (itPD->second.strs.size()>0)) 
				{ MAPSET(TianShanIce::Properties, prop, SYS_PROP(terminateReason), itPD->second.strs[0]); }

				itPD = _sess.privdata.find(SYS_PROP(teardownReason));
				if ((_sess.privdata.end() != itPD) && (itPD->second.type == TianShanIce::vtStrings) && (itPD->second.strs.size()>0) ) 
				{ MAPSET(TianShanIce::Properties, prop, SYS_PROP(teardownReason), itPD->second.strs[0]); }
			}
#if ICE_INT_VERSION / 100 >= 306
			TxnStateCBPtr commitCbPtr = new TxnStateCB(_env);
			Ice::CallbackPtr commitCB = Ice::newCallback(commitCbPtr, &TxnStateCB::commitStateChange);
			txnPrx->begin_commitStateChange(_sess.ident.name, _theState, sessPrx, prop ,commitCB);
#else	
			txnPrx->commitStateChange_async(new TxnCommitStateChangeAmiImpl(_env), _sess.ident.name, _theState, sessPrx, prop);
#endif
		}
		catch (TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionManager, "publish OnStateChanged event to LiveTxn error:%s"), ex.message.c_str());
		}
		catch (Ice::Exception & ex)
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionManager, "publish OnStateChanged event to LiveTxn error:%s"), ex.ice_name().c_str());
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionManager, "exception occurs when publish OnStateChanged event to LiveTxn"));
		}
	}

	if (sessPrx && _env._sessEventPublisher)
	{
		try 
		{
			envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessionState, "Sink the event: %s(%d) -> %s(%d) OK"), 
				::ZQTianShan::ObjStateStr(_oldState), _oldState, ::ZQTianShan::ObjStateStr(_theState), _theState);	
			ZQ::common::MutexGuard gd(_env._lockSessEventSink);
			_env._sessEventPublisher->OnStateChanged(_sess.sessId, _env._communicator->proxyToString(sessPrx), 
														_oldState, _theState);
		}
		catch (Ice::Exception& ex) 
		{
			envlog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(SessionManager, "caught exception[%s] when publish event"), ex.ice_name().c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(SessionManager, "exception occurs when publish OnStateChanged event"));
		}
	}

	{
		SESSWLOCK(_sess);
		_sess.state = _theState;
	}

	envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessionState, "commit state change: %s(%d) -> %s(%d) OK"), 
			::ZQTianShan::ObjStateStr(_oldState), _oldState, ::ZQTianShan::ObjStateStr(_theState), _theState);	
}

::Ice::Long SessStateBase::_addResourceEx(::TianShanIce::SRM::ResourceType type, 
										  const ::TianShanIce::SRM::Resource& res, 
										  const ::Ice::Current& c)
{
	TianShanIce::SRM::ResourceMap::iterator itRes = _sess.resources.find (type);
	if (itRes != _sess.resources.end ())
	{
		::TianShanIce::SRM::Resource& oldres = itRes->second;
		TianShanIce::ValueMap& data =oldres.resourceData;
		oldres.attr = res.attr;
		oldres.status = res.status;
		TianShanIce::ValueMap::const_iterator itResData = res.resourceData.begin ();
		for ( ; itResData != res.resourceData.end () ; itResData ++)
			MAPSET(TianShanIce::ValueMap, data, itResData->first, itResData->second);
	}
	else
	{		
//		SESSWLOCK(_sess);
		_sess.resources[type] = res;
	}

#pragma message ( __MSGLOC__ "Should dump the resource added here")
	envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(Session, " Resource with State(%s) with Type(%s) status(%s) attr(%s)"), 
									::ZQTianShan::ObjStateStr(_sess.state), 
									ZQTianShan::ResourceTypeStr(type), 
									ZQTianShan::ResourceStatusStr(res.status), 
									ZQTianShan::ResourceAttrStr(res.attr)	);

	const ::TianShanIce::ValueMap& valmap = res.resourceData;
	char	szBuf[4096];
	for (::TianShanIce::ValueMap::const_iterator it = valmap.begin(); it != valmap.end(); it ++)
	{
		memset(szBuf, 0, sizeof(szBuf));
		char* p= szBuf;
		sprintf( szBuf, "Variant Key:(%s)\t value:", it->first.c_str() );
		p+=strlen(p);
		const TianShanIce::Variant& val = it->second;
		sprintf(p, "{(%c) ", (val.bRange ? 'R' : 'E')); p += strlen(p);
		switch (val.type)
		{
		case ::TianShanIce::vtInts:
			{
				sprintf(p, "I: "); p += strlen(p);
				for (::TianShanIce::IValues::const_iterator it = val.ints.begin(); it < val.ints.end(); it++)
					sprintf(p, " %d; ", *it); p += strlen(p);
			}
			break;
			
		case ::TianShanIce::vtLongs:
			{
				sprintf(p, "L: "); p += strlen(p);
				for (::TianShanIce::LValues::const_iterator it = val.lints.begin(); it < val.lints.end(); it++)
					sprintf(p, " "FMT64"; ", *it); p += strlen(p);
			}
			break;
			
		case ::TianShanIce::vtStrings:
			{
				sprintf(p, "S: "); p += strlen(p);
				for (::TianShanIce::StrValues::const_iterator it = val.strs.begin(); it < val.strs.end(); it++)
					sprintf(p, " \"%s\"; ", it->c_str() ); p += strlen(p);
			}
			break;
			
		case ::TianShanIce::vtBin:
			{
				sprintf(p, "B: "); p += strlen(p);
				for (::TianShanIce::BValues::const_iterator it = val.bin.begin(); it < val.bin.end(); it++)
					sprintf(p, " %02x ", *it); p += strlen(p);
			}
			break;
		default:
			break;
		}
		sprintf(p, "}");
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(Session, "%s"), szBuf);
		//sprintf(p, "%s: ", it->first.c_str());
		//dumpVariant(it->second, buf, dumpLine);
	}
	return 0;
}

void SessStateBase::_removeResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{	
	_sess.resources.erase(type);
}

void SessStateBase::_attachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c)
{
	_sess.purchasePrxStr = purchase ? _env._communicator->proxyToString(purchase) : "";
	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(Session, "attachPurchase(%s)"), _sess.purchasePrxStr.c_str());
}

void SessStateBase::_updateExpiration(const ::Ice::Long newExpiration)
{
#pragma message ( __MSGLOC__ "TODO: SESSWLOCK(sess) here?")
	_sess.expiration = newExpiration;
	if (_sess.overrideExp >0 && _sess.expiration > _sess.overrideExp)
		_sess.expiration = _sess.overrideExp;

	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(Session, "update expiration with %lld(ms)"), newExpiration-now());
	if (_sess.expiration >0)
		_env._watchDog.watchSession(_sess.ident, (long)(_sess.expiration - now()));
}

/*
void SessStateBase::_updateTimer(const ::Ice::Long newTimeout)
{
	::Ice::Long nextTimeout = 0;
	if (NULL != _sess._nextTimerCmd)
	{
		try {
			nextTimeout = _sess._nextTimerCmd->getTimer64();

			if (0 == nextTimeout || newTimeout < nextTimeout)
			{
				_env._schd.clearTask(*_sess._nextTimerCmd);

				delete _sess._nextTimerCmd;
				_sess._nextTimerCmd = NULL;
			}
		}
		catch (...) {}
	}

	if (NULL == _sess._nextTimerCmd || 0 == nextTimeout || newTimeout < nextTimeout)
	{
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATEFMT(Session, "_updateTimer() update timeout=%lldms"), newTimeout);
		_sess._nextTimerCmd = new SessionTimerCommand(_env, _sess.ident);
		_sess._nextTimerCmd->startWait(newTimeout);
	}
}
*/
void SessStateBase::_destroySess(const ::Ice::Identity& ident)
{
	try
	{
		envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateBase, "enter _destroy session"));
		_env._eSession->remove(ident);

		// fire the events to the subscribers
		if (_env._sessEventPublisher)
		{
			try
			{
				_env._sessEventPublisher->OnDestroySession(ident.name);
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SessStateBase, "exception occurs when publish OnDestroySession event"));
			}
		}
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessStateBase, "object already gone, ignore"));
	}
	catch(const ::Ice::NotRegisteredException&)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessStateBase, "object already gone"));
	}
	catch(const ::Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, "SessStateBase", 1051, SESSIONSTATEFMT(SessStateBase, "_destroySess() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, "SessStateBase", 1052, SESSIONSTATEFMT(SessStateBase, "_destroySess() caught exception"));
	}
}

// -----------------------------
// class SessStateNotProvisioned
// -----------------------------
void SessStateNotProvisioned::enter(void)
{
	SessStateChangeGuard sscg(_sess, ::TianShanIce::stNotProvisioned);

	// only allowed to be entered from the state of stNotProvisioned
	switch(_sess.state)
	{
	case ::TianShanIce::stNotProvisioned:
		break; // do nothing, and continue with the initialization steps

	case ::TianShanIce::stProvisioned:
	case ::TianShanIce::stInService:
	case ::TianShanIce::stOutOfService:
	default:
		setLastErrorMsg("Invalid state change");
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_errLog, "SessStateNotProvisioned", 1061, SESSIONSTATEFMT(SessStateNotProvisioned, "not allowed to enter"));
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "enter SessStateNotProvisioned"));

	_updateExpiration(_env._ttlIdleSess+now());

	// fire the events to the subscribers
	//must make sure that the object exist
	::TianShanIce::SRM::SessionPrx prx = IdentityToObj(Session, _sess.ident);

	if (prx && _env._sessEventPublisher)
	{
		try 
		{
			ZQ::common::MutexGuard gd(_env._lockSessEventSink);
			_env._sessEventPublisher->OnNewSession(_sess.sessId, _env._communicator->proxyToString(prx));
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionManager, "exception occurs when publish OnNewSession event"));
		}
	}

	_updateExpiration(now() + _env._ttlIdleSess);

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "Commit State Changed"));
	_commitState(false); // do not send the state change for this state
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "Leave SessStateNotProvisioned"));
}

void SessStateNotProvisioned::OnTimer(const ::Ice::Current& c)
{
	if (_sess.expiration == 0)
		_updateExpiration(now() + MAX_TTL_IDLE_SESSION); // do not allow 0 expiration at this state

	SessStateBase::OnTimer(c);
}

void SessStateNotProvisioned::doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, "SessStateNotProvisioned", 1071, __MSGLOC__ "TODO: impl here");
}

::Ice::Long SessStateNotProvisioned::doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c)
{
	return _addResourceEx(type, res, c);
}

void SessStateNotProvisioned::doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{
	_removeResource(type, c);
}

void SessStateNotProvisioned::doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c)
{
	_attachPurchase(purchase, c);
}

// -----------------------------
// class SessStateProvisioned
// -----------------------------
void SessStateProvisioned::enter(void)
{
	SessStateChangeGuard sscg(_sess, ::TianShanIce::stProvisioned);

	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateProvisioned, "enter()"));
	
	// only allowed to be entered from the state of stNotProvisioned
	switch(_sess.state)
	{
	case ::TianShanIce::stProvisioned:
		return; // do nothing

	case ::TianShanIce::stNotProvisioned:
		SessStateNotProvisioned(_env, _sess).leave(); break;
		
	case ::TianShanIce::stInService:
	case ::TianShanIce::stOutOfService:
	default:
		setLastErrorMsg("Invalid state change");
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_errLog, "SessStateProvisioned", 1081, SESSIONSTATEFMT(SessStateProvisioned, "not allowed to enter"));
	}

	_updateExpiration(now()+_env._ttlIdleSess);
	
	::TianShanIce::SRM::SessionPrx proxyThis = IdentityToObj(Session, _sess.ident);

#pragma message ( __MSGLOC__ "TODO: double check the needed resource")
#pragma message ( __MSGLOC__ "TODO: perform stProvisioned entry steps")
	
	// step 1. resolve purchase based on the request URI
	// 1.1 connect to the BusinessRouter 
	//TianShanIce::Site::BusinessRouterPrx bizPrx;
	::TianShanIce::Application::PurchasePrx purchase;

	try 
	{
		::Ice::Long  stampStart = now();

		// append the server session id as unique deliveryId
		if (_sess.resources.end() == _sess.resources.find(TianShanIce::SRM::rtServiceGroup))
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (_errLog, ", SessStateProvisioned", 1082, SESSIONSTATEFMT(SessStateProvisioned, "no ServiceGroup is specified"));

		::TianShanIce::SRM::Resource& resServiceGroup = _sess.resources[TianShanIce::SRM::rtServiceGroup];
		if (resServiceGroup.resourceData.end() == resServiceGroup.resourceData.find("deliveryId")) // do not overwrite if exists
		{
			::TianShanIce::Variant v;
			v.bRange = false; v.type = ::TianShanIce::vtStrings;
			v.strs.push_back(_sess.sessId);
			MAPSET(::TianShanIce::ValueMap, resServiceGroup.resourceData, "deliveryId", v);
		}

		//bizPrx = TianShanIce::Site::BusinessRouterPrx::checkedCast(_env._communicator->stringToProxy(std::string(SERVICE_NAME_BusinessRouter ":") + _env._bsroutEndpoint));
		//purchase = _bizPrx->resolvePurchase(proxyThis);
		TianShanIce::Site::BusinessRouterPrx bsPrx = _env.getBusinessRouterPrx();
		if(!bsPrx)
		{
			envlog(ZQ::common::Log::L_ERROR, SESSIONSTATELOGFMT(SessStateProvisioned, "Can't get BusinessRouter proxy, check the SiteAdmin endpoint"));
			setLastErrorMsg("failed to connect to BusinessRouter");
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, ", SessStateProvisioned", 1082, SESSIONSTATEFMT(SessStateProvisioned, "No bussiness router is connected"));
		}
		else
		{
			envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateProvisioned, "resolving purchase"));
			purchase = bsPrx->resolvePurchase(proxyThis);
			envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateProvisioned, "resolve purchase done"));
		}
		::Ice::Long stampResolve = now();	

		if (!purchase)
		{
			setLastErrorMsg("NULL application purchase");
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, ", SessStateProvisioned", 1083, SESSIONSTATEFMT(SessStateProvisioned, "NULL purchase associated"));
		}
		else
		{
			{
				SESSWLOCK(_sess);
				_sess.purchasePrxStr = _env._communicator->proxyToString(purchase);
			}
			//let BussinessRouter do this

//			envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateProvisioned, "create purchase successful %s"), _sess.purchasePrxStr.c_str());
//			purchase->provision();
		}
		
//		envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateProvisioned, "took %lldms to resolve purchase, and %lldms to provision"), stampResolve - stampStart, now()- stampResolve);
	}
	catch(const ::TianShanIce::BaseException& e)
	{
		//be.ice_throw();
		setLastErrorMsg("failed to resolve purchase");
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateProvisioned", 1084, SESSIONSTATEFMT(SessStateProvisioned, "exception cought when resolve purchase on " SERVICE_NAME_BusinessRouter " at %s: %s"), _env._BussinessRouterEndpoint.c_str(), e.message.c_str());
	}
	catch(const ::Ice::Exception& e)
	{
		setLastErrorMsg("failed to resolve purchase");
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateProvisioned", 1085, SESSIONSTATELOGFMT(SessStateProvisioned, "exception caught when resolve purchase on " SERVICE_NAME_BusinessRouter " at %s: %s"), _env._BussinessRouterEndpoint.c_str(), e.ice_name().c_str());
	}
	catch(...)
	{
		setLastErrorMsg("failed to resolve purchase");
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateProvisioned", 1086, SESSIONSTATEFMT(SessStateProvisioned, "unkown exception cought when resolve purchase on " SERVICE_NAME_BusinessRouter " at %s"), _env._endpoint.c_str());
	}
	
	_updateExpiration(now() + _env._ttlIdleSess);

	_commitState();
}

void SessStateProvisioned::OnTimer(const ::Ice::Current& c)
{
	if (_sess.expiration == 0)
		_updateExpiration(now() + MAX_TTL_IDLE_SESSION); // do not allow 0 expiration at this state

	SessStateBase::OnTimer(c);
}

void SessStateProvisioned::leave(void)
{
#pragma message ( __MSGLOC__ "TODO: cleanup the resource allocated for this state")
}


::Ice::Long SessStateProvisioned::doAddResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c)
{
	return _addResourceEx(type, res, c);
}

void SessStateProvisioned::doRemoveResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{
	_removeResource(type, c);
}

void SessStateProvisioned::doNegotiateResources(::TianShanIce::State state, const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQTianShan::_IceThrow <TianShanIce::NotImplemented> (envlog, "SessStateProvisioned", 1091, __MSGLOC__ "TODO: impl here");
}

void SessStateProvisioned::doAttachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c)
{
	_attachPurchase(purchase, c);
}

// -----------------------------
// class SessStateInService
// -----------------------------
typedef struct _TicketEvalNode
{
	int score, cost;
	std::string ticketId;
	::TianShanIce::Transport::PathTicketPrx ticket;
} TicketEvalNode;
typedef std::vector< TicketEvalNode > TicketEval;

static int noNewLine (int c)
{
	if ('\n' == c || '\r' == c || '\t'==c)
		return ' ';
	else
		return c;
}
#ifdef _DEBUG
void printit(const char* p)
{
	printf("%s", p);
}
#endif

void SessStateInService::enter(void)
{	
	SessStateChangeGuard sscg(_sess, ::TianShanIce::stInService);

	std::string	strSessID = _sess.ident.name;

	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "Session[%s] enter()"), strSessID.c_str() );

	// only allowed to be entered from the state of Provisioned
	switch(_sess.state)
	{
	case ::TianShanIce::stInService:
		return; // do nothing

	case ::TianShanIce::stProvisioned:
		SessStateProvisioned(_env, _sess).leave(); break;

	case ::TianShanIce::stNotProvisioned:
	case ::TianShanIce::stOutOfService:
	default:
		setLastErrorMsg("Invalid state change");
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_errLog, "SessStateInService", 1101, SESSIONSTATEFMT(SessStateInService, "not allowed to enter"));
	}

	_updateExpiration(now()+_env._ttlIdleSess); // disable timer

#pragma message ( __MSGLOC__ "TODO: double check the needed resource")

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessStateInService, "Session[%s] connect purchase proxy [%s]"), 
								strSessID.c_str(), _sess.purchasePrxStr.c_str());
	::TianShanIce::Application::PurchasePrx purchase = ::TianShanIce::Application::PurchasePrx::uncheckedCast(_env._communicator->stringToProxy(_sess.purchasePrxStr));
	if (!purchase)
	{
		setLastErrorMsg("purchase is not ready");
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_errLog, "SessStateInService", 1102, SESSIONSTATEFMT(SessStateInService, "purchase is not ready"));
	}
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SessStateInService, "Session[%s] connect purchase proxy [%s] successfully"), 
								strSessID.c_str(), _sess.purchasePrxStr.c_str());

	::TianShanIce::SRM::SessionPrx proxyThis = IdentityToObj(Session, _sess.ident);

	// step 1. connect to the path manager
#if 0
	try 
	{
		// validate the connection to PathManager
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessStateInService, "Session[%s] connect to PathManager [%s]"), 
											strSessID.c_str(), _env._proxyPathMgr.c_str()	);

		if (!_env._pathmgr)
		{
//#ifndef EMBED_PATHSVC
			_env._pathmgr = ::TianShanIce::Transport::PathManagerPrx::checkedCast(_env._communicator->stringToProxy(_env._proxyPathMgr));
//#else
//			_env._pathmgr=::TianShanIce::Transport::PathManagerPrx::checkedCast(_env._communicator->stringToProxy(_env._proxyPathMgr)->ice_collocationOptimization(false));
//#endif // EMBED_PATHSVC
		}
		else 
		{
			_env._pathmgr->ice_ping();
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(SessStateInService, "Session[%s] connect to PathManager [%s] successfully"), 
											strSessID.c_str(), _env._proxyPathMgr.c_str()	);

	}
	catch(const ::TianShanIce::BaseException& e)
	{
		_updateExpiration(now() + _env._ttlIdleSess);
		setLastErrorMsg("failed to connect to path manager");
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (_errLog, "SessStateInService", 1103, SESSIONSTATEFMT(SessStateInService, "TianShan exception cought when check the connection to PathManager at %s: %s"), _env._proxyPathMgr.c_str(), e.message.c_str());
	}
	catch(const ::Ice::Exception& e)
	{
		_updateExpiration(now() + _env._ttlIdleSess);
		setLastErrorMsg("failed to connect to path manager");
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (_errLog, "SessStateInService", 1104, SESSIONSTATEFMT(SessStateInService, "Ice exception cought when check the connection to PathManager at %s: %s"), _env._proxyPathMgr.c_str(), e.ice_name().c_str());
	}
	catch(...)
	{
		_updateExpiration(now() + _env._ttlIdleSess);
		setLastErrorMsg("failed to connect to path manager");
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 1105, SESSIONSTATEFMT(SessStateInService, "unkown exception cought when check the connection to PathManager at %s"), _env._proxyPathMgr.c_str());
	}
#endif

	TicketEval ticketEval;
	long totalscore =0;

	::Ice::Long stampStart, stamp;
	stampStart = stamp = now();
	int selectionCount =0;

	try
	{
		// step 2 reserve the path tickets
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessStateInService, "Session[%s] reservepaths"), strSessID.c_str());
		stamp=now();
		::TianShanIce::Transport::PathTickets pathtickets = _env._pathmgr->reservePaths(0, _env._maxTickets, _env._ttlPathAllocation, proxyThis);
		
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "reserve path from path service took %lld ms, got %d ticket(s) from path service"), now()-stamp, pathtickets.size());
		if (pathtickets.size()==0)
		{
			//no appreciated ticket is commit
			setLastErrorMsg("out of path resource");
			_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
			ZQTianShan::_IceThrow <TianShanIce::ServerError>(_errLog, "SessStateInService", 1106, SESSIONSTATEFMT(SessStateInService, "No available ticket returned from PHO"));
		}

		// double check if the session has been out-of-service per external requests, because reservePaths() is a big step and may take long time
		switch(_sess.state)
		{
		case ::TianShanIce::stProvisioned:
		case ::TianShanIce::stInService:
			break; // do nothing

		case ::TianShanIce::stNotProvisioned:
		case ::TianShanIce::stOutOfService:
		default:
			for (::TianShanIce::Transport::PathTickets::iterator itTicket = pathtickets.begin(); itTicket < pathtickets.end(); itTicket++)
			{
				try
				{
					Ice::Context ctx;
					ctx["caller"]= __MSGLOC__;
					(*itTicket)->destroy(ctx);
				}
				catch(...) {}
			}

//			setLastErrorMsg("invalid state to enter InService after reserved paths");
			_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_errLog, "SessStateInService", 1111, SESSIONSTATELOGFMT(SessStateInService, "Invalid state[%s(%d)] to continue entering InService after reserving paths"), ::ZQTianShan::ObjStateStr(_sess.state), _sess.state);
		}

		stamp = now();

		// step 3.1 sum up scores to totalscore
		for (::TianShanIce::Transport::PathTickets::iterator it = pathtickets.begin();
																	it < pathtickets.end(); 
																	it ++)
		{
			try 
			{
				TicketEvalNode node;
				node.ticket = *it;
				node.cost = node.ticket->getCost();
				node.score = ::TianShanIce::Transport::MaxCost + 1 - node.cost;
				node.ticketId = node.ticket->getIdent().name;
				ticketEval.push_back(node);
				totalscore += node.score;
				envlog(Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "added ticket %s for selection"), node.ticketId.c_str());
			}
			catch (::Ice::Exception& ex)
			{
				envlog(Log::L_ERROR, SESSIONSTATELOGFMT(SessStateInService, "Get a expection is %s"), ex.ice_name().c_str());
			}
			catch(...) 
			{
				envlog(Log::L_ERROR, SESSIONSTATELOGFMT(SessStateInService, "Unexpect error when sum up toal score"));
			} // ignore any unaccessible tickets
		}
	}
	catch (const TianShanIce::BaseException& e)
	{
		setLastErrorMsg("failed to reserve path resource", true);
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(_errLog, e.category.c_str(), e.errorCode, e.message.c_str());
	}
	catch(const ::Ice::Exception& e)
	{
		setLastErrorMsg("failed to reserve path resource", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 1108, SESSIONSTATEFMT(SessStateInService, "failed to reserve path from PathManager at %s: exception[%s]"), _env._proxyPathMgr.c_str(), e.ice_name().c_str());
	}
	catch(...)
	{
		setLastErrorMsg("failed to reserve path resource", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 1109, SESSIONSTATEFMT(SessStateInService, "unkown exception caught when reserve path from PathManager at %s"), _env._proxyPathMgr.c_str());
	}

	// step 3 select tickets
	envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateInService, "potential %d path ticket(s) took %lldms, do selection"), ticketEval.size(), stamp - stampStart);
	stampStart = stamp;

	// step 3.2 generate a random drop;
	static bool bSeeded = false;
	if (!bSeeded)
	{
		::srand((unsigned) ::time(NULL));
		bSeeded = true;
	}

	TicketEval::iterator itSelectedEval = ticketEval.begin();
	::TianShanIce::Streamer::StreamPrx stream;
	std::string pathTicketIdSelected;
		
	std::vector<std::string>	vecErrMsg;
	while ( (!_sess.ticket) && (ticketEval.end() > itSelectedEval) )
	{
		// step 3.3 select the ticket
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "enter() select a ticket from potential %d tickets"), ticketEval.size());
		
		long drop = totalscore * ::rand() / RAND_MAX;
		for (; itSelectedEval < ticketEval.end(); itSelectedEval ++)
		{
//			long aaa = itSelectedEval->score;
			if (drop > itSelectedEval->score)
			{
				drop -= itSelectedEval->score;
				continue;
			}
			
			break;
		}
		
		if (ticketEval.end() == itSelectedEval)
			break;
		
		// we have picked a ticket here
		std::string pathTicketId = itSelectedEval->ticketId;
		selectionCount ++;
		::TianShanIce::Transport::Streamer streamerInfo;
		::TianShanIce::SRM::ResourceMap ticketResources;
		
		try 
		{
			envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "enter() try the PathTicket[%s] with evaluation cost %d"), pathTicketId.c_str(), itSelectedEval->cost);

// #ifdef _DEBUG
//			::ZQTianShan::dumpResourceMap(_sess.resources, "pre-narrow", dumpLine, &_env);				
// #endif //_DEBUG

			// 1. narrow the ticket
//#ifdef EMBED_PATHSVC
//			::TianShanIce::Transport::PathTicketPrx::checkedCast(itSelectedEval->ticket->ice_collocationOptimization(false))->narrow(proxyThis);
//#else
			itSelectedEval->ticket->narrow(proxyThis);
//#endif//EMBED_PATHSVC

#pragma message(__MSGLOC__"how many millisecond should be used to renew the ticket")
			itSelectedEval->ticket->renew(600* 1000); //TODO: const or configuration?

			::TianShanIce::Transport::StreamLinkPrx link = itSelectedEval->ticket->getStreamLink();
			streamerInfo = link->getStreamerInfo();
		}
		catch(const ::TianShanIce::BaseException& e)
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "%s caught when utilizing PathTicket[%s]: %s"), e.ice_name().c_str(), pathTicketId.c_str(), e.message.c_str());
		}
		catch(const ::Ice::TimeoutException&)
		{
			envlog(ZQ::common::Log::L_ERROR, SESSIONSTATELOGFMT(SessStateInService, "timeout at utilizing PathTicket[%s]: PHO might hang"), pathTicketId.c_str());
		}
		catch(const ::Ice::Exception& e)
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "%s caught when utilizing PathTicket[%s]"), e.ice_name().c_str(), pathTicketId.c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "exception caught when utilizing PathTicket[%s]"), pathTicketId.c_str());
		}

		::Ice::Long tmpStamp = stamp;
		
		try 
		{
			if (!streamerInfo.ifep.empty() && !streamerInfo.netId.empty())
			{
				// okay, now we have addressed the streamer, connect to it
				envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "connecting to StreameSvc[%s]"), streamerInfo.ifep.c_str());
				::TianShanIce::Streamer::StreamServicePrx streamer = ::TianShanIce::Streamer::StreamServicePrx::uncheckedCast(_env._communicator->stringToProxy(streamerInfo.ifep));

				envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "creating stream instance via PathTicket[%s] on Streamer[%s]"), pathTicketId.c_str(), streamerInfo.netId.c_str());

				int64 dwCreateStreamStart = SYS::getTickCount();
				if (streamer)
					stream = streamer->createStream(itSelectedEval->ticket);

				if (!stream)
				{
					envlog(Log::L_ERROR, SESSIONSTATELOGFMT(SessStateInService, "can't create stream on StreamSvc[%s]"), 
						streamerInfo.ifep.c_str());				
				}
				else
				{
					envlog(Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "create stream %s on %s with time count=[%u]"), 
						stream->getIdent().name.c_str(), streamerInfo.ifep.c_str(), SYS::getTickCount()-dwCreateStreamStart);
				}
				//Ice::Identity idTicket=itSelectedEval->ticket->getIdent();
				//renew the ticket here
#pragma message(__MSGLOC__"TODO:Should I renew the ticket here with _ttlIdleSess")
				itSelectedEval->ticket->renew(_env._ttlIdleSess);
			}
		}
		catch(const ::TianShanIce::BaseException& e)
		{
			itSelectedEval->ticket->setPenalty(gWeiwooServiceConfig.lMaxPenaltyValue);
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "exception caught when connect to Streamer[%s]: %s"), streamerInfo.ifep.c_str(), e.message.c_str());
			char szBuf[2048];
			szBuf[2047]='\0';
			snprintf(szBuf, 2047, "Can't create stream with ticket %s and streamer %s and exception:%s", 
				pathTicketId.c_str(), 
				streamerInfo.ifep.c_str(), 
				e.message.c_str());
			vecErrMsg.push_back(szBuf);
			//e.ice_throw();
		}
		catch(const ::Ice::Exception& e)
		{
			itSelectedEval->ticket->setPenalty(gWeiwooServiceConfig.lMaxPenaltyValue);
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "exception caught when connect to Streamer[%s]: %s"), streamerInfo.ifep.c_str(), e.ice_name().c_str());
			char	szBuf[2048];
			szBuf[2047]='\0';
			snprintf(szBuf, 2047, "Can't create stream with ticket %s and streamer %s and exception:%s", 
				pathTicketId.c_str(), 
				streamerInfo.ifep.c_str(), 
				e.ice_name().c_str());
			vecErrMsg.push_back(szBuf);
		}
		catch(...)
		{
			itSelectedEval->ticket->setPenalty(gWeiwooServiceConfig.lMaxPenaltyValue);
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "exception caught when connect to Streamer[%s]"), streamerInfo.ifep.c_str());
			char	szBuf[2048];
			szBuf[2047]='\0';
			snprintf(szBuf, 2047, "caught exception when create stream with ticket %s and streamer %s", 
				pathTicketId.c_str(), streamerInfo.ifep.c_str());
			vecErrMsg.push_back(szBuf);
		}

		stamp = now();
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "took %dms to try PathTicket[%s]"), (int)(stamp - tmpStamp), pathTicketId.c_str());

		try
		{
			if (stream)
			{//
				//dump ticket resource map into session resource map
				TianShanIce::SRM::ResourceMap resMap=itSelectedEval->ticket->getResources();

				{
					SESSWLOCK(_sess);
					TianShanIce::SRM::ResourceMap::const_iterator itMap=resMap.begin();
					for(;itMap!=resMap.end();itMap++)
						MAPSET(TianShanIce::SRM::ResourceMap, _sess.resources, itMap->first, itMap->second);
					
					_sess.ticket = itSelectedEval->ticket;
					_sess.streamPrxStr = _env._communicator->proxyToString(stream);
					std::transform(_sess.streamPrxStr.begin(), _sess.streamPrxStr.end(), 
						_sess.streamPrxStr.begin(), (int(*)(int)) noNewLine);
				}

				envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateInService, "stream instance with ticket[%s] created: %s"), 
											itSelectedEval->ticketId.c_str(), _sess.streamPrxStr.c_str());

//#ifdef EMBED_PATHSVC
//				::TianShanIce::Transport::PathTicketPrx::checkedCast(_sess.ticket->ice_collocationOptimization(false))->commit(proxyThis);
//#else
				_sess.ticket->commit(proxyThis);
				pathTicketIdSelected = itSelectedEval->ticketId;
//#endif//EMBED_PATHSVC

				//_sess.streamobj->allotAccreditPathTicket(itSelectedEval->ticket);

				//successfully commit stream here, so delete the PathTicket iterator
				ticketEval.erase(itSelectedEval);
				break; // if succeeded no more try on the other path tickets, quit while()
			}
		}
		catch( const TianShanIce::BaseException& ex )
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "exception caught [%s] when apply the PathTicket[%s] on to stream instance"), 
				ex.message.c_str(), pathTicketId.c_str());
		}
		catch( const Ice::Exception& ex )
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "exception caught [%s] when apply the PathTicket[%s] on to stream instance"), 
				ex.ice_name().c_str(), pathTicketId.c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateInService, "exception caught when apply the PathTicket[%s] on to stream instance"), pathTicketId.c_str());
		}
		
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "failed to employ PathTicket[%s], destroy and remove it from selection"), 	pathTicketId.c_str());

		try 
		{
			Ice::Context ctx;
			ctx["caller"]=__MSGLOC__;
			itSelectedEval->ticket->destroy(ctx);
		}
		catch(...) {}
		
		totalscore -= itSelectedEval->score;
		ticketEval.erase(itSelectedEval);
		itSelectedEval = ticketEval.begin();
	} // while

	stamp = now();

	envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateInService, "completed selection, picked[%s], tried %d times, took %lldms, destroying all unused %d PathTickets"), pathTicketIdSelected.c_str(), selectionCount, stamp - stampStart, ticketEval.size());
	for (itSelectedEval = ticketEval.begin(); itSelectedEval < ticketEval.end(); itSelectedEval ++)
	{
		try
		{
			Ice::Identity idTicket=itSelectedEval->ticket->getIdent();
			envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "destroy unused ticket %s"), idTicket.name.c_str());
			Ice::Context ctx;
			ctx["caller"]=__MSGLOC__;
			itSelectedEval->ticket->destroy(ctx);
		}
		catch(...) {}
	}

	ticketEval.clear();

	// step 4. render the stream
	// 4.1 validate the stream
#pragma message ( __MSGLOC__ "TODO: should we go directly to OutService if InService failed ?")
	if (!stream)
	{
		_updateExpiration(now() + _env._ttlIdleSess);

		if(vecErrMsg.size()>0)
		{
			std::string errMsg="failed to prepare stream instance:";
			for(int i=0;i<(int)vecErrMsg.size();i++)
			{
				errMsg+="\n";
				errMsg+=vecErrMsg[i];
			}
			envlog(ZQ::common::Log::L_ERROR, SESSIONSTATELOGFMT(SessStateInService, "failed to prepare stream instance"));
			setLastErrorMsg("failed to prepare stream instance");
			_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
			ZQTianShan::_IceThrow< TianShanIce::ServerError>(_errLog, "SessStateInService", 2101, errMsg.c_str());
		}
		//vecErrMsg		
		setLastErrorMsg("failed to prepare stream instance", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 2102, SESSIONSTATELOGFMT(SessStateInService, "failed to prepare stream instance"));
	}

	try
	{
		envlog(ZQ::common::Log::L_DEBUG, 
				SESSIONSTATELOGFMT(SessStateInService, "rendering the purchase onto stream: %s"), 
				_sess.streamPrxStr.c_str());
		stampStart = now();
		
		purchase->render(stream, proxyThis); // let the exception go to up layers
		//ok, now ticket is available for service		
		envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateInService, "render completed, %lldms: %s"), 
				now() - stampStart, _sess.streamPrxStr.c_str());
	}
	catch(const ::TianShanIce::BaseException& be)
	{
		setLastErrorMsg("failed to render stream", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		be.ice_throw();
	}
	catch(const ::Ice::Exception& e)
	{
		setLastErrorMsg("failed to render stream", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 2103, 
				SESSIONSTATELOGFMT(SessStateInService, "exception cought when render the purchase[%s] and stream [%s] and exception is[%s]"), 
				_sess.purchasePrxStr.c_str(), _sess.streamPrxStr.c_str(), e.ice_name().c_str());
	}
	catch(...)
	{
		setLastErrorMsg("failed to render stream", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 2104, SESSIONSTATELOGFMT(SessStateInService, "unkown exception cought when  render the purchase[%s] and stream [%s]"), 
				_sess.purchasePrxStr.c_str(), _sess.streamPrxStr.c_str());
	}

	try
	{
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "commiting stream[%s]"), _sess.streamPrxStr.c_str());
		stream->commit( );
		envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateInService, "committed stream[%s] successfully"), _sess.streamPrxStr.c_str());

		::TianShanIce::Variant v;
		v.type = ::TianShanIce::vtStrings;
		v.bRange =false;
		v.strs.push_back(pathTicketIdSelected);
		MAPSET(TianShanIce::ValueMap, _sess.privdata, SYS_PROP(pathTicket), v);

	}
	catch(const ::TianShanIce::BaseException& be)
	{
		setLastErrorMsg("failed to commit stream", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 2105, SESSIONSTATELOGFMT(SessStateInService, "failed to commit the stream[%s] due to %s[%s]"), 
				_sess.streamPrxStr.c_str(), be.ice_name().c_str(), be.message.c_str() );
		be.ice_throw();
	}
	catch(const ::Ice::Exception& e)
	{
		setLastErrorMsg("failed to commit stream", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 2105, SESSIONSTATELOGFMT(SessStateInService, "failed to commit the stream[%s] due to %s"), 
				_sess.streamPrxStr.c_str(), e.ice_name().c_str());
	}
	catch(...)
	{
		setLastErrorMsg("failed to commit stream", true);
		_updateExpiration(now() + TIMEOUT_YIELD_AFTER_ERROR_OCCURED);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_errLog, "SessStateInService", 2106, SESSIONSTATELOGFMT(SessStateInService, "failed to commit the stream[%s] due to exception"), 
				_sess.streamPrxStr.c_str());
	}

	::Ice::Long stampNow = now();

	// read if the privateData[overrideTimeout] ever been set
	_sess.overrideExp =0;
	::Ice::Int nextTimeout = _env._ttlIdleSess;
	::TianShanIce::ValueMap::const_iterator iter = _sess.privdata.find(SYS_PROP(overrideTimeout));
	if (_sess.privdata.end() != iter)
	{
		const TianShanIce::Variant& overrideTimeout = (*iter).second;
		if (overrideTimeout.type == TianShanIce::vtInts && overrideTimeout.ints.size() > 0) 
		{
			::Ice::Int iOverrideTimeout = overrideTimeout.ints[0];
			if (iOverrideTimeout>0 )
			{
				_sess.overrideExp = stampNow + iOverrideTimeout;
				char tmpBuf[32] ="n/a";
				TimeToUTC(_sess.overrideExp, tmpBuf, sizeof(tmpBuf));
				envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateInService, "enter() read privateData[overrideTimeout]=%d, set overrideExp[%s]"), iOverrideTimeout, tmpBuf);
				if (nextTimeout > iOverrideTimeout)
					nextTimeout = iOverrideTimeout;
			}
		}
	}

	// double check if the session has been out-of-service per external requests, because create and render stream are is a big steps and may take long time
	switch(_sess.state)
	{
	case ::TianShanIce::stProvisioned:
	case ::TianShanIce::stInService:
		break; // do nothing

	case ::TianShanIce::stNotProvisioned:
	case ::TianShanIce::stOutOfService:
	default:
		try
		{
			Ice::Context ctx;
			ctx["caller"]= __MSGLOC__;
			_sess.ticket->destroy(ctx);
			
			{
				SESSWLOCK(_sess);
				_sess.ticket = NULL;
			}

			if( stream)
				stream->destroy();
			{
				SESSWLOCK(_sess);
				_sess.streamPrxStr = "";
			}
		}
		catch(...) {}

		//	setLastErrorMsg("invalid state to continue entering InService after reserving paths");
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_errLog, "SessStateInService", 1112, SESSIONSTATELOGFMT(SessStateInService, "Invalid state[%s(%d)] to continue entering InService, withdrew the stream and resource"), ::ZQTianShan::ObjStateStr(_sess.state), _sess.state);
	}

	// note: keep _sess.expiration =0, and let external adjust this thru Session::renew()
	//I think stream must renew the ticket after createStream is invoked
	_updateExpiration(stampNow + nextTimeout);

	_commitState();
	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateInService, "enter() finished"));
}

void SessStateInService::OnTimer(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: renew PollingTimer and PathTicket when this is called")
	SessStateBase::OnTimer(c);
}

void SessStateInService::leave(void)
{
#pragma message ( __MSGLOC__ "TODO: cleanup all the resource of InService")
}

void SessStateOutOfService::failedToDestroyInstance()
{
	Ice::Long lastDestroyTime = 0;

	{
		SESSRLOCK(_sess);
		ZQTianShan::Util::getValueMapDataWithDefault( _sess.privdata, "__last remove time__", 0, lastDestroyTime );
	}

	if( lastDestroyTime <= 0 )
	{
		{
			SESSWLOCK(_sess);
			ZQTianShan::Util::updateValueMapData( _sess.privdata, "__last remove time__", ZQTianShan::now() );
		}

		_updateExpiration( now() + _env._ttlIdleSess);
		envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateOutOfService, "destory failed, retry to destroy stream in %ld ms"), _env._ttlIdleSess );
		// ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, "SessionOutOfService", 0, SESSIONSTATELOGFMT(SessStateOutOfService, "failed to destroy stream, retry in %dms"), _env._ttlIdleSess );
	}
	else
	{
		if ( (now() - lastDestroyTime) < 60*60*1000 )
		{			
			Ice::Long nextDestory = now() + _env._ttlIdleSess;
			if ( _sess.overrideExp <=0 || nextDestory <= _sess.overrideExp)					
			{
				//trigger exception only if timerTarget > override_expiration 
				// envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateOutOfService, "destroy stream in %ldms, lastTime[%lld]"), _env._ttlIdleSess, lastDestroyTime );
				_updateExpiration( nextDestory );
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, "SessionOutOfService", 0, SESSIONSTATELOGFMT(SessStateOutOfService, "failed to destroy stream, retry in %dms"), _env._ttlIdleSess );
			}
		}
	}
}
// -----------------------------
// class SessStateOutOfService
// -----------------------------
void SessStateOutOfService::enter(void)
{
	SessStateChangeGuard sscg(_sess, ::TianShanIce::stOutOfService, false);
//	if  (!canEnterState(TianShanIce::stOutOfService, sscg))
//		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "SessStateOutOfService", 400, SESSIONSTATEFMT(SessStateOutOfService, "operation double submitted"));

	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "enter() starts cleaning weiwoo session"));

	switch(_sess.state)
	{
	case ::TianShanIce::stOutOfService:
		return; // do nothing

	case ::TianShanIce::stNotProvisioned:
		SessStateNotProvisioned(_env, _sess).leave(); break;

	case ::TianShanIce::stProvisioned:
		SessStateProvisioned(_env, _sess).leave(); break;

	case ::TianShanIce::stInService:
		SessStateInService(_env, _sess).leave(); break;

	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "SessStateOutOfService", 1111, SESSIONSTATEFMT(SessStateOutOfService, "not allowed"));
	}

	_updateExpiration(now()+_env._ttlIdleSess); // disable timer

	// step 1. call to fixup the reasons
	_fixupReasons();

	// step 2. call to clean up each attachement
	bool bAwait = _cleanAttachmentIfAwait();

	// step 3. commit state change
	_updateExpiration(now() + (bAwait ? _env._ttlOutOfServiceSess :100));

	_commitState();
}

void SessStateOutOfService::_fixupReasons(void)
{
	// step 2. determin the reasons
	std::string oldTeardownReason, oldTerminateReason;

	// step a. read the reasons from private data
	{
		SESSRLOCK(_sess);
		TianShanIce::ValueMap::iterator itVal = _sess.privdata.find(SYS_PROP(teardownReason));
		if (_sess.privdata.end() != itVal && itVal->second.type == TianShanIce::vtStrings && itVal->second.strs.size()>0)
			_strTeardownReason = oldTeardownReason = itVal->second.strs[0];

		itVal = _sess.privdata.find(SYS_PROP(terminateReason));
		if (_sess.privdata.end() != itVal && itVal->second.type == TianShanIce::vtStrings && itVal->second.strs.size()>0)
			_strTerminateReason = oldTerminateReason = itVal->second.strs[0];
	}

	// step b, adjust and merge the reasons
	if (!_strTeardownReason.empty())
	{
		if (atol(_strTeardownReason.c_str()) < 100000) // the teardown reason string doesn't carry a code, insert one
			_strTeardownReason = "100000 " + _strTeardownReason;
	}
	else if (gWeiwooServiceConfig.lMixTeardownReasonAndTerminateReason >= 1) 
		_strTeardownReason = _strTerminateReason;

	if (_strTerminateReason.empty() && !_strTeardownReason.empty())
	{
		if (gWeiwooServiceConfig.lMixTeardownReasonAndTerminateReason >= 1)
			_strTerminateReason = _strTeardownReason;
		// ticket#16644 to leave empty strTerminateReason
		//else
		//	_strTerminateReason = "100000 User Requested TEARDOWN";
	}

	// step c, flush to private data if adjusted
	if (0 != _strTeardownReason.compare(oldTeardownReason) || 0 != _strTerminateReason.compare(oldTerminateReason))
	{
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "reasons fixed-up: TM[%s]=>[%s], TD[%s]=>[%s]"), 
			oldTerminateReason.c_str(), _strTerminateReason.c_str(), oldTeardownReason.c_str(), _strTeardownReason.c_str());	

		::TianShanIce::Variant vReason;
		vReason.bRange = false;
		vReason.type = TianShanIce::vtStrings;
		vReason.strs.push_back(_strTerminateReason);

		SESSWLOCK(_sess);
		MAPSET(TianShanIce::ValueMap, _sess.privdata, SYS_PROP(terminateReason), vReason);

		vReason.strs.clear();
		vReason.strs.push_back(_strTeardownReason);
		MAPSET(TianShanIce::ValueMap, _sess.privdata, SYS_PROP(teardownReason), vReason);
	}
}

#define TAG_STAMP_LAST_DETACH   "__stampLastDetach__"
bool SessStateOutOfService::_cleanAttachmentIfAwait(bool bEnforce)
{
	Ice::Long stampLastDetach = 0;

	{
		SESSRLOCK(_sess);
		ZQTianShan::Util::getValueMapDataWithDefault( _sess.privdata, TAG_STAMP_LAST_DETACH, 0, stampLastDetach);
	}

	if (stampLastDetach <= 0 )
	{
		stampLastDetach = ZQTianShan::now();
		SESSWLOCK(_sess);
		ZQTianShan::Util::updateValueMapData( _sess.privdata, TAG_STAMP_LAST_DETACH, stampLastDetach);
	}

	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "_cleanAttachments() double checking the attachments to free, stampLastDetach[%lld] enforce(%c)"), stampLastDetach, bEnforce?'T':'F');
	::TianShanIce::SRM::SessionPrx proxyThis = IdentityToObj(Session, _sess.ident);
	
	bool bSucc = true;
	std::string attachments;
	do {
		TianShanIce::Properties propDetachPurchase;

		// step 1. destroy stream if it is still running
		try
		{
			::TianShanIce::Streamer::StreamPrx stream = NULL;
			if( !_sess.streamPrxStr.empty() )
				stream =  ::TianShanIce::Streamer::StreamPrx::uncheckedCast(_env._communicator->stringToProxy(_sess.streamPrxStr));

			if (stream)
			{
				int64 stampStart = ZQ::common::now();
				std::string strProxy=_env._communicator->proxyToString(stream);
				envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "destroying stream[%s]"), strProxy.c_str());			
				Ice::Context ctx;
				ctx["caller"] = std::string("WeiwooService:") + _env._communicator->proxyToString(proxyThis);			
				ctx["reason"] = _strTeardownReason;

				::TianShanIce::Properties feedback;
				try { 
					// enh#20635 to pass FinalNPT to AppSvc
					// try to take destroy2(), switch to take the old destroy if failed
					stream->destroy2(feedback, ctx);

					// copy the feedback into purchase props
					for (::TianShanIce::Properties::iterator itFB = feedback.begin(); itFB != feedback.end(); itFB++)
					{
						// add the reason into the detach.prop
						MAPSET(TianShanIce::Properties, propDetachPurchase, itFB->first, itFB->second);

						// add the feedback into session's private data
						// ::TianShanIce::Variant v;
						// v.bRange = false;
						// v.type = TianShanIce::vtStrings;
						// v.strs.push_back(itFB->second);
						// MAPSET(TianShanIce::ValueMap, _sess.privdata, itFB->first, v);
					}

					envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateOutOfService, "_cleanAttachmentIfAwait() stream[%s] succeeded, took %dmsec"), strProxy.c_str(), (int)(ZQ::common::now() - stampStart));			
				}
				catch(const ::Ice::OperationNotExistException&)
				{
					stream->destroy(ctx);
					envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateOutOfService, "_cleanAttachmentIfAwait() stream[%s] succeeded, took %dmsec"), strProxy.c_str(), (int)(ZQ::common::now() - stampStart));			
				}
				// do NOT catch any other exceptions here
			}

			attachments += "stream, ";
		}
		catch (TianShanIce::BaseException& ex)
		{
			bSucc = false;
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception[%s] when destroying stream: %s"), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch( const Ice::ObjectNotExistException&)	{ }
		catch( const Ice::ObjectNotFoundException&)	{ }
		catch (Ice::Exception& ex)
		{
			bSucc = false;
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception[%s] when destroying stream"), ex.ice_name().c_str());
		}
		catch(...)
		{	
			bSucc = false;
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception when destroying stream"));
		}

		if (!bSucc && !bEnforce)
			break;
		else
		{
			SESSWLOCK(_sess);
			_sess.streamPrxStr = "";
		}

		// step 2. free ticket if it is still allocated
		try
		{
			::TianShanIce::Transport::PathTicketPrx ticket = NULL;		
			{
				SESSRLOCK(_sess);
				ticket = _sess.ticket;	
			}

			if(ticket)
			{
				int64 stampStart = ZQ::common::now();
				Ice::Identity idTicket=ticket->getIdent();
				envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "destroying PathTicket[%s]"), 	idTicket.name.c_str());
				if (ticket)
				{
					Ice::Context ctx;
					ctx["caller"]=__MSGLOC__;
					ticket->destroy(ctx);
					ticket = NULL;				
				}

				envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateOutOfService, "destroyed PathTicket[%s], took %dmsec"), 	idTicket.name.c_str(), (int)(ZQ::common::now() - stampStart));
			}

			attachments += "pticket, ";
		}
		catch (TianShanIce::BaseException& ex)
		{
			bSucc = false;
			envlog(ZQ::common::Log::L_ERROR, SESSIONSTATELOGFMT(SessStateOutOfService, "caught [%s] when destroying PathTicket: %s"), 
				ex.ice_name().c_str(), ex.message.c_str());
		}
		catch( const Ice::ObjectNotExistException&) {}
		catch( const Ice::ObjectNotFoundException&) {}
		catch (::Ice::Exception& ex)
		{
			bSucc = false;
			envlog(ZQ::common::Log::L_ERROR, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception[%s] when destroying PathTicket"), ex.ice_name().c_str());	
		}
		catch(...)
		{	
			bSucc = false;
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception when destroying PathTicket"));
		}

		if (!bSucc && !bEnforce)
			break;
		else
		{
			SESSWLOCK(_sess);
			_sess.ticket = NULL;
		}

		// step 3. destroy purchase if it is still bound
		try
		{
			envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "detaching puchase[%s]"), _sess.purchasePrxStr.c_str());
			if (! _sess.purchasePrxStr.empty() ) 
			{		
				::TianShanIce::Application::PurchasePrx purchase = ::TianShanIce::Application::PurchasePrx::uncheckedCast(_env._communicator->stringToProxy(_sess.purchasePrxStr));
				if (!purchase)
					envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "failed to touch purchase[%s]"), _sess.purchasePrxStr.c_str());
				else
				{
					// add the reason into the detach.prop
					MAPSET(TianShanIce::Properties, propDetachPurchase, SYS_PROP(terminateReason), _strTerminateReason);
					MAPSET(TianShanIce::Properties, propDetachPurchase, SYS_PROP(teardownReason), _strTeardownReason);
#if ICE_INT_VERSION / 100 >= 306
		Purchase_detachCBPtr purdetCbPtr = new Purchase_detachCB();
		Ice::CallbackPtr detachCB = Ice::newCallback(purdetCbPtr, &Purchase_detachCB::detach);
		purchase->begin_detach(_sess.ident.name, propDetachPurchase,detachCB);
#else	
					purchase->detach_async( new AMI_Purchase_detachImpl(), _sess.ident.name, propDetachPurchase);
#endif
					purchase = NULL;
				}
			}
			attachments += "purchase, ";
		}
		catch (TianShanIce::BaseException& ex)
		{
			bSucc = false;
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception[%s] when destroying purchase: %s"), 
				ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			bSucc = false;
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception[%s] when destroying purchase"), 
				ex.ice_name().c_str());
		}
		catch(...)
		{	
			bSucc = false;
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught exception when destroying purchase"));
		}

		if (!bSucc && !bEnforce)
			break;
		else
		{
			SESSWLOCK(_sess);
			_sess.purchasePrxStr = "";
		}

		// step 4. transfer liveTxn into YTDTxn
#pragma message(__MSGLOC__"TODO : transfer liveTxn into YTDTxn")
		try
		{
			//_env.getTxnServicePrx()->postYTD(_sess.ident.name);
			attachments += "txn";
		}
		catch (TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught [%s] when postYTD:%s"), 
				ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "caught [%s] when postYTD"), 
				ex.ice_name().c_str());
		}
		catch (...) 
		{
			envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateBase, "exception occurs when post YTD"));
		}

	} while (0);

	if (bSucc && !bEnforce)
	{
		envlog(ZQ::common::Log::L_INFO, SESSIONSTATELOGFMT(SessStateOutOfService, "no more attachments, cleaned: %s"), attachments.c_str());
		return false; // no attachemenet await
	}

	//if (stampLastDetach < now() - _env._ttlOutOfServiceSess *3)
	//{
	//	envlog(ZQ::common::Log::L_ERROR, SESSIONSTATELOGFMT(SessStateOutOfService, "timeout at retrying detach, cleaned: %s"), attachments.c_str());
	//	return false;
	//}

	envlog(ZQ::common::Log::L_WARNING, SESSIONSTATELOGFMT(SessStateOutOfService, "detaching incompleted, retry needed, cleaned: %s"), attachments.c_str());
	return true; // retry needed
}

void SessStateOutOfService::OnTimer(const ::Ice::Current& c)
{
	Ice::Long nextWait;
	int64 stampNow = now();
	::TianShanIce::Variant vcDestroys;
	{
		SESSRLOCK(_sess);
		nextWait  = max(_sess.expiration, _sess.overrideExp) - stampNow;
		::TianShanIce::ValueMap::const_iterator it = _sess.privdata.find(SYS_PROP(cDestroys));
		if (_sess.privdata.end() != it)
			vcDestroys = it->second;
	}

	bool bEnforce = (nextWait <=0);
	bool bInCompleted = _cleanAttachmentIfAwait(bEnforce);

	if (bInCompleted && !bEnforce)
	{
		vcDestroys.bRange=false;
		vcDestroys.type = TianShanIce::vtInts;
		int cDestorys = (vcDestroys.ints.size() >0) ? vcDestroys.ints[0] :0;
		if (cDestorys <0)
			cDestorys =0;
		vcDestroys.ints.clear();
		vcDestroys.ints.push_back(cDestorys+1);
		{
			SESSWLOCK(_sess);
			MAPSET(::TianShanIce::ValueMap, _sess.privdata, SYS_PROP(cDestroys), vcDestroys);
		}

		static const long intervals[] = { 10, 60, 300, 600 };
		if (cDestorys >= (sizeof(intervals)/sizeof(intervals[0])-1))
			cDestorys = (sizeof(intervals)/sizeof(intervals[0])-1);

		char buf[64];
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "cleaning attachment incompleted or expiration[%s] not yet reached, nextWait[%d] retry in %dmsec"), TimeToUTC(_sess.expiration, buf, sizeof(buf)-2), (int)nextWait, (int)(intervals[cDestorys]*1000));
		_updateExpiration(stampNow + intervals[cDestorys]*1000);
		return;
	}

	if (!bInCompleted || bEnforce)
	{
		envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "destroy due to timeout (%d msec) in the stOutOfService"), _env._ttlOutOfServiceSess);
		_destroySess(c.id);
	}
	else SessStateBase::OnTimer(c);
}

void SessStateOutOfService::doDestroy(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, SESSIONSTATELOGFMT(SessStateOutOfService, "doDestroy()"));
	_destroySess(c.id);
}

}} // namespace

