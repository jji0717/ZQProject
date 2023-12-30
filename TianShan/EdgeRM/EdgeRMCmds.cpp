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
// Ident : $Id: EdgeRMCmds.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMCmds.cpp $
// 
// 57    6/06/16 5:20p Li.huang
// ETV-TV-NOW-CR038
// 
// 56    1/12/16 11:40a Dejian.fei
// 
// 55    1/11/16 5:50p Dejian.fei
// 
// 54    1/16/14 1:31p Bin.ren
// 
// 53    1/15/14 3:48p Bin.ren
// 
// 52    1/10/14 10:27a Bin.ren
// add DB operation threads
// 
// 51    12/18/13 3:42p Bin.ren
// 
// 50    12/05/13 4:59p Bin.ren
// add auto link when import device for test
// 
// 49    11/29/13 5:36p Ketao.zhang
// 
// 48    11/27/13 2:29p Ketao.zhang
// add DiffAllocCmd
// 
// 47    11/15/13 12:59p Bin.ren
// 
// 46    11/14/13 4:21p Bin.ren
// 
// 45    11/14/13 3:55p Bin.ren
// 
// 44    11/13/13 4:25p Bin.ren
// 
// 43    11/13/13 2:45p Bin.ren
// 
// 42    11/13/13 1:32p Bin.ren
// 
// 41    11/11/13 4:03p Ketao.zhang
// 
// 40    11/08/13 5:43p Bin.ren
// 
// 39    11/08/13 3:17p Bin.ren
// 
// 38    11/08/13 1:57p Bin.ren
// add device update
// 
// 37    11/07/13 11:46a Bin.ren
// 
// 36    11/07/13 11:03a Bin.ren
// 
// 35    11/07/13 9:42a Bin.ren
// 
// 34    11/06/13 4:26p Bin.ren
// 
// 33    11/05/13 2:33p Bin.ren
// 
// 32    11/05/13 10:51a Bin.ren
// 
// 31    11/05/13 10:40a Bin.ren
// 
// 30    11/04/13 5:08p Bin.ren
// 
// 29    11/04/13 4:56p Bin.ren
// 
// 28    11/04/13 1:53p Bin.ren
// 
// 27    11/04/13 12:00p Bin.ren
// 
// 26    11/04/13 10:44a Bin.ren
// 
// 25    11/01/13 4:34p Bin.ren
// 
// 24    10/22/13 2:30p Bin.ren
// 
// 23    10/21/13 5:44p Bin.ren
// 
// 22    10/21/13 3:27p Bin.ren
// 
// 21    10/21/13 10:30a Bin.ren
// 
// 20    10/18/13 4:48p Bin.ren
// 
// 19    10/18/13 2:38p Bin.ren
// 
// 18    10/18/13 9:33a Bin.ren
// 
// 17    10/17/13 2:56p Bin.ren
// 
// 16    10/16/13 5:55p Bin.ren
// add sync deivces and routeName
// 
// 15    10/15/13 5:41p Bin.ren
// 
// 14    10/15/13 5:22p Bin.ren
// change ImportDeviceCmd from import only one device to import all
// devices in xml file
// 
// 13    10/11/13 5:49p Bin.ren
// add exportDevice interface
// 
// 12    9/13/13 2:22p Bin.ren
// 
// 11    9/11/13 4:48p Bin.ren
// 
// 10    9/11/13 4:13p Bin.ren
// 
// 9     9/11/13 1:26p Li.huang
// marshal alloction
// 
// 8     6/18/13 11:32a Li.huang
// update D6
// 
// 7     6/08/13 2:59p Li.huang
// add limit max session of channel
// 
// 6     5/23/13 4:00p Li.huang
// 
// 5     3/28/13 1:49p Bin.ren
// 
// 4     3/25/13 2:21p Bin.ren
// 
// 3     3/20/13 11:19a Li.huang
// add R6
// 
// 2     11/01/12 3:29p Li.huang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 28    10-02-01 17:17 Li.huang
// fix some bugs
// 
// 27    10-01-14 15:59 Li.huang
// add channel tsid and nitpid
// 
// 26    10-01-05 16:28 Li.huang
// add  replace deviceIp and Mac from import Device interface
// 
// 25    10-01-04 11:44 Li.huang
// modify resource edgeDeviceGroup to edgeDeviceZone
// 
// 24    09-12-18 9:33 Li.huang
// restore
// 
// 24    09-12-10 18:21 Li.huang
// remove deviceOfChannel Index
// 
// 23    09-11-18 16:04 Li.huang
// fix some bugs
// 
// 22    09-10-30 11:58 Li.huang
// 
// 21    09-10-21 17:02 Li.huang
// 
// 20    09-10-14 18:14 Li.huang
// 
// 19    09-09-28 16:10 Li.huang
// 
// 18    09-08-03 14:09 Xiaoming.li
// add symbolRate for channel
// 
// 17    09-07-31 9:36 Xiaoming.li
// 
// 16    09-07-30 17:16 Xiaoming.li
// get edgeport info
// 
// 15    09-07-27 17:00 Xiaoming.li
// add lock for allocation
// 
// 14    09-07-22 15:19 Xiaoming.li
// correct PN select and allocIdent
// 
// 13    09-07-16 9:55 Xiaoming.li
// add compress import function
// 
// 12    09-07-08 11:11 Xiaoming.li
// 
// 11    09-05-25 16:55 Xiaoming.li
// catch each channel exception when list channels
// 
// 10    09-04-13 16:16 Xiaoming.li
// impl edgedevice, channel, port, allocation list method
// 
// 9     09-04-08 15:05 Xiaoming.li
// 
// 7     09-03-31 9:13 Xiaoming.li
// impl list function
// 
// 6     09-03-26 15:16 Hui.shao
// added NegotiateResourcesCmd
// 
// 5     09-03-26 11:18 Hui.shao
// 
// 4     09-03-19 17:12 Hui.shao
// init draft of evaluate, commit and withdraw,
// plus the states of allocation
// 
// 3     09-03-09 19:22 Hui.shao
// 
// 2     09-03-09 17:29 Hui.shao
// 
// 1     09-03-05 19:41 Hui.shao
// initially created
// ===========================================================================
#include <fstream>
#include "EdgeRMCmds.h"
#include "EdgeRMImpl.h"

#include "expatxx.h"

#ifdef ZQ_OS_MSWIN
#include "Bz2Stream.h"
#endif

#include "EdgeRMEnv.h"
#include <algorithm>

#define MOLOG		(_env._log)
#define MOEVENTLOG	(_env._eventlog)

namespace ZQTianShan {
namespace EdgeRM {

// -----------------------------
// class BaseCmd
// -----------------------------
BaseCmd::BaseCmd(EdgeRMEnv& env)
: ThreadRequest(env._thpool), _env(env)
{
}

BaseCmd::~BaseCmd()
{
}

//-----------------------------
// class ListAllocationIdsCmd
//-----------------------------
///
ListAllocationIdsCmd::ListAllocationIdsCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeDeviceEx_listAllocationIdsPtr& amdCB, const ::Ice::Identity& deviceIdent)
:BaseCmd(env), _amdCB(amdCB), _deviceIdent(deviceIdent)
{

}

int ListAllocationIdsCmd::run()
{
	std::string lastError;
	std::string deviceName = _deviceIdent.name;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListAllocationIdsCmd, "device[%s] list allocations"), deviceName.c_str());
	IdentCollection channelIdents; 
	try
	{	
		::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
		DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(deviceName);
		if(itorDC != _env._devicechannels.end())
			channelIdents = itorDC->second;
	}
	catch (...){
	}
	if (channelIdents.size() <= 0)
	{
		lastError = "can not find specified Edge Device(deviceId:" + deviceName + ")";
		::TianShanIce::ServerError ex("ListAllocationIdsCmd", 404, lastError);
		_amdCB->ice_exception(ex);
		return 1;
	}
	else
	{
		int allocCount = 0;
		for(IdentCollection::iterator it = channelIdents.begin(); it != channelIdents.end(); it++)
		{
			::Ice::Identity channelId = *it;
			try
			{
				::TianShanIce::EdgeResource::EdgeChannelExPrx channelExPrx = IdentityToObjEnv2(_env, EdgeChannelEx, channelId);
				if(channelExPrx != NULL)
				{
					TianShanIce::EdgeResource::AllocationIds allocIds = channelExPrx->getAllocationIds();
					allocCount += allocIds.size();
					TianShanIce::EdgeResource::AllocationIds::iterator iter = allocIds.begin();
					for(; iter != allocIds.end(); iter++)
					{	
						_result.push_back(*iter);
					}
				}
			}
			catch(const ::Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListAllocationIdsCmd, "run() list local allocation by ident(%s) catch ice exception[%s]"), it->name.c_str(), ex.what());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListAllocationIdsCmd, "run() list local allocation by ident(%s) catch unknow exception"), it->name.c_str());
			}		
		}
		
		_amdCB->ice_response(_result);

		return 1;
	}

	TianShanIce::ServerError ex("ListAllocationIdsCmd", 500, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}

// -----------------------------
// class ListChannelOfDeviceCmd
// -----------------------------
///
ListChannelOfDeviceCmd::ListChannelOfDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeDevice_listChannelsPtr& amdCB, const ::Ice::Identity& deviceIdent, ::Ice::Short portId, const ::TianShanIce::StrValues& expectedMetaData, bool enabledOnly)
: BaseCmd(env), _amdCB(amdCB),_deviceIdent(deviceIdent), _portId(portId), _expectedMetaData(expectedMetaData), _enabledOnly(enabledOnly)
{
}

int ListChannelOfDeviceCmd::run(void)
{
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListChannelOfDeviceCmd, "port[%d] list contents"), _portId);

	try	{
		::Ice::Long stamp0 = ZQTianShan::now();

		//get channel identity set first
//		IdentCollection channelIdent = _env._idxChannelOfDevice->find(_deviceIdent);

		IdentCollection channelIdent; 
		try
		{	
			::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
			DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(_deviceIdent.name);
			if(itorDC != _env._devicechannels.end())
				channelIdent = itorDC->second;
		}
		catch (...){
		}
		if (channelIdent.size() <= 0)
		{
			lastError = "can not find specified Edge Device(deviceId:" + _deviceIdent.name + ")";
			::TianShanIce::ServerError ex("ListChannelOfDeviceCmd", 404, lastError);
			_amdCB->ice_exception(ex);
			return 1;
		}
		else
		{
			//get channel information from each channel
			::TianShanIce::StrValues isEnable;
			isEnable.push_back(SYS_PROP(Enabled));
			for (IdentCollection::iterator it= channelIdent.begin(); it != channelIdent.end(); it++)
			{
				try
				{
					Ice::Identity ident = *it;
					::TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = IdentityToObjEnv2(_env, EdgeChannelEx, ident);
					std::string outDeviceName; 
					::Ice::Short outRFPort;
					::Ice::Short outChNum;
					channelPrx->getHiberarchy(outDeviceName, outRFPort, outChNum);
					if (outRFPort == _portId)
					{
						if (_enabledOnly)
						{
							::TianShanIce::StatedObjInfo enableInfo = channelPrx->getInfo(isEnable);
							if (enableInfo.props.find(SYS_PROP(Enabled)) != enableInfo.props.end())
							{
								if (enableInfo.props[SYS_PROP(Enabled)] == "1")
									_result.push_back(channelPrx->getInfo(_expectedMetaData));
							}
							else
							{
								envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ListChannelOfDeviceCmd, "run() list channel by ident(%s) fail to get Channel Info"), it->name.c_str());
							}
						}
						else
							_result.push_back(channelPrx->getInfo(_expectedMetaData));
					}
				}
				catch(const ::Ice::Exception& ex)
				{
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListChannelOfDeviceCmd, "run() list channel by ident(%s) catch ice exception[%s]"), it->name.c_str(), ex.what());
				}
				catch(...)
				{
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ListChannelOfDeviceCmd, "run() list channel by ident(%s) catch unknow exception"), it->name.c_str());
				}		
			}

			_amdCB->ice_response(_result);
			return 0;
		}
	}
	catch(const ::Ice::Exception& ex)
	{
		lastError = ex.ice_name();
	}
	catch(...)
	{
		lastError = "ListChannelOfDeviceCmd: caught unknown exception";
	}

	TianShanIce::ServerError ex("ListChannelOfDeviceCmd", 500, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}

// -----------------------------
// class PopulateChannelCmd
// -----------------------------
///
PopulateChannelCmd::PopulateChannelCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeDevice_populateChannelsPtr& amdCB, const ::Ice::Identity& deviceIdent, ::Ice::Short portId)
: BaseCmd(env), _amdCB(amdCB),_deviceIdent(deviceIdent), _portId(portId)
{
}

