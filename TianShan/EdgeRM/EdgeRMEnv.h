// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: EdgeRMEnv.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMEnv.h $
// 
// 49    6/06/16 5:20p Li.huang
// ETV-TV-NOW-CR038
// 
// 48    4/01/15 4:58p Build
// cleaned old snmp
// 
// 47    5/13/14 1:47p Zonghuan.xiao
// snmp export of ERM, include ErmDevicesTable ,ErmPortsTable  ,
// ErmChannelsTable
// 
// 46    1/20/14 4:20p Bin.ren
// 
// 45    1/15/14 5:41p Bin.ren
// 
// 44    1/14/14 5:06p Bin.ren
// 
// 43    1/10/14 10:27a Bin.ren
// add DB operation threads
// 
// 42    11/27/13 2:21p Ketao.zhang
// EERM Fail back do 3 sync
// 
// 41    11/18/13 5:17p Bin.ren
// 
// 40    11/18/13 4:51p Bin.ren
// 
// 39    11/14/13 3:39p Bin.ren
// 
// 38    11/13/13 5:07p Bin.ren
// 
// 37    11/08/13 4:32p Ketao.zhang
// 
// 36    11/08/13 4:28p Ketao.zhang
// add get netId through routeName
// 
// 35    11/06/13 5:16p Bin.ren
// change max sessions count
// 
// 34    10/23/13 3:47p Bin.ren
// 
// 33    10/22/13 5:28p Bin.ren
// add change erm work mode
// 
// 32    10/16/13 5:55p Bin.ren
// add sync deivces and routeName
// 
// 31    10/15/13 5:22p Bin.ren
// change ImportDeviceCmd from import only one device to import all
// devices in xml file
// 
// 30    10/15/13 9:39a Hui.shao
// consts
// 
// 29    10/11/13 3:15p Hui.shao
// merged the two sync-ers into one
// 
// 28    10/11/13 11:53a Hui.shao
// rename ERMSync-ers
// 
// 27    10/11/13 10:59a Bin.ren
// 
// 26    9/24/13 4:33p Li.huang
// 
// 25    9/24/13 4:15p Li.huang
// 
// 24    9/24/13 2:42p Li.huang
// sync allocation
// 
// 23    9/13/13 2:24p Bin.ren
// 
// 21    9/11/13 4:53p Li.huang
// 
// 20    9/11/13 1:26p Li.huang
// marshal alloction
// 
// 19    7/01/13 3:58p Li.huang
// 
// 18    7/01/13 2:01p Bin.ren
// 
// 16    6/18/13 11:35a Li.huang
// 
// 15    6/08/13 2:59p Li.huang
// add limit max session of channel
// 
// 14    5/23/13 4:00p Li.huang
// 
// 13    5/22/13 3:40p Li.huang
// add it to linux build
// 
// 12    4/15/13 5:19p Li.huang
// 
// 11    3/28/13 1:49p Bin.ren
// 
// 10    3/25/13 2:21p Bin.ren
// 
// 9     3/20/13 11:36a Li.huang
// 
// 8     3/20/13 11:19a Li.huang
// add R6
// 
// 7     2/26/13 2:03p Li.huang
// 
// 6     1/16/13 10:11a Li.huang
// add tripserver
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
// 22    09-12-22 14:54 Hui.shao
// removed the unecessary locker-of-evictor
// 
// 21    09-12-10 18:20 Li.huang
// remove deviceOfchannel Index 
// 
// 20    09-11-26 15:42 Li.huang
// fix some bugs 
// 
// 19    09-11-24 17:39 Li.huang
// 
// 18    09-11-20 17:02 Li.huang
// 
// 17    09-11-18 16:51 Li.huang
// 
// 16    09-11-18 16:04 Li.huang
// fix some bugs
// 
// 15    09-11-02 16:36 Li.huang
// 
// 14    09-10-30 11:58 Li.huang
// 
// 13    09-10-19 10:28 Li.huang
// 
// 12    09-10-15 18:25 Li.huang
// 
// 11    09-10-08 15:01 Li.huang
// 
// 9     09-09-28 16:10 Li.huang
// 
// 8     09-08-10 14:38 Xiaoming.li
// add S6 message handler
// 
// 7     09-08-07 9:54 Xiaoming.li
// add S6 handler
// 
// 6     09-07-14 11:03 Xiaoming.li
// 
// 5     09-03-26 15:17 Hui.shao
// impl of state inservice and outofservice
// 
// 4     09-03-19 17:12 Hui.shao
// init draft of evaluate, commit and withdraw,
// plus the states of allocation
// 
// 3     09-03-11 11:38 Hui.shao
// build up the index from service group to channels
// 
// 2     09-03-05 19:41 Hui.shao
// defined program structure to impl
// 
// 1     09-02-26 17:53 Hui.shao
// initial created
// ===========================================================================

