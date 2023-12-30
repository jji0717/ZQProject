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
// Ident : $Id: ICPMHelperObj.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ICPHelper.h $
// 
// 6     7/16/15 12:02p Li.huang
// 
// 5     4/29/15 10:21a Zhiqiang.niu
// add Group CPH_AquaLib
// 
// 4     7/15/13 11:22a Li.huang
// add updateScheduledTime()
// 
// 3     3/21/11 2:17p Li.huang
// fix bug 13454 1#
// 
// 2     10-12-15 14:13 Li.huang
// use new bufferpool
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 13    09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 14    09-05-05 13:20 Jie.zhang
// 
// 13    09-04-14 19:24 Jie.zhang
// change the memory allocator
// 
// 12    09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 11    08-12-19 15:56 Yixin.tian
// modify interface BOOL  InitCPH(...)  to  bool InitCPH(...)
// 
// 10    08-11-18 11:09 Jie.zhang
// merge from TianShan1.8
// 
// 10    08-10-24 14:51 Jie.zhang
// changed the parameter meaning for teminate()
// 
// 9     08-06-02 16:39 Jie.zhang
// 
// 8     08-04-09 11:48 Hui.shao
// added ProvisionCost
// 
// 7     08-03-27 16:53 Jie.zhang
// 
// 7     08-03-07 18:14 Jie.zhang
// 
// 6     08-02-28 16:17 Jie.zhang
// 
// 5     08-02-20 16:14 Jie.zhang
// 
// 4     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 3     08-02-13 17:48 Hui.shao
// 
// 2     08-01-24 16:19 Hui.shao
// 
// 1     08-01-22 15:35 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_ICPHelper_H__
#define __ZQTianShan_ICPHelper_H__

#include "../common/TianShanDefines.h"
#include "CPE.h"
#include "Exception.h"
#include "Locks.h"
#include "DynSharedObj.h"
//#include "IMemAlloc.h"
#include "BufferPool.h"
#include "IMethodCost.h"


namespace ZQ{
	namespace common{
		class NativeThreadPool;
	}
}

namespace ZQTianShan {
namespace ContentProvision {

// #define PathTicketPD_Field(_FIELD)  (::TianShanIce::Transport::PathTicketPDPrefix + #_FIELD).c_str()

#define CPH_FILENAME_PREFIX		"CPH_"
#define GROUP_C2PROPAGATION   "CPH_C2Propagation"
#define GROUP_NASCOPY         "CPH_NASCOPY"
#define GROUP_NPVR            "CPH_NPVR"
#define GROUP_RDS             "CPH_RRDS"
#define GROUP_RTFRDS          "CPH_RTFRDS"
#define GROUP_RTI             "CPH_RTI"
#define GROUP_RTINAS          "CPH_RTINAS"
#define GROUP_CDN             "CPH_CDN"
#define GROUP_PCAP            "CPH_PCAP"
#define GROUP_AQUAREC         "CPH_AquaRec"
#define GROUP_AQUALIB         "CPH_AquaLib"
#define GROUP_CSI             "CPH_CSI"
class ICPHelper;
class ICPHManager;
class IPushSource;
class ICPHSession;

/*

// -----------------------------
// interface IProvisionSessBind
// -----------------------------
/// the per-sesson callback bind interface for a helper session to access the provision session in the engine
class IProvisionSessBind
{
// 	const char* getSessionId() const;
	const char* getMethodType() const;
	::TianShanIce::ContentProvision::ProvisionState getState() const;

	/// event will be triggered when a content's provision status is changed
	///@param[in] netId the net Id of the content store where the content is being provisioned
	///@param[in] contentname content name that can be used to access Content thru ContentStore::openContent()
	virtual void setState(const ::TianShanIce::ContentProvision::ProvisionState state) =0;

	/// event will be fired when a content provision is processing with progress
	///@param[in] processed the processed provision units in the current step
	///@param[in] total the total provision units in the current step. The unit can percentage, or file size in KB, etc
	///@param[in] stepNo the sequence number of the current step
	///@param[in] totalSteps the total step number must be performed for this provision procedure
	virtual void updateProgress(const ::Ice::Long processed, const ::Ice::Long total) =0;

	/// query for a content get a provision parameter
	///@param[in] key - the key to query
	///@return - the value of the parameter
	virtual bool getProvisionDestination(std::string& netId, std::string& volume, std::string& contentName) const =0;
	virtual bool getProvisionSource(std::string& url, std::string& srcContentType) const =0;

	virtual const ::TianShanIce::Storage::ContentPrx& getProvisionDestination() const =0;
	virtual const ::TianShanIce::Storage::ContentPrx& getProvisionSource() const =0;

	virtual void attachHelperSession(IHelperSession* pHelperSess) const =0;

	/// get a provision parameter
	///@param[in] key - the key to query
	///@return - the value of the parameter
	virtual const std::string& get(const std::string& key) const =0;

	/// update a provision parameter
	///@param[in] key - the parameter name to update
	///@param[in] value - the new value to update
	virtual void set(const std::string& key, const std::string& value) =0;

	//TODO get all attributes
	
};

*/


// -----------------------------
// interface ICPHManager
// -----------------------------
/// the interface a Provision helper object to access the engine
class ICPHManager
{
public:
	virtual ~ICPHManager(){}
	/// register a Content Provision Method, called by a CPMHelper plugin during InitCPMHelper() entry
	///@param[in] methodType the string type of Content Provision Method associated by this helper object
	///@param[in] helper the CP helper instance
	///@return    pointer to the helper instance if succeeded, otherwise NULL
	virtual bool registerHelper(const char* methodType, ICPHelper* helper, void* pCtx) =0;

