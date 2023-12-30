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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
//
// ===========================================================================

#include "TianShanDefines.h"
#include "CPCClient.h"
#include "Log.h"
#include "CPECfg.h"
#include "CPE.h"


using namespace std;
using namespace ZQ::common;

#define CPCClt			"CPCClt"
#define MOLOG					(glog)

#define MAX_INST_LEASE_TERM		60*1000	//60 seconds


CPCClient::CPCClient()
{
	_stampStartUp = 0;
#ifdef ZQ_OS_MSWIN
	_stopEvent = INVALID_HANDLE_VALUE;
#endif
	_bStop = false;
	_dwInstanceLeaseTermSecs = 5;
}

bool CPCClient::initModule(Ice::CommunicatorPtr ic)
{
	_bStop = false;
#ifdef ZQ_OS_MSWIN
	_stopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	sem_init(&_stopSem,0,0);
#endif
	_stampStartUp = ZQTianShan::now();

	if (!_gCPECfg._cpcEndPoint[0])
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPCClt, "key [CPCEndPoint] not set"));
		return false;
	}
	
	char szProxyEndPoint[256];
	sprintf(szProxyEndPoint, "%s:%s", SERVICE_NAME_ContentProvisionCluster, _gCPECfg._cpcEndPoint);
	
	bool bSucc = true;
	
	try {
		_ic = ic;

		std::string strContentProvisionSvc = std::string(SERVICE_NAME_ContentProvisionService ":") + _gCPECfg._cpeEndPoint;
		MOLOG(Log::L_DEBUG, CLOGFMT(CPCClt, "openning CPE interface with endpoint [%s]"), strContentProvisionSvc.c_str());	
		_cpePrx = ::TianShanIce::ContentProvision::ContentProvisionEnginePrx::checkedCast(_ic->stringToProxy(strContentProvisionSvc));
		MOLOG(Log::L_INFO, CLOGFMT(CPCClt, "CPE interface opened successful with endpoint[%s]"), strContentProvisionSvc.c_str());	

		MOLOG(Log::L_INFO, CLOGFMT(CPCClt, "connecting CPC with endpoint[%s]"), szProxyEndPoint);	
		Ice::ObjectPrx base = _ic->stringToProxy(szProxyEndPoint);
		_cpcPrx = ::TianShanIce::ContentProvision::ContentProvisionClusterPrx::checkedCast(base);
		if (!_cpcPrx)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPCClt, "Cann't connect to CPC with endpoint %s, please check the setting or network")
				, szProxyEndPoint);
			bSucc = false;	
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPCClt, "CPC[%s] connected"), szProxyEndPoint);	
		}
	} catch (const Ice::Exception& e) {		
		MOLOG(Log::L_WARNING, CLOGFMT(CPCClt, "Caught Ice::Exception: %s while connect to CPC with endpoint %s"),
			e.ice_name().c_str(), szProxyEndPoint);
		bSucc = false;
	}
	
	if (!bSucc)
	{
		try
		{
			Ice::ObjectPrx base = _ic->stringToProxy(szProxyEndPoint);
			_cpcPrx = ::TianShanIce::ContentProvision::ContentProvisionClusterPrx::uncheckedCast(base);
			if (!_cpcPrx)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CPCClt, "Failed to create CPC interface with endpoint %s, please check the setting or network")
					, _gCPECfg._cpcEndPoint);
			}
		}
		catch (const Ice::Exception& e)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPCClt, "Failed to create CPC interface with endpoint %s, Caught Ice::Exception: %s while connect to CPC"),
				szProxyEndPoint, e.ice_name().c_str());
		}
		
	}

	_dwInstanceLeaseTermSecs = 5;	
	
	start();
	MOLOG(Log::L_INFO, CLOGFMT(CPCClt, "CPCClient initialized successful"));	
	return true;
}

