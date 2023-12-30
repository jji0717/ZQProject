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
// Ident : $Id: BaseCPH.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/BaseCPH.cpp $
// 
// 2     4/30/15 11:20a Zhiqiang.niu
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 13    10-04-07 15:24 Jie.zhang
// add parameter on updateProgress
// 
// 12    09-12-29 12:16 Xia.chen
// merge form 1.10
// 
// 11    09-06-26 10:52 Yixin.tian
// 
// 10    09-02-12 13:58 Yixin.tian
// 
// 9     09-01-20 17:59 Jie.zhang
// 
// 8     08-11-18 10:59 Jie.zhang
// merge from TianShan1.8
// 
// 10    08-09-03 11:06 Xia.chen
// 
// 9     08-09-03 10:38 Xia.chen
// 
// 8     08-08-27 14:52 Xia.chen
// 
// 7     08-06-19 18:59 Jie.zhang
// log changes
// 
// 6     08-04-25 14:29 Jie.zhang
// 
// 8     08-04-17 18:18 Build
// 
// 7     08-04-11 18:29 Jie.zhang
// 
// 6     08-03-17 19:56 Jie.zhang
// 
// 5     08-03-06 15:57 Jie.zhang
// 
// 4     08-02-28 16:17 Jie.zhang
// 
// 3     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================
#include "BaseCPH.h"
#include "CPHInc.h"

