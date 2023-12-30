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
// Ident : $Id: AllocationState.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMImpl.cpp $
// 
// 35    6/06/16 5:20p Li.huang
// ETV-TV-NOW-CR038
// 
// 34    1/12/16 11:40a Dejian.fei
// 
// 33    10/09/14 11:11a Hui.shao
// marshal/unmarshal exported as template
// 
// 32    5/14/14 11:23a Zonghuan.xiao
// add allocation count for snmp
// 
// 31    1/15/14 3:48p Bin.ren
// 
// 30    1/10/14 10:27a Bin.ren
// add DB operation threads
// 
// 29    11/27/13 2:19p Ketao.zhang
// add diffAllocations_async and getAllocationIds
// 
// 28    11/13/13 1:32p Bin.ren
// 
// 27    11/08/13 2:11p Bin.ren
// add device update
// 
// 26    11/01/13 4:34p Bin.ren
// 
// 25    10/11/13 5:49p Bin.ren
// add exportDevice interface
// 
// 24    10/11/13 3:15p Hui.shao
// merged the two sync-ers into one
// 
// 23    9/24/13 4:15p Li.huang
// 
// 22    9/24/13 2:42p Li.huang
// sync allocation
// 
// 21    9/11/13 4:13p Bin.ren
// 
// 20    9/11/13 1:26p Li.huang
// marshal alloction
// 
// 19    7/04/13 5:00p Bin.ren
// 
// 18    7/01/13 2:01p Bin.ren
// 
// 16    6/20/13 2:10p Li.huang
// 
// 15    6/20/13 2:04p Hui.shao
// 
// 14    6/18/13 11:36a Li.huang
// 
// 13    6/03/13 4:41p Li.huang
// add getEdgePort From EdgeDevice
// 
// 12    5/23/13 4:07p Li.huang
// 
// 11    5/22/13 3:40p Li.huang
// add it to linux build
// 
// 10    4/15/13 5:23p Li.huang
// 
// 9     3/29/13 2:27p Bin.ren
// 
// 8     3/28/13 1:49p Bin.ren
// 
// 7     3/25/13 2:21p Bin.ren
// 
// 6     3/20/13 11:19a Li.huang
// add R6
// 
// 5     11/01/12 3:29p Li.huang
// 
// 4     10/31/12 2:47p Li.huang
// 
// 3     10/25/12 2:59p Li.huang
// updata s6SessionGroup
// 
// 2     1/20/11 4:31p Li.huang
// add D6
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 34    10-02-01 17:17 Li.huang
// fix some bugs
// 
// 33    10-01-13 15:15 Li.huang
// modify log
// 
// 32    10-01-05 16:31 Li.huang
// add symbolrate to channel info and  don't evaluated if the  channel
// states is disabled
// 
// 31    10-01-04 11:44 Li.huang
// modify resource edgeDeviceGroup to edgeDeviceZone
// 
// 30    09-12-23 21:12 Li.huang
// 
// 29    09-12-18 9:33 Li.huang
// restore
// 
// 29    09-12-10 18:21 Li.huang
// remove deviceOfchannel Index  and add associated servicegroup to freq
// range
// 
// 28    09-11-26 18:09 Li.huang
// 
// 27    09-11-26 15:42 Li.huang
// fix some bugs 
// 
// 26    09-11-18 16:04 Li.huang
// fix some bugs
// 
// 25    09-11-03 18:32 Li.huang
// 
// 24    09-11-02 16:36 Li.huang
// 
// 23    09-10-30 11:58 Li.huang
// 
// 21    09-10-22 18:06 Li.huang
// 
// 20    09-10-22 17:45 Li.huang
// 
// 19    09-10-21 17:02 Li.huang
// 
// 18    09-10-14 18:15 Li.huang
// 
// 17    09-09-28 16:10 Li.huang
// 
// 16    09-08-12 17:20 Xiaoming.li
// 
// 15    09-07-30 17:16 Xiaoming.li
// get edgeport info
// 
// 14    09-07-27 17:00 Xiaoming.li
// add lock for allocation
// 
// 13    09-07-22 15:19 Xiaoming.li
// correct PN select and allocIdent
// 
// 12    09-07-16 9:55 Xiaoming.li
// add compress import function
// 
// 11    09-05-25 16:55 Xiaoming.li
// impl of device destroy
// 
// 10    09-04-13 16:16 Xiaoming.li
// impl edgedevice, channel, port, allocation list method
// 
// 9     09-03-31 9:13 Xiaoming.li
// impl list function
// 
// 8     09-03-26 15:16 Hui.shao
// added NegotiateResourcesCmd
// 
// 7     09-03-19 17:33 Hui.shao
// removed state::addResource()
// 
// 6     09-03-19 17:12 Hui.shao
// init draft of evaluate, commit and withdraw,
// plus the states of allocation
// 
// 5     09-03-11 11:38 Hui.shao
// build up the index from service group to channels
// 
// 4     09-03-09 19:22 Hui.shao
// 
// 3     09-03-09 17:29 Hui.shao
// 
// 2     09-03-05 19:41 Hui.shao
// defined program structure to impl
// 
// 1     09-02-26 17:53 Hui.shao
// initial created
// ===========================================================================

#include "EdgeRMImpl.h"
#include "EdgeRMCmds.h"
#include "AllocationState.h"
#include "Log.h"
#include "Guid.h"
#include "TianShanIceHelper.h"

#define MAX_EVAL_CANDIDATES (50)

extern "C" {
#include <time.h>
}

namespace ZQTianShan {
namespace EdgeRM {

#define INTERFACE_NAME_EdgeRM "EdgeRM"
#define envlog (_env._log)
	std::string Int2Str(Ice::Int i)
	{
		char temp[65] = "";
		sprintf(temp, "%d\0", i);
		return temp;
	}
	std::string Long2Str(Ice::Long i)
	{
		char temp[65] = "";
		sprintf(temp, "%lld\0", i);
		return temp;
	}
	std::string Float2Str(Ice::Float f)
	{
		char temp[256] = "";
		sprintf(temp, "%.6f\0", f);
		return temp;
	}

// -----------------------------
// service EdgeRMImpl
// -----------------------------
int EdgeRMImpl::chopForHiberarchy(const std::string& entryName, OUT std::string& deviceName, OUT Ice::Short& portId, OUT Ice::Short& channelNum)
{
	int ret =0;
	portId = channelNum =-1;
	deviceName = "";
	const char *p =NULL;

	size_t pos = entryName.find_last_of(LOGIC_FNSEPS);
	if (std::string::npos != pos)
	{
		deviceName = entryName.substr(0, pos);
		p = entryName.c_str() + pos +1;
	}
	else deviceName = entryName;

	if (!deviceName.empty())
		ret ++; // found the deviceName

	if (NULL == p)
		return ret;

	char num[3][16];
	int c = ::sscanf(p, "%[0-9].%[0-9]", num[0], num[1]);
	if (c > 0)
	{
		portId = ::atoi(num[0]);
		ret++;
	}

	if (c > 1)
	{
		channelNum = ::atoi(num[1]);
		ret ++;
	}

	return ret;
}

const std::string EdgeRMImpl::formatHiberarchy(const std::string& deviceName, Ice::Short portId, Ice::Short channelNum)
{
	char buf[512];
	int bufSize = 0;
	char* p = buf;
	bufSize += snprintf(p + bufSize, sizeof(buf) - bufSize -2, "%s", deviceName.c_str());

	if (portId <0)
		return buf;

	bufSize += snprintf(p + bufSize, sizeof(buf) - bufSize -2, "/%d", portId);

	if (channelNum <0)
		return buf;

	bufSize += snprintf(p + bufSize, sizeof(buf) - bufSize -2, ".%d", channelNum);
	buf[bufSize] = 0;

	return buf;
}

EdgeRMImpl::EdgeRMImpl (EdgeRMEnv& env)
: _env(env), _state(TianShanIce::stNotProvisioned)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "add the interface \"EdgeRM\" on to the adapter \"%s\""), _env._adapter->getName().c_str());
	_localId = _env._adapter->getCommunicator()->stringToIdentity(INTERFACE_NAME_EdgeRM);
	_env._adapter->ZQADAPTER_ADD(_adapter->getCommunicator(), this, INTERFACE_NAME_EdgeRM);

	_state = TianShanIce::stProvisioned;

	Ice::Long stampStart = ZQTianShan::now();
	try	
	{
		::Freeze::EvictorIteratorPtr itObjId;

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "initialization: activating the device records from DB"));
		for (itObjId = _env._eEdgeDevice->getIterator("", _env._deviceEvictorSize); itObjId && itObjId->hasNext(); )
		{
			Ice::Identity objId = itObjId->next();
			try {
				TianShanIce::EdgeResource::EdgeDeviceExPrx prx = IdentityToObjEnv(_env, EdgeDeviceEx, objId);
				prx->OnRestore();
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate device[%s] exception[%s] %s"), objId.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate device[%s] exception[%s]"), objId.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate device[%s] unknown exception"), objId.name.c_str());
			}
		}

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "initialization: activating the channel records from DB"));
		for (itObjId = _env._eEdgeChannel->getIterator("", _env._channelEvictorSize); itObjId && itObjId->hasNext(); )
		{
			Ice::Identity objId = itObjId->next();
			try {
				TianShanIce::EdgeResource::EdgeChannelExPrx prx = IdentityToObjEnv(_env, EdgeChannelEx, objId);
				prx->OnRestore();
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate ch[%s] exception[%s] %s"), objId.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate ch[%s] exception[%s]"), objId.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate ch[%s] unknown exception"), objId.name.c_str());
			}
		}

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "initialization: activating the allocation records from DB"));
		for (itObjId = _env._eAllocation->getIterator("", _env._allocationEvictorSize); itObjId && itObjId->hasNext(); )
		{
			Ice::Identity objId = itObjId->next();
			try {
				TianShanIce::EdgeResource::AllocationExPrx prx = IdentityToObjEnv(_env, AllocationEx, objId);
				prx->OnRestore();
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate alloc[%s] exception[%s] %s"), objId.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate alloc[%s] exception[%s]"), objId.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "initialization: activate alloc[%s] unknown exception"), objId.name.c_str());
			}
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRM, "activate DB records caught exception[%s] %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRM, "activate DB records caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRM, "activate DB records caught unknown exception"));
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "initialization: activate DB records took %lldmsec, entering state[InService]"), now()-stampStart);
	_env._watchDog.watch(_localId, 0); // put self under watchDog's monitoring
	_state = TianShanIce::stInService;
}

EdgeRMImpl::~EdgeRMImpl()
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMEnv, "~EdgeRMEnv() removing the interface \"%s\" from the adapter \"%s\""), INTERFACE_NAME_EdgeRM, _env._adapter->getName().c_str());
	_env._adapter->remove(_localId);
	_localId.name = "";
}

std::string EdgeRMImpl::getAdminUri(const Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl getAdminUri()")
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, OBJEXPFMT(EdgeRM, 101, __MSGLOC__ "TODO: impl here"));
	return ""; // dummy statement to avoid compiler error
}

TianShanIce::State EdgeRMImpl::getState(const Ice::Current& c)
{
	return _state;
}

void EdgeRMImpl::importDevice_async(const TianShanIce::EdgeResource::AMD_EdgeRM_importDevicePtr& amdCB, const std::string& name, const std::string& deviceZone, const std::string& xmlDefFile, bool bCompress, const TianShanIce::Properties& props, const Ice::Current& c)
{
	try {
		(new ImportDeviceCmd(_env, amdCB, name, deviceZone, xmlDefFile, bCompress, props))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"importDevice_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to generate ImportDeviceCmd"));
	}
}

void EdgeRMImpl::importDeviceXML_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDeviceXMLPtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::TianShanIce::Properties& props,  const std::string& xmlBody,const ::Ice::Current& c)
{
	try {
		(new ImportDeviceCmd(_env, amdCB, name, deviceZone, props, xmlBody))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"importDevice_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to generate ImportDeviceCmd"));
	}
}

TianShanIce::EdgeResource::EdgeChannelExPrx EdgeRMImpl::openChannel(const std::string& chName, const Ice::Current& c)
{
	return _env._openChannel(chName);
}

// impls of EdgeResouceManager
TianShanIce::EdgeResource::AllocationPrx EdgeRMImpl::createAllocation(const TianShanIce::SRM::ResourceMap& resRequirement, Ice::Int TTL, const TianShanIce::EdgeResource::AllocationOwnerPrx& owner, const std::string& ownerContextKey, const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "enter() createAllocation()"));
	Ice::Long lstart  = ZQTianShan::now();
	TianShanIce::EdgeResource::AllocationPrx allocPrx;
    AllocationImpl::Ptr alloc;
	try {
		alloc  = new AllocationImpl(_env);
		alloc->ident              = AllocationImpl::generateIdent();
		alloc->state			  = TianShanIce::stNotProvisioned;
		alloc->resources          = resRequirement;
		alloc->owner              = owner;
		alloc->ownerKey           = ownerContextKey;
		alloc->udpPort            = -1;
		alloc->programNumber      = -1;
		alloc->maxJitter          = -1;
		alloc->sourceIP           = "";
		alloc->bandwidth          = -1;
		alloc->stampCreated       = now();
		alloc->stampCommitted     = 0;
		alloc->expiration         = alloc->stampCreated + TTL;
		alloc->retrytimes         = 0;
		alloc->sessionGroup		  = "";
		alloc->onDemandSessionId  = "";
		alloc->qamSessionId		  = "";
		alloc->qamSessGroup       = "";

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "createAllocation() adding alloc[%s] into the database"), alloc->ident.name.c_str());
		try
		{	
			Ice::Long lstart  = ZQTianShan::now();
			_env._eAllocation->add(alloc, alloc->ident);

			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "createAllocation() adding alloc[%s] into the database took %d ms "), 
				alloc->ident.name.c_str(), ZQTianShan::now() - lstart);

			lstart  = ZQTianShan::now();
			allocPrx = IdentityToObjEnv(_env, Allocation, alloc->ident);

			envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "alloc[%s] IdentityToObjEnv took %d ms "), 
				alloc->ident.name.c_str(), ZQTianShan::now() - lstart);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(Allocation,"Fail to add allocation [%s] object to DB caught '%s:%s'"), 
				alloc->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			return NULL;
		}
		catch (const Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(Allocation,"Fail to add allocation [%s] object to DB caught '%s'"), 
				alloc->ident.name.c_str(), ex.ice_name().c_str());
			return NULL;
		}
		if (allocPrx)
		{
#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			evntlog(EventFMT(_env._netId.c_str(), Allocation, Created, 0, "alloc[%s] is created"), alloc->ident.name.c_str());

			AllocStateNotProvisioned(_env, *alloc).enter();
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "createAllocation() caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "createAllocation() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "createAllocation() caught unknown exception"));
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "alloc[%s] leave createAllocation() took %d ms"), alloc->ident.name.c_str(), ZQTianShan::now() - lstart);

	return allocPrx;
}

