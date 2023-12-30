#include "ZQSnmpMgmt.hpp"
namespace ZQ{
namespace Snmp{

// Managed Object
SimpleObject::SimpleObject(VariablePtr var, AsnType type, Access access)
    :var_(var), type_(type), access_(access)
{
}

SimpleObject::~SimpleObject(){}

Status SimpleObject::get(const Oid& subid, SmiValue& val)
{
    // step 1: check access
    // step 2: check subid
    if(!subid.empty())
        return noSuchName;

    if(!var_->get(val, type_))
        return genErr;

    if(val.syntax() != type_)
    {
        // log warning here
        return genErr;
    }

    return noError;
}
Status SimpleObject::set(const Oid& subid, const SmiValue& val)
{
    // step 1: check access
    if(access_ != aReadWrite)
        return readOnly;

    // step 2: check subid
    if(!subid.empty())
        return noSuchName;

    // step 3: check type
    if(type_ != val.syntax())
        return badValue; //wrongType;

    // step 4: validate value
    if(!var_->validate(val))
        return badValue;
    if(!var_->set(val))
        return genErr;

    return noError;
}

Status SimpleObject::next(const Oid& subid, Oid& nextId) const
{ // out of range
    return noSuchName;
}

Status SimpleObject::first(Oid& firstId) const
{
    firstId.data().clear();
    return noError;
}

// CompositeObject
CompositeObject::~CompositeObject(){}
bool CompositeObject::add(const Oid& key, ManagedPtr val)
{
    if(!val)
        return false;

    // keep the sequence ascending
    std::vector< Item >::iterator it;
    for(it = data_.begin(); it != data_.end(); ++it)
    {
        int cmp = key.compare(it->key);
        if(cmp == 0)
        {
            return false;
        }
        else if(cmp < 0)
        {
            break; // get the position
        } // else continue;
    }

	if(it == data_.end())
		data_.push_back(Item(key, val));
	else
		data_.insert(it, Item(key, val));

	return true;
}
ManagedPtr CompositeObject::remove(const Oid& key)
{
    std::vector< Item >::iterator it;
    for(it = data_.begin(); it != data_.end(); ++it)
    {
        int cmp = key.compare(it->key);
        if(cmp == 0)
        {
            ManagedPtr val = it->val;
            data_.erase(it);
            return val;
        }
        else if(cmp > 0)
        {
            return ManagedPtr();
        } // else continue;
    }
    return ManagedPtr();
}
Status CompositeObject::get(const Oid& subid, SmiValue& val)
{
    std::vector< Item >::const_iterator it;
    for(it = data_.begin(); it != data_.end(); ++it)
    {
        int cmp = subid.compare(0, it->key.length(), it->key);
        if(cmp == 0)
        {
            return it->val->get(subid.sub(it->key.length()), val);
        }
        else if(cmp < 0)
        {
            break;
        } // else continue
    }
    return noSuchName;
}
Status CompositeObject::set(const Oid& subid, const SmiValue& val)
{
    std::vector< Item >::iterator it;
    for(it = data_.begin(); it != data_.end(); ++it)
    {
        int cmp = subid.compare(0, it->key.length(), it->key);
        if(cmp == 0)
        {
            return it->val->set(subid.sub(it->key.length()), val);
        }
        else if(cmp < 0)
        {
            break;
        } // else continue
    }

    return noSuchName;
}
Status CompositeObject::next(const Oid& subid, Oid& nextId) const
{
    std::vector< Item >::const_iterator it;
    for(it = data_.begin(); it != data_.end(); ++it)
    {
        int cmp = subid.compare(0, it->key.length(), it->key);
        if(cmp == 0)
        {
            Status stat = it->val->next(subid.sub(it->key.length()), nextId);
            if(stat != noSuchName)
            {
                if(stat == noError)
                    nextId = it->key + nextId;

                return stat;
            } // else continue
        }
        else if(cmp < 0)
        {
            break;
        } // else continue
    }

    if(it != data_.end())
    {
        for(; it != data_.end(); ++it)
        {
            Status stat = it->val->first(nextId);
            if(stat != noSuchName)
            {
                if(stat == noError)
                    nextId = it->key + nextId;
                return stat;
            } // else keep searching
        }
    }
    return noSuchName;
}

Status CompositeObject::first(Oid& firstId) const
{
    std::vector< Item >::const_iterator it;
    for(it = data_.begin(); it != data_.end(); ++it)
    {
        Status stat = it->val->first(firstId);
        if(stat != noSuchName)
        {
            if(stat == noError)
                firstId = it->key + firstId;
            return stat;
        } // else keep searching
    }
    return noSuchName;
}

//////////////////////////////////////////
//
// composite object with the thread protection
//

GuardedCompositeObject::~GuardedCompositeObject()
{
}

bool GuardedCompositeObject::add(const Oid& key, ManagedPtr val)
{
    ZQ::common::MutexGuard guard(lock_);
    return CompositeObject::add(key, val);
}

ManagedPtr GuardedCompositeObject::remove(const Oid& key)
{
    ZQ::common::MutexGuard guard(lock_);
    return CompositeObject::remove(key);
}

Status GuardedCompositeObject::get(const Oid& subid, SmiValue& val)
{
    ZQ::common::MutexGuard guard(lock_);
    return CompositeObject::get(subid, val);
}

Status GuardedCompositeObject::set(const Oid& subid, const SmiValue& val)
{
    ZQ::common::MutexGuard guard(lock_);
    return CompositeObject::set(subid, val);
}

Status GuardedCompositeObject::next(const Oid& subid, Oid& nextId) const
{
    ZQ::common::MutexGuard guard(lock_);
    return CompositeObject::next(subid, nextId);
}

Status GuardedCompositeObject::first(Oid& firstId) const
{
    ZQ::common::MutexGuard guard(lock_);
    return CompositeObject::first(firstId);
}

// Table
bool Table::addColumn(uint32 colId, AsnType type, Access access)
{
    if(columns_.find(colId) != columns_.end())
    {
        return false;
    }

    ColumnConf colconf(type, access, ObjectPtr(new CompositeObject));
    columns_.insert(std::make_pair(colId, colconf));

    add(buildIndex(colId), colconf.obj);
    return true;
}

bool Table::addRowData(uint32 colId, Oid rowIndex, VariablePtr var)
{
    std::map<uint32, ColumnConf>::const_iterator it;
    it = columns_.find(colId);
    if(it != columns_.end())
    {
        return it->second.obj->add(rowIndex, ManagedPtr(new SimpleObject(var, it->second.type, it->second.access)));
    }
    else
    {
        return false;
    }
}

Oid Table::buildIndex(const std::string& idx)
{
    // not implement
    Oid res;
    res.data().reserve(idx.size());
    for(size_t i = 0; i < idx.size(); ++i)
        res.data().push_back((unsigned char)idx[i]);

    return res;
}

Oid Table::buildIndex(uint32 idx)
{
    Oid res;
    res.data().push_back(idx);
    return res;
}

}} // namespace ZQ::Snmp
