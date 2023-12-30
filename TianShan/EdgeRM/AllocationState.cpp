// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Poscontention, use,
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
// Ident : $Id: AllocState.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/AllocationState.cpp $
// 
// 11    2/20/14 10:10a Li.huang
// 
// 10    1/10/14 10:27a Bin.ren
// add DB operation threads
// 
// 9     7/02/13 5:30p Bin.ren
// change ServiceGroup to RouteNames
// 
// 8     7/01/13 3:58p Li.huang
// 
// 7     7/01/13 2:01p Bin.ren
// 
// 6     5/23/13 4:00p Li.huang
// 
// 5     5/22/13 3:40p Li.huang
// add it to linux build
// 
// 4     3/25/13 2:21p Bin.ren
// 
// 3     3/20/13 11:19a Li.huang
// add R6
// 
// 2     11/01/12 3:30p Li.huang
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 27    10-02-11 13:42 Li.huang
// 
// 26    10-02-09 15:28 Li.huang
// 
// 25    10-02-01 17:17 Li.huang
// fix some bugs
// 
// 24    10-01-13 15:15 Li.huang
// modify log
// 
// 23    10-01-04 11:44 Li.huang
// modify resource edgeDeviceGroup to edgeDeviceZone
// 
// 22    09-12-23 21:12 Li.huang
// 
// 21    09-12-22 16:02 Hui.shao
// 
// 20    09-12-10 18:22 Li.huang
// modify allocation provision logic 
// 
// 19    09-11-26 18:09 Li.huang
// 
// 18    09-11-26 15:42 Li.huang
// fix some bugs 
// 
// 17    09-11-25 14:06 Li.huang
// add  S6 alllocation retry
// 
// 16    09-11-20 17:02 Li.huang
// 
// 15    09-11-18 16:04 Li.huang
// fix some bugs
// 
// 14    09-11-06 16:28 Li.huang
// 
// 13    09-11-03 18:32 Li.huang
// modify some errors
// 
// 12    09-10-30 11:58 Li.huang
// 
// 11    09-10-14 18:14 Li.huang
// 
// 10    09-09-28 16:10 Li.huang
// 
// 9     09-08-10 16:21 Xiaoming.li
// 
// 8     09-08-07 9:54 Xiaoming.li
// add S6 handler
// 
// 7     09-07-31 9:36 Xiaoming.li
// 
// 6     09-07-30 17:16 Xiaoming.li
// get edgeport info
// 
// 5     09-07-27 17:00 Xiaoming.li
// add lock for allocation
// 
// 4     09-07-22 15:19 Xiaoming.li
// correct PN select and allocIdent
// 
// 3     09-03-26 15:17 Hui.shao
// impl of state inservice and outofservice
// 
// 2     09-03-19 17:12 Hui.shao
// init draft of evaluate, commit and withdraw,
// plus the states of allocation
// 
// 1     09-03-05 19:41 Hui.shao
// initially created
// ===========================================================================

#include "AllocationState.h"
#include "EdgeRMImpl.h"
#include "EdgeRMCmds.h"
#include "TianShanIceHelper.h"

extern "C"
{
#include <stdlib.h>
#include <time.h>
}

#include <stdarg.h>
#include <stdio.h>

namespace ZQTianShan {
namespace EdgeRM {

#define AllocStateLOGFMT(_C, _X) CLOGFMT(_C, "alloc[%s:%s(%d)] " _X), _alloc.ident.name.c_str(), AllocStateBase::stateStr(_alloc.state), _alloc.state
#define AllocStateEXPFMT(_C, _ERRCODE, _X) EXPFMT(_C, _ERRCODE, "alloc[%s:%s(%d)] " _X), _alloc.ident.name.c_str(), AllocStateBase::stateStr(_alloc.state), _alloc.state
#define AllocEventFMT(_EVENT, _EVENTCODE, _X) EventFMT(_env._netId.c_str(), EdgeAlloc, _EVENT, _EVENTCODE, "Allocation[%s] " _X), _alloc.ident.name.c_str()

static int ChannelAssoc_comp(const TianShanIce::EdgeResource::ChannelAssoc& a, const TianShanIce::EdgeResource::ChannelAssoc& b)
{
	return (int) (a.costOfAlloc - b.costOfAlloc);
}

// -----------------------------
// class AllocStateBase
// -----------------------------
const char* AllocStateBase::stateStr(const TianShanIce::State state)
{
	return ZQTianShan::ObjStateStr(state);
}

TianShanIce::State AllocStateBase::stateId(const char* stateStr)
{
	return ZQTianShan::StrToStateId(stateStr);
}

AllocStateBase::AllocStateBase(EdgeRMEnv& env, AllocationImpl& alloc, const TianShanIce::State state)
: _env(env), _alloc(alloc), _oldState(alloc.state), _theState(state)
{
	_stampCreated = now();
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(EdgeRM,"AllocStateBase() OldState[%s]->State[%s] threadpool[%d/%d], pending[%d]"), ObjStateStr(_oldState), ObjStateStr(_theState), _env._thpool.activeCount(), _env._thpool.size(), _env._thpool.pendingRequestSize());
}

void AllocStateBase::leave(void)
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocState, "leave()"));
}