TianShanIce::EdgeResource::AllocationPrx EdgeRMImpl::openAllocation(const std::string& allocId, const Ice::Current& c)
{
	Ice::Identity identAlloc;
	identAlloc.category = DBFILENAME_Allocation;
	identAlloc.name = allocId;

	TianShanIce::EdgeResource::AllocationPrx prx = NULL;

	try
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "openAllocation() opening allocation by Id[%s]"), identAlloc.name.c_str());
		// must not use uncheckedCast here
		prx = IdentityToObjEnv2(_env, Allocation, identAlloc);
	}
	catch (const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "openAllocation() failed to open allocation by fullname[%s]: caught exception[%s]"), identAlloc.name.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "openAllocation() failed to open allocation by fullname[%s]: caught unknown exception"), identAlloc.name.c_str());
	}

	return prx;
}

void EdgeRMImpl::listAllocations_async(const TianShanIce::EdgeResource::AMD_EdgeResouceManager_listAllocationsPtr& amdCB, const std::string& deviceName, Ice::Short portId, Ice::Short chNum, const TianShanIce::StrValues& expectedMetaData, const Ice::Current& c) const
{
	try {
		(new ListAllocationsCmd(_env, amdCB, deviceName, portId, chNum, expectedMetaData))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"listAllocations_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to generate listAllocationsCmd"));
	}
}

void EdgeRMImpl::exportDevices_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_exportDevicesPtr& amdCB, const ::std::string& deviceName, const std::string& xmlDefFIle, const ::Ice::Current& c) const
{
	try {
		(new ExportDevicesCmd(_env, amdCB, deviceName, xmlDefFIle))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"Export Deivce failed to export "));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to generate ExportDeviceCmd"));
	}
}

void EdgeRMImpl::exportDeviceXML_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_exportDeviceXMLPtr& amdCB, const ::std::string& deviceName, const ::Ice::Current&) const
{
	try {
		(new ExportDeviceXMLBodyCmd(_env, amdCB, deviceName))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"Export Deivce failed to export "));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to generate ExportDeviceCmd"));
	}
}

void EdgeRMImpl::exportAllocations_async(const TianShanIce::EdgeResource::AMD_EdgeRM_exportAllocationsPtr& amdCB, const std::string& deviceName, const std::string& since, const Ice::Current& c) const
{
	try {
		(new exportAllocationsCmd(_env, amdCB, deviceName, since))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"exportAllocations_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 503, "failed to generate exportAllocationsCmd"));
	}
}

void EdgeRMImpl::forceBackupMode(const ::std::string& netId, ::Ice::Int mode, const Ice::Current& c)
{
  if(_env._pErmInstanceSyncer)
  {
	  _env._pErmInstanceSyncer->changeWorkForMode(netId, mode);
  }
}

TianShanIce::EdgeResource::EdgeDeviceInfos EdgeRMImpl::listDevices(const TianShanIce::StrValues& expectedMetaData, const Ice::Current& c) const
{
	TianShanIce::EdgeResource::EdgeDeviceInfos infos;

	IdentCollection Idents;
	try	{
			{
				::Freeze::EvictorIteratorPtr itptr = _env._eEdgeDevice->getIterator("", _env._deviceEvictorSize);
				while (itptr && itptr->hasNext())
					Idents.push_back(itptr->next());
			}

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "listDevices() %d devices found"), Idents.size());

		// build up the content info collection based on the search result
		for (IdentCollection::iterator it= Idents.begin(); it < Idents.end(); it++)
		{
			try {
				TianShanIce::EdgeResource::EdgeDevicePrx device = IdentityToObjEnv(_env, EdgeDevice, *it);
				TianShanIce::ObjectInfo info = device->getInfo(expectedMetaData);
				infos.push_back(info);
			}
			catch (...) {}
		}

		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "listDevices() get information of %d devices as the result"), infos.size());
	}
	catch(const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "listDevices() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "listDevices() caught unknown exception"));
	}

	return infos;
}

void EdgeRMImpl::addDevice_async(const TianShanIce::EdgeResource::AMD_EdgeResouceManager_addDevicePtr& amdCB, const std::string& name, const std::string& deviceZone, const std::string& vendor, const std::string& model, const std::string& adminUrl, const std::string& tftpUrl, const Ice::Current& c)
{
	try {
		(new AddDeviceCmd(_env, amdCB, name, deviceZone, vendor, model, adminUrl, tftpUrl))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"addDevice_async() failed to add device"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to add device"));
	}
}

TianShanIce::EdgeResource::EdgeDevicePrx EdgeRMImpl::openDevice(const std::string& name, const Ice::Current& c)
{
	if (name.empty())
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 2001, "openDevice() illegal device name[%s] to open"), name.c_str());

	Ice::Identity identDevice;
	identDevice.name = name;
	identDevice.category = DBFILENAME_EdgeDevice;

	TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
	try
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "openDevice() device[%s] openning the db record"), identDevice.name.c_str());
		// must not use checkedCast here
		devicePrx = IdentityToObjEnv2(_env, EdgeDevice, identDevice);
	}
	catch(const Ice::ObjectNotExistException&)
	{
		return NULL;
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, OBJEXPFMT(EdgeRM, 201, "openDevice() device[%s] caught exception[%s]"), identDevice.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, OBJEXPFMT(EdgeRM, 201, "openDevice() device[%s] caught unknown exception"), identDevice.name.c_str());
	}

	return devicePrx;
}

// impls of ReplicaSubscriber
void EdgeRMImpl::queryReplicas_async(const TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const std::string& category, const std::string& groupId, bool localOnly, const Ice::Current& c)
{
	try {
		(new QueryReplicaCmd(_env, amdCB, category, groupId, localOnly))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"queryReplicas_async() failed to query replicas"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to query replicas"));
	}
}

// impls of ReplicaQuery
void EdgeRMImpl::updateReplica_async(const TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const TianShanIce::Replicas& replicaInfos, const Ice::Current& c)
{
	try {
		(new UpdateReplicaCmd(_env, amdCB, replicaInfos))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"updateReplica_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to generate UpdateReplicaCmd"));
	}
}
 
// impls of TianShanUtils::TimeoutObj
void EdgeRMImpl::OnTimer(const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "OnTimer()"));

#pragma message ( __MSGLOC__ "TODO: add protection logic in EdgeRMImpl::OnTimer()")

	_env._watchDog.watch(_localId, MAX_INSTANCE_IDLE);
}
bool EdgeRMImpl::listPortsbyRouteName(const std::string routeName, IdentCollection& identPorts)
{	
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "enter listPortsbyRouteName() RouteName[%s]"), 
		routeName.c_str());
	try
	{
		::ZQ::common::MutexGuard gd(_env._lkIdxRouteName);
		EdgeRMEnv::RouteIndex::iterator it = _env._idxRouteName.find(routeName);
		EdgeRMEnv::RouteIndex::iterator itLast = _env._idxRouteName.upper_bound(routeName);
		if(it!= _env._idxRouteName.end())
		{
			for (; it != itLast; it++)
			{
				Ice::Identity ident;
				std::string deviceName;
				Ice::Short portId, channelname;
				//seperator the devicename portId channelname from Channel Identity
				chopForHiberarchy(it->second.name, deviceName, portId, channelname);

				//build port Identity <deviceName>/<portId>
				char strTemp[256]="\0";
				sprintf(strTemp,"%s/%d\0", deviceName.c_str(), portId);
				ident.name = strTemp;
				ident.category = DBFILENAME_Port;
				//find port from identPorts, if not exist,add it to identPorts;
				IdentCollection::iterator itor = std::find(identPorts.begin(), identPorts.end(), ident);
				if(itor == identPorts.end())
				{
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "ServiceGroup[%s] port[%s]"), 
						routeName.c_str(), ident.name.c_str());
					identPorts.push_back(ident);
				}
			}
		}
	}
	catch(Ice::Exception&ex)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "listPortsbyServiceGp() caught ice exception(%s)"),ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM, "listPortsbyServiceGp() caught unknow exception(%d)"),SYS::getLastErr());
		return false;
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "leave listPortsbyRouteName() RouteName[%s] PortSize[%d]"), routeName.c_str(), identPorts.size());

	return true;
}
TianShanIce::EdgeResource::EdgePortInfos EdgeRMImpl::findRFPortsByRouteName(const ::std::string& routeName, const Ice::Current & c)
{
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "enter findRFPortsByRouteName() RouteName[%s]"), routeName.c_str());

	TianShanIce::EdgeResource::EdgePortInfos rfportInfos;
	IdentCollection identPorts;
	typedef std::map < std::string, TianShanIce::IValues >Devices;
	Devices devices;

	listPortsbyRouteName(routeName, identPorts);

	for(IdentCollection::iterator itor = identPorts.begin(); itor != identPorts.end(); itor++)
	{
		Ice::Identity ident = *itor;
		std::string devicename; 
		Ice::Short  portId, channelname;
		chopForHiberarchy(itor->name, devicename, portId, channelname);
		Devices::iterator itorDevice = devices.find(devicename);
		if(itorDevice != devices.end())
		{
			itorDevice->second.push_back(portId);
		}
		else
		{
			TianShanIce::IValues ports;
			ports.push_back(portId);
			MAPSET(Devices, devices, devicename, ports);
		}	
	}

	for(Devices::iterator itorDevice = devices.begin(); itorDevice != devices.end(); itorDevice++)
	{
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "findRFPortsByRouteName() open device[%s]"), 
			(*itorDevice).first.c_str());

		try
		{
			Ice::Identity identDevice;
			identDevice.name = itorDevice->first;
			identDevice.category = DBFILENAME_EdgeDevice;
			TianShanIce::EdgeResource::EdgeDeviceExPrx  edgeDeviceExprx = NULL;
			edgeDeviceExprx = IdentityToObjEnv2(_env, EdgeDeviceEx, identDevice);
			TianShanIce::EdgeResource::EdgePortInfos edgeports = edgeDeviceExprx->listEdgePorts();

			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "findRFPortsByRouteName() open device[%s],portsize[%d]"), 
				(*itorDevice).first.c_str(), edgeports.size());

			for(TianShanIce::IValues::iterator itorPort = itorDevice->second.begin(); 
				itorPort != itorDevice->second.end(); itorPort++)
			{
				for(TianShanIce::EdgeResource::EdgePortInfos::iterator itorportinfos = edgeports.begin();
					itorportinfos != edgeports.end(); itorportinfos++)
				{
					if(itorportinfos->Id == *itorPort)
					{
						envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "device[%s],port[%d]"), 
							(*itorDevice).first.c_str(), *itorPort);
						rfportInfos.push_back(*itorportinfos);
						break;
					}
				}
			}
		}
		catch(Ice::Exception&ex)
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRM, "findRFPortsByRouteName RouteName[%s], devicename[%s] caught (%s)"), 
				routeName.c_str(), (*itorDevice).first.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRM, "findRFPortsByRouteName RouteName[%s], devicename[%s] caught unknown exception(%d)"), 
				routeName.c_str(), (*itorDevice).first.c_str(), SYS::getLastErr());
		}
	}
 
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "leave findRFPortsByRouteName() RouteName[%s]"), routeName.c_str());
	return rfportInfos;
}

TianShanIce::EdgeResource::ObjectInfos EdgeRMImpl::listRouteNames(const Ice::Current & c)
{
	TianShanIce::EdgeResource::ObjectInfos objectsinfos;
	TianShanIce::StrValues routeNames;
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "enter() listRouteName"));
	try
	{
		::ZQ::common::MutexGuard gd(_env._lkIdxRouteName);
		for (EdgeRMEnv::RouteIndex::iterator it = _env._idxRouteName.begin(); it != _env._idxRouteName.end(); it++)
		{
			TianShanIce::StrValues::iterator itor = std::find(routeNames.begin(), routeNames.end(), it->first);
			if(itor != routeNames.end())
				continue;	
			IdentCollection identPorts;
			listPortsbyRouteName(it->first, identPorts);

			TianShanIce::ObjectInfo objectinfo;

			//std::string sg = Int2Str(it->first);
			objectinfo.ident.name = it->first;
			objectinfo.ident.category = DBFILENAME_ServiceGroup;
			objectinfo.props["RFPortCount"] = Int2Str(identPorts.size());
			objectsinfos.push_back(objectinfo);
			routeNames.push_back(it->first);  
		}
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRM, "listRouteNames caught unknown exception(%d)"), 
			SYS::getLastErr());
	}
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "listRouteNames size[%d]"), objectsinfos.size());


	return objectsinfos;
}

void EdgeRMImpl::diffAllocations_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_diffAllocationsPtr& amdCB, const ::std::string& deviceName, const ::TianShanIce::StrValues& allocIds, const ::Ice::Current& c) const
{
	try {
		(new DiffAllocCmd(_env, amdCB, deviceName, allocIds))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM," diffAllocations_async() failed to get the diffAllocations "));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeRM", 502, "failed to generate DiffAllocCmd"));
	}
}

// -----------------------------
// class EdgeDeviceImpl
// -----------------------------
EdgeDeviceImpl::EdgeDeviceImpl(EdgeRMEnv& env)
:_env(env)
{
}

