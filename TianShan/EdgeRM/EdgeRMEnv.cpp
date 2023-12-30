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
// Ident : $Id: EdgeRMEnv.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMEnv.cpp $
// 
// 88    6/06/16 5:20p Li.huang
// ETV-TV-NOW-CR038
// 
// 87    1/11/16 5:50p Dejian.fei
// 
// 86    4/01/15 4:58p Build
// cleaned old snmp
// 
// 85    5/13/14 1:47p Zonghuan.xiao
// snmp export of ERM, include ErmDevicesTable ,ErmPortsTable  ,
// ErmChannelsTable
// 
// 84    1/20/14 4:20p Bin.ren
// 
// 83    1/16/14 1:31p Bin.ren
// 
// 81    1/15/14 3:48p Bin.ren
// 
// 80    1/14/14 5:29p Bin.ren
// 
// 79    1/14/14 5:08p Bin.ren
// 
// 78    1/14/14 5:05p Bin.ren
// 
// 77    1/14/14 1:07p Bin.ren
// 
// 76    1/13/14 2:25p Bin.ren
// 
// 75    1/10/14 10:27a Bin.ren
// add DB operation threads
// 
// 74    11/29/13 5:36p Ketao.zhang
// 
// 73    11/27/13 2:28p Ketao.zhang
// 1, EERM Fail back should not sleep 2min
// 2, map the netId and routeName before import device
// 3, change export remote alloctions to diffAllocations
// 4,  change getAllocs to getAlllocIds
//
// 72    11/18/13 4:51p Bin.ren
// 
// 71    11/14/13 3:39p Bin.ren
// 
// 70    11/13/13 5:07p Bin.ren
// 
// 69    11/13/13 1:32p Bin.ren
// 
// 68    11/08/13 4:40p Ketao.zhang
// 
// 67    11/08/13 4:33p Ketao.zhang
// 
// 66    11/08/13 4:28p Ketao.zhang
// add get netId through routeName
// 
// 65    11/07/13 3:58p Bin.ren
// 
// 64    11/05/13 10:40a Bin.ren
// 
// 63    11/04/13 10:46a Bin.ren
// 
// 62    11/01/13 3:43p Ketao.zhang
// 
// 61    10/31/13 3:55p Ketao.zhang
// 
// 60    10/25/13 5:35p Bin.ren
// add configuration of sync alloctions timeout
// 
// 59    10/24/13 3:52p Bin.ren
// 
// 58    10/24/13 2:34p Bin.ren
// 
// 57    10/22/13 5:26p Bin.ren
// 
// 56    10/22/13 2:30p Bin.ren
// 
// 55    10/21/13 5:42p Ketao.zhang
// 
// 54    10/21/13 3:27p Bin.ren
// 
// 53    10/18/13 4:51p Bin.ren
// 
// 52    10/18/13 2:40p Ketao.zhang
// 
// 51    10/18/13 11:43a Ketao.zhang
// 
// 50    10/18/13 11:03a Bin.ren
// 
// 49    10/18/13 9:33a Bin.ren
// 
// 48    10/16/13 5:55p Bin.ren
// add sync deivces and routeName
// 
// 47    10/15/13 5:22p Bin.ren
// change ImportDeviceCmd from import only one device to import all
// devices in xml file
// 
// 46    10/15/13 9:49a Ketao.zhang
// 
// 45    10/15/13 9:39a Hui.shao
// consts
// 
// 44    10/11/13 3:15p Hui.shao
// merged the two sync-ers into one
// 
// 43    10/11/13 11:53a Hui.shao
// rename ERMSync-ers
// 
// 42    10/11/13 11:26a Hui.shao
// 
// 41    10/11/13 10:59a Bin.ren
// 
// 40    9/24/13 4:33p Li.huang
// 
// 39    9/24/13 4:15p Li.huang
// 
// 38    9/24/13 2:42p Li.huang
// sync allocation
// 
// 37    9/16/13 5:25p Bin.ren
// 
// 36    9/16/13 2:00p Bin.ren
// 
// 35    9/13/13 2:24p Bin.ren
// 
// 34    9/12/13 4:31p Li.huang
// 
// 33    9/11/13 4:53p Li.huang
// 
// 32    9/11/13 1:26p Li.huang
// marshal alloction
// 
// 31    9/05/13 2:56p Bin.ren
// 
// 30    9/03/13 3:14p Bin.ren
// 
// 29    7/10/13 11:13a Build
// 
// 28    6/25/13 3:30p Li.huang
// 
// 27    6/25/13 2:38p Li.huang
// fix bug 18205
// 
// 26    6/18/13 11:35a Li.huang
// 
// 25    6/08/13 11:23a Li.huang
// 
// 24    5/27/13 2:52p Bin.ren
// 
// 23    5/24/13 3:10p Bin.ren
// 
// 22    5/23/13 4:00p Li.huang
// 
// 21    5/22/13 3:40p Li.huang
// add it to linux build
// 
// 20    4/15/13 5:23p Li.huang
// 
// 19    4/15/13 5:19p Li.huang
// 
// 14    3/28/13 1:49p Bin.ren
// 
// 13    3/25/13 2:21p Bin.ren
// 
// 12    3/20/13 11:19a Li.huang
// add R6
// 
// 11    3/01/13 2:20p Li.huang
// 
// 10    2/26/13 2:03p Li.huang
// 
// 9     2/06/13 4:12p Hui.shao
// 
// 8     2/06/13 4:06p Hui.shao
// corrected persistent/runtime DB locations,  DB_CONFIG generating
// 
// 7     1/16/13 10:11a Li.huang
// add tripserver
// 
// 6     11/02/12 2:50p Li.huang
// 
// 5     11/01/12 3:29p Li.huang
// 
// 4     10/31/12 2:47p Li.huang
// 
// 3     10/25/12 2:59p Li.huang
// updata s6SessionGroup
// 
// 2     1/20/11 4:32p Li.huang
// add D6
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 23    10-06-01 16:04 Li.huang
// add minKey 
// 
// 22    10-02-09 15:28 Li.huang
// 
// 21    09-12-23 21:12 Li.huang
// 
// 20    09-12-22 16:02 Li.huang
// remove evictor lock
// 
// 19    09-12-18 9:33 Li.huang
// restore
// 
// 19    09-12-10 18:20 Li.huang
// remove deviceOfchannel Index 
// 
// 18    09-11-26 15:42 Li.huang
// fix some bugs 
// 
// 17    09-11-24 17:39 Li.huang
// 
// 16    09-11-20 17:02 Li.huang
// 
// 15    09-11-18 16:51 Li.huang
// 
// 14    09-11-18 16:04 Li.huang
// fix some bugs
// 
// 13    09-10-30 11:58 Li.huang
// 
// 12    09-10-22 17:45 Li.huang
// 
// 11    09-10-15 18:25 Li.huang
// 
// 10    09-09-28 16:10 Li.huang
// 
// 9     09-08-10 16:49 Xiaoming.li
// 
// 8     09-08-10 16:21 Xiaoming.li
// 
// 7     09-08-10 14:38 Xiaoming.li
// add S6 message handler
// 
// 6     09-08-07 9:54 Xiaoming.li
// add S6 handler
// 
// 5     09-07-27 17:00 Xiaoming.li
// add lock for allocation
// 
// 4     09-04-08 11:48 Hui.shao
// 
// 3     09-03-19 17:12 Hui.shao
// init draft of evaluate, commit and withdraw,
// plus the states of allocation
// 
// 2     09-03-05 19:41 Hui.shao
// defined program structure to impl
// 
// 1     09-02-26 17:53 Hui.shao
// initial created
// ===========================================================================

#include "EdgeRMEnv.h"
#include "EdgeRMService.h" 
#include "Log.h"
// #include "SiteAdminImpl.h"
#include "EdgeRM.h"
#include "EdgeRMMsgHandler.h"
#include "EdgeRMCfgLoader.h"
#include "urlstr.h"

#ifndef ZQ_OS_MSWIN
extern "C" {
#include <sys/stat.h>
}
#endif // ZQ_OS_MSWIN


extern 	ZQ::common::Config::Loader<ZQTianShan::EdgeRM::EdgeRMCfgLoader > pConfig;

namespace ZQTianShan {
namespace EdgeRM {

#define ADAPTER_NAME_EdgeRM   "EdgeRM"
#define INTERFACE_NAME_AllocationOwner "AllocationOwner"
#define ERM_USER_AGENT        "EdgeRMService"
// -----------------------------
// class ERMISGWatchDog
// -----------------------------
QamSGWatchDog::QamSGWatchDog(EdgeRMEnv& env)
: _env(env),_bQuit(false)
{
}

QamSGWatchDog::~QamSGWatchDog()
{	
	//exit thread
	terminate(0);

	{
		ZQ::common::MutexGuard gd(_lockERMIGroups);
		_ERMIGroupsToSync.clear();
	}
}

int QamSGWatchDog::terminate(int code)
{
	_event.signal();
	//wait until the run function exit
	waitHandle(100000);

	return 1;
}

void QamSGWatchDog::quit()
{
	_bQuit = true;
	_event.signal();
	waitHandle(10000);
}

int QamSGWatchDog::run()
{
	while (!_bQuit)
	{
		//		Ice::Long stampNow = now();
		Ice::Long _nextWakeup = 0;

		if (0 != pConfig.ermiClient.enable)
		{
			ZQ::common::MutexGuard gd(_lockERMIGroups);
			for (ERMISyncMap::iterator iter = _ERMIGroupsToSync.begin(); !_bQuit && iter != _ERMIGroupsToSync.end(); iter++)
			{
				ERMISessionGroup::Ptr group = iter->first;
				if (group)
				{
					group->OnTimer();
					if (0 == _nextWakeup )
						_nextWakeup = group->getLastSync() + group->getSyncInterval();
					else
						_nextWakeup = (_nextWakeup > (group->getLastSync() + group->getSyncInterval())) ? (group->getLastSync() + group->getSyncInterval()) : _nextWakeup;
				}
			}
		}

		if (_bQuit)
			continue;

		if (0 != pConfig.r6Client.enable)
		{
			ZQ::common::MutexGuard gd(_lockR6Groups);
			for (R6SyncMap::iterator iter = _R6GroupsToSync.begin(); !_bQuit && iter != _R6GroupsToSync.end(); iter++)
			{
				R6SessionGroup::Ptr group = iter->first;
				if (group)
				{
					group->OnTimer();
					if (0 == _nextWakeup )
						_nextWakeup = group->getLastSync() + group->getSyncInterval();
					else
						_nextWakeup = (_nextWakeup > (group->getLastSync() + group->getSyncInterval())) ? (group->getLastSync() + group->getSyncInterval()) : _nextWakeup;
				}
			}
		}

		long sleepTime = (long) (_nextWakeup - now());

		if (sleepTime < 100)
			sleepTime = 100;

		if (_bQuit)
			continue;

		_event.wait(sleepTime);
	}

	return 1;
}

void QamSGWatchDog::watch(ERMISessionGroup::Ptr group, ::Ice::Long syncInterval)
{
	{
		if (syncInterval < 0)
			syncInterval = 0;
		::Ice::Long newSync = now() + syncInterval;

		ZQ::common::MutexGuard gd(_lockERMIGroups);
		_ERMIGroupsToSync.insert(std::make_pair(group, newSync));
	}

	_event.signal();
}

void QamSGWatchDog::unwatch(ERMISessionGroup::Ptr group)
{
	ZQ::common::MutexGuard gd(_lockERMIGroups);
	ERMISyncMap::iterator iter = _ERMIGroupsToSync.find(group);
	if (iter != _ERMIGroupsToSync.end())
		_ERMIGroupsToSync.erase(group);
}

void QamSGWatchDog::watch(R6SessionGroup::Ptr group,::Ice::Long syncInterval)
{
	{
		if (syncInterval < 0)
			syncInterval = 0;
		::Ice::Long newSync = now() + syncInterval;

		ZQ::common::MutexGuard gd(_lockR6Groups);
		_R6GroupsToSync.insert(std::make_pair(group, newSync));
	}

	_event.signal();
}

void QamSGWatchDog::unwatch(R6SessionGroup::Ptr group)
{
	ZQ::common::MutexGuard gd(_lockR6Groups);
	R6SyncMap::iterator iter = _R6GroupsToSync.find(group);
	if (iter != _R6GroupsToSync.end())
		_R6GroupsToSync.erase(group);
}

// -----------------------------
// class S6Connection
// -----------------------------
std::map<std::string, S6Connection::Ptr> S6Connection::_S6Connections;
ZQ::common::Mutex S6Connection::_lockS6Sessions;

S6Connection::S6Connection(EdgeRMEnv& env, const std::string groupName): 
_env(env), _groupName(groupName)
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6Connection, "[%s]create S6Connection"), _groupName.c_str());
	_sessConnectionIds.clear();
}