namespace ZQTianShan {
namespace ContentProvision {

// -----------------------------
// class BaseCPHelper
// -----------------------------
std::string BaseCPHelper::_confDir, BaseCPHelper::_logDir;	
ZQ::common::Log BaseCPHelper::NullLog;

BaseCPHelper::BaseCPHelper(ZQ::common::NativeThreadPool& pool, ICPHManager* mgr)
:_pCPELogger(NULL),_pool(pool),  _mgr(mgr), _bInCleanup(false)
{
	if (NULL != _mgr)
		_pCPELogger = _mgr->getLogger();
	if (NULL == _pCPELogger)
		_pCPELogger = &NullLog;
}

BaseCPHelper::~BaseCPHelper()
{
	cleanupSessions();
}

void BaseCPHelper::cleanupSessions()
{
	try {
		ZQ::common::MutexGuard gd(_lockSessMap);
		_bInCleanup = true;
		_sessMap.clear();
	}
	catch(...) {}
	_mgr = NULL;
}

ICPHSession* BaseCPHelper::find(const char* provisionId)
{
	ZQ::common::MutexGuard gd(_lockSessMap);
	SessMap::iterator it = _sessMap.find(provisionId);
	if (_sessMap.end() != it)
		return it->second;
	return NULL;
}

bool BaseCPHelper::reg(const ::std::string& provisionId, BaseCPHSession* pSess)
{
	if (provisionId.empty() || NULL == pSess)
	{
		EngHelperLog(ZQ::common::Log::L_ERROR, CLOGFMT(BaseCPHelper, "reg() NULL id or NULL session"));
		return false;
	}
	
	{
		ZQ::common::MutexGuard gd(_lockSessMap);
		SessMap::iterator it = _sessMap.find(provisionId);
		if (_sessMap.end() != it)
		{
			BaseCPHSession* ptr= it->second;			
			EngHelperLog(ZQ::common::Log::L_WARNING, CLOGFMT(BaseCPHelper, "reg() sess[%s] overwritting old instance:%08x with new instance:%08x "),
				provisionId.c_str() ,ptr , pSess );

			it->second = pSess;
		}
		else 
		{
			_sessMap.insert(SessMap::value_type(provisionId, pSess));
		}
	}

	EngHelperLog(ZQ::common::Log::L_DEBUG, CLOGFMT(BaseCPHelper, "reg() sess[%s] instance:%08x in CPE"), provisionId.c_str(), pSess);
	_mgr->registerHelperSession(provisionId.c_str(), pSess);

	return true;
}

void BaseCPHelper::unreg(const std::string& provisionId, const BaseCPHSession* pSess)
{
	if (!_bInCleanup)
	{

		ZQ::common::MutexGuard gd(_lockSessMap);
		SessMap::iterator it = _sessMap.find(provisionId);
		if (_sessMap.end() != it && it->second == pSess)
		{
			_sessMap.erase(it);
			if (NULL != _mgr)
				_mgr->unregisterHelperSession(provisionId.c_str());

			EngHelperLog(ZQ::common::Log::L_DEBUG, CLOGFMT(BaseCPHelper, "unreg() sess[%s] in CPE"), provisionId.c_str());
		}

	} 
}

void BaseCPHelper::increaseLoad(const std::string& methodtype,int nbandwidth)
{
	std::map<std::string ,int>::iterator iter,itor;
	typedef std::pair <std::string, int> stringIntPair;

	ZQ::common::Guard<ZQ::common::Mutex> op(_lockSessMap);
	iter = _allocSess.find(methodtype); 
	itor = _countSess.find(methodtype);
	if (iter != _allocSess.end())
	{
		(*iter).second = (*iter).second + nbandwidth/1024;
		(*itor).second += 1;
	}
	else
	{
		_allocSess.insert(stringIntPair(methodtype,nbandwidth/1024));
		_countSess.insert(stringIntPair(methodtype,1));
	}

}

void BaseCPHelper::decreaseLoad(const std::string& methodtype,int nbandwidth)
{
	std::map<std::string ,int>::iterator iter,itor;

	ZQ::common::Guard<ZQ::common::Mutex> op(_lockSessMap);
	iter = _allocSess.find(methodtype); 
	itor = _countSess.find(methodtype);
	if (iter != _allocSess.end())
	{
		if ((*iter).second - nbandwidth/1024 < 0)
		{
			(*iter).second = 0;
			EngHelperLog(ZQ::common::Log::L_WARNING, CLOGFMT(BaseCPHelper, "bandwidth appears negative."));
		}
		else
			(*iter).second = (*iter).second - nbandwidth/1024;
		if ((*itor).second -1 < 0)
		{
			(*itor).second = 0;
			EngHelperLog(ZQ::common::Log::L_WARNING, CLOGFMT(BaseCPHelper, "session number appears negative."));
		}
		else
			(*itor).second -= 1;
	}
}

void BaseCPHelper::getCurrentLoad(const std::string& methodtype, uint32& allocatedKbps,uint& sessions)
{
	std::map<std::string,int>::iterator iter,itor;
	ZQ::common::Guard<ZQ::common::Mutex> op(_lockSessMap);
	if (_sessMap.size() != 0)
	{
		iter = _allocSess.find(methodtype);
		itor = _countSess.find(methodtype);

		if (iter != _allocSess.end())
		{
			sessions = (*itor).second;
			allocatedKbps = (*iter).second;
		}
		else
		{
			sessions = 0;
			allocatedKbps = 0;
		}

	}
	else
	{
		sessions = 0;
		allocatedKbps = 0;
	}
}
// -----------------------------
// class BaseCPHSession
// -----------------------------
#undef cpelog 
#define cpelog (*_helper._pCPELogger)
BaseCPHSession::BaseCPHSession(BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
:ThreadRequest(helper._pool), _helper(helper), _sess(pSess), _pPushSource(NULL)
{
	if (pSess)
	{
		_provisionId = _sess->ident.name;
		_helper.reg(_provisionId, this);
	}
	
	_bPreloaded = false;
	_bInited = false;
	_nErrCode = 0;
	
	EngSessLog(ZQ::common::Log::L_DEBUG, CPHSESSLOGFMT(BaseCPHSession, "BaseCPHSession() this[0x%08x]"), this);
}

BaseCPHSession::~BaseCPHSession()
{
	_helper.unreg(_provisionId, this);
	EngSessLog(ZQ::common::Log::L_DEBUG, CPHSESSLOGFMT(BaseCPHSession, "~BaseCPHSession() this[0x%08x]"), this);
}

bool BaseCPHSession::init(void)
{
	if (_bInited)
		return _bInited;
	
	if (!_sess)
		return false;

	return preLoad();
}

bool BaseCPHSession::prime()
{
	if (!_bPreloaded)
	{
		if (!preLoad())
			return false;
		
		_bPreloaded = true;
	}

	if (::TianShanIce::ContentProvision::stPushTrigger == _sess->stType)
	{
		_pPushSource = _helper._mgr->findPushSource(_sess->contentKey);
	}	

	_bInited = true;
	return true;
}

void BaseCPHSession::final(int retcode, bool bCancelled)
{
	EngSessLog(ZQ::common::Log::L_DEBUG, CPHSESSLOGFMT(BaseCPHSession, "final() ret=%d, cancelled=%c; calling doCleanup()"), retcode, bCancelled?'Y':'N');
	terminate();
#ifdef ZQ_OS_MSWIN
	::Sleep(100);
#else
	usleep(100000);
#endif
	doCleanup();
	
	if (!bCancelled)
		return;
	
	char buf[32];
	::TianShanIce::Properties params;
	bool errorOccurs = (0 !=retcode);
	params["exitCode"] = itoa(retcode, buf, 10);

	if (bCancelled)
	{
		errorOccurs = true;
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseCPHSession, "final() reporting Stopped due to this is a cancel"));
		params["cancelled"] = "1";
	}

