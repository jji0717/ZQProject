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
// Ident : $Id: SentryImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/SentryImpl.cpp $
// 
// 7     3/16/16 10:53a Ketao.zhang
// 
// 6     12/12/13 1:58p Hui.shao
// %lld for int64
// 
// 5     8/29/13 6:00p Zonghuan.xiao
// add lock for _lockExpiredProcessIds while this set  is operating;
// ticket:14047
// 
// 4     7/10/13 1:35p Build
// 
// 3     7/09/13 12:19p Build
// 
// 2     6/07/13 3:25p Zonghuan.xiao
// fix Request 13616: sentry crash
// iterator invalid while vector erase it
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 11    09-07-06 14:05 Fei.huang
// * linux port
// 
// 10    08-06-06 12:06 Xiaohui.chai
// added getWebView()
// 
// 9     08-01-16 17:28 Xiaohui.chai
// 
// 8     07-11-05 15:52 Xiaohui.chai
// LocalAdapterRefreshing's interface changed
// 
// 7     07-11-01 14:40 Xiaohui.chai
// 
// 6     07-10-19 18:12 Xiaohui.chai
// 
// 5     07-09-18 12:56 Hongquan.zhang
// 
// 4     07-06-04 14:45 Hui.shao
// separated html pages from env
// 
// 3     07-05-29 18:47 Hui.shao
// rewrote the ZQAdapter
// 
// 2     07-05-22 17:30 Hui.shao
// added exporting logger information
// ===========================================================================

#include <boost/thread.hpp>
#include "SentryImpl.h"
#include "Log.h"
#include "Guid.h"
#include "SentryCommand.h"

#include "../Ice/TsApplication.h"
#include "../Ice/TsStreamer.h"
#include <Ice/Ice.h>

#ifndef ZQ_OS_MSWIN
extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}
#endif // ZQ_OS_MSWIN

namespace ZQTianShan {
namespace Sentry {

typedef ::std::vector< Ice::Identity > IdentCollection;

// -----------------------------
// loopback service AdapterCollectorImpl
// -----------------------------
AdapterCollectorImpl::AdapterCollectorImpl(SentryEnv& env)
: _env(env)
{

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AdapterCollector, "add the interface \"%s\" on to the loopback adapter"), SERVICE_NAME_AdapterCollector);
    _env._loopbackAdapter->add(this, _env._loopbackAdapter->getCommunicator()->stringToIdentity(SERVICE_NAME_AdapterCollector));
}

AdapterCollectorImpl::~AdapterCollectorImpl()
{
}

static bool isProcessActive(uint32 pid)
{
    bool bActive = false;
#ifdef ZQ_OS_MSWIN
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    // check the handle
    if(hProcess)
    {
        // check the exit code
        DWORD exitCode = 0;
        if(GetExitCodeProcess(hProcess, &exitCode) && (STILL_ACTIVE == exitCode))
            bActive = true;
        CloseHandle(hProcess);
    }
#else
    std::ostringstream oss;
    oss << "/proc/" << pid;

    struct stat buff;
    if(!stat(oss.str().c_str(), &buff)) {
        bActive = true;
    }
#endif
    return bActive;
}
::Ice::Int AdapterCollectorImpl::updateAdapter(::Ice::Int processId, const ::std::string& adapterId, ::Ice::Long lastChange, const ::Ice::Identity& identAdapterCB, const ::Ice::Current& c)
{
	WLock sync(*this);
	
	::Ice::Long stampExp = now() - _env._timeout * 2000;
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AdapterCollector, "updateAdapter() adapter[%s@%d]: lastChange=%lld"), adapterId.c_str(), processId, lastChange);
	LocalAdapterRefreshing* pCmd = NULL;
	
	bool bFoundProcess = false;
    ::ZqSentryIce::AdapterCBPrx callback = ::ZqSentryIce::AdapterCBPrx::uncheckedCast(c.con->createProxy(identAdapterCB));
	try	
	{
		ZQ::common::MutexGuard gd(_env._lockLocalProcesses);
		for (SentryEnv::Processes::iterator prcIt = _env._localProcesses.begin(); prcIt < _env._localProcesses.end(); )
		{
			if (processId == prcIt->processId)
			{
				prcIt->lastHeartbeat = now();
				bFoundProcess = true;
				if (NULL == pCmd)
					pCmd = new LocalAdapterRefreshing(_env, *prcIt, adapterId, lastChange, callback);
				else
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AdapterCollector, "updateAdapter() adapter[%s@%d] detail-refreshing needed but command has been already created. duplicated adapter may be registered"), adapterId.c_str(), processId);
				prcIt++;
				continue; // find the process registered
			}
            // remove inactive process
			if(!isProcessActive(prcIt->processId))
            {
                envlog(ZQ::common::Log::L_WARNING, CLOGFMT(AdapterCollector, "updateAdapter() process[%d] is inactive."), prcIt->processId);
				ZQ::common::MutexGuard gdExpiredProcessIds(_env._lockExpiredProcessIds);
                _env._expiredProcessIds.insert(prcIt->processId);
				prcIt = _env._localProcesses.erase(prcIt);
                continue;
            }
            // remove expired process
			if (prcIt->lastHeartbeat < stampExp)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(AdapterCollector, "updateAdapter() process[%d] expired, lastHeartbeat=%lld, expline=%lld"), prcIt->processId, prcIt->lastHeartbeat, stampExp);
				ZQ::common::MutexGuard gdExpiredProcessIds(_env._lockExpiredProcessIds);
				_env._expiredProcessIds.insert(prcIt->processId);
				prcIt = _env._localProcesses.erase(prcIt);
			}
			else prcIt++;
		}
		
		if (!bFoundProcess)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(AdapterCollector, "updateAdapter() new process[%d] detected: lastChange=%lld"), processId, lastChange);
			SentryEnv::ProcessInfo pinfo;
			pinfo.processId = processId;
			pinfo.lastHeartbeat = now();
			