void AllocStateBase::_commitState(bool fireEvent, const ::std::string& msg)
{
	//AllocationImpl::WLock lock(_alloc);
	
	_alloc.state = _theState;
	if (_theState == _oldState)
	{
#ifdef _DEBUG
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocState, "_commitState() same state, ignore"));
#endif // _DEBUG
		return;
	}

	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocState, "committing state change: %s(%d) -> %s(%d) state change took %dms"),
			AllocStateBase::stateStr(_oldState), _oldState, AllocStateBase::stateStr(_theState), _theState, now() - _stampCreated);

	if (!fireEvent)
		return;

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	evntlog(AllocEventFMT(StateChanged, 2, "oldState[%s(%d)] newState[%s(%d)] %s"),
		AllocStateBase::stateStr(_oldState), _oldState, AllocStateBase::stateStr(_theState), _theState, msg.c_str());	
}

#define MINI_TIMMER_MS		50
void AllocStateBase::_renew(const ::Ice::Long newExpiration)
{
	if (newExpiration >0)
	{
		int nTime = int(newExpiration-now());
		if (nTime < 0)
			nTime = 0;
		_alloc.expiration = newExpiration;
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocState, "update expiration with %d(ms)"), nTime);
		_env._watchDog.watch(_alloc.ident, (long) nTime);
	}
	else envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocState, "_renew() skip updating an invalid expiration value"));
}

void AllocStateBase::_destroy()
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocState, "_destroy() enter"));
	try
	{
		_env._eAllocation->remove(_alloc.ident);

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
		evntlog(AllocEventFMT(Destroyed, 1, ""));

		envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocState, "_destroy() alloc removed from DB"));
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocState, "_destroy() object already gone from the container, ignore"));
	}
	catch(const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocState, "_destroy() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocState, "_destroy() caught unknown exception"));
	}

}

void AllocStateBase::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = now();

	if (0 == _alloc.expiration) // adjust the expiration if it isn't set
	{
		if (0 == _alloc.stampCommitted)
			_alloc.expiration = _alloc.stampCreated + MAX_INSTANCE_IDLE;
		else _alloc.expiration = _alloc.stampCommitted + UNATTENDED_TIMEOUT;

		char buf[40];
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocState, "OnTimer() forced unset expiration to %s"), ZQTianShan::TimeToUTC(_alloc.expiration, buf, sizeof(buf)-2));
	}

	if (_alloc.expiration - stampNow <= 0)
	{
		AllocStateOutOfService(_env, _alloc).enter();
		return;
	}

	_renew(_alloc.expiration);
}

// -----------------------------
// class AllocStateNotProvisioned
// -----------------------------
void AllocStateNotProvisioned::enter(void)
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateNotProvisioned, "enter()"));

	switch(_alloc.state)
	{
	case TianShanIce::stNotProvisioned:
		break; // do nothing, and continue with the initialization steps

	case TianShanIce::stProvisioned:
	case TianShanIce::stInService:
	case TianShanIce::stOutOfService:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, AllocStateEXPFMT(AllocStateNotProvisioned, 0101, "not allowed to enter"));
	}

	_commitState(false); // do not send the state change for this state
	OnTimer(::Ice::Current());
}