int PopulateChannelCmd::run(void)
{
	_expectedMetaData.push_back(SYS_PROP(FreqRF));
	_expectedMetaData.push_back(SYS_PROP(TSID));
	_expectedMetaData.push_back(SYS_PROP(IntervalPAT));
	_expectedMetaData.push_back(SYS_PROP(IntervalPMT));
	_expectedMetaData.push_back(SYS_PROP(StampLastUpdated));
	_expectedMetaData.push_back(SYS_PROP(NITPID));
	_expectedMetaData.push_back(SYS_PROP(StartUDPPort));
	_expectedMetaData.push_back(SYS_PROP(UdpPortStepByPn));
	_expectedMetaData.push_back(SYS_PROP(StartProgramNumber));
	_expectedMetaData.push_back(SYS_PROP(MaxSessions));
	_expectedMetaData.push_back(SYS_PROP(LowBandwidthUtilization));
	_expectedMetaData.push_back(SYS_PROP(HighBandwidthUtilization));
	_expectedMetaData.push_back(SYS_PROP(Enabled));
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PopulateChannelCmd, "port[%d] Populate contents"), _portId);

	try	{
		::TianShanIce::StrValues expectedMetaData;
		::Ice::Long stamp0 = ZQTianShan::now();
		::TianShanIce::EdgeResource::EdgeChannelInfos retEdgeChannelInfos;

		//get channel identity set first
//		IdentCollection channelIdent = _env._idxChannelOfDevice->find(_deviceIdent);

		IdentCollection channelIdent; 
		try
		{	
			::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
			DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(_deviceIdent.name);
			if(itorDC != _env._devicechannels.end())
				channelIdent = itorDC->second;
		}
		catch (...){
		}

		if (channelIdent.size() <= 0)
		{
			lastError = "can not find specified Edge Device(deviceId:" + _deviceIdent.name + ")";
			::TianShanIce::ServerError ex("PopulateChannelCmd", 404, lastError);
			_amdCB->ice_exception(ex);
			return 1;
		}
		else
		{
			::Ice::Short maxChNum = 0;
			std::string outDeviceName; 
			::Ice::Short outRFPort;
			::Ice::Short outChNum;

			//get channel information from each channel
			for (IdentCollection::iterator it= channelIdent.begin(); it < channelIdent.end(); it++)
			{
				::TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = IdentityToObjEnv2(_env, EdgeChannelEx, *it);
				//check channel RF port id
				channelPrx->getHiberarchy(outDeviceName, outRFPort, outChNum);
				if (outRFPort == _portId)
				{
					if (outChNum > maxChNum)
						maxChNum = outChNum;
					retEdgeChannelInfos.push_back(channelPrx->getInfo(_expectedMetaData));
				}
			}

			//populate new channel
			maxChNum++;
			::ZQTianShan::EdgeRM::EdgeChannelImpl::Ptr edgeChannelPtr = new ::ZQTianShan::EdgeRM::EdgeChannelImpl(_env);
			edgeChannelPtr->ident.category = DBFILENAME_EdgeChannel;
			edgeChannelPtr->ident.name = ::ZQTianShan::EdgeRM::EdgeRMImpl::formatHiberarchy(outDeviceName, _portId, maxChNum);
			_env._eEdgeChannel->add(edgeChannelPtr, edgeChannelPtr->ident);

			::Ice::Current c;
			_result.push_back(edgeChannelPtr->getInfo(_expectedMetaData, c));

			//response here
			_amdCB->ice_response(retEdgeChannelInfos);
			return 0;
		}
	}
	catch(const ::Ice::Exception& ex)
	{
		lastError = ex.ice_name();
	}
	catch(...)
	{
		lastError = "ListChannelOfDeviceCmd: caught unknown exception";
	}

	TianShanIce::ServerError ex("PopulateChannelCmd", 500, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}

// -----------------------------
// class QueryReplicaCmd
// -----------------------------
///
QueryReplicaCmd::QueryReplicaCmd(EdgeRMEnv& env, const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly)
:BaseCmd(env), _amdCB(amdCB), _category(category), _groupId(groupId), _localOnly(localOnly)
{
}
	
int QueryReplicaCmd::run(void)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(QueryReplicaCmd, "QueryReplicaCmd() query for replica category[%s] and groupId[%s]"), _category.c_str(), _groupId.c_str());

	char buf[2048];
//#pragma message ( __MSGLOC__ "TODO: impl QueryReplicaCmd")
	if (0 != _category.compare(DBFILENAME_EdgeDevice))
	{
		snprintf(buf, sizeof(buf)-2, "BaseEdgeRMCmd::QueryReplicaCmd() only category[" DBFILENAME_EdgeDevice "] is supported, query for category[%s] and groupId[%s]", _category.c_str(), _groupId.c_str());
		TianShanIce::ServerError ex(DBFILENAME_EdgeDevice, 607, buf);
		_amdCB->ice_exception(ex);
		return 1;
	}

	std::string lastError;

	try {
//#pragma message ( __MSGLOC__ "TODO: impl QueryReplicaCmd")
		::TianShanIce::Replicas result;
//		uint8 maxPrioritySeenInGroup = _replicaPriority;
//
//		// step 1. prepare replica info about self
//		::TianShanIce::Replica selfReplic;
//		selfReplic.category  = DBFILENAME_EdgeDevice;
//		selfReplic.groupId   =  _groupId;
//		selfReplic.replicaId = _replicaId;
//		selfReplic.priority  = _replicaPriority;
//		selfReplic.obj		 = proxy();
//		selfReplic.stampBorn = 0;  //TODO: should be the start time of this run
//#ifndef _INDEPENDENT_ADAPTER
//		selfReplic.stampBorn = _adapter->getActivateTime();
//#endif
//		selfReplic.stampChanged = ZQTianShan::now();
//
//		result.push_back(selfReplic);
//
//		// step 2. copy the _storeReplicas to the result
//		{
//			if (_replicaTimeout <=0)
//				_replicaTimeout = MIN_UPDATE_INTERVAL;
//
//			::Ice::Long expiration = ZQTianShan::now() - _replicaTimeout *2;
//			::Ice::Long stampToClear = ZQTianShan::now() - _replicaTimeout *3;
//			::TianShanIce::StrValues repToClear;
//
//			ZQ::common::MutexGuard g(_lockStoreReplicas);
//			for (ReplicaMap::iterator it = _storeReplicas.begin(); it != _storeReplicas.end(); it ++)
//			{
//				if (it->second.stampUpdated < stampToClear)
//				{
//					repToClear.push_back(it->first);
//					continue;
//				}
//
//				if (it->second.stampUpdated < expiration)
//					continue; // treated as no more available
//
//				result.push_back(it->second.replicaData);
//				maxPrioritySeenInGroup = MAX(maxPrioritySeenInGroup, it->second.replicaData.maxPrioritySeenInGroup);
//			}
//
//			for (::TianShanIce::StrValues::iterator itClear = repToClear.begin(); itClear < repToClear.end(); itClear ++)
//				_storeReplicas.erase(*itClear);
//		}
//
//		//step 3. update the maxPrioritySeenInGroup
//		for (size_t i=0; i< result.size(); i++)
//			result[i].maxPrioritySeenInGroup = maxPrioritySeenInGroup;
//
//		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "exportStoreReplicas() ContentStore::%s %d records exported"), _replicaGroupId.c_str(), result.size());
//		return result;
//
//		if (!_localOnly)
//		{
//			_amdCB->ice_response(result);
//			return 0;
//		}
//
//		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "QueryReplicaCmd() forwarding (%d) neighborhood replia info into the result"), result.size() -1);
//		::TianShanIce::Replicas temp = result;
//		result.clear();
//		if (temp.size()>0)
//			result.push_back(temp[0]);
		_amdCB->ice_response(result);
		return 0;
	}
	catch(...)
	{
//		char buf[2048];
//		snprintf(buf, sizeof(buf)-2, "ContentStore::QueryReplicaCmd() caught unknown exception"); 
//		lastError = buf;
	}

	TianShanIce::ServerError ex("EdgeRM", 604, lastError);
	_amdCB->ice_exception(ex);

	return 1;
}
	
// -----------------------------
// class UpdateReplicaCmd
// -----------------------------
UpdateReplicaCmd::UpdateReplicaCmd(EdgeRMEnv& env, const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const ::TianShanIce::Replicas& edgeReplicas)
:BaseCmd(env), _amdCB(amdCB), _edgeReplicas(edgeReplicas)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "UpdateReplicaCmd() created"));
}
	
int UpdateReplicaCmd::run(void)
{
	std::string lastError;
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "UpdateReplicaCmd() enter"));

	try 
	{
#pragma message ( __MSGLOC__ "TODO: impl QueryReplicaCmd")
		return 0;
	}
	catch(...)
	{
//		char buf[2048];
//		snprintf(buf, sizeof(buf)-2, "ContentStore::UpdateReplicaCmd() caught unknown exception"); 
//		lastError = buf;

//		MOLOG(Log::L_ERROR, CLOGFMT(ContentStore, "UpdateReplicaCmd() caught unknown exception"));
	}
	TianShanIce::ServerError ex("EdgeRM", 607, lastError);
	_amdCB->ice_exception(ex);
	return 1;
}

// -----------------------------
// class ListAllocationsCmd
// -----------------------------
///
ListAllocationsCmd::ListAllocationsCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeResouceManager_listAllocationsPtr& amdCB, const ::std::string& deviceName, ::Ice::Short portId, ::Ice::Short chNum, const ::TianShanIce::StrValues& expectedMetaData)
:BaseCmd(env), _amdCB(amdCB), _deviceName(deviceName), _portId(portId), _chNum(chNum), _expectedMetaData(expectedMetaData)
{
}
	
int ListAllocationsCmd::run(void)
{
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListChannelOfDeviceCmd, "device[%s]port[%d]chNum[%d] list Allocations"), _deviceName.c_str(), _portId, _chNum);

	try	{
		::Ice::Long stamp0 = ZQTianShan::now();
		
		//format channel name from input
		const std::string strChannelName = ::ZQTianShan::EdgeRM::EdgeRMImpl::formatHiberarchy(_deviceName, _portId, _chNum);

		//get channel proxy
		::TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = _env._openChannel(strChannelName);

		//get allocations of this channel
		::TianShanIce::EdgeResource::Allocations result = channelPrx->getAllocations();
		for (::TianShanIce::EdgeResource::Allocations::iterator iter = result.begin(); iter != result.end(); iter++)
		{
			//get allocation info
			try
			{
				AllocationInfo allocInfo = (*iter)->getInfo();
				_allocationInfos.push_back(allocInfo);
			}
			catch (::Ice::Exception &ex)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "LisAllocationCmd() Allocation getInfo catch exception(%s)"), ex.what());
			}
			catch (...)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "LisAllocationCmd() Allocation getInfo catch unknown exception"));
			}
			
		}
		_amdCB->ice_response(_allocationInfos);
		return 1;
	}
	catch(const ::Ice::Exception& ex)
	{
		lastError = ex.ice_name();
	}
	catch(...)
	{
		lastError = "ListAllocationsCmd: caught unknown exception";
	}


	TianShanIce::ServerError ex("EdgeRM", 607, lastError);
	_amdCB->ice_exception(ex);
	return 1;
}




// -----------------------------
// class exportAllocationsCmd
// -----------------------------
///
exportAllocationsCmd::exportAllocationsCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_exportAllocationsPtr amdCB, const ::std::string& deviceName, const ::std::string& since)
:BaseCmd(env), _amdCB(amdCB), _deviceName(deviceName), _since(since)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "exportAllocationsCmd() created"));
}

int exportAllocationsCmd::run(void)
{
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ListChannelOfDeviceCmd, "device[%s] since[%s] export Allocations"), _deviceName.c_str(), _since.c_str());

	try	{
		::Ice::Long stamp0 = ZQTianShan::now();
		Ice::ObjectPrx objPrx = NULL;
		TianShanIce::EdgeResource::EdgeDevicePrx edgeDevicePrx = _env.getEdgeRMPrx()->openDevice(_deviceName);
		#if  ICE_INT_VERSION / 100 >= 306
			objPrx = edgeDevicePrx->ice_collocationOptimized(false);
		#else
			objPrx = edgeDevicePrx->ice_collocationOptimization(false);
		#endif
		edgeDevicePrx = TianShanIce::EdgeResource::EdgeDevicePrx::uncheckedCast(objPrx);

		if(edgeDevicePrx == NULL)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(exportAllocationsCmd, "open device[%s] failed"), _deviceName.c_str());
			return false;
		}
		TianShanIce::EdgeResource::EdgePortInfos edgePorts = edgeDevicePrx->listEdgePorts();
        int64 bytesSize = 0;
		for(int i = 0; i < edgePorts.size(); i++)
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
			for(int j = 0; j < edgeChannels.size(); j++)
			{
				//get channel proxy
				TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = _env._openChannel(edgeChannels[j].ident.name);
				if(channelPrx == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(exportAllocationsCmd, "open channel[%s] failed"), edgeChannels[j].ident.name.c_str());
					return false;
				}

				//get allocations of this channel
				TianShanIce::EdgeResource::Allocations result = channelPrx->getAllocations();

				for (TianShanIce::EdgeResource::Allocations::iterator iter = result.begin(); iter != result.end(); iter++)
				{
					//get allocation info
					try
					{
						AllocationInfo allocInfo = (*iter)->getInfo();
						//get other info
						TianShanIce::EdgeResource::AlloctionRecord allocRec;
                        allocRec.stampCommitted = _atoi64(allocInfo.props[SYS_PROP(StampCommitted)].c_str());
						/*
						int64 iSince = ZQ::common::TimeUtil::ISO8601ToTime(_since.c_str());
						if(allocRec.stampCommitted < iSince)
							continue;
						*/
						//init allocRec;
						std::string metadataVal;
						allocRec.ident = allocInfo.ident;
						allocRec.state = allocInfo.state;
						allocRec.resources = (*iter)->getResources();
						allocRec.owner = (*iter)->getOwner();
						metadataVal  = allocInfo.props[SYS_PROP(OwnerKey)];
						allocRec.ownerKey = metadataVal;
						metadataVal = allocInfo.props[SYS_PROP(UdpPort)];
						allocRec.udpPort = atoi(metadataVal.c_str());
						allocRec.programNumber = atoi(allocInfo.props[SYS_PROP(ProgramNumber)].c_str());
						allocRec.maxJitter = atof(allocInfo.props[SYS_PROP(MaxJitter)].c_str());
						allocRec.sourceIP = allocInfo.props[SYS_PROP(SourceIP)];
						allocRec.bandwidth = _atoi64(allocInfo.props[SYS_PROP(Bandwidth)].c_str());
						allocRec.stampCreated = _atoi64(allocInfo.props[SYS_PROP(StampCreated)].c_str());
						allocRec.stampProvisioned = _atoi64(allocInfo.props[SYS_PROP(StampProvisioned)].c_str());

						allocRec.expiration = _atoi64(allocInfo.props[SYS_PROP(Expiration)].c_str());
						TianShanIce::EdgeResource::AllocationExPrx allocExPrx = TianShanIce::EdgeResource::AllocationExPrx::uncheckedCast((*iter));
						if(allocExPrx == NULL)
						{
							MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "exportAllocationsCmd() wrong allocation proxy"));
							return false;
						}
						allocRec.channelAssocs = allocExPrx->getChannelAssocs();
						allocRec.retrytimes = atoi(allocInfo.props[SYS_PROP(RetryTimes)].c_str());
						allocRec.sessionGroup = allocInfo.props[SYS_PROP(SessionGroup)];
						allocRec.onDemandSessionId = allocInfo.props[SYS_PROP(OnDemandSessionId)];
						allocRec.qamSessionId = allocInfo.props[SYS_PROP(QamSessionId)];
						allocRec.qamSessGroup = allocInfo.props[SYS_PROP(QamSessionGroup)];
						allocRec.link = channelPrx->getAllocLink(allocRec.ident, allocRec.programNumber);

						TianShanIce::BValues value;
						AllocationImpl::marshal(allocRec, value, _env._adapter->getCommunicator());
						_allocationValues.push_back(value);
						bytesSize += value.size();
            		}
					catch (::Ice::Exception &ex)
					{
						envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "exportAllocationsCmd() Allocation getInfo catch exception(%s)"), ex.what());
					}
					catch (...)
					{
						envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "exportAllocationsCmd() Allocation getInfo catch unknown exception"));
					}

				}     
			}
		}

		_amdCB->ice_response(_allocationValues);

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "exportAllocationsCmd() Allocation getInfo(allocationSize:%d) spend %d ms, size: %lld"), _allocationValues.size(), (int)(ZQ::common::TimeUtil::now() - stamp0), bytesSize);
		return 1;
	}
	catch(const ::Ice::Exception& ex)
	{
		lastError = ex.ice_name();
	}
	catch(...)
	{
		lastError = "exportAllocationsCmd: caught unknown exception";
	}

	TianShanIce::ServerError ex("EdgeRM", 607, lastError);
	_amdCB->ice_exception(ex);
	return 1;
}