EdgeDeviceImpl::~EdgeDeviceImpl()
{
}

/*
void EdgeDeviceImpl::_updateServiceGroupIndex(Ice::Short portId, const TianShanIce::IValues& newServiceGroups)
{
	TianShanIce::EdgeResource::EdgePortMap::iterator itPort = edgePorts.find(portId);
	if (edgePorts.end() == itPort)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(EdgeDevice, 201, "device[%s] updateServiceGroupMap() invalid portId[%d]"), ident.name.c_str(), portId);

	// step 1. backup the old servicegroup bound on this portId
	TianShanIce::IValues oldSvcGrpsOfThePort = itPort->second.serviceGroups;
	itPort->second.serviceGroups = newServiceGroups;
	std::sort(itPort->second.serviceGroups.begin(), itPort->second.serviceGroups.end());
	std::string oldSvcGrpStr("old serviceGroup["), newSvcGrpStr("new serviceGroup[");
	TianShanIce::IValues::iterator itOldSg = oldSvcGrpsOfThePort.begin();
	TianShanIce::IValues::iterator itNewSg = itPort->second.serviceGroups.begin();
	{
		char buf[64];
		for (; itOldSg < oldSvcGrpsOfThePort.end(); itOldSg++)
		{
			snprintf(buf, sizeof(buf)-2, "%d,", *itOldSg);
			oldSvcGrpStr += buf;
		}
		oldSvcGrpStr += "]";

		for (; itNewSg < itPort->second.serviceGroups.end(); itNewSg++)
		{
			snprintf(buf, sizeof(buf)-2, "%d,", *itNewSg);
			newSvcGrpStr += buf;
		}
		newSvcGrpStr += "]";
	}
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeDevice, "device[%s] updateServiceGroupMap() updating port[%d] %s => %s"), ident.name.c_str(), portId, oldSvcGrpStr.c_str(), newSvcGrpStr.c_str());

	// step 2. enumerate all the channels of the portId
	const std::string portHirechy = EdgeRMImpl::formatHiberarchy(ident.name, portId, -1) + ".";
	typedef std::set<Ice::Identity > IdentSet;
	IdentSet identChOfPort;

	{
		IdentCollection tmp = _env._idxChannelOfDevice->find(ident);
		for (IdentCollection::iterator it = tmp.begin(); it < tmp.end(); it++)
		{
			if (0 != it->name.compare(0, portHirechy.length(), portHirechy))
				continue;

			identChOfPort.insert(*it);
		}
	}

	// step 3. update the channels of service group
	itOldSg = oldSvcGrpsOfThePort.begin();
	itNewSg = itPort->second.serviceGroups.begin();

	ZQ::common::MutexGuard(_env._lkIdxServiceGroup);
	while (itOldSg < oldSvcGrpsOfThePort.end())
	{
		Ice::Int workSvcGrp =-1;
		bool bFillChannelOfPort = false;

		// step 3.1 determ the action need to take
		if (itNewSg >= itPort->second.serviceGroups.end() || *itOldSg < *itNewSg )
		{
			// servicegroups only in old configuration -> to clean up
			workSvcGrp = *itOldSg;
			itOldSg ++;
			bFillChannelOfPort = false;
		}
		else if (itNewSg < itPort->second.serviceGroups.end() && *itOldSg == *itNewSg)
		{
			// servicegroups to keep
			workSvcGrp = *itOldSg;
			itOldSg ++;
			itNewSg++;
			bFillChannelOfPort = true;
		}
		else if (itNewSg < itPort->second.serviceGroups.end() && *itOldSg > *itNewSg)
		{
			// servicegroups newly added
			workSvcGrp = *itNewSg;
			itNewSg++;
			bFillChannelOfPort = true;
		}

		if (workSvcGrp <0)
			continue;

		IdentSet channelOfSvcGroup;
		EdgeRMEnv::ServiceGroupIndex::iterator itSvcMapStart = _env._idxServiceGroup.find(workSvcGrp);//find: 返回第一个对象的位置
		EdgeRMEnv::ServiceGroupIndex::iterator itSvcMapEnd   = _env._idxServiceGroup.upper_bound(workSvcGrp);//upper_bound: 返回>对象的第一个位置

		if(itSvcMapStart != _env._idxServiceGroup.end())/// workSvcGrp = 2000, 而_idxServiceGroup为(3000, 4000)
		{
			// step 3.2 backup the channel of other ports
			for (EdgeRMEnv::ServiceGroupIndex::iterator itSvcMap = itSvcMapStart; itSvcMap != itSvcMapEnd; itSvcMap++)
			{
				// filter out the channels of the port, keep those NOT of the port
				if (0 != itSvcMap->second.name.compare(0, portHirechy.length(), portHirechy))
					channelOfSvcGroup.insert(itSvcMap->second);
			}
		}

		// step 3.3 fill the channels of this port if necessary
		if (bFillChannelOfPort)
		{
			for (IdentSet::iterator itCh= identChOfPort.begin(); itCh != identChOfPort.end(); itCh ++)
				channelOfSvcGroup.insert(*itCh);
		}

		// replace the channels of port in the index
		if(itSvcMapStart != _env._idxServiceGroup.end())
			_env._idxServiceGroup.erase(itSvcMapStart, itSvcMapEnd);
		for (IdentSet::iterator itChannel = channelOfSvcGroup.begin(); itChannel != channelOfSvcGroup.end(); itChannel++)
			_env._idxServiceGroup.insert(EdgeRMEnv::ServiceGroupIndex::value_type(workSvcGrp, *itChannel));
	}

	// step 4, for the newly linked service groups, directly fill them into the index
	for (; itNewSg < itPort->second.serviceGroups.end(); itNewSg++)
	{
		for (IdentSet::iterator itChannel = identChOfPort.begin(); itChannel != identChOfPort.end(); itChannel++)
			_env._idxServiceGroup.insert(EdgeRMEnv::ServiceGroupIndex::value_type(*itNewSg, *itChannel));
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDevice, "device[%s] updateServiceGroupMap() updated port[%d] %s => %s"), ident.name.c_str(), portId, oldSvcGrpStr.c_str(), newSvcGrpStr.c_str());
	{
#if 0

		ZQ::common::MutexGuard(_env._lkIdxServiceGroup);
		EdgeRMEnv::ServiceGroupIndex::iterator itor = _env._idxServiceGroup.begin();
		while(itor != _env._idxServiceGroup.end())
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDevice, "****** [%d][%s]   *******"),itor->first,  itor->second.name.c_str());
			itor++;
		}

#endif
	}
}

*/
TianShanIce::EdgeResource::EdgeChannelInfos EdgeDeviceImpl::getChFreq(Ice::Short portId)
{
	TianShanIce::EdgeResource::EdgeChannelInfos infos;

	try	{
		IdentCollection channelIdents; 
		try
		{	
			::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
			DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(ident.name);
			if(itorDC != _env._devicechannels.end())
				channelIdents = itorDC->second;
		}
		catch (...){
		}

		//get channel information from each channel
		for (IdentCollection::iterator it= channelIdents.begin(); it != channelIdents.end(); it++)
		{
			try
			{
				Ice::Identity ident = *it;
				::TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = IdentityToObjEnv2(_env, EdgeChannelEx, ident);
				std::string outDeviceName; 
				::Ice::Short outRFPort;
				::Ice::Short outChNum;
				channelPrx->getHiberarchy(outDeviceName, outRFPort, outChNum);
				if (outRFPort != portId)
				{
					continue;
				}

				TianShanIce::StrValues expectedMetaData;
				expectedMetaData.push_back(SYS_PROP(FreqRF));
				EdgeChannelInfo info = channelPrx->getInfo(expectedMetaData);
				infos.push_back(info);
			}
			catch(const ::Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeDeviceImpl, "getChFreq() list channel info by ident(%s) catch ice exception[%s]"), it->name.c_str(), ex.what());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeDeviceImpl, "getChFreq() get channel info by ident(%s) catch unknow exception"), it->name.c_str());
			}		
		}
	}
	catch(const ::Ice::Exception& ex)
	{
	}
	catch(...)
	{
	}

	return infos;
}
void EdgeDeviceImpl::addChannel(::Ice::Short portId, ::Ice::Int chId, const ::TianShanIce::Properties& attributes, const ::Ice::Current& c)
{
  RLock sync(*this);
  ///step 1, find RFPort in Device, if not exists, throw exception
  TianShanIce::EdgeResource::EdgePortMap::iterator it=edgePorts.find(portId); 
  if (edgePorts.end() == it)
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 320, "the port[%d] not exists in Device"), portId);
  }
  TianShanIce::EdgeResource::EdgePortEx& edgePortEx = it->second;

  ///step2, find channel in Device RFPort, if exists, throw exception
  Ice::Identity identCh;
  identCh.name = ::ZQTianShan::EdgeRM::EdgeRMImpl::formatHiberarchy(ident.name, portId, chId);
  identCh.category = DBFILENAME_EdgeChannel;
  if(_env._eEdgeChannel->hasObject(identCh) != NULL)
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 321, "the channel[%s] exists in Device"), identCh.name.c_str());
  }
  ///step3, find freqRF in RFPort, if exists ,throw exception

  TianShanIce::Properties::const_iterator itorAttr =attributes.find(SYS_PROP(FreqRF));
  if(itorAttr == attributes.end())
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 322, "missing frequency attritbute in the channel[%s]"), identCh.name.c_str());
  }
  const std::string& frequency = itorAttr->second;
  TianShanIce::EdgeResource::EdgeChannelInfos infos = getChFreq(portId);

  for(int i = 0; i < infos.size(); i++)
  {
	  EdgeChannelInfo& info =  infos[i];
	  if(info.props.find(SYS_PROP(FreqRF)) == info.props.end())
	  {
		  envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeDeviceImpl, "addChannel() no frequency attritbute in channel[%s]"),info.ident.name.c_str());
		  continue;
	  }

	  const std::string& chFreq = info.props[SYS_PROP(FreqRF)];
	  if( stricmp(frequency.c_str(), chFreq.c_str()) == 0)
	  {
		  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 323, "same Frequency[%s]  in the channel[%s]"), frequency.c_str(), info.ident.name.c_str());
	  }
  }

  ::ZQTianShan::EdgeRM::EdgeChannelImpl::Ptr edgeChannelPtr;
  edgeChannelPtr = NULL;//destroy last EdgeChannel pointer
  edgeChannelPtr = new ::ZQTianShan::EdgeRM::EdgeChannelImpl(_env);
  edgeChannelPtr->ident = identCh;
  edgeChannelPtr->identDevice = ident;
  edgeChannelPtr->enabled = true;
  edgeChannelPtr->ePort = edgePortEx.portAttr;
  edgeChannelPtr->freqRF	= atol(frequency.c_str());
  edgeChannelPtr->stampLastUpdated = ::ZQTianShan::now();


  itorAttr =attributes.find("symbolRate");
  if(itorAttr == attributes.end())
  {
	ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 324, "missing symbolRate attritbute in the channel[%s]"), identCh.name.c_str());
  }
  ::Ice::Int symbolRate = atoi(itorAttr->second.c_str());
  ::TianShanIce::Variant value;
  value.type = ::TianShanIce::vtInts;
  value.bRange = false;
  value.ints.clear();
  value.ints.push_back(symbolRate);
  edgeChannelPtr->ePort.resAtscModulationMode.resourceData["symbolRate"] = value;

  edgeChannelPtr->TSID = 200;
  //init with reserved value
  edgeChannelPtr->NITPID = 6;
  edgeChannelPtr->deviceState = ::TianShanIce::stInService;

  edgeChannelPtr->provPort.enabled = 1;
  edgeChannelPtr->provPort.inbandMarker = "type=4;pidType=A;pidValue=01EE;dataType=T;insertDuration=10000;data=4002003030";
  edgeChannelPtr->provPort.reportTrafficMismatch = 0;
  edgeChannelPtr->provPort.jitterBuffer = 0;

  itorAttr =attributes.find(SYS_PROP(StartUDPPort));
  if(itorAttr == attributes.end())
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 324, "missing StartUDPPort attritbute in the channel[%s]"), identCh.name.c_str());
  }
  edgeChannelPtr->startUDPPort		=  atoi(itorAttr->second.c_str());

  itorAttr =attributes.find(SYS_PROP(UdpPortStepByPn));
  if(itorAttr == attributes.end())
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 326, "missing UdpPortStepByPn attritbute in the channel[%s]"), identCh.name.c_str());
  }
  edgeChannelPtr->udpPortStepByPn	=  atoi(itorAttr->second.c_str());

  itorAttr =attributes.find(SYS_PROP(StartProgramNumber));
  if(itorAttr == attributes.end())
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 327, "missing StartProgramNumber attritbute in the channel[%s]"), identCh.name.c_str());
  }
  edgeChannelPtr->startProgramNumber=  atoi(itorAttr->second.c_str());

  itorAttr =attributes.find(SYS_PROP(MaxSessions));
  if(itorAttr == attributes.end())
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 328, "missing MaxSessions attritbute in the channel[%s]"), identCh.name.c_str());
  }
  edgeChannelPtr->maxSessions		= atoi(itorAttr->second.c_str());

  itorAttr =attributes.find(SYS_PROP(LowBandwidthUtilization));
  if(itorAttr == attributes.end())
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 329, "missing LowBandwidthUtilization attritbute in the channel[%s]"), identCh.name.c_str());
  }
  edgeChannelPtr->lowBandwidthUtilization =  atoi(itorAttr->second.c_str()) * 1000;

  itorAttr =attributes.find(SYS_PROP(HighBandwidthUtilization));
  if(itorAttr == attributes.end())
  {
	  ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 330, "missing HighBandwidthUtilization attritbute in the channel[%s]"), identCh.name.c_str());
  }
  edgeChannelPtr->highBandwidthUtilization = (Ice::Long)atoi(itorAttr->second.c_str()) * 1000;

  edgeChannelPtr->intervalPAT = 40;
  edgeChannelPtr->intervalPMT = 400;

  _env._eEdgeChannel->add(edgeChannelPtr, edgeChannelPtr->ident);
  _env.addChannelToDevice(ident.name, edgeChannelPtr->ident);

  envlog(ZQ::common::Log::L_INFO, OBJLOGFMT(EdgeDeviceImpl, "addChannel() %s channel add"), edgeChannelPtr->ident.name.c_str());
  edgeChannelPtr = NULL;
}