// -----------------------------
// class AllocStateProvisioned
// -----------------------------
void AllocStateProvisioned::enter(void)
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter()"));

	Ice::Long lstart  =  ZQTianShan::now();

	// only allowed to be entered from the state of stNotProvisioned
	switch(_alloc.state)
	{
	case TianShanIce::stProvisioned:
		//		_renew(ZQTianShan::now() + UNATTENDED_TIMEOUT);
		return; // quit the following steps

	case TianShanIce::stNotProvisioned:
		AllocStateNotProvisioned(_env, _alloc).leave();
		break;

	case TianShanIce::stInService:
	case TianShanIce::stOutOfService:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, AllocStateEXPFMT(AllocStateProvisioned, 101, "enter() not allowed from this state"));
	}

	// step 1. read the requirements on route and device names
	// Ice::Int serviceGrp = -1;
	TianShanIce::StrValues routeNames;
	Ice::Int modulateFormat = -1;
	typedef std::set <std::string > StrSet;
	TianShanIce::StrValues requiredDevices;
	StrSet channelnames;
	IdentCollection identChannels;
	std::string  DevicesZone="";
	std::string strRouteNames;

	// step 1.1 read the RouteNames, if not find rtServiceGroup resource, throw exception
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter check rtServiceGroup condition"));
	if (_alloc.resources.end() != _alloc.resources.find(TianShanIce::SRM::rtServiceGroup))
	{
		TianShanIce::SRM::Resource& res = _alloc.resources[TianShanIce::SRM::rtServiceGroup];
		/*	if (res.resourceData.end() != res.resourceData.find("id"))
		{
		::TianShanIce::Variant& vm = res.resourceData["id"];
		if (vm.bRange || vm.ints.size() <=0 )
		ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] resource[rtServiceGroup].\"id\" must be a non-range single value requirement"), _alloc.ident.name.c_str());

		// serviceGrp = res.resourceData["id"].ints[0];
		char buf[100];
		snprintf(buf,sizeof(buf) -2, "SVCGRP.%d", vm.ints[0]);
		routeNames.push_back(buf);
		}*/
		if (res.resourceData.end() != res.resourceData.find("routeName"))
		{
			::TianShanIce::Variant& vm = res.resourceData["routeName"];
			if (vm.bRange || vm.strs.size() <=0 )
				ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] resource[rtServiceGroup].\"routeName\" must be a non-range string values"), _alloc.ident.name.c_str());
			// copy the "routeNames" into routeNames
			for (int i=0; i< vm.strs.size(); i++)
			{
				routeNames.push_back(vm.strs[i]);
				strRouteNames += vm.strs[i];
				strRouteNames += ",";
			}
		}
	}
	else
	{
		//throw exception
		ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] resource[rtServiceGroup] must be a specified to perform an allocation provisioning"), _alloc.ident.name.c_str());
	}
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave check rtServiceGroup condition, routeNames:%s"),strRouteNames.c_str());

	// step 1.2 read the rtAtscModulationMode
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter check rtAtscModulationMode condition"));
	if (_alloc.resources.end() != _alloc.resources.find(TianShanIce::SRM::rtAtscModulationMode))
	{
		TianShanIce::SRM::Resource& res = _alloc.resources[TianShanIce::SRM::rtAtscModulationMode];
		if (res.resourceData.end() == res.resourceData.find("modulationFormat") ||
			res.resourceData["modulationFormat"].bRange || res.resourceData["modulationFormat"].bin.size() < 1)
			ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] resource[rtAtscModulationMode].\"modulationFormat\" must be a non-range value requirement"), _alloc.ident.name.c_str());
		else
			modulateFormat = res.resourceData["modulationFormat"].bin[0];
	}
	else
	{
		//throw exception
		ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] resource[rtAtscModulationMode] must be a specified to perform an allocation provisioning"), _alloc.ident.name.c_str());
	}
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave check rtAtscModulationMode[%d] condition"), modulateFormat);

	// step 1.3 read the  DeviceZone, if devicezone not empty, list all device of zone
	// if requiredDevices  empty, throw exception
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter check rtPhysicalChannel condition"));
	if (_alloc.resources.end() != _alloc.resources.find(TianShanIce::SRM::rtPhysicalChannel))
	{
		TianShanIce::SRM::Resource& res = _alloc.resources[TianShanIce::SRM::rtPhysicalChannel];
		/*	
		//step 1.3.1 read edgeDeviceName resourcedata;
		if (res.resourceData.end() == res.resourceData.find("edgeDeviceName") || res.resourceData["edgeDeviceName"].bRange || res.resourceData["edgeDeviceName"].strs.size() < 1)
		{
		//	envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocStateProvisioned, "resource[rtPhysicalChannel].\"edgeDeviceName\" must be a non-range single value requirement"));
		requiredDevices.clear();
		}
		else
		{
		requiredDevices = res.resourceData["edgeDeviceName"].strs;
		}
		*/

		//step 1.3.2 read edgeDeviceZone resourcedata;
		requiredDevices.clear();
		DevicesZone = "";
		if (res.resourceData.end() == res.resourceData.find("edgeDeviceZone")||res.resourceData["edgeDeviceZone"].bRange || res.resourceData["edgeDeviceZone"].strs.size() < 1)
		{
			envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocStateProvisioned, "resource[rtPhysicalChannel].\"edgeDeviceZone\" must be a non-range single value requirement"));
		}
		else
			DevicesZone = res.resourceData["edgeDeviceZone"].strs[0];
		if (!DevicesZone.empty())
		{
			// step 1.3.3 populate the devices of the specified zone
			TianShanIce::StrValues deviceOfZones;
			IdentCollection ids = _env._idxDeviceOfZONE->find(DevicesZone);
			for (IdentCollection::iterator it = ids.begin(); it < ids.end(); it++)
			{	
				envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "find Device[%s] from Zone[%s]"), (it->name).c_str(), DevicesZone.c_str());
				deviceOfZones.push_back(it->name);
			}
			requiredDevices = deviceOfZones;
			if(requiredDevices.empty())
			{
				ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] invalid resource[rtPhysicalChannel].\"edgeDeviceName\" .\"edgeDeviceZone\" perform an allocation provisioning"),
					_alloc.ident.name.c_str());
			}

			/*	if (requiredDevices.size() <= 0)
			requiredDevices = deviceOfZones;
			else
			{
			// step 1.3.4 同时指定了 QAM zone和QAM list, 则需要根据QAM Zone来过滤 QAM list(取交集)
			TianShanIce::StrValues tmp = requiredDevices; // make a backup
			requiredDevices.clear();

			std::sort(tmp.begin(), tmp.end());
			std::sort(deviceOfZones.begin(), deviceOfZones.end());

			TianShanIce::StrValues::iterator it1 = tmp.begin(), it2 = deviceOfZones.begin();
			while (it2 < deviceOfZones.end() && it1 < tmp.end())
			{
			int diff = it1->compare(*it2);
			if (0 == diff)
			{
			requiredDevices.push_back(*it1);
			it1++; it2++;
			}
			else if (diff <0)
			it1 ++;
			else it2 ++;
			}
			}*/
		}
	}
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave check rtPhysicalChannel[%d] condition"), requiredDevices.size());

