// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/IndexI.h>
#include <Freeze/Util.h>
#include <Freeze/ObjectStore.h>
#include <Freeze/EvictorI.h>

using namespace Freeze;
using namespace Ice;
using namespace std;


static int 
callback(Db* secondary, const Dbt* key, const Dbt* value, Dbt* result)
{
    void* indexObj = secondary->get_app_private();
    IndexI* index = static_cast<IndexI*>(indexObj);
    assert(index != 0);
    return index->secondaryKeyCreate(secondary, key, value, result);
}


Freeze::IndexI::IndexI(Index& index) :
    _index(index),
    _store(0)
{
}
 
vector<Identity>
Freeze::IndexI::untypedFindFirst(const Key& bytes, Int firstN) const
{
    DeactivateController::Guard 
        deactivateGuard(_store->evictor()->deactivateController());

    Dbt dbKey;
    initializeInDbt(bytes, dbKey);
    //
    // When we have a custom-comparison function, Berkeley DB returns
    // the key on-disk (when it finds one). We disable this behavior:
    // (ref Oracle SR 5925672.992)
    //
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);


    Key pkey(1024);
    Dbt pdbKey;
    initializeOutDbt(pkey, pdbKey);

    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    Ice::CommunicatorPtr communicator = _store->communicator();
    _store->evictor()->saveNow();

    vector<Identity> identities;

    try
    {
        for(;;)
        {
            Dbc* dbc = 0;
            identities.clear();

            try
            {
                //
                // Move to the first record
                // 
                _db->cursor(0, &dbc, 0);
                u_int32_t flags = DB_SET;

                bool found;
                
                do
                {
                    for(;;)
                    {
                        try
                        {
                            //
                            // It is critical to set key size to key capacity before the
                            // get, as a resize that increases the size inserts 0
                            //
                            pkey.resize(pkey.capacity());

                            found = (dbc->pget(&dbKey, &pdbKey, &dbValue, flags) == 0);
                            if(found)
                            {
                                pkey.resize(pdbKey.get_size());
                                
                                Ice::Identity ident;
                                ObjectStore::unmarshal(ident, pkey, communicator);
                                identities.push_back(ident);
                                flags = DB_NEXT_DUP;
                            }
                            break; // for(;;)
                        }
                        catch(const DbDeadlockException&)
                        {
                            throw;
                        }
                        catch(const DbException& dx)
                        {
                            handleDbException(dx, pkey, pdbKey, __FILE__, __LINE__);
                        }
                    }
                }                   
                while((firstN <= 0 || identities.size() < static_cast<size_t>(firstN)) && found);

                Dbc* toClose = dbc;
                dbc = 0;
                toClose->close();
                break; // for (;;)
            }
            catch(const DbDeadlockException&)
            {
                if(dbc != 0)
                {
                    try
                    {
                        dbc->close();
                    }
                    catch(const DbDeadlockException&)
                    {
                        //
                        // Ignored
                        //
                    }
                }

                if(_store->evictor()->deadlockWarning())
                {
                    Warning out(_store->communicator()->getLogger());
                    out << "Deadlock in Freeze::IndexI::untypedFindFirst while searching \"" 
                        << _store->evictor()->filename() + "/" + _dbName << "\"; retrying ...";
                }

                //
                // Retry
                //
            }
            catch(...)
            {
                if(dbc != 0)
                {
                    try
                    {
                        dbc->close();
                    }
                    catch(const DbDeadlockException&)
                    {
                        //
                        // Ignored
                        //
                    }
                }
                throw;
            }
        }
    }
    catch(const DbException& dx)
    {
        handleDbException(dx, __FILE__, __LINE__);
    }
    
    return identities;
}
    
vector<Identity>
Freeze::IndexI::untypedFind(const Key& bytes) const
{
    return untypedFindFirst(bytes, 0);
}

