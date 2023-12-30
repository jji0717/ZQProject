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
// Ident : $Id: EdgeRMImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMImpl.h $
// 
// 19    6/06/16 5:20p Li.huang
// ETV-TV-NOW-CR038
// 
// 18    1/11/16 5:50p Dejian.fei
// 
// 17    1/15/14 3:48p Bin.ren
// 
// 16    11/27/13 2:19p Ketao.zhang
// add diffAllocations_async and getAllocationIds
// 
// 15    11/13/13 1:32p Bin.ren
// 
// 14    11/08/13 2:11p Bin.ren
// add device update
// 
// 13    10/11/13 5:49p Bin.ren
// add exportDevice interface
// 
// 12    9/24/13 2:42p Li.huang
// sync allocation
// 
// 11    9/11/13 4:13p Bin.ren
// 
// 10    9/11/13 1:26p Li.huang
// marshal alloction
// 
// 9     7/04/13 5:00p Bin.ren
// 
// 8     7/01/13 2:01p Bin.ren
// 
// 7     6/03/13 4:41p Li.huang
// add getEdgePort From EdgeDevice
// 
// 6     5/23/13 4:00p Li.huang
// 
// 5     3/28/13 1:49p Bin.ren
// 
// 4     3/25/13 2:21p Bin.ren
// 
// 3     11/01/12 3:29p Li.huang
// 
// 2     10/31/12 2:47p Li.huang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 16    10-01-04 11:44 Li.huang
// modify resource edgeDeviceGroup to edgeDeviceZone
// 
// 15    09-12-18 9:33 Li.huang
// restore
// 
// 15    09-12-10 18:21 Li.huang
// remove deviceOfchannel Index  and add associated servicegroup to freq
// range
// 
// 14    09-11-26 15:42 Li.huang
// fix some bugs 
// 
// 13    09-11-02 16:36 Li.huang
// 
// 12    09-10-30 11:58 Li.huang
// 
// 11    09-09-28 16:10 Li.huang
// 
// 10    09-07-30 17:16 Xiaoming.li
// get edgeport info
// 
// 9     09-07-27 17:00 Xiaoming.li
// add lock for allocation
// 
// 8     09-07-16 9:55 Xiaoming.li
// add compress import function
// 
// 7     09-07-08 11:11 Xiaoming.li
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

#ifndef __ZQTianShan_EdgeRMImpl_H__
#define __ZQTianShan_EdgeRMImpl_H__

#include "../common/TianShanDefines.h"
#include "ZQ_common_conf.h"
#include "Locks.h"

#include "EdgeRM.h"
//#include "EdgeRMEnv.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#include <set>

