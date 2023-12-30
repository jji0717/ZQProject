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
// Ident : $Id: EdgeRMCmds.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/EdgeRMCmds.h $
// 
// 9     1/15/14 3:48p Bin.ren
// 
// 8     1/10/14 10:27a Bin.ren
// add DB operation threads
// 
// 7     12/18/13 3:42p Bin.ren
// 
// 6     11/27/13 2:28p Ketao.zhang
// add DiffAllocCmd
// 
// 5     11/13/13 1:32p Bin.ren
// 
// 4     10/11/13 5:49p Bin.ren
// add exportDevice interface
// 
// 3     9/11/13 1:26p Li.huang
// marshal alloction
// 
// 2     5/23/13 4:00p Li.huang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 11    10-01-04 11:44 Li.huang
// modify resource edgeDeviceGroup to edgeDeviceZone
// 
// 10    09-12-18 9:33 Li.huang
// restore
// 
// 10    09-12-10 18:21 Li.huang
// remove deviceOfChannel Index
// 
// 9     09-07-16 9:55 Xiaoming.li
// add compress import function
// 
// 8     09-04-13 16:16 Xiaoming.li
// impl edgedevice, channel, port, allocation list method
// 
// 7     09-04-08 15:05 Xiaoming.li
// 
// 6     09-03-31 9:13 Xiaoming.li
// impl list function
// 
// 5     09-03-26 15:16 Hui.shao
// added NegotiateResourcesCmd
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

#ifndef __ZQTianShan_EdgeRMCmds_H__
#define __ZQTianShan_EdgeRMCmds_H__

#include "../common/TianShanDefines.h"

//xml parser header
#include "XMLPreferenceEx.h"

#include "EdgeRMImpl.h"

namespace ZQTianShan {
namespace EdgeRM {

// -----------------------------
// class BaseCmd
// -----------------------------
///
class BaseCmd : protected ZQ::common::ThreadRequest
{
protected:
	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
    BaseCmd(EdgeRMEnv& env);
	virtual ~BaseCmd();

public:

	void execute(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void) =0;
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:

	EdgeRMEnv&     _env;
};


// -----------------------------
// class ListChannelOfDeviceCmd
// -----------------------------
///
class ListChannelOfDeviceCmd : public BaseCmd
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	ListChannelOfDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeDevice_listChannelsPtr& amdCB, const ::Ice::Identity& deviceIdent, ::Ice::Short portId, const ::TianShanIce::StrValues& expectedMetaData, bool enabledOnly);
	virtual ~ListChannelOfDeviceCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::EdgeResource::AMD_EdgeDevice_listChannelsPtr _amdCB;
	::Ice::Short _portId;
	::Ice::Identity _deviceIdent;
	 ::TianShanIce::EdgeResource::AllocationInfos _result;
	const ::TianShanIce::StrValues _expectedMetaData;
	bool _enabledOnly;
};

class ListAllocationIdsCmd : public BaseCmd
{
public:
	ListAllocationIdsCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeDeviceEx_listAllocationIdsPtr& amdCB, const ::Ice::Identity& deviceIdent);
	virtual ~ListAllocationIdsCmd() {}

protected: // impls of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::EdgeResource::AMD_EdgeDeviceEx_listAllocationIdsPtr _amdCB;
	const ::Ice::Identity& _deviceIdent;
	::TianShanIce::StrValues _result;
};

// -----------------------------
// class PopulateChannelCmd
// -----------------------------
///
class PopulateChannelCmd : public BaseCmd
{
public:
	PopulateChannelCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeDevice_populateChannelsPtr& amdCB, const ::Ice::Identity& deviceIdent, ::Ice::Short portId);
	virtual ~PopulateChannelCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::EdgeResource::AMD_EdgeDevice_populateChannelsPtr _amdCB;
	::TianShanIce::EdgeResource::EdgeChannelInfos _result;
	::Ice::Short _portId;
	 Ice::Identity _deviceIdent;
	 ::TianShanIce::StrValues _expectedMetaData;
};