#pragma message ( __MSGLOC__ "TODO: check other necessary resources have been preset. i.e. bandwidth")
	// step 2. get the channels of devices
	/// 如果requiredDevices为空,则根据RoutenameList 列出所有的Channel
	/// 如果requiredDevices不为空, 则需要列出Device下的所有Channel,然后跟routename list列出的channel取交集
	///根据RoutenameList 列出所有的Channel
	IdentCollection identRouteChannels;
	{
		ZQ::common::MutexGuard gd(_env._lkIdxRouteName);
		for (int i=0; i< routeNames.size(); i++)
		{
			EdgeRMEnv::RouteIndex::iterator itLast = _env._idxRouteName.upper_bound(routeNames[i]);
			for (EdgeRMEnv::RouteIndex::iterator it = _env._idxRouteName.find(routeNames[i]); it != itLast && it != _env._idxRouteName.end(); it++)
			{
				identRouteChannels.push_back(it->second);
			}
		}    
	}
	if(requiredDevices.empty())
		identChannels = identRouteChannels;
	else
	{
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter get channels from %d devices"), requiredDevices.size());
		for (TianShanIce::StrValues::iterator itDevice = requiredDevices.begin(); itDevice < requiredDevices.end(); itDevice++)
		{
			::Ice::Identity identDevice;
			identDevice.category = DBFILENAME_EdgeDevice;
			identDevice.name = *itDevice;

			/*		
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter find channels from devices[%s]"), (*itDevice).c_str());
			IdentCollection ids = _env._idxChannelOfDevice->find(identDevice);
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter check channels identities of devices[%s]"), (*itDevice).c_str());
			for (IdentCollection::iterator it = ids.begin(); it < ids.end(); it++)
			{
			if (serviceGrp <0)
			{
			// if serviceGrp is not specified, take the results gotten by device directly
			identChannels.push_back(*it);
			continue;
			}
			channelnames.insert(it->name);
			}
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave find channels from devices[%s]"), (*itDevice).c_str());
			*/

			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter find channels from devices[%s]"), (*itDevice).c_str());
			::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
			DeviceChannelsMap::iterator itorDCMap= _env._devicechannels.find(*itDevice);
			if(itorDCMap == _env._devicechannels.end())
				continue;
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter check channels identities of devices[%s]"), (*itDevice).c_str());
			for (IdentCollection::iterator it = itorDCMap->second.begin(); it < itorDCMap->second.end(); it++)
			{
				if(find(identRouteChannels.begin(),identRouteChannels.end(),*it) !=  identRouteChannels.end())
					identChannels.push_back(*it);
			}
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave narrow the %d channel candidates by the specified routes[%s]"), channelnames.size(), strRouteNames.c_str());
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave find channels from devices[%s]"), (*itDevice).c_str());
		}
	}
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave get channels from device"));
	if (identChannels.empty()) // ServerError
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] no matched EdgeChannel found per resource[rtServiceGroup].\"id\" and resource[rtPhysicalChannel].\"edgeDeviceName\" must be a specified to perform an allocation provisioning"), _alloc.ident.name.c_str());

	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "enter random sort the pre-reserve channel list[%d]"), identChannels.size());
	::std::random_shuffle(identChannels.begin(), identChannels.end());
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave random sort the pre-reserve channel list"));

	if (_alloc._reserveNow)
	{
		if (_alloc._maxChannelCandidates <=0 || _alloc._maxChannelCandidates > _env._maxPreserveChannels)
			_alloc._maxChannelCandidates = _env._maxPreserveChannels;
	}
	else if (_alloc._maxChannelCandidates <=0 || _alloc._maxChannelCandidates > _env._maxEvaluateChannels)
		_alloc._maxChannelCandidates = _env._maxEvaluateChannels;

	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "%d EdgeChannel candidates found, evaluating %d from with preservation %s"), identChannels.size(), _alloc._maxChannelCandidates, _alloc._reserveNow? "ON":"OFF");

	for (IdentCollection::iterator itCh = identChannels.begin(); _alloc.channelAssocs.size() < _alloc._maxChannelCandidates && itCh < identChannels.end(); itCh++)
	{
		try {
#if 0
			::std::string deviceName;
			Ice::Short  portId, channelNum;
			if (EdgeRMImpl::chopForHiberarchy(*itCh, deviceName, portId, channelNum) <1)
				continue;

			// prepare the resource of rtPhysicalChannel
			var.bRange = false;
			var.type   = TianShanIce::vtStrings;
			var.strs.push_back(deviceName);
			TianShanIce::SRM::Resource resPhysicalChannel;
			resPhysicalChannel.status = rsRequested;
			resPhysicalChannel.attr = raNonMandatoryNonNegotiable;
			MAPSET(TianShanIce::ValueMap, resPhysicalChannel.resourceData, "edgeDeviceName", var);
#endif
			// prepare the resource map as the requirement to evaluate
			TianShanIce::EdgeResource::ChannelAssoc ca;
			ca.identCh = *itCh;
			ca.stampEmployed = ca.stampEvaluated =0;
			ca.resources = _alloc.resources;
			ca.pn = -1;
			ca.resources.erase(TianShanIce::SRM::rtPhysicalChannel);
			//			MAPSET(SRM::ResourceMap, _alloc.resources, TianShanIce::SRM::rtPhysicalChannel, resPhysicalChannel);

			// preform the evaluation per the resource requirement and get the result resource back
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "evaluating the book on ch[%s]"), ca.identCh.name.c_str());
			ca.ch = IdentityToObjEnv(_env, EdgeChannelEx, ca.identCh);
			ca.costOfAlloc = ca.ch->evaluate(ca.resources, ca.resources);

			// TODO:		if (ca.costOfAlloc > )
			//			continue;
			ca.stampEvaluated = now();

			if (_alloc._reserveNow)
			{
				envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "pre-reserve the book on ch[%s] with cost[%d]"), ca.identCh.name.c_str(), ca.costOfAlloc);
				ca.ch->commit(_alloc.ident, ca.resources, ca.resources);

				// read the output PN number
				if (ca.resources.end() == ca.resources.find(TianShanIce::SRM::rtMpegProgram))
					ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] channel[%s] _reserveNow=TRUE, resource[rtMpegProgram] not given by evaluation"), _alloc.ident.name.c_str(), ca.identCh.name.c_str());

				TianShanIce::SRM::Resource& res = ca.resources[TianShanIce::SRM::rtMpegProgram];
				if (res.resourceData.end() == res.resourceData.find("Id") || res.resourceData["Id"].lints.size() !=1)
					ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "enter() alloc[%s] channel[%s] _reserveNow=TRUE, resource[rtMpegProgram] has no unqiue assigned program number"), _alloc.ident.name.c_str(), ca.identCh.name.c_str());

				ca.pn = res.resourceData["Id"].lints[0];
				ca.stampEmployed = ca.stampEvaluated;
			}

			_alloc.channelAssocs.push_back(ca);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "enter() caught exception[%s] %s"), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "enter() caught exception[%s]"), ex.ice_name().c_str());
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "enter() caught unknown exception"));
		}
	}

	if (_alloc.channelAssocs.size() <= 0)
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT(AllocStateProvisioned, 1173, "alloc[%s] enter() no appreciated channels are found for the allocation, provision failed"), _alloc.ident.name.c_str());

	::std::sort(_alloc.channelAssocs.begin(), _alloc.channelAssocs.end(), ChannelAssoc_comp);

	{
		TianShanIce::Variant var;
		var.bRange = false;
		var.type = TianShanIce::vtLongs;

		TianShanIce::EdgeResource::ChannelAssocation::iterator itorChAss = _alloc.channelAssocs.begin();
		for(; itorChAss != _alloc.channelAssocs.end(); itorChAss++)
		{
			TianShanIce::SRM::ResourceMap::iterator itorRsMap =  itorChAss->resources.find(TianShanIce::SRM::rtPhysicalChannel);
			if(itorRsMap == itorChAss->resources.end())
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT(AllocStateProvisioned, 1173, "alloc[%s] enter() no appreciated channels are found for the allocation, provision failed"), _alloc.ident.name.c_str());
			}
			TianShanIce::SRM::Resource& resPyCh = itorRsMap->second;

			TianShanIce::ValueMap::iterator itorVMap =  resPyCh.resourceData.find("channelId");
			if(itorVMap == resPyCh.resourceData.end())
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT(AllocStateProvisioned, 1173, "alloc[%s] enter() no appreciated channels are found for the allocation, provision failed"), _alloc.ident.name.c_str());
			}
			TianShanIce::Variant& varCh = itorVMap->second;
			var.lints.push_back(varCh.lints[0]);
		}

		TianShanIce::SRM::ResourceMap& resources = _alloc.resources;
		TianShanIce::SRM::ResourceMap::iterator itorRes = resources.find(TianShanIce::SRM::rtPhysicalChannel);
		TianShanIce::SRM::Resource resource;
		if(itorRes == resources.end())
		{
			resource.status = TianShanIce::SRM::rsRequested;
			resource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
			MAPSET(TianShanIce::SRM::ResourceMap, _alloc.resources, TianShanIce::SRM::rtPhysicalChannel, resource);
		}

		MAPSET(TianShanIce::ValueMap, _alloc.resources[TianShanIce::SRM::rtPhysicalChannel].resourceData, "channelId", var);
	}

	_alloc.stampProvisioned = now();

	envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateProvisioned, "%d EdgeChannel candidates are taken with preservation %s"), _alloc.channelAssocs.size(), _alloc._reserveNow? "ON":"OFF");

	_alloc.expiration = _alloc.stampProvisioned + UNATTENDED_TIMEOUT;
	_renew(_alloc.expiration);

	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "leave enter() took %d ms"),  ZQTianShan::now() - lstart);
}

