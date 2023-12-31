// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: ObjectStore.h,v 1.7 2010/10/18 06:25:44 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : define ObjectStore over Redis
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ObjectStore/IceObjStore.h $
// 
// 3     6/25/15 11:56a Hui.shao
// 
// 2     6/10/15 1:50p Hui.shao
// 
// 1     6/09/15 7:06p Hui.shao
// 
// 1     5/28/15 7:36p Hui.shao
// ===========================================================================

#ifndef __ZQ_ObjectDB_IceObjStore_H__
#define __ZQ_ObjectDB_IceObjStore_H__

#include "ObjectStore.h"
#include "ObjLocator.h"

namespace ZQ {
namespace ObjectDB {

class OBJECTSTORE_API IceObjStore;

// -----------------------------
// class IceObjStore
// -----------------------------
// refer to ObjectStore.cpp, EvictorI.cpp(Not supports facet)
// key format in redis: 
//    a) primary key: StoredObject::KEY_PREFFIX "<identname>@<category>"
//    b) facet:       StoredObject::KEY_PREFFIX "<identname>@<cateory>:<facetname>"
class IceObjStore : public ObjectStore, public ::TianShanIce::ObjectDB::ObjLocator
{
public:
	typedef ::IceInternal::Handle< IceObjStore > Ptr;

    IceObjStore(const Ice::ObjectAdapterPtr adapter, const std::string& category, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, const std::string& dbUrl, const Index::List& indices =Index::List());
    virtual ~IceObjStore();

public: 
	virtual bool marshal(const StoredObject::Ptr& so, std::string& ostream) =0;
	virtual bool unmarshal(StoredObject::Ptr& so, const std::string& istream) =0;

protected: // impl of ServantLocator
    virtual ServantPtr locate(const ::Ice::Current& current, ::Ice::LocalObjectPtr& cookie);
    virtual void finished(const ::Ice::Current& current, const ::Ice::ObjectPtr& servant, const ::Ice::LocalObjectPtr& cookie);
	virtual void deactivate(const ::std::string& reason);

protected:
	virtual ServantPtr hatchServant(const std::string& classId);

	::Ice::ObjectAdapterPtr _adapter;

public: // clone of interface Freeze::Evictor
	virtual ::Ice::ObjectPrx add(const ServantPtr& servant, const ::Ice::Identity& ident);
	virtual ::Ice::ObjectPtr remove(const ::Ice::Identity& ident);
	virtual bool hasObject(const ::Ice::Identity& ident);
	virtual ObjectIterator::Ptr getIterator(size_t batchSize);
};

/*
class StoreStream : public BasicStream
{
public:
	StoreStream(IceInternal::InstancePtr inst) : BasicStream(inst, bool = false) {}

	void writeTypeId(const std::string&) {}
	void readTypeId(std::string&) {}

	writeTypeId(ice_staticId());
}
*/

}} // namespaces

#endif // __ZQ_ObjectDB_IceObjStore_H__

