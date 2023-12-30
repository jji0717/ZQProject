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
// Ident : $Id: SessionImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionImpl.cpp $
// 
// 11    3/03/16 5:34p Hui.shao
// 
// 10    3/03/16 5:31p Hui.shao
// 
// 9     2/25/16 4:50p Hui.shao
// added rtServiceGroup
// 
// 8     10/28/15 3:25p Hui.shao
// log fmt
// 
// 5     12/19/14 2:00p Hui.shao
// adjust sessionId to add a leading group name for future distributing by
// session group
// 
// 4     12/12/13 2:04p Hui.shao
// %lld for int64
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
// 45    09-06-05 17:17 Li.huang
// 
// 44    09-06-04 17:10 Hui.shao
// support of the override timeout of session
// 
// 43    08-11-24 15:29 Hongquan.zhang
// 
// 42    08-07-08 15:59 Hongquan.zhang
// 
// 41    08-03-18 14:55 Hongquan.zhang
// lock servant if using it
// 
// 40    07-12-14 18:03 Hongquan.zhang
// Use ZQTianShan::Adapter
// 
// 39    07-12-14 11:39 Hongquan.zhang
// Update Error Code
// 
// 38    07-12-05 15:49 Hongquan.zhang
// 
// 37    07-11-26 15:38 Hongquan.zhang
// modify because Purchase::detach has been changed
// 
// 36    07-11-19 11:53 Hongquan.zhang
// 
// 35    07-10-31 14:43 Hongquan.zhang
// 
// 34    07-10-25 14:07 Hongquan.zhang
// 
// 33    07-09-18 12:56 Hongquan.zhang
// 
// 32    07-06-21 15:37 Hongquan.zhang
// 
// 31    07-06-06 16:16 Hongquan.zhang
// 
// 30    07-05-24 11:35 Hongquan.zhang
// 
// 29    07-05-09 17:54 Hongquan.zhang
// 
// 28    07-05-09 17:45 Hongquan.zhang
// 
// 26    07-04-24 14:54 Hongquan.zhang
// 
// 26    07-04-12 14:02 Hongquan.zhang
// 
// 25    07-03-13 17:12 Hongquan.zhang
// 
// 24    07-03-01 15:27 Hongquan.zhang
// 
// 23    07-01-11 16:09 Hongquan.zhang
// 
// 22    07-01-09 15:15 Hongquan.zhang
// 
// 21    07-01-05 10:59 Hongquan.zhang
// 
// 20    06-12-25 15:51 Hui.shao
// uniform the throw
// 
// 19    06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 18    06-12-13 18:48 Hongquan.zhang
// 
// 17    06-10-16 12:59 Hongquan.zhang
// 
// 16    06-09-13 12:04 Hui.shao
// a simple edge to eliminate timer calls during state change procedure
// 
// 15    06-09-12 20:19 Hui.shao
// added SessionWatchDog
// 
// 14    06-08-29 12:33 Hui.shao
// 
// 13    06-08-28 18:30 Hui.shao
// 
// 12    06-08-24 19:23 Hui.shao
// 
// 11    06-08-21 17:05 Hui.shao
// 
// 5     06-07-17 14:46 Hui.shao
// initial impl of session manager
// 4     06-07-14 14:54 Hui.shao
// init impl on session record operations
// 1     06-07-10 14:24 Hui.shao
// console demo ready
// ===========================================================================

#include "SessionImpl.h"
#include "SessionState.h"
#include "SessionCommand.h"
#include "Log.h"
#include "Guid.h"

#include "../Ice/TsApplication.h"
#include "../Ice/TsStreamer.h"
#include <Ice/Ice.h>
#include <ServiceCommonDefintion.h>
#include "WeiwooConfig.h"

