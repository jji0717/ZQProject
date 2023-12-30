#include "ZQSnmp.hpp"
#include "oid.hpp"
#include "smival_netsnmp.h"

// implemation of conversion functions with snmpapi.lib
namespace ZQ{
namespace Snmp{

// oid conversion functions
template <>
bool oidFrom(Oid& id, const netsnmp_variable_list& vb)
{
    id.data().assign(vb.name, vb.name + vb.name_length);
    return true;
}

template <>
bool oidTo(const Oid& id, netsnmp_variable_list& vb)
{
    std::vector<oid> newId;
    newId.assign(id.data().begin(), id.data().end());
    return (0 == snmp_set_var_objid(&vb, (newId.empty() ? 0 : &newId[0]), newId.size()));
}

// smivalue conversion function
// variable_list
template <>
bool smivalFrom(SmiValue& val, const netsnmp_variable_list& from, AsnType t)
{
    if(t != from.type)
        return false;

    val.clear();
    return (0 == snmp_set_var_typed_value(&val.data(), t, from.val.string, from.val_len));
}

template <>
bool smivalTo(const SmiValue& val, netsnmp_variable_list& to)
{
    return (0 == snmp_set_var_typed_value(&to, val.syntax(), val.data().val.string, val.data().val_len));
}

// integer
template <>
bool smivalFrom(SmiValue& val, const int& from, AsnType t)
{
    if(t != AsnType_Integer)
        return false;
    val.clear();
    return (0 == snmp_set_var_typed_integer(&val.data(), t, from));
}

template <>
bool smivalTo(const SmiValue& val, int& to)
{
    if(val.syntax() != AsnType_Integer)
        return false;
    if(val.data().val.integer) {
        to = *val.data().val.integer;
        return true;
    } else {
        return false;
    }
}
// counter64
template <>
bool smivalFrom(SmiValue& val, const int64& from, AsnType t)
{
    if(t != AsnType_Counter64)
        return false;

    val.clear();
	AsnAny& data = val.data();
	data.val_len = sizeof(int64);
	data.val.counter64 = (struct counter64 *)data.buf;
	memmove(data.val.counter64, &from, sizeof(int64) );

	return true;

//    struct counter64 v;
//    // convert the from int64 to counter64
//    v.high = ((from >> 32) & 0x00000000FFFFFFFF);
//    v.low = (from & 0x00000000FFFFFFFF);
//printf("orig: %ld\n", from);
//printf("high: %ld\n", v.high);
//printf("low: %ld\n", v.low);
//    return (0 == snmp_set_var_typed_value(&val.data(), t, (const u_char*)&v, sizeof(struct counter64)));
}

template <>
bool smivalTo(const SmiValue& val, int64& to)
{
    if(val.syntax() != AsnType_Counter64)
        return false;

    if(val.data().val.counter64) {
        const struct counter64& v = *val.data().val.counter64;
        to = (((uint64)v.high) << 32) + v.low;
        return true;
    } else {
        return false;
    }
}

// octets
template <>
bool smivalFrom(SmiValue& val, const std::string& from, AsnType t)
{
    if(t != AsnType_Octets)
        return false;
    return (0 == snmp_set_var_typed_value(&val.data(), t, (const u_char*)from.data(), from.size()));
}

template <>
bool smivalTo(const SmiValue& val, std::string& to)
{
    if(val.syntax() != AsnType_Octets)
        return false;

    if(val.data().val.string) {
        to.assign((const char*)(val.data().val.string), val.data().val_len);
        return true;
    } else {
        return false;
    }
}
}} // namespace ZQ::Snmp
