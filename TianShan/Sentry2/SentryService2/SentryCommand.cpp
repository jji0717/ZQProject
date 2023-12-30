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
// Ident : $Id: SentryCommand.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/SentryCommand.cpp $
// 
// 6     3/16/16 10:53a Ketao.zhang
// 
// 5     7/28/15 10:43a Li.huang
// 
// 4     12/12/13 1:58p Hui.shao
// %lld for int64
// 
// 3     8/29/13 6:00p Zonghuan.xiao
// add lock for _lockExpiredProcessIds while this set  is operating;
// ticket:14047
// 
// 2     6/17/13 11:44a Zonghuan.xiao
// vector erase would cause  crash while  iterator invalid
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 17    09-11-18 20:02 Xiaohui.chai
// fixed bug: identity of adapter is wrong
// 
// 16    09-07-06 17:18 Fei.huang
// * linux port
// 
// 15    09-02-18 10:39 Xiaohui.chai
// new parsing implement
// 
// 14    08-11-24 19:18 Xiaohui.chai
// add multithread access protection for PeerInfoRefreshing::_pendingNodes
// 
// 13    08-08-26 17:13 Xiaohui.chai
// Support context variables.
// 
// 12    08-01-18 15:46 Xiaohui.chai
// 
// 11    08-01-18 14:24 Xiaohui.chai
// 
// 10    07-11-14 14:06 Xiaohui.chai
// Prevent duplicate adapter info during LocalAdapterRefreshing().
// 
// 9     07-11-05 15:51 Xiaohui.chai
// changed LocalAdapterRefreshing's interface
// 
// 8     07-10-24 12:15 Xiaohui.chai
// add thread protection to _env._localProcesses during
// LocalAdapterRefreshing::run()
// 
// 7     07-10-23 18:10 Xiaohui.chai
// 
// 6     07-10-19 18:14 Xiaohui.chai
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
#include "SentryCommand.h"
//#include "LogPaserManagement.h"

namespace ZQTianShan {
	namespace Sentry {
		
// -----------------------------
// class SentryCommand
// -----------------------------
SentryCommand::SentryCommand(SentryEnv& env)
	:ThreadRequest(env._thpool), _env(env) 
{
}

// -----------------------------
// class LocalAdapterRefreshing
// -----------------------------

LocalAdapterRefreshing::LocalAdapterRefreshing(
    SentryEnv& env,
    const ::ZQTianShan::Sentry::SentryEnv::ProcessInfo& processInfo,
    const std::string& adapterId,
    const ::Ice::Long lastChange,
    ::ZqSentryIce::AdapterCBPrx callback
    )
    : SentryCommand(env), _processInfo(processInfo), _lastChange(lastChange), _adapterId(adapterId), _callback(callback)
{
}
int LocalAdapterRefreshing::run(void)
{
	bool bNeedFresh = true;
//	SentryEnv::Adapters* pAdpt =NULL;
	SentryEnv::Adapters::iterator adptIt;

	{
        // need two lock here, beware of the danger of deadlock
		ZQ::common::MutexGuard gdAdapters(_env._lockLocalAdapters);
        ZQ::common::MutexGuard gdProcesses(_env._lockLocalProcesses);
		ZQ::common::MutexGuard gdExpiredProcessIds(_env._lockExpiredProcessIds);

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocalAdapterRefreshing, "freshing adapter[%s@%d]; expired process count:%d"), _adapterId.c_str(), _processInfo.processId, _env._expiredProcessIds.size());
		for (adptIt = _env._localAdapters.begin(); adptIt < _env._localAdapters.end(); )
		{
			if (_adapterId == adptIt->adapterId && _processInfo.processId == adptIt->processId)
			{
				if (adptIt->lastChange >= _lastChange)
				{
                    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocalAdapterRefreshing, "no need to refresh adapter[%s@%d] info due to newly updated(%lld >= %lld)"), adptIt->adapterId.c_str(), adptIt->processId, adptIt->lastChange, _lastChange);
					bNeedFresh = false;
					adptIt++;
				}
				else 
				{
                    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocalAdapterRefreshing, "clear adapter[%s@%d] due to adapter updated"), adptIt->adapterId.c_str(), adptIt->processId);