	/// unregister a Content Provision Method Helper, usually called by a CPMHelper plugin during UnInitCPMHelper() entry
	///@param[in] type	 the string type of Content Provision Method about to unregister
	///@return    true if unregister successfully
	virtual bool unregisterHelper(const char* methodType, void* pCtx) =0;

	/// find a Content Provision Method Helper by method type
	///@param[in] type	 the string type of Content Provision Method to look for
	///@return    pointer to the helper instance if succeeded, otherwise NULL
	virtual ICPHelper* findHelper(const char* methodType) =0;

    ///list all supported storage link types
	///@return a list of storage link string types
    virtual ::TianShanIce::StrValues listSupportedMethods() =0;

	virtual bool registerHelperSession(const char* provisionId, ICPHSession* pSess) =0;
	virtual void unregisterHelperSession(const char* provisionId) =0;

	virtual const char* getConfigDir() const =0;
	virtual const char* getLogDir() const =0;

//	virtual IMemAlloc* getMemoryAllocator() = 0;
	virtual ZQ::common::BufferPool* getMemoryAllocator() = 0;
	virtual ZQ::common::Log* getLogger() const =0;

	virtual ZQ::common::NativeThreadPool& getThreadPool() const =0;
	
	// access the ProvisionSession objects in CPE
	// ---------------------------------------------
	virtual const char* getMethodType(const char* provisionId) const =0;

	
	virtual IPushSource* findPushSource(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey) const =0;
/*	{
// 		PushSource* source = _env._ftpServer.findPushSource(key);
// 		if (NULL != source)
// 			return source;
// 		
// 		source = _env._httpServer.findPushSource(key);
// 		if (NULL != source)
// 			return source;
//		return NULL;

		return _env._pushTriggers.findPushSource(key);
	}
*/

	/*
	virtual ::TianShanIce::ContentProvision::ProvisionState getState(const char* provisionId) const =0;

	/// event will be triggered when a content's provision status is changed
	///@param[in] netId the net Id of the content store where the content is being provisioned
	///@param[in] contentname content name that can be used to access Content thru ContentStore::openContent()
	virtual void setState(const char* provisionId, const ::TianShanIce::ContentProvision::ProvisionState state) =0;
	
	/// event will be fired when a content provision is processing with progress
	///@param[in] processed the processed provision units in the current step
	///@param[in] total the total provision units in the current step. The unit can percentage, or file size in KB, etc
	///@param[in] stepNo the sequence number of the current step
	///@param[in] totalSteps the total step number must be performed for this provision procedure
	virtual void updateProgress(const char* provisionId, const ::Ice::Long processed, const ::Ice::Long total) =0;

	/// get a provision parameter
	///@param[in] key - the key to query
	///@return - the value of the parameter
	virtual const char* get(const char* provisionId, const char* key) const =0;
	
	/// update a provision parameter
	///@param[in] key - the parameter name to update
	///@param[in] value - the new value to update
	virtual bool set(const char* provisionId, const char* key, const char* value) =0;
*/	
	//TODO get all attributes