	notifyStopped(errorOccurs, params);

	delete this;
}


// access to the ProvisionSession
void BaseCPHSession::notifyStarted(const ::TianShanIce::Properties& params)
{
	try
	{
		_sess->notifyStarted(params, ::Ice::Current());
		EngSessLog(ZQ::common::Log::L_INFO, CPHSESSLOGFMT(BaseCPHSession, "notifyStarted()"));
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseCPHSession, "notifyStarted() caught unknown exception"));
	}	
}

void BaseCPHSession::notifyStopped(bool errorOccurred, const ::TianShanIce::Properties& params)
{
	::TianShanIce::Properties eventParams = params;

	try
	{
		_sess->notifyStopped(errorOccurred, eventParams, ::Ice::Current());
		EngSessLog(ZQ::common::Log::L_INFO, CPHSESSLOGFMT(BaseCPHSession, "notifyStopped() errorOccurred=%c"), errorOccurred?'Y':'N');
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseCPHSession, "notifyStopped() errorOccurred=%c caught unknown exception")
			, errorOccurred?'Y':'N');
	}
}

void BaseCPHSession::notifyStreamable(bool streamable)
{
	try
	{
		_sess->notifyStreamable(streamable, ::Ice::Current());
		EngSessLog(ZQ::common::Log::L_INFO, CPHSESSLOGFMT(BaseCPHSession, "notifyStreamable() streamable=%c"), streamable?'Y':'N');
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseCPHSession, "notifyStreamable() streamable=%c caught unknown exception")
			, streamable?'Y':'N');
	}
}

void BaseCPHSession::updateProgress(const ::Ice::Long processed, const ::Ice::Long total, const ::TianShanIce::Properties& params)
{
	try
	{
		_sess->updateProgress(processed, total, params, ::Ice::Current());
	#if 0
		EngSessLog(ZQ::common::Log::L_DEBUG, CPHSESSLOGFMT(BaseCPHSession, "updateProgress() %lld/%lld"), processed, total);
	#endif // _DEBUG
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseCPHSession, "updateProgress() %lld/%lld caught unknown exception")
			, processed, total);
	}
}

}} // namespace

#ifdef ZQ_OS_MSWIN

static HMODULE	_gCurrentModule=NULL;
BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	_gCurrentModule=(HMODULE) hModule;

    switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}

#endif