S6Connection::~S6Connection()
{
	ZQ::common::MutexGuard g(_lockSessConnectionIds);
	_sessConnectionIds.clear();

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6Connection, "[%s]destory S6Connection"), _groupName.c_str());
}

std::string S6Connection::getGroupName()const 
{
	return _groupName;
}

size_t S6Connection::getSessionList(std::vector <std::string>& sessions)
{
	ZQ::common::MutexGuard g(_lockSessConnectionIds);
	for (SessionConnections::iterator itor = _sessConnectionIds.begin(); itor != _sessConnectionIds.end(); ++itor)
	{
		sessions.push_back(itor->first);
	}

	return sessions.size();
}

void S6Connection::add(std::string sessionId, ZQ::DataPostHouse::IDataCommunicatorPtr connId)
{
	if (connId)
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6Connection, "[%s]add session[%s]CONN["FMT64U"][0x%08x] "), _groupName.c_str(), sessionId.c_str(), connId->getCommunicatorId(), connId.get());

	ZQ::common::MutexGuard g(_lockSessConnectionIds);
	SessionConnections::iterator itor  = _sessConnectionIds.find(sessionId);
	if (itor != _sessConnectionIds.end())
		itor->second = connId;
	else
		_sessConnectionIds[sessionId] = connId;
}

void S6Connection::update(std::string sessionId, ZQ::DataPostHouse::IDataCommunicatorPtr connId)
{
	if (connId)
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Connection, "[%s]update session[%s]CONN["FMT64U"][0x%08x] "), _groupName.c_str(), sessionId.c_str(), connId->getCommunicatorId(), connId.get());
	ZQ::common::MutexGuard g(_lockSessConnectionIds);
	SessionConnections::iterator itor  = _sessConnectionIds.find(sessionId);
	if (itor != _sessConnectionIds.end())
		itor->second = connId;
}

void S6Connection::remove(std::string sessionId)
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6Connection, "[%s]remove session[%s]"), _groupName.c_str(), sessionId.c_str());
	ZQ::common::MutexGuard g(_lockSessConnectionIds);
	SessionConnections::iterator itor  = _sessConnectionIds.find(sessionId);
	if (itor != _sessConnectionIds.end())
		_sessConnectionIds.erase(itor);
}

ZQ::DataPostHouse::IDataCommunicatorPtr S6Connection::findConnectionId(std::string sessionId)
{
	ZQ::common::MutexGuard g(_lockSessConnectionIds);
	SessionConnections::iterator itor  = _sessConnectionIds.find(sessionId);
	if (itor != _sessConnectionIds.end())
		return itor->second ;
	return NULL;
}

void S6Connection::updateS6ConnectionId(ZQ::DataPostHouse::IDataCommunicatorPtr connId)
{
	if (connId)
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Connection, "[%s]update S6ConnectionId CONN["FMT64U"][0x%08x] "), _groupName.c_str(), connId->getCommunicatorId(), connId.get());
	_connId = connId;
}

int	S6Connection::connectionError(ZQ::DataPostHouse::IDataCommunicatorPtr connId)
{
	if (connId)
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Connection, "[%s]connectionError CONN["FMT64U"][0x%08x]"), _groupName.c_str(), connId->getCommunicatorId(), connId.get());

	size_t updateCount = 0;
	ZQ::common::MutexGuard g(_lockSessConnectionIds);
	for (SessionConnections::iterator itor = _sessConnectionIds.begin(); itor != _sessConnectionIds.end(); ++itor)
	{
		if (itor->second == connId)
		{
			itor->second = NULL;
			updateCount++;
		}
	}

	if (connId)
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(S6Connection, "[%s]update [%d] session connection with CONN["FMT64U"][0x%08x]"), _groupName.c_str(), updateCount, connId->getCommunicatorId(), connId.get());
	
	return (int)updateCount;
}

S6Connection::Ptr S6Connection::findS6Connection(const std::string& groupName)
{
	S6Connection::Ptr s6Connection = NULL;
	ZQ::common::MutexGuard g(_lockS6Sessions);

	std::map<std::string, S6Connection::Ptr>::iterator itorConn = _S6Connections.find(groupName);
	if (itorConn != _S6Connections.end())
	{
		s6Connection =  itorConn->second;
	}

	return s6Connection;
}

std::vector<std::string> S6Connection::getAllS6Connection()
{
	std::vector<std::string> s6Connctions;
	ZQ::common::MutexGuard g(_lockS6Sessions);
	for (std::map<std::string, S6Connection::Ptr>::iterator itorSG = _S6Connections.begin(); itorSG != _S6Connections.end(); itorSG++)
	{
		s6Connctions.push_back(itorSG->first);
	}

	return s6Connctions;
}
S6Connection::Ptr S6Connection::OpenS6Connection(std::string groupName, EdgeRMEnv& env, bool bCreate)
{
	S6Connection::Ptr s6Connection = findS6Connection(groupName);

	if (s6Connection)
		return s6Connection;

	if (!bCreate)
		return NULL;
	
	{

		ZQ::common::MutexGuard g(_lockS6Sessions);
		s6Connection =  new S6Connection(env, groupName);
		if (s6Connection)
			_S6Connections[groupName] = s6Connection;
	}

	return s6Connection;
}

void  S6Connection::clearAll()
{
	ZQ::common::MutexGuard g(_lockS6Sessions);
	_S6Connections.clear();
}

// -----------------------------
// class EdgeRMEnv
// -----------------------------
EdgeRMEnv::EdgeRMEnv(ZQ::common::Log& log, ZQ::common::Log& eventlog,
					 ZQ::common::NativeThreadPool& threadPool, 
					 ZQADAPTER_DECLTYPE& adapter,
					 std::string  databasePath, 
					 ZQ::common::FileLog& RtspEnginelog)
					 : _log(log), _eventlog(eventlog), _adapter(adapter), _thpool(threadPool),
					 _watchDog(log, threadPool, adapter, "EdgeRM"), _factory(NULL), _RtspEngineLog(RtspEnginelog), _dbPath(databasePath),
					 _QamSGWatchDog(0)
{
	_programRootPath = getProgramRoot();
	_programRootPath += FNSEPS;
	_deviceEvictorSize = _channelEvictorSize = _allocationEvictorSize = pConfig.evictorSize;
	_maxPreserveChannels = MAX_PRESERVE_CHANNELS;
	_maxEvaluateChannels = MAX_EVALUATE_CHANNELS;
	_allocationLeaseMs   = UNATTENDED_TIMEOUT;
	_s6AllocationLeaseMs = 600000;
	_retryInterval = 600000;
	_retrytimes = 3;

	_pQamTimerObjects = NULL;
	_pVrepServer = NULL;
	_pD6Factory = NULL;
	_ermiSessTimeout = pConfig.ermiClient.sessionTimeOut;
	_userAgent = ERM_USER_AGENT;

	_enableERMI = pConfig.ermiClient.enable;
	_ermiBindAddr.setAddress(pConfig.ermiClient.bindAddress.c_str());

	_enableR6 = pConfig.r6Client.enable;
	_pTripServer = NULL;

	if (pConfig.clientPoolSize < 20 )
		pConfig.clientPoolSize = 20;
	_clientThpool.resize(pConfig.clientPoolSize);

	_pErmInstanceSyncer = NULL;

	_bS6Started = false;
	// init _bindAddr;
}

EdgeRMEnv::~EdgeRMEnv()
{
	uninitialize();
}

bool EdgeRMEnv::initialize()
{
	try
	{
		//		initializePortal(*this);
		openDB(pConfig.databaseRuntimepath.c_str());

		// init the object factory
		_factory = new EdgeRMFactory(*this);

		RestoreChannelsOfDevice();

		_edgeRMPtr = new ::ZQTianShan::EdgeRM::EdgeRMImpl(*this);

		try
		{
			std::string strEndpoint="";
			if (pConfig.edgeRMEndpoint.empty())
			{
				strEndpoint = ::std::string("EdgeRM:") + DEFAULT_ENDPOINT(EdgeRM);
			}
			else
			{
				strEndpoint = ::std::string("EdgeRM:") + pConfig.edgeRMEndpoint;
			}

			::std::string strERMPrxEndpoint = strEndpoint;
			Ice::ObjectPrx objPrx = NULL;
			_edgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_adapter->getCommunicator()->stringToProxy(strERMPrxEndpoint.c_str()));	
			#if  ICE_INT_VERSION / 100 >= 306
				objPrx = _edgeRMPrx->ice_collocationOptimized(false);
			#else
				objPrx = _edgeRMPrx->ice_collocationOptimization(false);
			#endif
			_edgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(objPrx);
			glog(::ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "Edge Resouce Management Proxy create success"));
		}
		catch(...)
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "Edge Resouce Management Proxy create failed"));
			return false;
		}

		_s6Handle = new (std::nothrow) ::ZQTianShan::EdgeRM::S6Handler(*this, _edgeRMPrx);

		_rtspDak = new (std::nothrow) ZQRtspCommon::RtspDak(_log, pConfig.receiveThreads, pConfig.processTheads);
		_rtspDak->registerHandler(_s6Handle);

		if (!_rtspDak->start())
			return false;

		_rtspEngine = new ZQRtspEngine::RtspEngine(_log, _rtspDak);

		//start S6 server in normal mode
		if (!pConfig._backup.enableSync || pConfig._backup.erms.empty() || !stricmp(pConfig._backup.mode.c_str(),"standby"))
		{
			if (!startS6())
				return false;
			_bS6Started = true;
		}

		if (pConfig.enabledSSL)
		{
			// add ssl communictaor
			_rtspEngine->setCertAndKeyFile(pConfig.publicKeyFile, pConfig.privateKeyFile, pConfig.privatePassword);
			_rtspEngine->startSSLRtsp(pConfig.ipv4, pConfig.ipv6, pConfig.sslPort);
		}

		if (pConfig.allocationLease < 60000)
			pConfig.allocationLease = 60000;

		_s6AllocationLeaseMs = pConfig.allocationLease;

		_allocOwnerPtr = new AllocationOwnerImpl(pConfig.allocationLease, *this);

		_adapter->ZQADAPTER_ADD(_adapter->getCommunicator(), _allocOwnerPtr, INTERFACE_NAME_AllocationOwner);

		Ice::Identity identAllocOwner;
		identAllocOwner = _adapter->getCommunicator()->stringToIdentity(INTERFACE_NAME_AllocationOwner);
		_allocOwnerPrx  = ::TianShanIce::EdgeResource::AllocationOwnerPrx::checkedCast(_adapter->createProxy(identAllocOwner));

		_retrytimes = pConfig.retrytimes;
		_retryInterval = pConfig.retryInterval;

		if (_retrytimes < 0)
			_retrytimes = 3;

		if (_retryInterval < 60000)
			_retryInterval = 60000;

		// start the watch dog
		_watchDog.start();

		try
		{
			_QamSGWatchDog = new QamSGWatchDog(*this);
			if (_QamSGWatchDog)
				_QamSGWatchDog->start();
		}
		catch (...) { }

		if (pConfig.ermiClient.enable || pConfig.r6Client.enable)
		{
			/*
			try
			{
			_QamSGWatchDog = new QamSGWatchDog(*this);
			if (_QamSGWatchDog)
			_QamSGWatchDog->start();
			}
			catch (...){
			}
			*/

			ZQ::common::RTSPSession::startDefaultSessionManager();

			IdentCollection Idents;

			{
				::Freeze::EvictorIteratorPtr itptr = _eEdgeDevice->getIterator("", _deviceEvictorSize);
				while (itptr && itptr->hasNext())
					Idents.push_back(itptr->next());
			}

			for (size_t i = 0; i < Idents.size(); i++)
			{
				Ice::Identity& ident = Idents[i];

				try
				{	
					TianShanIce::EdgeResource::EdgeDeviceExPrx edgeDevicePrx = IdentityToObjEnv2(*this, EdgeDeviceEx, ident);
					if (!edgeDevicePrx)
						continue;
					std::string deviceName = edgeDevicePrx->getName();
					TianShanIce::StrValues expectedMetaData;
					expectedMetaData.push_back(SYS_PROP(Tftp));
					expectedMetaData.push_back(SYS_PROP(AdminUrl));
					TianShanIce::ObjectInfo edgeDeviceInfo = edgeDevicePrx->getInfo(expectedMetaData);

					if (edgeDeviceInfo.props.find(SYS_PROP(Tftp)) == edgeDeviceInfo.props.end())
						continue;

					std::string  tftp = edgeDeviceInfo.props[SYS_PROP(Tftp)];
					createQamSessionGroup(deviceName, tftp);
					insertQam(deviceName,tftp);
				}
				catch (const Ice::Exception&)	{}
				catch (...)	{}
			}
		}

		//start S6 server in Master-Slave mode
		if (pConfig._backup.enableSync)
		{
			ErmInstanceSyncer::PeerinfoMap peerInfoMap;
			ErmInstanceSyncer::SyncMode    thisInstMode = ErmInstanceSyncer::esm_Active;
			if (0 == stricmp(pConfig._backup.mode.c_str(), "standby"))
				thisInstMode = ErmInstanceSyncer::esm_Standby;

			for (size_t i = 0;  i< pConfig._backup.erms.size(); i++)
			{
				ErmInstanceSyncer::PeerInfo pi;
				pi.netId = pConfig._backup.erms[i].netId;
				pi.endpoint = pConfig._backup.erms[i].endpoint;
				pi.stampDeviceLastSync = pi.stampLastSync = pi.stampStatusAsOf =0;
				pi.b1stS6Msg = true;

				pi.status = (ErmInstanceSyncer::esm_Active == thisInstMode) ? ErmInstanceSyncer::esm_Standby : ErmInstanceSyncer::esm_Active;
				MAPSET(ErmInstanceSyncer::PeerinfoMap, peerInfoMap, pi.netId, pi);
			}

			_pErmInstanceSyncer = new ErmInstanceSyncer(*this, thisInstMode, pConfig._backup.syncInterval, peerInfoMap);
		}

		OnRestore();
		initD6();
		if (!initTripServer())
			return false;

		try
		{
			_adapter->activate();
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "Caught ice exception while active adapter: %s"), ex.ice_name().c_str());
			return false;
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "Caught unknown exception while active adapter"));
			return false;
		}

		if (_pErmInstanceSyncer)
			_pErmInstanceSyncer->start();

		glog.flush();
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "failed to initialize Edge Resource Manager service"));
		return false;
	}
	// _edgeEnvSnmp.regSnmp(this);

	//	testMarshal();
	return true;
}