	///register  method info
	///@param[in] methodName - method name
	///@param[in] maxSession - max Session
	///@param[in] maxBw - max bandwidth
	///@param[in] groupId - method is to belong to which group
	///@param[in] flags - reserved paramter
	virtual void  registerLimitation(const char* methodName,int maxSession, int64 maxBw, const char*  groupId, uint32 flags) = 0;

	///@param[in] groupId - group id
	///@return - the MaxSession count of groupid, if groupId ="", return sum-up maxSession
	virtual const int  getMaxSessioOfGroup(const char* groupId) const = 0;

};

// -----------------------------
// interface ICPHSession
// -----------------------------
/// A per plugin interface exported from a MethodHelper object. The upper layer engine will invoke this
/// interface to drive the plugin to perform content provisioning
class ICPHSession
{
public:
	virtual ~ICPHSession(){}
	virtual bool preLoad() =0;

	virtual bool prime() =0;
	
	virtual void execute() =0;

	//
	// the helper ask provision to stop provision, and tell the final status is success or failure,
	// bProvisionSuccess true for success, false for failure
	//
	virtual void terminate(bool bProvisionSuccess=true) =0;

	virtual bool getProgress(Ice::Long& offset, Ice::Long& total) =0;
	virtual bool isStreamable() const =0;

	virtual void setErrorInfo(int nCode, const char* szErrMsg) = 0;
	virtual const char* getErrorMsg() = 0;
	virtual int getErrorCode() = 0;

	///add for CPH_AquaRec
	virtual void updateScheduledTime(const ::std::string& startTimeUTC, const ::std::string& endTimeUTC)= 0;
};

// -----------------------------
// plugin facet ICPHelper
// -----------------------------
/// A per plugin interface exported from a MethodHelper object. The upper layer engine will invoke this
/// interface to drive the plugin to perform content provisioning
class ICPHelper : public MethodCostCol
{
public:
	virtual ~ICPHelper(){}
    ///validate a potential ProvisionSession about to setup
	///@param[in] sess access to the ProvisionSession about to setup
	///@param[out] schema the collection of schema definition
	///@return true if succeeded
	virtual bool validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
		throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource) =0;

    /// query the current load information of a method type
	///@param[in] methodType to specify the method type to query
	///@param[out] allocatedKbps the current allocated bandwidth in Kbps
	///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
	///@param[out] sessions the current running session instances
	///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
	///@return true if the query succeeded
	virtual bool getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins) =0;

	virtual ICPHSession* createHelperSession(const char* methodType, const ::TianShanIce::ContentProvision::ProvisionSessionExPtr& pSess) =0;
	virtual ICPHSession* find(const char* provisionId) =0;
};

// -----------------------------
// DLL entries for a CPHelperObject
// -----------------------------
bool InitCPH(ICPHManager* pEngine, void* pCtx = NULL);
void UninitCPH(ICPHManager* pEngine, void* pCtx = NULL);

class CPHFacet : public ZQ::common::DynSharedFacet
{
	// declare this Facet object as a child of DynSharedFacet
	DECLARE_DSOFACET(CPHFacet, DynSharedFacet);

	// declare the API prototypes
	DECLARE_PROC(bool, InitCPH, (ICPHManager* mgr, void* pCtx));
	DECLARE_PROC(void, UninitCPH, (ICPHManager* pEngine, void* pCtx));

	// map the external APIs
	DSOFACET_PROC_BEGIN();
		DSOFACET_PROC(InitCPH);
		DSOFACET_PROC(UninitCPH);
	DSOFACET_PROC_END();
};


}} // namespace

#endif // __ZQTianShan_ICPHelper_H__
