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
// Ident : $Id: MetaLibImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MetaLib/MetaLibImpl.h $
// 
// 2     1/11/16 6:04p Dejian.fei
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 9     10-07-06 17:41 Yixin.tian
// merge for Linux OS
// 
// 8     10-06-22 15:10 Haoyuan.lu
// 
// 7     10-05-10 14:14 Haoyuan.lu
// 
// 6     10-04-16 15:25 Li.huang
// 
// 5     10-02-24 16:56 Haoyuan.lu
// 
// 4     09-06-29 16:45 Li.huang
// 
// 3     09-05-13 13:59 Haoyuan.lu
// 
// 2     08-03-18 15:47 Hui.shao
// 
// 1     08-03-17 21:08 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_MetaLibImpl_H__
#define __ZQTianShan_MetaLibImpl_H__

#include "../common/TianShanDefines.h"
#include "ZQ_common_conf.h"
#include "Locks.h"
#include "Log.h"
#include "NativeThreadPool.h"

#include "MetaLibFactory.h"
#include "MetaLib.h"
#include "ValueIdx.h"
#include "TypeIdx.h"


#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#define METADATA_INDEX			"mt_index"
#define METADATA_NONINDEX		"mt_nonindex"
#define OBJECTTYPE		"objectType"
namespace ZQTianShan {
namespace MetaLib {

class MetaLibImpl;

#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ
#define DECLARE_INDEX(_IDX)	TianShanIce::Repository::##_IDX##Ptr _idx##_IDX
#define METADATA_COMBINE_KEYNAME        "combine_keyname"

// -----------------------------
// module MetaLibImpl
// -----------------------------
class MetaLibImpl : virtual public ::TianShanIce::Repository::MetaLib
{
	friend class MetaLibFactory;
	friend class BaseCmd;
	friend class LookupCmd;
	friend class LocateByPidPAidCmd;
	friend class LocateVolumesByNidCmd;

public:

	typedef ::IceInternal::Handle< MetaLibImpl > Ptr;
    
	MetaLibImpl(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, Ice::CommunicatorPtr& communicator, const ::TianShanIce::StrValues& nonIndices, const char* databasePath = NULL, bool bIsCombineDB = false);
	virtual ~MetaLibImpl();

	::TianShanIce::Repository::MetaLibPrx& proxy();
	static ::Ice::Identity covertToMetaDataIdent(const ::Ice::Identity& identObj, const std::string& metadataName);
	static std::string metaDataCategory(const std::string& metadataName);
	static ::Ice::Identity covertToObjectIdent(const ::Ice::Identity& identIn);
	Freeze::EvictorPtr getObjectEvictor() { return _eMetaObj;};

public:	// impls of MetaLib

	virtual void registerMetaClass(const ::std::string& type, const ::TianShanIce::Repository::MetaDataMap& metaDataSchema, const ::Ice::Current& c = Ice::Current());
    virtual void setDefaultClassType(const ::std::string& type, const ::Ice::Current& c);
    virtual ::std::string getDefaultClassType(const ::Ice::Current& c);
    virtual void removeObject(const ::std::string& objId, const ::Ice::Current& c);
    virtual void removeObjects(const ::TianShanIce::StrValues& objIds, const ::Ice::Current& c);
    virtual void setMetaData(const ::std::string& objId, const ::std::string& name, const ::TianShanIce::Repository::MetaDataValue& value, bool skipSearch = false, const ::Ice::Current& c = Ice::Current());
    virtual void setMetaDatas(const ::std::string& objId, const ::TianShanIce::Repository::MetaDataMap& valueMap, bool skipSearch = false, const ::Ice::Current& c = Ice::Current());
//	virtual void addMetaDatas(const ::std::string& objId, const ::TianShanIce::Repository::MetaDataMap& valueMap, const ::Ice::Current& c);

