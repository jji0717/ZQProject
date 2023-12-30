// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================


#include "AquaRecLeadSessCol.h"
#include "AquaRecLeadSessFac.h"
#include "AquaRecLeadSessI.h"
#include "AquaRecVirtualSessI.h"
#include "CPH_AquaRecCfg.h"

#define MOLOG			(*_log)
#define LeaderCol		"LeaderCol"
#include "NativeThreadPool.h"

using namespace ZQ::common;

namespace ZQTianShan 
{

namespace ContentProvision
{



// -----------------------------
// class LeadSessInitCmd
// -----------------------------
///
class LeadSessInitCmd : protected ZQ::common::ThreadRequest
{
protected:
	virtual ~LeadSessInitCmd(){};

public:

	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
	LeadSessInitCmd(AquaRecLeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool);

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	AquaRecLeadSessI*	_pLeadSess;
};

LeadSessInitCmd::LeadSessInitCmd(AquaRecLeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool)
	:ThreadRequest(pool), _pLeadSess(pLeadSess)
{
}

int LeadSessInitCmd::run()
{
	if (!_pLeadSess->isInitialized())
	{
		_pLeadSess->initialize();
	}

	if (!_pLeadSess->isExecuted())
	{
		_pLeadSess->execute();		
	}

	return 0;
}


// -----------------------------
// class LeadSessCloseCmd
// -----------------------------
///
class LeadSessCloseCmd : protected ZQ::common::ThreadRequest
{
protected:
	virtual ~LeadSessCloseCmd(){};

public:
	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
	LeadSessCloseCmd(AquaRecLeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool);

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	AquaRecLeadSessI*	_pLeadSess;
};

LeadSessCloseCmd::LeadSessCloseCmd(AquaRecLeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool)
:_pLeadSess(pLeadSess), ThreadRequest(pool)
{
}

int LeadSessCloseCmd::run()
{
	_pLeadSess->stop();
	
	return 0;
}
// -----------------------------
// class LeadSessUpdate
// -----------------------------
class LeadSessUpdate : protected ZQ::common::ThreadRequest
{
protected:
	~LeadSessUpdate(){};
public:
	LeadSessUpdate(AquaRecVirtualSessI* pVirtualSess ,AquaRecLeadSessI*	pLeadSess,ZQ::common::NativeThreadPool& pool);
	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	AquaRecLeadSessI*	_pLeadSess;
	AquaRecVirtualSessI*	_pVirtualSess;
};
LeadSessUpdate::LeadSessUpdate(AquaRecVirtualSessI* pVirtualSess,AquaRecLeadSessI*	pLeadSess ,ZQ::common::NativeThreadPool& pool)
:_pVirtualSess(pVirtualSess),_pLeadSess(pLeadSess),ThreadRequest(pool)
{
}
int LeadSessUpdate::run()
{
	_pLeadSess->updateScheduledTime(_pVirtualSess);
	return  0;
}
AquaRecLeadSessCol::AquaRecLeadSessCol(ZQ::common::NativeThreadPool* pool, ZQ::common::Log* pLog, CdmiClientBase* cdmiClient)
:_threadPool(pool)
{
	_lsFac = new AquaRecLeadSessFac(pool, cdmiClient);
	/*_hThreadExit = CreateEvent(NULL, TRUE, FALSE, NULL);*/
	_log = pLog;
	_leadSessLists.clear();
}

AquaRecLeadSessCol::~AquaRecLeadSessCol()
{
	close();
}

void AquaRecLeadSessCol::close()
{
	stopMonitor();

	Guard<Mutex> op(_lockLeadsessLists);
	
	stopAllSession();

	if (_lsFac)
	{
		delete _lsFac;
		_lsFac = NULL;
	}
}

void AquaRecLeadSessCol::stopAllSession()
{
	LeadSessList::iterator iter = _leadSessLists.begin();

	for(;iter!=_leadSessLists.end();iter++)
	{
		AquaRecLeadSessI* pLeadSess = iter->second;
		//
		pLeadSess->uninitialize();
		//delete pLeadSess;
	}

	_leadSessLists.clear();
}

bool AquaRecLeadSessCol::startMonitor()
{
	return NativeThread::start();
}

void AquaRecLeadSessCol::stopMonitor()
{
//   	if (_hThreadExit == INVALID_HANDLE_VALUE)
//   		return;

	//SetEvent(_hThreadExit);
	_hThreadQuited.signal();
	NativeThread::waitHandle(_gCPHCfg.leadsesslagAfterIdle);
 	//CloseHandle(_hThreadExit);
 	//_hThreadExit = INVALID_HANDLE_VALUE;
}

bool AquaRecLeadSessCol::addVirtualSession(AquaRecVirtualSessI* pVirtualSess)
{
	AquaRecLeadSessI* pLeadSess = NULL;
	{
		Guard<Mutex> op(_lockLeadsessLists);
		MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "[%s]reservation for VirSess [%s]"),pVirtualSess->getChannnelName().c_str(), pVirtualSess->getContentId().c_str());

		pLeadSess = getLeadSession(pVirtualSess);
		if (!pLeadSess)
			return false;		
	}