					adptIt = _env._localAdapters.erase(adptIt); // delete the out-of-date record first
				}
				
				continue;
			}
			
			if (_env._expiredProcessIds.end() != _env._expiredProcessIds.find(adptIt->processId))
			{
                envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocalAdapterRefreshing, "clear adapter[%s@%d] due to process expired"), adptIt->adapterId.c_str(), adptIt->processId);

				adptIt = _env._localAdapters.erase(adptIt);
			}
			else
			{
				adptIt++;
			}
        }

        if (!_env._expiredProcessIds.empty())
        {
            SentryEnv::IntSet::const_iterator cit_expiredPid = _env._expiredProcessIds.begin();
            for (; cit_expiredPid != _env._expiredProcessIds.end(); ++cit_expiredPid)
            {
//                _env._logParserManagement->UpdateProcessLogInfo((*cit_expiredPid), ::ZqSentryIce::LoggerInfos());
                envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(LocalAdapterRefreshing,"removed the log information of process[%d]"), (*cit_expiredPid));
            }
            _env._expiredProcessIds.clear();
            _env._selfInfo.lastChange = now();
		}
	}
	
	if (!bNeedFresh)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LocalAdapterRefreshing, "adapter[%s@%d] is up to date"), _adapterId.c_str(), _processInfo.processId);
		return 0;
	}
	
	// refresh the interfaces of the adapter
	SentryEnv::AdapterInfo ainfo;
	
	// step 1. refresh the endpoint and interfaces
	try 
	{
		envlog(ZQ::common::Log::L_INFO,CLOGFMT(LocalAdapterRefreshing,"refresh adapter [%s@%d]"), _adapterId.c_str(), _processInfo.processId);
		ainfo.adapterId = _adapterId;
		ainfo.processId = _processInfo.processId;
		ainfo.endpoint = _callback->getEndpoint();
		ainfo.interfaces = _callback->listInterfaces();
		_processInfo.loggerInfos = _callback->listLoggers();
//		_env._logParserManagement->UpdateProcessLogInfo(_processInfo.processId, _processInfo.loggerInfos);
		//get all logs now,update the log information
        envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(LocalAdapterRefreshing,"updated the log information of process[%d]"), _processInfo.processId);

		ainfo.lastChange = _lastChange;
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(LocalAdapterRefreshing, "failed to query the data from local adapter callback of process[%d]: %s"), _processInfo.processId, ex.ice_name().c_str());
	    return -1;
    }
	
	// step 2. add the new adapter info
	{
		ZQ::common::MutexGuard gd(_env._lockLocalAdapters);

        // prevent duplicate adapter info
        SentryEnv::Adapters::iterator it_adptr;
        for (it_adptr = _env._localAdapters.begin(); it_adptr != _env._localAdapters.end(); ++it_adptr)
        {
            if(it_adptr->adapterId == ainfo.adapterId && it_adptr->processId == ainfo.processId)
            {
                // duplicate adapter detected
                break;
            }
        }
        if(it_adptr == _env._localAdapters.end())
        {
            // add
            _env._localAdapters.push_back(ainfo);
        }
        else
        {
            // update
            (*it_adptr) = ainfo;
        }
	}
	
	// step 3. update the remained per-process data
	{
		ZQ::common::MutexGuard gd(_env._lockLocalProcesses);
		for (SentryEnv::Processes::iterator prcIt = _env._localProcesses.begin(); prcIt != _env._localProcesses.end(); ++prcIt)
		{
			if (_processInfo.processId == prcIt->processId)
			{
				prcIt->loggerInfos = _processInfo.loggerInfos;
				break;
			}
		}
	}
	
	_env._selfInfo.lastChange = _lastChange;
	
	std::string itnames;
	for (::TianShanIce::StrValues::iterator it = ainfo.interfaces.begin(); it<ainfo.interfaces.end(); it++)
		itnames += (*it) + ", ";
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(LocalAdapterRefreshing, "refreshed local adapter[%d@%s] lastChange: %lld; interfaces(%d): %s"), ainfo.processId, ainfo.adapterId.c_str(), ainfo.lastChange, ainfo.interfaces.size(), itnames.c_str());

	return 0;
}

// -----------------------------
// class PeerInfoRefreshing
// -----------------------------
std::set<std::string> PeerInfoRefreshing::_pendingNodes;
ZQ::common::Mutex PeerInfoRefreshing::_pendingNodesLock;
bool PeerInfoRefreshing::isPending(const std::string& nodeId)
{
    ZQ::common::MutexGuard gd(_pendingNodesLock);
    return (_pendingNodes.find(nodeId) != _pendingNodes.end());
}

PeerInfoRefreshing::PeerInfoRefreshing(SentryEnv& env, const ::std::string& nodeId, const std::string& sentrysvcPrx, ::Ice::Long lastChanged)
	: SentryCommand(env), _nodeId(nodeId), _sentrysvcPrx(sentrysvcPrx), _lastChanged(lastChanged)
{
    ZQ::common::MutexGuard gd(_pendingNodesLock);
	_pendingNodes.insert(_nodeId);
}

