#ifndef __ZQ_Snmp_Mgmt_H__
#define __ZQ_Snmp_Mgmt_H__
#include "ZQSnmp.hpp"
#include "oid.hpp"
#include "smival.hpp"
#include <Locks.h>
#include <map>

namespace ZQ{
namespace Snmp{

class SimpleObject: public IManaged
{
public:
    SimpleObject(VariablePtr var, AsnType type, Access access);
    virtual ~SimpleObject();
    virtual Status get(const Oid& subid, SmiValue& val);
    virtual Status set(const Oid& subid, const SmiValue& val);
    virtual Status next(const Oid& subid, Oid& nextId) const;
    virtual Status first(Oid& firstId) const;
private:
    VariablePtr var_;
    AsnType type_;
    Access access_;
};

class CompositeObject: public IManaged
{
public:
	CompositeObject()
	{
        data_.reserve(64);// for insert and push_back not throw exeption
	}

    virtual ~CompositeObject();
    virtual bool add(const Oid& key, ManagedPtr val);
    virtual ManagedPtr remove(const Oid& key);
    virtual Status get(const Oid& subid, SmiValue& val);
    virtual Status set(const Oid& subid, const SmiValue& val);
    virtual Status next(const Oid& subid, Oid& nextId) const;
    virtual Status first(Oid& firstId) const;
private:
    struct Item
    {
        Item(){}
		Item(const Item &other)
		{
            if (this != &other)
            {
				key = other.key;
				val = other.val;
            }
		}

		Item & operator=(const Item & other)
		{
            if (this != &other)
            {
				key = other.key;
				val = other.val;
            }

			return *this;
		}

        Item(const Oid& k, const ManagedPtr& v):key(k), val(v){}
		~Item()
		{
		}

        Oid key;
        ManagedPtr val;
    };
    std::vector< Item > data_;
};
typedef boost::shared_ptr< CompositeObject > ObjectPtr;

//
// composite object with the thread protection
//
class GuardedCompositeObject: public CompositeObject
{
public:
    virtual ~GuardedCompositeObject();
    virtual bool add(const Oid& key, ManagedPtr val);
    virtual ManagedPtr remove(const Oid& key);
    virtual Status get(const Oid& subid, SmiValue& val);
    virtual Status set(const Oid& subid, const SmiValue& val);
    virtual Status next(const Oid& subid, Oid& nextId) const;
    virtual Status first(Oid& firstId) const;
private:
    ZQ::common::Mutex lock_;
};
typedef boost::shared_ptr<GuardedCompositeObject> GuardedObjectPtr;

// Table
class Table: public CompositeObject
{
public:
    bool addColumn(uint32 colId, AsnType type, Access access);
    bool addRowData(uint32 colId, Oid rowIndex, VariablePtr var);
    static Oid buildIndex(const std::string& idx);
    static Oid buildIndex(uint32 idx);
private:
    struct ColumnConf
    {
        ColumnConf(AsnType t, Access a, ObjectPtr o)
            :type(t), access(a), obj(o){}
        AsnType type;
        Access access;
        ObjectPtr obj;
    };
    std::map<uint32, ColumnConf> columns_;
};
typedef boost::shared_ptr< Table > TablePtr;
}} // namespace ZQ::Snmp
#endif