namespace ZQTianShan {
namespace Weiwoo {

typedef ::std::vector< Ice::Identity > IdentCollection;

// -----------------------------
// service SessionManagerImpl
// -----------------------------
SessionManagerImpl::SessionManagerImpl(WeiwooSvcEnv& env)
: _env(env)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "start session watchdog"));
	_env._watchDog.start();
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "restore all the sessions from last run"));
	IdentCollection identities;
	try	
	{
		ZQ::common::MutexGuard gd(_env._lockSession);
		::Freeze::EvictorIteratorPtr itptr = _env._eSession->getIterator("", 100);
		while (itptr->hasNext())
			identities.push_back(itptr->next());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionManager, "failed to list all the existing sessions"));
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SessionManager, "%d session(s) found need to restore"), identities.size());

	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++)
	{
		::TianShanIce::SRM::SessionExPrx sess ;
		try
		{
			sess = ::TianShanIce::SRM::SessionExPrx::uncheckedCast( _env._adapter->createProxy(*it));
			if( sess )
			{
				sess->onRestore();
			}
		}
		catch( const Ice::Exception& )
		{
		}
		std::string sessid = it->name;

		_env._watchDog.watchSession(*it, 0);
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "add the interface \"%s\" on to the adapter"), SERVICE_NAME_SessionManager);
	_localId=_env._communicator->stringToIdentity(SERVICE_NAME_SessionManager);
    //_env._adapter->add(this, _localId);
	_env._adapter->ZQADAPTER_ADD(_env._communicator, this, SERVICE_NAME_SessionManager);
	//_env._adapter->add(this, SERVICE_NAME_SessionManager);
}

SessionManagerImpl::~SessionManagerImpl()
{
//	if(_env._adapter->find(_localId)!=NULL)
//		_env._adapter->remove(_localId);
}

::std::string SessionManagerImpl::getAdminUri(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQTianShan::_IceThrow <TianShanIce::NotImplemented>("SessionManager", 1001, __MSGLOC__ "TODO: impl here");
	return "";
}

::TianShanIce::State SessionManagerImpl::getState(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented>("SessionManager", 1011, __MSGLOC__ "TODO: impl here");
	TianShanIce::State st=TianShanIce::stNotProvisioned ;
	return st;
}

::TianShanIce::SRM::SessionPrx SessionManagerImpl::openSession(const ::std::string& sessId, const ::Ice::Current& c)
{
	CONN_TRACE(c, SessionManager, openSession);
	Ice::Identity ident;
	ident.category=DBFILENAME_Session;
	ident.name=sessId;
	try
	{
		::TianShanIce::SRM::SessionPrx sessPrx=::TianShanIce::SRM::SessionPrx::checkedCast(_env._adapter->createProxy(ident));
		return sessPrx;
	}
	catch (...)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, "SessionManager", 1021, CLOGFMT(SessionManager, "Open session with sessid=%s failed"), sessId.c_str());
		return NULL; // dummy statement to avoid compiler error
	}
}

::TianShanIce::SRM::SessionPrx SessionManagerImpl::createSession(const ::TianShanIce::SRM::Resource& assetUri, const ::Ice::Current& c)
{
	CONN_TRACE(c, SessionManager, createSession);

	::TianShanIce::ValueMap::const_iterator itVal = assetUri.resourceData.find("uri");

	if (assetUri.resourceData.end() == itVal || ::TianShanIce::vtStrings != itVal->second.type)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, "SessionManager", 1031 , CLOGFMT(SessionManager, "No uri was found or invalid uri type, it must be vtString"));
	}

	if (itVal->second.strs.size() >1)
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(SessionManager, "multiple asset within an order may be not supported, only the first asset will be processed in this version"));

	const std::string strUri = itVal->second.strs[0];

	std::string	sessId = SessionImpl::generateSessionID();
	std::string sessGroup;
	
	{
		// adjust sessionId to add a leading group name for future distributing by session group
		::TianShanIce::ValueMap::const_iterator itGrp = assetUri.resourceData.find("sessiongroup");
		if (assetUri.resourceData.end() != itGrp && ::TianShanIce::vtStrings == itGrp->second.type && itGrp->second.strs.size() >0)
		{
			sessGroup = itGrp->second.strs[0];
			sessId = sessGroup + "." +sessId;
		}
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "creating sess[%s] for URI[%s]"), sessId.c_str(), strUri.c_str());

	int currentPendingSize = _env._thpool.pendingRequestSize();
	if( currentPendingSize  > gWeiwooServiceConfig.lMaxPendingRequestSize )
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionManager, "sess[%s] rejected per pending request[%d] maxPendingSize[%d]"), 
			sessId.c_str(), currentPendingSize , gWeiwooServiceConfig.lMaxPendingRequestSize  );
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, "SessionManager", -10 , CLOGFMT(SessionManager, "reject to create session due to too many pending request") );
	}

	SessionImpl::Ptr sess = new SessionImpl(_env);
	sess->expiration = sess->overrideExp = 0;
	sess->ident.category = DBFILENAME_Session;
	sess->state = ::TianShanIce::stNotProvisioned;
	sess->sessId = sess->ident.name = sessId;
	sess->resources[::TianShanIce::SRM::rtURI] = assetUri;

	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtServiceGroup))
	{
		// this is a bug that someone confused sessionGroup and serviceGroup
		// duplicate a resource of rtServiceGroup to add
		::TianShanIce::SRM::Resource resSvcGrp = assetUri; // inherits the resouce.attr
		resSvcGrp.resourceData.clear();
		::TianShanIce::Variant v;
		v.type = ::TianShanIce::vtInts;  v.bRange = false;
		v.ints.push_back(atol(sessGroup.c_str()));
		resSvcGrp.resourceData["id"] =v;

		sess->resources[::TianShanIce::SRM::rtServiceGroup] = resSvcGrp;
	}

	SessStateNotProvisioned(_env, *sess).enter();
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionManager, "adding new session[%s] into DB"), sess->sessId.c_str());
	_env._eSession->add(sess, sess->ident);	

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(SessionManager, "new session[%s] created: client[%s] asset[%s]"), 
		sess->sessId.c_str() , c.con->toString().c_str(), strUri.c_str());

	return IdentityToObj(Session, sess->ident);
}