// impls of EdgeDeviceEx
void EdgeDeviceImpl::addEdgePort(const TianShanIce::EdgeResource::EdgePort& port, const Ice::Current& c)
{
	WLock sync(*this);
	//ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, OBJEXPFMT(EdgeRM, 201, __MSGLOC__ "TODO: impl here"));
//#pragma message ( __MSGLOC__ "TODO: impl addEdgePort()")
	TianShanIce::EdgeResource::EdgePortEx edgePort;
	edgePort.portAttr = port;
	Ice::Short portId = port.Id;

	TianShanIce::EdgeResource::EdgePortMap::iterator it=edgePorts.find(portId); 
	if (edgePorts.end() == it)
	{
		//add to Edge port map
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDeviceImpl, "addEdgePort() %d ports add"), portId);
		edgePorts[portId] = edgePort;
	
	}
	else //port already in use, throw exception	
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 307, "the port[%d] already exists in Device"), portId);
	}
	return;
}
TianShanIce::EdgeResource::EdgePort EdgeDeviceImpl::getEdgePort(::Ice::Short portId, const ::Ice::Current& c)
{
	WLock sync(*this);
	TianShanIce::EdgeResource::EdgePort edgePort;
	TianShanIce::EdgeResource::EdgePortMap::iterator it=edgePorts.find(portId); 
	if (edgePorts.end() == it)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 308, "the port[%d] not exists in Device"), portId);
	}
	else
	{
		edgePort = it->second.portAttr;
	}
	return edgePort;
}

void EdgeDeviceImpl::updateAttributes(const ::TianShanIce::Properties& expectedMetaData, const ::Ice::Current& /* = ::Ice::Current */)
{
	TianShanIce::Properties::const_iterator iter = expectedMetaData.begin();
	for(iter; iter != expectedMetaData.end(); iter++)
	{
/*		if(iter->first == SYS_PROP(Name))
		{
			ident.name = iter->second;
		}
		else
*/

		if(iter->first == SYS_PROP(Zone))
		{
			deviceZone = iter->second;
		}
		else if(iter->first == SYS_PROP(Type))
		{
			type = iter->second;
		}
		else if(iter->first == SYS_PROP(Vendor))
		{
			vendor = iter->second;
		}
		else if(iter->first == SYS_PROP(Model))
		{
			model = iter->second;
		}
		else if(iter->first == SYS_PROP(Desc))
		{
			desc = iter->second;
		}
		else if(iter->first == SYS_PROP(Tftp))
		{
			tftpUrl = iter->second;
		}
		else if (iter->first == SYS_PROP(AdminUrl))
		{
			adminUrl = iter->second;
		}
	}
}

void EdgeDeviceImpl::removeEdgePort(Ice::Short portId, const Ice::Current& c)
{
	WLock sync(*this);
	TianShanIce::EdgeResource::EdgePortMap::iterator it=edgePorts.find(portId); 
	if (edgePorts.end() == it)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeRM, 308, "the port[%d] not exists in Device"), portId);
		return;
	}

	/// step 1. enumerate all the channels of the portId
	const std::string portHirechy = EdgeRMImpl::formatHiberarchy(ident.name, portId, -1) + ".";
	typedef std::set<Ice::Identity > IdentSet;
	IdentCollection channelIds;
	IdentCollection identChOfOtherPort;
	{
		//IdentCollection tmp = _env._idxChannelOfDevice->find(ident);
		IdentCollection tmp;
		try
		{	
			::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
			DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(ident.name);
			if(itorDC != _env._devicechannels.end())
				tmp = itorDC->second;
		}
		catch (...){	
		}

		for (IdentCollection::iterator it = tmp.begin(); it < tmp.end(); it++)
		{
			if (0 != it->name.compare(0, portHirechy.length(), portHirechy))
			{
				identChOfOtherPort.push_back(*it);
				continue;
			}
			else
			{
				channelIds.push_back(*it);
			}
		}
	}
    ///step 2. check all channel of port
	if (channelIds.size() > 0)
	{	
		envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeDeviceImpl, "removeEdgePort() Edge Device(deviceId: %s) to check channel info"), ident.name.c_str());

		//check allocations
		for (IdentCollection::iterator it= channelIds.begin(); it < channelIds.end(); it++)
		{
			try
			{
				TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = IdentityToObjEnv2(_env, EdgeChannelEx, *it);
				TianShanIce::EdgeResource::Allocations allocations = channelPrx->getAllocations();
				if (allocations.size() > 0)
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeDeviceImpl, 302, "active allocation in channel(%s)"), (*it).name.c_str());
				}
			}
			catch(const Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, OBJLOGFMT(EdgeDeviceImpl, "removeEdgePort() failed to get Allocation for channel(%s) caught ice exception[%s]"),
					it->name.c_str(), ex.ice_name().c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeDeviceImpl, 303, "failed to get Allocation for channel(%s) caught ice exception(%s)"),
					it->name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_ERROR, OBJLOGFMT(EdgeDeviceImpl, "removeEdgePort()failed to get Allocation for channel(%s)caught unknown exception"), 
					it->name.c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeDeviceImpl, 304, "failed to get Allocation for channel(%s)caught unknown exception"),
					it->name.c_str());
			}		
		}
	}

	///step 3. remove service groups Index
	TianShanIce::StrValues NilRouteNames;
	for(TianShanIce::EdgeResource::RoutesMap::iterator itorsgs = it->second.routes.begin();
		itorsgs !=  it->second.routes.end(); itorsgs++)
	{
		NilRouteNames.push_back(itorsgs->first);
	}

	try
	{
		_removeRouteIndex(portId, NilRouteNames); // clean up the service group index about this port
	}
	catch (...){	
	}

	///step 4. remove all channel of this port
	for (IdentCollection::iterator it= channelIds.begin(); it < channelIds.end(); it++)
	{
		try
		{
			_env._eEdgeChannel->remove(*it);    
		}
		catch (const Freeze::DatabaseException& ex)
		{
			envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeDeviceImpl, "removeEdgePort() Fail to remove channel [%s] object from DB caught (%s,%s)"), 
				(*it).name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeDeviceImpl, 305, "Fail to remove channel[%s] object from DB caught (%s,%s)"),
				(*it).name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeDeviceImpl, "removeEdgePort() Fail to remove channel  [%s] object from DB caught (%s)"), 
				(*it).name.c_str(), ex.ice_name().c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeDeviceImpl, 306, "Fail to remove channel[%s] object from DB caught (%s)"),
				(*it).name.c_str(), ex.ice_name().c_str());
		}		
	}
    ///step 5. remove channel of this port from DeviceChannelsMap
	try
	{	
		::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
        MAPSET(DeviceChannelsMap, _env._devicechannels, ident.name, identChOfOtherPort);
	}
	catch (...){	
	}

    ///step 6. remove this port
	edgePorts.erase(it);
}


// impls of EdgeDevice
std::string EdgeDeviceImpl::getName(const Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

TianShanIce::ObjectInfo EdgeDeviceImpl::getInfo(const TianShanIce::StrValues& expectedMetaData, const Ice::Current& c) const
{
	RLock sync(*this);
	EdgeDeviceInfo info;
	info.ident = ident;
	
//#pragma message ( __MSGLOC__ "TODO: fill in the metadata of device here")

	if (expectedMetaData.empty())
		return info;

	//some metadata wanted
#ifndef FINDINMETADATA
#define FINDINMETADATA(_METADATA, _FIELD) find(_METADATA.begin(), _METADATA.end(), _FIELD) != _METADATA.end()

	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Name)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Name),		ident.name);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Zone)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Zone),		deviceZone);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Type)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Type),		type);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Vendor)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Vendor),		vendor);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Model)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Model),		model);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Desc)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Desc),		desc);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Tftp)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Tftp),		tftpUrl);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(AdminUrl)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(AdminUrl),	adminUrl);

#undef FINDINMETADATA
#endif
	return info;
}

TianShanIce::EdgeResource::EdgePortInfos EdgeDeviceImpl::listEdgePorts(const Ice::Current& c) const
{
	RLock sync(*this);
	TianShanIce::EdgeResource::EdgePortInfos infos;
	for(TianShanIce::EdgeResource::EdgePortMap::const_iterator it=edgePorts.begin(); it != edgePorts.end(); it++)
		infos.push_back(it->second.portAttr);

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRM, "listEdgePorts() %d ports found"), infos.size());
	return infos;
}

void EdgeDeviceImpl::listChannels_async(const TianShanIce::EdgeResource::AMD_EdgeDevice_listChannelsPtr& amdCB, Ice::Short portId, const TianShanIce::StrValues& expectedMetaData, bool enabledOnly, const Ice::Current& c) const
{
	try {
		(new ListChannelOfDeviceCmd(_env, amdCB, ident,  portId, expectedMetaData, enabledOnly))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeDevice,"listChannels_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeDevice", 502, "failed to generate ListChannelOfDeviceCmd"));
	}
}

void EdgeDeviceImpl::listAllocationIds_async(const ::TianShanIce::EdgeResource::AMD_EdgeDeviceEx_listAllocationIdsPtr& amdCB, const ::Ice::Current& /* = ::Ice::Current */)
{
	try {
		(new ListAllocationIdsCmd(_env, amdCB, ident))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeDevice,"listAllocationIds_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeDevice", 503, "failed to generate listAllocationIdsCmd"));
	}
}

void EdgeDeviceImpl::populateChannels_async(const TianShanIce::EdgeResource::AMD_EdgeDevice_populateChannelsPtr& amdCB, Ice::Short portId, const Ice::Current& c)
{
	try {
		(new PopulateChannelCmd(_env, amdCB, ident, portId))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeDevice,"populateChannels_async() failed to list"));
		amdCB->ice_exception(TianShanIce::ServerError("EdgeDevice", 503, "failed to generate PopulateChannelCmd"));
	}
}

TianShanIce::EdgeResource::EdgeChannelPrx EdgeDeviceImpl::openChannel(Ice::Short portId, Ice::Short channelNum, const Ice::Current& c)
{
	if (portId <0 || channelNum <0)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeDevice, 201, "openChannel() invalid portId[%d] or channelNum[%d]"), portId, channelNum);
		
	return _env._openChannel(EdgeRMImpl::formatHiberarchy(ident.name, portId, channelNum));
}

TianShanIce::EdgeResource::RoutesMap EdgeDeviceImpl::getRoutesRestriction(Ice::Short portId, const Ice::Current& c) const
{
	TianShanIce::IValues NilSvcGrps;
	TianShanIce::EdgeResource::RoutesMap sergroups;
	RLock sync(*this);
	TianShanIce::EdgeResource::EdgePortMap::const_iterator it = edgePorts.find(portId);
	if (edgePorts.end() == it)
		return sergroups;

/*	for(TianShanIce::EdgeResource::ServiceGroups::const_iterator itorsgs = it->second.serviceGroups.begin();
		itorsgs !=  it->second.serviceGroups.end(); itorsgs++)
	{
		NilSvcGrps.push_back(itorsgs->first);
	}*/

	return it->second.routes;
}

/*void EdgeDeviceImpl::restrictServiceGroups(Ice::Short portId, const TianShanIce::IValues& serviceGroups, const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeDevice, "restrictServiceGroups() updating the service group setting for port[%d]"), portId);

	std::set <Ice::Int> sgSet;
	for (TianShanIce::IValues::const_iterator it = serviceGroups.begin(); it < serviceGroups.end(); it++)
		sgSet.insert(*it);

	TianShanIce::IValues sgRestirct;

	for (std::set<Ice::Int>::iterator it2 = sgSet.begin(); it2 != sgSet.end(); it2++)
		sgRestirct.push_back(*it2);

	WLock sync(*this);
	_updateServiceGroupIndex(portId, sgRestirct);
}*/
void EdgeDeviceImpl::_updateRouteIndex(Ice::Short portId, TianShanIce::EdgeResource::RoutesMap& routesMap)
{
	TianShanIce::EdgeResource::RoutesMap::iterator itorsgs =  routesMap.begin();
	for(; itorsgs != routesMap.end(); itorsgs++)
	{
		std::string routeName = itorsgs->first;
		TianShanIce::Variant freqs =  itorsgs->second;

		// step 1. enumerate all the channels of the portId
		const std::string portHirechy = EdgeRMImpl::formatHiberarchy(ident.name, portId, -1) + ".";
		typedef std::set<Ice::Identity > IdentSet;
		IdentSet identChOfPort;
		{
//			IdentCollection tmp = _env._idxChannelOfDevice->find(ident);
			IdentCollection tmp;
			try
			{	
				::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
				DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(ident.name);
				if(itorDC != _env._devicechannels.end())
					tmp = itorDC->second;
			}
			catch (...){	
			}

			for (IdentCollection::iterator it = tmp.begin(); it < tmp.end(); it++)
			{
				if (0 != it->name.compare(0, portHirechy.length(), portHirechy))
					continue;

				identChOfPort.insert(*it);
			}
		}

		IdentSet channelOfSvcGroup;

		// step 3. fill the channels of this port if necessary
		for (IdentSet::iterator itCh= identChOfPort.begin(); itCh != identChOfPort.end(); itCh ++)
		{
			try
			{
				TianShanIce::EdgeResource::EdgeChannelExPrx chprx  = IdentityToObjEnv2(_env, EdgeChannelEx, *itCh);
				TianShanIce::StrValues expectedMetaData;
				expectedMetaData.push_back(SYS_PROP(FreqRF));
				EdgeChannelInfo chinfo = chprx->getInfo(expectedMetaData);
				TianShanIce::Properties::iterator itorchfreq = chinfo.props.find(SYS_PROP(FreqRF));
				if(itorchfreq == chinfo.props.end())
					continue;
				Ice::Long chfreqs = atol((itorchfreq->second).c_str());
				if(freqs.bRange)
				{
					if(chfreqs >=  freqs.lints[0] && chfreqs <= freqs.lints[1])
						channelOfSvcGroup.insert(*itCh);
				}
				else
				{
					if(freqs.lints.size()  == 0)
					{
						channelOfSvcGroup.insert(*itCh);
					}
					else
					{
						TianShanIce::LValues::iterator itorFreq = std::find(freqs.lints.begin(), freqs.lints.end(), chfreqs);
						if(itorFreq != freqs.lints.end())
						{
							channelOfSvcGroup.insert(*itCh);
						}
					}
				}
			}
			catch (Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeDevice,"_updateRouteIndex() failed to get channel[%s] info (%s)"),
					(*itCh).name.c_str(), ex.ice_name().c_str());
				continue; 
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeDevice,"_updateRouteIndex() failed to get channel[%s] info(%d)"),
					(*itCh).name.c_str(), SYS::getLastErr());
                continue;
			}
		}

		// replace the channels of port in the index
		for (IdentSet::iterator itChannel = channelOfSvcGroup.begin(); itChannel != channelOfSvcGroup.end(); itChannel++)
			_env._idxRouteName.insert(EdgeRMEnv::RouteIndex::value_type(routeName, *itChannel));
		
		std::string strRFfreq="";
		if(freqs.bRange)
		{
			char strTemp[65]="";
			sprintf(strTemp, "%lld ~ %lld\0", freqs.lints[0], freqs.lints[1]); 
            strRFfreq = strTemp;
		}
		else
		{
			TianShanIce::LValues::iterator itorFreq = freqs.lints.begin();
			while(itorFreq != freqs.lints.end())
			{
				char strTemp[65]="";
				sprintf(strTemp, "%lld\0", *itorFreq);
				strRFfreq = strRFfreq + strTemp + ",";
				itorFreq++;
			}
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDevice, "device[%s] updateRouteNameMap() updated port[%d] route[%s], frequency[%s]"), ident.name.c_str(), portId, routeName.c_str(), strRFfreq.c_str());
	}

	{
#ifdef _TestLog
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDevice, "list RouteNames and channnels"));
		ZQ::common::MutexGuard(_env._lkIdxRouteName);
		EdgeRMEnv::RouteIndex::iterator itor = _env._idxRouteName.begin();
		while(itor != _env._idxRouteName.end())
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDevice, "****** [%d][%s]   *******"),itor->first,  itor->second.name.c_str());
			itor++;
		}