void EdgeRMEnv::uninitialize()
{
	// _edgeEnvSnmp.unRegSnmp();
	if (pConfig.ermiClient.enable || pConfig.r6Client.enable)
	{
		ZQ::common::RTSPSession::stopDefaultSessionManager();
	}

	try
	{	
		if (_pErmInstanceSyncer)
		{
			_pErmInstanceSyncer->stop();
			delete _pErmInstanceSyncer;
		}

		if (_pVrepServer)
		{
			_pVrepServer->stop();
			delete _pVrepServer;
		}

		if (_pD6Factory)
		{
			delete _pD6Factory;
		}

		if (_pQamTimerObjects)
		{
			_pQamTimerObjects->removeAll();
			delete _pQamTimerObjects;
		}

		if (_pTripServer)
		{
			_pTripServer->stop();
			delete _pTripServer;
		}
	}
	catch (...) {} 

	_pErmInstanceSyncer = NULL;
	_pQamTimerObjects = NULL;
	_pD6Factory = NULL;
	_pVrepServer = NULL;
	_pTripServer = NULL;

	_watchDog.quit();
	closeDB();

	try
	{
		if (_rtspEngine)
		{
			_rtspEngine->stopAllCommunicators();	
			_rtspEngine->release();
		}
	}
	catch (...) {}

	try
	{
		if (_rtspDak)
		{
			_rtspDak->stop();
			_rtspDak->release();
		}
	}
	catch (...) {}

	S6Connection::clearAll();

	try
	{
		if (_QamSGWatchDog)
		{
			_QamSGWatchDog->quit();
			delete _QamSGWatchDog;
		}

		_QamSGWatchDog = NULL;

		ERMISessionGroup::clearSessionGroup();
		R6SessionGroup::clearSessionGroup();
	}
	catch (...)	{}

	_factory = NULL;
	_edgeRMPrx = NULL;

}

void EdgeRMEnv::openDB(const char* dbRuntimePath)
{
	closeDB();

	if (_dbPath.size() <  1)
		_dbPath = _programRootPath + "data" ;

	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;

	if ( NULL == dbRuntimePath || strlen(dbRuntimePath)<1 ) 
	{
		_dbRuntimePath = _dbPath;
	}
	else
	{
		_dbRuntimePath = dbRuntimePath;
	}

	if (FNSEPC != _dbRuntimePath[_dbRuntimePath.length()-1])
		_dbRuntimePath += FNSEPS;

	try 
	{	
		// open the Indexes
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "opening database at path: %s"), _dbPath.c_str());

#ifdef ZQ_OS_MSWIN
		::CreateDirectory(_dbPath.c_str(), NULL);
#else
		mkdir(_dbPath.c_str(), 0777);
#endif
		std::string channelDbPathname    = ZQTianShan::createDBFolder(_log, "EdgeRM", _dbPath.c_str(), "Channels");
		std::string allocationDbPathname = ZQTianShan::createDBFolder(_log, "EdgeRM", _dbRuntimePath.c_str(), "Allocations");

		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();

		proper->setProperty("Freeze.Evictor.UseNonmutating",      "1");
		std::string dbAttrPrefix = std::string("Freeze.DbEnv.") + channelDbPathname;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");

		dbAttrPrefix = std::string("Freeze.DbEnv.") + allocationDbPathname;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");

		// persistent DB about channels and devices
		std::string evictorAttrPrefix = std::string("Freeze.Evictor.");
		proper->setProperty(evictorAttrPrefix + (std::string)DBFILENAME_EdgeChannel+ ".$default.BtreeMinKey",      "16");
		proper->setProperty(evictorAttrPrefix + (std::string)DBFILENAME_EdgeChannel ".PageSize",      "8192");
		{
			//			_idxChannelOfDevice = new TianShanIce::EdgeResource::ChannelOfDevice(INDEXFILENAME(ChannelOfDevice));
			std::vector<Freeze::IndexPtr> indices;
			//			indices.push_back(_idxChannelOfDevice);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "opening database " DBFILENAME_EdgeChannel " with index ChannelOfDevice"));
#if ICE_INT_VERSION / 100 >= 303
			_eEdgeChannel = ::Freeze::createBackgroundSaveEvictor(_adapter, channelDbPathname, DBFILENAME_EdgeChannel, 0, indices);
#else
			_eEdgeChannel = Freeze::createEvictor(_adapter, channelDbPathname, DBFILENAME_EdgeChannel, 0, indices);
#endif
			_adapter->addServantLocator(_eEdgeChannel, DBFILENAME_EdgeChannel);
			_eEdgeChannel->setSize(_deviceEvictorSize);
		}

		proper->setProperty(evictorAttrPrefix + (std::string)DBFILENAME_EdgeDevice+ ".$default.BtreeMinKey",      "16");
		proper->setProperty(evictorAttrPrefix + (std::string)DBFILENAME_EdgeDevice ".PageSize",      "8192");

		{
			std::vector<Freeze::IndexPtr> indices;
			_idxDeviceOfZONE = new TianShanIce::EdgeResource::DeviceOfZONE(INDEXFILENAME(DeviceOfZONE));
			indices.push_back(_idxDeviceOfZONE);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "opening database " DBFILENAME_EdgeDevice " with index DeviceOfZONE"));
#if ICE_INT_VERSION / 100 >= 303
			_eEdgeDevice = ::Freeze::createBackgroundSaveEvictor(_adapter, channelDbPathname, DBFILENAME_EdgeDevice, 0, indices);
#else
			_eEdgeDevice = Freeze::createEvictor(_adapter, channelDbPathname, DBFILENAME_EdgeDevice, 0, indices);
#endif
			_adapter->addServantLocator(_eEdgeDevice, DBFILENAME_EdgeDevice);
			_eEdgeDevice->setSize(_channelEvictorSize);
		}

		// runtime DB about allocations
		proper->setProperty(evictorAttrPrefix + (std::string)DBFILENAME_Allocation+ ".$default.BtreeMinKey",      "16");
		proper->setProperty(evictorAttrPrefix + (std::string)DBFILENAME_Allocation ".PageSize",      "8192");

		{
			std::vector<Freeze::IndexPtr> indices;
			_idxOwnerOfAllocationEx = new TianShanIce::EdgeResource::OwnerOfAllocationEx(INDEXFILENAME(OwnerOfAllocationEx));
			_idxQAMSessOfAllocationEx = new TianShanIce::EdgeResource::QAMSessOfAllocationEx(INDEXFILENAME(ERMISessOfAllocationEx));
			indices.push_back(_idxOwnerOfAllocationEx);
			//     indices.push_back(_idxERMISessOfAllocationEx);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "opening database " DBFILENAME_Allocation " "));
#if ICE_INT_VERSION / 100 >= 303
			_eAllocation = ::Freeze::createBackgroundSaveEvictor(_adapter, allocationDbPathname, DBFILENAME_Allocation, 0, indices);
#else
			_eAllocation = Freeze::createEvictor(_adapter, allocationDbPathname, DBFILENAME_Allocation, 0, indices);
#endif
			_adapter->addServantLocator(_eAllocation, DBFILENAME_Allocation);
			_eAllocation->setSize(_allocationEvictorSize);
		}
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(EdgeRMEnv, 1001, "openDB() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(EdgeRMEnv, 1001, "openDB() caught unknown exception"));
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "database ready"));
}

void EdgeRMEnv::closeDB(void)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "closing local database"));

	_eEdgeDevice  = NULL;
	_eEdgeChannel = NULL;

	_eAllocation  = NULL;

	//	_idxChannelOfDevice = NULL;
	_idxDeviceOfZONE = NULL;
}

TianShanIce::EdgeResource::EdgeChannelExPrx EdgeRMEnv::_openChannel(const ::std::string& chName)
{
	Ice::Identity identChannel;
	identChannel.category = DBFILENAME_EdgeChannel;
	identChannel.name = chName;

	TianShanIce::EdgeResource::EdgeChannelExPrx chPrx = NULL;

	try
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "_openChannel() EdgeChannel by fullname[%s]"), identChannel.name.c_str());
		// must not use uncheckedCast here
		chPrx = IdentityToObjEnv(*this, EdgeChannelEx, identChannel);
	}
	catch (const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "_openChannel() failed to EdgeChannel by fullname[%s]: caught exception[%s]"), identChannel.name.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "_openChannel() failed to EdgeChannel by fullname[%s]: caught unknown exception"), identChannel.name.c_str());
	}

	return chPrx;
}

bool EdgeRMEnv::RestoreChannelsOfDevice()
{
	IdentCollection Idents;
	try	{
		{
			::Freeze::EvictorIteratorPtr itptr = _eEdgeChannel->getIterator("", _channelEvictorSize);
			while (itptr && itptr->hasNext())
				Idents.push_back(itptr->next());
		}

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() %d channels found"), Idents.size());

		for (IdentCollection::iterator itChannel = Idents.begin(); itChannel < Idents.end(); itChannel++)
		{
			try
			{
				std::string deviceName;
				Ice::Short EdgePort,chNum;
				TianShanIce::EdgeResource::EdgeChannelExPrx edgechprx = IdentityToObjEnv2(*this, EdgeChannelEx, *itChannel);
				edgechprx->getHiberarchy(deviceName, EdgePort, chNum);

				ZQ::common::MutexGuard gd(_lkdevicechannels);
				DeviceChannelsMap::iterator itorDC = _devicechannels.find(deviceName);
				if (itorDC == _devicechannels.end())
				{
					IdentCollection channellists;
					channellists.push_back(*itChannel);
					MAPSET(DeviceChannelsMap, _devicechannels, deviceName, channellists);
				}
				else
				{
					itorDC->second.push_back(*itChannel);
				}
			}
			catch(const ::Ice::Exception& ex)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught exception[%s]"), ex.ice_name().c_str());
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught unknown exception"));
			}
		}

		{
#ifndef _TestLog
			DeviceChannelsMap::iterator itorDC = _devicechannels.begin();
			while(itorDC != _devicechannels.end())
			{
				for (IdentCollection::iterator itorch = itorDC->second.begin(); itorch != itorDC->second.end(); itorch++)
				{
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() Device[%s], Channel[%s]"), itorDC->first.c_str(), (*itorch).name.c_str());
				}
				itorDC++;
			}
#endif
		}      
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() successful"));
	}
	catch(const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught unknown exception"));
	}

	return true;
}