//------------------------------
//class ExportDevicesCmd
//------------------------------
///
ExportDevicesCmd::ExportDevicesCmd(EdgeRMEnv& env, const TianShanIce::EdgeResource::AMD_EdgeRM_exportDevicesPtr amdCB, const std::string& deviceName, const std::string& xmlDefFile)
:BaseCmd(env), _amdCB(amdCB), _deviceName(deviceName),_xmlDefFile(xmlDefFile)
{
}

int ExportDevicesCmd::run()
{
	int64 stampStart = ZQ::common::TimeUtil::now();
	int deviceCount = 0, RFPortCount = 0, channelCount = 0;
	TianShanIce::EdgeResource::DevicesRecs devicesRecs;

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "enter run()"));

	std::string ermEndpoint;
	TianShanIce::EdgeResource::EdgeRMPrx edgeRMPrx = NULL;
	try
	{
		edgeRMPrx = _env.getEdgeRMPrx();
		if(NULL == edgeRMPrx)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "get EdgeRMProxy failed"));
			return false;
		}
		ermEndpoint = _env._adapter->getCommunicator()->proxyToString(edgeRMPrx);
		if(ermEndpoint.empty())
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "get ermPrx endpoint failed"));
			return false;
		}
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get ERM proxy info at endpoint[%s] caught exception(%s)"), ermEndpoint.c_str(),  ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get ERM proxy info at endpoint[%s] caught unknown exception(%d)"), ermEndpoint.c_str(), SYS::getLastErr());
		return false;
	}

	if(_deviceName.empty())
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "export all devices from endpoint[%s]"), ermEndpoint.c_str());
	else
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "export device[%s] from endpoint[%s]"), _deviceName.c_str(), ermEndpoint.c_str());

	TianShanIce::EdgeResource::EdgeDeviceInfos devicesInfo;
	TianShanIce::StrValues metadata;
	metadata.push_back(SYS_PROP(Name));
	metadata.push_back(SYS_PROP(Zone));
	metadata.push_back(SYS_PROP(Type));
	metadata.push_back(SYS_PROP(Vendor));		
	metadata.push_back(SYS_PROP(Model));
	metadata.push_back(SYS_PROP(Desc));
	metadata.push_back(SYS_PROP(Tftp));	
	metadata.push_back(SYS_PROP(AdminUrl));
	
	try
	{
		devicesInfo = edgeRMPrx->listDevices(metadata);
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to list devices info at endpoint[%s] caught exception(%s)"), ermEndpoint.c_str(),  ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to list devices info at endpoint[%s] caught unknown exception(%d)"), ermEndpoint.c_str(), SYS::getLastErr());
		return false;
	}
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "processed list %d devices took %dms"), devicesInfo.size(), (int)(ZQ::common::TimeUtil::now()-stampStart));

	std::string ofilename;
	if(_xmlDefFile.empty())
		 ofilename = DEFAULT_EXPORTFILE;
	else
		ofilename = _xmlDefFile;

	std::ofstream file;
	file.open(ofilename.c_str(), std::ios::out);
	if(!file)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "can not open file[%s]"), ofilename.c_str());
		return false;
	}

	std::ostream* out = (std::ostream*)&file;

	TianShanIce::EdgeResource::EdgeDeviceInfos::iterator deviceIter = devicesInfo.begin();
	(*out)<<"<EdgeDevices>\n";
	//list device 
	for(deviceIter;deviceIter != devicesInfo.end();deviceIter++)
	{
		std::string deviceName, deviceZone, deviceVendor, deviceModel, deviceAdminUrl, deviceDesc;
		TianShanIce::Properties deviceProps = deviceIter->props;

		deviceName = deviceProps[SYS_PROP(Name)];
		deviceZone = deviceProps[SYS_PROP(Zone)];
		deviceVendor = deviceProps[SYS_PROP(Vendor)];
		deviceModel = deviceProps[SYS_PROP(Model)];
		deviceAdminUrl = deviceProps[SYS_PROP(AdminUrl)];
		deviceDesc = deviceProps[SYS_PROP(Desc)];

		TianShanIce::EdgeResource::DeviceRec deviceRec;
		deviceRec.deviceName = deviceName;
		deviceRec.deviceZone = deviceZone;
		devicesRecs.push_back(deviceRec);

		if(!_deviceName.empty() && deviceName != _deviceName)
			continue;

		deviceCount++;
		std::string netIdSlash = pConfig.netId + "/";
		if(deviceName.find(netIdSlash) == std::string::npos)
			deviceName = pConfig.netId + "/" + deviceName;
		(*out)<<"	<EdgeDevice name=\""<<deviceName<<"\""<<" zone=\""<<deviceZone<<"\""
								  <<" vendor=\""<<deviceVendor<<"\""<<" model=\""<<deviceModel<<"\""
								  <<" endpoint=\""<<ermEndpoint<<"\""<<" adminUrl=\""<<deviceAdminUrl<<"\""
								  <<" desc=\""<<deviceDesc<<"\">\n";

		std::string ipAddrs = "";
		std::string macAddrs = "";

		try
		{
			//list RFPort through routeNames
			TianShanIce::EdgeResource::ObjectInfos routeInfos = edgeRMPrx->listRouteNames();
			TianShanIce::EdgeResource::ObjectInfos::iterator routeIter = routeInfos.begin();
			for(routeIter; routeIter != routeInfos.end();routeIter++)
			{
				TianShanIce::EdgeResource::EdgePortInfos portInfos = edgeRMPrx->findRFPortsByRouteName(routeIter->ident.name);
				//make sure that one port had only one route had linked
				if(portInfos.size() > 1)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "error link, routeName[%s] linked too many port[coute:%d], just one port allowed"), routeIter->ident.name.c_str(), portInfos.size());
					continue;;
				}
				TianShanIce::EdgeResource::EdgePortInfos::iterator portIter = portInfos.begin();

				//first loop for finding deviceIp and deviceMac from port
				std::string deviceNameInPort;
				for(portIter; portIter != portInfos.end(); portIter++)
				{
					std::string tempIp, tempMac;
					deviceNameInPort = portIter->resPhysicalChannel.resourceData["edgeDeviceName"].strs[0];
					std::string netIdSlash = pConfig.netId + "/";
					if(deviceNameInPort.find(netIdSlash) == std::string::npos && pConfig._backup.mode == "active")
						deviceNameInPort =  pConfig.netId + "/" + deviceNameInPort;
					if(deviceNameInPort != deviceName)
						continue;
					if(portIter->resPhysicalChannel.resourceData["edgeDeviceIP"].strs[0].empty())
					{
						envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ExportDevicesCmd, "get device ip address failed on device[%s] port[%d]"), deviceIter->ident.name.c_str(), portIter->Id);
						continue;
					}
					tempIp = portIter->resPhysicalChannel.resourceData["edgeDeviceIP"].strs[0];

					/*if(portIter->resPhysicalChannel.resourceData["edgeDeviceMac"].strs[0].empty())
					{
						envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ExportDevicesCmd, "get device mac address failed on device[%s] port[%d]"), deviceIter->ident.name.c_str(), portIter->Id);
						continue;
					}*/
					tempMac = portIter->resPhysicalChannel.resourceData["edgeDeviceMac"].strs[0];

					if(tempIp != ipAddrs || tempMac != macAddrs)
					{
						ipAddrs = tempIp;
						macAddrs = tempMac;
						(*out)<<"		<DeviceIP>\n";
						(*out)<<"			<Address ip=\""<<ipAddrs<<"\""<<" mac=\""<<macAddrs<<"\"/>\n";
						(*out)<<"		</DeviceIP>\n";
					}
				}
				if(deviceNameInPort != deviceName)
					continue;

				portIter = portInfos.begin();
				// second loop for finding port infos
				for(portIter;portIter != portInfos.end();portIter++)
				{
					RFPortCount++;
					int modulationFormat = portIter->resAtscModulationMode.resourceData["modulationFormat"].bin[0];
					int interleaverMode = portIter->resAtscModulationMode.resourceData["interleaveDepth"].bin[0];
					int interleaverLevel = portIter->resAtscModulationMode.resourceData["FEC"].bin[0];

					(*out)<<"		<EdgePort id=\""<<portIter->Id<<"\""<<" powerLevel=\""<<portIter->powerLevel<<"\">\n";
					(*out)<<"			<AtscModulationMode modulationFormat=\""<<modulationFormat<<"\""
						<<" interleaverMode=\""<<interleaverMode<<"\""
						<<" interleaverLevel=\""<<interleaverLevel<<"\" />\n";

					Ice::ObjectPrx objPrx = NULL;
					TianShanIce::EdgeResource::EdgeDevicePrx devicePrx = edgeRMPrx->openDevice(deviceIter->ident.name);					
					#if  ICE_INT_VERSION / 100 >= 306
						objPrx = devicePrx->ice_collocationOptimized(false);
					#else
						objPrx = devicePrx->ice_collocationOptimization(false);
					#endif
					devicePrx = TianShanIce::EdgeResource::EdgeDevicePrx::uncheckedCast(objPrx);
					if(NULL == devicePrx)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "get deviceProxy[%s] failed"), deviceIter->ident.name.c_str());
						return false;
					}

					//list channel 
					TianShanIce::StrValues expectedMetadata;
					expectedMetadata.push_back(SYS_PROP(Enabled));
					expectedMetadata.push_back(SYS_PROP(FreqRF));
					expectedMetadata.push_back(SYS_PROP(StartUDPPort));
					expectedMetadata.push_back(SYS_PROP(UdpPortStepByPn));
					expectedMetadata.push_back(SYS_PROP(StartProgramNumber));
					expectedMetadata.push_back(SYS_PROP(LowBandwidthUtilization));
					expectedMetadata.push_back(SYS_PROP(HighBandwidthUtilization));
					expectedMetadata.push_back(SYS_PROP(MaxSessions));
					expectedMetadata.push_back(SYS_PROP(IntervalPAT));
					expectedMetadata.push_back(SYS_PROP(IntervalPMT));
					expectedMetadata.push_back(SYS_PROP(symbolRate));
					expectedMetadata.push_back(SYS_PROP(TSID));
					expectedMetadata.push_back(SYS_PROP(NITPID));

					TianShanIce::EdgeResource::EdgeChannelInfos channelInfos = devicePrx->listChannels(portIter->Id, expectedMetadata, false);
					TianShanIce::EdgeResource::EdgeChannelInfos::iterator channelIter = channelInfos.begin();
					for(channelIter;channelIter != channelInfos.end();channelIter++)
					{
						channelCount++;
						TianShanIce::Properties props = channelIter->props;
						std::string chFullName = channelIter->ident.name;
						int pos = chFullName.find_last_of('.');
						if(pos == std::string::npos)
						{
							envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "wrong channel name formate[%s], can not find channel id with last of '.' "), chFullName.c_str());
							continue;
						}
						std::string chId = chFullName.substr(pos + 1);
						(*out)<<"			<EdgeChannel id=\""<<chId<<"\""
							     <<" freq=\""<<props[SYS_PROP(FreqRF)]<<"\""
								 <<" symbolRate=\""<<props[SYS_PROP(symbolRate)]<<"\""
								 <<" tsId=\""<<props[SYS_PROP(TSID)]<<"\""
								 <<" nitpid=\""<<props[SYS_PROP(NITPID)]<<"\">\n";

						(*out)<<"				<Provision enabled=\""<<props[SYS_PROP(Enabled)]<<"\""
								 <<" inbandMarker=\""<<INBANDMARKER<<"\""
								 <<" reportTrafficMismatch=\""<<REPORTTRAFICMISMATCH<<"\""
								 <<" jitterBuffer=\""<<JITTERBUFFER<<"\" />\n";

						(*out)<<"				<UDP startPort=\""<<props[SYS_PROP(StartUDPPort)]<<"\""
								 <<" step=\""<<props[SYS_PROP(UdpPortStepByPn)]<<"\""
								 <<" startPN=\""<<props[SYS_PROP(StartProgramNumber)]<<"\""
								 <<" maxSession=\""<<props[SYS_PROP(MaxSessions)]<<"\" />\n";

						(*out)<<"				<UtilizationThreshold low=\""<<atol(props[SYS_PROP(LowBandwidthUtilization)].c_str())/1000<<"\""
								 <<" high=\""<<atol(props[SYS_PROP(HighBandwidthUtilization)].c_str())/1000<<"\" />\n";

						(*out)<<"				<MPTS intervalPAT=\""<<props[SYS_PROP(IntervalPAT)]<<"\""
								 <<" intervalPMT=\""<<props[SYS_PROP(IntervalPMT)]<<"\" />\n";

						(*out)<<"			</EdgeChannel>\n";
					}
					(*out)<<"		</EdgePort>\n";
				}
			}
			(*out)<<"	</EdgeDevice>\n";
		}
		catch (::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get channels info at endpoint[%s] caught exception(%s)"), ermEndpoint.c_str(),  ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get channels info at endpoint[%s] caught unknown exception(%d)"), ermEndpoint.c_str(), SYS::getLastErr());
			return false;
		}
	}
	(*out)<<"</EdgeDevices>\n";
	out->flush();
	out->clear();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "processed export all [%d]devices, [%d]RFPorts, [%d]channels spend %lldms"), deviceCount, RFPortCount, channelCount, ZQ::common::TimeUtil::now()-stampStart);
	_amdCB->ice_response(devicesRecs.size());
	
	return true;
}

