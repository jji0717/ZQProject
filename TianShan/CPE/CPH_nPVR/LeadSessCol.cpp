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


#include "LeadSessCol.h"
#include "LeadSessFac.h"
#include "LeadSessI.h"
#include "VirtualSessI.h"
#include "CPH_NPVRCfg.h"

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
	LeadSessInitCmd(LeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool);

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	LeadSessI*	_pLeadSess;
};

LeadSessInitCmd::LeadSessInitCmd(LeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool)
	:ThreadRequest(pool), _pLeadSess(pLeadSess)
{
}

int LeadSessInitCmd::run()
{
	if (!_pLeadSess->isInitialized())
	{
		_pLeadSess->initialize();
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
	LeadSessCloseCmd(LeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool);

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	LeadSessI*	_pLeadSess;
};

LeadSessCloseCmd::LeadSessCloseCmd(LeadSessI*	pLeadSess, ZQ::common::NativeThreadPool& pool)
:_pLeadSess(pLeadSess), ThreadRequest(pool)
{
}

int LeadSessCloseCmd::run()
{
	_pLeadSess->stop();
	
	return 0;
}

LeadSessCol::LeadSessCol(ZQ::common::NativeThreadPool* pool, ZQ::common::BufferPool* pAlloc, FileIoFactory* pFileIoFactory, ZQ::common::Log* pLog)
:_threadPool(pool)
{
	_lsFac = new LeadSessFac(pool, pAlloc, pFileIoFactory);
	_hThreadExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	_log = pLog;
}

LeadSessCol::~LeadSessCol()
{
	close();
}

void LeadSessCol::close()
{
	stopMonitor();

	Guard<Mutex> op(_lock);
	
	stopAllSession();

	if (_lsFac)
	{
		delete _lsFac;
		_lsFac = NULL;
	}
}

void LeadSessCol::stopAllSession()
{
	SubjectListType::iterator iter = _subjectList.begin();
	for(;iter!=_subjectList.end();iter++)
	{
		LeadSessI* pLeadSess = iter->second;
		//
		pLeadSess->uninitialize();
		//delete pLeadSess;
	}

	_subjectList.clear();
}

bool LeadSessCol::startMonitor()
{
	return NativeThread::start();
}

void LeadSessCol::stopMonitor()
{
	if (_hThreadExit == INVALID_HANDLE_VALUE)
		return;

	SetEvent(_hThreadExit);
	NativeThread::waitHandle(_gCPHCfg.leadsesslagAfterIdle);
	CloseHandle(_hThreadExit);
	_hThreadExit = INVALID_HANDLE_VALUE;
}

bool LeadSessCol::startLeadSession(LeadSessI* pLeadSess, VirtualSessI* pVirtualSess)
{
	pLeadSess->setLog(_log);
	pLeadSess->setEventHandle(this);

	MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "startLeadSession() before copy: [%s]"), pLeadSess->getSessionGroup().getPathName().c_str());
	pLeadSess->copy(pVirtualSess);
	MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "startLeadSession() after copy: [%s]"), pLeadSess->getSessionGroup().getPathName().c_str());
	
	//file name diffrent
	pLeadSess->setFilename(pLeadSess->getSessionGroup().getPathName());
	if (!pLeadSess->initialize())
	{
		std::string strErr;
		int nErrCode;

		pLeadSess->getLastError(strErr, nErrCode);
		setLastError(strErr, nErrCode);
		return false;
	}

	pLeadSess->execute();
	return true;
}

bool LeadSessCol::reservation(VirtualSessI* pVirtualSess)
{
	LeadSessI* pLeadSess = NULL;
	{
		Guard<Mutex> op(_lock);
		MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "reservation for virtual [%s]"), pVirtualSess->getSessionGroup().getPathName().c_str());

		pLeadSess = getLeadSession(pVirtualSess);
		if (!pLeadSess)
			return false;		
	}

	pLeadSess->makeReservation(pVirtualSess);
	return true;
}

// find session, create & start session if not exist
LeadSessI* LeadSessCol::getLeadSession(VirtualSessI* pVirtualSess)
{
	SessionGroupId sgId = pVirtualSess->getSessionGroup();

	// find if exist
	LeadSessI* pLeadSess = find(sgId.getValue());
	if (!pLeadSess)
	{
		pLeadSess = create(sgId.getValue());
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

		pLeadSess->copy(pVirtualSess);

		//file name diffrent
		pLeadSess->setFilename(pLeadSess->getSessionGroup().getPathName());
		
		(new LeadSessInitCmd(pLeadSess, *_threadPool))->execute();
	}

	return pLeadSess;
}

bool LeadSessCol::registerObserver(VirtualSessI* pVirtualSess)
{
	LeadSessI* pLeadSess = NULL;
	{
		Guard<Mutex> op(_lock);

		pLeadSess = getLeadSession(pVirtualSess);
		if (!pLeadSess)
			return false;		
	}

	pLeadSess->registerObserver(pVirtualSess);

	if (!pLeadSess->isExecuted())
	{
		pLeadSess->execute();		
	}

	return true;
}