::TianShanIce::SRM::SessionPrx SessionManagerImpl::createSessionBySSSI(const ::TianShanIce::SRM::Resource& assetUri, const ::TianShanIce::SRM::ResourceMap& resSSSI, const ::Ice::Current& c)
{
	CONN_TRACE(c, SessionManager, createSessionBySSSI);

#pragma message ( __MSGLOC__ "TODO: impl here")
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, "SessionManager", 1041, __MSGLOC__ "TODO: impl here");
	return NULL; // dummy statement to avoid compiler error
}

// -----------------------------
// class SessionImpl
// -----------------------------

#define SESSLOGFMT(_X) OBJLOGFMT(Session, _X)

::std::string SessionImpl::generateSessionID(void)
{
	char buf[32];
	ZQ::common::Guid id;
	id.create();
	id.toCompactIdstr(buf, sizeof(buf));
	return buf;
}

SessionImpl::SessionImpl(WeiwooSvcEnv& env)
:_env(env), _bStateInChange(false), _stateChangeTargetState(TianShanIce::stNotProvisioned)
{	
	 WLock sync(*this);
	 
}

void SessionImpl::onRestore(const ::Ice::Current&/* = ::Ice::Current()*/ )
{
	//set private data to path ticket if has one
	
	//Can't call ticket here because PathEnv is not ready

// 	if( ticket )
// 	{
// 		try
// 		{			
// 			TianShanIce::Variant varName;
// 			varName.type = TianShanIce::vtStrings;
// 			varName.strs.clear();
// 			varName.strs.push_back( ident.name );
// 			ticket->setPrivateData( WEIWOO_SESS_IDENT_NAME , varName );
// 
// 			TianShanIce::Variant varCata;
// 			varCata.type = TianShanIce::vtStrings;
// 			varCata.strs.clear();
// 			varCata.strs.push_back( ident.category );			
// 			ticket->setPrivateData( WEIWOO_SESS_IDENT_CATE , varCata );
// 		}
// 		catch( const Ice::Exception& ex )
// 		{
// 			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(Session, "onRestore() sess[%s] failed to associate its ticket due to [%s]"), 
// 				ident.name.c_str() , ex.ice_name().c_str() );
// 		}
// 	}
}

void SessionImpl::destroy(const ::Ice::Current& c)
{
    
	Ice::Context ctxt = c.ctx;
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("destroy session by [%s]"), ctxt["caller"].c_str());
	

	//setPrivateData(SYS_PROP(x-reason), varTeardown);
