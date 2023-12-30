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
// Ident : $Id: SentryEnv.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/SentryEnv.h $
// 
// 7     4/01/15 1:26p Build
// cleaned old snmp
// 
// 6     3/19/15 9:58p Zhiqiang.niu
// use ZQSnmp instead of old snmp
// 
// 5     8/29/13 6:00p Zonghuan.xiao
// add lock for _lockExpiredProcessIds while this set  is operating;
// ticket:14047
// 
// 4     7/30/12 3:24p Zonghuan.xiao
// refector  table of TianShan Modules(Sentry)  Oid =
// .1.3.6.1.4.1.22839.4.1.1100.3.1.1.1
// add  column IP
// 
// 3     7/27/12 4:55p Zonghuan.xiao
// implement  table of TianShan Modules(Sentry)  Oid =
// .1.3.6.1.4.1.22839.4.1.1100.3.1.1.1
// 
// 2     5/11/12 2:13p Hui.shao
// added config <http idleTimeout="300000" >
// 
// 2     5/11/12 2:05p Hui.shao
// added config <http idleTimeout="300000" />
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 15    09-07-06 14:31 Fei.huang
// * linux port
// 
// 14    08-07-14 19:33 Xiaohui.chai
// Added proxy page.
// 
// 13    08-06-10 11:31 Xiaohui.chai
// added machine type
// 
// 12    08-03-25 10:41 Xiaohui.chai
// 
// 11    07-12-14 16:18 Xiaohui.chai
// 
// 10    07-11-06 18:34 Xiaohui.chai
// 
// 9     07-11-05 15:57 Xiaohui.chai
//  remove adapter callback from ProcessInfo
// 
// 8     07-10-12 10:05 Xiaohui.chai
// 
// 7     07-09-18 12:56 Hongquan.zhang
// 
// 6     07-07-19 17:33 Hongquan.zhang
// 
// 5     07-07-19 17:29 Hongquan.zhang
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

#ifndef __ZQTianShan_SentrySvcConf_H__
#define __ZQTianShan_SentrySvcConf_H__

#include "../common/TianShanDefines.h"
#include "../common/ZqSentryIce.h"
#include "SentryPages.h"

//#include "SubAgent.hpp"
//#include "smival.hpp"

#include "Neighborhood.h"
#include "NativeThreadPool.h"
#include "InetAddr.h"
#include <set>


namespace ZQTianShan {
namespace Sentry {

#define MIN_GROUP_TIMEOUT        (5)  // 5 sec
#define MAX_GROUP_TIMEOUT        (5*60)  // 5 min
#ifdef _DEBUG
#  define DEFAULT_GROUP_TIMEOUT  (5)  // 5 sec
#else
#  define DEFAULT_GROUP_TIMEOUT  (60) // 1 min
#endif // _DEBUG

#define DECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ;
#define DECLARE_INDEX(_IDX)	TianShanIce::SRM::##_IDX##Ptr _idx##_IDX;

class LogParserManagement;
// -----------------------------
// class SentryEnv
// -----------------------------
class SentryEnv
{
public:
    SentryEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, Ice::CommunicatorPtr& communicator, const char* endpoint = DEFAULT_ENDPOINT_Sentry, const char* databasePath = NULL);
	virtual ~SentryEnv();

	void refreshSystemUsage();

public:	// exports all the members

	ZQ::common::InetMcastAddress _groupAddr;
	ZQ::common::InetHostAddress _groupBind;
	int				_groupPort;
	int				_timeout;

	SentryListener::Ptr _groupListener;
	SentryBarker::Ptr	 _groupBarker;
	//ZQ::Snmp::Subagent*  _snmpTableAgent;

	typedef struct _NodeInfo
	{
		std::string id;
		std::string name;
		std::string sentrysvcPrx;
		std::string adminRootUrl;
        ::Ice::Long osStartup;
		::Ice::Long lastChange;
		::Ice::Long lastHeartbeat;
		std::string cpu;
		uint32		cpuCount, cpuClockMHz;
		std::string os;
		uint32		memTotalPhys, memAvailPhys;
		uint32		memTotalVirtual, memAvailVirtual;
        std::string type;
	} NodeInfo;