/*bool EdgeRMEnv::RestoreChannelsOfDevice()
{
IdentCollection Idents;
try	{
{
ZQ::common::MutexGuard gd(_lockEdgeDevice);
::Freeze::EvictorIteratorPtr itptr = _eEdgeDevice->getIterator("", _deviceEvictorSize);
while (itptr && itptr->hasNext())
Idents.push_back(itptr->next());
}

_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() %d devices found"), Idents.size());

for (IdentCollection::iterator itDevice = Idents.begin(); itDevice < Idents.end(); itDevice++)
{
//				IdentCollection ids = _idxChannelOfDevice->find(*itDevice);`

_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "get channels identities of devices[%s]"), (*itDevice).name.c_str());

IdentCollection channellists;
for (IdentCollection::iterator it = ids.begin(); it < ids.end(); it++)
{
_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "channels[%s] of devices[%s]"), (*it).name.c_str(), (*itDevice).name.c_str());
channellists.push_back(*it);
}
MAPSET(DeviceChannelsMap, _devicechannels, (*itDevice).name, channellists);
}

_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() successful"));
}
catch(const ::Ice::Exception& ex)
{
_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught exception[%s]"), ex.ice_name().c_str());
}
catch(...)
{
_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught unknown exception"));
}
return true;
}*/
bool EdgeRMEnv::addChannelsToDevice(const std::string& deviceName, IdentCollection& channellists)
{
	try	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "addChannelsToDevice() devices[%s] channelsize[%d]"), deviceName.c_str(), channellists.size());
		ZQ::common::MutexGuard gd(_lkdevicechannels);
		MAPSET(DeviceChannelsMap, _devicechannels, deviceName, channellists);
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "addChannelsToDevice() successful"));
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "addChannelsToDevice() caught unknown exception"));
	}

	return true;
}
bool EdgeRMEnv::addChannelToDevice(const std::string& deviceName, const Ice::Identity& identCh)
{
	try	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "addChannelToDevice() devices[%s] channel name[%s]"), deviceName.c_str(), identCh.name.c_str());
		ZQ::common::MutexGuard gd(_lkdevicechannels);
		DeviceChannelsMap::iterator it = _devicechannels.find(deviceName);
		if( it != _devicechannels.end() && std::find(it->second.begin(), it->second.end(),identCh)  ==  it->second.end())
		{
				it->second.push_back(identCh);
		}
		else
		{
			IdentCollection channellists;
			channellists.push_back(identCh);
			MAPSET(DeviceChannelsMap, _devicechannels, deviceName, channellists);
		}
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "addChannelToDevice() devices[%s] channel name[%s] successful"), deviceName.c_str(), identCh.name.c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "addChannelToDevice() caught unknown exception"));
	}

	return true;
}
bool EdgeRMEnv::removeChannelsFromDevice(const std::string& deviceName)
{
	try
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "removeChannelsFormDevice() devices[%s]"), deviceName.c_str());

		::ZQ::common::MutexGuard gd(_lkdevicechannels);
		DeviceChannelsMap::iterator itor =_devicechannels.find(deviceName);
		if (itor != _devicechannels.end())
		{
			_devicechannels.erase(itor);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "removeChannelsFormDevice() remove channellist from Device[%s]"), deviceName.c_str());
		}
	}
	catch (...) { }
	return true;
}
bool EdgeRMEnv::removeChannelFromDevice(const std::string& deviceName, const Ice::Identity& identCh)
{
	try
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "removeChannelsFormDevice() devices[%s]"), deviceName.c_str());

		ZQ::common::MutexGuard gd(_lkdevicechannels);
		DeviceChannelsMap::iterator it =_devicechannels.find(deviceName);
		if (it != _devicechannels.end())
		{
			IdentCollection::iterator itorCh =  std::find(it->second.begin(), it->second.end(),identCh);
			if(itorCh !=  it->second.end()) 
				it->second.erase(itorCh);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "removeChannelFormDevice() remove channel[%s] from Device[%s]"), identCh.name.c_str(),deviceName.c_str());
		}
	}
	catch (...) { }
	return true;
}
/*bool EdgeRMEnv::AddChOfDevice(std::string deviceName)
{
Ice::Identity identDevice;
identDevice.name = deviceName;
identDevice.category  = DBFILENAME_EdgeDevice;
try	{

IdentCollection ids = _idxChannelOfDevice->find(identDevice);

_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "AddChOfDevice()get channels identities of devices[%s]"), deviceName.c_str());

IdentCollection channellists;
for (IdentCollection::iterator it = ids.begin(); it < ids.end(); it++)
{
_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "AddChOfDevice() channels[%s] of devices[%s]"), (*it).name.c_str(), deviceName.c_str());
channellists.push_back(*it);
}
::ZQ::common::MutexGuard gd(_lkdevicechannels);
MAPSET(DeviceChannelsMap, _devicechannels, deviceName, channellists);

_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "AddChOfDevice() successful"));
}
catch(const ::Ice::Exception& ex)
{
_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught exception[%s]"), ex.ice_name().c_str());
}
catch(...)
{
_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "RestoreChannelsOfDevice() caught unknown exception"));
}
return true;
}
bool EdgeRMEnv::RemoveChofDevice(std::string deviceName)
{
try
{
::ZQ::common::MutexGuard gd(_lkdevicechannels);

DeviceChannelsMap::iterator itor =_devicechannels.find(deviceName);
if (itor != _devicechannels.end())
{
_devicechannels.erase(itor);
_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "remove channellist from Device[%s]"), deviceName.c_str());
}
}
catch (...)
{

}
return true;
}*/

void EdgeRMEnv::OnRestore()
{
	Freeze::EvictorIteratorPtr itObjId;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "initialization: activating the channel records from DB"));
	for (itObjId = _eAllocation->getIterator("", _allocationEvictorSize); itObjId && itObjId->hasNext(); )
	{
		Ice::Identity objId = itObjId->next();
		try 
		{
			TianShanIce::EdgeResource::AllocationExPrx allocPrx = ::TianShanIce::EdgeResource::AllocationExPrx::checkedCast(_adapter->createProxy(objId));
			/*	 
			///restore ermi session
			if (pConfig.ermiClient.enable)
			{
			std::string eqamSessionId = allocPrx->getEqamSessionId();
			std::string eqamSessGroup = allocPrx->getEqamSessGroup();
			if (!eqamSessGroup.empty() && eqamSessionId.empty())
			{
			syncERMISession(objId.name, eqamSessGroup, eqamSessionId);  
			}
			} 
			*/
			///restore s6 session
			std::string strSessionGroup = allocPrx->getSessionGroup();
			if (strSessionGroup.empty())
				continue;

			std::string sessionId = allocPrx->getOwnerKey();

			//restore session group info
			S6Connection::Ptr s6Connection = S6Connection::OpenS6Connection(strSessionGroup, *this);
			if (s6Connection)
			{
				s6Connection->add(sessionId, NULL);
			}
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "OnRestore() caught exception[%s] %s"), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "OnRestore() caught exception[%s]"),ex.ice_name().c_str());
		}
		catch(...)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "OnRestore() caught unknown exception"));
		}
	}
}

bool EdgeRMEnv::initTripServer()
{
	if (!pConfig._tripServer.enable)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "initTripServer() disable trip server"));
		return true;
	}

	try
	{	
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "initTripServer() init trip server"));

		_pTripServer = new TripSocketServer(_log, pConfig._tripServer.receiveThreads, pConfig._tripServer.processThreads);

		if (!_pTripServer)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "failed to create trip socket server"));
			return false;
		}

		if (!_pTripServer->start())
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "failed to start trip socket server"));
			return false;
		}

		if (!_pTripServer->addListener(pConfig._tripServer.ip, pConfig._tripServer.port))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "init trip server failed to add listener at[%s:%s]"),
				pConfig._tripServer.ip.c_str(),  pConfig._tripServer.port.c_str());
			return false;
		}
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "initTripServer() init trip server caught unknown exception(%d)"),SYS::getLastErr());

		return false;
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "initTripServer() init trip server successfully"));
	return true;
}
bool EdgeRMEnv::initD6()
{
	if (!pConfig._stVrepServer.enable)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "initD6() disable vrep server"));
		return true;
	}

	try
	{	
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "initD6() init vrep server"));

		_pQamTimerObjects = new QAMTimerObjects(*this, _thpool, pConfig._stVrepServer.timeInterval);
		_pVrepServer = new ZQ::Vrep::Server(glog, _thpool);
		if (!_pVrepServer)
			return false;
		_pVrepServer->setBindAddress(pConfig._stVrepServer.ipAddress.c_str(), pConfig._stVrepServer.port);

		_pD6Factory = new D6Factory(*this, _edgeRMPrx);
		if (!_pD6Factory)
			return false;
		_pVrepServer->setMonitorFactory(*_pD6Factory);
		ZQ::Vrep::Configuration conf;
		conf.identifier = 1;
		conf.streamingZone = "ZQ";
		conf.componentName = "EdgeRM";
		conf.vendorString = "ZQ";
		conf.defaultHoldTimeSec = 60;
		conf.connectRetryTimeSec = 60;
		conf.connectTimeoutMsec = 2000;
		conf.keepAliveTimeSec = 10;
		conf.sendReceiveMode = VREP_ReceiveOnlyMode;
		_pVrepServer->configure(conf);
		_pVrepServer->start();
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "initD6() init vrep server caught unknown exception(%d)"),
			SYS::getLastErr());

		return false;
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "initD6() init vrep server successfully"));
	return true;
}

ZQ::common::Log::loglevel_t EdgeRMEnv::getRtspLogLevel()
{
	return (ZQ::common::Log::loglevel_t)pConfig.lLogLevel;
}

bool EdgeRMEnv::createQamSessionGroup(const std::string& qamName, const std::string& TFTP)
{
	//	if (!_enableERMI)
	//		return true;

	ZQ::common::URLStr url(TFTP.c_str());
	std::string portocol = url.getProtocol();
	std::string ip = url.getHost();
	int port = url.getPort();
	if (port <=0 )
		port = 554;
	char baseURL[512] = "";
	if (_enableERMI && stricmp(portocol.c_str(), "ERMI") == 0)
	{
		memset(baseURL,0,sizeof(baseURL));
		snprintf(baseURL, 512,"rtsp://%s:%d", ip.c_str(), port);

		ERMISessionGroup::Ptr group = ERMISessionGroup::findSessionGroup(qamName);
		if (group)
			return true;

		MutexGuard g(ERMISessionGroup::_lockGroups);		
		group = new ERMISessionGroup(*this, qamName, baseURL, 300, 600000);
		ERMISessionGroup::_groupMap.insert(ERMISessionGroup::SessionGroupMap::value_type(qamName, group));	
		if (_QamSGWatchDog)
			_QamSGWatchDog->watch(group, 600000);
		_log(::ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "ERMISessionGroup[%s] created to SS[%s]"), qamName.c_str(), baseURL);	
	}
	else if (_enableR6 && stricmp(portocol.c_str(), "NGODR6") == 0)
	{
		url.setProtocol("rtsp");
		char* baseurl = const_cast<char*>(url.generate());
		//format maybe not match 
		R6SessionGroup::Ptr group = R6SessionGroup::findSessionGroup(qamName);
		if (group)
			return true;
		group = new R6SessionGroup(*this, qamName, baseurl, 300, 600000);
		if (!group)
		{
			_log(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "R6SessionGroup create failed"));	
			return false;
		}

		{
			MutexGuard gd(R6SessionGroup::_lockGroups);		
			R6SessionGroup::_groupMap.insert(R6SessionGroup::SessionGroupMap::value_type(qamName, group));
		}

		if (_QamSGWatchDog)
			_QamSGWatchDog->watch(group, 600000);

		_log(::ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "R6SessionGroup[%s] created to SS[%s]"), qamName.c_str(), baseurl);	
	}

	return true;
}
bool EdgeRMEnv::removeQamSessionGroup(const std::string& qamName)
{
	_log(::ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "SessionGroup[%s] removed"), qamName.c_str());

	ERMISessionGroup::Ptr ermiGroup = ERMISessionGroup::findSessionGroup(qamName);
	if (ermiGroup)
		ermiGroup = NULL;

	R6SessionGroup::Ptr r6Group = R6SessionGroup::findSessionGroup(qamName);
	if (r6Group)
		r6Group = NULL;

	return true;
}

bool EdgeRMEnv::updataAllocation(const std::string& allocId, const std::string& eqamSessGroup, const std::string& sessionId)
{
	_log(::ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "update allocation[%s]=>qamSessGroup[%s] and qamSessionId[%s]"), allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str());	

	bool bRet = false;
	try
	{
		Ice::Identity ident;
		ident.name = allocId;
		ident.category = DBFILENAME_Allocation;

		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(*this, AllocationEx, ident);
		allocExPrx->setqamSessGroup(eqamSessGroup);
		allocExPrx->setqamSessionId(sessionId);
		bRet = true;
	}
	catch (const Ice::Exception& ex)
	{
		_log(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "allocation[%s] failed to get allocation object caught exception[%s]"), allocId.c_str(), ex.ice_name().c_str());	
	}
	catch (...)
	{
		_log(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "allocation[%s]failed to get allocation object caught exception"), allocId.c_str());	
	}

	_log(::ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "update allocation[%s]=>qamSessGroup[%s] and qamSessionId[%s] successful"), allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str());	

	return bRet;
}