namespace ZQTianShan {
namespace EdgeRM {
class EdgeRMEnv;
// -----------------------------
// class EdgeDeviceImpl
// -----------------------------
//class EdgeDeviceImpl : public ::TianShanIce::EdgeResource::EdgeDeviceEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class EdgeDeviceImpl : public ::TianShanIce::EdgeResource::EdgeDeviceEx, public ICEAbstractMutexRLock
{
public:
	typedef ::IceInternal::Handle< EdgeDeviceImpl > Ptr;

	EdgeDeviceImpl(EdgeRMEnv& env);
	virtual ~EdgeDeviceImpl();

public:	// impls of EdgeDeviceEx
    virtual void OnRestore(const ::Ice::Current& c);
	virtual void addChannel(::Ice::Short portId, ::Ice::Int chId, const ::TianShanIce::Properties& attributes, const ::Ice::Current& c);

    virtual void addEdgePort(const ::TianShanIce::EdgeResource::EdgePort& port, const ::Ice::Current& c);
    virtual void removeEdgePort(::Ice::Short portId, const ::Ice::Current& c);
    virtual TianShanIce::EdgeResource::EdgePort getEdgePort(::Ice::Short portId, const ::Ice::Current& c);
	virtual void updateAttributes(const ::TianShanIce::Properties&, const ::Ice::Current& /* = ::Ice::Current */);
public:	// impls of EdgeDevice
    virtual ::std::string getName(const ::Ice::Current& c) const;
    virtual ::TianShanIce::ObjectInfo getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const;
    virtual ::TianShanIce::EdgeResource::EdgePortInfos listEdgePorts(const ::Ice::Current& c) const;
    virtual void listChannels_async(const ::TianShanIce::EdgeResource::AMD_EdgeDevice_listChannelsPtr& amdCB, ::Ice::Short portId, const ::TianShanIce::StrValues& expectedMetaData, bool enabledOnly, const ::Ice::Current& c) const;
    virtual void populateChannels_async(const ::TianShanIce::EdgeResource::AMD_EdgeDevice_populateChannelsPtr&, ::Ice::Short portId, const ::Ice::Current& c);
    virtual ::TianShanIce::EdgeResource::EdgeChannelPrx openChannel(::Ice::Short portId, ::Ice::Short channelNum, const ::Ice::Current& c);
	virtual void listAllocationIds_async(const ::TianShanIce::EdgeResource::AMD_EdgeDeviceEx_listAllocationIdsPtr&, const ::Ice::Current& /* = ::Ice::Current */);

    //virtual ::TianShanIce::EdgeResource::ServiceGroupsMap getServiceGroupRestriction(::Ice::Short portId, const ::Ice::Current& c) const;
	virtual ::TianShanIce::EdgeResource::RoutesMap getRoutesRestriction(::Ice::Short portId, const ::Ice::Current& c) const;
//    virtual void restrictServiceGroups(::Ice::Short portId, const ::TianShanIce::IValues& serviceGroups, const ::Ice::Current& c);
	//void linkServiceGroups(::Ice::Short EdgePortId, ::Ice::Int serviceGroup, const ::TianShanIce::Variant& freqs, const ::Ice::Current& c);
	//void unlinkServiceGroups(::Ice::Short EdgePortId, ::Ice::Int serviceGroup, const ::Ice::Current& c);
	void linkRoutes(::Ice::Short EdgePortId, const ::std::string& routeName, const ::TianShanIce::Variant& freqs, const ::Ice::Current& c);
	void unlinkRoutes(::Ice::Short EdgePortId, const ::std::string& routeName, const ::Ice::Current& c);
	
	virtual void destroy(const ::Ice::Current& c);

public:	// impls of TianShanUtils::TimeoutObj
	virtual void OnTimer(const ::Ice::Current& c);

protected:

	EdgeRMEnv&	_env;
//	void _updateServiceGroupIndex(::Ice::Short portId, const ::TianShanIce::IValues& newServiceGroups);
	//void _updateServiceGroupIndex(::Ice::Short portId, TianShanIce::EdgeResource::ServiceGroupsMap& servicegroups);
	void _updateRouteIndex(::Ice::Short portId, TianShanIce::EdgeResource::RoutesMap& routesMap);
	void _removeRouteIndex(::Ice::Short portId, TianShanIce::StrValues routeNames);

	TianShanIce::EdgeResource::EdgeChannelInfos getChFreq(Ice::Short portId);
};

// -----------------------------
// class EdgeChannelImpl
// -----------------------------
//class EdgeChannelImpl : public ::TianShanIce::EdgeResource::EdgeChannelEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class EdgeChannelImpl : public ::TianShanIce::EdgeResource::EdgeChannelEx, public ICEAbstractMutexRLock
{
public:
	typedef ::IceInternal::Handle< EdgeChannelImpl > Ptr;

	EdgeChannelImpl(EdgeRMEnv& env);
	virtual ~EdgeChannelImpl();

public:	// impls of EdgeChannelEx
    virtual void OnRestore(const ::Ice::Current& c);
    virtual void updateAttributes(const ::TianShanIce::Properties& attrs, const ::Ice::Current& c);

public:	// impls of EdgeChannel
    virtual bool enable(bool toEnable, const ::Ice::Current& c);
    virtual ::std::string getId(const ::Ice::Current& c) const;
    virtual ::TianShanIce::State getState(const ::Ice::Current& c) const;
    virtual void getHiberarchy(::std::string& deviceName, ::Ice::Short& portId, ::Ice::Short& chNum, const ::Ice::Current& c) const;
    virtual ::TianShanIce::StatedObjInfo getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const;
    virtual ::TianShanIce::EdgeResource::Allocations getAllocations(const ::Ice::Current& c) const;
    virtual ::TianShanIce::EdgeResource::AllocationIds getAllocationIds(const ::Ice::Current& c) const;
//    virtual ::TianShanIce::IValues getServiceGroupRestriction(const ::Ice::Current& c) const;
//    virtual void restrictServiceGroups(const ::TianShanIce::IValues& serviceGroups, const ::Ice::Current& c);

//    virtual void evaluate_async(const ::TianShanIce::EdgeResource::AMD_EdgeChannel_evaluatePtr& amdCB, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::TianShanIce::SRM::ResourceMap& resources, const ::Ice::Current& c);
//    virtual void commit_async(const ::TianShanIce::EdgeResource::AMD_EdgeChannel_commitPtr& amdCB, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::TianShanIce::SRM::ResourceMap& resources, const ::Ice::Current& c);
//    virtual void withdraw_async(const ::TianShanIce::EdgeResource::AMD_EdgeChannel_withdrawPtr& amdCB, const ::TianShanIce::EdgeResource::AllocationPrx& allc, ::Ice::Int PN, const ::Ice::Current& c);

    virtual ::Ice::Int evaluate(const ::TianShanIce::SRM::ResourceMap& resRequirement, ::TianShanIce::SRM::ResourceMap& resResult, const ::Ice::Current& c);
    virtual void commit(const ::Ice::Identity& identAlloc, const ::TianShanIce::SRM::ResourceMap& resRequirement, ::TianShanIce::SRM::ResourceMap& resResult, const ::Ice::Current& c);
    virtual void withdraw(const ::Ice::Identity& identAlloc, ::Ice::Int programNumber, const ::Ice::Current& c);

	virtual ::TianShanIce::EdgeResource::EdgePort getEdgePort(const ::Ice::Current& c);
	virtual	::TianShanIce::EdgeResource::ProvisionPort getProvisionPort(const ::Ice::Current& c);

	virtual void addAllocLink(::Ice::Int programNumber, const ::TianShanIce::EdgeResource::AllocLink& link, const ::Ice::Current& c);
	virtual TianShanIce::EdgeResource::AllocLink getAllocLink(const ::Ice::Identity& identAlloc, ::Ice::Int programNumber, const ::Ice::Current& c);

public:	// impls of TianShanUtils::TimeoutObj
	virtual void OnTimer(const ::Ice::Current& c);

protected:
	EdgeRMEnv& _env;
};
typedef ::std::vector< ::Ice::Byte> Key;
// -----------------------------
// class AllocationImpl
// -----------------------------
//class AllocationImpl : public ::TianShanIce::EdgeResource::AllocationEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class AllocationImpl : public ::TianShanIce::EdgeResource::AllocationEx, public ICEAbstractMutexRLock
{
	friend class AllocStateProvisioned;

public:
	typedef ::IceInternal::Handle< AllocationImpl > Ptr;

	AllocationImpl(EdgeRMEnv& env);
	virtual ~AllocationImpl();

	static ::Ice::Identity generateIdent();

public:	// impls of AllocationEx
    virtual void OnRestore(const ::Ice::Current& c);

public:	// impls of Allocation
    virtual ::std::string getId(const ::Ice::Current& c) const;
    virtual ::TianShanIce::State getState(const ::Ice::Current& c) const;
    virtual ::TianShanIce::EdgeResource::AllocationOwnerPrx getOwner(const ::Ice::Current& c) const;
    virtual ::TianShanIce::EdgeResource::EdgeChannelPrx getChannel(const ::Ice::Current& c) const;
    virtual ::TianShanIce::StatedObjInfo getInfo(const ::Ice::Current& c) const;
    virtual void destroy(const ::Ice::Current& c);
    virtual ::Ice::Long addResource(::TianShanIce::SRM::ResourceType, const ::TianShanIce::SRM::Resource&, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::ResourceMap getResources(const ::Ice::Current& c) const;
    virtual void removeResource(::TianShanIce::SRM::ResourceType, const ::Ice::Current& c);

    virtual void provision(::Ice::Short maxChannelCandidates, bool reserveNow, const ::Ice::Current& c);
    virtual void negotiateResources_async(const TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr& amdCB, const ::Ice::Current& c);
    virtual void serve(const ::Ice::Current& c);
    virtual void renew(::Ice::Long, const ::Ice::Current& c);

	virtual void setRetrytimes(::Ice::Int retrytimes, const ::Ice::Current& c);

	virtual void setqamSessionId(const ::std::string& sessId, const ::Ice::Current& c);
	virtual void setqamSessGroup(const ::std::string& sessId, const ::Ice::Current& c);

	virtual void setSessionGroup(const ::std::string&  sessGroup, const ::Ice::Current& c);
	virtual void setOnDemandSessionId(const ::std::string& onDemandSessId, const ::Ice::Current& c);

	virtual ::std::string getOwnerKey(const ::Ice::Current& c);
	virtual ::std::string getSessionGroup(const ::Ice::Current& c);
	virtual ::std::string getOnDemandSessionId(const ::Ice::Current& c);
    virtual ::std::string  getqamSessionId(const ::Ice::Current& c);
    virtual ::std::string  getqamSessGroup(const ::Ice::Current& c);
	virtual TianShanIce::EdgeResource::ChannelAssocation getChannelAssocs(const ::Ice::Current& c);
public:	// impls of TianShanUtils::TimeoutObj
	virtual void OnTimer(const ::Ice::Current& c);
protected:
	EdgeRMEnv&	_env;

	size_t      _maxChannelCandidates;
	bool        _reserveNow;
public:
	static void marshal(const TianShanIce::EdgeResource::AlloctionRecord& record, TianShanIce::BValues& value, const Ice::CommunicatorPtr& communicator);
	static void unmarshal(TianShanIce::EdgeResource::AlloctionRecord& record, const TianShanIce::BValues& value, const Ice::CommunicatorPtr& communicator);
};

// -----------------------------
// class EdgeRMImpl
// -----------------------------
class EdgeRMImpl : public ::TianShanIce::EdgeResource::EdgeRM
{
public:
	typedef ::IceInternal::Handle< EdgeRMImpl > Ptr;

	EdgeRMImpl(EdgeRMEnv& env);
	virtual ~EdgeRMImpl();

	/// validate an allocation per Allocation::provision().
	/// The EdgeRM will validate the resource requirements collected in the Allocation, and evaluate
	/// the matched channels, evaluate each for the cost and build up Allocation::chCandidates
	///@param[in] pAlloc pointer to the Allocation instance
	///@param[in] maxChannelCandidates the max number of the channel candidates, quit validating if this has been met
	///@param[in] reservePN true if the PN is reserved at this step, when this reservation happens, maxChannelCandidates
	///           is forced to be non-zero and no bigger than EdgeRM::preReserveChannels
	///@return    the number of the associated validate channels
	int validateAlloc(AllocationImpl::Ptr& pAlloc, int reservePN =2, uint8 maxChannelCandidates=0);
	//	throws ServerError;

	static int chopForHiberarchy(const std::string& entryName, OUT std::string& deviceName, OUT ::Ice::Short& portId, OUT ::Ice::Short& channelNum);
	static const ::std::string formatHiberarchy(const ::std::string& deviceName, ::Ice::Short portId, ::Ice::Short channelNum);

public:	// impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// impls of EdgeRM
	virtual void importDevice_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDevicePtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::std::string& xmlDefFile, bool bCompress, const ::TianShanIce::Properties& props, const ::Ice::Current& c);
	virtual void importDeviceXML_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDeviceXMLPtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::TianShanIce::Properties& props, const std::string& xmlBody, const ::Ice::Current& c);
    virtual ::TianShanIce::EdgeResource::EdgeChannelExPrx openChannel(const ::std::string& chName, const ::Ice::Current& c);
	virtual void forceBackupMode(const ::std::string& netId, ::Ice::Int mode, const Ice::Current& c);
	virtual void exportAllocations_async(const TianShanIce::EdgeResource::AMD_EdgeRM_exportAllocationsPtr& amdCB, const std::string& deviceName, const std::string& since, const Ice::Current& c) const;
	virtual void exportDevices_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_exportDevicesPtr& amdCB, const ::std::string& deviceName, const std::string& xmlDefFIle, const ::Ice::Current& c) const;
	virtual void exportDeviceXML_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_exportDeviceXMLPtr& amdCB, const ::std::string& deviceName, const ::Ice::Current& c) const;
	virtual void diffAllocations_async(const ::TianShanIce::EdgeResource::AMD_EdgeRM_diffAllocationsPtr& amdCB, const ::std::string& deviceName, const ::TianShanIce::StrValues& allocIds, const ::Ice::Current& c) const;

