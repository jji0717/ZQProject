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
// Ident : $Id: BaseNPVRSession.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// ===========================================================================
#include "BaseCPHnPVR.h"
#include "CPHInc.h"

namespace ZQTianShan {
namespace ContentProvision {

// -----------------------------
// class BaseCPHelper
// -----------------------------
std::string BaseCPHelper::_confDir, BaseCPHelper::_logDir;	
ZQ::common::Log BaseCPHelper::NullLog;

BaseCPHelper::BaseCPHelper(ZQ::common::NativeThreadPool& pool, ICPHManager* mgr)
: _mgr(mgr), _pool(pool), _pCPELogger(NULL), _bInCleanup(false), _timerWatcher(*_mgr->getLogger(), pool)
{
	if (NULL != _mgr)
		_pCPELogger = _mgr->getLogger();
	if (NULL == _pCPELogger)
		_pCPELogger = &NullLog;

	_timerWatcher.start();
}

BaseCPHelper::~BaseCPHelper()
{
	_timerWatcher.quit();

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
		return it->second.get();
	return NULL;
}

bool BaseCPHelper::reg(const ::std::string& provisionId, BaseNPVRSession::Ptr pSess)
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
			BaseNPVRSession::Ptr& ptr= it->second;			
			EngHelperLog(ZQ::common::Log::L_WARNING, CLOGFMT(BaseCPHelper, "reg() sess[%s] overwritting old instance:%08x with new instance:%08x "),
				provisionId.c_str() ,ptr , pSess.get());

			it->second = pSess;
		}
		else 
		{
			_sessMap.insert(SessMap::value_type(provisionId, pSess));
		}
	}

	EngHelperLog(ZQ::common::Log::L_DEBUG, CLOGFMT(BaseCPHelper, "reg() sess[%s] instance:%08x in CPE"), provisionId.c_str(), pSess.get());
	_mgr->registerHelperSession(provisionId.c_str(), pSess.get());

	return true;
}

void BaseCPHelper::unreg(const std::string& provisionId)
{
	if (!_bInCleanup)
	{
		try
		{
			ZQ::common::MutexGuard gd(_lockSessMap);
			SessMap::iterator it = _sessMap.find(provisionId);
			if (_sessMap.end() != it)
				_sessMap.erase(it);
		}
		catch(...){ }
	} 

	EngHelperLog(ZQ::common::Log::L_DEBUG, CLOGFMT(BaseCPHelper, "unreg() sess[%s] in CPE"), provisionId.c_str());
	try{
		if (NULL != _mgr)
			_mgr->unregisterHelperSession(provisionId.c_str());
	} catch(...) {}
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

void BaseCPHelper::createTimer( TimerObject* pTimerObj, int nTimeOutInMs )
{
	_timerWatcher.watch(pTimerObj, nTimeOutInMs );
}

void BaseCPHelper::removeTimer( TimerObject* pTimerObj )
{
	_timerWatcher.remove(pTimerObj);
}


// -----------------------------
// class BaseNPVRSession
// -----------------------------
#undef cpelog 
#define cpelog (*_helper._pCPELogger)
BaseNPVRSession::BaseNPVRSession(BaseCPHelper& helper, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess)
: _helper(helper), _sess(pSess), _pPushSource(NULL)
{
	if (pSess)
	{
		_provisionId = _sess->ident.name;
		_helper.reg(_provisionId, this);
	}
	
	_bPreloaded = false;
	_bInited = false;
	_nErrCode = 0;
	
	EngSessLog(ZQ::common::Log::L_DEBUG, CPHSESSLOGFMT(BaseNPVRSession, "BaseNPVRSession() this[0x%08x]"), this);
}

BaseNPVRSession::~BaseNPVRSession()
{
	EngSessLog(ZQ::common::Log::L_DEBUG, CPHSESSLOGFMT(BaseNPVRSession, "~BaseNPVRSession() this[0x%08x]"), this);
}

bool BaseNPVRSession::prime()
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


// access to the ProvisionSession
void BaseNPVRSession::notifyStarted(const ::TianShanIce::Properties& params)
{
	try
	{
		_sess->notifyStarted(params, ::Ice::Current());
		EngSessLog(ZQ::common::Log::L_INFO, CPHSESSLOGFMT(BaseNPVRSession, "notifyStarted()"));
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseNPVRSession, "notifyStarted() caught unknown exception"));
	}	
}

void BaseNPVRSession::notifyStopped(bool errorOccurred, const ::TianShanIce::Properties& params)
{
	::TianShanIce::Properties eventParams = params;
	if (errorOccurred)
	{
		eventParams[EVTPM_ERRORMESSAGE] = _strErrMsg;		
		char tmp[16];
		sprintf(tmp, "%d", _nErrCode);
		eventParams[EVTPM_ERRORCODE] = tmp;
	}

	try
	{
		_sess->notifyStopped(errorOccurred, eventParams, ::Ice::Current());
		EngSessLog(ZQ::common::Log::L_INFO, CPHSESSLOGFMT(BaseNPVRSession, "notifyStopped() errorOccurred=%c"), errorOccurred?'Y':'N');
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseNPVRSession, "notifyStopped() errorOccurred=%c caught unknown exception")
			, errorOccurred?'Y':'N');
	}
}

void BaseNPVRSession::notifyStreamable(bool streamable)
{
	try
	{
		_sess->notifyStreamable(streamable, ::Ice::Current());
		EngSessLog(ZQ::common::Log::L_INFO, CPHSESSLOGFMT(BaseNPVRSession, "notifyStreamable() streamable=%c"), streamable?'Y':'N');
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseNPVRSession, "notifyStreamable() streamable=%c caught unknown exception")
			, streamable?'Y':'N');
	}
}

void BaseNPVRSession::updateProgress(const ::Ice::Long processed, const ::Ice::Long total)
{
	try
	{
		::TianShanIce::Properties params;
		_sess->updateProgress(processed, total, params,::Ice::Current());
	#if 0
		EngSessLog(ZQ::common::Log::L_DEBUG, CPHSESSLOGFMT(BaseNPVRSession, "updateProgress() %lld/%lld"), processed, total);
	#endif // _DEBUG
	}
	catch(...)
	{
		EngSessLog(ZQ::common::Log::L_WARNING, CPHSESSLOGFMT(BaseNPVRSession, "updateProgress() %lld/%lld caught unknown exception")
			, processed, total);
	}
}

void BaseNPVRSession::destroy()
{
	_helper.removeTimer(this);
	_helper.unreg(_provisionId);	
}

}} // namespace


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