bool EdgeRMEnv::syncERMISession(const std::string& allocId, const std::string& eqamSessGroup, const std::string& sessionId)
{
	bool bRet = false;
	try
	{
		Ice::Identity ident;
		ident.name = allocId;
		ident.category = DBFILENAME_Allocation;

		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(*this, AllocationEx, ident);
		if (allocExPrx)
		{
			allocExPrx->setqamSessionId(sessionId);
			allocExPrx->setqamSessGroup(eqamSessGroup);

			ERMISession::Ptr ermiSession = ERMISessionGroup::openSession(allocId, eqamSessGroup, true);
			if (ermiSession)
			{
				ermiSession->setSessionId(sessionId);
				bRet = true;
				glog(ZQ::common::Log::L_INFO, CLOGFMT(PhoEdgeRMEnv, "[%s][%s][%s]sync ERMISession successful"), allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str());
			}	
		}
	}
	catch (const Ice::Exception& ex)
	{
		_log(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "[%s][%s][%s]failed to get allocation object caught exception[%s]"),
			allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str(), ex.ice_name().c_str());	
	}
	catch (...)
	{
		_log(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "[%s][%s][%s]failed to get allocation object caught exception"),
			allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str());	
	}
	return bRet;
}

bool EdgeRMEnv::syncR6Session(const std::string& allocId, const std::string& eqamSessGroup, const std::string& sessionId)
{
	bool bRet = false;
	try
	{
		Ice::Identity ident;
		ident.name = allocId;
		ident.category = DBFILENAME_Allocation;

		TianShanIce::EdgeResource::AllocationExPrx allocExPrx = IdentityToObjEnv2(*this, AllocationEx, ident);
		if (allocExPrx)
		{
			allocExPrx->setqamSessionId(sessionId);
			allocExPrx->setqamSessGroup(eqamSessGroup);

			R6Session::Ptr r6Session = R6SessionGroup::openSession(allocId, eqamSessGroup, true);
			if (r6Session)
			{
				r6Session->setSessionId(sessionId);
				bRet = true;
				glog(ZQ::common::Log::L_INFO, CLOGFMT(PhoEdgeRMEnv, "[%s][%s][%s]sync R6Session successful"), allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str());
			}	
		}
	}
	catch (const Ice::Exception& ex)
	{
		_log(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "[%s][%s][%s]failed to get allocation object caught exception[%s]"),
			allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str(), ex.ice_name().c_str());	
	}
	catch (...)
	{
		_log(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMEnv, "[%s][%s][%s]failed to get allocation object caught exception"),
			allocId.c_str(), eqamSessGroup.c_str(), sessionId.c_str());	
	}

	return bRet;
}

void EdgeRMEnv::ermiSessSetup(std::string& allocId)
{
	if (!_enableERMI)
		return;

	try 
	{
		ERMISessSetupCmd* pSessSetup = new ERMISessSetupCmd(*this, allocId);
		if (pSessSetup)
			pSessSetup->execute();
	}
	catch(...)	{}
}

void EdgeRMEnv::ermiSessTearDown(const std::string& allocId, const std::string& qamName, const std::string& sessionID)
{
	if (!_enableERMI)
		return;

	try 
	{
		ERMISessTearDownCmd* pSessTearDown = new ERMISessTearDownCmd(*this, allocId, qamName, sessionID);
		if (pSessTearDown)
			pSessTearDown->execute();
	}
	catch(...)	{}
}

void EdgeRMEnv::r6SessSetup(std::string& allocId)
{
	if (!_enableR6)
		return;

	try
	{
		R6ProvPortCmd* pSessSetup = new R6ProvPortCmd(*this,allocId);
		if (pSessSetup)
			pSessSetup->execute();
	}
	catch(...)	{}
}

void EdgeRMEnv::r6SessTearDown(const std::string& allocId,const std::string& sessionID)
{
	if (!_enableR6)
		return;

	try
	{
		R6StopCheckCmd*    pSessStopChek = new R6StopCheckCmd(*this,allocId,sessionID);
		if (pSessStopChek)
			pSessStopChek->execute();
	}
	catch(...)	{}
}

bool EdgeRMEnv::insertQam(::std::string qamName,::std::string TFTP)
{
	::ZQ::common::MutexGuard gd(_lkIdxQamType);
	if (_idxQamType.find(qamName) != _idxQamType.end())
		return false;
	_idxQamType.insert(make_pair(qamName,TFTP));

	return true;
}	

bool EdgeRMEnv::removeQam(::std::string qamName)
{
	::ZQ::common::MutexGuard gd(_lkIdxQamType);
	QamTypeIndex::iterator iter;
	if ((iter = _idxQamType.find(qamName)) != _idxQamType.end())
	{
		_idxQamType.erase(iter);
		return true;
	}

	return false;
}

bool EdgeRMEnv::qamSessSetup(std::string& allocId,std::string qamName)
{
	if (_enableERMI == false && _enableR6 == true)
	{
		r6SessSetup(allocId);
		return true;
	}

	if (_enableERMI == true && _enableR6 == false)
	{
		ermiSessSetup(allocId);
		return true;
	}

	::ZQ::common::MutexGuard gd(_lkIdxQamType);
	QamTypeIndex::iterator iter = _idxQamType.find(qamName);
	if (iter == _idxQamType.end())
		return false;

	//maybe modify
	if (!iter->second.compare("ERMI"))
	{
		ermiSessSetup(allocId);
		return true;
	}

	if (!iter->second.compare("NGODR6"))
	{
		r6SessSetup(allocId);
		return true;
	}

	return false;
}

bool EdgeRMEnv::startS6()
{
	if (!_bS6Started)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMEnv, "opening S6 server-side at %s/%s:%s"), pConfig.ipv6.c_str(), pConfig.ipv4.c_str(), pConfig.tcpPort.c_str());
		if (!_rtspEngine->startTCPRtsp(pConfig.ipv4, pConfig.ipv6, pConfig.tcpPort))
		{
			_rtspDak->stop();
			return false;
		}

		_bS6Started = true;
	}

	return _bS6Started;
}

// -----------------------------
// class ErmInstanceSyncer
// -----------------------------
int	ErmInstanceSyncer::run()
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "starts with mode[%s]"), (esm_Standby==_instanceMode)?"standby":"active");

	uint cS6YieldRound = INST_SYNC_S6_SERVER_YIELD_ROUNDS;

	if (_syncInterval > _syncInterval)
		_syncInterval = INST_SYNC_INTERVAL_MIN;
	int64 syncDeivceInter = pConfig._backup.syncDeviceInter * 1000;

	if(_addWatchDog)
	{
		_addWatchDog->init();
		_addWatchDog->start();
	}

	if(_removeWatchDog)
	{
		_removeWatchDog->init();
		_removeWatchDog->start();
	}
	while (!_bQuit)
	{
		int nextSleep = _syncInterval;

		// step.1 performs a round of instance sync
		for (PeerinfoMap::iterator itor = _peerMap.begin(); !_bQuit && itor != _peerMap.end(); itor++)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync loop remote ERM endpoint[%s] mode[%s] in mode[%s]"), itor->second.endpoint.c_str(), (itor->second.status==esm_Standby?"Standby":"Active"), (_instanceMode==esm_Standby?"Standby":"Active"));
			// step 1.1 validate if the sync of this peer is necessary
			if (esm_Active == _instanceMode)
			{
				// the peer is just a backup instance, we only take the first iteration
				if (itor !=_peerMap.begin())
					break;

				// if the previous sync has ever succeeded, reduce the interval to speed up the second sync
				//if (itor->second.stampLastSync >0)
				nextSleep = 0; // immediately start
			}
			else
			{
				// "esm_Standby == _instanceMode" only backup for those active peer ERM instances
				if (esm_Active != itor->second.status)
					continue;
			}

			// step 1.2 perform the sync device and routes of the peer if erm in standby mode
			if(_instanceMode == esm_Standby)
			{
				if((ZQ::common::TimeUtil::now() - itor->second.stampDeviceLastSync) >= syncDeivceInter*0.8)
				{
					int64 syncDevicesStart = ZQ::common::TimeUtil::now();
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "syncing device peer ERM instance[%s]: endpoint[%s]"), itor->second.netId.c_str(), itor->second.endpoint.c_str());
					if(syncDevicesAndRoutes(itor->second.netId, "", itor->second.endpoint, 0))
					{
						itor->second.stampDeviceLastSync = ZQ::common::now();
						envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync-ed devices peer ERM instance[%s]: endpoint[%s]"), itor->second.netId.c_str(), itor->second.endpoint.c_str());
					}
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed syncing all devices peer ERM instance[%s]:endpoint[%s] took %dms"), itor->second.netId.c_str(), itor->second.endpoint.c_str(), (int)(ZQ::common::TimeUtil::now()-syncDevicesStart));
				}
			}

			// step 1.3 perform the sync allocations of the peer
			if(_instanceMode == esm_Standby || (_instanceMode == esm_Active && cS6YieldRound > 0))
			{
				int64 stampSyncStart = ZQ::common::TimeUtil::now();
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "syncing allocastions peer ERM instance[%s]: endpoint[%s]"), itor->second.netId.c_str(), itor->second.endpoint.c_str());
				if (syncAlloction(itor->second.netId, itor->second.endpoint, 0))
				{
					itor->second.stampLastSync = ZQ::common::now();
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync-ed allocations peer ERM instance[%s]: endpoint[%s]"), itor->second.netId.c_str(), itor->second.endpoint.c_str());
				}
				int timeUsed = (int)(ZQ::common::TimeUtil::now()-stampSyncStart);
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "data collection processed syncing all allocations peer ERM instance[%s]:endpoint[%s] took %dms"), itor->second.netId.c_str(), itor->second.endpoint.c_str(), timeUsed);
				nextSleep = nextSleep - timeUsed;
				if(nextSleep < 0)
					nextSleep = 1;
			}
		}

		// step.2 test if it is time to enable the S6 server-side
		if (cS6YieldRound >0)
		{
			if (0 == --cS6YieldRound)
			{
				// 同步三次后, 不管成功与否, 启动S6 Listener 端口
				_env.startS6();

				// step 2.1 sync finished here if this run as active mode
				if (esm_Active == _instanceMode)
				{
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync-for-init finished with mode[active]"));
					return 0;
				}
			}
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync device after sleep[%d]"), nextSleep);

		// test
		nextSleep = 0;
		if (!_bQuit && nextSleep >0)
			_hWait.wait(nextSleep);
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "stops"));
	return 0;
}

bool ErmInstanceSyncer::changeWorkForMode(const std::string& peerNetId, int mode)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "changeWorkForMode() thisMode[%s] peer[%s] mode[%s(%d)]"), (esm_Standby==_instanceMode)?"standby":"active", peerNetId.c_str(), (0==mode)?"inactive":"active", mode);
	ZQ::common::MutexGuard g(_lkPeerMap);
	PeerinfoMap::iterator it = _peerMap.find(peerNetId);
	std::string endpoint;

	if (esm_Active == _instanceMode)
	{
		if (_peerMap.end() == it && _peerMap.size() >0)
			it = _peerMap.begin();

		endpoint = it->second.endpoint;
		std::string localNetId = getNetId("");

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "changeWorkForMode() active instance forcing peer[%s] endpoint[%s] to mode[%s(%d)]"), it->second.netId.c_str(), endpoint.c_str(), (0==mode)?"inactive":"active", mode);

		///当收到第一个 S6 Setup请求后, 强制将GERM中 netId的 Edge ERM status变为ERMSync_StandbyInst::InActive. 
		///mode =0 代表ERMSync_StandbyInst::InActive, 1代表ERMSync_StandbyInst::Active;
		TianShanIce::EdgeResource::EdgeRMPrx edgeRMPrx = NULL;
		try 
		{
			edgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(endpoint));
			edgeRMPrx->forceBackupMode(localNetId, 1);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "changeWorkForMode() called peer[%s] endpoint[%s] to mode[%s(%d)]"), it->second.netId.c_str(), endpoint.c_str(), (0==mode)?"inactive":"active", mode);
		}
		catch (::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "calling forceBackupMode() to peer[%s] caught exception[%s]"), endpoint.c_str(),  ex.ice_name().c_str());
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "calling forceBackupMode() to peer[%s] caught exception, err[%d]"), endpoint.c_str(), SYS::getLastErr());
		}
	}
	else
	{
#pragma message ( __MSGLOC__ "TODO: change Edge ERM status")

		if (_peerMap.end() == it)
			return false;

		it->second.status = (0 == mode) ? esm_Standby : esm_Active;
		it->second.stampStatusAsOf = ZQ::common::now();
	}

	return true;  
}

std::string ErmInstanceSyncer::getNetId(const std::string& routeName)
{
	if (esm_Standby == _instanceMode)
	{
#pragma message ( __MSGLOC__ "TODO: get Edge NetId from routeName of EdgeERM netId map")
	return _routeNameToNetId[routeName];
	}

	return pConfig.netId;
}