ExportDeviceXMLBodyCmd::ExportDeviceXMLBodyCmd(EdgeRMEnv& env, const TianShanIce::EdgeResource::AMD_EdgeRM_exportDeviceXMLPtr amdCB, const std::string& deviceName)
:BaseCmd(env), _amdCB(amdCB), _deviceName(deviceName)
{
}

int ExportDeviceXMLBodyCmd::run()
{
	int64 stampStart = ZQ::common::TimeUtil::now();
	int deviceCount = 0, RFPortCount = 0, channelCount = 0;

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "enter run()"));

	std::string ermEndpoint;
	TianShanIce::EdgeResource::EdgeRMPrx edgeRMPrx = NULL;
	try
	{
		edgeRMPrx = _env.getEdgeRMPrx();
		if(NULL == edgeRMPrx)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "get EdgeRMProxy failed"));
			return false;
		}
		ermEndpoint = _env._adapter->getCommunicator()->proxyToString(edgeRMPrx);
		if(ermEndpoint.empty())
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "get ermPrx endpoint failed"));
			return false;
		}
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get ERM proxy info at endpoint[%s] caught exception(%s)"), ermEndpoint.c_str(),  ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get ERM proxy info at endpoint[%s] caught unknown exception(%d)"), ermEndpoint.c_str(), SYS::getLastErr());
		return false;
	}

	if(_deviceName.empty())
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "export all devices from endpoint[%s]"), ermEndpoint.c_str());
	else
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "export device[%s] from endpoint[%s]"), _deviceName.c_str(), ermEndpoint.c_str());

	TianShanIce::EdgeResource::EdgeDeviceInfos devicesInfo;
	TianShanIce::StrValues metadata;
	metadata.push_back(SYS_PROP(Name));
	metadata.push_back(SYS_PROP(Zone));
	metadata.push_back(SYS_PROP(Type));
	metadata.push_back(SYS_PROP(Vendor));		
	metadata.push_back(SYS_PROP(Model));
	metadata.push_back(SYS_PROP(Desc));
	metadata.push_back(SYS_PROP(Tftp));	
	metadata.push_back(SYS_PROP(AdminUrl));

	try
	{
		devicesInfo = edgeRMPrx->listDevices(metadata);
	}
	catch (::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to list devices info at endpoint[%s] caught exception(%s)"), ermEndpoint.c_str(),  ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to list devices info at endpoint[%s] caught unknown exception(%d)"), ermEndpoint.c_str(), SYS::getLastErr());
		return false;
	}
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "processed list %d devices took %dms"), devicesInfo.size(), (int)(ZQ::common::TimeUtil::now()-stampStart));

	std::ostringstream out;

	TianShanIce::EdgeResource::EdgeDeviceInfos::iterator deviceIter = devicesInfo.begin();
	out<<"<EdgeDevices>\n";
	//list device 
	for(deviceIter;deviceIter != devicesInfo.end();deviceIter++)
	{
		std::string deviceName, deviceZone, deviceVendor, deviceModel, deviceAdminUrl, deviceDesc;
		TianShanIce::Properties deviceProps = deviceIter->props;

		deviceName = deviceProps[SYS_PROP(Name)];
		deviceZone = deviceProps[SYS_PROP(Zone)];
		deviceVendor = deviceProps[SYS_PROP(Vendor)];
		deviceModel = deviceProps[SYS_PROP(Model)];
		deviceAdminUrl = deviceProps[SYS_PROP(AdminUrl)];
		deviceDesc = deviceProps[SYS_PROP(Desc)];

		TianShanIce::EdgeResource::DeviceRec deviceRec;
		deviceRec.deviceName = deviceName;
		deviceRec.deviceZone = deviceZone;

		if(!_deviceName.empty() && deviceName != _deviceName)
			continue;

		deviceCount++;
		std::string netIdSlash = pConfig.netId + "/";
		if(deviceName.find(netIdSlash) == std::string::npos)
			deviceName = pConfig.netId + "/" + deviceName;
		out<<"	<EdgeDevice name=\""<<deviceName<<"\""<<" zone=\""<<deviceZone<<"\""
			<<" vendor=\""<<deviceVendor<<"\""<<" model=\""<<deviceModel<<"\""
			<<" endpoint=\""<<ermEndpoint<<"\""<<" adminUrl=\""<<deviceAdminUrl<<"\""
			<<" desc=\""<<deviceDesc<<"\">\n";

		std::string ipAddrs = "";
		std::string macAddrs = "";

		try
		{
			//list RFPort through routeNames
			TianShanIce::EdgeResource::ObjectInfos routeInfos = edgeRMPrx->listRouteNames();
			TianShanIce::EdgeResource::ObjectInfos::iterator routeIter = routeInfos.begin();
			for(routeIter; routeIter != routeInfos.end();routeIter++)
			{
				TianShanIce::EdgeResource::EdgePortInfos portInfos = edgeRMPrx->findRFPortsByRouteName(routeIter->ident.name);
				//make sure that one port had only one route had linked
				if(portInfos.size() > 1)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "error link, routeName[%s] linked too many port[coute:%d], just one port allowed"), routeIter->ident.name.c_str(), portInfos.size());
					continue;;
				}
				TianShanIce::EdgeResource::EdgePortInfos::iterator portIter = portInfos.begin();

				//first loop for finding deviceIp and deviceMac from port
				std::string deviceNameInPort;
				for(portIter; portIter != portInfos.end(); portIter++)
				{
					std::string tempIp, tempMac;
					deviceNameInPort = portIter->resPhysicalChannel.resourceData["edgeDeviceName"].strs[0];
					if(deviceNameInPort != deviceName)
						continue;
					if(portIter->resPhysicalChannel.resourceData["edgeDeviceIP"].strs[0].empty())
					{
						envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ExportDevicesCmd, "get device ip address failed on device[%s] port[%d]"), deviceIter->ident.name.c_str(), portIter->Id);
						continue;
					}
					tempIp = portIter->resPhysicalChannel.resourceData["edgeDeviceIP"].strs[0];
                    /*
					if(portIter->resPhysicalChannel.resourceData["edgeDeviceMac"].strs[0].empty())
					{
						envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ExportDevicesCmd, "get device mac address failed on device[%s] port[%d]"), deviceIter->ident.name.c_str(), portIter->Id);
						continue;
					}*/
					tempMac = portIter->resPhysicalChannel.resourceData["edgeDeviceMac"].strs[0];

					if(tempIp != ipAddrs || tempMac != macAddrs)
					{
						ipAddrs = tempIp;
						macAddrs = tempMac;
						out<<"		<DeviceIP>\n";
						out<<"			<Address ip=\""<<ipAddrs<<"\""<<" mac=\""<<macAddrs<<"\"/>\n";
						out<<"		</DeviceIP>\n";
					}
				}
				if(deviceNameInPort != deviceName)
					continue;

				portIter = portInfos.begin();
				// second loop for finding port infos
				for(portIter;portIter != portInfos.end();portIter++)
				{
					RFPortCount++;
					int modulationFormat = portIter->resAtscModulationMode.resourceData["modulationFormat"].bin[0];
					int interleaverMode = portIter->resAtscModulationMode.resourceData["interleaveDepth"].bin[0];
					int interleaverLevel = portIter->resAtscModulationMode.resourceData["FEC"].bin[0];

					out<<"		<EdgePort id=\""<<portIter->Id<<"\""<<" powerLevel=\""<<portIter->powerLevel<<"\">\n";
					out<<"			<AtscModulationMode modulationFormat=\""<<modulationFormat<<"\""
						<<" interleaverMode=\""<<interleaverMode<<"\""
						<<" interleaverLevel=\""<<interleaverLevel<<"\" />\n";

					Ice::ObjectPrx objPrx = NULL;
					TianShanIce::EdgeResource::EdgeDevicePrx devicePrx = edgeRMPrx->openDevice(deviceIter->ident.name);
					#if  ICE_INT_VERSION / 100 >= 306
						objPrx = devicePrx->ice_collocationOptimized(false);
					#else
						objPrx = devicePrx->ice_collocationOptimization(false);
					#endif
					devicePrx = TianShanIce::EdgeResource::EdgeDevicePrx::uncheckedCast(objPrx);
					if(NULL == devicePrx)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "get deviceProxy[%s] failed"), deviceIter->ident.name.c_str());
						return false;
					}

					//list channel 
					TianShanIce::StrValues expectedMetadata;
					expectedMetadata.push_back(SYS_PROP(Enabled));
					expectedMetadata.push_back(SYS_PROP(FreqRF));
					expectedMetadata.push_back(SYS_PROP(StartUDPPort));
					expectedMetadata.push_back(SYS_PROP(UdpPortStepByPn));
					expectedMetadata.push_back(SYS_PROP(StartProgramNumber));
					expectedMetadata.push_back(SYS_PROP(LowBandwidthUtilization));
					expectedMetadata.push_back(SYS_PROP(HighBandwidthUtilization));
					expectedMetadata.push_back(SYS_PROP(MaxSessions));
					expectedMetadata.push_back(SYS_PROP(IntervalPAT));
					expectedMetadata.push_back(SYS_PROP(IntervalPMT));
					expectedMetadata.push_back(SYS_PROP(symbolRate));
					expectedMetadata.push_back(SYS_PROP(TSID));
					expectedMetadata.push_back(SYS_PROP(NITPID));

					TianShanIce::EdgeResource::EdgeChannelInfos channelInfos = devicePrx->listChannels(portIter->Id, expectedMetadata, false);
					TianShanIce::EdgeResource::EdgeChannelInfos::iterator channelIter = channelInfos.begin();
					for(channelIter;channelIter != channelInfos.end();channelIter++)
					{
						channelCount++;
						TianShanIce::Properties props = channelIter->props;
						std::string chFullName = channelIter->ident.name;
						int pos = chFullName.find_last_of('.');
						if(pos == std::string::npos)
						{
							envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "wrong channel name formate[%s], can not find channel id with last of '.' "), chFullName.c_str());
							continue;
						}
						std::string chId = chFullName.substr(pos + 1);
						out<<"			<EdgeChannel id=\""<<chId<<"\""
							<<" freq=\""<<props[SYS_PROP(FreqRF)]<<"\""
							<<" symbolRate=\""<<props[SYS_PROP(symbolRate)]<<"\""
							<<" tsId=\""<<props[SYS_PROP(TSID)]<<"\""
							<<" nitpid=\""<<props[SYS_PROP(NITPID)]<<"\">\n";
							

						out<<"				<Provision enabled=\""<<props[SYS_PROP(Enabled)]<<"\""
							<<" inbandMarker=\""<<INBANDMARKER<<"\""
							<<" reportTrafficMismatch=\""<<REPORTTRAFICMISMATCH<<"\""
							<<" jitterBuffer=\""<<JITTERBUFFER<<"\" />\n";

						out<<"				<UDP startPort=\""<<props[SYS_PROP(StartUDPPort)]<<"\""
							<<" step=\""<<props[SYS_PROP(UdpPortStepByPn)]<<"\""
							<<" startPN=\""<<props[SYS_PROP(StartProgramNumber)]<<"\""
							<<" maxSession=\""<<props[SYS_PROP(MaxSessions)]<<"\" />\n";

						out<<"				<UtilizationThreshold low=\""<<atol(props[SYS_PROP(LowBandwidthUtilization)].c_str())/1000<<"\""
							<<" high=\""<<atol(props[SYS_PROP(HighBandwidthUtilization)].c_str())/1000<<"\" />\n";

						out<<"				<MPTS intervalPAT=\""<<props[SYS_PROP(IntervalPAT)]<<"\""
							<<" intervalPMT=\""<<props[SYS_PROP(IntervalPMT)]<<"\" />\n";

						out<<"			</EdgeChannel>\n";
					}
					out<<"		</EdgePort>\n";
				}
			}
			out<<"	</EdgeDevice>\n";
		}
		catch (::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get channels info at endpoint[%s] caught exception(%s)"), ermEndpoint.c_str(),  ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ExportDevicesCmd, "failed to get channels info at endpoint[%s] caught unknown exception(%d)"), ermEndpoint.c_str(), SYS::getLastErr());
			return false;
		}
	}
	out<<"</EdgeDevices>\n";
	_amdCB->ice_response(out.str());
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ExportDevicesCmd, "processed export all [%d]devices, [%d]RFPorts, [%d]channels spend %lldms"), deviceCount, RFPortCount, channelCount, ZQ::common::TimeUtil::now()-stampStart);
	out.clear();

	return true;
}