	if(!pLeadSess->add(pVirtualSess))
		return false;
	return true;
}
bool AquaRecLeadSessCol::removeVirtualSession(AquaRecVirtualSessI* pVirtualSess)
{
	AquaRecLeadSessI* pLeadSess = NULL;
	{
		Guard<Mutex> op(_lockLeadsessLists);
		MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "[%s]remove for VirSess [%s]"), pVirtualSess->getChannnelName().c_str(), pVirtualSess->getContentId().c_str());

		pLeadSess = getLeadSession(pVirtualSess);
		if (!pLeadSess)
			return false;		
	}

	pLeadSess->remove(pVirtualSess);
	return true;
}
bool AquaRecLeadSessCol::updateScheduledTime(AquaRecVirtualSessI* pVirtualSess)
{
	MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "updateScheduledTime for VirSess [%s]"),  pVirtualSess->getContentId().c_str());
	AquaRecLeadSessI* pLeadSess = NULL;
	{
		Guard<Mutex> op(_lockLeadsessLists);
		pLeadSess = getLeadSession(pVirtualSess);
		if (!pLeadSess)
			return false;
	}
	(new LeadSessUpdate(pVirtualSess,pLeadSess, *_threadPool))->execute();
	//pLeadSess->updateScheduledTime(pVirtualSess);
	return true;
}
// find session, create & start session if not exist
AquaRecLeadSessI* AquaRecLeadSessCol::getLeadSession(AquaRecVirtualSessI* pVirtualSess)
{
	std::string chName = pVirtualSess->getChannnelName();

	// find if exist
	AquaRecLeadSessI* pLeadSess = find(chName);
	if (!pLeadSess)
	{
		pLeadSess = create(chName);
		if (!pLeadSess)
		{
			std::string strErr;
			int nErrCode;

			getLastError(nErrCode, strErr);
			pVirtualSess->setLastError(strErr, nErrCode);
			return false;		
		}

		pLeadSess->setLog(_log);
		pLeadSess->setEventHandle(this);
		pLeadSess->setChannelName(chName);
//		pLeadSess->setBitrateContainers();
		(new LeadSessInitCmd(pLeadSess, *_threadPool))->execute();
		MOLOG(Log::L_INFO,CLOGFMT(LeaderCol,"getLeadSession()  initialize a new LeadSess success with chName [%s]"),chName.c_str());
	}

	return pLeadSess;
}

void AquaRecLeadSessCol::monitorIdleSession()
{
//	MOLOG(Log::L_DEBUG, CLOGFMT(LeaderCol, "monitor idle session enter"));

	Guard<Mutex> op(_lockLeadsessLists);

	std::vector<std::string> toDelete;
	LeadSessList::iterator iter = _leadSessLists.begin();
	for(;iter!=_leadSessLists.end();iter++)
	{
		AquaRecLeadSessI* pLeadSess = iter->second;

		if (!pLeadSess->isIdle())
			continue;
		
		if (pLeadSess->getIdleTime() >= getMaxIdleTime())
		{
			MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "LeadSess [%s] idled for %d milli-seconds, stop now"),
				pLeadSess->getChannelName().c_str(), pLeadSess->getIdleTime());

			toDelete.push_back(iter->first);
		}		
	}

	for(std::vector<std::string>::iterator it=toDelete.begin();it!=toDelete.end();it++)
	{
		iter = _leadSessLists.find(*it);
		if (iter!=_leadSessLists.end())
		{			
			AquaRecLeadSessI* pLeadSess = iter->second;
			_leadSessLists.erase(iter);

			(new LeadSessCloseCmd(pLeadSess, *_threadPool))->execute();
		}
	}

//	MOLOG(Log::L_DEBUG, CLOGFMT(LeaderCol, "monitor idle session leave"));

}

int AquaRecLeadSessCol::run(void)
{
	while(1)
	{
		//DWORD dwRet = WaitForSingleObject(_hThreadExit, _gCPHCfg.monitorInterval);
		SYS::SingleObject::STATE sRet = _hThreadQuited.wait(_gCPHCfg.monitorInterval);
		if (sRet != SYS::SingleObject::TIMEDOUT)
			break;

		monitorIdleSession();
	}

	return 0;
}

AquaRecLeadSessI* AquaRecLeadSessCol::find(const std::string& chId)
{
	LeadSessList::iterator iter;
	{
		iter = _leadSessLists.find(chId);
		if (iter != _leadSessLists.end())
			return iter->second;
		else
			return NULL;
	}
	return NULL;
}
 
AquaRecLeadSessI* AquaRecLeadSessCol::create(const std::string& chId)
{
	// check if we reach the max session count
	if (_leadSessLists.size()>=_nMaxSessionCount)
	{
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "exceeded the max LeadSess count %d", _leadSessLists.size());
		setLastError(std::string(tmp), 0);
		return NULL;
	}
	
	AquaRecLeadSessI* pLeadSess = _lsFac->create();
	if (pLeadSess)
	{
		_leadSessLists[chId] = pLeadSess;
		//MOLOG(Log::L_INFO,CLOGFMT(LeaderCol,"create()  create LeadSess [%s] success with chId [%s]"),pLeadSess->getChannelName().c_str());
	}
	
	return pLeadSess;
}
 
void AquaRecLeadSessCol::remove(const std::string & chId)
{
	LeadSessList::iterator iter;
	{
		iter = _leadSessLists.find(chId);
		if (iter != _leadSessLists.end())
			_leadSessLists.erase(iter);
	}
}
 
bool AquaRecLeadSessCol::getMediaInfo(AquaRecVirtualSessI* pVirtualSess, MediaInfo& mInfo)
{
	// find if exist
	AquaRecLeadSessI* pLeadSess = find(pVirtualSess->getChannnelName());
	if (!pLeadSess)
		return false;

	return pLeadSess->getMediaInfo(mInfo);;
}

void AquaRecLeadSessCol::onDestroy(AquaRecLeadSessI* pLeadSess)
{
	std::string chName = pLeadSess->getChannelName();

	MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "LeadSess [%s] OnDestroy"), chName.c_str());

	Guard<Mutex> op(_lockLeadsessLists);
	remove(chName);
}
}
}