PeerInfoRefreshing::~PeerInfoRefreshing()
{
    ZQ::common::MutexGuard gd(_pendingNodesLock);
	_pendingNodes.erase(_nodeId);
}

int PeerInfoRefreshing::run(void)
{
#ifndef _DEBUG
	if (0 == _nodeId.compare(_env._selfInfo.id))
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PeerInfoRefreshing, "quit freshing due to peer[%s] is self node"), _nodeId.c_str());
		return 0;
	}
#endif // !_DEBUG
	
	bool bSucc = true;
	
	try
	{
		// connect to the peer Sentry
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PeerInfoRefreshing, "refreshed neighbor: node[%s]; proxy=\"%s\""), _nodeId.c_str(), _sentrysvcPrx.c_str());
		_proxy = ::ZqSentryIce::SentryServicePrx::checkedCast(_env._communicator->stringToProxy(_sentrysvcPrx));
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PeerInfoRefreshing, "failed to connect to peer[%s]: \"%s\": %s"), _nodeId.c_str(), _sentrysvcPrx.c_str(), ex.ice_name().c_str());
		bSucc = false;
	}
	catch (...) { bSucc = false; }
	
	if (_proxy)
	{
		try 
		{
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PeerInfoRefreshing, "start freshing basic information of peer[%s]"), _nodeId.c_str());
			
			std::string vcpu, vos, vurl; ::Ice::Long vlong; Ice::Int vint;
			_proxy->getProcessorInfo(vcpu, vint, vlong);
			vos = _proxy->getOSType();
			vurl = _proxy->getRootUrl();
			::ZQ::common::MutexGuard gd(_env._lockNeighbors);
			if (_env._neighbors.end() != _env._neighbors.find(_nodeId))
			{
				_env._neighbors[_nodeId].cpu    = vcpu;
				_env._neighbors[_nodeId].cpuCount = vint;
				_env._neighbors[_nodeId].cpuClockMHz = (uint32) vlong;
				_env._neighbors[_nodeId].os = vos;
				_env._neighbors[_nodeId].adminRootUrl = vurl;
			}
		}
		catch (const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PeerInfoRefreshing, "failed to refresh base info of peer[%s]: %s"), _nodeId.c_str(), ex.ice_name().c_str());
			bSucc = false;
		}
		catch (...) { bSucc = false; }
		
		::ZqSentryIce::ServiceInfos serviceinfos = _proxy->listServices();
		try 
		{
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PeerInfoRefreshing, "start freshing service information of peer[%s]"), _nodeId.c_str());
			::ZQ::common::MutexGuard gd(_env._lockRemoteServices);
			
			// clear all the old services records of this peer node
			int nCleared=0;
			for (SentryEnv::RemoteServices::iterator it = _env._remoteServices.begin(); it < _env._remoteServices.end();)
			{
				if (0 ==_nodeId.compare(it->nodeid))
				{
					nCleared ++;
					it = _env._remoteServices.erase(it);
				}
				else it ++;
			}
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PeerInfoRefreshing, "cleared all the old services records of this peer[%s]: %d services"), _nodeId.c_str(), nCleared);
			
			// insert the new service info
			for (::ZqSentryIce::ServiceInfos::iterator nit = serviceinfos.begin(); nit < serviceinfos.end(); nit++)
			{
				SentryEnv::RemoteServiceInfo newinfo;
				newinfo.baseInfo = *nit;
				newinfo.nodeid = _nodeId;
				_env._remoteServices.push_back(newinfo);
			}
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PeerInfoRefreshing, "service info of peer[%s] updated: %d services"), _nodeId.c_str(), serviceinfos.size());
		}
		catch (const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PeerInfoRefreshing, "failed to refresh service info of peer[%s]: %s"), _nodeId.c_str(), ex.ice_name().c_str());
			bSucc = false;
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(PeerInfoRefreshing, "failed to refresh service info of peer[%s]: Unknown exception caught"), _nodeId.c_str());
			bSucc = false;
		}
	}
	
	// reset the changed flag if this update is failed, let next heartbeat to retry
	if (!bSucc)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(PeerInfoRefreshing, "failed to fresh peer[%s], leave the freshing to the next heartbeat"), _nodeId.c_str());
		::ZQ::common::MutexGuard gd(_env._lockNeighbors);
		
		if (_env._neighbors.end() != _env._neighbors.find(_nodeId))
			_env._neighbors[_nodeId].lastChange    = _lastChanged - MIN_GROUP_TIMEOUT;
	}
	
//	_env._pages.refreshNeighborInfo();
	
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PeerInfoRefreshing, "fresh peer[%s] finished: %s"), _nodeId.c_str(), bSucc ? "succ" : "fail");
	return bSucc ? 0 : -1;
}

}} // namespace