#endif
	}
}
void EdgeDeviceImpl::linkRoutes(::Ice::Short EdgePortId, const ::std::string& routeName, const ::TianShanIce::Variant& freqs, const ::Ice::Current& c)
{
	WLock sync(*this);

	TianShanIce::EdgeResource::EdgePortMap::iterator itPort = edgePorts.find(EdgePortId);
	if (edgePorts.end() == itPort)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(EdgeDevice, 201, "device[%s] updateRoutesMap() invalid portId[%d]"), ident.name.c_str(), EdgePortId);

	/// step 1. update associated routeName to freq range
	TianShanIce::EdgeResource::RoutesMap& routes = itPort->second.routes;
	TianShanIce::EdgeResource::RoutesMap::iterator itorSg = itPort->second.routes.find(routeName);
	if(itorSg != itPort->second.routes.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(EdgeDevice, 202, "device[%s] updateRoutesMap()  route[%s] already exists in portID[%d]"), ident.name.c_str(), routeName.c_str(), EdgePortId);
	}

	if(freqs.type != TianShanIce::vtLongs || (freqs.bRange == true && freqs.lints.size() != 2))
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(EdgeDevice, 203, "device[%s] updateRoutesMap()  invalid RF frequency in portID[%d]"), ident.name.c_str(), EdgePortId);
	}

	MAPSET(TianShanIce::EdgeResource::RoutesMap, itPort->second.routes, routeName, freqs);
	TianShanIce::EdgeResource::RoutesMap rs;
	rs.insert(TianShanIce::EdgeResource::RoutesMap::value_type(routeName, freqs));
	///updateRoutesIndex 
	_updateRouteIndex(EdgePortId, rs); 
}
void EdgeDeviceImpl::_removeRouteIndex(Ice::Short portId, TianShanIce::StrValues rns)
{
	TianShanIce::StrValues::iterator itorRoutes =  rns.begin();
	for(; itorRoutes != rns.end(); itorRoutes++)
	{
		std::string routeName = *itorRoutes;

		typedef std::set<Ice::Identity > IdentSet;
		IdentSet channelOfSvcGroup;

		const std::string portHirechy = EdgeRMImpl::formatHiberarchy(ident.name, portId, -1) + ".";

		EdgeRMEnv::RouteIndex::iterator itSvcMapStart = _env._idxRouteName.find(routeName);//find: 返回第一个对象的位置
		EdgeRMEnv::RouteIndex::iterator itSvcMapEnd   = _env._idxRouteName.upper_bound(routeName);//upper_bound: 返回>对象的第一个位置

		if(itSvcMapStart != _env._idxRouteName.end())/// workSvcGrp = 2000, 而_idxServiceGroup为(3000, 4000)
		{
			// step 3.2 backup the channel of other ports
			for (EdgeRMEnv::RouteIndex::iterator itSvcMap = itSvcMapStart; itSvcMap != itSvcMapEnd; itSvcMap++)
			{
				// filter out the channels of the port, keep those NOT of the port
				if (0 != itSvcMap->second.name.compare(0, portHirechy.length(), portHirechy))
					channelOfSvcGroup.insert(itSvcMap->second);
			}
		}

		// replace the channels of port in the index
		if(itSvcMapStart != _env._idxRouteName.end())
			_env._idxRouteName.erase(itSvcMapStart, itSvcMapEnd);

		for (IdentSet::iterator itChannel = channelOfSvcGroup.begin(); itChannel != channelOfSvcGroup.end(); itChannel++)
			_env._idxRouteName.insert(EdgeRMEnv::RouteIndex::value_type(routeName, *itChannel));

		// replace the channels of port in the index
		for (IdentSet::iterator itChannel = channelOfSvcGroup.begin(); itChannel != channelOfSvcGroup.end(); itChannel++)
			_env._idxRouteName.insert(EdgeRMEnv::RouteIndex::value_type(routeName, *itChannel));

		envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDevice, "device[%s] removeRouteNameIndex() port[%d] route[%s]"), ident.name.c_str(), portId, routeName.c_str());
	}

	{
#ifdef _TestLog
		ZQ::common::MutexGuard(_env._lkIdxRouteName);
		EdgeRMEnv::RouteIndex::iterator itor = _env._idxRouteName.begin();
		while(itor != _env._idxRouteName.end())
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeDevice, "****** [%d][%s]   *******"),itor->first,  itor->second.name.c_str());
			itor++;
		}
#endif
	}
}
void EdgeDeviceImpl::unlinkRoutes(::Ice::Short EdgePortId, const ::std::string& routeName, const ::Ice::Current& c)
{
	WLock sync(*this);
	TianShanIce::EdgeResource::EdgePortMap::iterator itPort = edgePorts.find(EdgePortId);
	if (edgePorts.end() == itPort)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(EdgeDevice, 201, "device[%s] updateRouteNameMap() invalid portId[%d]"), ident.name.c_str(), EdgePortId);

	/// update associated route to freq range
	TianShanIce::EdgeResource::RoutesMap& routes = itPort->second.routes;
	TianShanIce::EdgeResource::RoutesMap::iterator itorRoutes = itPort->second.routes.find(routeName);
	if(itorRoutes == itPort->second.routes.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, EXPFMT(EdgeDevice, 204, "device[%s] updateRouteNameMap() invalid route[%s] in portID[%d]"), ident.name.c_str(), routeName.c_str(), EdgePortId);
	}
	itPort->second.routes.erase(itorRoutes);

	TianShanIce::StrValues rs;
	rs.push_back(routeName);
	_removeRouteIndex(EdgePortId, rs);
}

void EdgeDeviceImpl::destroy(const Ice::Current& c)
{
	//ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (envlog, OBJEXPFMT(EdgeRM, 201, __MSGLOC__ "TODO: impl destroy()"));
//#pragma message ( __MSGLOC__ "TODO: impl destroy()")

	envlog(ZQ::common::Log::L_INFO,OBJLOGFMT(EdgeDeviceImpl, "enter destory() edge device"));

	WLock sync(*this);
	IdentCollection channelIds;
/*	try
	{	
		channelIds = _env._idxChannelOfDevice->find(ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, OBJLOGFMT(EdgeDeviceImpl,"Fail to get EdgeDevice  object in DB caught (%s,%s)"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_ERROR, OBJLOGFMT(EdgeDeviceImpl,"Fail to get EdgeDevice object in DB caught (%s)"), 
			ex.ice_name().c_str());
	}*/

	try
	{	
		::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
		DeviceChannelsMap::iterator itorDC = _env._devicechannels.find(ident.name);
		if(itorDC != _env._devicechannels.end())
			channelIds = itorDC->second;
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, OBJLOGFMT(EdgeDeviceImpl,"Fail to get Channle of EdgeDevice caught unknown exception(%d)"), SYS::getLastErr());

		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeDeviceImpl, 301, "Fail to get Channle of EdgeDevice"));
	}

	if (channelIds.size() > 0)
	{	
		envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeDeviceImpl, "destroy() Edge Device(deviceId: %s) to check channel info"), ident.name.c_str());

		//check allocations
		for (IdentCollection::iterator it= channelIds.begin(); it < channelIds.end(); it++)
		{
			try
			{
				TianShanIce::EdgeResource::EdgeChannelExPrx channelPrx = IdentityToObjEnv2(_env, EdgeChannelEx, *it);
				TianShanIce::EdgeResource::Allocations allocations = channelPrx->getAllocations();
				if (allocations.size() > 0)
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeDeviceImpl, 302, "active allocation in channel(%s)"), (*it).name.c_str());
				}
			}
			catch(const Ice::Exception& ex)
			{
				envlog(ZQ::common::Log::L_ERROR, OBJLOGFMT(EdgeDeviceImpl, "destroy() failed to get Allocation for channel(%s) caught ice exception[%s]"),
					it->name.c_str(), ex.ice_name().c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeDeviceImpl, 303, "failed to get Allocation for channel(%s) caught ice exception(%s)"),
					it->name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_ERROR, OBJLOGFMT(EdgeDeviceImpl, "destroy()failed to get Allocation for channel(%s)caught unknown exception"), 
					it->name.c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeDeviceImpl, 304, "failed to get Allocation for channel(%s)caught unknown exception"),
					it->name.c_str());
			}		
		}

	}

	TianShanIce::EdgeResource::EdgePortMap::iterator it = edgePorts.begin(); 
	while(edgePorts.end() != it)
	{
		TianShanIce::StrValues NilRoutes;
		for(TianShanIce::EdgeResource::RoutesMap::iterator itorRoutes = it->second.routes.begin();
			itorRoutes !=  it->second.routes.end(); itorRoutes++)
		{
			NilRoutes.push_back(itorRoutes->first);
		}
		try
		{
			_removeRouteIndex(it->first, NilRoutes); // clean up the service group index about this port
		}
		catch (...)
		{
		}
		it++;
	}

	//destory channels of device
	for (IdentCollection::iterator it= channelIds.begin(); it < channelIds.end(); it++)
	{
		try
		{
			_env._eEdgeChannel->remove(*it);    
		}
		catch (const Freeze::DatabaseException& ex)
		{
			envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeDeviceImpl, "destroy() Fail to remove channel [%s] object from DB caught (%s,%s)"), 
				(*it).name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeDeviceImpl, 305, "Fail to remove channel[%s] object from DB caught (%s,%s)"),
				ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeDeviceImpl, "destroy() Fail to remove channel  [%s] object from DB caught (%s)"), 
				(*it).name.c_str(), ex.ice_name().c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeDeviceImpl, 306, "Fail to remove channel[%s] object from DB caught (%s)"),
				(*it).name.c_str(), ex.ice_name().c_str());
		}		
	}

	///remove DeviceChannelsMap index
	try
	{
	  ::ZQ::common::MutexGuard gd(_env._lkdevicechannels);
      _env.removeChannelsFromDevice(ident.name);
	}
	catch (...){
	}

	//if no allocation in device, remove it, otherwise an exception already throw
	//remove from evictor map
	try
	{	
		_env._eEdgeDevice->remove(ident);
		if(_env._enableERMI)
			_env.removeQamSessionGroup(ident.name);
		_env.removeQam(ident.name);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeDeviceImpl, "destroy() Fail to remove device  object from DB caught (%s,%s)"), 
			ex.ice_name().c_str(), ex.message.c_str());

		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeDeviceImpl, 307, "Fail to remove device object from DB caught (%s,%s)"),
			ex.ice_name().c_str(), ex.message.c_str());

	}
	catch (const Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeDeviceImpl, "destroy() Fail to remove device  object from DB caught (%s)"), 
			ex.ice_name().c_str());

		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeDeviceImpl, 308, "Fail to remove device object from DB caught (%s)"),
			ex.ice_name().c_str());
	}
	envlog(ZQ::common::Log::L_INFO,OBJLOGFMT(EdgeDeviceImpl, "leave destory() edge device"));
}

void EdgeDeviceImpl::OnRestore(const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeDevice, "OnRestore() rebuild servicegroup index for the device"));
	for (TianShanIce::EdgeResource::EdgePortMap::iterator itPort = edgePorts.begin(); itPort != edgePorts.end(); itPort++)
		_updateRouteIndex(itPort->first, itPort->second.routes);

	_env._watchDog.watch(ident, 0);
}

// impls of TianShanUtils::TimeoutObj
void EdgeDeviceImpl::OnTimer(const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeDevice, "OnTimer() "));

#pragma message ( __MSGLOC__ "TODO: EdgeDeviceImpl::OnTimer() check the device's status")

	_env._watchDog.watch(ident, MAX_INSTANCE_IDLE);
}

// -----------------------------
// class EdgeChannelImpl
// -----------------------------
EdgeChannelImpl::EdgeChannelImpl(EdgeRMEnv& env)
:_env(env)
{
}

