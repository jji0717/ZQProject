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
// Ident : $Id: ProvisionCmd.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionCmds.cpp $
// 
// 4     7/09/13 10:44a Li.huang
// 
// 3     9/13/11 10:21a Li.huang
// fix bug  15107
// 
// 2     10-12-03 15:35 Li.huang
// 13454
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 26    09-08-19 12:23 Jie.zhang
// merge from 1.10
// 
// 26    09-07-14 15:50 Jie.zhang
// tuning for the quick-pause
// 
// 25    09-01-09 11:30 Yixin.tian
// modify warning
// 
// 24    08-12-25 14:53 Yixin.tian
// 
// 23    08-12-15 17:53 Yixin.tian
// merge for Linux OS
// 
// 22    08-06-26 12:15 Jie.zhang
// fixed the int64/int convert to string issue in progress
// 
// 21    08-06-24 14:56 Jie.zhang
// changestatecmd removed, "setup","ontimer","onrestore" process in ice
// server dispatch thread.
// 
// 20    08-06-18 11:15 Xia.chen
// 
// 19    08-06-17 19:10 Xia.chen
// 
// 18    08-06-17 16:48 Xia.chen
// 
// 17    08-05-17 19:09 Jie.zhang
// 
// 16    08-05-14 22:07 Jie.zhang
// 
// 15    08-04-25 18:05 Jie.zhang
// 
// 14    08-04-23 15:58 Build
// 
// 13    08-04-17 11:01 Xia.chen
// 
// 12    08-04-09 15:37 Hui.shao
// impl listMethods
// 
// 11    08-04-09 11:48 Hui.shao
// added ProvisionCost
// 
// 10    08-04-02 15:47 Hui.shao
// per CPC ICE changes
// 
// 9     08-03-27 16:53 Jie.zhang
// 
// 10    08-03-24 19:41 Jie.zhang
// 
// 9     08-03-17 19:56 Jie.zhang
// 
// 8     08-02-28 16:17 Jie.zhang
// 
// 7     08-02-22 18:58 Jie.zhang
// 
// 6     08-02-21 15:33 Hui.shao
// 
// 5     08-02-21 15:08 Hui.shao
// added paged list
// 
// 4     08-02-19 14:33 Hongquan.zhang
// clear timeout cmd after executed it
// 
// 3     08-02-18 20:58 Hui.shao
// impl list provison
// 
// 2     08-02-18 18:40 Hongquan.zhang
// fix minor errors
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#include "ProvisionCmds.h"
#include "ProvisionState.h"
#include <algorithm>
#include "CPECfg.h"
#include "CPHInc.h"

namespace ZQTianShan {
namespace CPE {

#define PROVISIONSESSLOGFMT(_C, _X) CLOGFMT(_C, "provision[%s:%s(%d)] " _X), _pSess->ident.name.c_str(), ProvisionStateBase::stateStr(_pSess->state), _pSess->state
#define PROVISIONSESSEXPFMT(_C, _ERRCODE, _X) EXPFMT(_C, _ERRCODE, "provision[%s:%s(%d)] " _X), _pSess->ident.name.c_str(), ProvisionStateBase::stateStr(_pSess->state), _pSess->state


// -----------------------------
// class TimerCmd
// -----------------------------
TimerCmd::TimerCmd(CPEEnv& env, const ::Ice::Identity& provisionIdent)
: ThreadRequest(env._timerthpool), _env(env), _identProv(provisionIdent)
{
}

int TimerCmd::run(void)
{
	::TianShanIce::ContentProvision::ProvisionSessionExPrx sess;

	try
	{
		sess = IdentityToObjEnv(_env, ProvisionSessionEx, _identProv);
		sess->OnTimer();
		return 0;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "sess[%s] exception occurs: %s:%s"), _identProv.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "sess[%s] exception occurs: %s"), _identProv.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "sess[%s] unknown exception occurs"), _identProv.name.c_str());
	}
/*
	// when reaches here, an exception might occur, when re-post a timer command to ensure no action is dropped
	try
	{
		_env._watchDog.watch(_identProv, _env._defaultIdle);
	}
	catch(...)
	{
	}
*/
	return -1;
}

void TimerCmd::final(int retcode, bool bCancelled)
{
	if (bCancelled)
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerCmd, "sess[%s] user canceled timer activity"),_identProv.name.c_str());

	delete this;
}