bool ErmInstanceSyncer::syncDevicesAndRoutes(const std::string&netId, const std::string& deviceName, const std::string endpoint, int64 since)
{
	// param deviceName just for export device that specified
	int64 stampBegin = ZQ::common::TimeUtil::now();
	int deviceCount=0, routeCount=0, portCount=0;
	char timeBuf[65]="";
	ZQ::common::TimeUtil::TimeToUTC(since, timeBuf, sizeof(timeBuf) -1);

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync Devices from ERM[%s] endpoint[%s] since[%s] by mode[%s]"), netId.c_str(), endpoint.c_str(), timeBuf, (esm_Active == _instanceMode)? "active" :"standby" );

	///get remote ERM service proxy
	bool bret = false;
	TianShanIce::EdgeResource::EdgeRMPrx remoteEdgeRMPrx = NULL;
	TianShanIce::EdgeResource::EdgeRMPrx tempPrx = _env.getEdgeRMPrx();
	TianShanIce::EdgeResource::EdgeRMPrx localEdgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(tempPrx->ice_compress(true));
	try 
	{
		//step 1: get remote devices and import in local
		int64 stampStart = ZQ::common::TimeUtil::now();
		Ice::ObjectPrx objPrx = NULL;
		remoteEdgeRMPrx  = TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy((endpoint+" -z").c_str()));
		#if  ICE_INT_VERSION / 100 >= 306
			objPrx = remoteEdgeRMPrx->ice_collocationOptimized(false);
		#else
			objPrx = remoteEdgeRMPrx->ice_collocationOptimization(false);
		#endif
		remoteEdgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(objPrx->ice_compress(true));
		//remoteEdgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(endpoint));
		if (NULL == remoteEdgeRMPrx)
			return bret;
		std::string xmlBody = remoteEdgeRMPrx->exportDeviceXML(deviceName);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed sync devices step 1:export all devices took %dms"), (int)(ZQ::common::TimeUtil::now()-stampStart));
		stampStart = ZQ::common::TimeUtil::now();

//		TianShanIce::EdgeResource::ObjectInfos routeNameInfos = remoteEdgeRMPrx->listRouteNames();
// 		TianShanIce::EdgeResource::ObjectInfos::iterator routeIter = routeNameInfos.begin();
// 		for(routeIter; routeIter != routeNameInfos.end();routeIter++)
// 			_routeNameToNetId[routeIter->ident.name] = netId;
		
		TianShanIce::Properties props;
		std::string deviceNull = "deviceNull";
		TianShanIce::EdgeResource::EdgeDevices localEdgeDevices = localEdgeRMPrx->importDeviceXML(deviceNull, "zoneNull", props, xmlBody);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed sync devices step 2:import all %d devices took %dms"), localEdgeDevices.size(), (int)(ZQ::common::TimeUtil::now()-stampStart));
		stampStart = ZQ::common::TimeUtil::now();
		if(localEdgeDevices.size() == 0)
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ErmInstanceSyncer, "import 0 device"));
			return true;
		}
		deviceCount = localEdgeDevices.size();

		envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync devices, sync[%d] devices from remote ERM[%s] endpoint[%s]"), localEdgeDevices.size(), netId.c_str(), endpoint.c_str());

		//step 2: get remote routeNames and link to local edgePort
		TianShanIce::EdgeResource::ObjectInfos routeNameInfos = remoteEdgeRMPrx->listRouteNames();
		for(TianShanIce::EdgeResource::ObjectInfos::iterator routeIter = routeNameInfos.begin(); routeIter != routeNameInfos.end();routeIter++)
		{
			routeCount++;
			TianShanIce::EdgeResource::EdgePortInfos edgePortInfos = remoteEdgeRMPrx->findRFPortsByRouteName(routeIter->ident.name);
			TianShanIce::EdgeResource::EdgePortInfos::iterator portIter = edgePortInfos.begin();
			if(portIter == edgePortInfos.end())
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer,"sync routeName failed, can not get any port from routeName[%s] in remote ERM[%s] endpoint[%s]"), routeIter->ident.name.c_str(), netId.c_str(), endpoint.c_str());
				continue;;
			}
			for(portIter; portIter != edgePortInfos.end(); portIter++)
			{
				std::string deviceNameOfPort = portIter->resPhysicalChannel.resourceData["edgeDeviceName"].strs[0];
				TianShanIce::EdgeResource::EdgeDevices::iterator devicesIter = localEdgeDevices.begin();
				std::string iterDeviceName = (*devicesIter)->getName();
				bool bDeviceExist = false;
				for(devicesIter;devicesIter != localEdgeDevices.end(); devicesIter++)
				{
					if((*devicesIter)->getName() == deviceNameOfPort)
					{
						bDeviceExist = true;
						break;
					}
				}
				if(!bDeviceExist)
					continue;
				TianShanIce::EdgeResource::EdgeDevicePrx remoteDevicePrx = remoteEdgeRMPrx->openDevice(deviceNameOfPort);
				if(remoteDevicePrx == NULL)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "sync device[%s] failed, can not open remote device[%s] in ERM[%s] endpoint[%s]"), deviceNameOfPort.c_str(), deviceNameOfPort.c_str(), netId.c_str(), endpoint.c_str());
					continue;;
				}
				TianShanIce::EdgeResource::RoutesMap routesMap = remoteDevicePrx->getRoutesRestriction(portIter->Id);
				TianShanIce::EdgeResource::RoutesMap::iterator routesMapIter = routesMap.find(routeIter->ident.name);
				if(routesMapIter == routesMap.end())
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "sync routeName failed, can not find route[%s] in port[%d] from device[%s] in remote ERM[%s] endpoint[%s]"), routeIter->ident.name.c_str(), portIter->Id, deviceNameOfPort.c_str(), netId.c_str(), endpoint.c_str());
					continue;
				}
				portCount++;
				_routeNameToNetId[routeIter->ident.name] = netId;
				TianShanIce::EdgeResource::RoutesMap localRoutesMap = (*devicesIter)->getRoutesRestriction(portIter->Id);
				TianShanIce::EdgeResource::RoutesMap::iterator localRoutesMapIter = localRoutesMap.find(routeIter->ident.name);
				if(localRoutesMapIter != localRoutesMap.end())
				{
					(*devicesIter)->unlinkRoutes(portIter->Id, routeIter->ident.name);
				}
				(*devicesIter)->linkRoutes(portIter->Id, routeIter->ident.name, routesMapIter->second);
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "link port[%d] to route[%s]"), portIter->Id, routeIter->ident.name.c_str());
			}
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed sync devices step 3:link all routes took %dms"), (int)(ZQ::common::TimeUtil::now()-stampStart));
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get Edge Resource Manager proxy at endpoint[%s] caught exception(%s)"), endpoint.c_str(),  ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get Edge Resource Manager proxy at endpoint[%s] caught unknown exception(%d)"), endpoint.c_str(), SYS::getLastErr());
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed sync all [%d] devices, link [%d] port to [%d] routeNames took %lldms"), deviceCount, portCount, routeCount, ZQ::common::TimeUtil::now()-stampBegin);
	return true;
}
bool ErmInstanceSyncer::syncAlloction(const std::string& netId, const std::string& endpoint, int64 since)
{
	char timeBuf[65]="";
	ZQ::common::TimeUtil::TimeToUTC(since, timeBuf, sizeof(timeBuf) -1);

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "sync allocations from ERM[%s] endpoint[%s] since[%s] by mode[%s]"), netId.c_str(), endpoint.c_str(), timeBuf, (esm_Active == _instanceMode)? "active" :"standby" );

	///get remote ERM service proxy
	bool bret = false;
	TianShanIce::EdgeResource::EdgeRMPrx edgeRMPrx = NULL;
	try 
	{
		Ice::ObjectPrx objPrx = NULL;
		edgeRMPrx  = TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(endpoint));
		#if  ICE_INT_VERSION / 100 >= 306
			objPrx = edgeRMPrx->ice_collocationOptimized(false);
		#else
			objPrx = edgeRMPrx->ice_collocationOptimization(false);
		#endif
		edgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(objPrx);
		edgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(edgeRMPrx->ice_timeout(pConfig._backup.exportTimeout));
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get Edge Resource Manager proxy at endpoint[%s] caught exception(%s)"), endpoint.c_str(),  ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get Edge Resource Manager proxy at endpoint[%s] caught unknown exception(%d)"), endpoint.c_str(), SYS::getLastErr());
	}

	if (NULL == edgeRMPrx)
		return bret;

	TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfo;
	int64 subtotalListDeviceTime = 0, subtotalUnmarshalTime = 0;
	subtotalListDeviceTime = ZQ::common::TimeUtil::now();
	try
	{
		TianShanIce::StrValues metedata;

		int64 stampStart  = ZQ::common::TimeUtil::now();

		if (esm_Active == _instanceMode)
		{
			// if Edge ERM, list Devices from local
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "listing device info from local"));
			deviceInfo = _env.getEdgeRMPrx()->listDevices(metedata);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed listed device info from local, got %d devices, took %dmsec"), deviceInfo.size(), int(ZQ::common::TimeUtil::now() - stampStart));
		}
		else
		{
			// if Gloable ERM, list Devices info from Edge ERM
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "listing device info from Edge ERM endpoint[%s]"), endpoint.c_str());
			deviceInfo = edgeRMPrx->listDevices(metedata);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed listed device info from Edge ERM endpoint[%s], got %d devices, took %dmsec"), endpoint.c_str(), deviceInfo.size(), int(ZQ::common::TimeUtil::now() - stampStart));
		}
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to list devices info at endpoint[%s] caught exception(%s)"), endpoint.c_str(),  ex.ice_name().c_str());
		return bret;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to list devices info at endpoint[%s] caught unknown exception(%d)"), endpoint.c_str(), SYS::getLastErr());
		return bret;
	}
	subtotalListDeviceTime = ZQ::common::TimeUtil::now() - subtotalListDeviceTime;
	/*
	//非Edge ERM, 则需要将 Device 与 EdgeERM的Net关联起来. 目前看起来不需要这个
	if (!bEdgeERM)
	{
	ZQ::common::MutexGuard g(_lkDeviceNameOfERM);
	for (int i = 0; i < deviceInfo.size(); i++)
	{
	MAPSET(TianShanIce::Properties,  _deviceNameOfERM, deviceInfo[i].ident.name, netId);
	}
	}
	*/
	int64 subtotalReadLocalTime =0,subtotalReadRemoteTime=0,  subtotalLocalOrphanTime=0, subtotalPeerOrphanTime=0, subtotalRewatchTime=0;
	int subtotalReadLocalCount =0, subtotalLocalOrphanCount=0, subtotalPeerOrphanCount=0,subtotalRewatchCount=0;
	for (size_t i = 0; i < deviceInfo.size(); i++)
	{
		int useTime = 0;
		int readLocalAllocTime=0, diffEERMAllocTime=0, removeAllocTime=0, addAllocTime=0, unmarshalTime=0, rewatchTime=0;
		int64 totalSyncTime = ZQ::common::TimeUtil::now();
		TianShanIce::StrValues localAllocIds; ///loacl ERM AllocationIds

		//get local allocs
		int64 ltimeStamp = ZQ::common::now();
		TianShanIce::EdgeResource::AlloctionValues remoteOrphans;
		TianShanIce::StrValues localOrphans;

		try
		{
			TianShanIce::EdgeResource::EdgeDevicePrx devicePrx = _env.getEdgeRMPrx()->openDevice(deviceInfo[i].ident.name);	
			#if  ICE_INT_VERSION / 100 >= 306
				Ice::ObjectPrx objectPrx = devicePrx->ice_collocationOptimized(false);
			#else
				Ice::ObjectPrx objectPrx = devicePrx->ice_collocationOptimization(false);
			#endif
			TianShanIce::EdgeResource::EdgeDeviceExPrx deviceExPrx = TianShanIce::EdgeResource::EdgeDeviceExPrx::uncheckedCast(objectPrx);
			localAllocIds = deviceExPrx->listAllocationIds();
			/*
			if (!getAllocations(deviceInfo[i].ident.name, localAllocIds))
			continue;
			*/
			useTime = (int)(ZQ::common::TimeUtil::now()-ltimeStamp);
			readLocalAllocTime = useTime;
			subtotalReadLocalTime += useTime;
			subtotalReadLocalCount += localAllocIds.size();
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed get [%d] local alloctions from device[%s], took %dmsec"),localAllocIds.size(), deviceInfo[i].ident.name.c_str(),useTime);	
		}
		catch (::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get local allocations in device[%s] endpoint[%s] caught exception(%s)"), deviceInfo[i].ident.name.c_str(),  endpoint.c_str(), ex.ice_name().c_str());
			continue;
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get local allocations in device[%s] endpoint[%s] caught unknown exception(%d)"), deviceInfo[i].ident.name.c_str(), endpoint.c_str(), SYS::getLastErr());
			continue;
		}
		try
		{
			//get remote allocs
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, " calling edgerm[%s] to get diffAllocations"), _env._adapter->getCommunicator()->proxyToString(edgeRMPrx).c_str());
			ltimeStamp = ZQ::common::now();
			//allocValues = edgeRMPrx->exportAllocations(deviceInfo[i].ident.name, timeBuf);
			remoteOrphans = edgeRMPrx->diffAllocations(deviceInfo[i].ident.name,localAllocIds,localOrphans);
			useTime = (int)(ZQ::common::now() - ltimeStamp);
			diffEERMAllocTime = useTime;
			subtotalReadRemoteTime += useTime;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed diffAllocations get [%d] remote orphan allocations  and  [%d] local orphan allocations  from peer[%s] device[%s], took %dmsec"), 
				remoteOrphans.size(), localOrphans.size(), endpoint.c_str(), deviceInfo[i].ident.name.c_str(), useTime);
		}
		catch (::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get diffAllocations in device[%s] endpoint[%s] caught exception(%s)"), deviceInfo[i].ident.name.c_str(),  endpoint.c_str(), ex.ice_name().c_str());
			continue;
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "failed to get diffAllocations in device[%s] endpoint[%s] caught unknown exception(%d)"), deviceInfo[i].ident.name.c_str(), endpoint.c_str(), SYS::getLastErr());
			continue;
		}
		typedef std::vector<TianShanIce::EdgeResource::AlloctionRecord> vecAllocs;
		vecAllocs allocRecs;
		int64 stampTemp = ZQ::common::TimeUtil::now();
		for (size_t k= 0; k < remoteOrphans.size(); k++)
		{
			TianShanIce::EdgeResource::AlloctionRecord allocRec;
			AllocationImpl::unmarshal(allocRec, remoteOrphans[k], _env._adapter->getCommunicator());
			allocRecs.push_back(allocRec);
		}
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "processed unmarshal [%d] allocations from peer[%s] device[%s], took %dmsec"), remoteOrphans.size(), endpoint.c_str(), deviceInfo[i].ident.name.c_str(), (int)(ZQ::common::TimeUtil::now()-stampTemp));
		subtotalUnmarshalTime = ZQ::common::TimeUtil::now() - stampTemp;
		unmarshalTime = subtotalUnmarshalTime;
		//local ERM中多余出来的Allocaion	 从DB中删除. Channel中删除
		stampTemp = ZQ::common::TimeUtil::now();
		for (size_t k= 0; k < localOrphans.size(); k++)
		{
			_removeWatchDog->addWatch(localOrphans[k]);
		}
		useTime = (int)(ZQ::common::TimeUtil::now()-stampTemp);

		removeAllocTime = useTime;
		subtotalLocalOrphanTime+= useTime;
		subtotalLocalOrphanCount += localOrphans.size();
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "processed  remove [%d] local allocations from peer[%s] device[%s], took %dmsec"), localOrphans.size(), endpoint.c_str(), deviceInfo[i].ident.name.c_str(), useTime);
		TianShanIce::StrValues rewatchAllocIds;
		std::sort(localAllocIds.begin(), localAllocIds.end());
		std::sort(localOrphans.begin(), localOrphans.end());
		std::set_difference(localAllocIds.begin(), localAllocIds.end(), localOrphans.begin(), localOrphans.end(), back_inserter(rewatchAllocIds));

		stampTemp = ZQ::common::TimeUtil::now();
		//交集rewatchAllocIds中的allocations 重新加入WatchDog;
		for (size_t k= 0; k < rewatchAllocIds.size(); k++)
		{ 
			 ::Ice::Identity allocIdentity ;
			 allocIdentity.name = rewatchAllocIds[k];
			 allocIdentity.category = DBFILENAME_Allocation;
			 _env._watchDog.watch(allocIdentity, 1000);
//			TianShanIce::EdgeResource::AllocationExPrx allocPrx = IdentityToObjEnv(_env, AllocationEx, allocIdentity);
//			allocPrx->OnRestore();
//			envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "rewatch allocation[%s]"), rewatchAllocIds[k].c_str());
		}
		useTime = (int)(ZQ::common::now() - stampTemp);
		rewatchTime = useTime;
		subtotalRewatchTime += useTime;
		subtotalRewatchCount +=  rewatchAllocIds.size();
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ErmInstanceSyncer, "processed rewatch [%d] local alloctions from peer[%s] device[%s], took %dmsec"), rewatchAllocIds.size(), endpoint.c_str(), deviceInfo[i].ident.name.c_str(), (int)(ZQ::common::TimeUtil::now()-stampTemp));
		stampTemp = ZQ::common::TimeUtil::now();
		int count = 0;
		//remoteDiffAllocIds, remote ERM多余出来的Allocaion, sync到local ERM
		for (size_t k= 0; k < allocRecs.size(); k++)
		{
			_addWatchDog->addWatch(allocRecs[k]);
		}
		useTime = (int)(ZQ::common::now() - stampTemp);
		addAllocTime = useTime;
		subtotalPeerOrphanTime += useTime;
		subtotalPeerOrphanCount += allocRecs.size();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "processed add [%d] alloctions to peer peer[%s] device[%s] took %dmsec"), allocRecs.size(), endpoint.c_str(), deviceInfo[i].ident.name.c_str(), (int)(ZQ::common::TimeUtil::now()-stampTemp));
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(ErmInstanceSyncer, "data collection processed sync device[%s]: read [%d] local allocations took %dms, diffAllocations took %dms, localOrphan[%d] took %dms, RemoteOrphan[%d] took %dms, unmarshal took[%d], rewatch took[%d] total %dms"), 
			deviceInfo[i].ident.name.c_str(), localAllocIds.size(), readLocalAllocTime, diffEERMAllocTime, localOrphans.size(), removeAllocTime, remoteOrphans.size(), addAllocTime, unmarshalTime, rewatchTime, (int)(ZQ::common::TimeUtil::now() - totalSyncTime));

		sleep(1);
	}
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(ErmInstanceSyncer,"data collection processed sync peer[%s]  deviceNum[%d] alloctions:read [%d] local alloction took %dms, diffAllocations took %dms,  localOrphan[%d] took %dms, PeerOrphan[%d] took %dms, Rewatch[%d] took %dms, get remote device took %dms, 	unmarshal allocations took %dms"),
		endpoint.c_str(), deviceInfo.size(),subtotalReadLocalCount,(int)(subtotalReadLocalTime),(int)(subtotalReadRemoteTime), subtotalLocalOrphanCount,(int)(subtotalLocalOrphanTime), subtotalPeerOrphanCount,(int)(subtotalPeerOrphanTime),subtotalRewatchCount,(int)(subtotalRewatchTime), (int)subtotalListDeviceTime, (int)subtotalUnmarshalTime);
	return true;
}

