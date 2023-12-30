#ifndef __ZQ_Snmp_smival_win_H__
#define __ZQ_Snmp_smival_win_H__

// implemation of SmiValue with snmpapi.lib

#include "ZQSnmp.hpp"
#include <snmp.h>

#pragma comment(lib,"snmpapi.lib")

namespace ZQ{
namespace Snmp{

class SmiValue
{
public:
    SmiValue() { memset(&data_, 0, sizeof(AsnAny)); }
    ~SmiValue() { SnmpUtilAsnAnyFree(&data_); }
    void clear() { SnmpUtilAsnAnyFree(&data_); }

    AsnType syntax() const { return data_.asnType; }

    AsnAny& data() { return data_; }
    const AsnAny& data() const { return data_; }
private:
    SmiValue(const SmiValue&);
    SmiValue& operator=(const SmiValue&);
private:
    // implementation-specified
    AsnAny data_;
};
}} // namespace ZQ::Snmp
#endif