// -----------------------------
// class ProvisionWatchDog
// -----------------------------
#ifdef ZQ_OS_MSWIN
ProvisionWatchDog::ProvisionWatchDog(CPEEnv& env)
:ThreadRequest(env._thpool), _bQuit(false), _hWakeupEvent(NULL), _env(env), _nextWakeup(now() + MAX_IDLE)
{
	_hWakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

ProvisionWatchDog::~ProvisionWatchDog()
{
	_bQuit = true;
	wakeup();
	
	::Sleep(1);
	if(_hWakeupEvent)
		::CloseHandle(_hWakeupEvent);
}

void ProvisionWatchDog::wakeup()
{
	::SetEvent(_hWakeupEvent);
}

bool ProvisionWatchDog::init(void)
{	
	return (NULL != _hWakeupEvent && !_bQuit);
}

#else
ProvisionWatchDog::ProvisionWatchDog(CPEEnv& env)
:ThreadRequest(env._thpool), _nextWakeup(now() + MAX_IDLE), _env(env), _bQuit(false)
{
	sem_init(&_wakeupSem,0,0);
}

ProvisionWatchDog::~ProvisionWatchDog()
{
	_bQuit = true;
	wakeup();

	usleep(1000);
	try
	{
		sem_destroy(&_wakeupSem);
	}
	catch(...){}
}

void ProvisionWatchDog::wakeup()
{
	sem_post(&_wakeupSem);
}

bool ProvisionWatchDog::init(void)
{
	int nVal;
	bool re = sem_getvalue(&_wakeupSem,&nVal);
	return (re ==0 && !_bQuit);	
}
#endif

#define MIN_YIELD	(100)  // 100 msec

int ProvisionWatchDog::run()
{
	_env._log(ZQ::common::Log::L_DEBUG, "ProvisionWatchDog() Session WatchDog is running");
	while(!_bQuit)
	{
		::Ice::Long timeOfNow = now();

		IdentCollection sessIdentsToExecute;

		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ProvisionWatchDog, "%d session(s) under watching"), _expirations.size());
			_nextWakeup = timeOfNow + MAX_IDLE;
			
			for(ExpirationMap::iterator it = _expirations.begin(); it != _expirations.end(); it ++)
			{
				if (it->second <= timeOfNow)
					sessIdentsToExecute.push_back(it->first);
				else
					_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
			}

			for (IdentCollection::iterator it2 = sessIdentsToExecute.begin(); it2 < sessIdentsToExecute.end(); it2++)
				_expirations.erase(*it2);
		}

		if(_bQuit)
			break;	// should quit polling

		for (IdentCollection::iterator it = sessIdentsToExecute.begin(); it < sessIdentsToExecute.end(); it++)
			(new TimerCmd(_env, *it))->start();
		
		sessIdentsToExecute.clear ();

		if(_bQuit)
			break;	// should quit polling

		long sleepTime = (long) (_nextWakeup - now());

		if (sleepTime < MIN_YIELD)
			sleepTime = MIN_YIELD;
#ifdef ZQ_OS_MSWIN
		::WaitForSingleObject(_hWakeupEvent, sleepTime);
#else
		struct timespec ts;
		struct timeval tmval;
		gettimeofday(&tmval,(struct timezone*)NULL);
		int64 nMicro = sleepTime*1000ll + tmval.tv_usec;
		ts.tv_sec = tmval.tv_sec + nMicro/1000000;
		ts.tv_nsec = (nMicro%1000000) * 1000;
		sem_timedwait(&_wakeupSem,&ts);
#endif
	} // while

	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionWatchDog, "end of Session WatchDog"));

	return 0;
}

void ProvisionWatchDog::quit()
{
	_bQuit=true;
	wakeup();
}

void ProvisionWatchDog::watchSession(const ::Ice::Identity& sessIdent, long timeout)
{
	if (timeout > 0)
	{
		::Ice::Long newExp = now() + timeout;

		ZQ::common::MutexGuard gd(_lockExpirations);
//		_expirations[sessIdent] = newExp;
        MAPSET(ExpirationMap , _expirations, sessIdent, newExp);
		if (newExp < _nextWakeup)
		{
			wakeup();
		}
	}
	else
	{
		ZQ::common::MutexGuard gd(_lockExpirations);
		ExpirationMap::iterator it = _expirations.find(sessIdent);
		if (it!=_expirations.end())
		{
			_expirations.erase(it);
		}

		(new TimerCmd(_env, sessIdent))->start();
	}
}