#pragma message ( __MSGLOC__ "TODO: associate the daemon name by the process id here")
			
			_env._localProcesses.push_back(pinfo);
			if (NULL == pCmd)
				pCmd = new LocalAdapterRefreshing(_env, pinfo, adapterId, lastChange, callback);
			else
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AdapterCollector, "updateAdapter() adapter[%s@%d] detail-refreshing needed but command has been already created. duplicated adapter may be registered"), adapterId.c_str(), processId);

		}
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AdapterCollector, "updateAdapter() exception caught: adapter[%s@%d], lastChange=%lld"), adapterId.c_str(), processId, lastChange);
	}
	
	if (NULL != pCmd)
		pCmd->execute();
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AdapterCollector, "updateAdapter() finished adapter[%s@%d]: detail-refreshing %s needed, %d process expired"), adapterId.c_str(), processId, (NULL!=pCmd)?"is":"isnot", _env._expiredProcessIds.size());
	return _env._timeout;
}

::std::string AdapterCollectorImpl::getRootUrl(const ::Ice::Current& c)
{
	RLock sync(*this);
	return _env._selfInfo.adminRootUrl;
}

// -----------------------------
// public service SentryServiceImpl
// -----------------------------
SentryServiceImpl::SentryServiceImpl(SentryEnv& env)
:_env(env)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "add the interface \"%s\" on to the adapter \"%s\""), ADAPTER_NAME_Sentry, _env._endpoint.c_str());
    _env._adapter->add(this, ADAPTER_NAME_Sentry);
}

SentryServiceImpl::~SentryServiceImpl()
{
}

::std::string SentryServiceImpl::getAdminUri(const ::Ice::Current& c)
{
	RLock sync(*this);
#pragma message ( __MSGLOC__ "TODO: impl here")
	return "";
}

::TianShanIce::State SentryServiceImpl::getState(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "getState()"));
	RLock sync(*this);
	return ::TianShanIce::stInService;
}

void SentryServiceImpl::getGroupAddress(::std::string& McastIp, ::Ice::Int& port, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "getGroupAddress()"));
	RLock sync(*this);
	McastIp = _env._groupAddr.getHostAddress();
	port	= _env._groupPort;
}

void SentryServiceImpl::getProcessorInfo(::std::string& processor, ::Ice::Int& count, ::Ice::Long& frequencyMhz, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "getProcessorInfo()"));
	RLock sync(*this);
	processor = _env._selfInfo.cpu;
	count     = _env._selfInfo.cpuCount;
	frequencyMhz = _env._selfInfo.cpuClockMHz;
}

::std::string SentryServiceImpl::getOSType(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "getOSType()"));
	RLock sync(*this);
	return _env._selfInfo.os;
}

::std::string SentryServiceImpl::getRootUrl(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "getRootUrl()"));
	RLock sync(*this);
	return _env._selfInfo.adminRootUrl;
}

::ZqSentryIce::ServiceInfos SentryServiceImpl::listServices(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "listServices()"));
	RLock sync(*this);
	::ZqSentryIce::ServiceInfos services;

	//step 1. gather all the interfaces associated with adapters
	ZQ::common::MutexGuard gd(_env._lockLocalAdapters);
	for (SentryEnv::Adapters::iterator it = _env._localAdapters.begin(); it < _env._localAdapters.end(); it++)
	{
		for (::TianShanIce::StrValues::iterator snit = it->interfaces.begin(); snit < it->interfaces.end(); snit++)
		{
			if ((*snit).empty())
				continue;
			::ZqSentryIce::ServiceInfo svcinfo;
			svcinfo.name = *snit;
			svcinfo.adapterId = it->adapterId;
			svcinfo.processId = it->processId;
			svcinfo.proxystr  = svcinfo.name + ":" + it->endpoint;
			services.push_back(svcinfo);
		}
	}

	return services;
}

::Ice::Int SentryServiceImpl::stopDaemonByPid(::Ice::Int pid, bool restart, const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "stopDaemonByPid()"));
	WLock sync(*this);
#pragma message ( __MSGLOC__ "TODO: impl here")
	return pid;
}

::ZqSentryIce::WebView SentryServiceImpl::getWebView(const ::Ice::Current& c)
{
    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService, "getWebView()"));
    RLock sync(*this);
    SentryPages::NavNode localEntries = _env._pages.getLocalActiveEntries();

    ::ZqSentryIce::WebView view;
    view.base.name = localEntries.displayname;
    view.base.URL = localEntries.href;
    view.services.reserve(localEntries.children.size());
    std::vector<SentryPages::NavNode>::iterator it;
    for(it = localEntries.children.begin(); it != localEntries.children.end(); ++it)
    {
        ::ZqSentryIce::WebEntry entry;
        entry.name = it->displayname;
        entry.URL = it->href;
        view.services.push_back(entry);
    }

    return view;
}
}} // namespace