void LeadSessCol::removeObserver(VirtualSessI* pVirtualSess)
{
	SessionGroupId sgId = pVirtualSess->getSessionGroup();

	LeadSessI* pLeadSess = NULL;

	{
		Guard<Mutex> op(_lock);
		pLeadSess = find(sgId.getValue());	
	}

	if (pLeadSess)
	{
		pLeadSess->removeObserver(pVirtualSess);	
	}
}

void LeadSessCol::monitorIdleSession()
{
//	MOLOG(Log::L_DEBUG, CLOGFMT(LeaderCol, "monitor idle session enter"));

	Guard<Mutex> op(_lock);

	std::vector<std::string> toDelete;
	SubjectListType::iterator iter = _subjectList.begin();
	for(;iter!=_subjectList.end();iter++)
	{
		LeadSessI* pLeadSess = iter->second;

		if (!pLeadSess->isIdle())
			continue;
		
		if (pLeadSess->getIdleTime() >= getMaxIdleTime())
		{
			MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "leader session [%s] idled for %d milli-seconds, stop now"),
				pLeadSess->getSessionGroup().getPathName().c_str(), pLeadSess->getIdleTime());

			toDelete.push_back(iter->first);
		}		
	}

	for(std::vector<std::string>::iterator it=toDelete.begin();it!=toDelete.end();it++)
	{
		iter = _subjectList.find(*it);
		if (iter!=_subjectList.end())
		{			
			LeadSessI* pLeadSess = iter->second;
			_subjectList.erase(iter);

			(new LeadSessCloseCmd(pLeadSess, *_threadPool))->execute();
		}
	}

//	MOLOG(Log::L_DEBUG, CLOGFMT(LeaderCol, "monitor idle session leave"));
}

int LeadSessCol::run(void)
{
	while(1)
	{
		DWORD dwRet = WaitForSingleObject(_hThreadExit, _gCPHCfg.monitorInterval);
		if (dwRet != WAIT_TIMEOUT)
			break;

		monitorIdleSession();
	}

	return 0;
}

LeadSessI* LeadSessCol::find(const std::string& uniqueId)
{
	SubjectListType::iterator iter;
	{
		iter = _subjectList.find(uniqueId);
		if (iter != _subjectList.end())
			return iter->second;
		else
			return NULL;
	}
	return NULL;
}
 
LeadSessI* LeadSessCol::create(const std::string& uniqueId)
{
	// check if we reach the max session count
	if (_subjectList.size()>=_nMaxSessionCount)
	{
		char tmp[256];
		sprintf_s(tmp, sizeof(tmp), "exceeded the max leader session count %d", _subjectList.size());
		setLastError(std::string(tmp), 0);
		return NULL;
	}

	LeadSessI* pLeadSess = _lsFac->create();
	if (pLeadSess)
	{
		_subjectList.insert(std::pair<std::string, LeadSessI*>(uniqueId, pLeadSess));
	}
	
	return pLeadSess;
}
 
void LeadSessCol::remove(const std::string & uniqueId)
{
	SubjectListType::iterator iter;
	{
		iter = _subjectList.find(uniqueId);
		if (iter != _subjectList.end())
			_subjectList.erase(iter);
	}
}
 
bool LeadSessCol::getMediaInfo(VirtualSessI* pVirtualSess, MediaInfo& mInfo)
{
	SessionGroupId sgId = pVirtualSess->getSessionGroup();

	ZQ::common::Guard<ZQ::common::Mutex>  opLock(_lock);

	// find if exist
	LeadSessI* pLeadSess = find(sgId.getValue());
	if (!pLeadSess)
		return false;

	return pLeadSess->getMediaInfo(mInfo);
}

std::string LeadSessCol::getLeadSessPathName(VirtualSessI* pVirtualSess)
{
	SessionGroupId sgId = pVirtualSess->getSessionGroup();
	
	ZQ::common::Guard<ZQ::common::Mutex>  opLock(_lock);
	
	LeadSessI* pLeadSess = find(sgId.getValue());
	if (pLeadSess)
	{
		return pLeadSess->getSessionGroup().getPathName();
	}
	else
	{
		MOLOG(Log::L_WARNING, CLOGFMT(LeaderCol, "could not find lead session for session [%s]"), sgId.getSourceFile().c_str());
		return sgId.getPathName();
	}
}

void LeadSessCol::onDestroy(LeadSessI* pLeadSess)
{
	SessionGroupId sgId = pLeadSess->getSessionGroup();

	MOLOG(Log::L_INFO, CLOGFMT(LeaderCol, "leader session [%s] OnDestroy"), sgId.getPathName().c_str());

	ZQ::common::Guard<ZQ::common::Mutex>  opLock(_lock);
	remove(sgId.getValue());
}


}}