// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : DODContentStore Interface implementation class
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/DODContentStoreImpl.h 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/DODContentStoreImpl.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 5     10-02-02 14:22 Li.huang
// 
// 4     09-12-29 12:02 Li.huang
// 
// 3     09-06-03 15:56 Li.huang
// 
// 3     09-05-14 10:25 Build
// add method adjustProvisionSchedule()
// 
// 2     08-12-09 17:01 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 7     08-10-29 10:10 Ken.qian
// compatile with new ContentStore interface
// 
// 6     07-05-18 11:20 Li.huang
// 
// 5     07-05-17 12:15 Li.huang
// 
// 4     07-05-17 10:43 Li.huang
// 
// 3     07-04-17 10:54 Ken.qian
// 
// 2     07-04-16 11:03 Ken.qian
// 
// 1     07-04-11 20:10 Ken.qian


#ifndef __ZQ_DODContentStoreI_H__
#define __ZQ_DODContentStoreI_H__

#include "Locks.h"
#include <list>

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
//#include <Ice/IdentityUtil.h>

#include "TsStorage.h"
#include "DODContentStore.h"

#include "GraphPool.h"


class DODContentI;
typedef IceUtil::Handle<DODContentI> DODContentIPtr;

class DODContentStoreI;
typedef IceUtil::Handle<DODContentStoreI> DODContentStoreIPtr;


//////////////////////////////////////////////////////////////////
//          Definition of DODContentFactory						//
//////////////////////////////////////////////////////////////////
class DODContentFactory : virtual public Ice::ObjectFactory
{
public:
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

};

typedef IceUtil::Handle<DODContentFactory> DODContentFactoryPtr;

//////////////////////////////////////////////////////////////////
//          Definition of ContentI							//
// Wrap and Re-direct the invoking from Ice to ZQ Content       //
//////////////////////////////////////////////////////////////////

class DODContentI : public DataOnDemand::DODContent, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	DODContentI(const ::std::string& name, std::string destinationContentType, bool createIfNotExist);
	DODContentI();

	~DODContentI();

public:
    virtual TianShanIce::Storage::ContentStorePrx getStore(const ::Ice::Current& current = ::Ice::Current());

    virtual ::std::string getName(const ::Ice::Current& current = ::Ice::Current()) const;

    virtual bool isProvisioned(const ::Ice::Current& current = ::Ice::Current()) const;

    virtual ::std::string getProvisionTime(const ::Ice::Current& = ::Ice::Current()) const;

    virtual std::string getLocaltype(const ::Ice::Current& current = ::Ice::Current()) const;

	virtual std::string getSubtype(const ::Ice::Current& current = ::Ice::Current()) const;
	
    virtual ::Ice::Float getFramerate(const ::Ice::Current& current = ::Ice::Current()) const;

    virtual void getResolution(::Ice::Int& pixelHorizontal, ::Ice::Int& pixelVertical, const ::Ice::Current& current = ::Ice::Current()) const;

    virtual ::Ice::Long getFilesize(const ::Ice::Current& = ::Ice::Current()) const;

	virtual ::Ice::Long getSupportFileSize(const ::Ice::Current& = ::Ice::Current()) const;

    virtual ::Ice::Long getPlayTime(const ::Ice::Current& = ::Ice::Current()) const;

    virtual ::Ice::Int getBitRate(const ::Ice::Current& = ::Ice::Current()) const;

    virtual void getTrickSpeedCollection(TianShanIce::Storage::TrickSpeedCollection& speeds, const ::Ice::Current& current = ::Ice::Current()) const;

    virtual ::std::string getSourceUrl(const ::Ice::Current& current = ::Ice::Current()) const;

    virtual void provision(const ::std::string& sourceUrl, const std::string& sourceContentType, bool overwrite, 
							const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, 
							::Ice::Int maxTransferBitrate, const ::Ice::Current& current = ::Ice::Current());

    virtual ::std::string provisionPassive(const ::std::string& sourceContentType, bool overwrite, 
							const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC,
							::Ice::Int maxTransferBitrate, const ::Ice::Current& current = ::Ice::Current());

	virtual void destroy(const ::Ice::Current& current = ::Ice::Current());
	
	virtual void destroy2(bool, const ::Ice::Current& = ::Ice::Current());

    virtual void cancelProvision(const ::Ice::Current& = ::Ice::Current());

	virtual void setDataWrappingParam(const ::DataOnDemand::DataWrappingParam& param, const ::Ice::Current& = ::Ice::Current());

    virtual ::std::string getMD5Checksum(const ::Ice::Current& current = ::Ice::Current()) const;

	virtual ::std::string getExportURL(const ::std::string&, ::Ice::Int, ::Ice::Int&, ::TianShanIce::Properties&, const ::Ice::Current&);

    virtual TianShanIce::Storage::ContentStorePrx getStore(const Ice::Current &) const;
	virtual TianShanIce::Properties getMetaData(const Ice::Current &) const;
	virtual void setUserMetaData(const std::string &,const std::string &,const Ice::Current &);
	virtual void setUserMetaData2(const TianShanIce::Properties &,const Ice::Current &);
	virtual TianShanIce::Storage::ContentState getState(const Ice::Current &) const;
	virtual Ice::Long getPlayTimeEx(const Ice::Current &) const;
	virtual TianShanIce::Storage::TrickSpeedCollection getTrickSpeedCollection(const Ice::Current &) const;
	TianShanIce::Storage::VolumePrx theVolume(const Ice::Current &) const;
	virtual void adjustProvisionSchedule(const std::string&, const std::string&, const Ice::Current &);