void ProvisionWatchDog::final(int retcode, bool bCancelled)
{
	envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionWatchDog, "session watch dog stopped"));
}

// -----------------------------
// class RestoreCmd
// -----------------------------
RestoreCmd::RestoreCmd(CPEEnv& env, const ::Ice::Identity& provisionIdent)
: ThreadRequest(env._thpool), _env(env), _identProv(provisionIdent)
{
}

int RestoreCmd::run(void)
{
	::TianShanIce::ContentProvision::ProvisionSessionExPrx sess;

	try
	{
		sess = IdentityToObjEnv(_env, ProvisionSessionEx, _identProv);
		sess->OnRestore();
		return 0;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		return 0;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(RestoreCmd, "sess[%s] exception occurs: %s:%s"), _identProv.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(RestoreCmd, "sess[%s] exception occurs: %s"), _identProv.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(RestoreCmd, "sess[%s] unknown exception occurs"), _identProv.name.c_str());
	}

	return -1;
}

void RestoreCmd::final(int retcode, bool bCancelled)
{
	if (bCancelled)
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(RestoreCmd, "sess[%s] user canceled timer activity"),_identProv.name.c_str());

	delete this;
}

// -----------------------------
// class ChangeStateCmd
// -----------------------------
#define PROVSTATELOGFMT(_C, _X) CLOGFMT(_C, "provision[%s:%s(%d)] " _X), _sess.ident.name.c_str(), ProvisionStateBase::stateStr(_sess.state), _sess.state
#define PROVSTATEEXPFMT(_C, _ERRCODE, _X) EXPFMT(_C, _ERRCODE, "provision[%s:%s(%d)] " _X), _sess.ident.name.c_str(), ProvisionStateBase::stateStr(_sess.state), _sess.state


// -----------------------------
// class ListProvisionCmd
// -----------------------------
ListProvisionCmd::ListProvisionCmd(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listSessionsPtr& amdCB, CPEEnv& env, const ::std::string& methodType, const ::TianShanIce::StrValues& paramNames, const ::std::string& startId, ::Ice::Int maxCount)
: ThreadRequest(env._timerthpool), _env(env), _amdCB(amdCB), _paramNames(paramNames), _methodType(methodType), _startId(startId), _maxCount(maxCount)
{
}

bool ListProvisionCmd::init(void)
{
	return (0 != _amdCB);
}