// -----------------------------
// class QueryReplicaCmd
// -----------------------------
///
class QueryReplicaCmd : public BaseCmd
{
public:
	/// constructor
    QueryReplicaCmd(EdgeRMEnv& env, const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly);
	virtual ~QueryReplicaCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr _amdCB;
	::std::string _category;
	::std::string _groupId;
	bool _localOnly;
};

// -----------------------------
// class UpdateReplicaCmd
// -----------------------------
///
class UpdateReplicaCmd : public BaseCmd
{
public:
	/// constructor
    UpdateReplicaCmd(EdgeRMEnv& env, const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const ::TianShanIce::Replicas& stores);
	virtual ~UpdateReplicaCmd() {}
	
protected: // impls of BaseCmd
	
	virtual int run(void);
	
protected:
	
	::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr _amdCB;
	::TianShanIce::Replicas _edgeReplicas;
};

// -----------------------------
// class ListAllocationsCmd
// -----------------------------
///
class ListAllocationsCmd : public BaseCmd
{
public:

	ListAllocationsCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeResouceManager_listAllocationsPtr& amdCB, const ::std::string& deviceName, ::Ice::Short portId, ::Ice::Short chNum, const ::TianShanIce::StrValues& expectedMetaData);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::EdgeResource::AMD_EdgeResouceManager_listAllocationsPtr _amdCB;
	::std::string _deviceName;
	::Ice::Short _portId, _chNum;
	::TianShanIce::StrValues _expectedMetaData;
	::TianShanIce::EdgeResource::AllocationInfos _allocationInfos;
};



// -----------------------------
// class exportAllocationsCmd
// -----------------------------
///
class exportAllocationsCmd : public BaseCmd
{
public:

	exportAllocationsCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_exportAllocationsPtr amdCB, const ::std::string& deviceName, const ::std::string& since);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::EdgeResource::AMD_EdgeRM_exportAllocationsPtr _amdCB;
	::std::string _deviceName;
	::std::string _since;
	::TianShanIce::EdgeResource::AlloctionValues _allocationValues;
};

//-----------------------------
// class ExportDeviceCmd
//-----------------------------
///
#define DEFAULT_EXPORTFILE	"c:/tianshan/ex_devices.xml"
#define INBANDMARKER	"type=4;pidType=A;pidValue=01EE;dataType=T;insertDuration=10000;data=4002003030"
#define REPORTTRAFICMISMATCH	"0"
#define JITTERBUFFER	"0"
class ExportDevicesCmd: public BaseCmd
{
public:
	ExportDevicesCmd(EdgeRMEnv& env, const TianShanIce::EdgeResource::AMD_EdgeRM_exportDevicesPtr amdCB, const std::string& deviceName, const std::string& xmlDefFile);
protected:
	virtual int run(void);

protected:
	TianShanIce::EdgeResource::AMD_EdgeRM_exportDevicesPtr _amdCB;
	std::string _deviceName;// if _deviceName=NULL, than export all device
	std::string _xmlDefFile;// if _xmlDefFile=NULL, than create xmlFIle under default path
};

class ExportDeviceXMLBodyCmd: public BaseCmd
{
public:
	ExportDeviceXMLBodyCmd(EdgeRMEnv& env, const TianShanIce::EdgeResource::AMD_EdgeRM_exportDeviceXMLPtr amdCB, const std::string& deviceName);
protected:
	virtual int run(void);

protected:
	TianShanIce::EdgeResource::AMD_EdgeRM_exportDeviceXMLPtr _amdCB;
	std::string _deviceName;// if _deviceName=NULL, than export all device
};

// -----------------------------
// class AddDeviceCmd
// -----------------------------
///
class AddDeviceCmd : public BaseCmd
{
public:

	AddDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeResouceManager_addDevicePtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::std::string& vendor, const ::std::string& model, const ::std::string& adminUrl, const ::std::string& tftpUrl);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::EdgeResource::AMD_EdgeResouceManager_addDevicePtr _amdCB;
	::std::string _name, _deviceZone, _vendor, _model;
	::std::string _adminUrl, _tftpUrl;
};

// -----------------------------
// class ImportDeviceCmd
// -----------------------------
///
class ImportDeviceCmd : public BaseCmd
{
public:

	ImportDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDevicePtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::std::string& xmlDefFile, bool bCompress, const ::TianShanIce::Properties& props);
	ImportDeviceCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_importDeviceXMLPtr& amdCB, const ::std::string& name, const ::std::string& deviceZone, const ::TianShanIce::Properties& props, const std::string& xmlBody);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::EdgeResource::AMD_EdgeRM_importDevicePtr _amdCB;
	::TianShanIce::EdgeResource::AMD_EdgeRM_importDeviceXMLPtr _amdCBXML;

	::std::string _name, _deviceZone, _xmlDefFile;
	bool _bCompress;
	::TianShanIce::Properties _props;
	bool _bXMLBody;
	std::string _xmlBody;

//add by lxm for XML file parser
private:
	char strValue[1024];
	::std::string getXMLAttribute(::ZQ::common::XMLPreferenceEx *element, const char *attribute);
	::Ice::Int getXMLIntAttribute(::ZQ::common::XMLPreferenceEx *element, const char *attribute);
};

// -----------------------------
// class NegociateAllocCmd
// -----------------------------
///
class NegociateAllocCmd : public BaseCmd
{
public:

	NegociateAllocCmd(EdgeRMEnv& env, AllocationImpl& alloc, const ::TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr& amdCB);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	AllocationImpl& _alloc;
	::TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr _amdCB;
};

// -----------------------------
// class NegotiateResourcesCmd
// -----------------------------
///
class NegotiateResourcesCmd : public BaseCmd
{
public:

	NegotiateResourcesCmd(EdgeRMEnv& env, const TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr& amdCB, AllocationImpl& alloc);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	TianShanIce::EdgeResource::AMD_Allocation_negotiateResourcesPtr _amdCB;
	AllocationImpl& _alloc;
};

// -----------------------------
// class DiffAllocCmd
// -----------------------------
class DiffAllocCmd : public BaseCmd
{
public:
	DiffAllocCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeRM_diffAllocationsPtr& amdCB, const std::string& deviceName, const ::TianShanIce::StrValues&allocIds);

protected:
	virtual int run(void);

	::TianShanIce::EdgeResource::AMD_EdgeRM_diffAllocationsPtr _amdCB;
	 std::string _deviceName;
	 ::TianShanIce::StrValues _remoteAllocIds;
 // test data for get the total count and time of one sync

public:
	static int _testDeviceNum;
	static int _testGetIdsTime;
	static int _testCompareTime;
	static int _testGetAllocTime;
	static int _testTotalTime;

	static int _testGetIdsCount;
	static int _testGetAllocCount;
};

/*
// -----------------------------
// class CommitAllocCmd
// -----------------------------
///
class CommitAllocCmd : public BaseCmd
{
public:

	CommitAllocCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeChannel_commitPtr& amdCB, EdgeChannelImpl& channel, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, const ::TianShanIce::SRM::ResourceMap& resources);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::EdgeResource::AMD_EdgeChannel_commitPtr _amdCB;
	EdgeChannelImpl& _channel;
	::TianShanIce::EdgeResource::AllocationPrx _alloc;
	::TianShanIce::SRM::ResourceMap _resources;

};

// -----------------------------
// class WithdrawAllocCmd
// -----------------------------
///
class WithdrawAllocCmd : public BaseCmd
{
public:

	WithdrawAllocCmd(EdgeRMEnv& env, const ::TianShanIce::EdgeResource::AMD_EdgeChannel_withdrawPtr& amdCB, EdgeChannelImpl& channel, const ::TianShanIce::EdgeResource::AllocationPrx& alloc, ::Ice::Int PN);

protected: // overwrite of BaseCmd

	virtual int run(void);

protected:

	::TianShanIce::EdgeResource::AMD_EdgeChannel_withdrawPtr _amdCB;
	EdgeChannelImpl& _channel;
	::TianShanIce::EdgeResource::AllocationPrx _alloc;
	::Ice::Int _PN;
};
*/
}} // namespace

#endif // __ZQTianShan_EdgeRMCmds_H__