void CPCClient::unInitModule()
{
	_bStop = true;
#ifdef ZQ_OS_MSWIN
	if (_stopEvent&&_stopEvent!=INVALID_HANDLE_VALUE)
	{
		SetEvent(_stopEvent);
		waitHandle(INFINITE);
		CloseHandle(_stopEvent);
		_stopEvent = NULL;
	}
#else
	int nVal = 0;
	try
	{
		if(sem_getvalue(&_stopSem,&nVal) != -1)
		{
			sem_post(&_stopSem);
			waitHandle(2000);
			sem_destroy(&_stopSem);
		}
	}
	catch(...){}
#endif
	if (_ic)
	{
		try
		{
			_cpcPrx = NULL;
			_ic->destroy();
		}
		catch (const Ice::Exception&) 
		{			
		}
		_ic = NULL;
		
		MOLOG(Log::L_INFO, CLOGFMT(CPCClt, "CPCClient uninitialized"));
	}	
}

int CPCClient::run()
{
#ifdef ZQ_OS_MSWIN
	MOLOG(Log::L_DEBUG, CLOGFMT(CPCClt, "CPCClient thread enter, thread id [%04x]"), GetCurrentThreadId());
#else
	MOLOG(Log::L_DEBUG, CLOGFMT(CPCClt, "CPCClient thread enter, thread id [%04x]"), pthread_self());
#endif

	int nTimeWait;
	int nErrorOutputCount = 0;
	int64 timeNow,timeLast;
	timeLast = ZQTianShan::now();	

	std::string netId = _gCPECfg._cpeNetId;

	while(!_bStop)
	{
		timeLast = ZQTianShan::now();
		try
		{
			_dwInstanceLeaseTermSecs = _cpcPrx->reportEngine(netId, _cpePrx, _stampStartUp);
			if (nErrorOutputCount)
			{
				MOLOG(Log::L_INFO, CLOGFMT(CPCClt, "reportEngine() successful, CPC connected"));
				nErrorOutputCount = 0;
			}
		}
		catch (const Ice::Exception& e) {		

			// LOG it
			if (nErrorOutputCount < 3)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(CPCClt, "failed to reportEngine() to CPC with error: [caught Ice::Exception: %s]"), e.ice_name().c_str());
			}
			
			nErrorOutputCount++;
		}
			
		if (_bStop)
			break;
		timeNow = ZQTianShan::now();
		if(timeNow > timeLast)
		{
			nTimeWait = _dwInstanceLeaseTermSecs * 1000 - (int)(timeNow - timeLast);
			if (nTimeWait < 0)
			{
				nTimeWait = 0;
			}

			if (nTimeWait>MAX_INST_LEASE_TERM)
				nTimeWait = MAX_INST_LEASE_TERM;
		}
		else
		{
			//maybe the GetTickCount() reset, run more than 46 days
			nTimeWait = _dwInstanceLeaseTermSecs * 1000;
		}
#ifdef ZQ_OS_MSWIN		
		DWORD dwRet = WaitForSingleObject(_stopEvent, nTimeWait);
		if (dwRet == WAIT_OBJECT_0)
		{
			// exist the thread
			break;
		}
		else if (dwRet != WAIT_TIMEOUT)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPCClt, "WaitForSingleObject(_stopEvent) failed with code %d"), GetLastError());
			break;
		}
#else
		struct timespec ts;
		struct timeval tmval;
		gettimeofday(&tmval,(struct timezone*)NULL);
		
		int64 nMicro = nTimeWait*1000ll + tmval.tv_usec;
		ts.tv_sec = tmval.tv_sec + nMicro/1000000;
		ts.tv_nsec = (nMicro%1000000) * 1000;
		int nRet = sem_timedwait(&_stopSem,&ts);
		if(nRet == 0)//wait semaphone
			break;
		else if(errno != ETIMEDOUT && errno != EINTR)//not time out,have a error
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPCClt, "sem_timedwait(_stopSem) failed with code %d"), errno);
			break;
		}
#endif
	}
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPCClt, "CPCClient thread leave"));
	
	return 0;
}