/*
void AllocStateProvisioned::OnTimer(const ::Ice::Current& c)
{
	::Ice::Long stampNow = now();

	::Ice::Long scheduledEnd = 0;
	::std::string scheduledEndStr;
	if (_alloc.metaData.end() != _alloc.metaData.find(METADATA_ScheduledProvisonEnd))
	{
		scheduledEndStr = _alloc.metaData[METADATA_ScheduledProvisonEnd];
		scheduledEnd = ISO8601ToTime(scheduledEndStr.c_str());
	}
	else
	{
		scheduledEnd = _alloc.stampCreated + MAX(MAX_NOT_PROVISIONED_TIMEOUT + (1*24*60*60*1000), (3*24*60*60*1000)); // remain 3 days for maximal
		char buf[64];
		scheduledEndStr = TimeToUTC(scheduledEnd, buf, sizeof(buf) -2);
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "OnTimer() metaData[" METADATA_ScheduledProvisonEnd "] is not specified, using expiration[%s]"), scheduledEndStr.c_str());
		MAPSET(TianShanIce::Properties, _alloc.metaData, METADATA_ScheduledProvisonEnd, scheduledEndStr);
	}

	if (stampNow < scheduledEnd + UNATTENDED_TIMEOUT)
	{
		try {
			if (_alloc.bDirtyAttrs)
			{
				// avoid the attrbiute populating from file won't happened too frequently
				if (stampNow - _alloc.stampLastUpdated > MIN_UPDATE_INTERVAL)
				{
					envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "OnTimer() the attributes are out-of-date, force to popluate attributes from the file"));
					(new PopulateFileAttrsCmd(_env, _alloc.ident))->execute();
				}
				else _renew(_alloc.stampLastUpdated + MIN_UPDATE_INTERVAL);

				return;
			}
		}
		catch(const ZQ::common::Exception& ex)
		{
			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AllocStateProvisioned,"OnTimer() caugh exception: %s"), ex.getString());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "OnTimer() caught unknown exception when add PopulateFileAttrsCmd"));
		}

		_renew(stampNow + UNATTENDED_TIMEOUT); // do not allow 0 expiration by default
		return;
	}

	// stampNow > scheduledEnd + UNATTENDED_TIMEOUT	if the program reached here,

	// check the provision session if there is any bound
	try {
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "OnTimer() metaData[" METADATA_ScheduledProvisonEnd "]=[%s] checking the Privisioning session[%s]"), scheduledEndStr.c_str(), _alloc.provisionPrxStr.c_str());
		TianShanIce::ContentProvision::ProvisionSessionPrx provisionSess;
		provisionSess = TianShanIce::ContentProvision::ProvisionSessionPrx::uncheckedCast(_env._adapter->getCommunicator()->stringToProxy(_alloc.provisionPrxStr));
		provisionSess->ice_ping();
	}
	catch(const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(AllocStateProvisioned,"OnTimer() force to populate due to cast ProvisionSession caugh exception: %s"), ex.ice_name().c_str());
		_alloc.bDirtyAttrs = true;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "OnTimer() force to populate due to cast ProvisionSession caugh unknown exception"));
		_alloc.bDirtyAttrs = true;
	}

	try {
		if (_alloc.bDirtyAttrs)
		{
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "OnTimer() attributes needs populated due to ProvisionSession status"));
			(new PopulateFileAttrsCmd(_env, _alloc.ident))->execute();
		}
	}
	catch(const ZQ::common::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AllocStateProvisioned,"OnTimer() populate attrs per provision session, caugh exception: %s"), ex.getString());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(AllocStateProvisioned,"OnTimer() populate attrs per provision session, caugh unknown exception: %s"));
	}

	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateProvisioned, "OnTimer() renew the timer for next wakup in %d msec"), UNATTENDED_TIMEOUT);
	_renew(stampNow + UNATTENDED_TIMEOUT); // do not allow 0 expiration by default
}
// */

