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
// Ident : $Id: PathManagerImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathManagerImpl.h $
// 
// 7     1/11/16 4:56p Dejian.fei
// 
// 6     7/09/14 6:25p Zhiqiang.niu
// add function importStatus to set the flags of the link
// 
// 5     2/20/14 5:25p Hui.shao
// merged from V1.16
// 
// 5     2/13/14 4:09p Hui.shao
// declare Link::status() as readonly to reduce disk io
// 
// 4     3/05/13 11:52a Hui.shao
// enh#17725 Weiwoo to specially deal with "$Standby" StorageLinks
// 
// 3     12/21/12 11:25a Hui.shao
// enable/disable streamlink
// 
// 2     3/09/11 4:26p Hongquan.zhang
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 53    09-09-22 17:15 Xiaohui.chai
// implement the importXml2()
// 
// 52    09-09-07 13:58 Xiaohui.chai
// add error detail for import failure
// 
// 51    08-12-29 12:21 Hui.shao
// 
// 50    08-10-09 16:52 Xiaohui.chai
// update error handling in importXml()
// 
// 49    08-08-14 14:57 Hui.shao
// merged from 1.7.10
// 
// 48    08-07-08 13:37 Hui.shao
// fixed for ICE 3.3 new syntax
// 
// 47    08-03-24 17:46 Build
// check in for 64bit build
// 
// 46    08-01-03 15:52 Hui.shao
// 
// 45    07-09-21 13:02 Yixin.tian
// 
// 44    07-09-18 12:55 Hongquan.zhang
// 
// 42    07-08-15 16:58 Hui.shao
// added dumpDot(), dumpXml(), importXml()
// 
// 41    07-06-06 16:07 Hongquan.zhang
// 
// 40    07-03-22 17:41 Hongquan.zhang
// 
// 39    07-03-22 17:24 Hongquan.zhang
// 
// 38    07-03-14 12:33 Hongquan.zhang
// 
// 37    07-01-05 18:13 Hongquan.zhang
// 
// 36    07-01-05 10:59 Hongquan.zhang
// 
// 35    06-12-25 12:23 Hongquan.zhang
// 
// 34    06-12-18 14:02 Hongquan.zhang
// 
// 33    06-12-13 18:47 Hongquan.zhang
// 
// 32    06-09-19 11:45 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_ADPIceImpl_H__
#define __ZQTianShan_ADPIceImpl_H__

#include <fstream>
#include "../common/TianShanDefines.h"
#include "TsPathAdmin.h"
#include "PathFactory.h"
#include "PathSvcEnv.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <XMLPreferenceEx.h>
#include <set>

