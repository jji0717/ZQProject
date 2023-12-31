// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_INDEX_I_H
#define FREEZE_INDEX_I_H

#include <Ice/Ice.h>
#include <Freeze/Index.h>
#include <Freeze/EvictorI.h>

namespace Freeze
{

class ObjectStore;

class IndexI
{
public:
    
    IndexI(Index&);

    std::vector<Ice::Identity> untypedFindFirst(const Key&, Ice::Int) const;
    
    std::vector<Ice::Identity> untypedFind(const Key&) const;
    
    Ice::Int untypedCount(const Key&) const;
    
    void
    associate(ObjectStore* store, DbTxn* txn, bool createDb, bool populateIndex);

    int
    secondaryKeyCreate(Db*, const Dbt*, const Dbt*, Dbt*);

    void
    close();
    
private:

   

    Index& _index;
    std::string _dbName;
    std::auto_ptr<Db> _db;
    ObjectStore* _store;
};

}
#endif