#ifndef __ZQTianShan_EdgeRMEnv_H__
#define __ZQTianShan_EdgeRMEnv_H__

#include "../common/TianShanDefines.h"
#include "EdgeRM.h"
#include "EdgeRMImpl.h"
#include "SystemUtils.h"
#include "RtspDak.h"
#include "RtspEngine.h"
#include "EdgeRMFactory.h"
#include "DeviceOfZONE.h"
#include "AllocationOwnerImpl.h"
#include "S6Handler.h"
#include "NativeThreadPool.h"
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include "../common/vrep/VrepListener.h"
#include "D6Update.h"
#include "InetAddr.h"
#include "ERMIClient.h"
#include "R6Client.h"
#include "OwnerOfAllocationEx.h"
#include "QAMSessOfAllocationEx.h"
#include "ClientCmds.h"
#include "TripServerService.h"
#include "SystemUtils.h"
// #include "EdgeRMEnvSnmp.h"

#ifdef _DEBUG
#  pragma comment(lib, "Iced")
#  pragma comment(lib, "IceUtild")
#  pragma comment(lib, "freezed")
#else
#  pragma comment(lib, "Ice")
#  pragma comment(lib, "IceUtil")
#  pragma comment(lib, "freeze")
#endif //_DEBUG

// #define YTD_EVICTOR

namespace ZQTianShan {
namespace EdgeRM {

#define MAX_INSTANCE_IDLE            (2*60*60*1000)  // 2 hr
#define UNATTENDED_TIMEOUT           (30*1000)  // 30 sec
#define OUTSERVICE_TIMEOUT           (5*1000)  // 5 sec
#define COST_MAX                     (10000)
#define COST_UNAVAILABLE             (COST_MAX +1)

#define MAX_PRESERVE_CHANNELS        (2)
#define MAX_EVALUATE_CHANNELS        (5)

#define MAX_SESSION_CHANNELS         (50)

#define INST_SYNC_S6_SERVER_YIELD_ROUNDS  (3)
#define INST_SYNC_INTERVAL_MIN       (2*60*1000)  // 2min

// #define	_TestLog

class EdgeRMImpl;
class S6Handler;
class ErmInstanceSyncer;
class EdgeRMEnv;
//class EnvSnmpRegistor;


#define DECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; // ZQ::common::Mutex _lock##_OBJ;
#define DECLARE_INDEX(_IDX)	TianShanIce::EdgeResource::_IDX##Ptr _idx##_IDX;

typedef std::map<std::string, IdentCollection> DeviceChannelsMap;

// -----------------------------
// class ERMISGWatchDog
// -----------------------------
class QamSGWatchDog : public ZQ::common::NativeThread
{
	friend class ERMISessionGroup;
	friend class R6SessionGroup;
	typedef SYS::SingleObject Event; // for the stupid naming of SingleObject

public:
	QamSGWatchDog(EdgeRMEnv& env);
	virtual ~QamSGWatchDog();

public:
	///@param[in] contentIdent identity of the object
	///@param[in] timeout the timeout to wake up timer to check the specified object
	void    watch(ERMISessionGroup::Ptr group, ::Ice::Long syncInterval = 0);
	void    watch(R6SessionGroup::Ptr group,::Ice::Long syncInterval = 0);
	void	unwatch(ERMISessionGroup::Ptr group);
	void	unwatch(R6SessionGroup::Ptr group);
	//quit watching
	void quit();

protected:

	int		run();
	//used for third party to stop this thread
	int		terminate(int code );

private:

	EdgeRMEnv&             _env;

	typedef std::multimap <ERMISessionGroup::Ptr, Ice::Long > ERMISyncMap; // sessGroup to expiration map
	typedef std::multimap <R6SessionGroup::Ptr,Ice::Long >    R6SyncMap;
	ZQ::common::Mutex   _lockERMIGroups;
	ZQ::common::Mutex	_lockR6Groups;
	ERMISyncMap			_ERMIGroupsToSync;
	R6SyncMap			_R6GroupsToSync;
	Event	            _event;