/*
	{
		WLock sync(*this);
		privdata[SYS_PROP(x-reason)]		= varTeardown;
		privdata[SYS_PROP(teardownReason)]	= varTeardown;
	}
*/
	switch (state)
	{
	case ::TianShanIce::stNotProvisioned:
	case ::TianShanIce::stProvisioned:
	case ::TianShanIce::stInService:
		{
			TianShanIce::Variant varTerminateReason ;
			varTerminateReason.type = TianShanIce::vtStrings;
			varTerminateReason.bRange = false;
			varTerminateReason.strs.clear();

			TianShanIce::Variant varTeardown;
			varTeardown.type = TianShanIce::vtStrings;
			varTerminateReason.bRange = false;
			varTeardown.strs.clear();
			
			if ( ctxt["caller_type"] == "rtsp_server_destroy")
			{
				std::string errorMsg = ctxt["caller"];
				if( errorMsg.length() > 0 )
				{//log if error message is not empty
					varTerminateReason.strs.push_back( errorMsg );
					{
						WLock sync(*this);
						privdata[SYS_PROP(terminateReason)] = varTerminateReason;
					}
				}
			}
			else
			{				
				/*
				char szBuf[1024];
				sprintf(szBuf, "2110%d0 Server session destory from state[%s] per external request", state, ZQTianShan::ObjStateStr(state));			
				*/
				varTeardown.strs.push_back(ctxt["caller"]);	
				{
					WLock sync(*this);
					privdata[SYS_PROP(teardownReason)] = varTeardown;
					privdata[SYS_PROP(x-reason)]		= varTeardown;
				}
			}
		}

		SessStateOutOfService(_env, *this).enter();
		// note: there is no "break" statement here

	case ::TianShanIce::stOutOfService:
		// up to the OutOfService procedure destroying itself: SessStateOutOfService(_env, *this).doDestroy(c);
		break;
	};
}

::std::string SessionImpl::getId(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return sessId;
}
::Ice::Int SessionImpl::setPrivateData2(const ::TianShanIce::ValueMap& valMap, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	WLock sync(*this);	
	::TianShanIce::ValueMap::const_iterator it = valMap.begin();
	std::string kvstr;
	for ( ; it != valMap.end() ; it++ )
	{
		kvstr += it->first + ", ";
		privdata[it->first] = it->second;
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(Session, "sess[%s] setPrivateData2() keys: %s"), ident.name.c_str(), kvstr.c_str());
	return (Ice::Int)valMap.size();
}
void SessionImpl::renew(::Ice::Long TTL, const ::Ice::Current& c)
{
	if (TTL < 0)
		return;
	//ZQ::common::MutexGuard gd(_updateTimerMutex);
#pragma message(__MSGLOC__"Should I use Mutex here??")
	if(state==::TianShanIce::stOutOfService)
	{
		envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("session is in outOfserviceState, renew is ignored"));
		return;
	}
	
	::Ice::Long exp, stampNow = now();
	 exp= stampNow + TTL;
	
#pragma message(__MSGLOC__"这里该如何操作，才可以使得没有进行renew操作的session不会被杀掉")
	WLock sync(*this);
//TODO:	SessStateBase(env, *this, ...)._updateExpiration(exp);
	expiration = exp;
	if (overrideExp >0 && expiration > overrideExp)
		expiration = overrideExp;

	char tmpBuf[32] ="n/a";
	TimeToUTC(expiration, tmpBuf, sizeof(tmpBuf));
	long newTTL = (long) (expiration - stampNow);
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("renew with %lldmsec, new expiration[%s] in %dmsec"), TTL, tmpBuf, newTTL);
	_env._watchDog.watchSession(ident, newTTL);   
}

void SessionImpl::OnTimer(const ::Ice::Current& c)
{
	if (_bStateInChange)
	{
		// if the state is in change, the timer is supposed to be changed by SessionState
		// just give a big timeout to ensure no timer call will be missed
		_env._watchDog.watchSession(ident, MAX_TTL_IDLE_SESSION);
		return;
	}
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("Session OnTimer"));
	switch (state)
	{
	case ::TianShanIce::stNotProvisioned:
		SessStateNotProvisioned(_env, *this).OnTimer(c);
		break;

	case ::TianShanIce::stProvisioned:
		SessStateProvisioned(_env, *this).OnTimer(c);
		break;

	case ::TianShanIce::stInService:
		SessStateInService(_env, *this).OnTimer(c);
		break;

	case ::TianShanIce::stOutOfService:
		{
			//WLock sync(*this);
			SessStateOutOfService(_env, *this).OnTimer(c);
		}
		break;

	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "Session", 1051, "SessionEx::OnTimer() unknown state: %d", state);
	};
}