// -----------------------------
// class AllocStateInService
// -----------------------------
void AllocStateInService::enter(void)
{	
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateInService, "enter()"));

	switch(_alloc.state)
	{

	case TianShanIce::stInService:
		return; // do nothing

	case TianShanIce::stNotProvisioned:
		AllocStateNotProvisioned(_env, _alloc).leave();
		break;

	case TianShanIce::stProvisioned:
		AllocStateProvisioned(_env, _alloc).leave();
		break;

	case TianShanIce::stOutOfService:
	default:
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, AllocStateEXPFMT(AllocStateInService, 101, "enter() not allowed from this state"));
	}

	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateInService, "enter()"));

	TianShanIce::EdgeResource::ChannelAssocation selectChannel;
	selectChannel.clear();
	for (TianShanIce::EdgeResource::ChannelAssocation::iterator it = _alloc.channelAssocs.begin(); it < _alloc.channelAssocs.end(); it++)
	{
		try {
			if (!selectChannel.empty())
			{
				// already has a picked channel, free others
				envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateInService, "enter() giving up to book on ch[%s] pn[%d]"), it->identCh.name.c_str(), it->pn);
				if (it->pn >0 && it->stampEmployed >0)
				{
					it->ch->withdraw(_alloc.ident, it->pn);
					envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "enter() ch[%s] reservation on PN[%d] withdrawn"), it->identCh.name.c_str(), it->pn);
					it->pn = -1;
					it->stampEmployed = -1;
				}
				continue;
			}

			if (it->pn >0 && it->stampEmployed >0)
			{
				it->ch->ice_ping();
				envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "enter() ch[%s] PN[%d] is selected"), it->identCh.name.c_str(), it->pn);
				selectChannel.push_back(*it);
				continue;
			}

			// call to commit the channel
			envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateInService, "enter() committing the previous book on ch[%s]"), it->identCh.name.c_str(), it->pn);
			it->ch->commit(_alloc.ident, it->resources, it->resources);

			// read the output PN number
			if (it->resources.end() == it->resources.find(TianShanIce::SRM::rtMpegProgram))
				ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT(AllocStateProvisioned, 1173, "resource[rtMpegProgram] not given by evaluation of channel[%s]"), _alloc.ident.name.c_str(), it->identCh.name.c_str());

			TianShanIce::SRM::Resource& res = it->resources[TianShanIce::SRM::rtMpegProgram];
			if (res.resourceData.end() == res.resourceData.find("Id") || res.resourceData["Id"].lints.size() !=1)
				ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, EXPFMT(AllocStateProvisioned, 1173, "resource[rtMpegProgram] has no unqiue assigned program number"), _alloc.ident.name.c_str(), it->identCh.name.c_str());

			it->pn = res.resourceData["Id"].lints[0];
			it->stampEmployed = now();
			selectChannel.push_back(*it);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "ch[%s] pn[%d] caught exception[%s] %s"), it->identCh.name.c_str(), it->pn, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "ch[%s] pn[%d] caught exception[%s]"), it->identCh.name.c_str(), it->pn, ex.ice_name().c_str());
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateProvisioned, "ch[%s] pn[%d] caught unknown exception"), it->identCh.name.c_str(), it->pn);
		}
	}

	_alloc.channelAssocs = selectChannel;

	if (_alloc.channelAssocs.size() != 1)
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, EXPFMT(AllocStateInService, 1173, "enter() no appreciated channels [%d] are narrowed, serve failed"), _alloc.ident.name.c_str(), _alloc.channelAssocs.size());

	envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "enter() selected ch[%s] pn[%d]"), _alloc.channelAssocs[0].identCh.name.c_str(), _alloc.channelAssocs[0].pn);
	_alloc.resources = _alloc.channelAssocs[0].resources;
	_alloc.stampCommitted = now();
	_alloc.programNumber = _alloc.channelAssocs[0].pn;

	try
	{
	    ZQTianShan::Util::getResourceData(_alloc.channelAssocs[0].resources, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", _alloc.bandwidth);
		TianShanIce::EdgeResource::EdgePort edgePort = _alloc.channelAssocs[0].ch->getEdgePort();

		ZQTianShan::Util::getValueMapData(edgePort.resPhysicalChannel.resourceData, "edgeDeviceIP", _alloc.sourceIP);
		std::string edgDeviceName, edgeDeviceZone;
		ZQTianShan::Util::getValueMapData(edgePort.resPhysicalChannel.resourceData, "edgeDeviceName", edgDeviceName);
		ZQTianShan::Util::getValueMapData(edgePort.resPhysicalChannel.resourceData, "edgeDeviceZone", edgeDeviceZone);
		TianShanIce::Variant var;
		var.type = TianShanIce::vtStrings;
		var.bRange = false;
		var.strs.push_back(edgDeviceName);
		_alloc.resources[TianShanIce::SRM::rtPhysicalChannel].resourceData["edgeDeviceName"] = var;
		
		var.strs.clear();
		var.strs.push_back(edgeDeviceZone);
		_alloc.resources[TianShanIce::SRM::rtPhysicalChannel].resourceData["edgeDeviceZone"] = var;

		::ZQTianShan::Util::getResourceData(_alloc.channelAssocs[0].resources, TianShanIce::SRM::rtPhysicalChannel, "destPort", _alloc.udpPort); 

		_env.qamSessSetup(_alloc.ident.name,edgDeviceName);
	}
	catch (TianShanIce::InvalidParameter &ex)
	{
		envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "get bandwidth/edgeDeviceIP failed"));
	}

	_commitState();
	_renew(_alloc.stampCommitted + _env._allocationLeaseMs);
}