	typedef std::map<std::string, NodeInfo> NodeMap; // nodeid to NodeInfo map

	typedef struct _RemoteServiceInfo
	{
		::ZqSentryIce::ServiceInfo baseInfo;
		std::string				 nodeid;
	} RemoteServiceInfo;

	typedef std::vector<RemoteServiceInfo> RemoteServices; // collection of remote services

	ZQ::common::Mutex	_lockNeighbors;			NodeMap  _neighbors;
	ZQ::common::Mutex	_lockRemoteServices;	RemoteServices  _remoteServices;

	// local machine information definitions
	NodeInfo _selfInfo;
	typedef struct _ProcessInfo
	{
		int				processId;
		std::string		daemon;			// name of the daemon or NT service
		::ZqSentryIce::LoggerInfos loggerInfos;
		::Ice::Long		lastHeartbeat;
	} ProcessInfo;

	typedef struct _AdapterInfo
	{
		std::string		adapterId;
		int				processId;
		std::string		endpoint;
		::Ice::Long		lastChange;
		::TianShanIce::StrValues interfaces;
	} AdapterInfo;

	typedef std::vector<ProcessInfo> Processes; // processId to ProcessInfo map
	typedef std::vector<AdapterInfo> Adapters; // adapterId to AdapterInfo map
	typedef std::set<int> IntSet;

	ZQ::common::Mutex	_lockLocalAdapters;		Adapters	_localAdapters;
	ZQ::common::Mutex	_lockLocalProcesses;	Processes	_localProcesses;
	ZQ::common::Mutex	_lockExpiredProcessIds;	IntSet		_expiredProcessIds; 

	::Ice::Long			_stampLastChange; // the stamp when any local process or interfaces has changed

	std::string				_dbPath;
	std::string				_endpoint;
	std::string				_programRootPath;
	ZQ::common::NativeThreadPool& _thpool;

	std::string				_webBind;
	unsigned short          _webPort;

	//for http service
	std::string				_webRootPath;
	std::string				_dllRootPath;



	Ice::CommunicatorPtr	_communicator;
    ZQADAPTER_DECLTYPE      _adapter;
    Ice::ObjectAdapterPtr   _loopbackAdapter;
	int						_loopbackPort;

	ZQ::common::Log&		_log;

	SentryPages				_pages;

	ZQTianShan::Sentry::LogParserManagement*	_logParserManagement;

	//////////////////////////////////////////////////////////////////////////
	//configuration for embeded http service
public:	
	const char*		FindReference(const std::string& key )
	{
		std::map<std::string,std::string>::iterator it = _reference.find(key);
        if(_reference.end() != it)
            return it->second.c_str();
		return NULL;
	}
public:
	std::map<std::string,std::string>	_reference;
	std::string					_strWebRoot;	
	std::string					_strDllConfig;
	std::string					_strDefaultPage;
	ZQ::common::Log*			_pHttpSvcLog;
	uint32                      _httpConnIdleTimeout;
//////////////////////////////////////////////////////////////////////////
	
protected:

	void gatherStaticSystemInfo();

public:
	typedef struct _FatNodeInfo
	{
		SentryEnv::NodeInfo baseNodeInfo;
		::ZqSentryIce::ServiceInfos services;
	} FatNodeInfo;

	typedef std::map<std::string, FatNodeInfo> FatNodeMap;

public:
	//bool refreshParticipantMachineTable();

	//ZQ::Snmp::TablePtr 	initParticipantMachineTable();

	//bool refreshNeighborsModulesTable();

	//ZQ::Snmp::TablePtr 	initNeighborsModulesTable();

	bool gatherNodeMap(FatNodeMap & fatNodeMap);
};

#define IdentityToObj(_CLASS, _ID) ::TianShanIce::SRM::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#define envlog			(_env._log)
#define httplog         (*_env._pHttpSvcLog)


}} // namespace

#endif // __ZQTianShan_SessionImpl_H__