	bool				_bQuit;
};

// -----------------------------
// class S6Connection
// -----------------------------
class S6Connection : public virtual ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer < S6Connection > Ptr;

public:
	S6Connection(EdgeRMEnv& env, const std::string groupName);
	virtual ~S6Connection();

public:
	std::string getGroupName()const;
	size_t getSessionList(std::vector<std::string>& sessions);

	void add(std::string sessionId, ZQ::DataPostHouse::IDataCommunicatorPtr connId);
	void update(std::string sessionId, ZQ::DataPostHouse::IDataCommunicatorPtr connId);
	void remove(std::string sessionId);

	ZQ::DataPostHouse::IDataCommunicatorPtr findConnectionId(std::string sessionId);
	int	connectionError(ZQ::DataPostHouse::IDataCommunicatorPtr connId);

	void updateS6ConnectionId(ZQ::DataPostHouse::IDataCommunicatorPtr connId);
	ZQ::DataPostHouse::IDataCommunicatorPtr getSGConnectionId(){return _connId;};
    
protected:
	typedef std::map< std::string, ZQ::DataPostHouse::IDataCommunicatorPtr > SessionConnections; /// map sessionID to connectId;

	SessionConnections _sessConnectionIds;
	ZQ::common::Mutex  _lockSessConnectionIds;

	std::string _groupName;
	EdgeRMEnv&  _env;

	ZQ::DataPostHouse::IDataCommunicatorPtr _connId;

public:	 
	static std::map<std::string, S6Connection::Ptr> _S6Connections;
	static ZQ::common::Mutex _lockS6Sessions;

	static S6Connection::Ptr findS6Connection(const std::string& groupName); // to find session group
	static std::vector<std::string> getAllS6Connection(); // return all group name
	static S6Connection::Ptr OpenS6Connection(std::string groupName, EdgeRMEnv& env,bool bCreate = true);
	static void  clearAll();
};

// -----------------------------
// class EdgeRMEnv
// -----------------------------
class EdgeRMEnv
{
	friend class PathHelperMgr;

public:
	EdgeRMEnv(ZQ::common::Log& log, ZQ::common::Log& eventlog,
					 ZQ::common::NativeThreadPool& threadPool, 
					 ZQADAPTER_DECLTYPE& adapter,
					std::string databasePath, 
					ZQ::common::FileLog& _pRtspEngine);
//	EdgeRMEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, 
//		Ice::CommunicatorPtr& communicator, const char* endpoint = DEFAULT_ENDPOINT_EdgeRM, const char* databasePath = NULL);

	virtual ~EdgeRMEnv();

	virtual bool initialize();
	void uninitialize();

public:
	TianShanIce::EdgeResource::EdgeChannelExPrx _openChannel(const ::std::string& chName);
	void updateServiceGroupIndex(IN const ::std::string& portHirechy, IN OUT ::TianShanIce::IValues& PortSvcGrpOfDevice, const ::TianShanIce::IValues& newServiceGroups);
    
	bool updataAllocation(const std::string& allocId, const std::string& eqamSessGroup, const std::string& sessionId);
	bool syncERMISession(const std::string& allocId, const std::string& eqamSessGroup, const std::string& sessionId);
	bool syncR6Session(const std::string& allocId, const std::string& eqamSessGroup, const std::string& sessionId);

	bool startS6();

	TianShanIce::EdgeResource::EdgeRMPrx getEdgeRMPrx(){ return _edgeRMPrx;};
//    bool getAllocation(const std::string& sessionId);

public:
	ZQ::common::Log&		        _log;
	ZQ::common::Log&		        _eventlog;
	ZQADAPTER_DECLTYPE&		        _adapter;
	ZQ::common::NativeThreadPool&	_thpool;
	ZQ::common::NativeThreadPool	_clientThpool;
	ZQTianShan::TimerWatchDog	    _watchDog;
	QamSGWatchDog*	                _QamSGWatchDog;

//	DECLARE_INDEX(ChannelOfDevice);
	DECLARE_INDEX(DeviceOfZONE);
	DECLARE_INDEX(OwnerOfAllocationEx);
    DECLARE_INDEX(QAMSessOfAllocationEx);

	DECLARE_CONTAINER(EdgeChannel);	