void AllocStateInService::OnTimer(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateInService, "OnTimer() enter"));
	::Ice::Long stampNow = now();

	if (0 == _alloc.expiration) // adjust the expiration if it isn't set
	{
		if (0 == _alloc.stampCommitted)
			_alloc.expiration = _alloc.stampCreated + MAX_INSTANCE_IDLE;
		else _alloc.expiration = _alloc.stampCommitted + UNATTENDED_TIMEOUT;

		char buf[32];
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateInService, "OnTimer() forced unset expiration to %s"), ZQTianShan::TimeToUTC(_alloc.expiration, buf, sizeof(buf)-2));
	}

	::Ice::Long diff = _alloc.expiration - stampNow;

	if (diff > 0)
	{
		_renew(_alloc.expiration);
		return;
	}

	bool bForceExpire = (diff > UNATTENDED_TIMEOUT);

	try {
		::Ice::Int renewTTL =-1;
		int mode = ErmInstanceSyncer::esm_Active;

		if(_env._pErmInstanceSyncer)
			mode = _env._pErmInstanceSyncer->getInstanceMode();

		if (_alloc.owner && (mode == ErmInstanceSyncer::esm_Active))
			renewTTL = _alloc.owner->OnAllocationExpiring(_alloc.ownerKey, IdentityToObjEnv(_env, Allocation, _alloc.ident));

		if(mode == ErmInstanceSyncer::esm_Standby)
			renewTTL = _env._allocationLeaseMs;

		if (renewTTL >0)
		{
			_renew(stampNow + renewTTL);
			_alloc.retrytimes = 0;
			return;
		}
		else
			bForceExpire = true;
	}
	catch(TianShanIce::ServerError &ex)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner caught exception[%s, %d, %s]"), 
			ex.category.c_str(), ex.errorCode, ex.message.c_str());

		if(_alloc.retrytimes >=  _env._retrytimes)
		{
			envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner, already retry (%d) times, Max retrytimes(%d)"), 
				_alloc.retrytimes, _env._retrytimes);
			bForceExpire = true;
		}
		else
		{
			bForceExpire = false;
			_alloc.retrytimes++;

			envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner, ready for retry (%d) times, renewing for %d ms more"), 
				_alloc.retrytimes, _env._retryInterval);
			_renew(stampNow + _env._retryInterval);
			return;
		}
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocStateInService, "OnTimer() alloc owner not exist, force to enter state[OutOfService]"));
		bForceExpire = true;
	}
	catch(const ::Ice::Exception& e)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner caught exception[%s]"), e.ice_name().c_str());
		if(_alloc.retrytimes >=  _env._retrytimes)
		{
			envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner, already retry (%d) times, Max retrytimes(%d)"), 
				_alloc.retrytimes, _env._retrytimes);
			bForceExpire = true;
		}
		else
		{
			bForceExpire = false;
			_alloc.retrytimes++;

			envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner, ready for retry (%d) times, renewing for %d ms more"), 
				_alloc.retrytimes, _env._retryInterval);
			_renew(stampNow + _env._retryInterval);
			return;
		}
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner caught unknown exception(%d)"), SYS::getLastErr());
		if(_alloc.retrytimes >=  _env._retrytimes)
		{
			envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner, already retry (%d) times, Max retrytimes(%d)"), 
				_alloc.retrytimes, _env._retrytimes);
			bForceExpire = true;
		}
		else
		{
			bForceExpire = false;
			_alloc.retrytimes++;

			envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "OnTimer() check alloc owner, ready for retry (%d) times, renewing for %d ms more"), 
				_alloc.retrytimes, _env._retryInterval);
			_renew(stampNow + _env._retryInterval);
			return;
		}
	}

	if (bForceExpire)
	{
		AllocStateOutOfService(_env, _alloc).enter();
		return;
	}

	envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocStateInService, "OnTimer() uncertain expiration, renewing for %d more"), UNATTENDED_TIMEOUT/3);
	_renew(stampNow + UNATTENDED_TIMEOUT/3);
}