int ListProvisionCmd::run(void)
{
	std::string lastError;
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListProvisionCmd, "list hosted provisions"));
	
	IdentCollection Idents;
	try	{
		::Freeze::EvictorIteratorPtr itptr = _env._eProvisionSession ->getIterator("", 100);
		while (itptr && itptr->hasNext())
			Idents.push_back(itptr->next());
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ListProvisionCmd, "caught exception[%s] when enumerate provisons"), ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ListProvisionCmd, "caught unknown exception when enumerate provisons"));
	}
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListProvisionCmd, "found %d matched transactions"), Idents.size());
	
	// build up the transaction info collection based on the search result
	::TianShanIce::ContentProvision::ProvisionInfos results;

	try {

		::std::sort(Idents.begin(), Idents.end());

		if (_maxCount <= 0)
			_maxCount = Idents.size();

		for (IdentCollection::iterator it= Idents.begin(); it < Idents.end() && _maxCount>0 && results.size() < _maxCount; it++)
		{
			if (!_startId.empty() && it->name.compare(_startId) <0)
				continue;

			try {
				::TianShanIce::ContentProvision::ProvisionSessionPrx provision = IdentityToObjEnv(_env, ProvisionSession, *it);
				::TianShanIce::ContentProvision::ProvisionInfo provInfo;
				provInfo.contentKey = provision->getContentKey();
				provInfo.state = provision->getState();
				
				::TianShanIce::Properties props = provision->getProperties();
				std::string startTimeUTC, endTimeUTC,methodType;
				Ice::Long processedbytes,totalbytes;
				provision->getScheduledTime(startTimeUTC, endTimeUTC);
				methodType = provision->getMethodType();
				provision->queryProgress(processedbytes,totalbytes);
				//std::string processed, total;
				char processed[64] = {0};
				char total[64] = {0};

				std::stringstream os;
				os<<processedbytes;
				os>>processed;
				
				os.clear();
				os<<totalbytes;
				os>>total;

#define TXNPARAM_BEGIN  if (0)
#define TXNPARAM_ITEM(FIELD, SVALUE)   else if (0 == pit->compare(FIELD)) provInfo.params.insert(::TianShanIce::Properties::value_type(FIELD, SVALUE))
#define TXNPARAM_END else if (props.end() !=props.find(*pit)) provInfo.params.insert(::TianShanIce::Properties::value_type(*pit, props[*pit]))
				
				for (::TianShanIce::StrValues::iterator pit= _paramNames.begin(); pit < _paramNames.end(); pit++)
				{
					TXNPARAM_BEGIN;
					TXNPARAM_ITEM(SYS_PROP(contentName), provInfo.contentKey.content);
					TXNPARAM_ITEM(SYS_PROP(contentMethod), methodType);
					TXNPARAM_ITEM(SYS_PROP(contentStore), provInfo.contentKey.contentStoreNetId);
					TXNPARAM_ITEM(SYS_PROP(volume), provInfo.contentKey.volume);
					TXNPARAM_ITEM(SYS_PROP(scheduledStart),  startTimeUTC);
					TXNPARAM_ITEM(SYS_PROP(scheduledEnd),  endTimeUTC);
					TXNPARAM_ITEM(SYS_PROP(processedBytes),  std::string(processed));
					TXNPARAM_ITEM(SYS_PROP(totalBytes),  std::string(total));
//#pragma message(__MSGLOC__"TODO: more non-property parameters here")
					TXNPARAM_END;
				}
				
				results.push_back(provInfo);
			}
			catch (...) {}
		}
		
		_amdCB->ice_response(results);
		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListProvisionCmd caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListProvisionCmd caught unknown exception");
		lastError = buf;
	}
	
	TianShanIce::ServerError ex("CPE", 501, lastError);
	_amdCB->ice_exception(ex);
	
	return 1;
}

// -----------------------------
// class ListMethodCmd
// -----------------------------
ListMethodCmd::ListMethodCmd(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listMethodsPtr& amdCB, CPEEnv& env)
: ThreadRequest(env._timerthpool), _env(env), _amdCB(amdCB)
{
}

bool ListMethodCmd::init(void)
{
	return (0 != _amdCB);
}

int ListMethodCmd::run(void)
{
	std::string lastError;
	
	try {
		
		::TianShanIce::ContentProvision::MethodInfos result;
		
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListMethodCmd, "list hosted methods"));
		::TianShanIce::StrValues methods = _env._provisionFactory->listSupportedMethods();
		for (size_t i =0; i < methods.size(); i++)
		{
			try {
				::TianShanIce::ContentProvision::MethodInfo info;
				info.methodType = methods[i];
				
				uint32  allocatedKbps, maxKbps;
				uint sessions, maxsessions;
				
				ICPHelper* pHelper = _env._provisionFactory->findHelper(info.methodType.c_str());
#pragma message(__MSGLOC__"TODO: fillin the version info here")

				if (NULL != pHelper && pHelper->getLoad(info.methodType.c_str(), allocatedKbps, maxKbps, sessions, maxsessions))
				{
					info.allocatedKbps = allocatedKbps;
					info.maxKbps		= maxKbps;
					info.sessions = sessions;
					info.maxsessions = maxsessions;
					
					result.push_back(info);
				}
				
			}
			catch (...) {}
		}
		
		_amdCB->ice_response(result);
		return 0;
	}
	catch(const ::Ice::Exception& ex)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListMethodCmd caught exception[%s]", ex.ice_name().c_str());
		lastError = buf;
	}
	catch(...)
	{
		char buf[2048];
		snprintf(buf, sizeof(buf)-2, "ListMethodCmd caught unknown exception");
		lastError = buf;
	}
	
	TianShanIce::ServerError ex("CPE", 501, lastError);
	_amdCB->ice_exception(ex);
	
	return 1;
}

}} // namespace