	::TianShanIce::Repository::MetaDataValue getMetaData(const ::Ice::Identity& identObj, const ::std::string& key);

public:	// impls of MetaDataLibrary
	virtual ::TianShanIce::Repository::MetaObjectInfo openObject(const ::std::string& objId, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current& c);
    virtual ::std::string createObject(const ::std::string& type, ::Ice::Long timeout, const ::Ice::Current& c);
    virtual void lookup_async(const ::TianShanIce::Repository::AMD_MetaDataLibrary_lookupPtr& amdCB, const ::std::string& type, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames, const ::Ice::Current& c);

protected:
	void openDB(const char* databasePath=NULL);
	void closeDB();
	void openMetaData(const std::string& metaDataName, bool indexFlag);
	void setMetaDataEx(const ::Ice::Identity& identObj, const ::std::string& name, const ::TianShanIce::Repository::MetaDataValue& value, bool skipSearch = false, bool bVerifyObj=true);

//	virtual TianShanIce::Repository::LibMetaObjectPtr newObject(const ::std::string& type, ::Ice::Long timeout);
public: 
	bool addObject(::Ice::ObjectPtr obj, Ice::Identity ident);
	void removeObject(::Ice::Identity ident);

public:
	Ice::ObjectAdapterPtr	_adapter;

	typedef struct _MetaDataContainer
	{
		std::string metaDataName;
		TianShanIce::Repository::ValueIdxPtr idxValue;
		Freeze::EvictorPtr eMetaData;
	} MetaDataContainer;

	typedef std::map < std::string, MetaDataContainer > MetaDataContainerMap;
	MetaDataContainerMap	_metaDataContainerMap;

	IceUtil::RWRecMutex		_metaDataContainerLocker;

protected:
	ZQ::common::Log&		_log;
	ZQ::common::NativeThreadPool& _thpool;

	Ice::CommunicatorPtr	_communicator;
	Ice::Identity			_identBind;
	::TianShanIce::Repository::MetaLibPrx _thisPrx;
	Freeze::EvictorPtr		_eMetaObj;
//	IceUtil::AbstractMutexI<IceUtil::RecMutex>	_evitObjLock; // lock for _eMetaObj
	MetaLibFactory::Ptr		_factory;

	bool                    _bIsCombineDB;

	TianShanIce::Repository::TypeIdxPtr _idxType;

	std::string				_defaultType;

	std::string				_programRootPath;
	std::string				_dbPath;

	TianShanIce::StrValues  _nonIndices;
};

// -----------------------------
// class LibMetaValueImpl
// -----------------------------
//class LibMetaValueImpl : public TianShanIce::Repository::LibMetaValue, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class LibMetaValueImpl : public TianShanIce::Repository::LibMetaValue, public  ICEAbstractMutexRLock
{
	friend class MetaLibImpl;

public:

	typedef ::IceInternal::Handle< LibMetaValueImpl > Ptr;

    LibMetaValueImpl(MetaLibImpl& lib);

public: // impl of TianShanIce::Repository::LibMetaValue
    virtual ::Ice::Identity getObjId(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Repository::MetaDataValue get(const ::Ice::Current& c) const;
    virtual void set(const ::TianShanIce::Repository::MetaDataValue& value, const ::Ice::Current& c);

protected:

	MetaLibImpl& _lib;
};

// -----------------------------
// class LibMetaObjectImpl
// -----------------------------
//class LibMetaObjectImpl : public TianShanIce::Repository::LibMetaObject, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class LibMetaObjectImpl : public TianShanIce::Repository::LibMetaObject, public ICEAbstractMutexRLock
{
	friend class MetaLibImpl;
	
public:
	
	typedef ::IceInternal::Handle< LibMetaObjectImpl > Ptr;

    LibMetaObjectImpl(MetaLibImpl& lib);
	
public: // impl of TianShanIce::Repository::LibMetaObject
    virtual ::Ice::Identity getObjId(const ::Ice::Current& c) const;
    virtual ::std::string getType(const ::Ice::Current& c) const;
    virtual ::std::string getCreatedTime(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Repository::MetaDataValue get(const ::std::string& key, const ::Ice::Current& c) const;
    virtual void set(const ::std::string& key, const ::TianShanIce::Repository::MetaDataValue& value, const ::Ice::Current& c);
	virtual ::TianShanIce::Repository::MetaDataMap getMetaDataMap(const ::Ice::Current& c) const;
	
protected:
	
	MetaLibImpl& _lib;
};

#define OBJMD_SEP		"?"
#define META_OBJECT		"metaObject"
#define META_DATA	    "MetaData"

}} // namespace

#endif // __ZQTianShan_MetaLibImpl_H__