// -----------------------------
// class AddDeviceCmd
// -----------------------------
///
AddDeviceCmd::AddDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeResouceManager_addDevicePtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::std::string& vendor, const ::std::string& model, const ::std::string& adminUrl, const ::std::string& tftpUrl)
:BaseCmd(env), _amdCB(amdCB), _name(name), _deviceZone(deviceZone), _vendor(vendor), _model(model), _adminUrl(adminUrl), _tftpUrl(tftpUrl)
{
}

int AddDeviceCmd::run(void)
{
	std::string lastError;
	try 
	{
		::ZQTianShan::EdgeRM::EdgeDeviceImpl::Ptr edgeDevice = new ::ZQTianShan::EdgeRM::EdgeDeviceImpl(_env);
		edgeDevice->ident.name = _name;
		edgeDevice->ident.category = DBFILENAME_EdgeDevice;

		//get device info from XML
		edgeDevice->deviceZone	= _deviceZone;
		edgeDevice->type		= "QAM";
		edgeDevice->vendor		= _vendor;
		edgeDevice->model		= _model;
		edgeDevice->desc		= "";
		edgeDevice->tftpUrl		= _tftpUrl;
		edgeDevice->adminUrl	= _adminUrl;

		//add to evictor map
		_env._eEdgeDevice->add(edgeDevice, edgeDevice->ident);
		TianShanIce::EdgeResource::EdgeDevicePrx edgeDevicePrx = IdentityToObjEnv2(_env, EdgeDeviceEx, edgeDevice->ident);

		_env.createQamSessionGroup(_name, _tftpUrl);
		//response result
		_amdCB->ice_response(edgeDevicePrx);
		return 0;
	}
	catch(...)
	{
		lastError = "AddDeviceCmd: caught unknown exception";
	}

	TianShanIce::ServerError ex("EdgeRM", 607, lastError);
	_amdCB->ice_exception(ex);
	return 1;
}
	
// -----------------------------
// class ImportDeviceCmd
// -----------------------------
/* schema of the XML file:
<!-- Settings of a QAM device
     @attr name  - name the name of device, must be unique in the zone
     @attr vendor - the vendor name
     @attr model  - the product model of the device
     @attr tftp   - the tftp URL to flash settings if allowed
     @attr adminUrl - the URL to the administration interface of the device
     @attr desc   - the optional description of the device
-->
<EdgeDevice name="QAM1" zone="SEAC.SHANGHAI"
            vendor="CASA" model="2700"
            tftp=""
            adminUrl=""
            desc="qam on rack 11-3" >

			<!-- Settings of this device
			-->
	<DeviceIP>
		<!-- Settings of an IP address
		@attr value  - the IP address
		-->
		<Address value="192.168.0.1"/>
		<MacAddress value="01:02:03:04:05:06"/>
	</DeviceIP>
            
        <!-- Settings of an RF port
             @attr id  - the sequence port id in the device
             @attr powerLevel - the power level, in dBmV, of the RF port
        -->
	<EdgePort id="1" powerLevel="50">

            <!-- Settings of ATSC modulation
                 @attr modulationFormat  - the id maps to the modultion format, i.e. 8 = QAM64
                 @attr interleaverMode   - The interleaving depth of the interleaver, i.e. 1 = FEC-I-128-J-1
                 @attr interleaverLevel  - Interleaver level for FEC encoding
            -->
	    <AtscModulationMode modulationFormat="8" interleaverMode="1" interleaverLevel="1" />

            <!-- Settings of a RF channel or frequency
                 @attr id   - the unique sequence channel id in the port
                 @attr freq - the frequency, in KHz, of the channel
				 @attr symbolRate - the symbol rate of the channel
            -->
	    <EdgeChannel id="1" freq="106000" symbolRate="6875000">
                <!-- Settings about the input UDP inteface
                     @attr startPort - the start UDP port of this channel
                     @attr step - the step size of UDP port per program number, default 1
                     @attr startPN - the start program number
                     @attr maxSession - the maximal allowed session number in the channel
                -->
	        <UDP startPort="6000" step="1" startPN="21" maxSession="20" />

                <!-- Settings about the threshold of channel utilization
                     @attr low - the lowest threshold, in KMbps, to utilize the channel
                     @attr high - the highest threshold, in KMbps, to utilize the channel
                -->
	        <UtilizationThreshold low="200" high="6000" />

                <!-- Settings about MPEG TS encoding
                     @attr intervalPAT - the interval, in msec, to insert the PAT
                     @attr intervalPMT - the interval, in msec, to insert the PMT
                -->
	        <MPTS intervalPAT="40" intervalPMT="400" />
	    </EdgeChannel>
	</EdgePort>
</EdgeDevice>
*/

/// -----------------------------
/// class XmlImporter
/// -----------------------------
#ifdef ZQ_OS_MSWIN
#define ENABLE_COMPRESS
#endif

int portCount = 0;

bool IdentCompare( Ice::Identity ident1,  Ice::Identity ident2)
{
	return (ident1.name < ident2.name);
}

class XmlImporter : public ZQ::common::ExpatBase
{
public:
	XmlImporter(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDevicePtr& amdCB, ImportDeviceCmd* device, const ::std::string& name, const ::std::string& deviceZone, const ::TianShanIce::Properties& props, bool bXMLBody)
		:_env(env),_amdCB(amdCB),_device(device),_name(name),_deviceZone(deviceZone), _props(props),_bXMLBody(bXMLBody)
	{
		_bDeviceExist = false;
		_bChannelExist = false;
		_bPortExist = false;

	}
	XmlImporter(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDeviceXMLPtr& amdCB, ImportDeviceCmd* device, const ::std::string& name, const ::std::string& deviceZone, const ::TianShanIce::Properties& props, bool bXMLBody, std::string xmlBody)
		:_env(env),_amdCBXML(amdCB),_device(device),_name(name),_deviceZone(deviceZone), _props(props),_bXMLBody(bXMLBody), _xmlBody(xmlBody)
	{
		_bDeviceExist = false;
		_bChannelExist = false;
		_bPortExist = false;
	}

	virtual ~XmlImporter() {}

	virtual int parseFile(const char *szFilename, bool compressed=false)
	{
		if(_bXMLBody)
		{
			std::ofstream out;
			out.open(szFilename, std::ios::out);
			out<<_xmlBody;
			out.flush();
		}
		std::istream* pin = NULL;
		if (NULL == szFilename)
			return -1;

#ifndef ENABLE_COMPRESS
		compressed=false;
#endif // ENABLE_COMPRESS

		// Open the specified file
		std::ifstream file(szFilename);
		if(!file)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ImportDeviceCmd, "can not open file[%s]"), szFilename);
			return -1;
		}
		if (!file.is_open())
			return -2;

		pin = &file;

#ifdef ENABLE_COMPRESS
		std::ifstream zfile(szFilename, std::ios::in | std::ios::binary);
		if (compressed && !zfile.is_open())
			return -3;

		ZQ::common::Bz2InStream unbz2(zfile);
		if (compressed)
			pin = &unbz2;	
#endif // ENABLE_COMPRESS

		// Read to EOF
		char szBuffer[8192];
		int cBytes = 0;
		for (bool done = (*pin).eof(); !done;)
		{
			// Read data from the input file; store the bytes read
			(*pin).read(szBuffer, sizeof(szBuffer));
			done = (*pin).eof();
			ZQ::common::ExpatBase::parse(szBuffer, (*pin).gcount(), done);
			cBytes += (*pin).gcount();
		}
		return cBytes;
	}

