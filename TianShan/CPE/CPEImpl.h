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
// Ident : $Id: CPEImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPEImpl.h $
// 
// 2     1/12/16 9:31a Dejian.fei
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 24    10-04-07 15:24 Jie.zhang
// add parameter on updateProgress
// 
// 23    09-07-24 15:06 Xia.chen
// change the second parameter for getExportUrl from contentName to
// contentKey
// 
// 22    09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 22    09-05-13 21:52 Jie.zhang
// 
// 21    09-02-06 10:41 Jie.zhang
// 
// 20    09-01-21 14:30 Jie.zhang
// the canceled provision would not send "stop event"
// 
// 19    08-12-12 13:45 Yixin.tian
// modify for Linux OS
// 
// 18    08-11-18 10:59 Jie.zhang
// merge from TianShan1.8
// 
// 19    08-09-11 17:42 Jie.zhang
// getExportUrl parameter changed
// 
// 18    08-08-14 18:46 Jie.zhang
// add getExportUrl
// 
// 17    08-07-11 13:50 Jie.zhang
// add export content support
// 
// 16    08-05-14 22:07 Jie.zhang
// 
// 15    08-04-09 18:16 Jie.zhang
// 
// 14    08-04-09 15:37 Hui.shao
// impl listMethods
// 
// 13    08-04-09 11:48 Hui.shao
// added ProvisionCost
// 
// 12    08-04-02 15:47 Hui.shao
// per CPC ICE changes
// 
// 11    08-03-28 16:11 Build
// 
// 10    08-03-27 16:15 Jie.zhang
// 
// 10    08-03-17 19:56 Jie.zhang
// 
// 9     08-02-28 16:17 Jie.zhang
// 
// 8     08-02-21 15:08 Hui.shao
// added paged list
// 
// 7     08-02-20 16:16 Jie.zhang
// 
// 6     08-02-18 20:59 Hui.shao
// replaced priviate data with property
// 
// 5     08-02-18 18:49 Jie.zhang
// changes check in
// 
// 4     08-02-18 18:44 Hui.shao
// 
// 3     08-02-14 18:47 Hui.shao
// impled ProvisionSessionBind callbacks
// 
// 2     08-02-14 16:26 Hui.shao
// added ami callbacks
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#ifndef __ZQTianShan_CPEImpl_H__
#define __ZQTianShan_CPEImpl_H__

#include "../common/TianShanDefines.h"
#include "ZQ_common_conf.h"
#include "Locks.h"

#include "CPE.h"
#include "ICPHelper.h"

