#ifndef __ZQ_Snmp_smival_netsnmp_H__
#define __ZQ_Snmp_smival_netsnmp_H__

// implemation of SmiValue with netsnmp

#include "ZQSnmp.hpp"
#include "SnmpUtil.h" 

namespace ZQ{
namespace Snmp{

typedef netsnmp_variable_list AsnAny;
class SmiValue
{
public:
    SmiValue() { memset(&data_, 0, sizeof(netsnmp_variable_list)); }
    ~SmiValue() { clear(); }
    void clear() {
        snmp_reset_var_buffers(&data_);
    }

    AsnType syntax() const { return data_.type; }

    AsnAny& data() { return data_; }
    const AsnAny& data() const { return data_; }
private:
    SmiValue(const SmiValue&);
    SmiValue& operator=(const SmiValue&);
private:
    // implementation-specified
    netsnmp_variable_list data_;
};
}} // namespace ZQ::Snmp
#endif