::Ice::ObjectPrx SessionImpl::getOwner(const ::Ice::Current& c) const
{
    RLock sync(*this);
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, "Session", 1061, __MSGLOC__ "TODO: impl here");
	return NULL; // dummy statement to avoid compiler error
}

::TianShanIce::State SessionImpl::getState(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return state;
}

void SessionImpl::setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& value, const ::Ice::Current& c)
{
	if (key.empty())
	{
		envlog(ZQ::common::Log::L_ERROR, SESSLOGFMT("setPrivateData() empty key"));
		return;
	}

    WLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("setPrivateData(%s)"), key.c_str());
	privdata[key] = value;
}

::TianShanIce::ValueMap SessionImpl::getPrivateData(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return privdata;
}

char ResourceTypeStr[][64] = {
	{"rtURI"},
	{"rtStorage"},
	{"rtStreamer"},
	{"rtServiceGroup"},
	{"rtMpegProgram"},
	{"rtTsDownstreamBandwidth"},
	{"rtIP"},
	{"rtEthernetInterface"},
	{"rtPhysicalChannel"},
	{"rtAtscModulationMode"},
	{"rtHeadendId"},
	{"rtClientConditionalAccess"},
    {"rtServerConditionalAccess"}
};

::Ice::Long SessionImpl::addResource(::TianShanIce::SRM::ResourceType type, ::TianShanIce::SRM::ResourceAttribute attribute, const ::TianShanIce::ValueMap& resourceData, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("addResource() with type[%s]"), ResourceTypeStr[type]);
	::TianShanIce::SRM::Resource res;
	res.resourceData = resourceData;
	res.status	= ::TianShanIce::SRM::rsRequested;
	res.attr	= attribute;
    return addResourceEx(type, res, c);
}

::Ice::Long SessionImpl::addResourceEx(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::SRM::Resource& res, const ::Ice::Current& c)
{
    WLock sync(*this);
	
	switch (state)
	{
	case ::TianShanIce::stNotProvisioned:
		return SessStateNotProvisioned(_env, *this).doAddResourceEx(type, res, c);

	case ::TianShanIce::stProvisioned:
		return SessStateProvisioned(_env, *this).doAddResourceEx(type, res, c);

	case ::TianShanIce::stInService:
		return SessStateInService(_env, *this).doAddResourceEx(type, res, c);

	case ::TianShanIce::stOutOfService:
		return SessStateOutOfService(_env, *this).doAddResourceEx(type, res, c);

	default:
		ZQTianShan::_IceThrow <TianShanIce::InvalidStateOfArt> (envlog, "Session", 1071, "Session::addResourceEx() unknown state: %d", state);
	};

	return 0;
}

::TianShanIce::SRM::ResourceMap SessionImpl::getReources(const ::Ice::Current& c) const
{
    RLock sync(*this);
	return resources;
}

void SessionImpl::removeResource(::TianShanIce::SRM::ResourceType type, const ::Ice::Current& c)
{
    WLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("removeResource() with type is %s"), ResourceTypeStr[type]);
	switch (state)
	{
	case ::TianShanIce::stNotProvisioned:
		SessStateNotProvisioned(_env, *this).doRemoveResource(type, c); break;

	case ::TianShanIce::stProvisioned:
		SessStateProvisioned(_env, *this).doRemoveResource(type, c); break;

	case ::TianShanIce::stInService:
		SessStateInService(_env, *this).doRemoveResource(type, c); break;

	case ::TianShanIce::stOutOfService:
		SessStateOutOfService(_env, *this).doRemoveResource(type, c); break;

	default:
		ZQTianShan::_IceThrow <TianShanIce::InvalidStateOfArt> (envlog, "Session", 1081, "Session::removeResource() unknown state: %d", state);
	};
}