namespace ZQTianShan {
namespace AccreditedPath {

// -----------------------------
// class StorageLinkImpl
// -----------------------------
//class StorageLinkImpl : public TianShanIce::Transport::StorageLinkEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class StorageLinkImpl : public TianShanIce::Transport::StorageLinkEx, public ICEAbstractMutexRLock
{
public:

    StorageLinkImpl(PathSvcEnv& env);

    virtual ::Ice::Identity getIdent(const ::Ice::Current& c) const;
    virtual ::std::string	getType(const ::Ice::Current& c) const;

    virtual ::TianShanIce::Transport::Storage getStorageInfo(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Transport::Streamer getStreamerInfo(const ::Ice::Current& c) const;

    virtual bool setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& val, const ::Ice::Current& c);
    virtual ::TianShanIce::ValueMap getPrivateData(const ::Ice::Current& c) const;

    virtual void destroy(const Ice::Current& c);

	virtual ::std::string getStorageId(const ::Ice::Current&) const { return storageId; }
    virtual ::std::string getStreamerId(const ::Ice::Current&) const { return streamerId; }

    virtual ::Ice::Int getRevision(const ::Ice::Current& c) const;
    virtual void updateRevision(::Ice::Int newRevision, const ::Ice::Current& c);
    virtual bool updatePrivateData(const ::TianShanIce::ValueMap& newValues, const ::Ice::Current& c);
    virtual void enableLink(bool, const ::Ice::Current& c);
    virtual int  status(const ::Ice::Current& c) const;
	virtual void importStatus(::Ice::Int status, const ::Ice::Current& c);

	typedef ::IceInternal::Handle< StorageLinkImpl> Ptr;

protected:

	PathSvcEnv& _env;
};

// -----------------------------
// class StreamLinkImpl
// -----------------------------
//class StreamLinkImpl : public TianShanIce::Transport::StreamLinkEx, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class StreamLinkImpl : public TianShanIce::Transport::StreamLinkEx, public ICEAbstractMutexRLock
{
public:

    StreamLinkImpl(PathSvcEnv& env);

    virtual ::Ice::Identity getIdent(const ::Ice::Current& c) const;
    virtual ::std::string	getType(const ::Ice::Current& c) const;

    virtual ::TianShanIce::Transport::ServiceGroup getServiceGroupInfo(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Transport::Streamer getStreamerInfo(const ::Ice::Current& c) const;

    virtual bool setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& val, const ::Ice::Current& c);
    virtual ::TianShanIce::ValueMap getPrivateData(const ::Ice::Current& c) const;

    virtual void destroy(const Ice::Current& c);

    virtual ::std::string getStreamerId(const ::Ice::Current&) const { return streamerId; }
	virtual ::Ice::Int getServiceGroupId(const ::Ice::Current&) const { return servicegroupId; }

    virtual ::Ice::Int getRevision(const ::Ice::Current& c) const;
    virtual void updateRevision(::Ice::Int newRevision, const ::Ice::Current& c);
    virtual bool updatePrivateData(const ::TianShanIce::ValueMap& newValues, const ::Ice::Current& c);
    virtual void enableLink(bool, const ::Ice::Current& c);
    virtual int  status(const ::Ice::Current& c) const;
	virtual void importStatus(::Ice::Int status, const ::Ice::Current& c);

	typedef ::IceInternal::Handle< StreamLinkImpl> Ptr;

protected:

	PathSvcEnv& _env;
};

// -----------------------------
// class ADPathTicketImpl
// -----------------------------
//class ADPathTicketImpl : public TianShanIce::Transport::PathTicket, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class ADPathTicketImpl : public TianShanIce::Transport::PathTicket, public ICEAbstractMutexRLock
{
	friend class AccreditedPathsImpl;

public:

    ADPathTicketImpl(PathSvcEnv& env);
	~ADPathTicketImpl();

    virtual ::Ice::Identity getIdent(const ::Ice::Current& c) const;
	virtual ::TianShanIce::Transport::StorageLinkPrx getStorageLink(const ::Ice::Current&) const;
    virtual ::TianShanIce::Transport::StreamLinkPrx getStreamLink(const ::Ice::Current&) const;

    virtual ::Ice::Int getCost(const ::Ice::Current& c) const;
    virtual ::Ice::Int getLeaseLeft(const ::Ice::Current&) const;
    virtual ::TianShanIce::State getState(const ::Ice::Current& c) const;
    virtual ::TianShanIce::ValueMap getPrivateData(const ::Ice::Current& c) const;
    virtual void renew(::Ice::Int, const ::Ice::Current&);
	virtual ::TianShanIce::SRM::ResourceMap getResources(const ::Ice::Current& = ::Ice::Current()) const;

    virtual void narrow_async(const ::TianShanIce::Transport::AMD_PathTicket_narrowPtr& amdCB, const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);
//    virtual void narrow(const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);
    virtual void destroy(const ::Ice::Current&);
    
	virtual void commit_async(const ::TianShanIce::Transport::AMD_PathTicket_commitPtr&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current()) ;

	virtual ::Ice::Byte getPenalty(const ::Ice::Current& = ::Ice::Current()) const ;
    
    virtual void setPenalty(::Ice::Byte, const ::Ice::Current& = ::Ice::Current()) ;	

	typedef ::IceInternal::Handle< ADPathTicketImpl> Ptr;

	virtual bool setPrivateData(const ::std::string&, const ::TianShanIce::Variant&, const ::Ice::Current&/* = ::Ice::Current()*/);

private:
	void destroyEx(const ::Ice::Identity& id);

protected:

	PathSvcEnv& _env;
};

#ifdef _DEBUG
#  define DEFAULT_ALLOCATE_TICKET_LEASETERM	(60*1000)		// 60 sec
#  define MAX_ALLOCATE_TICKET_LEASETERM		(10* 60*1000)	// 10 min
#else
#  define DEFAULT_ALLOCATE_TICKET_LEASETERM	(2*1000)		// 2 sec
#  define MAX_ALLOCATE_TICKET_LEASETERM		(60*1000)		// 1 min
#endif // _DEBUG

#define MAX_TICKET_LEASETERM				(6*3600*1000)	// 6 hour

// -----------------------------
// service AccreditedPathsImpl
// -----------------------------
class AccreditedPathsImpl : public TianShanIce::Transport::PathAdmin
{
	friend class ADPathTicketImpl;
	friend class ReserveTicketCommand;

public:

    AccreditedPathsImpl(PathSvcEnv& env);
	virtual ~AccreditedPathsImpl();

public:	// impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// implemenations of PathManager

    virtual ::TianShanIce::StrValues listSupportedStorageLinkTypes(const ::Ice::Current& c) const;
    virtual ::TianShanIce::StrValues listSupportedStreamLinkTypes(const ::Ice::Current& c) const;
    virtual ::TianShanIce::PDSchema getStorageLinkSchema(const ::std::string& type, const ::Ice::Current& c) const;
    virtual ::TianShanIce::PDSchema getStreamLinkSchema(const ::std::string& type, const ::Ice::Current& c) const;