// -----------------------------
// class AllocStateOutOfService
// -----------------------------
void AllocStateOutOfService::enter(void)
{	
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "enter()"));
	
	try {
		switch(_alloc.state)
		{
		case TianShanIce::stOutOfService:
			break; // do nothing, and continue with the initialization steps

		case TianShanIce::stNotProvisioned:
			AllocStateNotProvisioned(_env, _alloc).leave();
			break;

		case TianShanIce::stProvisioned:
			AllocStateProvisioned(_env, _alloc).leave();
			break;

		case TianShanIce::stInService:
			AllocStateInService(_env, _alloc).leave();
			break;

		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, AllocStateEXPFMT(AllocStateProvisionedStreamable, 101, "enter() not allowed from this state"));

		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocStateOutOfService, "enter() ignored exception[%s] when leaving prev state: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocStateOutOfService, "enter() ignored exception[%s] when leaving prev state"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_WARNING, AllocStateLOGFMT(AllocStateOutOfService, "enter() ignored unknown exception when leaving prev state"));
	}

	for (TianShanIce::EdgeResource::ChannelAssocation::iterator it = _alloc.channelAssocs.begin(); it < _alloc.channelAssocs.end(); it++)
	{
		try {
			// already has a picked channel, free others
			if (it->pn >0 && it->stampEmployed >0)
			{
				envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "enter() withdraw the utilization on ch[%s] pn[%d]"), it->identCh.name.c_str(), it->pn);
				it->ch->withdraw(_alloc.ident, it->pn);
				envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateOutOfService, "enter() ch[%s] reservation on PN[%d] withdrawn"), it->identCh.name.c_str(), it->pn);
				it->pn = -1;
				it->stampEmployed = -1;
			}
		}
		catch (const TianShanIce::BaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateOutOfService, "ch[%s] pn[%d] caught exception[%s] %s"), it->identCh.name.c_str(), it->pn, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateOutOfService, "ch[%s] pn[%d] caught exception[%s]"), it->identCh.name.c_str(), it->pn, ex.ice_name().c_str());
		}
		catch (...)
		{
			envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateOutOfService, "ch[%s] pn[%s] caught unknown exception"), it->identCh.name.c_str(), it->pn);
		}
	}

	_commitState();
	_alloc.expiration = ZQTianShan::now() + OUTSERVICE_TIMEOUT;
	_renew(_alloc.expiration);
}

void AllocStateOutOfService::OnTimer(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "OnTimer()"));
	::Ice::Long stampNow = now();

	if (_alloc.expiration <= 0)
	{
		_alloc.expiration = stampNow + OUTSERVICE_TIMEOUT;
		char buf[40];
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "OnTimer() uninitialized expiration, set to %s"), ZQTianShan::TimeToUTC(_alloc.expiration, buf, sizeof(buf)-2));
	}

	::Ice::Long diff = _alloc.expiration - stampNow;

	if (diff >0)
	{
		_renew(_alloc.expiration);
		return;
	}

	_destroy();
}

/*


void AllocStateInService::OnRestore(const ::Ice::Current& c)
{
	if (stampLastFileWrite.empty())
	{
		envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateInService, "OnRestore() force to out-service since the file doesn't exist on filesystem anymore"));
		AllocStateOutOfService(_env, _alloc).enter();
		return;
	}

	::Ice::Long lastWrite = ZQTianShan::ISO8601ToTime(stampLastFileWrite.c_str());
	if (lastWrite > _alloc.stampLastUpdated)
		_alloc.bDirtyAttrs = true;

	OnTimer(c);
}

// -----------------------------
// class AllocStateOutOfService
// -----------------------------
void AllocStateOutOfService::enter(void)
{	
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "enter()"));
	
	try {
		switch(_alloc.state)
		{
		caseTianShanIce::stOutOfService:
			break; // do nothing, and continue with the initialization steps

		case TianShanIce::stNotProvisioned:
			AllocStateNotProvisioned(_env, _alloc).leave();
			break;

		case TianShanIce::stProvisioning:
			_cancelProvision();
			AllocStateProvisioned(_env, _alloc).leave();
			break;

		case TianShanIce::stProvisioningStreamable:
			_cancelProvision();
			AllocStateProvisionedStreamable(_env, _alloc).leave();
			break;

		case TianShanIce::stInService:
			AllocStateInService(_env, _alloc).leave();
			break;

		default:
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, AllocStateEXPFMT(AllocStateProvisionedStreamable, 101, "enter() not allowed from this state"));

		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateOutOfService, "enter() ignore the exContentStoretion[%s] when leaving prev state: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateOutOfService, "enter() ignore the exContentStoretion[%s] when leaving prev state"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, AllocStateLOGFMT(AllocStateOutOfService, "enter() ignore the unknown exContentStoretion when leaving prev state"));
	}

	_commitState();
	_renew(ZQTianShan::now() + OUTSERVICE_TIMEOUT);
}

void AllocStateOutOfService::leave()
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "leave()"));
}

void AllocStateOutOfService::OnTimer(const ::Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "OnTimer()"));
	::Ice::Long stampNow = now();

	if (_alloc.expiration <= 0)
	{
		envlog(ZQ::common::Log::L_DEBUG, AllocStateLOGFMT(AllocStateOutOfService, "OnTimer() uninitialized expiration, set to expire in %d msec"), OUTSERVICE_TIMEOUT);
		_renew(stampNow + OUTSERVICE_TIMEOUT); // do not allow 0 expiration at this state
		return;
	}

	envlog(ZQ::common::Log::L_INFO, AllocStateLOGFMT(AllocStateOutOfService, "OnTimer() entering AllocStateCleaning"));
	AllocStateCleaning(_env, _alloc).enter();
}

void AllocStateOutOfService::OnRestore(const ::Ice::Current& c)
{
	OnTimer(c); // force to put self under watching
}
// */

}} // namespace