	DECLARE_CONTAINER(EdgeDevice);
	
	DECLARE_CONTAINER(Allocation);

	DECLARE_CONTAINER(PhoAllocation);

	
	EdgeRMFactory::Ptr			_factory;

	std::string				    _programRootPath;
	std::string                 _netId;

//	typedef ::std::multimap <Ice::Int, Ice::Identity> ServiceGroupIndex;
//	ServiceGroupIndex      _idxServiceGroup;  // the index of servicegroup Id to channel ident 
//	ZQ::common::Mutex      _lkIdxServiceGroup;

	typedef std::multimap <std::string, Ice::Identity> RouteIndex;
	typedef std::map <std::string, std::string> QamTypeIndex;
	RouteIndex				_idxRouteName;  // the index of route name to channel ident 

	QamTypeIndex		   _idxQamType;
	ZQ::common::Mutex      _lkIdxRouteName;
	ZQ::common::Mutex	   _lkIdxQamType;
	DeviceChannelsMap      _devicechannels;
	ZQ::common::Mutex      _lkdevicechannels;

	int      _retryInterval;
	int      _retrytimes;

protected:	
    ZQTianShan::EdgeRM::S6Handler* _s6Handle;
	ZQRtspCommon::IRtspDak*       _rtspDak;
	ZQRtspEngine::RtspEngine*     _rtspEngine;	
    bool                          _bS6Started;
	EdgeRMImpl::Ptr					_edgeRMPtr;
	TianShanIce::EdgeResource::EdgeRMPrx _edgeRMPrx;

	TripSocketServer* _pTripServer;
//	EnvSnmpRegistor   _edgeEnvSnmp;

public: // service configurations
	std::string				_dbPath;
	std::string				_dbRuntimePath;
	std::string				_modulePath;
	int _deviceEvictorSize, _channelEvictorSize, _allocationEvictorSize;
	size_t _maxPreserveChannels, _maxEvaluateChannels;
	Ice::Long             _allocationLeaseMs; ///< the lease time in msec
	Ice::Long             _s6AllocationLeaseMs;

	AllocationOwnerImplPtr	_allocOwnerPtr;
    TianShanIce::EdgeResource::AllocationOwnerPrx       _allocOwnerPrx;

	ZQ::common::FileLog& _RtspEngineLog;

	ZQ::Vrep::Server* _pVrepServer;
	D6Factory*  _pD6Factory;
	QAMTimerObjects *_pQamTimerObjects;

	std::string						_userAgent	;
	int								_ermiSessTimeout;

	ZQ::common::InetHostAddress     _ermiBindAddr;

	int								_r6SessTimeout;
	ZQ::common::InetHostAddress     _r6BindAddr;

	bool                            _enableERMI;
	bool                            _enableR6;

	ErmInstanceSyncer*               _pErmInstanceSyncer;

public:
	void OnRestore();
	bool RestoreChannelsOfDevice();
//	bool RestoreChannelsOfDeviceSample();
//	bool AddChOfDevice(std::string deviceName);
	bool addChannelsToDevice(const std::string& deviceName, IdentCollection& channellists);
	bool addChannelToDevice(const std::string& deviceName, const Ice::Identity& identCh);
	bool removeChannelsFromDevice(const std::string& deviceName);
	bool removeChannelFromDevice(const std::string& deviceName,const Ice::Identity& identCh);

	inline ZQ::common::Log&	getLogger() {return _log;};
	inline ZQ::common::NativeThreadPool& getThreadPool(){ return _thpool;};
	inline ZQ::common::NativeThreadPool& getClientThreadPool(){return _clientThpool;}
	ZQ::common::Log::loglevel_t getRtspLogLevel();
	EdgeRMImpl::Ptr getEdgeRmPtr(void){return _edgeRMPtr;}

	bool insertQam(::std::string qamName,::std::string TFTP);
	bool removeQam(::std::string qamName);

	bool createQamSessionGroup(const std::string& qamName, const std::string& TFTP);
	bool removeQamSessionGroup(const std::string& qamName);

	bool qamSessSetup(std::string& allocId,std::string qamName);

	void ermiSessSetup(std::string& allocId);
	void ermiSessTearDown(const std::string& allocId, const std::string& qamName, const std::string& sessionID);

	//R6
	void r6SessSetup(std::string& allocId);
	void r6SessTearDown(const std::string& allocId, const std::string& sessionID);

protected:
	void openDB(const char* dbRuntimePath);
	void closeDB(void);