protected:	// impls of EdgeResouceManager
    virtual ::TianShanIce::EdgeResource::AllocationPrx createAllocation(const ::TianShanIce::SRM::ResourceMap& resRequirement, ::Ice::Int TTL, const ::TianShanIce::EdgeResource::AllocationOwnerPrx& owner, const ::std::string& ownerContextKey, const ::Ice::Current& c);
    virtual ::TianShanIce::EdgeResource::AllocationPrx openAllocation(const ::std::string& Id, const ::Ice::Current& c);
    virtual void listAllocations_async(const ::TianShanIce::EdgeResource::AMD_EdgeResouceManager_listAllocationsPtr& amdCB, const ::std::string& deviceName, ::Ice::Short portId, ::Ice::Short chNum, const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const;
    virtual ::TianShanIce::EdgeResource::EdgeDeviceInfos listDevices(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& c) const;
	virtual void addDevice_async(const ::TianShanIce::EdgeResource::AMD_EdgeResouceManager_addDevicePtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::std::string& vendor, const ::std::string& model, const ::std::string& adminUrl, const ::std::string& tftpUrl, const ::Ice::Current& c);
    virtual ::TianShanIce::EdgeResource::EdgeDevicePrx openDevice(const ::std::string& name, const ::Ice::Current& c);

	virtual TianShanIce::EdgeResource::EdgePortInfos findRFPortsByRouteName(const std::string& routeName, const Ice::Current & c);
	virtual TianShanIce::EdgeResource::ObjectInfos listRouteNames(const Ice::Current & c);
public:	// impls of ReplicaSubscriber
    virtual void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly, const ::Ice::Current& c);

public:	// impls of ReplicaQuery
    virtual void updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const ::TianShanIce::Replicas& replicaInfos, const ::Ice::Current& c);

public:	// impls of TianShanUtils::TimeoutObj
	virtual void OnTimer(const ::Ice::Current& c);
protected:
	bool listPortsbyRouteName(const std::string routeName, IdentCollection& identPorts);
protected:
	EdgeRMEnv&	_env;
	::TianShanIce::State _state;
	Ice::Identity  _localId;
};

}} // namespace

#endif // __ZQTianShan_EdgeRMImpl_H__