EdgeChannelImpl::~EdgeChannelImpl()
{
}

// impls of EdgeChannel
bool EdgeChannelImpl::enable(bool toEnable, const Ice::Current& c)
{
	WLock(*this);
	enabled = toEnable;
	return enabled;
}

std::string EdgeChannelImpl::getId(const Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

TianShanIce::State EdgeChannelImpl::getState(const Ice::Current& c) const
{
	RLock sync(*this);
	return deviceState;
}

void EdgeChannelImpl::getHiberarchy(std::string& deviceName, Ice::Short& portId, Ice::Short& chNum, const Ice::Current& c) const
{
	RLock sync(*this);
	EdgeRMImpl::chopForHiberarchy(ident.name, deviceName, portId, chNum);

	if (deviceName.empty() || portId<0 || chNum<0)
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeRM, 201, "ch[%s] getHiberarchy() illegal channel hiberarchy"), ident.name.c_str());
}

/*std::string Int2Str(Ice::Int i)
{
	char temp[65] = "";
	sprintf(temp, "%d\0", i);
	return temp;
}
std::string Long2Str(Ice::Long i)
{
	char temp[65] = "";
	sprintf(temp, "%lld\0", i);
	return temp;
}
std::string Float2Str(Ice::Float f)
{
	char temp[256] = "";
	sprintf(temp, "%.6f\0", f);
	return temp;
}*/

TianShanIce::StatedObjInfo EdgeChannelImpl::getInfo(const TianShanIce::StrValues& expectedMetaData, const Ice::Current& c) const
{
	RLock sync(*this);
	EdgeChannelInfo info;
	info.ident = ident;
	info.state = deviceState;

//#pragma message ( __MSGLOC__ "TODO: fill in the meta data of channel here")

	if (expectedMetaData.empty())
		return info;

	//some metadata wanted
#ifndef FINDINMETADATA
#define FINDINMETADATA(_METADATA, _FIELD) find(_METADATA.begin(), _METADATA.end(), _FIELD) != _METADATA.end()
#define INTTOSTR(_DATA) (#_DATA)

	std::string metaValue;

	metaValue = Long2Str(stampLastUpdated);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(StampLastUpdated)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(StampLastUpdated),			metaValue);
	if (enabled)
		metaValue = Int2Str(1);
	else
		metaValue = Int2Str(0);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(Enabled)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Enabled),				metaValue);
	metaValue = Long2Str(freqRF);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(FreqRF)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(FreqRF),						metaValue);
	metaValue = Int2Str(TSID);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(TSID)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(TSID),						metaValue);
	metaValue = Int2Str(NITPID);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(NITPID)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(NITPID),						metaValue);
	metaValue = Int2Str(startUDPPort);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(StartUDPPort)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(StartUDPPort),				metaValue);
	metaValue = Int2Str(udpPortStepByPn);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(UdpPortStepByPn)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(UdpPortStepByPn),			metaValue);
	metaValue = Int2Str(startProgramNumber);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(StartProgramNumber)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(StartProgramNumber),			metaValue);
	metaValue = Long2Str(lowBandwidthUtilization);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(LowBandwidthUtilization)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(LowBandwidthUtilization),	metaValue);
	metaValue = Long2Str(highBandwidthUtilization);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(HighBandwidthUtilization)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(HighBandwidthUtilization),	metaValue);
	metaValue = Int2Str(maxSessions);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(MaxSessions)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(MaxSessions),				metaValue);
	metaValue = Int2Str(intervalPAT);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(IntervalPAT)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(IntervalPAT),				metaValue);
	metaValue = Int2Str(intervalPMT);
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(IntervalPMT)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(IntervalPMT),				metaValue);

	if (FINDINMETADATA(expectedMetaData, SYS_PROP(symbolRate)))
	{
		TianShanIce::ValueMap::const_iterator itor = ePort.resAtscModulationMode.resourceData.find("symbolRate");
		if(itor != ePort.resAtscModulationMode.resourceData.end() && itor->second.ints.size() > 0)
		{
			metaValue = Int2Str(itor->second.ints[0]);
			MAPSET(TianShanIce::Properties, info.props, SYS_PROP(symbolRate),				metaValue);
		}
	}

	metaValue = Int2Str(allocMap.size());
	if (FINDINMETADATA(expectedMetaData, SYS_PROP(AllocationCount)))
		MAPSET(TianShanIce::Properties, info.props, SYS_PROP(AllocationCount),				metaValue);
	
#undef FINDINMETADATA
#endif
	return info;
}

void EdgeChannelImpl::updateAttributes(const TianShanIce::Properties& attrs, const Ice::Current& c)
{
	/// entry from the EdgeDeviceHelper to update the attributes of the channel
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeChannel,"updateAttributes()"));
    #pragma message ( __MSGLOC__ "TODO: entry from the EdgeDeviceHelper to update the attributes of the channel")

	if(allocMap.size() > 0)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(EdgeChannel, 1180, "updateAttributes() alloction count[%d] resource reserverd, wait release all alloction and disabled channel"), allocMap.size());
	}
    for(TianShanIce::Properties::const_iterator itor = attrs.begin(); itor != attrs.end(); itor++)
	{
/*		if(itor->first == SYS_PROP(FreqRF)) // update to port 
		{
           freqRF = _atoi64((itor->second).c_str());
		}
		else
*/
		if(itor->first == SYS_PROP(TSID))
		{
			TSID = atoi((itor->second).c_str());
		}
		else if(itor->first == SYS_PROP(StartUDPPort))
		{
             startUDPPort = atoi((itor->second).c_str());
		}
		else if(itor->first == SYS_PROP(UdpPortStepByPn))
		{
			udpPortStepByPn = (::Ice::Byte)(atoi((itor->second).c_str()));
		}
		else if(itor->first == SYS_PROP(StartProgramNumber))
		{
             startProgramNumber = atoi((itor->second).c_str());
		}
		else if(itor->first == SYS_PROP(NITPID))
		{
			NITPID = atoi((itor->second).c_str());
		}
		else if(itor->first == SYS_PROP(MaxSessions))
		{
             maxSessions = atoi((itor->second).c_str());
		}
		else if(itor->first == SYS_PROP(LowBandwidthUtilization))
		{
             lowBandwidthUtilization = _atoi64((itor->second).c_str())*1000;
		}
		else if(itor->first == SYS_PROP(HighBandwidthUtilization))
		{
			highBandwidthUtilization = _atoi64((itor->second).c_str())*1000;
		}
		else if(itor->first == SYS_PROP(IntervalPAT))
		{
			intervalPAT = atoi((itor->second).c_str());
		}
		else if(itor->first == SYS_PROP(IntervalPMT))
		{
			intervalPMT = atoi((itor->second).c_str());
		}
		else if(itor->first == SYS_PROP(ModulationFMT))
		{
			TianShanIce::ValueMap::iterator itorport = ePort.resAtscModulationMode.resourceData.find("modulationFormat");
			if(itorport != ePort.resAtscModulationMode.resourceData.end() && itorport->second.bin.size() > 0)
			{
				itorport->second.bin.clear();
				itorport->second.bin.push_back(atoi((itor->second).c_str()));
			}
		}
		else if(itor->first == SYS_PROP(InterleaverDepth))
		{
			TianShanIce::ValueMap::iterator itorport = ePort.resAtscModulationMode.resourceData.find("interleaveDepth");
			if(itorport != ePort.resAtscModulationMode.resourceData.end() && itorport->second.bin.size() > 0)
			{
				itorport->second.bin.clear();
				itorport->second.bin.push_back(atoi((itor->second).c_str()));
			}
		}
		else if(itor->first == SYS_PROP(DeviceZone))
		{
			TianShanIce::ValueMap::iterator itorport = ePort.resPhysicalChannel.resourceData.find("edgeDeviceZone");
			if(itorport != ePort.resPhysicalChannel.resourceData.end() && itorport->second.strs.size() > 0)
			{
				itorport->second.strs.clear();
				itorport->second.strs.push_back(itor->second);
			}
		}
		else if(itor->first == SYS_PROP(DeviceIP))
		{
			TianShanIce::ValueMap::iterator itorport = ePort.resPhysicalChannel.resourceData.find("edgeDeviceIP");
			if(itorport != ePort.resPhysicalChannel.resourceData.end() && itorport->second.strs.size() > 0)
			{
				itorport->second.strs.clear();
				itorport->second.strs.push_back(itor->second);
			}
		}
		else if(itor->first == SYS_PROP(DeviceState))
		{
			deviceState = (TianShanIce::State)atoi(itor->second.c_str());
		}
	}
}

TianShanIce::EdgeResource::Allocations EdgeChannelImpl::getAllocations(const Ice::Current& c) const
{
	TianShanIce::EdgeResource::Allocations allocs;

	RLock sync(*this);
	for (TianShanIce::EdgeResource::AllocationMap::const_iterator it =allocMap.begin(); it !=allocMap.end(); it ++)
	{
		try {
			TianShanIce::EdgeResource::AllocationPrx alloc = IdentityToObjEnv(_env, Allocation, it->second.identAlloc);
			alloc->ice_ping();
			allocs.push_back(alloc);
		}
		catch(...) {}
	}

	return allocs;
}
::TianShanIce::EdgeResource::AllocationIds EdgeChannelImpl::getAllocationIds(const Ice::Current& c) const
{
	::TianShanIce::EdgeResource::AllocationIds allocIds;

	RLock sync(*this);
	for (TianShanIce::EdgeResource::AllocationMap::const_iterator it =allocMap.begin(); it !=allocMap.end(); it ++)
	{
		std::string allocId = it->second.identAlloc.name;
		allocIds.push_back(allocId);
	}

	return allocIds;
}
void EdgeChannelImpl::OnRestore(const Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl EdgeChannelImpl::OnRestore()")

	TianShanIce::IValues delAllocs;
	WLock sync(*this);
	{
		TianShanIce::EdgeResource::AllocationMap::iterator itAlloc;

		for(itAlloc = allocMap.begin(); itAlloc != allocMap.end(); itAlloc++)
		{
			try
			{
				TianShanIce::EdgeResource::AllocationPrx alloc = IdentityToObjEnv(_env, Allocation, itAlloc->second.identAlloc);
				alloc->ice_ping();	
			}
			catch (Ice::ObjectNotExistException)
			{		
				delAllocs.push_back(itAlloc->first);
//				itAlloc = allocMap.erase(itAlloc);
			}
		}
		for(int i = 0; i < delAllocs.size(); i++)
		{
			itAlloc = allocMap.find(delAllocs[i]);
			if(itAlloc != allocMap.end())
				allocMap.erase(itAlloc);
		}
	}

	_env._watchDog.watch(ident, 0);
}

// impls of TianShanUtils::TimeoutObj
void EdgeChannelImpl::OnTimer(const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeChannel, "OnTimer() "));

#pragma message ( __MSGLOC__ "TODO: impl EdgeChannelImpl::OnTimer()")
	_env._watchDog.watch(ident, MAX_INSTANCE_IDLE);
}

static void printLine(const char* line, void* pCtx=NULL)
{
	if (NULL == pCtx)
		return;

	EdgeRMEnv* pEnv = (EdgeRMEnv*) pCtx;
	pEnv->_log(ZQ::common::Log::L_DEBUG, line);
}

Ice::Int EdgeChannelImpl::evaluate(const TianShanIce::SRM::ResourceMap& resRequirement, TianShanIce::SRM::ResourceMap& resResult, const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeChannel, "Enter evaluate()"));

	Ice::Long stampStart = now();
	TianShanIce::EdgeResource::AllocLink allocLink;
	TianShanIce::IValues pnCandidates;

	RLock sync(*this);
	Ice::Int cost = COST_UNAVAILABLE;
	if (TianShanIce::stInService != deviceState || !enabled)
	{
		envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeChannel, "deviceState not InService or disabled"));
		return cost;
	}

	Ice::Long allocBW = 0; // need read from the resource requirement

	char dumpPrefix[DumpPrefixBufSize];
	snprintf(dumpPrefix, sizeof(dumpPrefix)-2, OBJLOGFMT(EdgeChannel, "evaluate() resource requirement: "));
	ZQTianShan::dumpResourceMap(resRequirement, dumpPrefix, printLine, &_env);

	TianShanIce::SRM::ResourceMap::const_iterator itResReq;

	// step 1.1. read the bandwidth from he res[rtTsDownstreamBandwidth]
	itResReq = resRequirement.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
	if (resRequirement.end() != itResReq)
	{
		TianShanIce::ValueMap::const_iterator itResData = itResReq->second.resourceData.find("bandwidth");
		if (itResReq->second.resourceData.end() != itResData &&	itResData->second.lints.size() >0)
			allocBW = itResData->second.lints[0];
	}

	if (allocBW <=0)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeChannel, 1173, "evaluate() res[rtTsDownstreamBandwidth][bandwidth] must be a valid requirement"));
	else
		MAPSET(TianShanIce::SRM::ResourceMap, resResult, TianShanIce::SRM::rtTsDownstreamBandwidth, itResReq->second);

	// step 1.2. verify about resource rtAtscModulationMode
	itResReq = resRequirement.find(TianShanIce::SRM::rtAtscModulationMode);
	if (resRequirement.end() == itResReq)
		MAPSET(TianShanIce::SRM::ResourceMap, resResult, TianShanIce::SRM::rtAtscModulationMode, ePort.resAtscModulationMode);
	else
	{
		TianShanIce::SRM::Resource resultAtscModulationMode;
		if (!ZQTianShan::InterRestrictResource(itResReq->second, ePort.resAtscModulationMode, resultAtscModulationMode))
			return cost;

		MAPSET(TianShanIce::SRM::ResourceMap, resResult, TianShanIce::SRM::rtAtscModulationMode, resultAtscModulationMode);
	}

	Ice::Long stampInputValidated = now();