	bool initD6();
	bool initTripServer();
};


class AddWatchDog : public ZQ::common::NativeThread
{
public:
	AddWatchDog(EdgeRMEnv& env, int interCount = 1);
	virtual ~AddWatchDog();

	virtual bool init(void);
	virtual int run();
	virtual void final(void);
	virtual void stop();

	int addWatch(TianShanIce::EdgeResource::AlloctionRecord& allocRec);
	typedef std::map<std::string, TianShanIce::EdgeResource::AlloctionRecord> AllocRecs;

private:
	AllocRecs _allocRescs;
	SYS::SingleObject _event;
	int _interCount;
	bool _bQuit;
	int _watchCount;
	int _processedCout;
	ZQ::common::Mutex _lock;
	EdgeRMEnv& _env;

	enum DBWatchDogStatus{
		active,
		inactive
	}_status;
};

class RemoveWatchDog : public ZQ::common::NativeThread
{
public:
	RemoveWatchDog(	EdgeRMEnv& _env, int interCount = 1);
	virtual ~RemoveWatchDog();

	virtual bool init(void);
	virtual int run();
	virtual void final(void);
	virtual void stop();

	int addWatch(const std::string& allocId);

	typedef std::map<std::string, int> RemoveAllocIds;
private:
	RemoveAllocIds _allocIds;
	SYS::SingleObject  _event;
	int _interCount;
	bool _bQuit;
	int _count;
	int _watchCount;
	int _processedCout;
	ZQ::common::Mutex _lock;
	EdgeRMEnv& _env;

	enum DBWatchDogStatus{
		active,
		inactive
	}_status;

};

// -----------------------------
// class ErmInstanceSyncer
// -----------------------------
class ErmInstanceSyncer : public ZQ::common::NativeThread
{
public:
	typedef enum 
	{
		esm_Standby = 0, 
		esm_Active,
	} SyncMode;

	typedef struct  
	{
		std::string     netId;
		std::string		endpoint;            // Edge ERM enpoint
		int64           stampLastSync;       // last allocation sync timestamp
		int64			stampDeviceLastSync;
		SyncMode        status;              // Edge ERM status, "active" or "inactive"
		int64			stampStatusAsOf;     // Edge ERM status timestamp
		bool            b1stS6Msg;
	} PeerInfo;

	typedef std::map< std::string, PeerInfo > PeerinfoMap; //map netid to Edge ERM info


public:

	ErmInstanceSyncer(EdgeRMEnv& env, SyncMode instanceMode, int syncInterval, const PeerinfoMap& peerMap)
		: _env(env), _instanceMode(instanceMode), _syncInterval(syncInterval), _bQuit(false), _peerMap(peerMap)
	{
		_addWatchDog = new AddWatchDog(_env, 1000);
		_removeWatchDog = new RemoveWatchDog(_env, 1000);
	}

	virtual ~ErmInstanceSyncer() {};

	virtual void stop()
	{
		_bQuit = true;

		if(_addWatchDog)
		{
			_addWatchDog->stop();
			delete _addWatchDog;
		}

		if(_removeWatchDog)
		{
			_removeWatchDog->stop();
			delete _removeWatchDog;
		}
		_hWait.signal();
	}

public:
	/// notify change Edge ERM state, if local ERM is Global ERM, update EdgeERM state from Edge ERM map
	/// else local ERM is Edge ERM, when it receive first S6 message, call forceBackupMode interface notify GERM updata it state
	///@param[in] netId     Edge ERM netId
	///@param[in] mode      state, 1 "active", 0 "inactive"
	virtual bool changeWorkForMode(const std::string& peerNetId, int mode);

	/// get ERM netId by routeName
	///@param[in] routeName    route name
	virtual std::string getNetId(const std::string& routeName);

	PeerinfoMap& getPeerInfoMap(){ return _peerMap; }
	SyncMode getInstanceMode(){ return _instanceMode; }
	void setInstanceMode(ZQTianShan::EdgeRM::ErmInstanceSyncer::SyncMode mode){ _instanceMode = mode; }

	//	virtual bool rejectRequest(const std::string& deviceName){return  false;};
protected:

	virtual int	 run();