void SessionImpl::negotiateResources(::TianShanIce::State state, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("negotiateResources(%d)"), state);
	
	switch (state)
	{
	case ::TianShanIce::stNotProvisioned:
		SessStateNotProvisioned(_env, *this).doNegotiateResources(state, c); break;

	case ::TianShanIce::stProvisioned:
		SessStateProvisioned(_env, *this).doNegotiateResources(state, c); break;

	case ::TianShanIce::stInService:
		SessStateInService(_env, *this).doNegotiateResources(state, c); break;

	case ::TianShanIce::stOutOfService:
		SessStateOutOfService(_env, *this).doNegotiateResources(state, c); break;

	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "Session", 1091, "Session::negotiateResources() unknown state: %d", state);
	};
}

::TianShanIce::Application::PurchasePrx SessionImpl::getPurchase(const ::Ice::Current& c) const
{
	try {
		RLock sync(*this);
		if (!purchasePrxStr.empty())
			return ::TianShanIce::Application::PurchasePrx::uncheckedCast(_env._communicator->stringToProxy(purchasePrxStr));
	}
	catch(const ::Ice::Exception& e)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, "Session", 1101, SESSLOGFMT("getPurchase() %s caught when access purchase instance: %s"), e.ice_name().c_str(), purchasePrxStr.c_str());
	}
	
	envlog(ZQ::common::Log::L_WARNING, SESSLOGFMT("getPurchase() no purchase has been attached"));
	return NULL;
}

::TianShanIce::Streamer::StreamPrx SessionImpl::getStream(const ::Ice::Current& c) const
{
	try 
	{
		RLock sync(*this);
		if (!streamPrxStr.empty())
		{
			return ::TianShanIce::Streamer::StreamPrx::uncheckedCast(_env._communicator->stringToProxy(streamPrxStr));
		}
		else
		{
			ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, "Session", 1111, SESSLOGFMT("getStream() no stream attached with current session"));
		}
	}
	catch(const ::Ice::Exception& e)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, "Session", 1112, SESSLOGFMT("getStream() %s caught when access stream instance: %s"), e.ice_name().c_str(), streamPrxStr.c_str());
	}
	
	envlog(ZQ::common::Log::L_WARNING, SESSLOGFMT("getStream() no stream has been attached"));
	return NULL;
}

void SessionImpl::attachPurchase(const ::TianShanIce::Application::PurchasePrx& purchase, const ::Ice::Current& c)
{
    WLock sync(*this);

	switch (state)
	{
	case ::TianShanIce::stNotProvisioned:
		SessStateNotProvisioned(_env, *this).doAttachPurchase(purchase, c); break;

	case ::TianShanIce::stProvisioned:
		SessStateProvisioned(_env, *this).doAttachPurchase(purchase, c); break;

	case ::TianShanIce::stInService:
		SessStateInService(_env, *this).doAttachPurchase(purchase, c); break;

	case ::TianShanIce::stOutOfService:
		SessStateOutOfService(_env, *this).doAttachPurchase(purchase, c); break;

	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, "Session", 1121, "Session::attachPurchase() unknown state: %d", state);
	};
}

void SessionImpl::provision(const ::Ice::Current& c)
{
	try 
	{
		SessStateProvisioned(_env, *this).enter();
	}
	catch(const ::TianShanIce::BaseException& be)
	{
		be.ice_throw();
	}
	catch(const ::Ice::Exception& e)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, "Session", 1131, OBJLOGFMT(Session, "provision(): exception cought: %s"), e.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, "Session", 1132, OBJLOGFMT(Session, "provision(): unkown exception"));
	}
}

void SessionImpl::provision_async(const ::TianShanIce::SRM::AMD_Session_provisionPtr& amdCB, const ::Ice::Current& c)
{	
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("provision_async() pending threadPool[%d/%d] pendingRequest[%d]"), 
		_env._thpool.activeCount(), _env._thpool.size(), _env._thpool.pendingRequestSize());
	(new SessionProvisionCommand(amdCB, _env, *this))->execute();
}

void SessionImpl::serve(const ::Ice::Current& c)
{
	SessStateInService(_env, *this).enter();
}

void SessionImpl::serve_async(const ::TianShanIce::SRM::AMD_Session_servePtr& amdCB, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("serve_async() pending threadPool[%d/%d] pendingRequest[%d]"), 
			_env._thpool.activeCount(), _env._thpool.size(), _env._thpool.pendingRequestSize());
	(new SessionServeCommand(amdCB, _env, *this))->execute();
}

}} // namespace