#pragma message ( __MSGLOC__ "TODO: verify more requirement meet this channel attributes")

	// calculate the cost
	Ice::Long usedBW = 0;
	Ice::Long stampExpired = now() - UNATTENDED_TIMEOUT*2;
	for (int pn = startProgramNumber; pn < startProgramNumber + maxSessions; pn++)
	{
		TianShanIce::EdgeResource::AllocationMap::iterator itAlloc = allocMap.find(pn);
		if (allocMap.end() != itAlloc && itAlloc->second.bandwidth >0)
		{
			// sum up the reserved bandwidth of the allocation
			usedBW += itAlloc->second.bandwidth;
			continue; // skip this pn
		}

		TianShanIce::EdgeResource::ChannelUsageNodeMap::iterator itUsage = busyPN.find(pn);
		if (busyPN.end() != itUsage && itUsage->second.stampUpdated < stampExpired && itUsage->second.bandwidth >0)
		{
			envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeChannel, "evaluate() PN[%d] appears busy but not under the management of this EdgeRM, bitrate[%lld]"),pn, itUsage->second.bandwidth);
			// sum up the reserved bandwidth of the usage
			usedBW += itUsage->second.bandwidth;
			continue; // skip this pn
		}

		pnCandidates.push_back(pn);
	}

	if (!pnCandidates.empty())
		cost = 1* COST_MAX / pnCandidates.size();

	Ice::Long newBW = allocBW + usedBW;
	if (highBandwidthUtilization >0 && newBW > lowBandwidthUtilization && newBW < highBandwidthUtilization)
		cost = (Ice::Int) max(cost, COST_MAX * (newBW / highBandwidthUtilization));

	Ice::Long stampCost = now();
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeChannel, "evaluate() PN[%d] for alloc bandwidth [%lld] found for the allocation, channel bandwidth usage is %lld / %lld, min %lld, cost[%d]"),
		pnCandidates.size(), allocBW, usedBW, highBandwidthUtilization, lowBandwidthUtilization, cost);

	{
		TianShanIce::Variant var, var2;
		var.type = TianShanIce::vtLongs;
		var.bRange = false;
     	var.lints.push_back(freqRF);

		MAPSET(TianShanIce::SRM::ResourceMap, resResult, TianShanIce::SRM::rtPhysicalChannel, ePort.resPhysicalChannel);
		MAPSET(TianShanIce::ValueMap, resResult[TianShanIce::SRM::rtPhysicalChannel].resourceData, "channelId", var);
		var.lints.clear();

		//static const TianShanIce::SRM::Resource NilRes;
		::TianShanIce::SRM::Resource NilRes;

		var.type = TianShanIce::vtInts;
		var.bRange = false;
		var2.type = TianShanIce::vtLongs;
		var2.bRange = false;

		for (size_t i =0; i < MAX_EVAL_CANDIDATES && i < pnCandidates.size(); i++)
		{
			var2.lints.push_back(pnCandidates[i]);
			var.ints.push_back(startUDPPort + (pnCandidates[i] * udpPortStepByPn));
		}

		MAPSET(TianShanIce::SRM::ResourceMap, resResult, TianShanIce::SRM::rtEthernetInterface, NilRes);
		MAPSET(TianShanIce::SRM::ResourceMap, resResult, TianShanIce::SRM::rtMpegProgram, NilRes);
		MAPSET(TianShanIce::ValueMap, resResult[TianShanIce::SRM::rtPhysicalChannel].resourceData, "destPort", var);
		MAPSET(TianShanIce::ValueMap, resResult[TianShanIce::SRM::rtMpegProgram].resourceData, "Id", var2);
#pragma message ( __MSGLOC__ "TODO: fill in more result resources")
	}

#ifdef _DEBUG
	snprintf(dumpPrefix, sizeof(dumpPrefix)-2, OBJLOGFMT(EdgeChannel, "evaluate() resource result: "));
	ZQTianShan::dumpResourceMap(resRequirement, dumpPrefix, printLine, &_env);
#endif // _DEBUG

	Ice::Long stampFinished = now();
    envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeChannel, "Leave evaluate(), cost[%d] took %lldmsec (%lld+%lld+%lld)"),
		cost, stampFinished - stampStart, stampInputValidated -stampStart, stampCost - stampInputValidated, stampFinished -stampCost);

	return cost;
}

void EdgeChannelImpl::commit(const Ice::Identity& identAlloc, const TianShanIce::SRM::ResourceMap& resRequirement, TianShanIce::SRM::ResourceMap& resResult, const Ice::Current& c)
{
	Ice::Long stampStart = now();
	TianShanIce::EdgeResource::AllocLink link;
	try {
		IdentityToObjEnv(_env, AllocationEx, identAlloc)->ice_ping();
		link.identAlloc = identAlloc;
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeChannel, 101, "commit() alloc[%s] can not been found in local EdgeRM"), identAlloc.name.c_str());
		return;
	}

	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeChannel, "commit() alloc[%s]"), link.identAlloc.name.c_str());
	Ice::Long allocBW; // need read from the resource requirement
	TianShanIce::IValues pnCandidates;
#pragma message ( __MSGLOC__ "TODO: verify if the requirement meet this channel attributes")

	// calculate the cost
	Ice::Long usedBW = 0;
	Ice::Long stampExpired = now() - UNATTENDED_TIMEOUT*2;
	int pnSelected = -1;

	TianShanIce::SRM::ResourceMap::const_iterator itResReq;

	// step 1.1. double check if the res[rtPhysicalChannel] refer to self
	itResReq = resRequirement.find(TianShanIce::SRM::rtPhysicalChannel);
	if (resRequirement.end() == itResReq)
		ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, OBJEXPFMT(EdgeChannel, 101, "commit() alloc[%s] didn't specify res[rtPhysicalChannel] to commit"), identAlloc.name.c_str());
	else
	{
		RLock wsync(*this);
		TianShanIce::SRM::Resource interset;
//		TianShanIce::ValueMap::const_iterator itResData = itResReq->second.resourceData.find("edgeDeviceName");
		TianShanIce::ValueMap::const_iterator itResData = itResReq->second.resourceData.find("channelId");
		if (1 != itResData->second.lints.size() || !ZQTianShan::InterRestrictResource(itResReq->second, ePort.resPhysicalChannel, interset))
			ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, OBJEXPFMT(EdgeChannel, 101, "commit() alloc[%s] requirement res[rtPhysicalChannel] doesn't match this channel"), identAlloc.name.c_str());
	}

	// step 1.2 directly copy the requirement to resResult
	resResult = resRequirement;

	// step 1.3. read the bandwidth from the res[rtTsDownstreamBandwidth]
	itResReq = resRequirement.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
	if (resRequirement.end() != itResReq)
	{
		TianShanIce::ValueMap::const_iterator itResData = itResReq->second.resourceData.find("bandwidth");
		if (itResReq->second.resourceData.end() != itResData &&	itResData->second.lints.size() >0)
			allocBW = itResData->second.lints[0];
	}

	if (allocBW <=0)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeChannel, 1173, "evaluate() res[rtTsDownstreamBandwidth][bandwidth] must be a valid requirement"));

	// step 1.4. read the PN candidates from res[rtMpegProgram]
	itResReq = resRequirement.find(TianShanIce::SRM::rtMpegProgram);
	if (resRequirement.end() != itResReq)
	{
		TianShanIce::ValueMap::const_iterator itResData = itResReq->second.resourceData.find("Id");
		for (TianShanIce::LValues::const_iterator it = itResData->second.lints.begin(); it < itResData->second.lints.end(); it++)
			pnCandidates.push_back((Ice::Int) *it);
	}

	if (pnCandidates.size() <=0)
		ZQTianShan::_IceThrow<TianShanIce::SRM::InvalidResource> (envlog, OBJEXPFMT(EdgeChannel, 101, "commit() alloc[%s] didn't specify program number candidates via res[rtMpegProgram] to commit"), identAlloc.name.c_str());

	Ice::Long stampInputRead = now();

	WLock wsync(*this);

	// step 1.5. overwrite rtAtscModulationMode of port directly as result
	MAPSET(TianShanIce::SRM::ResourceMap, resResult, TianShanIce::SRM::rtAtscModulationMode, ePort.resAtscModulationMode);

	// step 2, determine the program number by double check the current usages
	for (int pn = startProgramNumber; pn < startProgramNumber + maxSessions; pn++)
	{
		TianShanIce::EdgeResource::AllocationMap::iterator itAlloc = allocMap.find(pn);
		if (allocMap.end() != itAlloc && itAlloc->second.bandwidth >0)
		{
			// sum up the reserved bandwidth of the allocation
			usedBW += itAlloc->second.bandwidth;
			continue; // skip this pn
		}

		TianShanIce::EdgeResource::ChannelUsageNodeMap::iterator itUsage = busyPN.find(pn);
		if (busyPN.end() != itUsage && itUsage->second.stampUpdated <stampExpired && itUsage->second.bandwidth >0)
		{
			envlog(ZQ::common::Log::L_WARNING, OBJLOGFMT(EdgeChannel, "commit() PN[%d] appears busy but not under the management of this EdgeRM, bitrate[%lld]"), pn,  itUsage->second.bandwidth);
			// sum up the reserved bandwidth of the usage
			usedBW += itUsage->second.bandwidth;
			continue; // skip this pn
		}
		
		for (TianShanIce::IValues::const_iterator it = pnCandidates.begin(); pnSelected <0 && it < pnCandidates.end(); it++)
		{
			if (pn == *it)
			{
				pnSelected = pn;
				break;
			}
		}
	}

	if (pnSelected < 0)
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeChannel, 101, "commit() alloc[%s] no idle program number per the required PN set"), identAlloc.name.c_str());

	Ice::Long stampSelected = now();
	Ice::Long newBW = allocBW + usedBW;
	if (newBW < lowBandwidthUtilization || (highBandwidthUtilization >0 && newBW > highBandwidthUtilization))
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeChannel, 101, "commit() alloc[%s] bandwidth[%lld + %lld] out-of-range [%lld, %lld]"), identAlloc.name.c_str(), allocBW, usedBW, lowBandwidthUtilization, highBandwidthUtilization);
		return;
	}

	pnCandidates.clear();
	pnCandidates.push_back(pnSelected);

#pragma message ( __MSGLOC__ "TODO: fill in the result resources")
	{
		TianShanIce::Variant ivar, lvar;
		ivar.type = TianShanIce::vtInts;
		ivar.bRange = false;
		lvar.type = TianShanIce::vtLongs;
		lvar.bRange = false;

		lvar.lints.push_back(pnSelected);
		ivar.ints.push_back(startUDPPort + (pnSelected * udpPortStepByPn));

		MAPSET(TianShanIce::ValueMap, resResult[TianShanIce::SRM::rtPhysicalChannel].resourceData, "destPort", ivar);
		MAPSET(TianShanIce::ValueMap, resResult[TianShanIce::SRM::rtMpegProgram].resourceData, "Id", lvar);
		lvar.lints.clear();
		ivar.ints.clear();

		lvar.lints.clear();
		lvar.lints.push_back(freqRF);
		MAPSET(TianShanIce::ValueMap, resResult[TianShanIce::SRM::rtPhysicalChannel].resourceData, "channelId", lvar);
	}
   
	link.stampCommitted = link.stampUpdated = now();
	link.bandwidth = allocBW;
	MAPSET(TianShanIce::EdgeResource::AllocationMap, allocMap, pnSelected, link);
	
	Ice::Long stampFinished = now();
	envlog(ZQ::common::Log::L_INFO, OBJLOGFMT(EdgeChannel, "commit() alloc[%s] committed at PN[%d] with BW[%lld], took %lldmsec(%lld+%lld+%lld)"), link.identAlloc.name.c_str(), pnSelected, link.bandwidth,
		stampFinished - stampStart, stampInputRead -stampStart, stampSelected - stampInputRead, stampFinished - stampSelected);
}
void EdgeChannelImpl::addAllocLink(::Ice::Int programNumber, const ::TianShanIce::EdgeResource::AllocLink& link, const ::Ice::Current& c)
{
	WLock sync(*this);
	MAPSET(TianShanIce::EdgeResource::AllocationMap, allocMap, programNumber, link);
}
TianShanIce::EdgeResource::AllocLink EdgeChannelImpl::getAllocLink(const Ice::Identity& identAlloc, ::Ice::Int programNumber, const ::Ice::Current& c)
{
   RLock rLock(*this);

	TianShanIce::EdgeResource::AllocationMap::iterator itAlloc = allocMap.find(programNumber);
	if (allocMap.end() == itAlloc || identAlloc != itAlloc->second.identAlloc)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeChannel, 102, "getAllocLink() no such alloc[%s] on pn[%d]"), identAlloc.name.c_str(), programNumber);

	return  itAlloc->second;
}

void EdgeChannelImpl::withdraw(const Ice::Identity& identAlloc, Ice::Int programNumber, const Ice::Current& c)
{
	WLock sync(*this);
	TianShanIce::EdgeResource::AllocationMap::iterator itAlloc = allocMap.find(programNumber);
	if (allocMap.end() == itAlloc) // || identAlloc != itAlloc->second.identAlloc)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (envlog, OBJEXPFMT(EdgeChannel, 101, "withdraw() no such alloc[%s] on pn[%d]"), identAlloc.name.c_str(), programNumber);

	TianShanIce::EdgeResource::AllocLink link = itAlloc->second;
	allocMap.erase(itAlloc);

	envlog(ZQ::common::Log::L_INFO, OBJLOGFMT(EdgeChannel, "withdraw() alloc[%s] is withdrawn from PN[%d], bandwidth[%lld]"), link.identAlloc.name.c_str(), programNumber, link.bandwidth);
}

TianShanIce::EdgeResource::EdgePort EdgeChannelImpl::getEdgePort(const Ice::Current& c)
{
	RLock rLock(*this);
	return ePort;
}

TianShanIce::EdgeResource::ProvisionPort EdgeChannelImpl::getProvisionPort(const Ice::Current& c)
{
	RLock rLock(*this);
	return provPort;
}

// -----------------------------
// class AllocationImpl
// -----------------------------
Ice::Identity AllocationImpl::generateIdent()
{
	Ice::Identity ident;
	ident.category = DBFILENAME_Allocation;

	char buf[32];
	ZQ::common::Guid id;
	id.create();
	id.toCompactIdstr(buf, sizeof(buf));
	ident.name = buf;

	return ident;
}