protected:
	EdgeRMEnv& _env;

	::TianShanIce::EdgeResource::AMD_EdgeRM_importDevicePtr _amdCB;
	::TianShanIce::EdgeResource::AMD_EdgeRM_importDeviceXMLPtr _amdCBXML;

	ImportDeviceCmd* _device;

	::std::string _name, _deviceZone, _xmlDefFile;

	::std::string _currentEdgePort, _currentEdgeChannel;

	::ZQTianShan::EdgeRM::EdgeDeviceImpl::Ptr _edgeDevice;
	bool  _bDeviceExist;
	bool  _bChannelExist;
	bool  _bPortExist;
	bool  _bXMLBody;
	std::string _netId;
	std::string _xmlBody;
	TianShanIce::EdgeResource::EdgeChannelExPrx  _channelExPrx;
	TianShanIce::EdgeResource::EdgeDevices _edgeDevices;
	
	::TianShanIce::EdgeResource::EdgePort _edgePort;
	::ZQTianShan::EdgeRM::EdgeChannelImpl::Ptr _edgeChannelPtr;

	::TianShanIce::StrValues _deviceIpList;
	::TianShanIce::StrValues _deviceMacList;

	::TianShanIce::Properties _props;
    
    IdentCollection identChannels;
	IdentCollection _remoteIdentChannels;
	IdentCollection _remoteIdentDevices;

	// overridable callbacks, from ExpatBase
	virtual void OnStartElement(const XML_Char* name, const XML_Char** atts)
	{
		::std::string hiberarchyName = getHiberarchyName();
		::TianShanIce::Properties attrMap;

		for (int n = 0; atts[n]; n += 2)
			MAPSET(::TianShanIce::Properties, attrMap, atts[n], atts[ n + 1 ]);

		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice"))
		{
			_edgeDevice = new ::ZQTianShan::EdgeRM::EdgeDeviceImpl(_env);
			_name = attrMap["name"];
			std::string netIdSlash = pConfig.netId + "/";
			if(_name.find(netIdSlash) == std::string::npos && pConfig._backup.mode == "active")
				_name =  pConfig.netId + "/" + _name;
			_edgeDevice->ident.name = _name;
			_netId = _name.substr(0, _name.find_first_of('/'));
			_edgeDevice->ident.category = DBFILENAME_EdgeDevice;
			_remoteIdentDevices.push_back(_edgeDevice->ident);

			//get device info from XML
			_deviceZone = attrMap["zone"];
			_edgeDevice->deviceZone	= _deviceZone;
			_edgeDevice->type			= "QAM";
			_edgeDevice->vendor			= attrMap["vendor"];
			_edgeDevice->model			= attrMap["model"];
			_edgeDevice->desc			= attrMap["desc"];
			_edgeDevice->tftpUrl		= attrMap["endpoint"];
			_edgeDevice->adminUrl		= attrMap["adminUrl"];

			///1)new device, update _bDeviceExist=false;
			_bDeviceExist = false;
			
			///2) check DB, if device exist, update _bDeviceExist=true;
			if(_env._eEdgeDevice->hasObject(_edgeDevice->ident))
				_bDeviceExist = true;

			_currentEdgePort.clear();
			return;
		}

		///get device ip and mac info
		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/DeviceIP/Address"))
		{
			_deviceIpList.push_back(attrMap["ip"]);
			_deviceMacList.push_back(attrMap["mac"]);
			return;
		}
		///get Edge Port info
		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort"))
		{
			//get EdgePort attribute from XML
			_edgePort.Id			= atoi(attrMap["id"].c_str());
			_edgePort.powerLevel	= atoi(attrMap["powerLevel"].c_str());
			_currentEdgePort		= attrMap["id"];

			//clear old attribute data
			_currentEdgeChannel.clear();
			_edgePort.resAtscModulationMode.resourceData.clear();
			_edgePort.resPhysicalChannel.resourceData.clear();
			return;
		}

		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort/AtscModulationMode") && !_currentEdgePort.empty())
		{
			TianShanIce::EdgeResource::EdgeDeviceExPrx deviceExPrx = NULL;
			std::string deivceName = "";
			_bPortExist = false;
			try
			{ 
				if(_bDeviceExist)
				{
					deivceName = _edgeDevice->ident.name;
					Ice::ObjectPrx objPrx = NULL;
					TianShanIce::EdgeResource::EdgeDevicePrx devicePrx = _env.getEdgeRMPrx()->openDevice(_edgeDevice->ident.name);
					#if  ICE_INT_VERSION / 100 >= 306
						objPrx = devicePrx->ice_collocationOptimized(false);
					#else
						objPrx = devicePrx->ice_collocationOptimization(false);
					#endif
					deviceExPrx = TianShanIce::EdgeResource::EdgeDeviceExPrx::uncheckedCast(objPrx);
					_edgePort =  deviceExPrx->getEdgePort(_edgePort.Id);
					_bPortExist = true;
				}
			}
			catch(TianShanIce::InvalidParameter)
			{
			}
			catch (::Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(XmlImporter, "failed to get device proxy[%s] caught exception(%s)"), deivceName.c_str(),  ex.ice_name().c_str());
			}
			catch (...)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(XmlImporter, "failed to get device proxy[%s] caught unknown exception(%d)"), deivceName.c_str(), SYS::getLastErr());
			}

			///if  new port, add it to device: 
			///if edgeDevice not exist, add port to _edgeDevice object, else add port to exist device
			///if port exist, ingore
			if(!_bPortExist)
			{
				::Ice::Int modulationFormat	= atoi(attrMap["modulationFormat"].c_str());
				::Ice::Int interleaverMode	= atoi(attrMap["interleaverMode"].c_str());
				::Ice::Int interleaverLevel	= atoi(attrMap["interleaverLevel"].c_str());

				TianShanIce::Variant value;
				_edgePort.resAtscModulationMode.status	= ::TianShanIce::SRM::rsAssigned;
				_edgePort.resAtscModulationMode.attr		= ::TianShanIce::SRM::raMandatoryNegotiable;

				//add resAtscModulationMode resource map
				//add in modulationFormat
				value.type = ::TianShanIce::vtBin;
				value.bRange = false;
				value.bin.clear();
				value.bin.push_back(modulationFormat);
				_edgePort.resAtscModulationMode.resourceData["modulationFormat"] = value;

				//add in modulationMode
				value.type = ::TianShanIce::vtBin;
				value.bRange = false;
				value.bin.clear();
				value.bin.push_back(interleaverMode);
				_edgePort.resAtscModulationMode.resourceData["interleaveDepth"] = value;

				//add in FEC
				value.type = ::TianShanIce::vtBin;
				value.bRange = false;
				value.bin.clear();
				value.bin.push_back(interleaverLevel);
				_edgePort.resAtscModulationMode.resourceData["FEC"] = value;

				//add resPhysicalChannel resource map
				//add in edgeDeviceName
				value.type = ::TianShanIce::vtStrings;
				value.bRange = false;
				value.strs.clear();
				value.strs.push_back(_name);
				_edgePort.resPhysicalChannel.resourceData["edgeDeviceName"] = value;

				//add in edgeDeviceIP
				value.type = ::TianShanIce::vtStrings;
				value.bRange = false;
				value.strs.clear();
				value.strs = _deviceIpList;
				_edgePort.resPhysicalChannel.resourceData["edgeDeviceIP"] = value;

				//add in edgeDeviceMac
				value.type = ::TianShanIce::vtStrings;
				value.bRange = false;
				value.strs.clear();
				value.strs = _deviceMacList;
				_edgePort.resPhysicalChannel.resourceData["edgeDeviceMac"] = value;

				//add in edgeDeviceZone
				value.type = ::TianShanIce::vtStrings;
				value.bRange = false;
				value.strs.clear();
				value.strs.push_back(_deviceZone);
				_edgePort.resPhysicalChannel.resourceData["edgeDeviceZone"] = value;

				//add EdgePort to device
				::Ice::Current c;
				if(!_bDeviceExist)
					_edgeDevice->addEdgePort(_edgePort, c);
				else if(_bDeviceExist)
					deviceExPrx->addEdgePort(_edgePort);
			}
			return;
		}

		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort/EdgeChannel") && !_currentEdgePort.empty())
		{
			_edgeChannelPtr = NULL;//destroy last EdgeChannel pointer
			_edgeChannelPtr = new ::ZQTianShan::EdgeRM::EdgeChannelImpl(_env);

			_edgeChannelPtr->identDevice = _edgeDevice->ident;
			std::string deivceName = _edgeDevice->ident.name;
			std::string strId = attrMap["id"];
			int pos = strId.find_last_of('.');
			if(pos != std::string::npos)
			{
				strId = strId.substr(pos+1);
			}
			int chId = atoi(strId.c_str());
			_edgeChannelPtr->ident.name = ::ZQTianShan::EdgeRM::EdgeRMImpl::formatHiberarchy(_name, _edgePort.Id, chId);
			_edgeChannelPtr->ident.category = DBFILENAME_EdgeChannel;
			_remoteIdentChannels.push_back(_edgeChannelPtr->ident);
			_edgeChannelPtr->ePort = _edgePort;
			_edgeChannelPtr->enabled = true;

			//add resAtscModulationMode resource map
			//add in symbolRate
			::Ice::Int symbolRate = atoi(attrMap["symbolRate"].c_str());
			::TianShanIce::Variant value;
			value.type = ::TianShanIce::vtInts;
			value.bRange = false;
			value.ints.clear();
			value.ints.push_back(symbolRate);
			_edgeChannelPtr->ePort.resAtscModulationMode.resourceData["symbolRate"] = value;

			//std::string strId = attrMap["id"];
			std::string netIdSlash = pConfig.netId + "/";
			if(strId.find(netIdSlash) == std::string::npos)
				strId = pConfig.netId + "/" + strId;
			_currentEdgeChannel = strId;

			_edgeChannelPtr->TSID = atoi(attrMap["tsId"].c_str());
			_edgeChannelPtr->freqRF	= atol(attrMap["freq"].c_str());
			//init with reserved value
			_edgeChannelPtr->NITPID = atoi(attrMap["nitpid"].c_str());
			_edgeChannelPtr->deviceState = ::TianShanIce::stInService;
			if(_bDeviceExist)
			{
				_channelExPrx = NULL;
				Ice::Identity identCh;
				identCh.name = EdgeRMImpl::formatHiberarchy(_edgeDevice->ident.name, _edgePort.Id, chId);
				
				_bChannelExist = false;
				if(_env._eEdgeChannel->hasObject(_edgeChannelPtr->ident) != NULL)
				{
					_bChannelExist = true;
					_channelExPrx = _env._openChannel(identCh.name);
					envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(XmlImporter, "channel[%s] has been existed in record"), _edgeChannelPtr->ident.name.c_str());
				}
			}

			identChannels.push_back(_edgeChannelPtr->ident);
			return;
		}
		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort/EdgeChannel/Provision") && !_currentEdgePort.empty())
		{
			_edgeChannelPtr->provPort.enabled = atoi(attrMap["enabled"].c_str());
			_edgeChannelPtr->provPort.inbandMarker = attrMap["inbandMarker"];
			_edgeChannelPtr->provPort.reportTrafficMismatch = atoi(attrMap["reportTrafficMismatch"].c_str());
			_edgeChannelPtr->provPort.jitterBuffer = atoi(attrMap["jitterBuffer"].c_str());
			return;
		}
		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort/EdgeChannel/UDP") && !_currentEdgeChannel.empty())
		{
			_edgeChannelPtr->startUDPPort		= atoi(attrMap["startPort"].c_str());
			_edgeChannelPtr->udpPortStepByPn	= atoi(attrMap["step"].c_str());
			_edgeChannelPtr->startProgramNumber	= atoi(attrMap["startPN"].c_str());
			if(atoi(attrMap["maxSession"].c_str()) > MAX_SESSION_CHANNELS)
				_edgeChannelPtr->maxSessions = MAX_SESSION_CHANNELS;
			else
				_edgeChannelPtr->maxSessions = atoi(attrMap["maxSession"].c_str());
			return;
		}

		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort/EdgeChannel/UtilizationThreshold") && !_currentEdgeChannel.empty())
		{
			_edgeChannelPtr->lowBandwidthUtilization = atoi(attrMap["low"].c_str()) * 1000;
			_edgeChannelPtr->highBandwidthUtilization = (Ice::Long)(atoi(attrMap["high"].c_str())) * 1000;
			return;
		}

		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort/EdgeChannel/MPTS") && !_currentEdgeChannel.empty())
		{
			_edgeChannelPtr->intervalPAT = atoi(attrMap["intervalPAT"].c_str());
			_edgeChannelPtr->intervalPMT = atoi(attrMap["intervalPMT"].c_str());
			return;
		}
	}

	virtual void OnEndElement(const XML_Char*)
	{
		::std::string hiberarchyName = getHiberarchyName();

		if(0 == hiberarchyName.compare("/EdgeDevices"))
		{
/*			try
			{
			// compare local devices(listdevices()) and remote devices(_remoteDevices)
				//if exist in local but not in remote than remove it
				TianShanIce::StrValues metadata;
				metadata.push_back(SYS_PROP(Tftp));
				TianShanIce::EdgeResource::EdgeDeviceInfos localDeivcesInfo = _env.getEdgeRMPrx()->listDevices(metadata);
				TianShanIce::EdgeResource::EdgeDeviceInfos::iterator deivceIter = localDeivcesInfo.begin();
				IdentCollection localDeviceIdents;
				
				for(deivceIter; deivceIter != localDeivcesInfo.end(); deivceIter++)
				{
					int pos = deivceIter->ident.name.find('/');
					std::string localNetId = deivceIter->ident.name.substr(0, pos);
					if(_netId == localNetId)
						localDeviceIdents.push_back(deivceIter->ident);
				}
				std::sort(localDeviceIdents.begin(), localDeviceIdents.end(), IdentCompare);
				std::sort(_remoteIdentDevices.begin(), _remoteIdentDevices.end(), IdentCompare);

				IdentCollection localDiffDevices;
				std::set_difference(localDeviceIdents.begin(), localDeviceIdents.end(), _remoteIdentDevices.begin(), _remoteIdentDevices.end(), back_inserter(localDiffDevices));
				IdentCollection::iterator identIter = localDiffDevices.begin();
				for(identIter; identIter != localDiffDevices.end(); identIter++)
				{
					_env._eEdgeDevice->remove(*identIter);
				}
			}
			catch (const ::Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ImportDeviceCmd, "XmlImporter::OnEndElement() EdgeDevice[%s] caught exception[%s]"), _edgeDevice->ident.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ImportDeviceCmd, "XmlImporter::OnEndElement() EdgeDevice[%s] caught unknown exception"), _edgeDevice->ident.name.c_str());
			}
			*/
			if(_bXMLBody)
				_amdCBXML->ice_response(_edgeDevices);
			else
				_amdCB->ice_response(_edgeDevices);
		}
		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice"))
		{
			try
			{
				if(!_bDeviceExist)
				{
					_bDeviceExist = false;
					//add to evictor map
					_env._eEdgeDevice->add(_edgeDevice, _edgeDevice->ident);
					TianShanIce::EdgeResource::EdgeDevicePrx devicePrx = IdentityToObjEnv2(_env, EdgeDeviceEx, _edgeDevice->ident);
					_edgeDevices.push_back(devicePrx);
					_env.addChannelsToDevice(_edgeDevice->ident.name, identChannels);
					_env.createQamSessionGroup(_edgeDevice->ident.name, _edgeDevice->tftpUrl);

/*
#ifndef TESTFORPRESS
#define TESTFORPRESS
#endif
*/
#ifdef	TESTFORPRESS
					char routeName[20] = "";
					TianShanIce::EdgeResource::EdgePortInfos portInfos = devicePrx->listEdgePorts();
					TianShanIce::EdgeResource::EdgePortInfos::iterator portIter = portInfos.begin();
					for(;portIter != portInfos.end(); portIter++)
					{
						snprintf(routeName, sizeof(routeName) - 2, "QAMTest%03d", portCount);
						TianShanIce::Variant var_freqs;
						var_freqs.type = ::TianShanIce::vtLongs;
						var_freqs.bRange = false;
						std::string freqs = "";
						for(int i = 0; i < freqs.size(); i++)
						{
							if(!isdigit(freqs[i]) && freqs[i] != ';' && freqs[i] != '~')
							{
								return;
							}
							else if(freqs[i] == '~')
							{
								var_freqs.bRange = true;
							}
						}
						if(var_freqs.bRange)
						{
							TianShanIce::StrValues strVec = ZQ::common::stringHelper::split(freqs, '~');
							if(strVec.size() < 2)
							{
								return;
							}
							else
							{
								if(atol(strVec[0].c_str()) >= atol(strVec[1].c_str()))
								{
									return;
								}
								var_freqs.lints.push_back(atol(strVec[0].c_str()));
								var_freqs.lints.push_back(atol(strVec[1].c_str()));
							}
						}
						else
						{
							TianShanIce::StrValues strVec = ZQ::common::stringHelper::split(freqs, ';');
							for(TianShanIce::StrValues::iterator it = strVec.begin(); it != strVec.end(); it++)
							{
								if(it->size() <= 0)
									continue;
								if(atol(it->c_str()) > 0)
									var_freqs.lints.push_back(atol(it->c_str()));
							}
						}
						devicePrx->linkRoutes(portIter->Id, routeName, var_freqs);
						portCount++;
					}

#endif
					_edgeDevice = NULL;//release local reference
					identChannels.clear();
				}
				else
				{
					Ice::ObjectPrx objPrx = NULL;
					TianShanIce::EdgeResource::EdgeDevicePrx devicePrx = _env.getEdgeRMPrx()->openDevice(_edgeDevice->ident.name);
					#if  ICE_INT_VERSION / 100 >= 306
						objPrx = devicePrx->ice_collocationOptimized(false);
					#else
						objPrx = devicePrx->ice_collocationOptimization(false);
					#endif
					TianShanIce::EdgeResource::EdgeDeviceExPrx deviceExPrx = TianShanIce::EdgeResource::EdgeDeviceExPrx::uncheckedCast(objPrx);
					if(deviceExPrx == NULL)
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ImportDeviceCmd, "can not open existed deivce[%s] of endpoint[%s]"), _edgeDevice->ident.name.c_str(), _edgeDevice->tftpUrl.c_str());
						return;
					}
					_edgeDevices.push_back(devicePrx);
					//get local channels
					TianShanIce::EdgeResource::EdgePortInfos localPorts = deviceExPrx->listEdgePorts();
					TianShanIce::EdgeResource::EdgePortInfos::iterator portIter = localPorts.begin();
					IdentCollection localChannels;

					for(portIter; portIter != localPorts.end(); portIter++)
					{
						TianShanIce::StrValues metaData;
						TianShanIce::EdgeResource::EdgeChannelInfos chInfos = deviceExPrx->listChannels(portIter->Id, metaData, false);
						TianShanIce::EdgeResource::EdgeChannelInfos::iterator chIter = chInfos.begin();
						for(chIter; chIter != chInfos.end(); chIter++)
						{
							localChannels.push_back(chIter->ident);
						}
					}

					//remove local channels that has been not exist in remote ERM
					IdentCollection localDiffChannels;
					std::sort(_remoteIdentChannels.begin(), _remoteIdentChannels.end(), IdentCompare);
					std::sort(localChannels.begin(), localChannels.end(), IdentCompare);
					std::set_difference(localChannels.begin(), localChannels.end(), _remoteIdentChannels.begin(), _remoteIdentChannels.end(), back_inserter(localDiffChannels));

					IdentCollection::iterator iter = localDiffChannels.begin();
					for(iter; iter != localDiffChannels.end(); iter++)
					{
							_env._eEdgeChannel->remove(*iter);
					}
					if(!identChannels.empty())
					{
						_env.addChannelsToDevice(_edgeDevice->ident.name, identChannels);
					}
					TianShanIce::StrValues expectedMetadata;
					expectedMetadata.push_back(SYS_PROP(Name));
					expectedMetadata.push_back(SYS_PROP(Zone));
					expectedMetadata.push_back(SYS_PROP(Type));
					expectedMetadata.push_back(SYS_PROP(Vendor));
					expectedMetadata.push_back(SYS_PROP(Model));
					expectedMetadata.push_back(SYS_PROP(Desc));
					expectedMetadata.push_back(SYS_PROP(Tftp));
					expectedMetadata.push_back(SYS_PROP(AdminUrl));

					TianShanIce::ObjectInfo deviceInfo = deviceExPrx->getInfo(expectedMetadata);
					TianShanIce::Properties attrs;
					Ice::Current c;
					std::string deviceName = _edgeDevice->getName(c);
					if(deviceInfo.props[SYS_PROP(Name)] != deviceName)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(Name), deviceName);
					if(deviceInfo.props[SYS_PROP(Zone)] != _edgeDevice->deviceZone)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(Zone), _edgeDevice->deviceZone);
					if(deviceInfo.props[SYS_PROP(Type)] != _edgeDevice->type)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(Type), _edgeDevice->type);
					if(deviceInfo.props[SYS_PROP(Vendor)] != _edgeDevice->vendor)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(Vendor), _edgeDevice->vendor);
					if(deviceInfo.props[SYS_PROP(Model)] != _edgeDevice->model)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(Model), _edgeDevice->model);
					if(deviceInfo.props[SYS_PROP(Desc)] != _edgeDevice->desc)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(Desc), _edgeDevice->desc);
					if(deviceInfo.props[SYS_PROP(Tftp)] != _edgeDevice->tftpUrl)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(Tftp), _edgeDevice->tftpUrl);
					if(deviceInfo.props[SYS_PROP(AdminUrl)] != _edgeDevice->adminUrl)
						MAPSET(TianShanIce::Properties, attrs, SYS_PROP(AdminUrl), _edgeDevice->adminUrl);

					deviceExPrx->updateAttributes(attrs);
				}
				_bDeviceExist = false;
			}
			catch (const ::Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ImportDeviceCmd, "XmlImporter::OnEndElement() EdgeDevice[%s] caught exception[%s]"), _edgeDevice->ident.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ImportDeviceCmd, "XmlImporter::OnEndElement() EdgeDevice[%s] caught unknown exception"), _edgeDevice->ident.name.c_str());
			}
			identChannels.clear();
			_deviceIpList.clear();
			_deviceMacList.clear();
			return;
		}

		if (0 == hiberarchyName.compare("/EdgeDevices/EdgeDevice/EdgePort/EdgeChannel") && !_currentEdgeChannel.empty())
		{
			try
			{
				_edgeChannelPtr->stampLastUpdated = ::ZQTianShan::now();
				//channel has been existed
				if(_bChannelExist)
				{
					_bChannelExist = false;
					//channel has been existed than update channel
					TianShanIce::StrValues metaData;
					metaData.push_back(SYS_PROP(FreqRF));
					metaData.push_back(SYS_PROP(TSID));
					metaData.push_back(SYS_PROP(NITPID));
					metaData.push_back(SYS_PROP(StartUDPPort));
					metaData.push_back(SYS_PROP(UdpPortStepByPn));
					metaData.push_back(SYS_PROP(StartProgramNumber));
					metaData.push_back(SYS_PROP(LowBandwidthUtilization));
					metaData.push_back(SYS_PROP(HighBandwidthUtilization));
					metaData.push_back(SYS_PROP(MaxSessions));
					metaData.push_back(SYS_PROP(IntervalPAT));
					metaData.push_back(SYS_PROP(IntervalPMT));
					metaData.push_back(SYS_PROP(symbolRate));

					TianShanIce::StatedObjInfo chInfo = _channelExPrx->getInfo(metaData);
					TianShanIce::EdgeResource::EdgePort  edgePort = _channelExPrx->getEdgePort();
					::TianShanIce::Properties chAttrs;

					char temp[65] = "";
					memset(temp, 0, sizeof(temp));
					sprintf(temp, "%lld", _edgeChannelPtr->freqRF);
					if(chInfo.props[SYS_PROP(FreqRF)] != temp)
						MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(FreqRF), temp);

					/*
					if(chInfo.props[SYS_PROP(TSID)] != _edgeChannelPtr->ident.name)
						MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(TSID), _edgeChannelPtr->ident.name);
						*/

					TianShanIce::ValueMap::iterator itor;
					itor = _edgePort.resAtscModulationMode.resourceData.find("modulationFormat");
					if(itor != _edgePort.resAtscModulationMode.resourceData.end())
					{
						memset(temp, 0, sizeof(temp));
						sprintf(temp, "%d", itor->second.bin[0]);
						if(chInfo.props[SYS_PROP(ModulationFMT)] != temp)
							MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(ModulationFMT), temp);
					}

					itor = _edgePort.resAtscModulationMode.resourceData.find("interleaveDepth");
					if(itor != _edgePort.resAtscModulationMode.resourceData.end())
					{
						memset(temp, 0, sizeof(temp));
						sprintf(temp, "%d", itor->second.bin[0]);
						if(chInfo.props[SYS_PROP(InterleaverDepth)] != temp)
							MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(InterleaverDepth), temp);
					}

					memset(temp, 0, sizeof(temp));
					sprintf(temp, "%lld", _edgeChannelPtr->highBandwidthUtilization);
					if(chInfo.props[SYS_PROP(HighBandwidthUtilization)] != temp)
						MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(HighBandwidthUtilization), temp);

					memset(temp, 0, sizeof(temp));
					sprintf(temp, "%d", _edgeChannelPtr->startUDPPort);
					if(chInfo.props[SYS_PROP(StartUDPPort)] != temp)
						MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(StartUDPPort), temp);

					memset(temp, 0, sizeof(temp));
					sprintf(temp, "%d", _edgeChannelPtr->startProgramNumber);
					if(chInfo.props[SYS_PROP(StartProgramNumber)] != temp)
						MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(StartProgramNumber), temp);

					memset(temp, 0, sizeof(temp));
					sprintf(temp, "%d", _edgeChannelPtr->maxSessions);
					if(chInfo.props[SYS_PROP(MaxSessions)] != temp)
						MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(MaxSessions), temp);

					itor = _edgePort.resAtscModulationMode.resourceData.find("edgeDeviceZone");
					if(itor != _edgePort.resAtscModulationMode.resourceData.end())
					{
						if(chInfo.props[SYS_PROP(DeviceZone)] != itor->second.strs[0])
							MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(DeviceZone), itor->second.strs[0]);
					}

					/*
					itor = _edgePort.resPhysicalChannel.resourceData.find("edgeDeviceIP");
					if(itor != _edgePort.resPhysicalChannel.resourceData.end())
					{
						if(chInfo.props[SYS_PROP(DeviceIP)] != _edgePort.resPhysicalChannel.resourceData["edgeDeviceIP"].strs[0])
							MAPSET(TianShanIce::Properties, chAttrs, SYS_PROP(DeviceZone), itor->second.strs[0]);
					}
					*/
					
					if(chInfo.state != TianShanIce::stInService)
					{
						char state[32]="";
						itoa(TianShanIce::stInService, state, 10);
						chAttrs[SYS_PROP(DeviceState)] = state;
					}
					
					if(!chAttrs.empty())
					{
						_channelExPrx->updateAttributes(chAttrs);

						::TianShanIce::Properties::iterator iterAttrs = chAttrs.begin();
						for(iterAttrs;iterAttrs != chAttrs.end();iterAttrs++)
							glog(::ZQ::common::Log::L_DEBUG,CLOGFMT(UpdateChannel,"channel attributes: [%s,%s] updated"),iterAttrs->first.c_str(),iterAttrs->second.c_str());
					}
					glog(ZQ::common::Log::L_DEBUG,CLOGFMT(UpdateChannel,"channel attributes :%d attributes has been updated"),chAttrs.size());
					return;
				}
				else
				{
					_env._eEdgeChannel->add(_edgeChannelPtr, _edgeChannelPtr->ident);
				}
				//release local reference
				_edgeChannelPtr = NULL;
				_channelExPrx = NULL;
			}
			catch (const ::Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ImportDeviceCmd, "XmlImporter::OnEndElement() EdgeChannel[%s] caught exception[%s]"), _edgeChannelPtr->ident.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(ImportDeviceCmd, "XmlImporter::OnEndElement() EdgeChannel[%s] caught unknown exception"), _edgeChannelPtr->ident.name.c_str());
			}
			return;
		}
	}

	virtual void OnCharData(const XML_Char*, int len) {}
	virtual void OnLogicalClose() {}
};