	/// sync allocation info from remote ERM
	///@param[in] netId     remote ERM netId
	///@param[in] endpoint  remote ERM endpoint
	///@param[in] since     sync time point
	///@param[in] bEdgeERM  indicate local ERM is EdgeERM or GERM
	bool syncAlloction(const std::string& netId, const std::string& endpoint, int64 since);

	bool syncDevicesAndRoutes(const std::string& netId, const std::string& deviceName, const std::string endpoint, int64 since);

	/// get local allocation info by device name
	///@param[in] deviceName device name
	///@param[out] allocIds  local ERM allocation info
	bool getAllocations(const std::string deviceName, TianShanIce::StrValues& allocIds);

protected:

	//add map routeName to netId map
	EdgeRMEnv&             _env;
	bool                   _bQuit;
	SYS::SingleObject      _hWait;
	int                    _syncInterval;

	SyncMode               _instanceMode;
	PeerinfoMap            _peerMap;
	ZQ::common::Mutex      _lkPeerMap;
	std::map <std::string, std::string> _routeNameToNetId;
	ZQ::common::Mutex      _lkDeviceNameOfERM;
	TianShanIce::Properties _deviceNameOfERM; // map devicename to  edge ERM netId, 暂时不需要
	AddWatchDog* _addWatchDog;
	RemoveWatchDog* _removeWatchDog;
};

/*
// -----------------------------
// class ERMSync_StandbyInst
// -----------------------------
class ERMSync_StandbyInst : public ErmInstanceSyncer
{
public:
	typedef enum 
	{
		InActive = 0, 
		Active,
	} EdgeERMStatus;

	typedef struct  
	{
		std::string		endpoint;            ///Edge ERM enpoint
		int64           lastSyncTimeStamp;   /// last allocation sync timestamp
		EdgeERMStatus	status;              ///Edge ERM status, "active" or "inactive"
		int64			statusTimeStamp;     ///Edge ERM status timestamp
	} EdgeERMinfo;

	typedef std::map< std::string, EdgeERMinfo > EdgeERMinfos; //map netid to Edge ERM info

public:
	ERMSync_StandbyInst(EdgeRMEnv& env, int syncInterval, EdgeERMinfos& edgeERMs);
	virtual ~ERMSync_StandbyInst() {}

public:
	virtual bool notifyChangeMode(const std::string& netId, int mode);
	virtual std::string getNetId(const std::string& routeName);

//	virtual bool rejectRequest(const std::string& deviceName);

protected:
	virtual int		run();

private:
	ZQ::common::Mutex      _lkEdgeERMs;
	EdgeERMinfos		   _edgeERMs;
};

// -----------------------------
// class ERMSync_ActiveInst
// -----------------------------
class ERMSync_ActiveInst : public ErmInstanceSyncer
{
public:
	ERMSync_ActiveInst(EdgeRMEnv& env, int syncInterval, const std::string& gERMEndpoint, const std::string& netId, const std::string& localNetId);
	virtual ~ERMSync_ActiveInst() {};

protected:
	virtual int		run();

public:
	virtual bool notifyChangeMode(const std::string& netId, int mode);
	virtual std::string getNetId(const std::string& routeName);

private:
	std::string            _gERMEndPoint; /// Gloable ERM endpoint;
	std::string            _netId;        /// Gloable ERM netId;
	std::string            _localNetId;   /// Local ERM netId;
};

*/


#define DBFILENAME_EdgeDevice			"EdgeDevices"
#define DBFILENAME_EdgeChannel			"EdgeChannels"
#define DBFILENAME_Allocation			"Allocations"
//#define DBFILENAME_S6Allocation		    "S6Allocations"
#define DBFILENAME_Port		            "Ports"
#define DBFILENAME_ServiceGroup		    "ServiceGroups"
#define INDEXFILENAME(_IDX)	#_IDX "Idx"

#define IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::EdgeResource::_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))
#define IdentityToObjEnv2(_ENV, _CLASS, _ID) ::TianShanIce::EdgeResource::_CLASS##Prx::checkedCast((_ENV)._adapter->createProxy(_ID))
#define envlog			(_env._log)
#define evntlog		    (_env._eventlog)

typedef TianShanIce::ObjectInfo         EdgeDeviceInfo;
typedef TianShanIce::StatedObjInfo      EdgeChannelInfo;
typedef TianShanIce::StatedObjInfo      AllocationInfo;

}} // namespace

#endif // __ZQTianShan_EdgeRMEnv_H__
