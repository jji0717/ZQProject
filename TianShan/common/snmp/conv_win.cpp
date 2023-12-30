#include "ZQSnmp.hpp"
#include "oid.hpp"
#include "smival_win.h"

// implemation of conversion functions with snmpapi.lib
namespace ZQ{
namespace Snmp{

// oid conversion functions
template <>
bool oidFrom(Oid& id, const AsnObjectIdentifier& from)
{
    id.data().assign(from.ids, from.ids + from.idLength);
    return true;
}

template <>
bool oidTo(const Oid& id, AsnObjectIdentifier& to)
{
    SnmpUtilOidFree(&to);

    if(id.length() != 0)
        to.ids = (UINT*)SnmpUtilMemAlloc(sizeof(UINT) * id.length());

    std::copy(id.data().begin(), id.data().end(), to.ids);
    to.idLength = id.length();
    return true;
}

// smivalue conversion function
// AsnAny
template <>
bool smivalFrom(SmiValue& val, const AsnAny& from, AsnType t)
{
    if(t != from.asnType)
        return false;

    val.clear();
    return (0 != SnmpUtilAsnAnyCpy(&val.data(), (AsnAny*)&from));
}

template <>
bool smivalTo(const SmiValue& val, AsnAny& to)
{
    SnmpUtilAsnAnyFree(&to);
    return (0 != SnmpUtilAsnAnyCpy(&to, (AsnAny*)&val.data()));
}

// integer
template <>
bool smivalFrom(SmiValue& val, const int& from, AsnType t)
{
    if(t != AsnType_Integer)
        return false;
    val.clear();
    val.data().asnType = t;
    val.data().asnValue.number = from;
    return true;
}

template <>
bool smivalTo(const SmiValue& val, int& to)
{
    if(val.syntax() != AsnType_Integer)
        return false;
    to = val.data().asnValue.number;
    return true;
}
// counter64
template <>
bool smivalFrom(SmiValue& val, const int64& from, AsnType t)
{
    if(t != AsnType_Counter64)
        return false;
    val.clear();
    val.data().asnType = t;
    val.data().asnValue.counter64.QuadPart = from;
    return true;
}

template <>
bool smivalTo(const SmiValue& val, int64& to)
{
    if(val.syntax() != AsnType_Counter64)
        return false;

    to = val.data().asnValue.counter64.QuadPart;
    return true;
}

// octets
template <>
bool smivalFrom(SmiValue& val, const std::string& from, AsnType t)
{
    if(t != AsnType_Octets)
        return false;

    val.clear();
    if(!from.empty())
    {
        UINT len = (UINT)from.size();
        BYTE *data = (BYTE *)SnmpUtilMemAlloc(len);
        if(NULL == data)
            return false;
        memcpy(data, from.data(), len);

        val.data().asnValue.string.stream = data;
        val.data().asnValue.string.length = len;
        val.data().asnValue.string.dynamic = TRUE;
    }
    else
    {
        val.data().asnValue.string.stream = NULL;
        val.data().asnValue.string.length = 0;
        val.data().asnValue.string.dynamic = FALSE;
    }
    val.data().asnType = t;

    return true;
}

template <>
bool smivalTo(const SmiValue& val, std::string& to)
{
    if(val.syntax() != AsnType_Octets)
        return false;
    const AsnOctetString& str = val.data().asnValue.string;
    if(NULL != str.stream)
    {
        to.assign((const char*)str.stream, str.length);
    }
    else
    {
        to.clear();
    }
    return true;
}
}} // namespace ZQ::Snmp