ImportDeviceCmd::ImportDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDevicePtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::std::string& xmlDefFile, bool bCompress,const ::TianShanIce::Properties& props)
:BaseCmd(env), _amdCB(amdCB), _name(name), _deviceZone(deviceZone), _xmlDefFile(xmlDefFile),_bCompress(bCompress),_props(props)
{
	_bXMLBody = false;
}

ImportDeviceCmd::ImportDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDeviceXMLPtr& amdCB, const ::std::string& name, const ::std::string& deviceZone,  const ::TianShanIce::Properties& props, const std::string& xmlBody)
:BaseCmd(env), _amdCBXML(amdCB), _name(name), _deviceZone(deviceZone), _props(props),_xmlBody(xmlBody)
{
	_bXMLBody = true;
	_xmlDefFile = "";
	_bCompress = false;
}

int ImportDeviceCmd::run(void)
{
	int64 stampStart = ZQ::common::TimeUtil::now();
	std::string lastError;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ImportDeviceCmd, "Edge Device[%s]Zone[%s] enter Import"), _name.c_str(), _deviceZone.c_str());
	XmlImporter* deviceXMLImporterPtr = NULL;
	try 
	{
/*		::Ice::Identity deviceIdent;
		deviceIdent.name = _name;
		deviceIdent.category = DBFILENAME_EdgeDevice;
		if (!_env._eEdgeDevice->hasObject(deviceIdent))
*/
		if(1)
		{
			if(_bXMLBody)
			{
				_xmlDefFile = "../ex_devices.xml";
				deviceXMLImporterPtr = new XmlImporter(_env, _amdCBXML, this, _name, _deviceZone, _props, _bXMLBody, _xmlBody);
			}
			else
				deviceXMLImporterPtr = new XmlImporter(_env, _amdCB, this, _name, _deviceZone, _props, _bXMLBody);

			int size = deviceXMLImporterPtr->parseFile(_xmlDefFile.c_str(), _bCompress);
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(ImportDeviceCmd, "importXml(%s) processed, %d bytes"), _xmlDefFile.empty()?"null":_xmlDefFile.c_str(), size);
			if (size < 0)
				lastError = "process xml error";
			else
				return 1;
		}
		else
			lastError = "Device[" + _name + "] already exist";
	}
	catch (::ZQ::common::XMLException &ex)
	{
		lastError = ex.what();
	}
	catch (::Ice::Exception &ex)
	{
		lastError = ex.what();
	}
	catch(...)
	{
		lastError = "ImportDeviceCmd: caught unknown exception";
	}

	TianShanIce::ServerError ex("EdgeRM", 607, lastError);
	_amdCB->ice_exception(ex);

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(ImportDeviceCmd, "processed import all devices took %dms"), (int)(ZQ::common::TimeUtil::now()-stampStart));
	envlog(ZQ::common::Log::L_ERROR, CLOGFMT(ImportDeviceCmd, "importXml(%s) %s"), _xmlDefFile.empty()?"null":_xmlDefFile.c_str(), lastError.c_str());
	delete deviceXMLImporterPtr;
	deviceXMLImporterPtr = NULL;

	return 1;
}