    virtual ::TianShanIce::Transport::ServiceGroups listServiceGroups(const ::Ice::Current& c);
    virtual ::TianShanIce::Transport::Storages listStorages(const ::Ice::Current& c);
    virtual ::TianShanIce::ValueMap getStoragePrivateData(const ::std::string& netId, const ::Ice::Current& c);
    virtual ::TianShanIce::Transport::Streamers listStreamers(const ::Ice::Current& c);
    virtual ::TianShanIce::ValueMap getStreamerPrivateData(const ::std::string& netId, const ::Ice::Current& c);
    virtual ::TianShanIce::Transport::StorageLinks listStorageLinksByStorage(const ::std::string& storageId, const ::Ice::Current&c);
    virtual ::TianShanIce::Transport::StorageLinks listStorageLinksByStreamer(const ::std::string& streamerId, const ::Ice::Current&c);
    virtual ::TianShanIce::Transport::StreamLinks listStreamLinksByStreamer(const ::std::string& streamerId, const ::Ice::Current&c);
    virtual ::TianShanIce::Transport::StreamLinks listStreamLinksByServiceGroup(::Ice::Int servicegroup, const ::Ice::Current&c);
    
	virtual ::TianShanIce::Transport::PathTickets reservePaths(::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintLease, const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);

public:	// implemenations of PathAdmin

    virtual bool updateServiceGroup(::Ice::Int, const ::std::string&, const ::Ice::Current& c);
    virtual bool removeServiceGroup(::Ice::Int, const ::Ice::Current& c);
    virtual bool updateStorage(const ::std::string&, const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool removeStorage(const ::std::string&, const ::Ice::Current& c);
    virtual bool setStoragePrivateData(const ::std::string&, const ::std::string&, const ::TianShanIce::Variant&, const ::Ice::Current& c);
    virtual bool updateStreamer(const ::std::string&, const ::std::string&, const ::std::string&, const ::std::string&, const ::Ice::Current& c);
    virtual bool removeStreamer(const ::std::string&, const ::Ice::Current& c);
    virtual bool setStreamerPrivateData(const ::std::string&, const ::std::string&, const ::TianShanIce::Variant&, const ::Ice::Current& c);
    virtual ::TianShanIce::Transport::StorageLinkPrx linkStorage(const ::std::string&, const ::std::string&, const ::std::string&, const ::TianShanIce::ValueMap& linkPD, const ::Ice::Current& c);
    virtual ::TianShanIce::Transport::StreamLinkPrx linkStreamer(::Ice::Int, const ::std::string&, const ::std::string&, const ::TianShanIce::ValueMap& linkPD, const ::Ice::Current& c);
    virtual ::TianShanIce::Transport::PathTickets listTickets(const ::Ice::Current& c);

	virtual ::TianShanIce::Transport::PathTickets reservePathsByStorage(const ::TianShanIce::Variant& sourceStorages, ::Ice::Int serviceGroup,
																  ::Ice::Long bandwidth, ::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintLease, const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);

    virtual ::TianShanIce::Transport::PathTickets reservePathsByStreamer(const ::TianShanIce::Variant& streamers, ::Ice::Int serviceGroup, ::Ice::Long bandwidth,
																  ::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintLease, const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c);

    virtual ::TianShanIce::Transport::PathTickets reservePathsEx(::Ice::Int maxCost, ::Ice::Int maxTickets, ::Ice::Int hintLease,
		                                                          const ::TianShanIce::ValueMap& contextIn, const ::TianShanIce::SRM::SessionPrx& sess, ::TianShanIce::ValueMap& contextOut, const ::Ice::Current& c);
	virtual void reservePaths_async(const ::TianShanIce::Transport::AMD_PathManager_reservePathsPtr&, ::Ice::Int, ::Ice::Int, ::Ice::Int, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());
	
    virtual void dumpDot(const ::TianShanIce::ValueMap& context, ::std::string& storageGraph, ::std::string& streamGraph, ::std::string& sgGraph, ::std::string& storageLinksGraph, ::std::string& streamLinksGraph, const ::Ice::Current& c);
    
    virtual ::std::string dumpXml(const ::Ice::Current& c);
    virtual void dumpToXmlFile(const ::std::string& filename, const ::Ice::Current& c);
    virtual void importXml(const ::std::string& pathsDef, bool cleanup, const ::Ice::Current& c);

private:
	::std::string privDataToXml(::TianShanIce::ValueMap& valMap);
	::TianShanIce::ValueMap xmlToPrivData(ZQ::common::XMLPreferenceEx* pPre);
	void convertSetToRandomVector(std::set<std::string>& sets , std::vector<std::string>& vectors );

	bool doImportXml(const std::string& xmlData, bool cleanup, std::string& err, const ::Ice::Current& c); // the implemention of importXml()
    bool doImportXml2(const std::string& xmlData, bool validateOnly, std::string& err, const ::Ice::Current& c);

	bool validateStandbyStoreLink(::TianShanIce::Transport::StorageLinkExPrx& storeLink, const std::string& linkId, const std::string& clientdesc);

protected:
	Ice::Identity	_localId;
	PathSvcEnv&		_env;
	PathHelperMgr&	_pathHelperMgr;
};

#define IdentityToObj(_CLASS, _ID) IdentityToObjEnv(_env, _CLASS, _ID)

}} // namespace

#endif // __ZQTianShan_ADPIceImpl_H__