AllocationImpl::AllocationImpl(EdgeRMEnv& env)
:_env(env)
{
}

AllocationImpl::~AllocationImpl()
{
}

// impls of Allocation
std::string AllocationImpl::getId(const Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

TianShanIce::State AllocationImpl::getState(const Ice::Current& c) const
{
	RLock sync(*this);
	return state;
}

TianShanIce::EdgeResource::AllocationOwnerPrx AllocationImpl::getOwner(const Ice::Current& c) const
{
	RLock sync(*this);
	return owner;
}

TianShanIce::EdgeResource::EdgeChannelPrx AllocationImpl::getChannel(const Ice::Current& c) const
{
	RLock sync(*this);
	if (state < TianShanIce::stInService || channelAssocs.size() !=1)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(Allocation, 101, "Alloc[%s@%s] channels haven't been committed yet"), ident.name.c_str(), ZQTianShan::ObjStateStr(state));

	if (channelAssocs.size() !=1)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(Allocation, 101, "Alloc[%s@%s] channels haven't been committed yet"), ident.name.c_str(), ZQTianShan::ObjStateStr(state));

	return channelAssocs[0].ch;
}

TianShanIce::StatedObjInfo AllocationImpl::getInfo(const Ice::Current& c) const
{
	RLock rLock(*this);
	AllocationInfo info;
	info.ident = ident;
	info.state = state;

//#pragma message ( __MSGLOC__ "TODO: fill in the metadata of allocation here")

	//some metadata wanted
#ifndef FINDINMETADATA
#define FINDINMETADATA(_METADATA, _FIELD) find(_METADATA.begin(), _METADATA.end(), _FIELD) != _METADATA.end()

	std::string metaValue;
	std::stringstream ss;
	
	metaValue = ownerKey;
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(OwnerKey),			metaValue);
	metaValue = Int2Str(udpPort);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(UdpPort),			metaValue);
	metaValue = Int2Str(programNumber);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(ProgramNumber),		metaValue);
	metaValue = Float2Str(maxJitter);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(MaxJitter),			metaValue);
	metaValue = sourceIP;
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(SourceIP),			metaValue);
	metaValue = Long2Str(bandwidth);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Bandwidth),			metaValue);
	metaValue = Long2Str(stampCreated);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(StampCreated),		metaValue);
	metaValue = Long2Str(stampProvisioned);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(StampProvisioned),	metaValue);
	metaValue = Long2Str(stampCommitted);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(StampCommitted),		metaValue);
	metaValue = Long2Str(expiration);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(Expiration),			metaValue);
	metaValue = Int2Str(retrytimes);
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(RetryTimes),			metaValue);
	metaValue = sessionGroup;
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(SessionGroup),			metaValue);
	metaValue = onDemandSessionId;
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(OnDemandSessionId),			metaValue);
	metaValue = qamSessionId;
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(QamSessionId),			metaValue);
	metaValue = qamSessGroup;
	MAPSET(TianShanIce::Properties, info.props, SYS_PROP(QamSessionGroup),			metaValue);

#undef FINDINMETADATA
#endif
	return info;
}

void AllocationImpl::destroy(const Ice::Current& c)
{

	WLock wLock(*this);
	Ice::Context ctxt = c.ctx;
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeAllocation, "destroy() demanded by [%s]"), ctxt["caller"].c_str());

	switch (state)
	{
	case TianShanIce::stNotProvisioned:
	case TianShanIce::stProvisioned:
	case TianShanIce::stInService:
		AllocStateOutOfService(_env, *this).enter();
		// note: there is no "break" statement here

	case TianShanIce::stOutOfService:
		try
		{
			if(_env._enableERMI)
			{
				std::string  eqamSessGroup = getqamSessGroup(c);
				std::string  eqamSeessionId = getqamSessionId(c);
				if(!eqamSessGroup.empty() && !eqamSeessionId.empty())
					_env.ermiSessTearDown(ident.name, eqamSessGroup, eqamSeessionId);
			}
			if(_env._enableR6)
			{
				std::string  eqamSessGroup = getqamSessGroup(c);
				std::string  eqamSeessionId = getqamSessionId(c);
				//error code defined
				std::string	 reason = R6_RESPONSE_200;
				if(!eqamSessGroup.empty() && !eqamSeessionId.empty())
					_env.r6SessTearDown(ident.name, eqamSeessionId);
			}

			envlog(ZQ::common::Log::L_INFO, OBJLOGFMT(EdgeAllocation, "state[%s] destroy() removing the object"), ObjStateStr(state));
			_env._eAllocation->remove(ident);

		}
		catch(const Ice::ObjectNotExistException&)
		{
			envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(EdgeAllocation, "destroy() object already gone, ignore"));
			return;
		}
		catch(const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeAllocation, 1051, "alloc[%s] destroy() caught exception: %s"), ident.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow <TianShanIce::ServerError> (envlog, OBJEXPFMT(EdgeAllocation, 1051, "alloc[%s] destroy() caught unknown exception"), ident.name.c_str());
		}

#pragma message ( __MSGLOC__ "TODO: AllocationImpl::destroy() fire the event")
	};
}

Ice::Long AllocationImpl::addResource(TianShanIce::SRM::ResourceType type, const TianShanIce::SRM::Resource& res, const Ice::Current& c)
{
	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(Allocation, "addResource() with type is %d"), type);

	switch (state)
	{
	case TianShanIce::stNotProvisioned:
	case TianShanIce::stProvisioned:
		break;

	case TianShanIce::stInService:
	case TianShanIce::stOutOfService:
	default:
		ZQTianShan::_IceThrow <TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(Allocation, 1071, "addResource() not allowed in state: %s(%d)"), ZQTianShan::ObjStateStr(state), state);
	};

#ifdef _DEBUG
	char dumpPrefix[128];
	snprintf(dumpPrefix, sizeof(dumpPrefix)-2, OBJLOGFMT(Allocation, "addResource(): "));
	ZQTianShan::dumpResource(res, dumpPrefix, printLine, &_env);
#endif // _DEBUG

    WLock sync(*this);
	TianShanIce::SRM::ResourceMap::iterator itRes = resources.find(type);
	if (itRes == resources.end())
		MAPSET(TianShanIce::SRM::ResourceMap, resources, type, res);
	else
	{
		TianShanIce::SRM::Resource& oldres = itRes->second;
		TianShanIce::ValueMap& data =oldres.resourceData;
		oldres.attr = res.attr;
		oldres.status = res.status;
		TianShanIce::ValueMap::const_iterator itResData = res.resourceData.begin();
		for ( ; itResData != res.resourceData.end () ; itResData ++)
			data[itResData->first] = itResData->second;
	}

	envlog(ZQ::common::Log::L_DEBUG, OBJLOGFMT(Allocation, "Leave addResource() with type is %d"), type);

	return 0;
}

TianShanIce::SRM::ResourceMap AllocationImpl::getResources(const Ice::Current& c) const
{
	RLock sync(*this);
	return resources;
}

void AllocationImpl::removeResource(TianShanIce::SRM::ResourceType type, const Ice::Current& c)
{
	if (state >= TianShanIce::stInService)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (envlog, OBJEXPFMT(Allocation, 201, "Allocation[%s:%s] removeResource() not allowed in this state"), ident.name.c_str(), ZQTianShan::ObjStateStr(state));
	
	WLock sync(*this);
	resources.erase(type);
}

void AllocationImpl::provision(Ice::Short maxChannelCandidates, bool reserveNow, const Ice::Current& c)
{
	WLock wLock(*this);
	try 
	{
		_maxChannelCandidates = maxChannelCandidates;
		_reserveNow = reserveNow;

		AllocStateProvisioned(_env, *this).enter();
	}
	catch(const TianShanIce::BaseException& be)
	{
		be.ice_throw();
	}
	catch(const Ice::Exception& e)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, "EdgeAllocation", 1131, OBJLOGFMT(EdgeAllocation, "provision(): caught Ice caught: %s"), e.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, "EdgeAllocation", 1132, OBJLOGFMT(EdgeAllocation, "provision(): caught unknown exception"));
	}
}

void AllocationImpl::serve(const Ice::Current& c)
{
	WLock wLock(*this);
	try 
	{
		AllocStateInService(_env, *this).enter();
	}
	catch(const TianShanIce::BaseException& be)
	{
		be.ice_throw();
	}
	catch(const Ice::Exception& e)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, "EdgeAllocation", 1131, OBJLOGFMT(EdgeAllocation, "serve(): exception caught: %s"), e.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (envlog, "EdgeAllocation", 1132, OBJLOGFMT(EdgeAllocation, "serve(): unkown exception"));
	}
}

void AllocationImpl::renew(Ice::Long TTL, const Ice::Current& c)
{
	if (TTL < 0)
		return;

	if(TianShanIce::stOutOfService == state)
	{
//		envlog(ZQ::common::Log::L_DEBUG, SESSLOGFMT("session is in outOfserviceState,renew is ignored"));
		return;
	}
	
	WLock sync(*this);
	expiration = now() + TTL;
//	char buf[64];
//	envlog(ZQ::common::Log::L_DEBUG,SESSLOGFMT("renew time to=%s"), ZQTianShan::TimeToUTC(expiration, buf, sizeof(buf)-2);
	_env._watchDog.watch(ident, (long)TTL);   
}
void AllocationImpl::setRetrytimes(Ice::Int retimes, const Ice::Current& c)
{
    WLock sync(*this);
    retrytimes = retimes;
}
void AllocationImpl::setqamSessionId(const std::string& sessId, const Ice::Current& c)
{
	WLock sync(*this);
	qamSessionId = sessId;
}
void AllocationImpl::setqamSessGroup(const std::string& sessGroup, const Ice::Current& c)
{
	WLock sync(*this);
	qamSessGroup = sessGroup;
}
void AllocationImpl::setSessionGroup(const std::string& sessGroup, const Ice::Current& c)
{
	WLock sync(*this);
	sessionGroup = sessGroup;
}
void AllocationImpl::setOnDemandSessionId(const std::string& onDemandSessId, const Ice::Current& c)
{
	WLock sync(*this);
	onDemandSessionId = onDemandSessId;
}
std::string  AllocationImpl::getOwnerKey(const Ice::Current& c)
{
//	RLock sync(*this);
	return ownerKey;
}
std::string  AllocationImpl::getqamSessionId(const Ice::Current& c)
{
	//	RLock sync(*this);
	return qamSessionId;
}
std::string  AllocationImpl::getqamSessGroup(const Ice::Current& c)
{
	//	RLock sync(*this);
	return qamSessGroup;
}
std::string  AllocationImpl::getSessionGroup(const Ice::Current& c)
{
//	RLock sync(*this);
	return sessionGroup;
}
std::string  AllocationImpl::getOnDemandSessionId(const Ice::Current& c)
{
//	RLock sync(*this);
	return onDemandSessionId;
}
TianShanIce::EdgeResource::ChannelAssocation AllocationImpl::getChannelAssocs(const Ice::Current& c)
{
	return channelAssocs;
}
void AllocationImpl::OnRestore(const Ice::Current& c)
{
	WLock sync(*this);
	switch(state)
	{
	case TianShanIce::stNotProvisioned:
		AllocStateNotProvisioned(_env, *this).OnRestore(c);
		break;

	case TianShanIce::stProvisioned:
		AllocStateProvisioned(_env, *this).OnRestore(c);
		break;

	case TianShanIce::stInService:
		AllocStateInService(_env, *this).OnRestore(c);
		break;

	case TianShanIce::stOutOfService:
		AllocStateOutOfService(_env, *this).OnRestore(c);
		break;
	}
}
// impls of TianShanUtils::TimeoutObj
void AllocationImpl::OnTimer(const Ice::Current& c)
{
	WLock sync(*this);
	switch(state)
	{
	case TianShanIce::stNotProvisioned:
		AllocStateNotProvisioned(_env, *this).OnTimer(c);
		break;

	case TianShanIce::stProvisioned:
		AllocStateProvisioned(_env, *this).OnTimer(c);
		break;

	case TianShanIce::stInService:
		AllocStateInService(_env, *this).OnTimer(c);
		break;

	case TianShanIce::stOutOfService:
		AllocStateOutOfService(_env, *this).OnTimer(c);
		break;
	}
}

void AllocationImpl::negotiateResources_async(const TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr& amdCB, const Ice::Current& c)
{
	try {
		(new NegotiateResourcesCmd(_env, amdCB, *this))->execute();
	}
	catch(...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRM,"negotiateResources_async() initialize command"));
		amdCB->ice_exception(TianShanIce::ServerError("Allocation", 502, "negotiateResources_async() initialize command"));
	}
}

/*
void AllocationImpl::marshal(const TianShanIce::EdgeResource::AlloctionRecord& record, TianShanIce::BValues& value, const Ice::CommunicatorPtr& communicator)
{
	IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
	IceInternal::BasicStream stream(instance.get());
	record.__write(&stream);
	std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(value);
}
void AllocationImpl::unmarshal(TianShanIce::EdgeResource::AlloctionRecord& record, const TianShanIce::BValues& value, const Ice::CommunicatorPtr& communicator )
{
	IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
	IceInternal::BasicStream stream(instance.get());
	stream.b.resize(value.size());
	memcpy(&stream.b[0], &value[0], value.size());
	stream.i = stream.b.begin();
	record.__read(&stream);
}
*/

void AllocationImpl::marshal(const TianShanIce::EdgeResource::AlloctionRecord& record, TianShanIce::BValues& value, const Ice::CommunicatorPtr& communicator)
{
	ZQTianShan::Util::marshal< TianShanIce::EdgeResource::AlloctionRecord >(record, value, communicator);
}

void AllocationImpl::unmarshal(TianShanIce::EdgeResource::AlloctionRecord& record, const TianShanIce::BValues& value, const Ice::CommunicatorPtr& communicator )
{
	ZQTianShan::Util::unmarshal< TianShanIce::EdgeResource::AlloctionRecord >(record, value, communicator);
}

}} // namespace