bool ErmInstanceSyncer::getAllocations(const std::string deviceName, TianShanIce::StrValues& allocIds)
{
	// get local allocation of device 
	Ice::ObjectPrx objPrx = NULL;
	TianShanIce::EdgeResource::EdgeDevicePrx edgeDevicePrx = _env.getEdgeRMPrx()->openDevice(deviceName);
	if(edgeDevicePrx == NULL)
	{ 	
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(ErmInstanceSyncer, "openDevice use [%s] failed"),deviceName.c_str());
		return false;
	}
	#if  ICE_INT_VERSION / 100 >= 306
		objPrx = edgeDevicePrx->ice_collocationOptimized(false);
	#else
		objPrx = edgeDevicePrx->ice_collocationOptimization(false);
	#endif
	edgeDevicePrx = TianShanIce::EdgeResource::EdgeDevicePrx::uncheckedCast(objPrx);

	if (edgeDevicePrx == NULL)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "open device[%s] failed"), deviceName.c_str());
		return false;
	}

	TianShanIce::EdgeResource::EdgePortInfos edgePorts = edgeDevicePrx->listEdgePorts();
	int64 bytesSize = 0;

	for (size_t i = 0; i < edgePorts.size(); i++)
	{
		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(SYS_PROP(Enabled));
		expectedMetaData.push_back(SYS_PROP(FreqRF));
		expectedMetaData.push_back(SYS_PROP(StartUDPPort));
		expectedMetaData.push_back(SYS_PROP(UdpPortStepByPn));
		expectedMetaData.push_back(SYS_PROP(StartProgramNumber));
		expectedMetaData.push_back(SYS_PROP(LowBandwidthUtilization));
		expectedMetaData.push_back(SYS_PROP(HighBandwidthUtilization));
		expectedMetaData.push_back(SYS_PROP(MaxSessions));
		expectedMetaData.push_back(SYS_PROP(IntervalPAT));
		expectedMetaData.push_back(SYS_PROP(IntervalPMT));
		expectedMetaData.push_back(SYS_PROP(symbolRate));

		TianShanIce::EdgeResource::EdgeChannelInfos edgeChannels = edgeDevicePrx->listChannels(edgePorts[i].Id, expectedMetaData, false);
		for (size_t j = 0; j < edgeChannels.size(); j++)
		{
			//get channel proxy
			TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = _env._openChannel(edgeChannels[j].ident.name);
			if (channelPrx == NULL)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ErmInstanceSyncer, "open channel[%s] failed"), edgeChannels[j].ident.name.c_str());
				return false;
			}

			//get allocations of this channel
			TianShanIce::EdgeResource::AllocationIds result = channelPrx->getAllocationIds();
			allocIds.insert(allocIds.end(),result.begin(),result.end());
		}
	}

	return true;
}

//-----------------------------
// class AddWatchDog
//-----------------------------
AddWatchDog::AddWatchDog(EdgeRMEnv& env, int interCount /* = 1000 */): _env(env), _interCount(interCount), _bQuit(false), _watchCount(0),_processedCout(0)
{
	_status = inactive;
}

AddWatchDog::~AddWatchDog()
{
	stop();
}

bool AddWatchDog::init(void)
{
	return true;
}

int AddWatchDog::run()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(AddWatchDog, "enter add DB watch dog thread"));
	int interCount = 0;
	int64 interTimeStamp = ZQ::common::TimeUtil::now();
	while(!_bQuit)
	{
		//			TianShanIce::EdgeResource::AlloctionRecord allocRec;
		//			AllocationImpl::unmarshal(allocRec, allocValues[k], _env._adapter->getCommunicator());
		std::vector<TianShanIce::EdgeResource::AlloctionRecord> toAddAllocs;
		TianShanIce::StrValues addedAllocs;
		int64 timeStamp = ZQ::common::TimeUtil::now();
		_processedCout = 0;
		int total = 0;
		{
			ZQ::common::MutexGuard lk(_lock);
			AllocRecs::iterator iter = _allocRescs.begin();

			for(iter; iter != _allocRescs.end();)
			{
				if(_status == inactive)
					_status = active;

				if(_processedCout >= _interCount)
				{
					_processedCout = 0;
					break;
				}
				toAddAllocs.push_back(iter->second);
				_processedCout++;
				_allocRescs.erase(iter++);
			}
		}

		std::vector<TianShanIce::EdgeResource::AlloctionRecord>::iterator toAddIter = toAddAllocs.begin();
		for(;toAddIter != toAddAllocs.end();toAddIter++)
		{
			TianShanIce::EdgeResource::AlloctionRecord allocRec = (*toAddIter);
			Ice::Long lstart  = ZQTianShan::now();
			TianShanIce::EdgeResource::AllocationExPrx allocPrx;
			AllocationImpl::Ptr alloc;

			try {
				total++;
				alloc  = new AllocationImpl(_env);
				alloc->ident              = allocRec.ident;
				alloc->state			  = allocRec.state;
				alloc->resources          = allocRec.resources;
				alloc->owner              = allocRec.owner;
				alloc->ownerKey           = allocRec.ownerKey;

				alloc->udpPort            = allocRec.udpPort;
				alloc->programNumber      = allocRec.programNumber;
				alloc->maxJitter          = allocRec.maxJitter;
				alloc->sourceIP           = allocRec.sourceIP;
				alloc->bandwidth          = allocRec.bandwidth;

				alloc->channelAssocs      = allocRec.channelAssocs;

				alloc->stampCreated       = allocRec.stampCreated;
				alloc->stampProvisioned   = allocRec.stampProvisioned;
				alloc->stampCommitted     = allocRec.stampCommitted;
				alloc->expiration         = allocRec.expiration;

				alloc->retrytimes         = allocRec.retrytimes;
				alloc->sessionGroup		  = allocRec.sessionGroup;
				alloc->onDemandSessionId  = allocRec.onDemandSessionId;
				alloc->qamSessionId		  = allocRec.qamSessionId;
				alloc->qamSessGroup       = allocRec.qamSessGroup;

				if (!alloc->sessionGroup.empty())
					alloc->owner = _env._allocOwnerPrx;

				TianShanIce::EdgeResource::EdgeChannelExPrx chExprx = _env._openChannel(allocRec.channelAssocs[0].identCh.name);
				alloc->channelAssocs[0].ch  = chExprx;

				//glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AddWatchDog, "adding alloc[%s] into the database"), alloc->ident.name.c_str());

				try
				{	
					Ice::Long lstart  = ZQTianShan::now();
					_env._eAllocation->add(alloc, alloc->ident);

					lstart  = ZQTianShan::now();
					allocPrx = IdentityToObjEnv(_env, AllocationEx, alloc->ident);
					alloc->channelAssocs[0].ch->addAllocLink(allocRec.programNumber, allocRec.link);

					allocPrx->OnRestore();
					//addedAllocs.push_back(alloc->ident.name);
					glog(ZQ::common::Log::L_INFO, CLOGFMT(AddWatchDog, "test add watch: add[%s] pn[%d]"), alloc->ident.name.c_str(), alloc->programNumber);
					interCount++;

					_watchCount--;
				}
				catch (const Freeze::DatabaseException& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddWatchDog,"Fail to add allocation [%s] object to DB caught '%s:%s'"), 
						alloc->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				}
				catch (const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddWatchDog,"Fail to add allocation [%s] object to DB caught '%s'"), 
						alloc->ident.name.c_str(), ex.ice_name().c_str());
				}
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddWatchDog, "caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddWatchDog, "caught exception[%s]"), ex.ice_name().c_str());
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(AddWatchDog, "caught unknown exception"));
			}
		}
		/*
		{
			TianShanIce::StrValues::iterator iter = addedAllocs.begin();
			ZQ::common::MutexGuard lk(_lock);
			for(;iter != addedAllocs.end(); iter++)
			{
				AllocRecs::iterator allocIter = _allocRescs.find(*iter);
				if(allocIter != _allocRescs.end())
				{
					_allocRescs.erase(allocIter);
				}
			}
		}
		addedAllocs.clear();
		*/
		toAddAllocs.clear();
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AddWatchDog, "processed added [%d] allocations took %lldms,"), total, ZQ::common::TimeUtil::now()-timeStamp);
		if((ZQ::common::TimeUtil::now() - interTimeStamp) > 60000)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(AddWatchDog, "add [%d] allocations in 1min"), interCount);
			interCount = 0;
			interTimeStamp = ZQ::common::TimeUtil::now();
		}
		_status = inactive;
		_event.wait(2000);
	}

	return 1;
}