extern "C"
{
#include <sys/timeb.h>
}
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace CPE {

#define DEFUALT_SITENAME "."

class CPEEnv;

// -----------------------------
// service CPEImpl
// -----------------------------
class CPEImpl : public ::TianShanIce::ContentProvision::ContentProvisionEngine
{
public:
    CPEImpl(CPEEnv& env);
	virtual ~CPEImpl();

public:	// impls of BaseService
	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// impls of ContentProvisionService
    virtual ::std::string getNetId(const ::Ice::Current& c) const;
    virtual ::TianShanIce::ContentProvision::ProvisionSessionPrx createSession(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, const ::std::string& methodType, ::TianShanIce::ContentProvision::ProvisionOwnerType ownerType, const ::TianShanIce::Storage::ContentPrx& primaryContent, const ::TianShanIce::ContentProvision::ProvisionSessionBindPrx& owner, const ::Ice::Current& c);
    virtual ::TianShanIce::ContentProvision::ProvisionSessionPrx openSession(const ::TianShanIce::ContentProvision::ProvisionContentKey& contentKey, const ::Ice::Current& c);

	virtual void listSessions_async(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listSessionsPtr&, const ::std::string& methodType, const ::TianShanIce::StrValues& paramNames, const ::std::string& startId, ::Ice::Int maxCount, const ::Ice::Current& c) const;
    virtual void listMethods_async(const ::TianShanIce::ContentProvision::AMD_ContentProvisionService_listMethodsPtr&, const ::Ice::Current& = ::Ice::Current()) const;	
	virtual ::TianShanIce::ContentProvision::ExportMethods listExportMethods(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual ::std::string getExportURL(const ::std::string&, const ::TianShanIce::ContentProvision::ProvisionContentKey&, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::Ice::Int& permittedBitrate, const ::Ice::Current& = ::Ice::Current());

protected:

	CPEEnv&	_env;

};

// -----------------------------
// class ProvisionSessImpl
// -----------------------------
//class ProvisionSessImpl : public TianShanIce::ContentProvision::ProvisionSessionEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class ProvisionSessImpl : public TianShanIce::ContentProvision::ProvisionSessionEx, public ICEAbstractMutexRLock
{
	friend class CPEImpl;

public:

    ProvisionSessImpl(CPEEnv& env);

	typedef ::IceInternal::Handle< ProvisionSessImpl> Ptr;

	static Ice::Identity ContentKeyToIdent(const ::TianShanIce::ContentProvision::ProvisionContentKey& key);
	static const char* typeStr(const ::TianShanIce::ContentProvision::ProvisionType type);
	static const char* startTypeStr(const ::TianShanIce::ContentProvision::StartType stype);

public: // impl of TianShanIce::ContentProvision::ProvisionSessionEx

    virtual ::std::string getProvisionId(const ::Ice::Current& c) const;
    virtual ::Ice::Long getBandwidth(const ::Ice::Current& c) const;

    virtual void updateProgress(::Ice::Long processed, ::Ice::Long total, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void setSessionType(::TianShanIce::ContentProvision::ProvisionType ptype, ::TianShanIce::ContentProvision::StartType stype, const ::Ice::Current& c);
	
    virtual void notifyStarted(const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void notifyStopped(bool errorOccurred, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void notifyStreamable(bool streamable, const ::Ice::Current& c);
	
    virtual void OnTimer(const ::Ice::Current& c);
    virtual void OnRestore(const ::Ice::Current& c);
	
    virtual void forceToStart(const ::Ice::Current& c);

	virtual void cancel(::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
public: // impl of TianShanIce::ContentProvision::ProvisionSession
	
    virtual ::TianShanIce::ContentProvision::ProvisionState getState(const ::Ice::Current& c) const;
    virtual ::TianShanIce::ContentProvision::ProvisionContentKey getContentKey(const ::Ice::Current& c) const;
    virtual ::TianShanIce::ContentProvision::ProvisionType getProvisionType(const ::Ice::Current& c) const;
    virtual ::std::string getMethodType(const ::Ice::Current& c) const;

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& c) const;
    virtual void setProperty(const ::std::string& key, const ::std::string& val, const ::Ice::Current& c);

	virtual void setTrickSpeedCollection(const ::TianShanIce::ContentProvision::TrickSpeedCollection& col, const ::Ice::Current& c);

    virtual void addResource(::TianShanIce::SRM::ResourceType type, const ::TianShanIce::ValueMap& resData, const ::Ice::Current& c);
    virtual ::TianShanIce::SRM::ResourceMap getResources(const ::Ice::Current& c) const;

    virtual void getScheduledTime(::std::string& startTimeUTC, ::std::string& endTimeUTC, const ::Ice::Current& c) const;
    virtual void updateScheduledTime(const ::std::string& startTimeUTC, const ::std::string& endTimeUTC, const ::Ice::Current& c);

    virtual void queryProgress(::Ice::Long& processed, ::Ice::Long& total, const ::Ice::Current& c) const;
	
    virtual void setup_async(const ::TianShanIce::ContentProvision::AMD_ProvisionSession_setupPtr&, const ::std::string& startTimeUTC, const ::std::string& endTimeUTC, const ::Ice::Current& c);
    virtual void commit(const ::Ice::Current& c);

    virtual bool isStreamable(const ::Ice::Current& c);

    virtual ::TianShanIce::ContentProvision::ProvisionSubscribeMask getSubscribeMask(const ::Ice::Current& c) const;
    virtual void setSubscribeMask(const ::TianShanIce::ContentProvision::ProvisionSubscribeMask& mask, const ::Ice::Current& c);

	void notifyError(int nErrCode=0, const char* szErrMsg=0);

	bool				_bSucc;

	// the function begin with "_" means no lock
	virtual ::Ice::Long _getBandwidth() const;

protected:
	// the function begin with "_" means no lock

	virtual ::TianShanIce::Properties _getProperties() const;

	//without lock, same function with setProperty
	virtual void _setProperty(const ::std::string& key, const ::std::string& val);


	bool				_isProvisionCanceled();
	void				_setProvisionCanceled();

protected:

	CPEEnv& _env;
	::Ice::Long _progressProcessed, _progressTotal, _progressLatest;
	struct timeb			_progressLatestStamp;
	ZQTianShan::ContentProvision::ICPHSession* getCPHSession() const;
};

#if  ICE_INT_VERSION / 100 >= 306
class OnProvisionStateCB : public IceUtil::Shared
{
public:
	OnProvisionStateCB(CPEEnv& env);
private:
	void handleException(const std::string& name, const Ice::Exception&);
public:
	void OnProvisionStarted(const Ice::AsyncResultPtr&);	
	void OnProvisionStopped(const Ice::AsyncResultPtr&);
	void OnProvisionProgress(const Ice::AsyncResultPtr&);
	void OnProvisionStateChanged(const Ice::AsyncResultPtr&);
	void OnProvisionStreamable(const Ice::AsyncResultPtr&);
	void OnProvisionDestroyed(const Ice::AsyncResultPtr&);	

protected:
	CPEEnv& _env;
};
typedef IceUtil::Handle<OnProvisionStateCB> OnProvisionStateCBPtr;
#else
// -----------------------------
// callback OnProvisionStartedAmiCBImpl
// -----------------------------
class OnProvisionStartedAmiCBImpl : public TianShanIce::ContentProvision::AMI_ProvisionSessionBind_OnProvisionStarted
{
public:
    OnProvisionStartedAmiCBImpl(CPEEnv& env);
    virtual void ice_response() {} // do nothing
    virtual void ice_exception(const ::Ice::Exception& ex);
	
protected:
	CPEEnv& _env;
};

// -----------------------------
// callback OnProvisionProgressAmiCBImpl
// -----------------------------
class OnProvisionStoppedAmiCBImpl : public TianShanIce::ContentProvision::AMI_ProvisionSessionBind_OnProvisionStopped
{
public:
    OnProvisionStoppedAmiCBImpl(CPEEnv& env);
    virtual void ice_response() {} // do nothing
    virtual void ice_exception(const ::Ice::Exception& ex);
	
protected:
	CPEEnv& _env;
};

// -----------------------------
// callback OnProvisionProgressAmiCBImpl
// -----------------------------
class OnProvisionProgressAmiCBImpl : public TianShanIce::ContentProvision::AMI_ProvisionSessionBind_OnProvisionProgress
{
public:
    OnProvisionProgressAmiCBImpl(CPEEnv& env);
    virtual void ice_response() {} // do nothing
    virtual void ice_exception(const ::Ice::Exception& ex);

protected:
	CPEEnv& _env;
};

// -----------------------------
// callback OnProvisionStateChangedAmiCBImpl
// -----------------------------
class OnProvisionStateChangedAmiCBImpl : public TianShanIce::ContentProvision::AMI_ProvisionSessionBind_OnProvisionStateChanged
{
public:
    OnProvisionStateChangedAmiCBImpl(CPEEnv& env);
    virtual void ice_response() {} // do nothing
    virtual void ice_exception(const ::Ice::Exception& ex);
	
protected:
	CPEEnv& _env;
};

// -----------------------------
// callback OnProvisionStreamableAmiCBImpl
// -----------------------------
class OnProvisionStreamableAmiCBImpl : public TianShanIce::ContentProvision::AMI_ProvisionSessionBind_OnProvisionStreamable
{
public:
    OnProvisionStreamableAmiCBImpl(CPEEnv& env);
    virtual void ice_response() {} // do nothing
    virtual void ice_exception(const ::Ice::Exception& ex);
	
protected:
	CPEEnv& _env;
};

// -----------------------------
// callback OnProvisionDestroyedAmiCBImpl
// -----------------------------
class OnProvisionDestroyedAmiCBImpl : public TianShanIce::ContentProvision::AMI_ProvisionSessionBind_OnProvisionDestroyed
//class OnProvisionDestroyedAmiCBImpl : public TianShanIce::ContentProvision::Callback_ProvisionSessionBind_OnProvisionDestroyed_Base
{
public:
    OnProvisionDestroyedAmiCBImpl(CPEEnv& env);
    virtual void ice_response() {} // do nothing
    virtual void ice_exception(const ::Ice::Exception& ex);
	
protected:
	CPEEnv& _env;
};
#endif

}} // namespace

#endif // __ZQTianShan_CPEImpl_H__