::std::string ImportDeviceCmd::getXMLAttribute(::ZQ::common::XMLPreferenceEx *element, const char *attribute)
{
	if (element->getAttributeValue(attribute, strValue))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ImportDeviceCmd, "Edge Device[%s]Zone[%s] get attribute(%s) value(%s)"), _name.c_str(), _deviceZone.c_str(), attribute, strValue);
	}
	else
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ImportDeviceCmd, "Edge Device[%s]Zone[%s] get attribute(%s) failed"), _name.c_str(), _deviceZone.c_str(), attribute);
		strValue[0] = 0;
	}
	return ::std::string(strValue);
}

::Ice::Int ImportDeviceCmd::getXMLIntAttribute(::ZQ::common::XMLPreferenceEx *element, const char *attribute)
{
	::Ice::Int iRet = -1;
	if (element->getAttributeValue(attribute, strValue))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ImportDeviceCmd, "Edge Device[%s]Zone[%s] get attribute(%s) value(%s)"), _name.c_str(), _deviceZone.c_str(), attribute, strValue);
		iRet = atoi(strValue);
	}
	else
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ImportDeviceCmd, "Edge Device[%s]Zone[%s] get attribute(%s) failed"), _name.c_str(), _deviceZone.c_str(), attribute);
	}
	return iRet;
}

// -----------------------------
// class NegociateAllocCmd
// -----------------------------
///
NegociateAllocCmd::NegociateAllocCmd(EdgeRMEnv& env, AllocationImpl& alloc, const ::TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr& amdCB)
:BaseCmd(env), _amdCB(amdCB), _alloc(alloc)
{
}


int NegociateAllocCmd::run(void)
{
	std::string lastError;
	try 
	{
#pragma message ( __MSGLOC__ "TODO: impl NegociateAllocCmd")
		return 0;
	}
	catch(...)
	{
	}

	TianShanIce::ServerError ex("EdgeRM", 607, lastError);
	_amdCB->ice_exception(ex);
	return 1;

}

// -----------------------------
// class NegotiateResourcesCmd
// -----------------------------
NegotiateResourcesCmd::NegotiateResourcesCmd(EdgeRMEnv& env, const TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr& amdCB, AllocationImpl& alloc)
: BaseCmd(env), _amdCB(amdCB), _alloc(alloc)
{
}

int NegotiateResourcesCmd::run(void)
{
	std::string lastError;
	try 
	{
#pragma message ( __MSGLOC__ "TODO: impl NegotiateResourcesCmd")
		return 0;
	}
	catch(...)
	{
	}

	TianShanIce::ServerError ex("NegotiateResourcesCmd", 607, lastError);
	_amdCB->ice_exception(ex);
	return 1;

}

// -----------------------------
// class DiffAllocCmd
// -----------------------------
// test data for get the total count and time of one sync

int DiffAllocCmd::_testDeviceNum = 0;
int DiffAllocCmd::_testGetIdsTime = 0;
int DiffAllocCmd::_testCompareTime = 0;
int DiffAllocCmd::_testGetAllocTime = 0;
int DiffAllocCmd::_testTotalTime = 0;

int DiffAllocCmd::_testGetIdsCount = 0;
int DiffAllocCmd::_testGetAllocCount = 0;

DiffAllocCmd::DiffAllocCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_diffAllocationsPtr& amdCB, const std::string& deviceName, const ::TianShanIce::StrValues& allocIds)
:BaseCmd(env), _amdCB(amdCB), _deviceName(deviceName),_remoteAllocIds(allocIds)
{

}

int DiffAllocCmd::run(void)
{
	// test data for get the total count and time of one sync

	if (_testDeviceNum == 0)
	{
		_testGetIdsTime = 0;
		_testCompareTime =0;
		_testGetAllocTime =0;
		_testTotalTime = 0;
		_testGetIdsCount = 0;
		_testGetAllocCount =0;
	}
	_testDeviceNum = (_testDeviceNum + 1) %5;
	
	int64 ltimeTotal = ZQ::common::TimeUtil::now();
	std::string lastError;
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(DiffAllocCmd, "sync Allocations of device[%s] "), _deviceName.c_str());
	::TianShanIce::EdgeResource::AlloctionValues addAllocs;
	::TianShanIce::StrValues localAllocIds;
	try{
		// step 1. read local allocations
		int64 ltimeStamp = ZQ::common::TimeUtil::now();
		Ice::ObjectPrx objPrx = NULL;
		TianShanIce::EdgeResource::EdgeDevicePrx edgeDevicePrx = _env.getEdgeRMPrx()->openDevice(_deviceName);
		if(edgeDevicePrx == NULL)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(DiffAllocCmd, "failed to open device[%s] "), _deviceName.c_str());
			return -1;
		}
		#if  ICE_INT_VERSION / 100 >= 306
			objPrx = edgeDevicePrx->ice_collocationOptimized(false);
		#else
			objPrx = edgeDevicePrx->ice_collocationOptimization(false);
		#endif
		TianShanIce::EdgeResource::EdgeDeviceExPrx edgeDeviceExPrx = TianShanIce::EdgeResource::EdgeDeviceExPrx::uncheckedCast(objPrx);
		localAllocIds = edgeDeviceExPrx->listAllocationIds();

 		_testGetIdsTime += (int)(ZQ::common::TimeUtil::now() - ltimeStamp);
 		_testGetIdsCount += localAllocIds.size();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(DiffAllocCmd, "read local [%d] alloctionIds from device[%s], took %dmsec"),localAllocIds.size(), _deviceName.c_str(), (int)(ZQ::common::TimeUtil::now() - ltimeStamp));	
		// step 2. compare localIds with remote _allocIds, group them into three categories
		//              2a. local-orphans, 2b. both have, 2c remote orphans
		// localAllocIds;       local ERM AllocationIds	
		// _remoteAllocIds;       remote ERM AllocationIds
		// intersectionAllocIds; local and remote ERM allocaionId 交集
		// localDiffAllocIds;   local ERM allocaionId与 intersectionAllocIds交集的差集, 代表Local ERM多余的Allocations, 删除
		// remoteDiffAllocIds;   remote ERM allocaionId与 intersectionAllocIds交集的差集, 代表Remote ERM多余的Allocations,增加到Loacl ERM
		TianShanIce::StrValues localDiffAllocIds, remoteDiffAllocIds, intersectionAllocIds;
		ltimeStamp = ZQ::common::TimeUtil::now();
		//取交集 localAllocIds and remoteAllocIds, 
		//intersectionAllocIds =  交集(localAllocIds,  remoteAllocIds);
		std::sort(localAllocIds.begin(), localAllocIds.end());
		std::sort(_remoteAllocIds.begin(), _remoteAllocIds.end());
		std::set_intersection(localAllocIds.begin(), localAllocIds.end(), _remoteAllocIds.begin(), _remoteAllocIds.end(), back_inserter(intersectionAllocIds));
		//取差集 localAllocIds和intersectionAllocIds差集
		//localDiffAllocIds = 差集(localAllocIds, intersectionAllocIds)
		std::set_difference(localAllocIds.begin(), localAllocIds.end(), intersectionAllocIds.begin(), intersectionAllocIds.end(), back_inserter(localDiffAllocIds));

		//取差集 remoteAllocIds和intersectionAllocIds差集
		//_remoteDiffAllocIds = 差集(remoteAllocIds, intersectionAllocIds)
		std::set_difference(_remoteAllocIds.begin(), _remoteAllocIds.end(), intersectionAllocIds.begin(), intersectionAllocIds.end(), back_inserter(remoteDiffAllocIds));
		_testCompareTime += (int)(ZQ::common::TimeUtil::now() - ltimeStamp);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(DiffAllocCmd, "compare allocations of device[%s], took %dmsec"), _deviceName.c_str(), (int)(ZQ::common::TimeUtil::now() - ltimeStamp));

		// step 3. put the allocIds of 2c into amdCB->allocIdsOfGone;
		// step 4. for each item in 2a, construct BValues, then group them into AlloctionValues, put the list into amdCB->ret
		ltimeStamp = ZQ::common::TimeUtil::now();
		for (TianShanIce::StrValues::iterator localAllIt = localDiffAllocIds.begin(); localAllIt != localDiffAllocIds.end(); localAllIt++)
		{		
			//addAllocs.push_back(localAllocs[*localAllIt]);
			Ice::Identity identAlloc;
			identAlloc.category = DBFILENAME_Allocation;
			identAlloc.name = *localAllIt;
			try {
				TianShanIce::EdgeResource::AllocationPrx alloc = IdentityToObjEnv(_env, Allocation, identAlloc);
				alloc->ice_ping();
				AllocationInfo allocInfo = alloc->getInfo();
				//get other info
				TianShanIce::EdgeResource::AlloctionRecord allocRec;
			      allocRec.stampCommitted = _atoi64(allocInfo.props[SYS_PROP(StampCommitted)].c_str());
				//init allocRec;
				std::string metadataVal;
				allocRec.ident = allocInfo.ident;
				allocRec.state = allocInfo.state;
				allocRec.resources = alloc->getResources();
				allocRec.owner = alloc->getOwner();
				metadataVal  = allocInfo.props[SYS_PROP(OwnerKey)];
				allocRec.ownerKey = metadataVal;
				metadataVal = allocInfo.props[SYS_PROP(UdpPort)];
				allocRec.udpPort = atoi(metadataVal.c_str());
				allocRec.programNumber = atoi(allocInfo.props[SYS_PROP(ProgramNumber)].c_str());
				allocRec.maxJitter = atof(allocInfo.props[SYS_PROP(MaxJitter)].c_str());
				allocRec.sourceIP = allocInfo.props[SYS_PROP(SourceIP)];
				allocRec.bandwidth = _atoi64(allocInfo.props[SYS_PROP(Bandwidth)].c_str());
				allocRec.stampCreated = _atoi64(allocInfo.props[SYS_PROP(StampCreated)].c_str());
				allocRec.stampProvisioned = _atoi64(allocInfo.props[SYS_PROP(StampProvisioned)].c_str());

				allocRec.expiration = _atoi64(allocInfo.props[SYS_PROP(Expiration)].c_str());
				TianShanIce::EdgeResource::AllocationExPrx allocExPrx = TianShanIce::EdgeResource::AllocationExPrx::uncheckedCast(alloc);
				if(allocExPrx == NULL)
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(DiffAllocCmd, "exportAllocationsCmd() wrong allocation proxy"));
					return false;
				}
				allocRec.channelAssocs = allocExPrx->getChannelAssocs();
				allocRec.retrytimes = atoi(allocInfo.props[SYS_PROP(RetryTimes)].c_str());
				allocRec.sessionGroup = allocInfo.props[SYS_PROP(SessionGroup)];
				allocRec.onDemandSessionId = allocInfo.props[SYS_PROP(OnDemandSessionId)];
				allocRec.qamSessionId = allocInfo.props[SYS_PROP(QamSessionId)];
				allocRec.qamSessGroup = allocInfo.props[SYS_PROP(QamSessionGroup)];
				TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = _env._openChannel(allocRec.channelAssocs[0].identCh.name);
				if (channelPrx == NULL)
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT(DiffAllocCmd, "failed to open channel use name[%s]"),allocRec.channelAssocs[0].identCh.name.c_str());
					continue;
				}
				allocRec.link = channelPrx->getAllocLink(allocRec.ident, allocRec.programNumber);
				TianShanIce::BValues value;
				AllocationImpl::marshal(allocRec, value, _env._adapter->getCommunicator());
				addAllocs.push_back(value);
            	}
			catch(const ::Ice::Exception& ex) 
			{
				lastError = ex.ice_name();
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(DiffAllocCmd, "get orphan allocations[%s] of device[%s] caught exception [%s]"), localAllIt->c_str(),  _deviceName.c_str(), lastError.c_str());
			}
			catch (...)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(DiffAllocCmd, "exportAllocationsCmd() Allocation getInfo catch unknown exception"));
			}
		}
 		_testGetAllocTime += (int)(ZQ::common::TimeUtil::now() - ltimeStamp);
 		_testGetAllocCount += localDiffAllocIds.size();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(DiffAllocCmd, "get [%d] orphan allocations of device[%s], took %dmsec"), localDiffAllocIds.size(),  _deviceName.c_str(), (int)(ZQ::common::TimeUtil::now() - ltimeStamp));
		_amdCB->ice_response(addAllocs,remoteDiffAllocIds);
		_testTotalTime += (int)(ZQ::common::TimeUtil::now() - ltimeTotal);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(DiffAllocCmd, "sync all allocation of device[%s], took %dmsec"), _deviceName.c_str(), (int)(ZQ::common::TimeUtil::now() - ltimeTotal));
		if (_testDeviceNum == 0)
 		{
 			envlog(ZQ::common::Log::L_INFO, CLOGFMT(DiffAllocCmd, "sync all allocations of 5 device, got [%d]AllocIds took %dmsec, compare the AllocIds took %dmsec, fill [%d] Allocs into respose took %dmsec, subtotal took %dmsec")
 				                                                                   , _testGetIdsCount,_testGetIdsTime,_testCompareTime,_testGetAllocCount,_testGetAllocTime,_testTotalTime);
 		}
		return 1;
	}catch(const ::Ice::Exception& ex)
	{
		lastError = ex.ice_name();
	}
	catch(...)
	{
		lastError = "DiffAllocCmd: caught unknown exception";
	}
	TianShanIce::ServerError ex("EdgeRM", 607, lastError);
	_amdCB->ice_exception(ex);
	return -1;
}



}} // namespace
