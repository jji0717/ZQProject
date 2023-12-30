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
// Ident : $Id: ProvisionFactory.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/ProvisionFactory.h $
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
// 7     09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 7     09-04-14 19:25 Jie.zhang
// improve the buffer queue manager performance
// 
// 6     08-12-12 13:45 Yixin.tian
// 
// 5     08-11-18 11:09 Jie.zhang
// merge from TianShan1.8
// 
// 5     08-08-21 18:18 Xia.chen
// 
// 4     08-03-27 16:53 Jie.zhang
// 
// 4     08-03-07 18:14 Jie.zhang
// 
// 3     08-03-04 17:15 Jie.zhang
// 
// 2     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_ProvisionFactory_H__
#define __ZQTianShan_ProvisionFactory_H__

#include "ZQ_common_conf.h"
#include "TsContentProv.h"
#include "Exception.h"
#include "Locks.h"

#include "ICPHelper.h"
#include "CPEImpl.h"
#include "BufferPool.h"
//class QueueBufMgr;
namespace ZQTianShan {
namespace CPE {

class CPEEnv;
class ProvisionSessImpl;
typedef struct
{
	std::string methodName;
	int         maxSessions;
	int64       maxBw;
}MethodInfo;
typedef std::vector<MethodInfo>MethodInfos;
typedef std::map<std::string, MethodInfos>MethodOfGroupsMap;
// -----------------------------
// ProvisionFactory
// -----------------------------
/// an internal module of CPE. it is a management of multiple CPHO
class ProvisionFactory : public ::Ice::LocalObject, public ZQTianShan::ContentProvision::ICPHManager
{
public:
	
	ProvisionFactory(CPEEnv& env);
	virtual ~ProvisionFactory();
	
	ZQTianShan::ContentProvision::ICPHSession* resolve(::TianShanIce::ContentProvision::ProvisionSessionExPtr pSess);
    ZQTianShan::ContentProvision::ICPHSession* findHelperSession(const char* provisionId);
	
	int populate();
    bool findHelperInfo(const char* methodType, ::ZQ::common::DynSharedObj::ImageInfo& info);
	
	// impl of ICPHManager
	::TianShanIce::StrValues listSupportedMethods();
	virtual bool registerHelper(const char* methodType, ZQTianShan::ContentProvision::ICPHelper* helper, void* pCtx);
	virtual bool unregisterHelper(const char* methodType, void* pCtx);
	virtual ZQTianShan::ContentProvision::ICPHelper* findHelper(const char* methodType);
	virtual ZQTianShan::ContentProvision::IPushSource* findPushSource(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey) const;
	virtual bool registerHelperSession(const char* provisionId, ZQTianShan::ContentProvision::ICPHSession* pSess);
	virtual void unregisterHelperSession(const char* provisionId);

	//virtual IMemAlloc* getMemoryAllocator();
	virtual ZQ::common::BufferPool* getMemoryAllocator();
	
	virtual const char* getConfigDir() const;
	virtual const char* getLogDir() const;
	
	virtual ZQ::common::Log* getLogger() const;

	virtual ZQ::common::NativeThreadPool& getThreadPool() const;

	virtual const char* getMethodType(const char* provisionId) const;

	virtual void  registerLimitation(const char* methodName,int maxSession, int64 maxBw, const char*  groupId, uint32 flags);

	virtual const int  getMaxSessioOfGroup(const char* groupId) const;

/*
	virtual ::TianShanIce::ContentProvision::ProvisionState getState(const char* provisionId) const;
	virtual void setState(const char* provisionId, const ::TianShanIce::ContentProvision::ProvisionState state);
	virtual void updateProgress(const char* provisionId, const ::Ice::Long processed, const ::Ice::Long total);

	virtual const char* get(const char* provisionId, const char* key) const;
	virtual bool set(const char* provisionId, const char* key, const char* value);
*/

protected:

	ZQ::common::DynSharedObj* loadPlugin(const char* filename);

protected:

	CPEEnv& _env;

	typedef struct _CPHNode
	{
		ZQTianShan::ContentProvision::ICPHelper*    pPlugin;
		std::string   fnPlugin;
	} CPHNode;

	typedef std::map <std::string, CPHNode> MethodMap; // map of provision method type to helper 
	MethodMap _methodMap;
	ZQ::common::Mutex   _lockMethodMap;

	typedef std::map <std::string, ZQTianShan::ContentProvision::ICPHSession* > HelperSessionMap; // map of <provisionId, HelperSession>
	HelperSessionMap _sessMap;
	ZQ::common::Mutex   _lockSessMap;


	ZQ::common::Mutex   _lockmethodofgroupsMap;
	MethodOfGroupsMap  _methodofgroups;

private:
	
	typedef std::map<std::string, ZQ::common::DynSharedObj* > SoMap; ///< a map of link-type to StorageLinkHelper object
	SoMap  _soMap;
	ZQ::common::Mutex   _lockSoMap;
//	QueueBufMgr*		_pAlloc;
	ZQ::common::BufferPool*		_pAlloc;

};


}} // namespace

#endif // __ZQTianShan_ProvisionFactory_H__