Int
Freeze::IndexI::untypedCount(const Key& bytes) const
{
    DeactivateController::Guard 
        deactivateGuard(_store->evictor()->deactivateController());

    Dbt dbKey;
    initializeInDbt(bytes, dbKey);
    //
    // When we have a custom-comparison function, Berkeley DB returns
    // the key on-disk (when it finds one). We disable this behavior:
    // (ref Oracle SR 5925672.992)
    //
    dbKey.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    _store->evictor()->saveNow();
    Int result = 0;
    
    try
    {
        for(;;)
        {
            Dbc* dbc = 0;
            
            try
            {
                //
                // Move to the first record
                // 
                _db->cursor(0, &dbc, 0);
                bool found = (dbc->get(&dbKey, &dbValue, DB_SET) == 0);
                
                if(found)
                {
                    db_recno_t count = 0;
                    dbc->count(&count, 0);
                    result = static_cast<Int>(count);
                }
                
                Dbc* toClose = dbc;
                dbc = 0;
                toClose->close();
                break; // for (;;)
            }
            catch(const DbDeadlockException&)
            {
                if(dbc != 0)
                {
                    try
                    {
                        dbc->close();
                    }
                    catch(const DbDeadlockException&)
                    {
                        //
                        // Ignored
                        //
                    }
                }

                if(_store->evictor()->deadlockWarning())
                {
                    Warning out(_store->communicator()->getLogger());
                    out << "Deadlock in Freeze::IndexI::untypedCount while searching \"" 
                        << _store->evictor()->filename() + "/" + _dbName << "\"; retrying ...";
                }

                //
                // Retry
                //
            }
            catch(...)
            {
                if(dbc != 0)
                {
                    try
                    {
                        dbc->close();
                    }
                    catch(const DbDeadlockException&)
                    {
                        //
                        // Ignored
                        //
                    }
                }
                throw;
            }
        }
    }
    catch(const DbException& dx)
    {
        DatabaseException ex(__FILE__, __LINE__);
        ex.message = dx.what();
        throw ex;
    }
    
    return result;
}

void
Freeze::IndexI::associate(ObjectStore* store, DbTxn* txn, 
                          bool createDb, bool populateIndex)
{
    assert(txn != 0);
    _store = store;
    _index._communicator = store->communicator();
    
    _db.reset(new Db(store->evictor()->dbEnv()->getEnv(), 0));
    _db->set_flags(DB_DUP | DB_DUPSORT);
    _db->set_app_private(this);

	//merge from Ice3.3.1
	_dbName = EvictorI::indexPrefix + store->dbName() + "." + _index.name();

    Ice::PropertiesPtr properties = store->communicator()->getProperties();
    string propPrefix = "Freeze.Evictor." + store->evictor()->filename() + ".";

    int btreeMinKey = properties->getPropertyAsInt(propPrefix + _dbName + ".BtreeMinKey");
    if(btreeMinKey > 2)
    {
        if(store->evictor()->trace() >= 1)
        {
            Trace out(store->evictor()->communicator()->getLogger(), "Freeze.Evictor");
            out << "Setting \"" << store->evictor()->filename() + "." + _dbName << "\"'s btree minkey to " << btreeMinKey;
        }
        _db->set_bt_minkey(btreeMinKey);
    }
        
    bool checksum = properties->getPropertyAsInt(propPrefix + "Checksum") > 0;
    if(checksum)
    {
        //
        // No tracing on purpose
        //

        _db->set_flags(DB_CHKSUM);
    }
    //end of merge
    
    u_int32_t flags = 0;
    if(createDb)
    {
        flags = DB_CREATE;
    }

    _dbName = EvictorI::indexPrefix + store->dbName() + "." + _index.name();

    _db->open(txn, store->evictor()->filename().c_str(), _dbName.c_str(), DB_BTREE, flags, FREEZE_DB_MODE);

    flags = 0;
    if(populateIndex)
    {
        flags = DB_CREATE;
    }
    store->db()->associate(txn, _db.get(), callback, flags);
}

int
Freeze::IndexI::secondaryKeyCreate(Db* secondary, const Dbt* dbKey, 
                                   const Dbt* dbValue, Dbt* result)
{
    Ice::CommunicatorPtr communicator = _store->communicator();

    ObjectRecord rec;
    Byte* first = static_cast<Byte*>(dbValue->get_data());
    Value value(first, first + dbValue->get_size());
    ObjectStore::unmarshal(rec, value, communicator);

    Key bytes;
    if(_index.marshalKey(rec.servant, bytes))
    {
        result->set_flags(DB_DBT_APPMALLOC);
        void* data = malloc(bytes.size());
        memcpy(data, &bytes[0], bytes.size());
        result->set_data(data);
        result->set_size(static_cast<u_int32_t>(bytes.size()));
        return 0;
    }
    else
    {
        //
        // Don't want to index this one
        //
        return DB_DONOTINDEX;
    }
}

void
Freeze::IndexI::close()
{
    if(_db.get() != 0)
    {
        try
        {
            _db->close(0);
        }
        catch(const DbException& dx)
        {
            DatabaseException ex(__FILE__, __LINE__);
            ex.message = dx.what();
            throw ex;
        }
        _db.reset(0);   
    }
}