void AddWatchDog::stop()
{
	_bQuit = true;
	_event.signal();
	SYS::sleep(1); // yield for 1 milli second.
}

void AddWatchDog::final(void)
{

}

int AddWatchDog::addWatch(TianShanIce::EdgeResource::AlloctionRecord& allocRec)
{
	{
		ZQ::common::MutexGuard lk(_lock);
		MAPSET(AllocRecs, _allocRescs, allocRec.ident.name, allocRec);
	}
	_watchCount++;
	if(_status == inactive)
		_event.signal();

	return 1;
}

//-----------------------------
// class RemoveWatchDog
//-----------------------------
RemoveWatchDog::RemoveWatchDog(	EdgeRMEnv& env, int interCount /* = 1000 */):_env(env), _interCount(interCount), _bQuit(false), _count(0), _watchCount(0),_processedCout(0)
{
	_status = inactive;
}

RemoveWatchDog::~RemoveWatchDog()
{
	stop();
}

bool RemoveWatchDog::init()
{
	return true;
}

int RemoveWatchDog::run()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(RemoveWatchDog, "enter remove DB watch dog thread"));
	int interCount = 0;
	int64 interTimeStamp = ZQ::common::TimeUtil::now();
	while(!_bQuit)
	{
		int64 timeStamp = ZQ::common::TimeUtil::now();
		_processedCout = 0;
		std::vector<std::string> toRemoveAllocs;
		TianShanIce::StrValues removedAllocs;
		int total = 0;
		{
			ZQ::common::MutexGuard lk(_lock);
			RemoveAllocIds::iterator iter = _allocIds.begin();
			for(iter;iter != _allocIds.end();)
			{
				if(_status = inactive)
					_status = active;

				if(_processedCout == _interCount)
				{
					_processedCout = 0;
					break;
				}
				_processedCout++;
				toRemoveAllocs.push_back(iter->first);
				_allocIds.erase(iter++);
			}
		}

		std::vector<std::string>::iterator removeAllocIter = toRemoveAllocs.begin();
		for (;removeAllocIter != toRemoveAllocs.end();removeAllocIter++)
		{
			std::string allocId;
			try
			{
				Ice::Identity identAlloc;
				identAlloc.category = DBFILENAME_Allocation;
				identAlloc.name = (*removeAllocIter);
				allocId = (*removeAllocIter);

				TianShanIce::EdgeResource::AllocationPrx allocPrx =  IdentityToObjEnv2(_env, Allocation, identAlloc);
				if (allocPrx == NULL)
				{
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(RemoveWatchDog, "remove GERM allocation in get Allocation[%s] proxy failed"), allocId.c_str());
					continue;
				}
				AllocationInfo allocInfo = allocPrx->getInfo();
				Ice::Int programNumber = atoi(allocInfo.props[SYS_PROP(ProgramNumber)].c_str());
				glog(ZQ::common::Log::L_INFO, CLOGFMT(RemoveWatchDog, "test remove watch, remove[%s] pn[%d]"), allocId.c_str(), programNumber);
	
				allocPrx->destroy();
				//removedAllocs.push_back(allocId.c_str());
				//envlog(ZQ::common::Log::L_INFO, CLOGFMT(RemoveWatchDog, "remove allocation[%s] from GERM"), allocId.c_str());

				_watchCount--;
				total++;
				interCount++;
			}
			catch (::Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(RemoveWatchDog, "failed to remove Allocation[%s] caught exception(%s)"), allocId.c_str(), ex.ice_name().c_str());
				continue;
			}
			catch (...)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(RemoveWatchDog, "failed to remove Allocation[%s] caught unknown exception(%d)"), allocId.c_str(), SYS::getLastErr());
				continue;
			}
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(RemoveWatchDog, "processed removed [%d] allocations took %lldms"), total, ZQ::common::TimeUtil::now()-timeStamp);

		/*
		{
			TianShanIce::StrValues::iterator iter = removedAllocs.begin();
			ZQ::common::MutexGuard lk(_lock);
			for(;iter != removedAllocs.end(); iter++)
			{
				RemoveAllocIds::iterator allocIter = _allocIds.find(*iter);
				if(allocIter != _allocIds.end())
				{
					_allocIds.erase(allocIter);
				}
			}
		}
		*/
		if((ZQ::common::TimeUtil::now() - interTimeStamp) > 60000)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(RemoveWatchDog, "remove [%d] allocations in 1min"), interCount);
			interCount = 0;
			interTimeStamp = ZQ::common::TimeUtil::now();
		}

		_status = inactive;
		_event.wait(2000);
	}
	return 1;
}

void RemoveWatchDog::stop()
{
	_bQuit = true;
	_event.signal();
	SYS::sleep(1);// yield for 1 milli second.
}

void RemoveWatchDog::final()
{
	
}

int RemoveWatchDog::addWatch(const std::string& allocId)
{
	std::string temp = allocId;
	if(_count == _interCount)
		_count = 0;

	{
		ZQ::common::MutexGuard lk(_lock);
		MAPSET(RemoveAllocIds, _allocIds, temp, _count);
	}

	_count++;
	_watchCount++;
	if(_status == inactive)
		_event.signal();
	
	return 1;
}

#if 0
// -----------------------------
// class ERMSync_StandbyInst
// -----------------------------
ERMSync_StandbyInst::ERMSync_StandbyInst(EdgeRMEnv& env, int syncInterval, EdgeERMinfos& edgeERMs)
: ErmInstanceSyncer(env, syncInterval), _edgeERMs(edgeERMs)
{
}

int	ERMSync_StandbyInst::run()
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ERMSync_StandbyInst, "enter run()"));
	int cFieldSyncRound = 2;

	while (!_bQuit)
	{
		_hWait.wait(_syncInterval);
		if (_bQuit)
			break;

		//sync alloction
		EdgeERMinfos::iterator itor;
		for (itor = _edgeERMs.begin(); !_bQuit && itor != _edgeERMs.end(); itor++)
		{
			// sync alloction from Edge ERM when Edge ERM status is ERMSync_StandbyInst::Active
			// 当Global ERM接手EdgeERM时,请更新EdgeERM status为ERMSync_StandbyInst::InActive.应该在S6的代码中添加
			if (itor->second.status == ERMSync_StandbyInst::Active && syncAlloction(itor->second.endpoint, 0, false))
			{
				itor->second.stampLastSync = ZQ::common::now();
			}
		}
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ERMSync_StandbyInst, "leave run()"));
	return -1;
}

bool ERMSync_StandbyInst::notifyChangeMode(const std::string& netId, int mode)
{
#pragma message ( __MSGLOC__ "TODO: change Edge ERM status")

	ZQ::common::MutexGuard g(_lkEdgeERMs);
	if (_edgeERMs.find(netId) == _edgeERMs.end())
		return false;

	if (mode == 0)
		_edgeERMs[netId].status = ERMSync_StandbyInst::InActive;
	else
		_edgeERMs[netId].status = ERMSync_StandbyInst::Active;
	_edgeERMs[netId].statusTimeStamp = ZQ::common::now();

	return true;  
}

std::string ERMSync_StandbyInst::getNetId(const std::string& routeName)
{
#pragma message ( __MSGLOC__ "TODO: get Edge NetId from routeName of EdgeERM netId map")
	return "";
}

/*
bool ERMSync_StandbyInst::rejectRequest(const std::string& deviceName)
{
std::string netId;
{
//在deviceName to Edge ERM Map中查找 Edge ERM, 如果找不到返回false, receive request
ZQ::common::MutexGuard g(_lkDeviceNameOfERM);
if (_deviceNameOfERM.find(deviceName) == _deviceNameOfERM.end())
return false;

netId = _deviceNameOfERM[deviceName];	
}

//根据NetId, 在 edgeERMs map中, 查找 Edge ERM的状态, 如果找不到返回false, receive request
{
//在deviceName to Edge ERM Map中查找 Edge ERM, 如果找不到返回false, receive request
//如果 EdgeERM的状态为 ERMSync_StandbyInst::Active,则reject request, 否则接受所有request
if (_edgeERMs.find(netId) == _edgeERMs.end())
return false;
if (_edgeERMs[netId].status == ERMSync_StandbyInst::InActive)
return false;
else if (_edgeERMs[netId].status == ERMSync_StandbyInst::Active)
return true;
}
}
*/
// -----------------------------
// class ERMSync_ActiveInst
// -----------------------------
ERMSync_ActiveInst::ERMSync_ActiveInst(EdgeRMEnv& env, int syncInterval, const std::string& gERMEndpoint, const std::string& netId, const std::string& localNetId)
: ErmInstanceSyncer(env, syncInterval), _gERMEndPoint(gERMEndpoint), _netId(netId), _localNetId(localNetId)
{

}

int	ERMSync_ActiveInst::run()
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ERMSync_ActiveInst, "enter run(), netId(%s)endpoint(%s)"), _netId.c_str(), _gERMEndPoint.c_str());

	for (int syncCount = 0; !_bQuit && syncCount < 2; syncCount++)
	{
		_hWait.wait(_syncInterval);
		if (_bQuit)
			break;

		//sync allocation from Global ERM.
		syncAlloction(_netId, _gERMEndPoint, 0);
	}

	// 同步两次后, 不管成功与否, 启动S6 Listener 端口
	_env.startS6();

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ERMSync_ActiveInst, "run() finished"));
	return -1;
}

bool ERMSync_ActiveInst::notifyChangeMode(const std::string& netId, int mode)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMSync_ActiveInst, "notifyChangeMode() netId[%s]"), netId.c_str());

	///当收到第一个 S6 Setup请求后, 强制将GERM中 netId的 Edge ERM status变为ERMSync_StandbyInst::InActive. 
	///mode =0 代表ERMSync_StandbyInst::InActive,1代表ERMSync_StandbyInst::Active;
	TianShanIce::EdgeResource::EdgeRMPrx edgeRMPrx = NULL;
	try 
	{
		edgeRMPrx = TianShanIce::EdgeResource::EdgeRMPrx::checkedCast(_env._adapter->getCommunicator()->stringToProxy(_gERMEndPoint));
		edgeRMPrx->forceBackupMode(netId, 1);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMSync_ActiveInst, "forceBackupMode netId[%s] to netId[%s ==> %s]"), netId.c_str(), _netId.c_str(), _gERMEndPoint.c_str());
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ERMSync_ActiveInst, "failed to get Edge Resource Manager proxy at endpoint[%s] caught exception(%s)"), _gERMEndPoint.c_str(),  ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ERMSync_ActiveInst, "failed to get Edge Resource Manager proxy at endpoint[%s] caught unknown exception(%d)"), _gERMEndPoint.c_str(), SYS::getLastErr());
	}

	return true;  
}

std::string ERMSync_ActiveInst::getNetId(const std::string& routeName)
{
	return  _localNetId;
}

#endif //0

}} // namespace