public:
	
private:
	const DODContentStoreIPtr  _nodeStoreI;

	bool            _destroyed;
	bool            _destroyAfterProv;

	::DataOnDemand::DataWrappingParam _wrapParam;
	bool                              _paramConfiged;
};


//////////////////////////////////////////////////////////////////
//          Definition of ContentStoreI				        //
// Wrap and Re-direct the invoking from Ice to ZQ ContentStore  //
//////////////////////////////////////////////////////////////////
class DODContentStoreI : public TianShanIce::Storage::ContentStore
{
	friend class DODContentI;
public:   
    DODContentStoreI(std::string netId);

    virtual ~DODContentStoreI();
	
public:
    virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

    virtual ::std::string getNetId(const ::Ice::Current& current = ::Ice::Current()) const;

    virtual std::string type(const ::Ice::Current& current = ::Ice::Current()) const;

    virtual bool isValid(const ::Ice::Current& current = ::Ice::Current()) const;

    virtual void getCapacity(::Ice::Long& freeMB, ::Ice::Long& totalMB, const ::Ice::Current& current = ::Ice::Current());

	virtual ::Ice::Byte getCacheLevel(const ::Ice::Current& = ::Ice::Current()) const;

    virtual ::TianShanIce::StrValues listContent(const ::std::string& condition, const ::Ice::Current& = ::Ice::Current()) const;

    virtual TianShanIce::Storage::ContentPrx openContent(const ::std::string& name, const std::string& destinationContentType, bool createIfNotExist, const ::Ice::Current& current = ::Ice::Current());

//	virtual bool sinkStateChangeEvent(const TianShanIce::Storage::ProvisionStateChangeSinkPrx& event, const ::Ice::Current& current = ::Ice::Current());
    
//    virtual bool sinkProgressEvent(const TianShanIce::Storage::ProvisionProgressSinkPrx&, const ::Ice::Current& = ::Ice::Current());

    virtual bool unsinkEvent(const ::TianShanIce::Events::BaseEventSinkPrx&, const ::Ice::Current& = ::Ice::Current());
	
	virtual TianShanIce::Storage::VolumePrx openVolume(const std::string &,const Ice::Current &) const;

	virtual void listVolumes_async(const TianShanIce::Storage::AMD_ContentStore_listVolumesPtr &,const std::string &,bool,const Ice::Current &) const;
	virtual TianShanIce::Storage::ContentPrx openContentByFullname(const std::string &,const Ice::Current &) const;
	virtual std::string getVolumeName(const Ice::Current &) const;
	virtual TianShanIce::Replicas getStreamServices(const Ice::Current &) const;
	virtual void listContents_async(const ::TianShanIce::Storage::AMD_Folder_listContentsPtr&, const ::TianShanIce::StrValues&, const ::std::string&, ::Ice::Int, const ::Ice::Current& ) const;
	virtual TianShanIce::Storage::VolumePrx openSubVolume(const std::string &,bool,Ice::Long,const Ice::Current &);
	virtual ::TianShanIce::Storage::FolderPrx openSubFolder(const ::std::string&, bool, ::Ice::Long, const ::Ice::Current&);
	virtual ::std::string getName(const ::Ice::Current& ) const;
	virtual ::TianShanIce::Storage::FolderPrx parent(const Ice::Current &) const;
	virtual void destroy(const Ice::Current &);

	virtual TianShanIce::Storage::FolderInfos listSubFolders(const Ice::Current &) const;

			
private:
	bool destroyServantByName(std::string name);

	void RemoveEventsink();

private:
	Freeze::EvictorPtr						_evictor;
	DODContentFactoryPtr                    _contentFactory;

	ZQ::common::Mutex  _openMutex;

//    std::list<TianShanIce::Storage::ProvisionStateChangeSinkPrx> _eventStageClients;
	ZQ::common::Mutex  _eventStateMutex;

//    std::list<TianShanIce::Storage::ProvisionProgressSinkPrx> _eventProgressClients;
	ZQ::common::Mutex  _eventProgressMutex;

	TianShanIce::Storage::ContentStorePrx    _cntStoreProxy;

	std::string _netId;

private:
	ZQ::Content::Process::GraphPool*        _graphPool;     
	
};

#endif

